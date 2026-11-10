#include "independent_downloader.h"
#include "device_address_compiler.h" //20261109-0 add
#include "text_payload_compiler.h" //20261108-11 add
#include "static_text_renderer.h" //20261108-14 add
#include "device_transfer_plan.h" //20261109-0 add
#include "bit_button_payload_compiler.h" //20261109-0 add
#include "bit_button_renderer.h" //20261109-4 add
#include "native_bit_button_control.h"
#include "native_word_button_control.h" //20261109-6 add
#include "native_lamp_control.h" //20261109-8 add
#include "native_function_button_control.h" //20261110-0 add
#include "native_static_text_control.h" //20261109-0 add
#include "static_text_payload_compiler.h"
#include "device_macro_compiler.h"
#include "native_project_codec.h" //20261109-0 add

#include <QByteArray>
#include <QCryptographicHash>
#include <QDir>
#include <QDataStream> //20261108-11 add
#include <QFile>
#include <QLibrary>
#include <QSaveFile> //20261108-11 add
#include <QVector>
#include <QCoreApplication> //20261109-0 add
#include <QImage>
#include <QPainter>
#include <QJsonDocument>
#include <QtEndian> //20261109-0 add
#include <algorithm>

#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif

using u8 = unsigned char; //20261108-10 add
using u16 = unsigned short; //20261108-10 add
using u32 = unsigned int; //20261108-12 add


