#include "ellipse_curve_candidate.h"

#include <algorithm>
#include <cmath>

namespace
{
constexpr qint64 kMaxInputElements = 512;
constexpr qint64 kMaxOutputElements = 4096;
constexpr qint64 kMaxDepth = 8;

struct Point
{
    qint64 x;
    qint64 y;
};

struct Cubic
{
    Point a;
    Point b;
    Point c;
    Point d;
};

/******************************************************************************
 * Function Name   : refuse
 * Description     : Preserve caller outputs while returning an explicit diagnostic.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.14
 ******************************************************************************/
bool refuse(QString *error, const QString &message) //20261114-5 add
{
    if (error)
    {
        *error = message;
    }
    return false;
}

/******************************************************************************
 * Function Name   : magnitude
 * Description     : Compute a signed magnitude after the bounded coordinate guard.
 * Input Parameters: value, whose magnitude is below 2^30 in this candidate
 * Return Value    : Nonnegative magnitude.
 * Date            : 2026.11.14
 ******************************************************************************/
qint64 magnitude(qint64 value) //20261114-5 add
{
    return value < 0 ? -value : value;
}

/******************************************************************************
 * Function Name   : midpoint
 * Description     : Retain every integer de Casteljau rounding on nonnegative coordinates.
 * Input Parameters: first, second in the 8-bit subpixel lattice
 * Return Value    : Truncated integer midpoint, equivalent to floor for accepted coordinates.
 * Date            : 2026.11.14
 ******************************************************************************/
Point midpoint(const Point &first, const Point &second) //20261114-5 add
{
    return {(first.x + second.x) / 2, (first.y + second.y) / 2};
}

/******************************************************************************
 * Function Name   : appendPoint
 * Description     : Bound generated geometry before allocating the next straight element.
 * Input Parameters: path, stats, point, error
 * Return Value    : False when the element budget would be exceeded.
 * Date            : 2026.11.14
 ******************************************************************************/
bool appendPoint(QPainterPath *path, EllipseFlattenStats *stats,
                 const Point &point, QString *error) //20261114-5 add
{
    if (stats->generatedElements >= kMaxOutputElements)
    {
        return refuse(error, QStringLiteral("Generated element budget exceeded (4096)."));
    }
    path->lineTo(qreal(point.x) / 256, qreal(point.y) / 256);
    stats->generatedElements++;
    return true;
}

/******************************************************************************
 * Function Name   : emitCubic
 * Description     : Expand a finite binary subdivision and emit each leaf midpoint and end.
 * Input Parameters: curve, depth, path, stats, error
 * Return Value    : True after all leaf edges were emitted within the resource budgets.
 * Date            : 2026.11.14
 ******************************************************************************/
bool emitCubic(const Cubic &curve, qint64 depth, QPainterPath *path,
               EllipseFlattenStats *stats, QString *error) //20261114-5 add
{
    Point ab;
    Point bc;
    Point cd;
    Point abc;
    Point bcd;
    Point center;
    Point leafMidpoint;
    Cubic left;
    Cubic right;

    if (depth < 0 || depth > kMaxDepth)
    {
        return refuse(error, QStringLiteral("Cubic subdivision depth outside 0..8."));
    }
    if (depth != 0)
    {
        ab = midpoint(curve.a, curve.b);
        bc = midpoint(curve.b, curve.c);
        cd = midpoint(curve.c, curve.d);
        abc = midpoint(ab, bc);
        bcd = midpoint(bc, cd);
        center = midpoint(abc, bcd);
        left = {curve.a, ab, abc, center};
        right = {center, bcd, cd, curve.d};
        return emitCubic(left, depth - 1, path, stats, error)
                && emitCubic(right, depth - 1, path, stats, error);
    }
    leafMidpoint = {(curve.a.x + 3 * curve.b.x + 3 * curve.c.x + curve.d.x) / 8,
                    (curve.a.y + 3 * curve.b.y + 3 * curve.c.y + curve.d.y) / 8};
    return appendPoint(path, stats, leafMidpoint, error)
            && appendPoint(path, stats, curve.d, error);
}

/******************************************************************************
 * Function Name   : chooseDepth
 * Description     : Preserve the historical diagnostic heuristic including its XY cross-term.
 * Input Parameters: curve in 26.6 coordinates, cubicThreshold, thirdThreshold, depth, error
 * Return Value    : True after proving the subdivision depth is within the fixed budget.
 * Date            : 2026.11.14
 ******************************************************************************/
bool chooseDepth(const Cubic &curve, qint64 cubicThreshold, qint64 thirdThreshold,
                 qint64 *depth, QString *error) //20261114-5 add
{
    qint64 secondMagnitude;
    qint64 thirdMagnitude;
    qint64 result;

    secondMagnitude = std::max(magnitude(curve.a.x + curve.d.x - 2 * curve.b.x),
                               magnitude(curve.a.y + curve.d.y - 2 * curve.b.y)) / cubicThreshold;
    // This X/Y cross-term is intentional compatibility behavior, not a geometry simplification.
    thirdMagnitude = std::max(magnitude(curve.a.x + curve.d.x - 3 * (curve.b.x + curve.c.x)),
                              magnitude(curve.a.y + curve.d.y - 3 * (curve.b.x + curve.c.y))) / thirdThreshold;
    result = 0;
    while (secondMagnitude != 0 || thirdMagnitude != 0)
    {
        secondMagnitude >>= 2;
        thirdMagnitude >>= 3;
        result++;
        if (result > kMaxDepth)
        {
            return refuse(error, QStringLiteral("Historical depth exceeds the maximum of 8."));
        }
    }
    *depth = result;
    return true;
}

/******************************************************************************
 * Function Name   : upscale
 * Description     : Convert already guarded 26.6 coordinates to the 8-bit subpixel lattice.
 * Input Parameters: point
 * Return Value    : Equivalent subpixel point.
 * Date            : 2026.11.14
 ******************************************************************************/
Point upscale(const Point &point) //20261114-5 add
{
    return {point.x * 4, point.y * 4};
}
}

