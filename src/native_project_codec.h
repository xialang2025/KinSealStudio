#ifndef NATIVE_PROJECT_CODEC_H
#define NATIVE_PROJECT_CODEC_H

#include <QByteArray>
#include <QString>
#include <QStringList>
#include <QList> //20261110-5 add: Publish inspected typed spans without exposing editable parser state.

struct HmiProject;

using u8 = unsigned char;
using u16 = unsigned short;
using u32 = unsigned int;

//20261109-0 add: Envelope inspection does not validate the opaque native project body.
struct NativeProjectEnvelope
{
    QString signature;
    u32 version = 0;
    QString targetModel;
    qsizetype payloadOffset = 0;
    QByteArray originalBytes;
    QByteArray originalSha256;
    QStringList limitations;
};

//20261109-0 add: Preserve the four source flags without guessing their device-record meaning.
struct NativeSourceAddress
{
    QString text;
    u8 field0 = 0;
    u8 field1 = 0;
    u8 field2 = 0;
    QString numericText;
    QString registerText;
    u8 field3 = 0;
    u32 rawLength = 0;
    QString auxiliary;
};

//20261110-5 add: A span identifies verified stream boundaries, not writable semantic support.
struct NativeSourceFieldSpan
{
    QString path;
    char kind = 0;
    qsizetype start = 0;
    qsizetype end = 0;
};

/******************************************************************************
 * Function Name   : inspectNativeProjectSourceFields
 * Description     : Return ordered typed spans only after complete supported source parsing.
 * Input Parameters: data, fields, error
 * Return Value    : True after transactional replacement; unknown bodies remain archive-only.
 * Date            : 2026.11.10
 ******************************************************************************/
bool inspectNativeProjectSourceFields(const QByteArray &data, QList<NativeSourceFieldSpan> *fields,
                                      QString *error = nullptr); //20261110-5 add

/******************************************************************************
 * Function Name   : inspectNativeProjectEnvelope
 * Description     : Validate the recovered source header and retain the unparsed body verbatim.
 * Input Parameters: data, envelope, error
 * Return Value    : True for a supported header; body and controls remain unvalidated.
 * Date            : 2026.11.09
 ******************************************************************************/
bool inspectNativeProjectEnvelope(const QByteArray &data, NativeProjectEnvelope *envelope,
                                  QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : restoreNativeProjectArchive
 * Description     : Return an unchanged native snapshot after checking its original digest.
 * Input Parameters: envelope, data, error
 * Return Value    : True after exact restoration; no semantic export is performed.
 * Date            : 2026.11.09
 ******************************************************************************/
bool restoreNativeProjectArchive(const NativeProjectEnvelope &envelope, QByteArray *data,
                                  QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : validateNativeProjectSourceStructure
 * Description     : Traverse recovered page lists and five known control families without enabling multi-page edits.
 * Input Parameters: data, error
 * Return Value    : True only for complete typed boundaries; semantic import remains separately restricted.
 * Date            : 2026.11.09
 ******************************************************************************/
bool validateNativeProjectSourceStructure(const QByteArray &data,
                                           QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : nativeProjectUsesSupportedDeviceDefaults
 * Description     : Check exact observed native system and project configuration fingerprints.
 * Input Parameters: data, error
 * Return Value    : True only for configuration bytes matching the verified device defaults.
 * Date            : 2026.11.09
 ******************************************************************************/
bool nativeProjectUsesSupportedDeviceDefaults(const QByteArray &data,
                                                QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : nativeProjectInitialLanguageIndex
 * Description     : Preserve the native startup language by name after editor language registration changes.
 * Input Parameters: project, index, error
 * Return Value    : True after replacing index transactionally; new projects and deleted startup languages use zero.
 * Date            : 2026.11.09
 ******************************************************************************/
bool nativeProjectInitialLanguageIndex(const HmiProject &project, u16 *index,
                                       QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : readNativeSourceAddress
 * Description     : Decode one bounded source address using the recovered Qt stream field order.
 * Input Parameters: data, offset, address, nextOffset, error
 * Return Value    : True after replacing address and nextOffset transactionally.
 * Date            : 2026.11.09
 ******************************************************************************/
bool readNativeSourceAddress(const QByteArray &data, qsizetype offset,
                              NativeSourceAddress *address, qsizetype *nextOffset,
                              QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : writeNativeSourceAddress
 * Description     : Encode a source address while preserving opaque flags and null QStrings.
 * Input Parameters: address, data, error
 * Return Value    : True after replacing data transactionally.
 * Date            : 2026.11.09
 ******************************************************************************/
bool writeNativeSourceAddress(const NativeSourceAddress &address, QByteArray *data,
                               QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : importNativeProjectSource
 * Description     : Import recovered version-122 single-page bit/word/function/lamp and static-text records.
 * Input Parameters: data, project, error
 * Return Value    : True after complete supported parsing; failed imports leave project untouched.
 * Date            : 2026.11.09
 ******************************************************************************/
bool importNativeProjectSource(const QByteArray &data, HmiProject *project,
                                QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : exportNativeProjectSource
 * Description     : Save supported native controls and languages while preserving all other original metadata bytes.
 * Input Parameters: project, data, error
 * Return Value    : True for supported snapshot edits; unsupported changes leave data untouched.
 * Date            : 2026.11.09
 ******************************************************************************/
bool exportNativeProjectSource(const HmiProject &project, QByteArray *data,
                                QString *error = nullptr); //20261109-0 add

#endif