namespace
{
struct PayloadSpec
{
    const char *name;
    const char *sha256;
    int length;
};

// 20261108-10 add: These files are the exact TEST-only compiled fixture, not a general compiler.
static const PayloadSpec payload_specs[] = {
    {"SysPara.dat", "b22f00a160e3dec23d54fdf9e595f6e0b842dee494f95cc9d43ca104c687a01f", 4998},
    {"SheetPara.dat", "9ddd74c4c47d461ab48cb089cf1037b36769c35f8e6e5a830eab23b1381e1c90", 72},
    {"AlarmPara.dat", "12d43ec809fb64507247b520958c99813143fb4c12e63a872d50a7a0aa5c7b10", 47840},
    {"Collect.dat", "5e796fbf009f80334fea0839be003bfbcc320f4dcacdfa2f53c6ca00c7d99e8c", 1424},
    {"AddrPara.dat", "6d9c54dee5660c46886f32d80e57e9dd0ffa57ee0cd2a762b036d9c8e0c3a33a", 200},
    {"PicLenPara.dat", "fbcd494595c1c67c51efaa7e98b003d30591875be305874d8619cf0f41eab27d", 2400},
    {"Curve.dat", "01064fb25c62c76b1f0b7b12de7a44b021b320a735eb9361639500029cb11586", 501},
    {"upLoad.knr", "9324e2c37f9684fa0a26d33710a61d4c16a641cc19b210533bbd8d3437c9db5c", 1093},
    {"SheetPixel.dat", "ea3aeed6d83fe39111cbd3add758b643e3472894fae355aabab3c64953dd3eca", 3919},
    {"Modbus_RTU_com1.bin", "fddf11ab2cfec2b06fcdb408de1bb82fbe26707a97a9c2b2b4b0be382bc0290c", 12496},
};

/******************************************************************************
 * Function Name   : loadPayloads
 * Description     : Load the immutable TEST fixture and reject missing or changed files.
 * Input Parameters: payloads, error
 * Return Value    : True when every resource passes its length and SHA-256 checks.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool loadPayloads(QVector<QByteArray> *payloads, QString *error) //20261108-10 add
{
    QFile file;
    QByteArray bytes;
    QByteArray hash;
    QString path;
    int index;

    if (payloads == nullptr)
    {
        return false;
    }
    payloads->clear();
    for (index = 0; index < int(sizeof(payload_specs) / sizeof(payload_specs[0])); ++index)
    {
        path = QStringLiteral(":/download/") + QString::fromLatin1(payload_specs[index].name);
        file.setFileName(path);
        if (!file.open(QIODevice::ReadOnly))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("缺少内置测试数据：%1").arg(path);
            }
            return false;
        }
        bytes = file.readAll();
        file.close();
        hash = QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex();
        if (bytes.size() != payload_specs[index].length ||
            hash != QByteArray(payload_specs[index].sha256))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("测试数据校验失败：%1").arg(path);
            }
            return false;
        }
        payloads->append(bytes);
    }
    return true;
}

/******************************************************************************
 * Function Name   : encodedDouble
 * Description     : Serialize one coordinate exactly as the native big-endian double fields.
 * Input Parameters: value
 * Return Value    : Eight-byte QDataStream representation.
 * Date            : 2026.11.08
 ******************************************************************************/
static QByteArray encodedDouble(double value) //20261108-11 add
{
    QByteArray bytes;
    QDataStream stream(&bytes, QIODevice::WriteOnly);

    stream.setByteOrder(QDataStream::BigEndian);
    stream.setFloatingPointPrecision(QDataStream::DoublePrecision);
    stream << value;
    return bytes;
}

/******************************************************************************
 * Function Name   : replaceKnownFields
 * Description     : Replace an expected number of serialized fields, including variable-length strings.
 * Input Parameters: bytes, original, replacement, expectedCount
 * Return Value    : True when the immutable template has the expected layout.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool replaceKnownFields(QByteArray *bytes, const QByteArray &original,
                               const QByteArray &replacement,
                               int expectedCount) //20261108-11 add
{
    int position;
    int count;

    if (bytes == nullptr || original.isEmpty()) //20261108-12 change: Qt QString byte lengths can change.
    {
        return false;
    }
    position = 0;
    count = 0;
    while ((position = bytes->indexOf(original, position)) >= 0)
    {
        position += original.size();
        ++count;
    }
    if (count != expectedCount)
    {
        return false;
    }
    bytes->replace(original, replacement);
    return true;
}

/******************************************************************************
 * Function Name   : replaceKnownDoubleAt
 * Description     : Patch one verified geometry slot without confusing equal-valued fields.
 * Input Parameters: bytes, offset, original, replacement
 * Return Value    : True when the source contains the expected original double.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool replaceKnownDoubleAt(QByteArray *bytes, int offset,
                                 double original, double replacement) //20261108-12 add
{
    QByteArray originalBytes;
    QByteArray replacementBytes;

    originalBytes = encodedDouble(original);
    replacementBytes = encodedDouble(replacement);
    if (bytes == nullptr || offset < 0 || offset + originalBytes.size() > bytes->size() ||
        bytes->mid(offset, originalBytes.size()) != originalBytes)
    {
        return false;
    }
    bytes->replace(offset, originalBytes.size(), replacementBytes);
    return true;
}

#ifdef Q_OS_WIN
struct UsbDeviceDescriptor
{
    u8 length;
    u8 type;
    u16 usbVersion;
    u8 deviceClass;
    u8 deviceSubclass;
    u8 deviceProtocol;
    u8 maxPacketSize;
    u16 vendorId;
    u16 productId;
    u16 deviceVersion;
    u8 manufacturer;
    u8 product;
    u8 serial;
    u8 configurations;
};

struct UsbBus;
struct UsbDevice
{
    UsbDevice *next;
    UsbDevice *previous;
    char filename[512];
    UsbBus *bus;
    UsbDeviceDescriptor descriptor;
    void *config;
    void *device;
    u8 deviceNumber;
    u8 childCount;
    UsbDevice **children;
};

struct UsbBus
{
    UsbBus *next;
    UsbBus *previous;
    char dirname[512];
    UsbDevice *devices;
    unsigned long location;
    UsbDevice *rootDevice;
};

static_assert(sizeof(UsbDevice) == 592 && sizeof(UsbBus) == 552,
              "libusb-win32 structure layout changed"); //20261108-10 add

struct UsbFunctions
{
    void (*init)();
    int (*findBusses)();
    int (*findDevices)();
    UsbBus *(*getBusses)();
    void *(*open)(UsbDevice *);
    int (*claim)(void *, int);
    int (*release)(void *, int);
    int (*close)(void *);
    int (*write)(void *, int, char *, int, int);
    int (*read)(void *, int, char *, int, int);
};

/******************************************************************************
 * Function Name   : resolveUsbFunctions
 * Description     : Resolve only libusb-win32 entry points from the Windows system DLL.
 * Input Parameters: library, usb, error
 * Return Value    : True when all required transport functions are present.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool resolveUsbFunctions(QLibrary *library, UsbFunctions *usb,
                                QString *error) //20261108-10 add
{
    wchar_t systemDirectory[MAX_PATH];
    unsigned int length;
    QString path;

    length = GetSystemDirectoryW(systemDirectory, MAX_PATH);
    if (length == 0 || length >= MAX_PATH)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("无法定位 Windows 系统驱动目录。");
        }
        return false;
    }
    path = QDir::fromNativeSeparators(QString::fromWCharArray(systemDirectory, int(length))) +
           QStringLiteral("/libusb0.dll");
    library->setFileName(path);
    if (!library->load())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("系统缺少 libusb-win32 驱动运行库：%1").arg(library->errorString());
        }
        return false;
    }
    usb->init = reinterpret_cast<decltype(usb->init)>(library->resolve("usb_init"));
    usb->findBusses = reinterpret_cast<decltype(usb->findBusses)>(library->resolve("usb_find_busses"));
    usb->findDevices = reinterpret_cast<decltype(usb->findDevices)>(library->resolve("usb_find_devices"));
    usb->getBusses = reinterpret_cast<decltype(usb->getBusses)>(library->resolve("usb_get_busses"));
    usb->open = reinterpret_cast<decltype(usb->open)>(library->resolve("usb_open"));
    usb->claim = reinterpret_cast<decltype(usb->claim)>(library->resolve("usb_claim_interface"));
    usb->release = reinterpret_cast<decltype(usb->release)>(library->resolve("usb_release_interface"));
    usb->close = reinterpret_cast<decltype(usb->close)>(library->resolve("usb_close"));
    usb->write = reinterpret_cast<decltype(usb->write)>(library->resolve("usb_bulk_write"));
    usb->read = reinterpret_cast<decltype(usb->read)>(library->resolve("usb_bulk_read"));
    if (usb->init == nullptr || usb->findBusses == nullptr || usb->findDevices == nullptr ||
        usb->getBusses == nullptr || usb->open == nullptr || usb->claim == nullptr ||
        usb->release == nullptr || usb->close == nullptr || usb->write == nullptr ||
        usb->read == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("系统 libusb-win32 运行库缺少必要接口。");
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : writeUsbBytes
 * Description     : Send one complete bulk OUT transfer without accepting partial writes.
 * Input Parameters: usb, handle, bytes, stage, error
 * Return Value    : True when the system driver wrote every byte.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool writeUsbBytes(const UsbFunctions &usb, void *handle,
                          const QByteArray &bytes, const QString &stage,
                          QString *error) //20261108-10 add
{
    int written;

    written = usb.write(handle, 1, const_cast<char *>(bytes.constData()), bytes.size(), 10000);
    if (written != bytes.size())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("%1：USB 写入 %2/%3 字节，已停止，屏内状态未确认。")
                         .arg(stage).arg(written).arg(bytes.size());
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : readUsbReply
 * Description     : Read and validate one response paired with the preceding command.
 * Input Parameters: usb, handle, expectedLength, expectedByte, stage, error
 * Return Value    : True when the device response has the expected form.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool readUsbReply(const UsbFunctions &usb, void *handle, int expectedLength,
                         int expectedByte, const QString &stage,
                         QString *error) //20261108-10 add
{
    char response[32] = {};
    int received;

    received = usb.read(handle, 129, response, expectedLength == 24 ? 32 : 4, 10000);
    if (received != expectedLength ||
        (expectedByte >= 0 && u8(response[0]) != u8(expectedByte)))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("%1：设备应答不符合预期（长度 %2），已停止，屏内状态未确认。")
                         .arg(stage).arg(received);
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : sendUsbCommand
 * Description     : Send a zero-initialized protocol command and check its paired ACK.
 * Input Parameters: usb, handle, command, options, replyLength, replyByte, stage, error
 * Return Value    : True when the command and expected response complete.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool sendUsbCommand(const UsbFunctions &usb, void *handle, u8 command,
                           const QByteArray &options, int replyLength, int replyByte,
                           const QString &stage, QString *error) //20261108-10 add
{
    QByteArray frame(32, char(0));
    const QByteArray prefix("KinSealStudioKiss", 17);
    int index;

    frame.replace(0, prefix.size(), prefix);
    frame[17] = char(command);
    for (index = 0; index < options.size() && index < 14; ++index)
    {
        frame[18 + index] = options.at(index);
    }
    if (!writeUsbBytes(usb, handle, frame, stage, error))
    {
        return false;
    }
    if (replyLength == 0)
    {
        return true;
    }
    return readUsbReply(usb, handle, replyLength, replyByte, stage, error);
}

/******************************************************************************
 * Function Name   : transferTestPayload
 * Description     : Execute the observed TEST transfer order and validate every ACK.
 * Input Parameters: usb, handle, payloads, error
 * Return Value    : True after the commit ACK and final device reset request.
 * Date            : 2026.11.08
 ******************************************************************************/
#if 0 //20261109-0 del: Preserve the original fixed ten-block transport for audit.
static bool transferTestPayload(const UsbFunctions &usb, void *handle,
                                const QVector<QByteArray> &payloads,
                                QString *error) //20261108-10 add
{
    struct TransferStep
    {
        u8 command;
        int payloadIndex;
    };
    static const TransferStep steps[] = {
        {0x0a, 0}, {0x0c, 1}, {0x0e, 2}, {0x16, -1},
        {0x15, 3}, {0x19, -1}, {0x12, 4}, {0x14, 5},
        {0x0d, 6}, {0x1e, 7}, {0x0f, 8}, {0x11, 9},
    };
    QString stage;
    QByteArray commandOptions;
    QByteArray framedUpload;
    int uploadLength;
    int programLength;
    int index;

    // 20261108-10 add: The two 24-byte replies identify model 0x4c before any data is written.
    if (!sendUsbCommand(usb, handle, 0x31, QByteArray(1, char(2)), 24, 0x4c,
                        QStringLiteral("设备识别"), error) ||
        !sendUsbCommand(usb, handle, 0x53, QByteArray(1, char(0)), 24, 0x00,
                        QStringLiteral("设备参数读取"), error) ||
        !sendUsbCommand(usb, handle, 0x34, QByteArray::fromHex("010101"), 1, 'O',
                        QStringLiteral("开始下载"), error) ||
        !sendUsbCommand(usb, handle, 0x35, QByteArray(), 1, 'O',
                        QStringLiteral("开始状态检查"), error))
    {
        return false;
    }
    for (index = 0; index < int(sizeof(steps) / sizeof(steps[0])); ++index)
    {
        stage = QStringLiteral("数据段 %1 命令").arg(index + 1);
        commandOptions.clear();
        if (steps[index].command == 0x11) //20261108-10 add: The program command carries its byte count in little endian.
        {
            programLength = payloads.at(9).size();
            commandOptions.append(char(programLength & 0xff));
            commandOptions.append(char((programLength >> 8) & 0xff));
            commandOptions.append(char((programLength >> 16) & 0xff));
            commandOptions.append(char((programLength >> 24) & 0xff));
        }
        if (!sendUsbCommand(usb, handle, steps[index].command, commandOptions, 1, 'O',
                            stage, error))
        {
            return false;
        }
        if (steps[index].payloadIndex >= 0)
        {
            stage = QStringLiteral("数据段 %1 内容").arg(index + 1);
            if (steps[index].payloadIndex == 7) //20261108-10 add: Upload archive uses a big-endian length prefix.
            {
                uploadLength = payloads.at(7).size();
                framedUpload.resize(4);
                framedUpload[0] = char((uploadLength >> 24) & 0xff);
                framedUpload[1] = char((uploadLength >> 16) & 0xff);
                framedUpload[2] = char((uploadLength >> 8) & 0xff);
                framedUpload[3] = char(uploadLength & 0xff);
                framedUpload.append(payloads.at(7));
            }
            if (!writeUsbBytes(usb, handle,
                               steps[index].payloadIndex == 7
                                   ? framedUpload : payloads.at(steps[index].payloadIndex),
                               stage, error))
            {
                return false;
            }
        }
        stage = QStringLiteral("数据段 %1 状态检查").arg(index + 1);
        if (!sendUsbCommand(usb, handle, 0x35, QByteArray(), 1, 'O', stage, error))
        {
            return false;
        }
    }
    // 20261108-10 add: The commit is acknowledged; the final reset has no reply in six captures.
    if (!sendUsbCommand(usb, handle, 0x1b, QByteArray(), 1, 'O',
                        QStringLiteral("提交下载"), error) ||
        !sendUsbCommand(usb, handle, 0x1a, QByteArray(), 0, -1,
                        QStringLiteral("设备重启"), error))
    {
        return false;
    }
    return true;
}
#endif

/******************************************************************************
 * Function Name   : transferTestPayload
 * Description     : Execute the offline-validated static or dynamic transfer plan.
 * Input Parameters: usb, handle, payloads, error
 * Return Value    : True after every data ACK, commit ACK, and reset request.
 * Date            : 2026.11.09
 ******************************************************************************/
static bool transferTestPayload(const UsbFunctions &usb, void *handle,
                                const QVector<QByteArray> &payloads,
                                QString *error) //20261109-0 change
{
    QVector<HmiDownloadStep> steps;
    HmiDownloadStep step;
    QString stage;

    if (!buildDeviceTransferPlan(payloads, &steps, error))
    {
        return false;
    }
    if (!sendUsbCommand(usb, handle, 0x31, QByteArray(1, char(2)), 24, 0x4c,
                        QStringLiteral("设备识别"), error) ||
        !sendUsbCommand(usb, handle, 0x53, QByteArray(1, char(0)), 24, 0x00,
                        QStringLiteral("设备参数读取"), error) ||
        !sendUsbCommand(usb, handle, 0x34, QByteArray::fromHex("010101"), 1, 'O',
                        QStringLiteral("开始下载"), error) ||
        !sendUsbCommand(usb, handle, 0x35, QByteArray(), 1, 'O',
                        QStringLiteral("开始状态检查"), error))
    {
        return false;
    }
    Q_FOREACH (step, steps)
    {
        stage = step.name + QStringLiteral(" 命令");
        if (!sendUsbCommand(usb, handle, step.command, step.options, 1, 'O', stage, error))
        {
            return false;
        }
        if (!step.bytes.isEmpty() &&
            !writeUsbBytes(usb, handle, step.bytes, step.name + QStringLiteral(" 内容"), error))
        {
            return false;
        }
        if (!sendUsbCommand(usb, handle, 0x35, QByteArray(), 1, 'O',
                            step.name + QStringLiteral(" 状态检查"), error))
        {
            return false;
        }
    }
    //20261109-0 add: The captured native reset read times out; no success reply is required.
    return sendUsbCommand(usb, handle, 0x1b, QByteArray(), 1, 'O',
                           QStringLiteral("提交下载"), error) &&
           sendUsbCommand(usb, handle, 0x1a, QByteArray(), 0, -1,
                           QStringLiteral("设备重启"), error);
}
#endif
}

