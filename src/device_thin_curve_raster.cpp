#include "device_thin_curve_raster.h"
#include "graphic_properties.h"
#include <QImage>
#include <QPainter>
#include <algorithm>
#include <array>
#include <cmath>
#include <limits>

namespace {

//20261128-11 add: Curves without handles contain individually composited flat-ended segments.
constexpr qint64 unit = 65536;
struct Corner { long double x; long double y; };
struct Edge {
    qint64 firstY;
    qint64 lastY;
    int firstRow;
    qint64 firstX;
    qint64 step;
    qint64 inverse;
};
struct RowEdge { qint64 top; qint64 bottom; qint64 height; qint64 inverse; };

/******************************************************************************
 * Function Name   : makeEdge
 * Description     : Start a 16.16 slope accumulator at the scanline above the first corner.
 * Input Parameters: start and end corners, horizontal step per row and inverse step
 * Return Value    : Fixed-point scan edge.
 * Date            : 2026.11.28
 ******************************************************************************/
Edge makeEdge(Corner start, Corner end, double slope, double inverse) //20261128-11 add
{
    int row = int(std::floor(start.y));
    return {qint64(start.y * unit), qint64(end.y * unit), row,
            qint64((start.x + (row - start.y) * slope) * unit),
            qint64(slope * unit), qint64(inverse * unit)};
}

/******************************************************************************
 * Function Name   : intersectRow
 * Description     : Clip one edge to a pixel row and calculate its two horizontal intersections.
 * Input Parameters: edge and absolute row
 * Return Value    : Intersections in 16.16 and the vertical span within this row.
 * Date            : 2026.11.28
 ******************************************************************************/
RowEdge intersectRow(const Edge &edge, int y) //20261128-11 add
{
    qint64 rowY = qint64(y) * unit;
    qint64 top = std::max(qint64(0), edge.firstY - rowY);
    qint64 bottom = std::min(unit, edge.lastY - rowY);
    qint64 x = edge.firstX + (y - edge.firstRow) * edge.step;

    if (bottom <= top) { return {}; }
    return {x + ((top * edge.step) >> 16), x + ((bottom * edge.step) >> 16),
            bottom - top, edge.inverse};
}

/******************************************************************************
 * Function Name   : leftArea
 * Description     : Integrate the portion of this pixel to the left of a directed trapezoid edge.
 * Input Parameters: clipped edge and absolute column
 * Return Value    : Covered area on a grid where one pixel has area 65536.
 * Date            : 2026.11.28
 ******************************************************************************/
qint64 leftArea(const RowEdge &edge, int x) //20261128-11 add
{
    qint64 top = edge.top - qint64(x) * unit;
    qint64 bottom = edge.bottom - qint64(x) * unit;
    qint64 low = std::min(top, bottom);
    qint64 high = std::max(top, bottom);
    qint64 yAtLeft;
    qint64 yAtRight;

    if (edge.height == 0 || high <= 0) { return 0; }
    if (low >= unit) { return edge.height; }
    if (low >= 0 && high <= unit) { return (edge.height * (low + ((high - low) >> 1))) >> 16; }

    // Orient both boundary crossings from the narrow end of the trapezoid.
    yAtLeft = (-top * edge.inverse) >> 16;
    yAtRight = yAtLeft + edge.inverse;
    if (bottom < top) {
        yAtLeft = edge.height - yAtLeft;
        yAtRight = edge.height - yAtRight;
    }
    if (low >= 0) { return edge.height - ((((unit - low) >> 1) * yAtRight) >> 16); }
    if (high <= unit) { return ((high >> 1) * (edge.height - yAtLeft)) >> 16; }
    return edge.height - yAtRight + ((yAtRight - yAtLeft) >> 1);
}

/******************************************************************************
 * Function Name   : applyBlack
 * Description     : Blend black with the actual layer below before quantizing the page to RGB565.
 * Input Parameters: patch, absolute origin and pixel location, 8-bit stroke coverage
 * Return Value    : None.
 * Date            : 2026.11.28
 ******************************************************************************/
void applyBlack(QImage *patch, QPoint origin, int x, int y, int alpha) //20261128-11 add
{
    QRgb *row;
    QRgb pixel;
    int inverse = 255 - alpha;
    auto channel = [inverse](int value) {
        int product = value * inverse;
        return (product + (product >> 8) + 128) >> 8;
    };

    if (alpha <= 0) { return; }
    row = reinterpret_cast<QRgb *>(patch->scanLine(y - origin.y()));
    pixel = row[x - origin.x()];
    row[x - origin.x()] = qRgb(channel(qRed(pixel)), channel(qGreen(pixel)), channel(qBlue(pixel)));
}

/******************************************************************************
 * Function Name   : paintObliqueSegment
 * Description     : Scan the four edges of a one-pixel-wide flat-ended segment against the page.
 * Input Parameters: RGB32 patch and origin, endpoints in ascending Y order
 * Return Value    : None.
 * Date            : 2026.11.28
 ******************************************************************************/
void paintObliqueSegment(QImage *patch, QPoint origin, QPoint a, QPoint b) //20261128-11 add
{
    long double dx = b.x() - a.x();
    long double dy = b.y() - a.y();
    long double half = 0.5L * double(1.0L / std::sqrt(dx * dx + dy * dy));
    Corner normal{dy * half, -dx * half};
    Corner aPlus{a.x() + normal.x, a.y() + normal.y};
    Corner aMinus{a.x() - normal.x, a.y() - normal.y};
    Corner bPlus{b.x() + normal.x, b.y() + normal.y};
    Corner bMinus{b.x() - normal.x, b.y() - normal.y};
    Corner top = dx > 0 ? aPlus : aMinus;
    Corner left = dx > 0 ? aMinus : bMinus;
    Corner right = dx > 0 ? bPlus : aPlus;
    Corner bottom = dx > 0 ? bMinus : bPlus;
    // The native x87 pipeline stores both X coordinates and the left Y in binary64.
    // Top Y remains extended through the division. On long diagonals one 16.16 step matters.
    long double slope = (double(left.x) - double(top.x)) / (double(left.y) - top.y);
    double decreasing = double(slope);
    double increasing = double(-1.0L / slope);
    std::array<Edge, 4> edges = {makeEdge(top, left, decreasing, -increasing),
                                 makeEdge(left, bottom, increasing, -decreasing),
                                 makeEdge(top, right, increasing, -decreasing),
                                 makeEdge(right, bottom, decreasing, -increasing)};
    std::array<RowEdge, 4> row;
    qint64 firstX;
    qint64 lastX;
    qint64 area;
    int x;
    int y;
    size_t i;

    for (y = int(std::floor(top.y)); y <= int(std::floor(bottom.y)); ++y) {
        firstX = std::numeric_limits<qint64>::max();
        lastX = std::numeric_limits<qint64>::min();
        for (i = 0; i < edges.size(); ++i) {
            row[i] = intersectRow(edges[i], y);
            if (row[i].height > 0) {
                firstX = std::min(firstX, std::min(row[i].top, row[i].bottom));
                lastX = std::max(lastX, std::max(row[i].top, row[i].bottom));
            }
        }
        if (firstX > lastX) { continue; }
        for (x = std::max(origin.x(), int(firstX >> 16));
             x <= std::min(origin.x() + patch->width() - 1, int(lastX >> 16)); ++x) {
            area = leftArea(row[2], x) + leftArea(row[3], x) - leftArea(row[0], x) - leftArea(row[1], x);
            applyBlack(patch, origin, x, y, int(std::clamp(area, qint64(0), unit) * 255 / unit));
        }
    }
}
} // namespace

