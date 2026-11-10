#ifndef BIT_BUTTON_PAYLOAD_COMPILER_H
#define BIT_BUTTON_PAYLOAD_COMPILER_H

#include "project_model.h"
#include <QByteArray>
#include <QList>
#include <QImage> //20261109-0 add

using u32 = unsigned int;

//20261109-0 add: Length records describe independently padded control-state images.
struct BitButtonSpriteSet
{
    QByteArray pixelData;
    QByteArray lengthRecords;
    u32 rawLength = 0;
};

/******************************************************************************
 * Function Name   : buildBitButtonSprites
 * Description     : Render explicit states into packed RGB565 and prepare native LZO records.
 * Input Parameters: object, language, stateIndices, sprites, error
 * Return Value    : True when each state passed compression round-trip verification.
 * Date            : 2026.11.09
 ******************************************************************************/
bool buildBitButtonSprites(const HmiObject &object, const QString &language,
                            const QList<int> &stateIndices, BitButtonSpriteSet *sprites,
                            QString *error, const QImage &underlay = QImage(),
                            const QList<HmiObject> &overlays = QList<HmiObject>()); //20261109-0 add

#endif