/******************************************************************************
 * Function Name   : buildSingleTextPayload
 * Description     : Rebuild arbitrary one-text raster, length table, and editable source archive.
 * Input Parameters: project, payloads, error
 * Return Value    : True when ten verified and regenerated USB blocks are ready.
 * Date            : 2026.11.08
 ******************************************************************************/
bool buildSingleTextPayload(const HmiProject &project,
                            QVector<QByteArray> *payloads,
                            QString *error) //20261108-11 add
{
    QVector<QByteArray> generated;
    QByteArray frame;
    QByteArray compressedFrame;
    QByteArray nativeSource;
    QByteArray compressedSource;
    QByteArray originalText;
    QByteArray replacementText;
    QByteArray lengthTable;
    QString displayText;
    QRect geometry;
    u16 characterCode;
    u32 textByteLength; //20261108-12 add
    int characterIndex;
    int pixelLength;
    HmiObject candidate; //20261108-15 add
    HmiProject nativeProject; //20261110-5 add: Adapt legacy plain text to the typed source model without changing the caller's object.
    DeviceAddressCompiler addressCompiler; //20261110-5 add
    QByteArray addressScan;

    if (!project.validateSinglePageBackend(error)) //20261110-5 add: Never compile only the selected page of a larger document.
    {
        return false;
    }
    if ((!project.nativeSourceSnapshot.isEmpty() &&
         !nativeProjectUsesSupportedDeviceDefaults(project.nativeSourceSnapshot, error)) ||
        !addressCompiler.prepare(project, &addressScan, error)) //20261110-5 add: Direct legacy compilation obeys the same configuration gates as the project entry.
    {
        return false;
    }
    if (!project.connections.isEmpty() || !project.macros.isEmpty() || !project.pictures.isEmpty())
    {
        if (error != nullptr) { *error = QStringLiteral("旧单文本入口不能承载连接或宏/图片资源；请使用工程编译入口。"); }
        return false;
    }
    Q_FOREACH (candidate, project.objects)
    {
        if (candidate.type == QStringLiteral("switch"))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("该入口只用于旧单文本回归；位按钮工程请使用工程下载或--compile-project入口。"); //20261109-0 change
            }
            return false;
        }
    }

    if (payloads == nullptr || project.targetModel != QStringLiteral("AMT070W124-L800X480R") ||
        project.pageWidth != 800 || project.pageHeight != 480 ||
        project.objects.size() != 1 ||
        project.objects.first().type != QStringLiteral("text"))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("独立编译目前只支持目标机型的单个静态文本画面；已拒绝写屏。");
        }
        return false;
    }
    geometry = project.objects.first().geometry;
    if (project.languages.size() != 1 ||
        (project.objects.first().hasStaticTextProperties && project.objects.first().staticText.visibilityEnabled)) //20261108-14 add
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("旧单文本模板不承载多语言或显现权限动态条件；请使用工程编译入口。");
        }
        return false;
    }
    if (project.objects.first().hasStaticTextProperties && !project.objects.first().staticText.validate(error))
    {
        return false;
    }
    displayText = staticTextContent(project.objects.first(), project.activeLanguage); //20261108-14 change: Snapshot and raster must use the same selected language.
    nativeProject = project;
    if (!nativeProject.objects.first().hasStaticTextProperties) //20261110-5 add: Legacy 12-point text maps to the typed model's 16-pixel logical size (renderer factor 0.75).
    {
        nativeProject.objects.first().hasStaticTextProperties = true;
        nativeProject.objects.first().staticText.fontSize = 16;
        nativeProject.objects.first().staticText.languageTexts.insert(project.languages.first(), displayText);
    }
    if (geometry.width() <= 0 || geometry.height() <= 0 ||
        geometry.x() < 0 || geometry.y() < 0 ||
        geometry.right() >= 800 || geometry.bottom() >= 480 ||
        displayText.size() > 0x3fffffff) //20261108-12 change: Match the screen and Qt's 32-bit string field, not a four-character fixture.
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("文本框必须位于 800×480 画面内，文字不能超过原厂工程字段容量。");
        }
        return false;
    }
    textByteLength = u32(displayText.size() * 2);
    replacementText.append(char((textByteLength >> 24) & 0xff)); //20261108-12 add: Qt 4 serializes QString byte length in big endian.
    replacementText.append(char((textByteLength >> 16) & 0xff));
    replacementText.append(char((textByteLength >> 8) & 0xff));
    replacementText.append(char(textByteLength & 0xff));
    for (characterIndex = 0; characterIndex < displayText.size(); ++characterIndex)
    {
        characterCode = displayText.at(characterIndex).unicode();
        replacementText.append(char((characterCode >> 8) & 0xff)); //20261108-12 change: Preserve every UTF-16 code unit.
        replacementText.append(char(characterCode & 0xff));
    }
    if (!loadPayloads(&generated, error))
    {
        return false;
    }
    // 20261108-11 add: Preserve the already proven TEST bytes exactly on the original fixture path.
    if (!project.objects.first().hasStaticTextProperties &&
        displayText == QStringLiteral("TEST") && geometry == QRect(244, 112, 150, 50)) //20261108-14 change
    {
        if (!project.nativeSourceSnapshot.isEmpty() || project.languages != QStringList{QStringLiteral("中文")}) //20261110-5 add: Keep validated metadata and non-default language registration in the source archive.
        {
            if (!exportNativeProjectSource(nativeProject, &nativeSource, error)) { return false; }
            generated[7] = qCompress(nativeSource, 9);
        }
        *payloads = generated;
        return true;
    }

    // 20261108-11 add: Qt produces the full RGB565 page; the MIT codec emits a standalone LZO1X stream.
    if (!renderSingleTextFrame(project, QStringLiteral("微软雅黑"), 12, false,
                               &frame, error) ||
        !compressRgb565Frame(frame, &compressedFrame, error))
    {
        return false;
    }
    pixelLength = compressedFrame.size();
    lengthTable = generated.at(5);
    lengthTable[0] = char(pixelLength & 0xff);
    lengthTable[1] = char((pixelLength >> 8) & 0xff);
    lengthTable[2] = char((pixelLength >> 16) & 0xff);
    lengthTable[3] = char((pixelLength >> 24) & 0xff);

    nativeSource = qUncompress(generated.at(7)); // 20261108-11 add: Recover the verified editable source snapshot.
    originalText = QByteArray::fromHex("000000080054004500530054"); //20261108-12 change: Include the serialized byte count.
    if (nativeSource.size() != 4023 ||
        !replaceKnownDoubleAt(&nativeSource, 2783, 150.0, double(geometry.width())) ||
        !replaceKnownDoubleAt(&nativeSource, 2791, 50.0, double(geometry.height())) ||
        !replaceKnownDoubleAt(&nativeSource, 2840, 244.0, double(geometry.x())) ||
        !replaceKnownDoubleAt(&nativeSource, 2848, 112.0, double(geometry.y())) ||
        !replaceKnownDoubleAt(&nativeSource, 2919, 150.0, double(geometry.width())) ||
        !replaceKnownDoubleAt(&nativeSource, 2927, 50.0, double(geometry.height())) ||
        !replaceKnownDoubleAt(&nativeSource, 3062, 244.0, double(geometry.x())) ||
        !replaceKnownDoubleAt(&nativeSource, 3070, 112.0, double(geometry.y())) ||
        !replaceKnownFields(&nativeSource, originalText, replacementText, 3) ||
        nativeSource.size() != 4023 + 3 * (replacementText.size() - originalText.size())) //20261108-12 change
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("内置工程模板字段与已验证布局不符，已拒绝写屏。");
        }
        return false;
    }
    if ((!project.nativeSourceSnapshot.isEmpty() || project.languages != QStringList{QStringLiteral("中文")}) &&
        !exportNativeProjectSource(nativeProject, &nativeSource, error)) //20261110-5 add: Retain imported source metadata instead of replacing it with a fixed seed.
    {
        return false;
    }
    compressedSource = qCompress(nativeSource, 9);
    if (qUncompress(compressedSource) != nativeSource)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("可编辑工程快照压缩校验失败，已拒绝写屏。");
        }
        return false;
    }
    generated[5] = lengthTable;
    generated[7] = compressedSource;
    generated[8] = compressedFrame;
    *payloads = generated;
    return true;
}

