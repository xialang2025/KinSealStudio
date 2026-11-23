#include "stage2_file_protection.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QLabel>
#include <QLineEdit>
#include <QMessageBox>
#include <QPushButton>
#include <QScopeGuard>
#include <QStyle>
#include <cstring>
#include <limits>
#include <utility>

#ifdef Q_OS_WIN
#include <qt_windows.h>
#include <bcrypt.h>
#endif

namespace {
constexpr char fileMagic[] = "KSP-CNG-1\r\n";
constexpr qsizetype magicSize = sizeof(fileMagic) - 1;
constexpr qsizetype saltSize = 16;
constexpr qsizetype nonceSize = 12;
constexpr qsizetype tagSize = 16;
constexpr qsizetype saltOffset = magicSize;
constexpr qsizetype nonceOffset = saltOffset + saltSize;
constexpr qsizetype tagOffset = nonceOffset + nonceSize;
static_assert(tagOffset + tagSize == stage2ProtectedFileOverhead);

/******************************************************************************
 * Function Name   : protectionError
 * Description     : Return a localized failure without modifying the output payload.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.23
 ******************************************************************************/
bool protectionError(QString *error, const QString &message) //20261123-17 add
{
    if (error != nullptr)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : validFilePassword
 * Description     : Apply the original password length and ensure lossless UTF-8 encoding.
 * Input Parameters: password
 * Return Value    : True for nonempty passwords within the original limit.
 * Date            : 2026.11.23
 ******************************************************************************/
bool validFilePassword(const QString &password) //20261123-17 add
{
    return !password.isEmpty() && password.size() <= stage2FilePasswordLimit && password.isValidUtf16();
}

#ifdef Q_OS_WIN
constexpr ULONG keySize = 32;
constexpr ULONGLONG derivationRounds = 600000;
constexpr NTSTATUS invalidAuthTag = static_cast<NTSTATUS>(0xc000a002U);
constexpr NTSTATUS invalidOutput = static_cast<NTSTATUS>(0xc000000dU);

/******************************************************************************
 * Function Name   : transformFilePayload
 * Description     : Authenticate the versioned header and transform the payload with AES-256-GCM.
 * Input Parameters: password, header, authenticationTag, input, payloadLength, output, encrypt
 * Return Value    : CNG status. Success means every requested byte was processed.
 * Date            : 2026.11.23
 ******************************************************************************/
NTSTATUS transformFilePayload(const QString &password, const char *header, char *authenticationTag,
                              const char *input, ULONG payloadLength, char *output, bool encrypt) //20261123-17 add
{
    BCRYPT_ALG_HANDLE shaAlgorithm = nullptr;
    BCRYPT_ALG_HANDLE aesAlgorithm = nullptr;
    BCRYPT_KEY_HANDLE aesKey = nullptr;
    BCRYPT_AUTHENTICATED_CIPHER_MODE_INFO auth;
    BYTE derivedKey[keySize] = {};
    QByteArray encodedPassword = password.toUtf8();
    const auto clearPassword = qScopeGuard([&]() {
        SecureZeroMemory(encodedPassword.data(), size_t(encodedPassword.size()));
        SecureZeroMemory(derivedKey, sizeof(derivedKey));
    });
    const auto closeHandles = qScopeGuard([&]() {
        if (aesKey != nullptr)
        {
            BCryptDestroyKey(aesKey);
        }
        if (aesAlgorithm != nullptr)
        {
            BCryptCloseAlgorithmProvider(aesAlgorithm, 0);
        }
        if (shaAlgorithm != nullptr)
        {
            BCryptCloseAlgorithmProvider(shaAlgorithm, 0);
        }
    });
    ULONG written = 0;
    NTSTATUS status;

    status = BCryptOpenAlgorithmProvider(&shaAlgorithm, BCRYPT_SHA256_ALGORITHM, nullptr,
                                         BCRYPT_ALG_HANDLE_HMAC_FLAG);
    if (status < 0)
    {
        return status;
    }
    status = BCryptDeriveKeyPBKDF2(shaAlgorithm, reinterpret_cast<PUCHAR>(encodedPassword.data()),
                                  ULONG(encodedPassword.size()),
                                  reinterpret_cast<PUCHAR>(const_cast<char *>(header + saltOffset)),
                                  ULONG(saltSize), derivationRounds, derivedKey, keySize, 0);
    if (status < 0)
    {
        return status;
    }
    status = BCryptOpenAlgorithmProvider(&aesAlgorithm, BCRYPT_AES_ALGORITHM, nullptr, 0);
    if (status < 0)
    {
        return status;
    }
    status = BCryptSetProperty(aesAlgorithm, BCRYPT_CHAINING_MODE,
                               reinterpret_cast<PUCHAR>(const_cast<wchar_t *>(BCRYPT_CHAIN_MODE_GCM)),
                               ULONG(sizeof(BCRYPT_CHAIN_MODE_GCM)), 0);
    if (status < 0)
    {
        return status;
    }
    status = BCryptGenerateSymmetricKey(aesAlgorithm, &aesKey, nullptr, 0, derivedKey, keySize, 0);
    if (status < 0)
    {
        return status;
    }

    // The authenticated header includes the format identifier, salt and nonce.
    BCRYPT_INIT_AUTH_MODE_INFO(auth);
    auth.pbNonce = reinterpret_cast<PUCHAR>(const_cast<char *>(header + nonceOffset));
    auth.cbNonce = ULONG(nonceSize);
    auth.pbAuthData = reinterpret_cast<PUCHAR>(const_cast<char *>(header));
    auth.cbAuthData = ULONG(tagOffset);
    auth.pbTag = reinterpret_cast<PUCHAR>(authenticationTag);
    auth.cbTag = ULONG(tagSize);
    if (encrypt)
    {
        status = BCryptEncrypt(aesKey, reinterpret_cast<PUCHAR>(const_cast<char *>(input)),
                               payloadLength, &auth, nullptr, 0, reinterpret_cast<PUCHAR>(output),
                               payloadLength, &written, 0);
    }
    else
    {
        status = BCryptDecrypt(aesKey, reinterpret_cast<PUCHAR>(const_cast<char *>(input)),
                               payloadLength, &auth, nullptr, 0, reinterpret_cast<PUCHAR>(output),
                               payloadLength, &written, 0);
    }
    return status >= 0 && written != payloadLength ? invalidOutput : status;
}
#endif

/******************************************************************************
 * Function Name   : passwordMessage
 * Description     : Display the localized confirmation button for validation and help messages.
 * Input Parameters: parent, icon, title, message
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void passwordMessage(QWidget *parent, QMessageBox::Icon icon, const QString &title,
                     const QString &message) //20261123-17 add
{
    QMessageBox dialog(icon, title, message, QMessageBox::Ok, parent);

    dialog.button(QMessageBox::Ok)->setText(QStringLiteral("确定"));
    dialog.exec();
}
}

/******************************************************************************
 * Function Name   : isStage2ProtectedFile
 * Description     : Identify the versioned Lite protected KSP envelope before JSON parsing.
 * Input Parameters: bytes
 * Return Value    : True when the file magic matches.
 * Date            : 2026.11.23
 ******************************************************************************/
bool isStage2ProtectedFile(const QByteArray &bytes) //20261123-17 add
{
    return bytes.startsWith(fileMagic);
}

/******************************************************************************
 * Function Name   : protectStage2ProjectFile
 * Description     : Seal the serialized project under a freshly salted password and random nonce.
 * Input Parameters: plain, password, protectedBytes, error
 * Return Value    : True only after authenticated encryption succeeds.
 * Date            : 2026.11.23
 ******************************************************************************/
bool protectStage2ProjectFile(const QByteArray &plain, const QString &password,
                              QByteArray *protectedBytes, QString *error) //20261123-17 add
{
    if (protectedBytes == nullptr || plain.isEmpty() || !validFilePassword(password))
    {
        return protectionError(error, QStringLiteral("文件密码必须为 1–11 个有效字符。"));
    }

#ifdef Q_OS_WIN
    QByteArray envelope;
    char *bytes;
    NTSTATUS status;

    if (quint64(plain.size()) > std::numeric_limits<ULONG>::max())
    {
        return protectionError(error, QStringLiteral("受密码保护的工程文件过大。"));
    }
    envelope.resizeForOverwrite(stage2ProtectedFileOverhead + plain.size());
    bytes = envelope.data();
    std::memcpy(bytes, fileMagic, size_t(magicSize));
    status = BCryptGenRandom(nullptr, reinterpret_cast<PUCHAR>(bytes + saltOffset),
                             ULONG(saltSize + nonceSize), BCRYPT_USE_SYSTEM_PREFERRED_RNG);
    if (status < 0 || transformFilePayload(password, bytes, bytes + tagOffset, plain.constData(),
                                          ULONG(plain.size()), bytes + stage2ProtectedFileOverhead, true) < 0)
    {
        return protectionError(error, QStringLiteral("无法加密工程文件。"));
    }
    *protectedBytes = std::move(envelope);
    return true;
#else
    return protectionError(error, QStringLiteral("此平台无法写入受密码保护的 Lite 工程文件。"));
#endif
}

/******************************************************************************
 * Function Name   : unprotectStage2ProjectFile
 * Description     : Authenticate before returning any project data to the caller.
 * Input Parameters: protectedBytes, password, plain, needsPassword, error
 * Return Value    : True when the envelope is valid and the password matches.
 * Date            : 2026.11.23
 ******************************************************************************/
bool unprotectStage2ProjectFile(const QByteArray &protectedBytes, const QString &password,
                                QByteArray *plain, bool *needsPassword, QString *error) //20261123-17 add
{
    const qsizetype payloadSize = protectedBytes.size() - stage2ProtectedFileOverhead;

    if (needsPassword != nullptr)
    {
        *needsPassword = false;
    }
    if (plain == nullptr || !isStage2ProtectedFile(protectedBytes) || payloadSize <= 0)
    {
        return protectionError(error, QStringLiteral("受密码保护的工程文件格式不完整。"));
    }
#ifdef Q_OS_WIN
    const char *bytes = protectedBytes.constData();
    QByteArray decoded;
    char tag[tagSize];
    NTSTATUS status;

    if (quint64(payloadSize) > std::numeric_limits<ULONG>::max())
    {
        return protectionError(error, QStringLiteral("受密码保护的工程文件过大。"));
    }
    if (!validFilePassword(password))
    {
        if (needsPassword != nullptr)
        {
            *needsPassword = true;
        }
        return protectionError(error, password.isEmpty() ? QStringLiteral("该文件有密码保护") :
            QStringLiteral("请输入正确的文件密码！"));
    }
    std::memcpy(tag, bytes + tagOffset, size_t(tagSize));
    decoded.resizeForOverwrite(payloadSize);
    status = transformFilePayload(password, bytes, tag, bytes + stage2ProtectedFileOverhead,
                                  ULONG(payloadSize), decoded.data(), false);
    if (status < 0)
    {
        SecureZeroMemory(decoded.data(), size_t(decoded.size()));
        if (status == invalidAuthTag)
        {
            if (needsPassword != nullptr)
            {
                *needsPassword = true;
            }
            return protectionError(error, QStringLiteral("请输入正确的文件密码！"));
        }
        return protectionError(error, QStringLiteral("无法解密工程文件。"));
    }
    *plain = std::move(decoded);
    return true;
#else
    Q_UNUSED(password);
    return protectionError(error, QStringLiteral("此平台无法打开受密码保护的 Lite 工程文件。"));
#endif
}

/******************************************************************************
 * Function Name   : editStage2FileProtection
 * Description     : Edit file protection on a temporary password draft.
 * Input Parameters: parent, initialPassword, result
 * Return Value    : True when the user confirms a valid draft.
 * Date            : 2026.11.23
 ******************************************************************************/
bool editStage2FileProtection(QWidget *parent, const QString &initialPassword, QString *result) //20261123-17 add
{
    QDialog dialog(parent);
    QCheckBox usePassword(QStringLiteral("使用密码保护文件"), &dialog);
    QLabel label(QStringLiteral("密码："), &dialog);
    QLineEdit field(initialPassword, &dialog);
    QLabel warning(&dialog);
    QLabel explanation(QStringLiteral("设置密码后，该文件需要密码才能打开，请牢记您的密码。 （密码区分大小写）"), &dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, &dialog);

    if (result == nullptr)
    {
        return false;
    }
    dialog.setObjectName(QStringLiteral("FileProtectionDialog"));
    dialog.setWindowTitle(QStringLiteral("保护文件"));
    dialog.setFixedSize(386, 220);
    usePassword.setObjectName(QStringLiteral("bUsePWD"));
    usePassword.setGeometry(31, 23, 230, 23);
    usePassword.setChecked(!initialPassword.isEmpty());
    label.setGeometry(85, 54, 46, 25);
    label.setBuddy(&field);
    field.setObjectName(QStringLiteral("sPWD"));
    field.setEchoMode(QLineEdit::Password);
    field.setMaxLength(int(stage2FilePasswordLimit));
    field.setGeometry(131, 56, 211, 23);
    field.setStyleSheet(QStringLiteral("QLineEdit { background: #caeace; }"
                                       "QLineEdit:disabled { background: #adc9e6; }"));
    field.setEnabled(usePassword.isChecked());
    warning.setGeometry(31, 101, 51, 41);
    warning.setAlignment(Qt::AlignCenter);
    warning.setPixmap(dialog.style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(40, 40));
    explanation.setGeometry(100, 101, 245, 50);
    explanation.setWordWrap(true);
    buttons.setObjectName(QStringLiteral("buttonBox"));
    buttons.setGeometry(139, 187, 235, 21);
    buttons.button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons.button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    buttons.button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    for (QAbstractButton *button : buttons.buttons())
    {
        button->setFixedSize(73, 21);
    }
    QObject::connect(&usePassword, &QCheckBox::toggled, &dialog, [&](bool use) {
        field.setEnabled(use);
        if (use)
        {
            field.setFocus();
        }
    });
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (usePassword.isChecked() && !validFilePassword(field.text()))
        {
            passwordMessage(&dialog, QMessageBox::Warning, QStringLiteral("KinSealStudio"),
                field.text().isEmpty() ? QStringLiteral("密码不能为空！") : QStringLiteral("文件密码格式无效。"));
            field.setFocus();
            return;
        }
        dialog.accept();
    });
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::helpRequested, &dialog, [&]() {
        passwordMessage(&dialog, QMessageBox::Information, QStringLiteral("保护文件"),
            QStringLiteral("勾选使用密码保护文件，输入密码并保存工程。重新打开此文件时需要输入同一密码，密码区分大小写。"
                           "取消勾选并保存工程可解除文件保护。"));
    });
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    *result = usePassword.isChecked() ? field.text() : QString();
    return true;
}

