#ifndef DEVICE_TARGET_H
#define DEVICE_TARGET_H

#include <QByteArray>
#include <QString>

/******************************************************************************
 * Function Name   : isAmz070Target
 * Description     : Recognize the AMZ project key and the physical model name.
 * Input Parameters: model
 * Return Value    : True for AMZ070W01 or AMZ070W01R.
 * Date            : 2026.11.22
 ******************************************************************************/
inline bool isAmz070Target(const QString &model) //20261122-3 add
{
    return model == QStringLiteral("AMZ070W01") || model == QStringLiteral("AMZ070W01R");
}

/******************************************************************************
 * Function Name   : deviceTargetIdentityByte
 * Description     : Map a device profile to the verified USB identify reply.
 * Input Parameters: model
 * Return Value    : The first reply byte, or -1 for an unsupported model.
 * Date            : 2026.11.22
 ******************************************************************************/
inline int deviceTargetIdentityByte(const QString &model) //20261122-3 add
{
    if (isAmz070Target(model))
    {
        return 0x46;
    }
    if (model == QStringLiteral("AMT070W124-L800X480R"))
    {
        return 0x4c;
    }
    return -1;
}

/******************************************************************************
 * Function Name   : deviceTargetAcceptsStartOptions
 * Description     : Require the default start flags for the AMZ profile.
 * Input Parameters: model, options
 * Return Value    : True for a supported target and supported start flags.
 * Date            : 2026.11.22
 ******************************************************************************/
inline bool deviceTargetAcceptsStartOptions(const QString &model, const QByteArray &options) //20261122-3 add
{
    return deviceTargetIdentityByte(model) >= 0 &&
           (options == QByteArray::fromHex("000000") ||
            (!isAmz070Target(model) && options == QByteArray::fromHex("010101")));
}

#endif