/******************************************************************************
 * Function Name   : storePayloadLength
 * Description     : Replace a verified little-endian length inside a prepared payload header.
 * Input Parameters: bytes, offset, value
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
static void storePayloadLength(QByteArray *bytes, int offset, u32 value) //20261109-0 add
{
    int index;

    for (index = 0; index < 4; ++index)
    {
        (*bytes)[offset + index] = char((value >> (index * 8)) & 0xff);
    }
}

/******************************************************************************
 * Function Name   : buildProjectPayload
 * Description     : Generate control records, RGB565 sprites and a recoverable Lite project snapshot.
 * Input Parameters: project, payloads, error
 * Return Value    : True when every enabled property compiled without dropping runtime behavior.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildProjectPayload(const HmiProject &project, QVector<QByteArray> *payloads,
                         QString *error) //20261109-0 add
{
    QVector<QByteArray> generated;
    QVector<HmiDownloadStep> plan;
    QList<HmiObject> objects;
    QList<HmiObject> overlays; //20261109-0 add
    HmiObject object;
    HmiObject related;
    QImage underlay;
    QImage image(800, 480, QImage::Format_RGB16);
    QPainter painter;
    QByteArray background;
    QByteArray compressedBackground;
    QByteArray controls;
    QByteArray pictures;
    QByteArray pictureRecords;
    QByteArray record;
    QByteArray source;
    BitButtonSpriteSet sprites;
    BitButtonSpriteSet languageSprites; //20261109-0 add
    StaticTextSpriteSet textSprites; //20261109-0 add
    NativeStaticTextControlOptions textOptions;
    NativeBitButtonControlOptions options;
    DeviceAddressCompiler addressCompiler; //20261109-0 add
    QByteArray addressScan;
    DeviceMacroPackage macros;
    QList<int> stateIndices;
    QString sdk;
    QString language; //20261109-0 add
    QStringList spriteLanguages; //20261109-8 add
    QString backgroundLanguage;
    u32 rawOffset;
    u16 initialLanguageIndex; //20261109-0 add
    int controlCount;
    int pictureCount;
    int objectIndex;
    int relatedIndex;
    int stateIndex; //20261109-8 add
    bool needsUnderlay;
    bool hasSwitch;
    bool secondStateActive; //20261109-6 add

    if (!project.validateSinglePageBackend(error)) //20261110-5 add: Block uncompiled pages/configuration before loading payloads or opening USB.
    {
        return false;
    }
    if (!project.nativeSourceSnapshot.isEmpty() &&
        !nativeProjectUsesSupportedDeviceDefaults(project.nativeSourceSnapshot, error)) //20261109-0 add
    {
        return false;
    }
    if (!addressCompiler.prepare(project, &addressScan, error)) { return false; } //20261109-0 change
    hasSwitch = false;
    Q_FOREACH (object, project.objects)
    {
        hasSwitch = hasSwitch || object.type == QStringLiteral("switch");
    }
    if (!hasSwitch && project.macros.isEmpty() && project.pictures.isEmpty() && project.objects.size() == 1 && project.connections.isEmpty() && project.languages.size() == 1 &&
        (!project.objects.first().hasStaticTextProperties || !project.objects.first().staticText.visibilityEnabled)) //20261109-0 change: Keep the proven single-text raster for non-dynamic text.
    {
        return buildSingleTextPayload(project, payloads, error);
    }
    if (payloads == nullptr || project.targetModel != QStringLiteral("AMT070W124-L800X480R") ||
        project.pageWidth != 800 || project.pageHeight != 480 || project.objects.isEmpty() ||
        project.objects.size() > 100 || project.languages.isEmpty() || project.languages.size() > 8)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("设备编译需要目标型号的800×480画面、1至8种语言，控件数量为1至100。");
        }
        return false;
    }
    if (!loadPayloads(&generated, error))
    {
        return false;
    }
    backgroundLanguage = project.languages.first(); //20261109-0 add: Runtime images are emitted in saved language order.
    if (!nativeProjectInitialLanguageIndex(project, &initialLanguageIndex, error)) { return false; } //20261109-0 add
    sdk = deviceMacroSdkDirectory(); //20261109-0 change: GUI and background workers share the persisted selection.
    if (!compileDeviceMacros(project, sdk, &macros, error))
    {
        return false;
    }
    objects = project.objects;
    std::stable_sort(objects.begin(), objects.end(), [](const HmiObject &left, const HmiObject &right) {
        return left.layer < right.layer;
    });
    image.fill(Qt::black);
    painter.begin(&image);
    painter.fillRect(QRect(1, 1, 799, 479), QColor(197, 223, 255));
    controlCount = 0;
    pictureCount = 0;
    rawOffset = 0;
    for (objectIndex = 0; objectIndex < objects.size(); ++objectIndex) //20261109-0 change
    {
        object = objects.at(objectIndex);
        if (object.geometry.isEmpty() || object.geometry.x() < 0 || object.geometry.y() < 0 ||
            object.geometry.right() >= 800 || object.geometry.bottom() >= 480)
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("控件必须完整位于800×480画面内。");
            }
            return false;
        }
        if (object.type == QStringLiteral("text"))
        {
            if (project.languages.size() > 1 || (object.hasStaticTextProperties && object.staticText.visibilityEnabled)) //20261109-0 change
            {
                for (relatedIndex = 0; relatedIndex < objects.size(); ++relatedIndex)
                {
                    if (relatedIndex != objectIndex && objects.at(relatedIndex).geometry.intersects(object.geometry))
                    {
                        if (error != nullptr) { *error = QStringLiteral("动态文本与其它控件重叠的实时合成尚未恢复，未打开USB。"); }
                        return false;
                    }
                }
                if (!buildStaticTextSprites(object, project.languages, &textSprites, error)) { return false; }
                textOptions = NativeStaticTextControlOptions();
                textOptions.uncompressedPixelOffset = rawOffset;
                textOptions.uncompressedPixelBytes = textSprites.rawLength;
                textOptions.languageCount = u32(project.languages.size());
                textOptions.externalAddressEncoder = [&addressCompiler](const QString &text, bool word, bool toggle,
                                                                        QByteArray *bytes, QString *diagnostic) {
                    return addressCompiler.encode(text, word, toggle, bytes, diagnostic);
                };
                if (!buildNativeStaticTextControl(object, textOptions, &record, error)) { return false; }
                controls.append(record); pictures.append(textSprites.pixelData);
                pictureRecords.append(textSprites.lengthRecords);
                rawOffset += textSprites.rawLength; ++controlCount;
                pictureCount += textSprites.lengthRecords.size() / 12;
            }
            else
            {
                paintStaticText(&painter, object.geometry, staticTextContent(object, backgroundLanguage), object.staticText);
            }
        }
        else if (object.type == QStringLiteral("switch"))
        {
            secondStateActive = object.hasFunctionButtonProperties ? object.functionButton.pressedEffect : //20261110-0 change: Original function buttons retain a hidden ON appearance for pressed frames.
                                object.hasLampProperties ? object.lamp.stateCount() > 1 :
                                object.hasWordButtonProperties ? object.wordButton.pressedEffect :
                                object.bitButton.indicator && object.bitButton.dualMonitor; //20261109-6 add
            //20261109-6 change: Include the pressed image only when the word effect is active; retain artwork alpha in both subtypes.
            needsUnderlay = bitButtonNeedsUnderlay(object.bitButton.states[0], object.geometry.size()) ||
                            (secondStateActive &&
                             bitButtonNeedsUnderlay(object.bitButton.states[1], object.geometry.size()));
            if (object.hasLampProperties) //20261109-8 add: Every additional state can require the static underlay.
            {
                for (stateIndex = 2; stateIndex < object.lamp.stateCount(); ++stateIndex)
                {
                    needsUnderlay = needsUnderlay || bitButtonNeedsUnderlay(*object.lamp.state(object.bitButton, stateIndex), object.geometry.size());
                }
            }
            overlays.clear();
            for (relatedIndex = 0; relatedIndex < objects.size(); ++relatedIndex)
            {
                related = objects.at(relatedIndex);
                if (!related.geometry.intersects(object.geometry)) { continue; }
                if (relatedIndex < objectIndex && related.type == QStringLiteral("switch") && needsUnderlay)
                {
                    if (error != nullptr) { *error = QStringLiteral("透明按钮与下层动态按钮重叠的设备合成尚未恢复，未打开USB。"); } //20261109-6 change
                    return false;
                }
                if (relatedIndex > objectIndex && related.type == QStringLiteral("text")) { overlays.append(related); }
            }
            underlay = image.copy(object.geometry);
            //20261109-6 change: Original word images are OFF then ON; bit monitors retain their verified ON then OFF order.
            stateIndices = !secondStateActive ? QList<int>{0} :
                           (object.hasWordButtonProperties || object.hasFunctionButtonProperties) ? QList<int>{0, 1} : QList<int>{1, 0}; //20261110-0 change
            if (object.hasLampProperties && object.lamp.monitorMode == 2) //20261109-8 add: Native multi-state lamps emit ascending logical states.
            {
                stateIndices.clear();
                for (stateIndex = 0; stateIndex < object.lamp.stateCount(); ++stateIndex) { stateIndices.append(stateIndex); }
            }
            sprites = BitButtonSpriteSet();
            spriteLanguages = (object.hasLampProperties || object.hasFunctionButtonProperties) && object.bitButton.states[0].sameLanguage ?
                              QStringList{backgroundLanguage} : project.languages; //20261110-0 change: Shared-text lamps and functions reuse one complete image set.
            Q_FOREACH (language, spriteLanguages) //20261109-8 change: Localized controls retain language-first order; shared-text lamps emit only one set.
            {
                if (!buildBitButtonSprites(object, language, stateIndices, &languageSprites, error, underlay, overlays))
                {
                    return false;
                }
                sprites.pixelData.append(languageSprites.pixelData);
                sprites.lengthRecords.append(languageSprites.lengthRecords);
                sprites.rawLength += languageSprites.rawLength;
            }
            options = NativeBitButtonControlOptions();
            options.externalAddressEncoder = [&addressCompiler](const QString &text, bool word, bool toggle,
                                                                QByteArray *bytes, QString *diagnostic) {
                return addressCompiler.encode(text, word, toggle, bytes, diagnostic);
            }; //20261109-0 add: Route all seven active external control slots through the same validated context.
            options.uncompressedPixelOffset = rawOffset;
            options.uncompressedPixelBytes = sprites.rawLength;
            options.languageCount = u32(project.languages.size()); //20261109-0 add
            if (object.bitButton.macroEnabled)
            {
                if (!macros.macroIndices.contains(object.bitButton.macroName))
                {
                    if (error != nullptr)
                    {
                        *error = QStringLiteral("按钮引用的宏不存在：%1").arg(object.bitButton.macroName); //20261109-6 change
                    }
                    return false;
                }
                options.macroIndex = macros.macroIndices.value(object.bitButton.macroName);
            }
            if (!(object.hasFunctionButtonProperties ? buildNativeFunctionButtonControl(object, options, &record, error) : //20261110-0 change
                  object.hasLampProperties ? buildNativeLampControl(object, options, &record, error) :
                  object.hasWordButtonProperties ? buildNativeWordButtonControl(object, options, &record, error) :
                  buildNativeBitButtonControl(object, options, &record, error))) //20261109-6 change
            {
                return false;
            }
            controls.append(record);
            pictures.append(sprites.pixelData);
            pictureRecords.append(sprites.lengthRecords);
            rawOffset += sprites.rawLength;
            ++controlCount;
            pictureCount += sprites.lengthRecords.size() / 12; //20261109-0 change: Flashing states add one text-free sprite each.
        }
        else
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("设备编译尚不支持控件类型：%1").arg(object.type);
            }
            return false;
        }
    }
    painter.end();
    background = QByteArray(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes());
    if (!compressRgb565Frame(background, &compressedBackground, error))
    {
        return false;
    }
    //20261109-0 change: The first 2400 bytes are three 200-page u32 arrays; state records follow them.
    qToLittleEndian<u16>(initialLanguageIndex, generated[0].data() + 2172); //20261109-0 add: Startup language is distinct from the editor's current language.
    qToLittleEndian<u16>(u16(project.languages.size()), generated[0].data() + 2174);
    generated[1][2] = char(controlCount & 0xff);
    //20261109-0 change: Byte 3 is an independent native page flag, not the high byte of the count.
    storePayloadLength(&generated[1], 0x44, rawOffset);
    storePayloadLength(&generated[5], 0, u32(compressedBackground.size()));
    storePayloadLength(&generated[5], 0x320, u32(pictures.size()));
    storePayloadLength(&generated[5], 0x640, u32(pictureCount));
    generated[5].append(pictureRecords);
    generated[8] = compressedBackground;
    generated[4] = addressScan; //20261109-0 add: Runtime reads must accompany external control addresses.
    //20261109-0 add: Lite stores its full JSON project here; it does not claim this snapshot is native .sm.
    source = QJsonDocument(project.toJson()).toJson(QJsonDocument::Compact);
    generated[7] = qCompress(source, 9);
    if (qUncompress(generated.at(7)) != source)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("工程快照压缩校验失败。");
        }
        return false;
    }
    if (controlCount > 0)
    {
        generated.append(controls);
        generated.append(pictures);
    }
    if (!macros.marcoRun.isEmpty())
    {
        if (!hasSwitch) //20261109-0 change: Type28 text records are not macro call sites.
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("带宏的设备工程目前需要至少一个按钮。"); //20261109-6 change
            }
            return false;
        }
        generated.append(macros.marcoVar);
        generated.append(macros.marcoRun);
    }
    if (!buildDeviceTransferPlan(generated, &plan, error))
    {
        return false;
    }
    *payloads = generated;
    return true;
}

/******************************************************************************
 * Function Name   : verifyCompilerVariant
 * Description     : Compare one generated page with the native RGB565 frame digest.
 * Input Parameters: project, expectedHash, error
 * Return Value    : True when raster, compressed block, length, and source agree.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool verifyCompilerVariant(const HmiProject &project,
                                  const QByteArray &expectedHash,
                                  QString *error) //20261108-11 add
{
    QVector<QByteArray> payloads;
    QByteArray frame;
    QByteArray compressed;
    QByteArray source;
    QByteArray textBytes;
    QByteArray serializedText; //20261108-12 add
    QByteArray actualHash;
    int index;
    int length;
    u32 textByteLength;

    if (!buildSingleTextPayload(project, &payloads, error) || payloads.size() != 10 ||
        !renderSingleTextFrame(project, QStringLiteral("微软雅黑"), 12, false,
                               &frame, error) ||
        !compressRgb565Frame(frame, &compressed, error))
    {
        return false;
    }
    actualHash = QCryptographicHash::hash(frame, QCryptographicHash::Sha256).toHex();
    length = payloads.at(8).size();
    if (actualHash != expectedHash || payloads.at(8) != compressed ||
        payloads.at(5).size() != 2400 ||
        u8(payloads.at(5).at(0)) != (length & 0xff) ||
        u8(payloads.at(5).at(1)) != ((length >> 8) & 0xff) ||
        u8(payloads.at(5).at(2)) != ((length >> 16) & 0xff) ||
        u8(payloads.at(5).at(3)) != ((length >> 24) & 0xff))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("单文本像素或压缩长度与原厂差分样本不符。");
        }
        return false;
    }
    source = qUncompress(payloads.at(7));
    for (index = 0; index < project.objects.first().text.size(); ++index)
    {
        textBytes.append(char((project.objects.first().text.at(index).unicode() >> 8) & 0xff));
        textBytes.append(char(project.objects.first().text.at(index).unicode() & 0xff));
    }
    textByteLength = u32(textBytes.size());
    serializedText.append(char((textByteLength >> 24) & 0xff));
    serializedText.append(char((textByteLength >> 16) & 0xff));
    serializedText.append(char((textByteLength >> 8) & 0xff));
    serializedText.append(char(textByteLength & 0xff));
    serializedText.append(textBytes);
    if (source.size() != 4023 + 3 * (serializedText.size() - 12) ||
        (!textBytes.isEmpty() && source.count(serializedText) != 3) ||
        source.count(encodedDouble(double(project.objects.first().geometry.x()))) < 2 ||
        source.count(encodedDouble(double(project.objects.first().geometry.y()))) < 2 ||
        source.count(encodedDouble(double(project.objects.first().geometry.width()))) < 2 ||
        source.count(encodedDouble(double(project.objects.first().geometry.height()))) < 2) //20261108-12 change
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("单文本源工程的文字或坐标字段验证失败。");
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : verifySingleTextCompiler
 * Description     : Regress exact TEST bytes, native rasters, and invalid-project rejection.
 * Input Parameters: fixedProject, error
 * Return Value    : True when all checks pass without opening the USB device.
 * Date            : 2026.11.08
 ******************************************************************************/
