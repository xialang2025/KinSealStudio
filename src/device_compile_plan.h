#ifndef DEVICE_COMPILE_PLAN_H
#define DEVICE_COMPILE_PLAN_H

#include "project_model.h"
#include <QJsonObject>

//20261113-11 add: This immutable projection has no device handle or publication side effect.
struct DeviceCompilePlan
{
    QList<HmiObject> objects;
    QJsonObject sourceSnapshot;
    int staticPolylineCount = 0;
    int staticTableCount = 0; //20261113-12 add: Static tables allocate background pixels, never dynamic control records.
    int staticRectCount = 0; //20261113-13 add: Closed rectangle rasters have no device control union.
    int staticLineCount = 0; //20261113-14 add: Canonical integer straight lines allocate background pixels only.
    int staticSectorCount=0; //20261114-3 add: Default-angle sectors allocate only verified background pixels.
    int staticEllipseCount=0; //20261114-2 add: Count only source-verified ellipse background semantics.
    int staticPolygonCount=0; //20261114-1 add: Count only sampled static polygon background rasters.
    int staticRoundrectCount = 0; //20261113-16 add: Rounded-box spans allocate only page background pixels.
    int staticFreelineCount = 0; //20261113-15 add: Only the verified two-endpoint freeline subset is baked into the background.
};

bool prepareDeviceCompilePlan(const HmiProject &project, DeviceCompilePlan *plan,
                             QString *error, QJsonObject *issue = nullptr); //20261113-11 add

#endif
