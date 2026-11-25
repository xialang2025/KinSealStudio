#include "auxiliary_settings.h"
#include "dialog_button_text.h"

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QLockFile>
#include <QMessageBox>
#include <QPushButton>
#include <QRadioButton>
#include <QRegularExpression>
#include <QSaveFile>
#include <QSet>
#include <QSettings>
#include <QSpinBox>
#include <QTabWidget>

/******************************************************************************
 * Function Name   : AuxiliarySettings::load
 * Description     : Load the studio-wide save preferences and bound external values to the native limits.
 * Input Parameters: None
 * Return Value    : Save and backup preferences.
 * Date            : 2026.11.25
 ******************************************************************************/
AuxiliarySettings AuxiliarySettings::load() //20261125-82 add
{
    QSettings ini(QSettings::defaultFormat(), QSettings::UserScope, QStringLiteral("KinSealStudioLite"), QStringLiteral("Editor"));
    AuxiliarySettings result;

    result.autoSave = ini.value(QStringLiteral("display/bAutoSave"), false).toBool();
    result.minutes = qBound(3, ini.value(QStringLiteral("display/AutoSaveTime"), 3).toInt(), 60);
    result.backup = ini.value(QStringLiteral("autoSaveProInfo/IsBakup"), false).toBool();
    result.timedMode = ini.value(QStringLiteral("autoSaveProInfo/BakupMode"), 0).toInt() == 1;
    result.customPath = ini.value(QStringLiteral("autoSaveProInfo/BakupPath"), 0).toInt() == 1;
    result.copies = qBound(3, ini.value(QStringLiteral("autoSaveProInfo/BakupTotals"), 5).toInt(), 20);
    result.directory = ini.value(QStringLiteral("autoSaveProInfo/BakupFilePath")).toString();
    return result;
}

/******************************************************************************
 * Function Name   : AuxiliarySettings::store
 * Description     : Commit all accepted auxiliary preferences to the editor's settings store.
 * Input Parameters: None
 * Return Value    : True after a successful sync.
 * Date            : 2026.11.25
 ******************************************************************************/
bool AuxiliarySettings::store() const //20261125-82 add
{
    QSettings ini(QSettings::defaultFormat(), QSettings::UserScope, QStringLiteral("KinSealStudioLite"), QStringLiteral("Editor"));

    ini.setValue(QStringLiteral("display/bAutoSave"), autoSave);
    ini.setValue(QStringLiteral("display/AutoSaveTime"), minutes);
    ini.setValue(QStringLiteral("autoSaveProInfo/IsBakup"), backup);
    ini.setValue(QStringLiteral("autoSaveProInfo/BakupMode"), int(timedMode));
    ini.setValue(QStringLiteral("autoSaveProInfo/BakupPath"), int(customPath));
    ini.setValue(QStringLiteral("autoSaveProInfo/BakupTotals"), copies);
    ini.setValue(QStringLiteral("autoSaveProInfo/BakupFilePath"), directory);
    ini.sync();
    return ini.status() == QSettings::NoError;
}

/******************************************************************************
 * Function Name   : AuxiliarySettings::backUpSavedFile
 * Description     : Copy committed bytes to the first free numbered slot or replace the oldest one atomically.
 * Input Parameters: fileName, the document just saved.
 * Return Value    : Empty on success or a diagnostic. Old copies survive a failed replacement.
 * Date            : 2026.11.25
 ******************************************************************************/
QString AuxiliarySettings::backUpSavedFile(const QString &fileName) const //20261125-82 add
{
    const QFileInfo source(fileName);
    const QDir dest(customPath ? directory : source.absolutePath());
    const QString prefix = source.completeBaseName() + QStringLiteral("-bak");
    const QString suffix = QStringLiteral(".") + source.suffix();
    const QRegularExpression pattern(QStringLiteral("^%1([1-9][0-9]*)%2$").arg(
        QRegularExpression::escape(prefix), QRegularExpression::escape(suffix)), QRegularExpression::CaseInsensitiveOption);
    QLockFile guard(dest.filePath(prefix + suffix + QStringLiteral(".lock")));
    QList<QFileInfo> existing;
    QSet<QString> occupied;
    QString target;
    QFile input(fileName);
    QSaveFile output;
    QByteArray buffer(65536, Qt::Uninitialized);
    qint64 count;
    const int limit = qBound(3, copies, 20);

    if (!backup) { return {}; }
    if ((customPath && directory.isEmpty()) || !dest.exists()) { return QStringLiteral("工程备份路径不存在，请重新设置！"); }
    if (!guard.tryLock(50)) { return QStringLiteral("无法锁定工程备份目录：%1").arg(dest.path()); }
    if (!input.open(QIODevice::ReadOnly)) { return input.errorString(); }
    // Match complete numeric filenames only. Other documents, directories and links never enter rotation.
    for (const QFileInfo &entry : dest.entryInfoList(QDir::Files | QDir::NoDotAndDotDot, QDir::Time | QDir::Reversed)) {
        if (pattern.match(entry.fileName()).hasMatch()) {
            occupied.insert(entry.fileName().toCaseFolded());
            if (!entry.isSymLink()) { existing.append(entry); }
        }
    }
    if (existing.size() >= limit) {
        target = existing.takeFirst().absoluteFilePath();
    } else {
        for (int slot = 1; slot <= limit; ++slot) {
            const QString name = prefix + QString::number(slot) + suffix;
            const QString candidate = dest.filePath(name);
            if (!occupied.contains(name.toCaseFolded()) && !QFileInfo::exists(candidate) && !QFileInfo(candidate).isSymLink()) {
                target = candidate;
                break;
            }
        }
    }
    if (target.isEmpty()) { return QStringLiteral("没有可写的工程备份位置。"); }
    output.setFileName(target);
    if (!output.open(QIODevice::WriteOnly)) { return output.errorString(); }
    while ((count = input.read(buffer.data(), buffer.size())) > 0) {
        if (output.write(buffer.constData(), count) != count) { return output.errorString(); }
    }
    if (count < 0) { return input.errorString(); }
    if (!output.commit()) { return output.errorString(); }
    // When the limit is reduced, trim obsolete copies only after the replacement exists.
    while (existing.size() >= limit) {
        const QString oldest = existing.takeFirst().absoluteFilePath();
        if (!QFile::remove(oldest)) { return QStringLiteral("无法清理旧备份：%1").arg(oldest); }
    }
    return {};
}

