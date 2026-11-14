#ifndef ELLIPSE_CURVE_CANDIDATE_H
#define ELLIPSE_CURVE_CANDIDATE_H

#include <QPainterPath>
#include <QString>
#include <QVector>

struct EllipseFlattenStats
{
    qint64 inputElements = 0;
    qint64 generatedElements = 0;
    qint64 extentX = 0;
    qint64 extentY = 0;
    qint64 adaptation = 0;
    qint64 cubicThreshold = 0;
    qint64 thirdThreshold = 0;
    QVector<qint64> depths;
};

/******************************************************************************
 * Function Name   : flattenHistoricalEllipseCandidate
 * Description     : Produce bounded straight-edge geometry from independent Qt6 paths.
 * Input Parameters: input, output, error, optional stats
 * Return Value    : True after atomic output replacement; false with a refusal reason.
 * Date            : 2026.11.14
 ******************************************************************************/
bool flattenHistoricalEllipseCandidate(const QPainterPath &input, QPainterPath *output,
                                      QString *error, EllipseFlattenStats *stats = nullptr); //20261114-5 add

#endif
