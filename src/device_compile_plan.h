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
};

bool prepareDeviceCompilePlan(const HmiProject &project, DeviceCompilePlan *plan,
                             QString *error, QJsonObject *issue = nullptr); //20261113-11 add

#endif