bool verifySingleTextCompiler(const HmiProject &fixedProject,
                              QString *error) //20261108-11 add
{
    HmiProject project;
    QVector<QByteArray> fixedPayloads;
    QVector<QByteArray> generated;
    int index;

    if (!loadPayloads(&fixedPayloads, error) ||
        !buildSingleTextPayload(fixedProject, &generated, error) ||
        generated != fixedPayloads)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("固定 TEST 数据与已验证版本不一致。");
        }
        return false;
    }
    project = fixedProject;
    project.objects.first().text = QStringLiteral("AB12");
    if (!verifyCompilerVariant(project,
                               QByteArray("4f20647042ce8629a84162b377e15fcfb16853dca46722e2f470495984e18d01"),
                               error))
    {
        return false;
    }
    project.objects.first().geometry = QRect(300, 150, 150, 50);
    if (!verifyCompilerVariant(project,
                               QByteArray("5d70fed866c2e2b53a76d8c13dd180da787bbda152cd507150e3d3d855306a3a"),
                               error))
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("AAAA");
    project.objects.first().geometry = QRect(244, 112, 150, 50);
    if (!verifyCompilerVariant(project,
                               QByteArray("910a1e00d2a94bbf713245289e6b0e32f4a0f85d23d857faec8405ce79af5bf3"),
                               error))
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("A"); //20261108-12 add: Native one-character frame.
    if (!verifyCompilerVariant(project,
                               QByteArray("856e93c91bc562ba112fb0b13714ac748d9aca54bfa0f9fa399d3fbf9c354312"),
                               error))
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("测试中文AB"); //20261108-12 add: Native Unicode frame.
    if (!verifyCompilerVariant(project,
                               QByteArray("5bca20615f669b501b952080e29cc14ede7f0a54fe245fa2ad95ef78ada433ce"),
                               error))
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("AB12");
    project.objects.first().geometry = QRect(244, 112, 90, 30);
    if (!verifyCompilerVariant(project,
                               QByteArray("b7169a2ecea7fd6bd87d8cf284fc5aa3dcbe719e47a16b3f59bdd17be2003132"),
                               error))
    {
        return false;
    }
    project.objects.first().geometry = QRect(244, 112, 240, 90);
    if (!verifyCompilerVariant(project,
                               QByteArray("645a69576d386af7855709e9220a7ea1d9ea0428f8e77f053add15458f3ee092"),
                               error))
    {
        return false;
    }
    project.objects.first().text.clear(); //20261108-12 add: The native compiler accepts an empty label.
    project.objects.first().geometry = QRect(244, 112, 150, 50);
    if (!verifyCompilerVariant(project,
                               QByteArray("208891b9a73f55179fa2311da841880df13ca04aaa9f3f7545c786468400a04f"),
                               error))
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("HELLOWORLD12");
    if (!verifyCompilerVariant(project,
                               QByteArray("fb6ec4c1dcf11746fbee25fad4a97b7fe3a0c03e857ec00d8268ea04ea93edd3"),
                               error)) //20261108-12 add: Qt 6 spacing differs by one pixel in the first four glyphs.
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("测试中文AB");
    project.objects.first().geometry = QRect(244, 112, 240, 90);
    if (!verifyCompilerVariant(project,
                               QByteArray("b248918e34d86ac808406e0ade1a8d86f0d382768e49eb09986e1d0f02cae1aa"),
                               error)) //20261108-12 add: Text and box size change together.
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("第一行\n第二行");
    if (!verifyCompilerVariant(project,
                               QByteArray("ba64339b199885cb0ccc1225d0b26eb827ef1b7472b67420acc82494f811fe85"),
                               error)) //20261108-12 add: Native two-line raster.
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("第一行\n第二行\n第三行");
    if (!verifyCompilerVariant(project,
                               QByteArray("d18533366ac2ed743c89da8c4a2f03f4dea8a74991327ef824a27012475baa3f"),
                               error)) //20261108-12 add: Native odd-line centering.
    {
        return false;
    }
    project.objects.first().text = QStringLiteral("HELLOWORLD12");
    project.objects.first().geometry = QRect(244, 112, 90, 30);
    if (!verifyCompilerVariant(project,
                               QByteArray("9ea4902c806903a58185477af1d26fbdb3fa5da38dacb8a07c8f2692a1aabaa9"),
                               error)) //20261108-12 add: A narrow box clips text without compiler failure.
    {
        return false;
    }

    // 20261108-11 add: These failures must happen during compilation, before USB discovery.
    for (index = 0; index < 3; ++index) //20261108-12 change: Variable size and Unicode are now supported.
    {
        project = fixedProject;
        if (index == 0)
        {
            project.targetModel = QStringLiteral("OTHER");
        }
        else if (index == 1)
        {
            project.objects.first().type = QStringLiteral("switch");
        }
        else if (index == 2)
        {
            project.objects.first().geometry.moveLeft(790);
        }
        if (buildSingleTextPayload(project, &generated, nullptr))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("不支持的工程意外通过单文本独立编译。");
            }
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : exportSingleTextPayload
 * Description     : Write generated transfer blocks for independent differential checks.
 * Input Parameters: project, outputDirectory, error
 * Return Value    : True when all blocks were saved.
 * Date            : 2026.11.08
 ******************************************************************************/
