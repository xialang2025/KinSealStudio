#include "bit_button_renderer.h"

#include <QCryptographicHash>
#include <QFontInfo>
#include <QGuiApplication>
#include <QImage>
#include <QPainter>
#include <QtTest>

//20261109-0 add: Native black-caption masks were captured from original compiled RGB565 sprites.
class BitButtonRendererTests : public QObject
{
    Q_OBJECT
private slots:
    void nativeCaptionMasks_data();
    void nativeCaptionMasks();
};

/******************************************************************************
 * Function Name   : BitButtonRendererTests::nativeCaptionMasks_data
 * Description     : Preserve four independent original Chinese and English glyph masks.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void BitButtonRendererTests::nativeCaptionMasks_data() //20261109-0 add
{
    QTest::addColumn<QString>("content");
    QTest::addColumn<QByteArray>("sha256");
    QTest::newRow("on-chinese") << QStringLiteral("开启3") << QByteArrayLiteral("59db6486035072059b6312ab4a8a69275d86252b987500399c6300b526a4af55");
    QTest::newRow("off-chinese") << QStringLiteral("关闭3") << QByteArrayLiteral("f2e94e99a303efeae1a3051e79ec8c726a73fac9a9eddea4382a953b32b180ce");
    QTest::newRow("on-english") << QStringLiteral("RUN3") << QByteArrayLiteral("b269d1236bc8e66585d1203ea5e9a70a581a8eb8dde767b1b3e60353f566e7e4");
    QTest::newRow("off-english") << QStringLiteral("STOP3") << QByteArrayLiteral("04e094f98a9b6b8216fa7df67dc6fe78bf1c9c78d5ab3fcf3bda00970a6d5f76");
}

/******************************************************************************
 * Function Name   : BitButtonRendererTests::nativeCaptionMasks
 * Description     : Verify the actual button renderer against original caption pixels without SVG noise.
 * Input Parameters: Qt data row
 * Return Value    : None
 * Date            : 2026.11.09
 ******************************************************************************/
void BitButtonRendererTests::nativeCaptionMasks() //20261109-0 add
{
    QFETCH(QString, content);
    QFETCH(QByteArray, sha256);
    BitButtonState state;
    QImage image;
    QPainter painter;
    QByteArray mask;
    int x;
    int y;

    state.useImage = false;
    state.sameLanguage = true;
    state.sharedText = content;
    state.text.fontFamily = QStringLiteral("宋体");
    state.text.fontSize = 16;
    QVERIFY2(!QFontInfo(QFont(state.text.fontFamily)).family().isEmpty(), "Native caption goldens require the Windows SimSun font engine.");
    image = QImage(120, 60, QImage::Format_RGB16);
    image.fill(Qt::white);
    painter.begin(&image);
    paintBitButton(&painter, image.rect(), state, QStringLiteral("中文"));
    painter.end();
    for (y = 18; y <= 43; ++y)
    {
        for (x = 30; x <= 90; ++x)
        {
            mask.append(image.pixel(x, y) == qRgb(0, 0, 0) ? char(1) : char(0));
        }
    }
    QCOMPARE(QCryptographicHash::hash(mask, QCryptographicHash::Sha256).toHex(), sha256);
}

/******************************************************************************
 * Function Name   : main
 * Description     : Initialize Windows font rasterization without creating or showing any window.
 * Input Parameters: argc, argv
 * Return Value    : Qt test result code.
 * Date            : 2026.11.09
 ******************************************************************************/
int main(int argc, char **argv) //20261109-0 add
{
    QGuiApplication *application;
    BitButtonRendererTests tests;
    int result;

    // The offscreen QPA plugin has a different font backend; this test creates no QWidget or QWindow.
    qputenv("QT_QPA_PLATFORM", QByteArrayLiteral("windows"));
    application = new QGuiApplication(argc, argv);
    result = QTest::qExec(&tests, argc, argv);
    delete application;
    return result;
}

#include "bit_button_renderer_tests.moc"