/******************************************************************************
 * Function Name   : flattenHistoricalEllipseCandidate
 * Description     : Flatten bounded independently generated paths without any native DLL input.
 * Input Parameters: input, output, error, optional stats
 * Return Value    : True only after output and stats can be atomically replaced.
 * Date            : 2026.11.14
 ******************************************************************************/
bool flattenHistoricalEllipseCandidate(const QPainterPath &input, QPainterPath *output,
                                      QString *error, EllipseFlattenStats *stats) //20261114-5 add
{
    QPainterPath candidate;
    EllipseFlattenStats details;
    QVector<Point> quantized;
    QPainterPath::Element element;
    Point point;
    Point previous = {0, 0};
    Cubic curve;
    Cubic highResolution;
    qint64 minimumX;
    qint64 maximumX;
    qint64 minimumY;
    qint64 maximumY;
    qint64 depth;
    qint64 leafElements;
    qint64 index;
    bool havePrevious;

    if (!output || input.elementCount() == 0 || input.elementCount() > kMaxInputElements)
    {
        return refuse(error, QStringLiteral("Null output, empty path or input element budget exceeded (512)."));
    }
    details.inputElements = input.elementCount();
    quantized.reserve(input.elementCount());
    minimumX = 800 * 64;
    maximumX = 0;
    minimumY = 480 * 64;
    maximumY = 0;
    for (index = 0; index < details.inputElements; index++)
    {
        element = input.elementAt(int(index));
        if (!std::isfinite(element.x) || !std::isfinite(element.y)
                || element.x < 0 || element.x > 800 || element.y < 0 || element.y > 480)
        {
            return refuse(error, QStringLiteral("Control point is nonfinite or outside the nonnegative 800x480 guard."));
        }
        point = {qint64(element.x * 64), qint64(element.y * 64)};
        quantized.append(point);
        minimumX = std::min(minimumX, point.x);
        maximumX = std::max(maximumX, point.x);
        minimumY = std::min(minimumY, point.y);
        maximumY = std::max(maximumY, point.y);
    }
    details.extentX = (maximumX + 63) / 64 - minimumX / 64;
    details.extentY = (maximumY + 63) / 64 - minimumY / 64;
    details.adaptation = (details.extentX > 24 || details.extentY > 24 ? 1 : 0)
            + (details.extentX > 120 || details.extentY > 120 ? 1 : 0);
    details.cubicThreshold = qint64(16) << details.adaptation;
    details.thirdThreshold = qint64(32) << details.adaptation;
    candidate.setFillRule(input.fillRule());
    havePrevious = false;
    for (index = 0; index < details.inputElements; index++)
    {
        element = input.elementAt(int(index));
        point = quantized.at(int(index));
        if (element.type == QPainterPath::MoveToElement)
        {
            if (details.generatedElements >= kMaxOutputElements)
            {
                return refuse(error, QStringLiteral("Generated move element exceeds budget."));
            }
            candidate.moveTo(qreal(point.x) / 64, qreal(point.y) / 64);
            details.generatedElements++;
            previous = point;
            havePrevious = true;
        }
        else if (element.type == QPainterPath::LineToElement)
        {
            if (!havePrevious || !appendPoint(&candidate, &details, upscale(point), error))
            {
                return refuse(error, QStringLiteral("Line has no preceding move or exceeds the element budget."));
            }
            previous = point;
        }
        else if (element.type == QPainterPath::CurveToElement)
        {
            if (!havePrevious || index + 2 >= details.inputElements
                    || input.elementAt(int(index + 1)).type != QPainterPath::CurveToDataElement
                    || input.elementAt(int(index + 2)).type != QPainterPath::CurveToDataElement)
            {
                return refuse(error, QStringLiteral("Malformed cubic element sequence."));
            }
            curve = {previous, point, quantized.at(int(index + 1)), quantized.at(int(index + 2))};
            if (!chooseDepth(curve, details.cubicThreshold, details.thirdThreshold, &depth, error))
            {
                return false;
            }
            leafElements = qint64(2) << depth;
            if (leafElements > kMaxOutputElements - details.generatedElements)
            {
                return refuse(error, QStringLiteral("Whole cubic would exceed the generated element budget."));
            }
            details.depths.append(depth);
            highResolution = {upscale(curve.a), upscale(curve.b), upscale(curve.c), upscale(curve.d)};
            if (!emitCubic(highResolution, depth, &candidate, &details, error))
            {
                return false;
            }
            previous = curve.d;
            index += 2;
        }
        else
        {
            return refuse(error, QStringLiteral("Unattached curve data or unknown element type."));
        }
    }
    *output = candidate;
    if (stats)
    {
        *stats = details;
    }
    if (error)
    {
        error->clear();
    }
    return true;
}