/******************************************************************************
 * Function Name   : paintDeviceThinCurve
 * Description     : Render verified opaque black straight subpaths against the caller's actual RGB565 layers.
 * Input Parameters: active device painter, page image, object geometry and curve properties
 * Return Value    : True after drawing, false before any page change for other curve forms.
 * Date            : 2026.11.28
 ******************************************************************************/
bool paintDeviceThinCurve(QPainter *painter, const QImage &page, const QRect &geometry,
                          const GraphicProperties &properties) //20261128-11 add
{
    QVector<QPointF> vertices;
    QVector<QPoint> points;
    QRect bounds;
    QImage patch;
    QRgb *row;
    QPoint a;
    QPoint b;
    QPoint point;
    int i;
    int x;
    int y;

    if (!painter || std::numeric_limits<long double>::digits < 64 || properties.lineStyle != 1 ||
        properties.lineWidth != 1 || properties.alpha != 255 || properties.lineColor != QColor(Qt::black) ||
        properties.endArrow != 0 || properties.endCap != 0 || !properties.curveControls.isEmpty()) {
        return false;
    }
    vertices = graphicHandles(QStringLiteral("linecurve"), QRectF(geometry), properties);
    if (vertices.size() < 2) { return false; }
    for (const QPointF &vertex : vertices) {
        point = vertex.toPoint();
        if (vertex != QPointF(point) || !page.rect().adjusted(1, 1, -1, -1).contains(point)) { return false; }
        points.append(point);
        bounds = bounds.united(QRect(point, QSize(1, 1)));
    }
    bounds = bounds.adjusted(-1, -1, 1, 1).intersected(page.rect());
    patch = page.copy(bounds).convertToFormat(QImage::Format_RGB32);
    if (patch.isNull()) { return false; }
    for (y = 0; y < patch.height(); ++y) {
        row = reinterpret_cast<QRgb *>(patch.scanLine(y));
        for (x = 0; x < patch.width(); ++x) {
            // The native page's background loses its low-order bits when expanded back from RGB565.
            if (row[x] == qRgb(198, 223, 255)) { row[x] = qRgb(197, 222, 255); }
        }
    }
    for (i = 1; i < points.size(); ++i) {
        a = points[i - 1];
        b = points[i];
        if (a.y() > b.y()) { std::swap(a, b); }
        if (a.y() == b.y()) {
            for (x = std::min(a.x(), b.x()); x < std::max(a.x(), b.x()); ++x) {
                applyBlack(&patch, bounds.topLeft(), x, a.y() - 1, 127);
                applyBlack(&patch, bounds.topLeft(), x, a.y(), 127);
            }
        } else if (a.x() == b.x()) {
            for (y = a.y(); y < b.y(); ++y) {
                applyBlack(&patch, bounds.topLeft(), a.x() - 1, y, 127);
                applyBlack(&patch, bounds.topLeft(), a.x(), y, 127);
            }
        } else {
            paintObliqueSegment(&patch, bounds.topLeft(), a, b);
        }
    }
    painter->drawImage(bounds.topLeft(), patch);
    return true;
}