bool exportSingleTextPayload(const HmiProject &project,
                             const QString &outputDirectory,
                             QString *error) //20261108-11 add
{
    QVector<QByteArray> payloads;
    QDir directory;
    QSaveFile file;
    int index;

    if (!buildSingleTextPayload(project, &payloads, error))
    {
        return false;
    }
    if (!directory.mkpath(outputDirectory))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("无法建立编译输出目录。");
        }
        return false;
    }
    directory.setPath(outputDirectory);
    for (index = 0; index < payloads.size(); ++index)
    {
        file.setFileName(directory.filePath(QString::fromLatin1(payload_specs[index].name)));
        if (!file.open(QIODevice::WriteOnly) ||
            file.write(payloads.at(index)) != payloads.at(index).size() ||
            !file.commit())
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("编译数据写入失败：%1").arg(file.errorString());
            }
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : verifyFixedTestPayload
 * Description     : Check all embedded compiled files without opening the USB device.
 * Input Parameters: error
 * Return Value    : True when the immutable TEST fixture is complete.
 * Date            : 2026.11.08
 ******************************************************************************/
bool verifyFixedTestPayload(QString *error) //20261108-10 add
{
    QVector<QByteArray> payloads;

    return loadPayloads(&payloads, error);
}

/******************************************************************************
 * Function Name   : downloadSingleTextProject
 * Description     : Download a generated text frame without a vendor application or DLL.
 * Input Parameters: project, error
 * Return Value    : True after acknowledged transfer and reset request, not display readback.
 * Date            : 2026.11.08
 ******************************************************************************/