/******************************************************************************
 * Function Name   : requestStage2FilePassword
 * Description     : Prompt for one protected file password without persisting it in settings.
 * Input Parameters: parent, password
 * Return Value    : True and a password draft when confirmed.
 * Date            : 2026.11.23
 ******************************************************************************/
bool requestStage2FilePassword(QWidget *parent, QString *password) //20261123-17 add
{
    QDialog dialog(parent);
    QLabel icon(&dialog);
    QLabel message(QStringLiteral("该文件有密码保护"), &dialog);
    QLabel label(QStringLiteral("请输入密码："), &dialog);
    QLineEdit input(&dialog);
    QPushButton confirm(QStringLiteral("确定"), &dialog);
    QPushButton cancel(QStringLiteral("取消"), &dialog);

    if (password == nullptr)
    {
        return false;
    }
    dialog.setObjectName(QStringLiteral("QFilePassWordDlg"));
    dialog.setWindowTitle(QStringLiteral("密码"));
    dialog.setFixedSize(424, 100);
    dialog.setStyleSheet(QStringLiteral("QDialog#QFilePassWordDlg { background: #e0edfe; }"
                                        "QLineEdit { background: #caeace; }"));
    icon.setGeometry(10, 11, 61, 60);
    icon.setAlignment(Qt::AlignCenter);
    icon.setPixmap(dialog.style()->standardIcon(QStyle::SP_MessageBoxWarning).pixmap(48, 48));
    message.setGeometry(77, 11, 210, 26);
    label.setGeometry(155, 40, 75, 20);
    label.setBuddy(&input);
    input.setObjectName(QStringLiteral("FilePassWordEntry"));
    input.setEchoMode(QLineEdit::Password);
    input.setGeometry(230, 40, 181, 20);
    confirm.setObjectName(QStringLiteral("OKBtn"));
    confirm.setGeometry(261, 71, 73, 21);
    confirm.setDefault(true);
    cancel.setObjectName(QStringLiteral("CancelBtn"));
    cancel.setGeometry(338, 71, 73, 21);
    QObject::connect(&confirm, &QPushButton::clicked, &dialog, &QDialog::accept);
    QObject::connect(&cancel, &QPushButton::clicked, &dialog, &QDialog::reject);
    input.setFocus();
    if (dialog.exec() != QDialog::Accepted)
    {
        return false;
    }
    *password = input.text();
    return true;
}
