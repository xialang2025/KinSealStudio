#include "text_payload_compiler.h"
#include "static_text_renderer.h" //20261108-14 add

#include <QColor>
#include <QFont>
#include <QImage>
#include <QPainter>

#include "lzokay.hpp" //20261108-11 add: MIT-licensed LZO1X codec, pinned in third_party/lzokay.

#include <cstring>

/******************************************************************************
 * Function Name   : renderSingleTextFrame
 * Description     : Render the known one-text page in the verified RGB565 layout.
 * Input Parameters: project, pointSize, frame, error
 * Return Value    : True when the complete raster was copied to frame.
 * Date            : 2026.11.08
 ******************************************************************************/
bool renderSingleTextFrame(const HmiProject &project, const QString &fontFamily,
                           int fontSize, bool pixelSize,
                           QByteArray *frame, QString *error) //20261108-11 add
{
    QImage image(800, 480, QImage::Format_RGB16);
    QPainter painter;
    QFont font;
    int rasterOffset; //20261108-12 add

    if (!project.validateSinglePageBackend(error)) //20261110-5 add: A raster request must not hide other pages or configuration.
    {
        return false;
    }
    if (frame == nullptr || project.targetModel != QStringLiteral("AMT070W124-L800X480R") ||
        project.pageWidth != 800 || project.pageHeight != 480 ||
        project.objects.size() != 1 ||
        project.objects.first().type != QStringLiteral("text") ||
        fontSize < 6 || fontSize > 30)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("只支持目标型号的单个静态文本画面。");
        }
        return false;
    }

    // 20261108-11 add: The captured frame leaves the top row and left column black.
    image.fill(Qt::black);
    painter.begin(&image);
    painter.fillRect(QRect(1, 1, 799, 479), QColor(197, 223, 255));
    font = QFont(fontFamily);
    if (pixelSize)
    {
        font.setPixelSize(fontSize);
    }
    else
    {
        font.setPointSize(fontSize);
    }
    font.setStyleStrategy(QFont::NoAntialias);
    painter.setFont(font);
    painter.setPen(Qt::black);
    painter.setRenderHint(QPainter::TextAntialiasing, false);
    // 20261108-12 change: Qt 4 and Qt 6 round centered single and multiple lines differently.
    rasterOffset = project.objects.first().text.count(QLatin1Char('\n')) % 2 == 0 ? -1 : 0; //20261108-12 change: Match odd/even line-centering parity.
    if (project.objects.first().hasStaticTextProperties) //20261108-14 add: Reuse exactly the editor appearance for new property-aware objects.
    {
        paintStaticText(&painter, project.objects.first().geometry,
                         staticTextContent(project.objects.first(), project.activeLanguage),
                         project.objects.first().staticText);
    }
    else
    {
        painter.drawText(project.objects.first().geometry.translated(0, rasterOffset), Qt::AlignCenter,
                         project.objects.first().text);
    }
    painter.end();
    if (image.bytesPerLine() != 1600 || image.sizeInBytes() != 768000)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("RGB565 画面布局异常。");
        }
        return false;
    }
    *frame = QByteArray(reinterpret_cast<const char *>(image.constBits()),
                        image.sizeInBytes());
    return true;
}

/******************************************************************************
 * Function Name   : compressRgb565Pixels
 * Description     : Compress a packed RGB565 image and verify that it decodes byte-for-byte.
 * Input Parameters: frame, compressed, error
 * Return Value    : True for one valid LZO1X RGB565 payload.
 * Date            : 2026.11.09
 ******************************************************************************/
bool compressRgb565Pixels(const QByteArray &frame, QByteArray *compressed,
                          QString *error) //20261109-0 change: Support packed control sprites as well as a full page.
{
    QByteArray output;
    QByteArray decoded;
    std::size_t outputLength;
    std::size_t decodedLength;
    lzokay::EResult result;

    if (compressed == nullptr || frame.isEmpty() || frame.size() > 768000 || (frame.size() % 2) != 0)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("RGB565 图片必须为有效的像素对，且不能大于 800×480 画面。");
        }
        return false;
    }
    output.resize(qsizetype(lzokay::compress_worst_size(std::size_t(frame.size()))));
    outputLength = 0;
    result = lzokay::compress(reinterpret_cast<const unsigned char *>(frame.constData()),
                              std::size_t(frame.size()),
                              reinterpret_cast<unsigned char *>(output.data()),
                              std::size_t(output.size()), outputLength);
    if (result != lzokay::EResult::Success)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("RGB565 画面 LZO 压缩失败。");
        }
        return false;
    }
    output.resize(qsizetype(outputLength));
    decoded.resize(frame.size());
    decodedLength = 0;
    result = lzokay::decompress(reinterpret_cast<const unsigned char *>(output.constData()),
                                std::size_t(output.size()),
                                reinterpret_cast<unsigned char *>(decoded.data()),
                                std::size_t(decoded.size()), decodedLength);
    if (result != lzokay::EResult::Success || decodedLength != std::size_t(frame.size()) ||
        std::memcmp(decoded.constData(), frame.constData(), std::size_t(frame.size())) != 0)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("RGB565 画面 LZO 往返校验失败。");
        }
        return false;
    }
    *compressed = output;
    return true;
}

/******************************************************************************
 * Function Name   : compressRgb565Frame
 * Description     : Preserve the original full-screen size guard for static text callers.
 * Input Parameters: frame, compressed, error
 * Return Value    : True for a verified 800 by 480 RGB565 frame.
 * Date            : 2026.11.09
 ******************************************************************************/
bool compressRgb565Frame(const QByteArray &frame, QByteArray *compressed,
                         QString *error) //20261109-0 change
{
    if (frame.size() != 768000)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("RGB565 画面长度必须为 768000 字节。");
        }
        return false;
    }
    return compressRgb565Pixels(frame, compressed, error);
}