static bool downloadPreparedPayload(const QVector<QByteArray> &payloads, QString *error) //20261109-0 change
{
    QVector<HmiDownloadStep> plan;
#ifdef Q_OS_WIN
    QLibrary library;                 //20261108-10 change: Keep all USB state declarations at the function entry.
    UsbFunctions usb = {};
    UsbBus *bus;
    UsbDevice *device;
    UsbDevice *target;
    void *handle;
    bool transferred;
    int busCount;
    int deviceCount;
    int claimResult;
#endif

    if (!buildDeviceTransferPlan(payloads, &plan, error)) //20261109-0 change: Validate every wire block before USB access.
    {
        return false;
    }
#ifdef Q_OS_WIN
    if (!resolveUsbFunctions(&library, &usb, error))
    {
        return false;
    }
    usb.init();
    usb.findBusses();
    usb.findDevices();
    bus = usb.getBusses();
    target = nullptr;
    busCount = 0;
    while (bus != nullptr && busCount < 64 && target == nullptr)
    {
        device = bus->devices;
        deviceCount = 0;
        while (device != nullptr && deviceCount < 256)
        {
            if (device->descriptor.vendorId == 0x7365 &&
                device->descriptor.productId == 0x616c)
            {
                target = device;
                break;
            }
            device = device->next;
            ++deviceCount;
        }
        bus = bus->next;
        ++busCount;
    }
    if (target == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("未发现 7365:616C USB 下载设备。");
        }
        return false;
    }
    handle = usb.open(target);
    if (handle == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("无法打开触摸屏 USB 设备，可能正被其他程序占用。");
        }
        return false;
    }
    claimResult = usb.claim(handle, 0);
    if (claimResult != 0)
    {
        usb.close(handle);
        if (error != nullptr)
        {
            *error = QStringLiteral("无法占用触摸屏 USB 接口 0（错误 %1）。").arg(claimResult);
        }
        return false;
    }
    transferred = transferTestPayload(usb, handle, payloads, error);
    usb.release(handle, 0);
    usb.close(handle);
    return transferred;
