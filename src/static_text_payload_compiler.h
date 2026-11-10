#ifndef STATIC_TEXT_PAYLOAD_COMPILER_H
#define STATIC_TEXT_PAYLOAD_COMPILER_H

#include "project_model.h"

//20261109-0 add: Localized static text uses one native indexed image per language.
struct StaticTextSpriteSet
{
    QByteArray pixelData;
    QByteArray lengthRecords;
    u32 rawLength = 0;
};

bool buildStaticTextSprites(const HmiObject &object, const QStringList &languages,
                            StaticTextSpriteSet *sprites, QString *error); //20261109-0 add

#endif