/******************************************************************************
 * Function Name   : editAuxiliarySettings
 * Description     : Open the native auxiliary settings page and publish choices only after its confirmation steps.
 * Input Parameters: parent, projectFile, settings.
 * Return Value    : True after the preferences were persisted.
 * Date            : 2026.11.25
 ******************************************************************************/
bool editAuxiliarySettings(QWidget *parent, const QString &projectFile, AuxiliarySettings *settings) //20261125-82 add
{
    QDialog dialog(parent);
    QVBoxLayout outer(&dialog);
    QTabWidget tabs;
    QWidget page;
    QGridLayout pageLayout(&page);
    QCheckBox autoSave(dialogButtonText(QStringLiteral("启用自动保存"), "Enable auto save"));
    QSpinBox minutes;
    QLabel unit(dialogButtonText(QStringLiteral("分钟（3-60分钟）"), "minutes (3-60 minutes)"));
    QGroupBox backup(dialogButtonText(QStringLiteral("启用自动备份工程"), "Enable automatic project backup"));
    QVBoxLayout backupLayout(&backup);
    QGroupBox mode(dialogButtonText(QStringLiteral("备份方式"), "Backup mode"));
    QVBoxLayout modeLayout(&mode);
    QRadioButton onSave(dialogButtonText(QStringLiteral("点击“保存”按钮时备份"), "Back up when Save is clicked"));
    QRadioButton onTime(dialogButtonText(QStringLiteral("跟随自动保存时间备份"), "Back up with auto save"));
    QGroupBox pathBox(dialogButtonText(QStringLiteral("备份路径"), "Backup path"));
    QGridLayout pathLayout(&pathBox);
    QRadioButton original(dialogButtonText(QStringLiteral("原组态路径"), "Original project path"));
    QRadioButton custom(dialogButtonText(QStringLiteral("自定义路径"), "Custom path"));
    QHBoxLayout pathRow;
    QLineEdit path;
    QPushButton browse(dialogButtonText(QStringLiteral("浏览..."), "Browse..."));
    QLabel naming(dialogButtonText(QStringLiteral("备份工程命名方式：原工程-bak*.ksp"), "Backup name: original project-bak*.ksp"));
    QLabel copiesLabel(dialogButtonText(QStringLiteral("工程备份数量："), "Number of project backups:"));
    QSpinBox copies;
    QLabel tip(dialogButtonText(QStringLiteral("注意：超过工程备份数量时，自动覆盖旧工程"), "Older backups are replaced when the limit is reached."));
    QHBoxLayout countRow;
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    const QString originalDir = projectFile.isEmpty() ? QString() : QDir::toNativeSeparators(QFileInfo(projectFile).absolutePath());
    AuxiliarySettings draft = *settings;

    dialog.setObjectName(QStringLiteral("ProjectSetDlg"));
    dialog.setWindowTitle(dialogButtonText(QStringLiteral("辅助功能"), "Auxiliary function"));
    dialog.setWindowFlags(dialog.windowFlags() & ~Qt::WindowContextHelpButtonHint);
    dialog.setFixedSize(428, 466);
    tabs.setObjectName(QStringLiteral("tabWidget"));
    page.setObjectName(QStringLiteral("tab_2"));
    backup.setObjectName(QStringLiteral("groupBox_autoBakupPro")); backup.setCheckable(true);
    mode.setObjectName(QStringLiteral("groupBox_BakupMode"));
    pathBox.setObjectName(QStringLiteral("groupBox_BakupPath"));
    autoSave.setObjectName(QStringLiteral("checkBox_autoSave"));
    minutes.setObjectName(QStringLiteral("spinBox_autoSave")); minutes.setRange(3, 60);
    unit.setObjectName(QStringLiteral("label_autoSave"));
    onSave.setObjectName(QStringLiteral("rBtn_saveBackup")); onTime.setObjectName(QStringLiteral("rBtn_timeBackup"));
    original.setObjectName(QStringLiteral("rBtn_oriProPath")); custom.setObjectName(QStringLiteral("rBtn_customPath"));
    path.setObjectName(QStringLiteral("lineEdit_bakupPath")); browse.setObjectName(QStringLiteral("btn_browse"));
    copies.setObjectName(QStringLiteral("spinBox_backupNums")); copies.setRange(3, 20);
    naming.setObjectName(QStringLiteral("label_bakupName")); tip.setObjectName(QStringLiteral("label_bakupTips"));
    tip.setStyleSheet(QStringLiteral("QLabel { color: blue; }"));
    buttons.setObjectName(QStringLiteral("buttonBox"));
    buttons.button(QDialogButtonBox::Ok)->setText(dialogButtonText(QStringLiteral("确定"), "OK"));
    buttons.button(QDialogButtonBox::Cancel)->setText(dialogButtonText(QStringLiteral("取消"), "Cancel"));
    pageLayout.addWidget(&autoSave, 0, 0); pageLayout.addWidget(&minutes, 0, 1); pageLayout.addWidget(&unit, 0, 2);
    pageLayout.addWidget(&backup, 1, 0, 1, 3);
    modeLayout.addWidget(&onSave); modeLayout.addWidget(&onTime);
    pathLayout.addWidget(&original, 0, 0); pathLayout.addWidget(&custom, 0, 1);
    pathRow.addWidget(&path, 1); pathRow.addWidget(&browse);
    pathLayout.addLayout(&pathRow, 1, 0, 1, 2);
    pathLayout.addWidget(&naming, 2, 0, 1, 2);
    backupLayout.addWidget(&mode); backupLayout.addWidget(&pathBox);
    countRow.addWidget(&copiesLabel); countRow.addWidget(&copies); countRow.addStretch();
    backupLayout.addLayout(&countRow); backupLayout.addWidget(&tip);
    tabs.addTab(&page, dialogButtonText(QStringLiteral("自动保存设置"), "Auto save settings"));
    outer.addWidget(&tabs); outer.addWidget(&buttons);
    autoSave.setChecked(draft.autoSave); minutes.setValue(draft.minutes);
    onTime.setEnabled(draft.autoSave);
    (draft.timedMode ? onTime : onSave).setChecked(true);
    (draft.customPath ? custom : original).setChecked(true);
    path.setText(draft.customPath ? draft.directory : originalDir); browse.setEnabled(draft.customPath);
    copies.setValue(draft.copies); backup.setChecked(draft.backup);
    QObject::connect(&autoSave, &QCheckBox::clicked, &dialog, [&](bool checked) {
        onSave.setChecked(true); onTime.setEnabled(checked);
    });
    QObject::connect(&custom, &QRadioButton::toggled, &dialog, [&](bool selected) {
        browse.setEnabled(selected); path.setText(selected ? QString() : originalDir);
    });
    QObject::connect(&browse, &QPushButton::clicked, &dialog, [&]() {
        const QString chosen = QFileDialog::getExistingDirectory(&dialog,
            dialogButtonText(QStringLiteral("选择要存储的位置"), "Choose the storage directory"), path.text());
        if (!chosen.isEmpty()) { path.setText(QDir::toNativeSeparators(chosen)); }
    });
    QObject::connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    QObject::connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        if (autoSave.isChecked() || backup.isChecked()) {
            // The native question asks whether to cancel BOTH settings. OK disables them and continues saving.
            const auto response = QMessageBox::warning(&dialog, QStringLiteral("KinSealStudio"),
                dialogButtonText(QStringLiteral("不建议使用该功能，是否取消设置"), "It is not recommended to use this function. Do you want to cancel the setting"),
                QMessageBox::Ok | QMessageBox::No);
            if (response == QMessageBox::Ok) { autoSave.setChecked(false); backup.setChecked(false); }
        }
        if (backup.isChecked() && custom.isChecked() && (path.text().trimmed().isEmpty() || !QFileInfo(path.text()).isDir())) {
            QMessageBox::warning(&dialog, QStringLiteral("KinSealStudio"),
                dialogButtonText(QStringLiteral("自定义路径不存在，请重新选择！"), "The custom path does not exist. Please select again!"));
            return;
        }
        draft.autoSave = autoSave.isChecked(); draft.minutes = minutes.value(); draft.backup = backup.isChecked();
        draft.timedMode = onTime.isChecked(); draft.customPath = custom.isChecked(); draft.copies = copies.value();
        draft.directory = path.text();
        if (!draft.store()) {
            QMessageBox::warning(&dialog, dialog.windowTitle(), dialogButtonText(QStringLiteral("无法保存设置。"), "Unable to save settings."));
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() != QDialog::Accepted) { return false; }
    *settings = draft;
    return true;
}