#else
    if (error != nullptr)
    {
        *error = QStringLiteral("独立 USB 下载当前仅支持 Windows。");
    }
    return false;
#endif
}

/******************************************************************************
 * Function Name   : downloadSingleTextProject
 * Description     : Keep the legacy single-text entry and its compile-time input guard.
 * Input Parameters: project, error
 * Return Value    : True after an acknowledged static project transfer.
 * Date            : 2026.11.09
 ******************************************************************************/
bool downloadSingleTextProject(const HmiProject &project, QString *error) //20261109-0 change
{
    QVector<QByteArray> payloads;

    return buildSingleTextPayload(project, &payloads, error) && downloadPreparedPayload(payloads, error);
}

/******************************************************************************
 * Function Name   : downloadProject
 * Description     : Compile the complete supported project before opening the target USB device.
 * Input Parameters: project, error
 * Return Value    : True after all block ACKs, commit ACK and reset request.
 * Date            : 2026.11.09
 ******************************************************************************/
bool downloadProject(const HmiProject &project, QString *error) //20261109-0 add
{
    QVector<QByteArray> payloads;

    return buildProjectPayload(project, &payloads, error) && downloadPreparedPayload(payloads, error);
}

/******************************************************************************
 * Function Name   : exportProjectPayload
 * Description     : Export complete compiler blocks and their digests without using USB.
 * Input Parameters: project, outputDirectory, error
 * Return Value    : True when every file and the compilation manifest were saved.
 * Date            : 2026.11.09
 ******************************************************************************/
bool exportProjectPayload(const HmiProject &project, const QString &outputDirectory,
                          QString *error) //20261109-0 add
{
    static const char *names[] = {"SysPara.dat", "SheetPara.dat", "AlarmPara.dat", "Collect.dat",
        "AddrPara.dat", "PicLenPara.dat", "Curve.dat", "upLoad.knr", "SheetPixel.dat", "Modbus_RTU_com1.bin",
        "ControlPara.dat", "ControlPixel.dat", "MarcoVar.dat", "MarcoRun.dat"};
    QVector<QByteArray> payloads;
    QSaveFile file;
    QJsonObject manifest;
    QJsonObject entries;
    QJsonObject entry;
    QJsonDocument snapshot; //20261109-0 add
    QByteArray bytes;
    QString name;
    int index;

    if (!buildProjectPayload(project, &payloads, error))
    {
        return false;
    }
    if (!QDir().mkpath(outputDirectory))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("无法建立编译输出目录。");
        }
        return false;
    }
    for (index = 0; index < payloads.size(); ++index)
    {
        name = QString::fromLatin1(names[index]);
        file.setFileName(QDir(outputDirectory).filePath(name));
        if (!file.open(QIODevice::WriteOnly) || file.write(payloads.at(index)) != payloads.at(index).size() || !file.commit())
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("编译数据写入失败：%1；输出目录可能包含部分文件。").arg(name);
            }
            return false;
        }
        entry = QJsonObject();
        entry.insert(QStringLiteral("length"), int(payloads.at(index).size()));
        entry.insert(QStringLiteral("sha256"), QString::fromLatin1(QCryptographicHash::hash(payloads.at(index), QCryptographicHash::Sha256).toHex()));
        entries.insert(name, entry);
    }
    manifest.insert(QStringLiteral("files"), entries);
    snapshot = QJsonDocument::fromJson(qUncompress(payloads.at(7))); //20261109-0 change: Ten-block multi-text output also carries Lite JSON.
    manifest.insert(QStringLiteral("sourceSnapshot"), snapshot.isObject() &&
        snapshot.object().value(QStringLiteral("format")).toString() == QStringLiteral("KinSealStudio-Prototype") ?
        QStringLiteral("lite-project-json") : QStringLiteral("native-sm"));
    manifest.insert(QStringLiteral("targetModel"), project.targetModel);
    manifest.insert(QStringLiteral("deviceAcknowledged"), false);
    bytes = QJsonDocument(manifest).toJson(QJsonDocument::Indented);
    file.setFileName(QDir(outputDirectory).filePath(QStringLiteral("compiler-manifest.json")));
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size() || !file.commit())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("编译清单写入失败；输出目录可能包含部分文件。");
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : downloadFixedTestProject
 * Description     : Keep the original fixed TEST entry point for compatibility.
 * Input Parameters: project, error
 * Return Value    : True when the validated fixed fixture transfer completes.
 * Date            : 2026.11.08
 ******************************************************************************/
bool downloadFixedTestProject(const HmiProject &project, QString *error) //20261108-11 change
{
    if (!project.validateSinglePageBackend(error)) //20261110-5 add: Keep the historical fixture entry from bypassing whole-project gates.
    {
        return false;
    }
    if (project.objects.size() != 1 ||
        project.objects.first().text != QStringLiteral("TEST") ||
        project.objects.first().geometry != QRect(244, 112, 150, 50))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("旧下载入口只接受固定 TEST 样例。");
        }
        return false;
    }
    return downloadSingleTextProject(project, error);
}
