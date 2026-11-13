#include <QCoreApplication>
#include <QFile>
#include <QImage>
#include <QPainter>
#include <QPainterPath>

/******************************************************************************
 * Function Name   : main
 * Description     : Export isolated Qt6 coverage masks for offline raster diagnosis.
 * Input Parameters: output, family, layer, mode
 * Return Value    : Process status.
 * Date            : 2026.11.13
 ******************************************************************************/
int main(int argc, char **argv) //20261113-16 add
{
    QCoreApplication app(argc, argv);
    QImage image(800, 480, QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    QPainterPath path;
    QRectF geometry(100, 100, 200, 100);
    QString family;
    QString layer;
    QString mode;
    QFile output;

    if (argc != 5) { return 2; }
    family = QString::fromLocal8Bit(argv[2]);
    layer = QString::fromLocal8Bit(argv[3]);
    mode = QString::fromLocal8Bit(argv[4]);
    if (family == QStringLiteral("ellipse")) { path.addEllipse(geometry); }
    else if (family == QStringLiteral("roundrect")) { path.addRoundedRect(geometry, 10, 10); }
    else if (family == QStringLiteral("polygon"))
    {
        path.moveTo(100, 100); path.lineTo(300, 100); path.lineTo(250, 220); path.closeSubpath();
    }
    else if (family == QStringLiteral("sector"))
    {
        geometry.setHeight(200); path.moveTo(geometry.center()); path.arcTo(geometry, 45, 90);
    }
    else { return 3; }
    image.fill(Qt::transparent);
    painter.begin(&image);
    painter.setRenderHint(QPainter::Antialiasing, mode != QStringLiteral("alias"));
    painter.setPen(layer == QStringLiteral("fill") ? QPen(Qt::NoPen) : QPen(Qt::white, 1));
    painter.setBrush(layer == QStringLiteral("stroke") ? QBrush(Qt::NoBrush) : QBrush(Qt::white));
    painter.drawPath(path);
    painter.end();
    output.setFileName(QString::fromLocal8Bit(argv[1]));
    if (!output.open(QIODevice::WriteOnly) || output.write(reinterpret_cast<const char *>(image.constBits()), image.sizeInBytes()) != image.sizeInBytes()) { return 4; }
    return 0;
}
