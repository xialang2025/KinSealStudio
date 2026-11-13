#ifndef DEVICE_PACKAGE_H
#define DEVICE_PACKAGE_H
#include "project_model.h"
#include <QJsonObject>
#include <functional>

using DevicePackageWriter = std::function<bool(const QString &, const QByteArray &, QString *)>;
bool validateDevicePayloadStructure(const QVector<QByteArray> &payloads, QString *error); //20261113-11 add
bool verifyDevicePackage(const QString &directory, QJsonObject *manifest, QString *error); //20261113-11 add
bool publishDevicePackage(const HmiProject &project, const QVector<QByteArray> &payloads,
                          const QString &directory, QString *error,
                          const DevicePackageWriter &writer = DevicePackageWriter()); //20261113-11 add
#endif
