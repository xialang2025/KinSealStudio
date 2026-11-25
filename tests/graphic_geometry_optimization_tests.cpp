#include "graphic_geometry_optimization.h"
#include "project_model.h"

#include <QTest>

namespace {
/******************************************************************************
 * Function Name   : makeShape
 * Description     : Build a portable fixture using the editor's normalized vertex coordinates.
 * Input Parameters: type, geometry, points
 * Return Value    : Editable HMI shape
 * Date            : 2026.11.25
 ******************************************************************************/
HmiObject makeShape(const QString &type, const QRect &geometry, const QVector<QPointF> &points) //20261125-83 add
{
    HmiObject shape;
    shape.type = type;
    shape.geometry = geometry;
    shape.graphic.points = points;
    return shape;
}

/******************************************************************************
 * Function Name   : pointsOf
 * Description     : Read model vertices in visible scene coordinates without using the optimizer.
 * Input Parameters: shape
 * Return Value    : Observable vertex locations
 * Date            : 2026.11.25
 ******************************************************************************/
QVector<QPointF> pointsOf(const HmiObject &shape) //20261125-83 add
{
    return graphicHandles(shape.type, QRectF(shape.geometry).translated(shape.subpixelOffset), shape.graphic);
}
}

class GraphicGeometryOptimizationTests : public QObject {
    Q_OBJECT
private slots:
    void nativeThreePointCurve();
    void upwardAndDownwardRounding();
    void nonCosmeticStroke();
    void unsupportedGeometryPreserved();
};

/******************************************************************************
 * Function Name   : nativeThreePointCurve
 * Description     : The three-click SM has a zero-width pen and native sceneReads of (100,50,200,100).
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void GraphicGeometryOptimizationTests::nativeThreePointCurve() //20261125-83 add
{
    const QVector<QPointF> controls = {{0, 1}, {.5, 0}, {1, 1}};
    HmiObject shape = makeShape(QStringLiteral("linecurve"), {100, 50, 200, 100}, controls);
    QVERIFY(optimizeGraphicVertices(&shape));
    QCOMPARE(shape.geometry, QRect(100, 50, 200, 100));
    QCOMPARE(shape.subpixelOffset, QPointF());
    QCOMPARE(shape.graphic.points, controls);
}

/******************************************************************************
 * Function Name   : upwardAndDownwardRounding
 * Description     : Check both-axis rounding against the two native vertex resize policies.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void GraphicGeometryOptimizationTests::upwardAndDownwardRounding() //20261125-83 add
{
    const QRect box(100, 50, 8, 8);
    const QVector<QPointF> points = {{0, 0}, {5.75 / 8, 2.25 / 8}};
    HmiObject line = makeShape(QStringLiteral("line"), box, points);
    HmiObject curve = makeShape(QStringLiteral("linecurve"), box, points);
    line.subpixelOffset = curve.subpixelOffset = QPointF(.125, .25);
    QVERIFY(optimizeGraphicVertices(&line));
    QVERIFY(optimizeGraphicVertices(&curve));
    QCOMPARE(pointsOf(line), QVector<QPointF>({{100, 50}, {106, 52}}));
    QCOMPARE(pointsOf(curve), QVector<QPointF>({{100, 50}, {105.75, 52}}));
}

/******************************************************************************
 * Function Name   : nonCosmeticStroke
 * Description     : Test the native flat-cap width projection and the half-pixel polygon outline.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void GraphicGeometryOptimizationTests::nonCosmeticStroke() //20261125-83 add
{
    HmiObject line = makeShape(QStringLiteral("line"), {100, 51, 8, 1}, {{0, 0}, {6.25 / 8, 0}});
    HmiObject polygon = makeShape(QStringLiteral("polygon"), {200, 100, 10, 6}, {{0, 0}, {1, 0}, {1, 1}, {0, 1}});
    line.graphic.lineWidth = 4;
    line.subpixelOffset = QPointF(.25, .25);
    QVERIFY(optimizeGraphicVertices(&line));
    QCOMPARE(pointsOf(line), QVector<QPointF>({{100, 51}, {106, 51}}));
    QVERIFY(optimizeGraphicVertices(&polygon));
    QCOMPARE(pointsOf(polygon), QVector<QPointF>({{200.5, 100.5}, {210.5, 100.5}, {210.5, 106.5}, {200.5, 106.5}}));
}

/******************************************************************************
 * Function Name   : unsupportedGeometryPreserved
 * Description     : Preserve decorated or mirrored paths for which the native point cache is unknown.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.25
 ******************************************************************************/
void GraphicGeometryOptimizationTests::unsupportedGeometryPreserved() //20261125-83 add
{
    HmiObject line = makeShape(QStringLiteral("line"), {100, 50, 20, 20}, {{0, 0}, {1, 1}});
    line.graphic.endArrow = 1;
    QVERIFY(!optimizeGraphicVertices(&line));
    QCOMPARE(line.geometry, QRect(100, 50, 20, 20));
    QCOMPARE(line.graphic.points, QVector<QPointF>({{0, 0}, {1, 1}}));
}
QTEST_MAIN(GraphicGeometryOptimizationTests)
#include "graphic_geometry_optimization_tests.moc"
