#include "stage3_project_edits.h"
#include "animation_resources.h" //20261112-0 add
#include "plc_native_catalog.h"

#include <QComboBox>
#include <QCheckBox>
#include <QCryptographicHash>
#include <QDialogButtonBox>
#include <QFormLayout>
#include <QGroupBox>
#include <QHeaderView>
#include <QLabel>
#include <QPushButton>
#include <QSet>
#include <QSpinBox>
#include <QTableWidget>
#include <QUuid>
#include <QVBoxLayout>
#include <limits>

namespace
{
struct AddressSlot
{
    Stage3AddressField field;
    QString *value;
    PlcAddressRole role;
    u32 count;
};

/******************************************************************************
 * Function Name   : editError
 * Description     : Fail an editor transaction without publishing its intermediate values.
 * Input Parameters: error, message
 * Return Value    : False.
 * Date            : 2026.11.10
 ******************************************************************************/
bool editError(QString *error, const QString &message) //20261110-8 add
{
    if (error != nullptr) { *error = message; }
    return false;
}

/******************************************************************************
 * Function Name   : addConditionSlots
 * Description     : Enumerate enabled access conditions while leaving dormant address drafts untouched.
 * Input Parameters: condition, touch, addressSlots
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void addConditionSlots(StaticTextProperties *condition, bool touch, QList<AddressSlot> *addressSlots) //20261110-8 add
{
    if (!condition->visibilityEnabled) { return; }
    if (condition->bitEnabled)
    {
        addressSlots->append({touch ? Stage3AddressField::TouchBit : Stage3AddressField::VisibilityBit,
                       &condition->bitAddress, PlcAddressRole::Bit, 1U});
    }
    if (condition->wordEnabled)
    {
        addressSlots->append({touch ? Stage3AddressField::TouchWord : Stage3AddressField::VisibilityWord,
                       &condition->wordAddress, PlcAddressRole::Word,
                       (condition->dataType == 0 || condition->dataType == 2) ? 1U : 2U});
    }
}

/******************************************************************************
 * Function Name   : activeSlots
 * Description     : Recover the five existing families' active read/write carriers and their full transfer spans.
 * Input Parameters: object
 * Return Value    : Typed address slots; disabled drafts and image text metadata are not rewritten.
 * Date            : 2026.11.10
 ******************************************************************************/
QList<AddressSlot> activeSlots(HmiObject *object) //20261110-8 add
{
    QList<AddressSlot> addressSlots;
    NumberDisplayProperties *ascii;
    MultiFunctionCommand *mfCommand; //20261112-0 add
    int index; //20261112-0 add: Each MF command owns an independently validated address slot.
    MultiStateButtonProperties *multi; //20261112-0 add
    BitButtonProperties *screen; //20261112-0 add
    if (object->type == QStringLiteral("chart")) //20261112-1 add: Increment only the active monitor and dynamic source carriers.
    {
        if (object->chart.shape == 0)
        {
            addressSlots.append({Stage3AddressField::Operation, &object->chart.bar.address, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.bar.dataType)});
            if (object->chart.bar.dynamicRange) { addressSlots.append({Stage3AddressField::DisplayMinimumWord, &object->chart.bar.minimumAddress, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.bar.dataType)}); addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->chart.bar.maximumAddress, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.bar.dataType)}); }
        }
        else if (object->chart.shape == 1) { addressSlots.append({Stage3AddressField::Operation, &object->chart.meter.address, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.meter.dataType)}); }
        else { addressSlots.append({Stage3AddressField::Operation, &object->chart.donut.address, PlcAddressRole::Word, ChartProperties::wordCount(object->chart.donut.dataType)}); }
        addConditionSlots(&object->chart.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("historytrend")) { addConditionSlots(&object->historyTrend.visibility, false, &addressSlots); } //20261112-1 add
    if (object->type == QStringLiteral("trend")) //20261112-1 add: Sampling uses a shared full-span read carrier and independent trigger/clear bits.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->trend.readAddress, PlcAddressRole::Word, TrendProperties::wordCount(object->trend.dataType) * object->trend.channelCount});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->trend.triggerAddress, PlcAddressRole::Bit, 1U});
        addressSlots.append({Stage3AddressField::ClearBit, &object->trend.clearAddress, PlcAddressRole::Bit, 1U});
        addConditionSlots(&object->trend.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("xycurve")) //20261113-1 add: Guard paired X/Y coordinates and keep trigger/clear increments independent.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->xyCurve.readAddress, PlcAddressRole::Word, XyCurveProperties::wordCount(object->xyCurve.dataType) * u32(object->xyCurve.channelCount) * 2U});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->xyCurve.triggerAddress, PlcAddressRole::Bit, 1U}); addressSlots.append({Stage3AddressField::ClearBit, &object->xyCurve.clearAddress, PlcAddressRole::Bit, 1U}); addConditionSlots(&object->xyCurve.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("alarmbar")) { addConditionSlots(&object->alarmBar.visibility, false, &addressSlots); } //20261113-2 add
    if (object->type == QStringLiteral("datagroup")) //20261113-1 add: A group owns one full point buffer and independent active sample/maximum words.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->dataGroup.readAddress, PlcAddressRole::Word, object->dataGroup.readWordSpan()});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->dataGroup.triggerAddress, PlcAddressRole::Bit, 1U});
        if (object->dataGroup.sampleMode == 1) { addressSlots.append({Stage3AddressField::SamplingWord, &object->dataGroup.sampleAddress, PlcAddressRole::Word, 1U}); }
        if (object->dataGroup.rangeSource == 1) { addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->dataGroup.maximumAddress, PlcAddressRole::Word, DataGroupProperties::wordCount(object->dataGroup.dataType)}); }
        addConditionSlots(&object->dataGroup.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("alarmdisplay")) { addConditionSlots(&object->alarmDisplay.visibility, false, &addressSlots); } //20261113-2 add
    if (object->type == QStringLiteral("track")) //20261112-0 add: Track constants and dynamic ranges have independent word-carrier roles.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->moveTrack.address, PlcAddressRole::Word, 1U});
        if (object->moveTrack.dynamicRange) { addressSlots.append({Stage3AddressField::DisplayMinimumWord, &object->moveTrack.minimumAddress, PlcAddressRole::Word, 1U}); addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->moveTrack.maximumAddress, PlcAddressRole::Word, 1U}); }
        addConditionSlots(&object->moveTrack.visibility, false, &addressSlots); addConditionSlots(&object->moveTrack.touch, true, &addressSlots);
        if (object->moveTrack.notice && object->moveTrack.noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &object->moveTrack.noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (object->moveTrack.notice && object->moveTrack.noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &object->moveTrack.noticeWordAddress, PlcAddressRole::Word, object->moveTrack.noticeWordType == 0 || object->moveTrack.noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("wheel")) //20261112-0 add: Wheel has one U16 operation carrier and no slider ranges/notifications.
    { addressSlots.append({Stage3AddressField::Operation, &object->wheel.address, PlcAddressRole::Word, 1U}); addConditionSlots(&object->wheel.visibility, false, &addressSlots); addConditionSlots(&object->wheel.touch, true, &addressSlots); }
    if (object->type == QStringLiteral("slider")) //20261112-0 add: Operation/range and shared conditions retain independent full-width roles.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->slider.address, PlcAddressRole::Word, u32(object->slider.transferWordCount())});
        if (object->slider.dynamicRange) { addressSlots.append({Stage3AddressField::DisplayMinimumWord, &object->slider.minimumAddress, PlcAddressRole::Word, u32(object->slider.transferWordCount())}); addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->slider.maximumAddress, PlcAddressRole::Word, u32(object->slider.transferWordCount())}); }
        addConditionSlots(&object->slider.visibility, false, &addressSlots); addConditionSlots(&object->slider.touch, true, &addressSlots);
        if (object->slider.notice && object->slider.noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &object->slider.noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (object->slider.notice && object->slider.noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &object->slider.noticeWordAddress, PlcAddressRole::Word, object->slider.noticeWordType == 0 || object->slider.noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("flow")) //20261112-0 add: Trigger and visibility are separate read-only roles; no touch/notice carriers are created.
    { addressSlots.append({Stage3AddressField::MonitorBit, &object->flowBlock.triggerAddress, PlcAddressRole::Bit, 1U}); addConditionSlots(&object->flowBlock.visibility, false, &addressSlots); }
    if (object->type == QStringLiteral("dropdown")) //20261112-0 add: The selector owns one full-width operation carrier and independent notification roles.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->dropdown.address, PlcAddressRole::Word, u32(object->dropdown.transferWordCount())});
        addConditionSlots(&object->dropdown.visibility, false, &addressSlots); addConditionSlots(&object->dropdown.touch, true, &addressSlots);
        if (object->dropdown.notice && object->dropdown.noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &object->dropdown.noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (object->dropdown.notice && object->dropdown.noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &object->dropdown.noticeWordAddress, PlcAddressRole::Word, object->dropdown.noticeWordType == 0 || object->dropdown.noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("multifunction")) //20261112-0 add: Apply operation increments to each independent bit/word command.
    {
        screen = &object->multiFunctionButton.common;
        for (index = 0; index < object->multiFunctionButton.commands.size(); ++index)
        { mfCommand = &object->multiFunctionButton.commands[index]; if (mfCommand->kind != 1) { addressSlots.append({Stage3AddressField::Operation, &mfCommand->address, mfCommand->kind == 0 ? PlcAddressRole::Bit : PlcAddressRole::Word, mfCommand->kind == 0 ? 1U : u32(mfCommand->transferWordCount())}); } }
        addConditionSlots(&screen->visibility, false, &addressSlots); addConditionSlots(&screen->touch, true, &addressSlots);
    }
    if (object->type == QStringLiteral("multistate")) //20261112-0 add: Monitor and input are distinct unsigned-word roles.
    {
        multi = &object->multiStateButton; screen = &multi->common;
        addressSlots.append({Stage3AddressField::MonitorWord, &multi->monitorAddress, PlcAddressRole::Word, multi->dataType == 0 ? 1U : 2U});
        if (!multi->sameAddress) { addressSlots.append({Stage3AddressField::Operation, &multi->writeAddress, PlcAddressRole::Word, multi->dataType == 0 ? 1U : 2U}); }
        addConditionSlots(&screen->visibility, false, &addressSlots); addConditionSlots(&screen->touch, true, &addressSlots);
        if (screen->notice && screen->noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &screen->noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (screen->notice && screen->noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &screen->noticeWordAddress, PlcAddressRole::Word, screen->noticeWordType == 0 || screen->noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("screen")) //20261112-0 add: Navigation is not an operating PLC address.
    {
        screen = &object->screenButton.common;
        if (screen->indicator) { addressSlots.append({screen->monitorType == 0 ? Stage3AddressField::MonitorBit : Stage3AddressField::MonitorWord, screen->monitorType == 0 ? &screen->monitorBitAddress : &screen->monitorWordAddress, screen->monitorType == 0 ? PlcAddressRole::Bit : PlcAddressRole::Word, 1U}); }
        addConditionSlots(&screen->visibility, false, &addressSlots); addConditionSlots(&screen->touch, true, &addressSlots);
        if (screen->notice && screen->noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &screen->noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (screen->notice && screen->noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &screen->noticeWordAddress, PlcAddressRole::Word, screen->noticeWordType == 0 || screen->noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->type == QStringLiteral("time")) { addConditionSlots(&object->timeDisplay.visibility, false, &addressSlots); } //20261112-0 add: RTC exposes only visibility carriers.
    if (object->type == QStringLiteral("ascii")) //20261112-0 add: Keep ASCII base carriers separate from unknown string spans and inactive numeric ranges.
    {
        ascii = &object->asciiDisplay.common;
        addressSlots.append({Stage3AddressField::MonitorWord, &ascii->monitorAddress, PlcAddressRole::Word, 1U});
        if (ascii->allowInput && !ascii->sameMonitor) { addressSlots.append({Stage3AddressField::Operation, &ascii->inputAddress, PlcAddressRole::Word, 1U}); }
        if (ascii->offsetEnabled) { addressSlots.append({Stage3AddressField::OffsetWord, &ascii->offsetAddress, PlcAddressRole::Word, 1U}); }
        addConditionSlots(&ascii->visibility, false, &addressSlots); if (ascii->allowInput) { addConditionSlots(&ascii->touch, true, &addressSlots); }
        if (ascii->allowInput && ascii->notice && ascii->noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &ascii->noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (ascii->allowInput && ascii->notice && ascii->noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &ascii->noticeWordAddress, PlcAddressRole::Word, ascii->noticeWordType == 0 || ascii->noticeWordType == 2 ? 1U : 2U}); }
    }
    if (object->hasNumberDisplayProperties) //20261112-0 add: Numeric input, monitor and range roles remain independent.
    {
        addressSlots.append({Stage3AddressField::MonitorWord, &object->numberDisplay.monitorAddress, PlcAddressRole::Word, u32(NumberDisplayProperties::wordSpan(object->numberDisplay.dataType))});
        if (object->numberDisplay.allowInput && !object->numberDisplay.sameMonitor) { addressSlots.append({Stage3AddressField::Operation, &object->numberDisplay.inputAddress, PlcAddressRole::Word, u32(NumberDisplayProperties::wordSpan(object->numberDisplay.dataType))}); }
        if (object->numberDisplay.offsetEnabled) { addressSlots.append({Stage3AddressField::OffsetWord, &object->numberDisplay.offsetAddress, PlcAddressRole::Word, 1U}); }
        if (object->numberDisplay.allowInput && object->numberDisplay.inputRangeSource == 1)
        {
            addressSlots.append({Stage3AddressField::InputMinimumWord, &object->numberDisplay.inputMinimumAddress, PlcAddressRole::Word, u32(NumberDisplayProperties::wordSpan(object->numberDisplay.dataType))});
            addressSlots.append({Stage3AddressField::InputMaximumWord, &object->numberDisplay.inputMaximumAddress, PlcAddressRole::Word, u32(NumberDisplayProperties::wordSpan(object->numberDisplay.dataType))});
        }
        if (object->numberDisplay.displayRange && object->numberDisplay.displayRangeSource == 1)
        {
            addressSlots.append({Stage3AddressField::DisplayMinimumWord, &object->numberDisplay.displayMinimumAddress, PlcAddressRole::Word, u32(NumberDisplayProperties::wordSpan(object->numberDisplay.dataType))});
            addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->numberDisplay.displayMaximumAddress, PlcAddressRole::Word, u32(NumberDisplayProperties::wordSpan(object->numberDisplay.dataType))});
        }
        if (object->numberDisplay.allowInput && object->numberDisplay.notice && object->numberDisplay.noticeBit) { addressSlots.append({Stage3AddressField::NoticeBit, &object->numberDisplay.noticeBitAddress, PlcAddressRole::Bit, 1U}); }
        if (object->numberDisplay.allowInput && object->numberDisplay.notice && object->numberDisplay.noticeWord) { addressSlots.append({Stage3AddressField::NoticeWord, &object->numberDisplay.noticeWordAddress, PlcAddressRole::Word, object->numberDisplay.noticeWordType == 0 || object->numberDisplay.noticeWordType == 2 ? 1U : 2U}); }
        addConditionSlots(&object->numberDisplay.visibility, false, &addressSlots); if (object->numberDisplay.allowInput) { addConditionSlots(&object->numberDisplay.touch, true, &addressSlots); }
    }

    if (object->type == QStringLiteral("gif")) //20261112-0 add: Playback gating and visibility use independent typed address slots.
    {
        if (object->gifDisplay.bitControl) { addressSlots.append({Stage3AddressField::MonitorBit, &object->gifDisplay.controlAddress, PlcAddressRole::Bit, 1U}); }
        addConditionSlots(&object->gifDisplay.visibility, false, &addressSlots);
    }
    if (object->hasImageDisplayProperties) //20261112-0 add: Static/time variants retain inactive address drafts.
    {
        if (object->imageDisplay.conditionEnabled && object->imageDisplay.condition != 0)
        { addressSlots.append({object->imageDisplay.condition == 2 ? Stage3AddressField::MonitorBit : Stage3AddressField::MonitorWord, &object->imageDisplay.monitorAddress, object->imageDisplay.condition == 2 ? PlcAddressRole::Bit : PlcAddressRole::Word, 1U}); }
        addConditionSlots(&object->imageDisplay.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("qrcode")) //20261112-0 add: String span remains unknown; only validate its base carrier.
    {
        addressSlots.append({Stage3AddressField::MonitorWord, &object->qrCode.monitorAddress, PlcAddressRole::Word, 1U});
        addConditionSlots(&object->qrCode.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("message")) //20261112-0 add: Monitor and visibility are reads, never an operating write slot.
    {
        addressSlots.append({Stage3AddressField::MonitorWord, &object->messageDisplay.monitorAddress, PlcAddressRole::Word, object->messageDisplay.dataType == 0 ? 1U : 2U});
        addConditionSlots(&object->messageDisplay.appearance, false, &addressSlots);
    }
    if (object->type == QStringLiteral("switch") && !object->hasLampProperties && !object->hasFunctionButtonProperties)
    {
        addressSlots.append({Stage3AddressField::Operation, &object->address,
                      object->hasWordButtonProperties ? PlcAddressRole::Word : PlcAddressRole::Bit,
                      object->hasWordButtonProperties ? u32(object->wordButton.transferWordCount()) : 1U});
    }
    if (object->hasStaticTextProperties) { addConditionSlots(&object->staticText, false, &addressSlots); }
    if (!object->hasBitButtonProperties) { return addressSlots; }
    if (object->hasLampProperties && object->lamp.monitorMode == 2)
    {
        addressSlots.append({Stage3AddressField::LampMultiMonitor, &object->lamp.multiMonitorAddress,
                      PlcAddressRole::Word, u32(object->lamp.transferWordCount())});
    }
    else if ((!object->hasLampProperties || object->lamp.monitorMode != 0) && object->bitButton.indicator &&
             !(object->bitButton.dualMonitor && object->bitButton.monitorSame))
    {
        if (!object->bitButton.dualMonitor || object->bitButton.monitorType == 1)
        {
            addressSlots.append({Stage3AddressField::MonitorWord, &object->bitButton.monitorWordAddress, PlcAddressRole::Word, 1U});
        }
        else { addressSlots.append({Stage3AddressField::MonitorBit, &object->bitButton.monitorBitAddress, PlcAddressRole::Bit, 1U}); }
    }
    if (object->bitButton.notice && object->bitButton.noticeBit)
    {
        addressSlots.append({Stage3AddressField::NoticeBit, &object->bitButton.noticeBitAddress, PlcAddressRole::Bit, 1U});
    }
    if (object->bitButton.notice && object->bitButton.noticeWord)
    {
        addressSlots.append({Stage3AddressField::NoticeWord, &object->bitButton.noticeWordAddress, PlcAddressRole::Word,
                      (object->bitButton.noticeWordType == 0 || object->bitButton.noticeWordType == 2) ? 1U : 2U});
    }
    addConditionSlots(&object->bitButton.visibility, false, &addressSlots);
    addConditionSlots(&object->bitButton.touch, true, &addressSlots);
    return addressSlots;
}

/******************************************************************************
 * Function Name   : selectedObjects
 * Description     : Require an exact nonempty active-page selection and keep its persisted object order.
 * Input Parameters: source, ids, objects, error
 * Return Value    : True with validated selected copies; duplicate or missing IDs fail atomically.
 * Date            : 2026.11.10
 ******************************************************************************/
bool selectedObjects(const HmiProject &source, const QStringList &ids, QList<HmiObject> *objects, QString *error) //20261110-8 add
{
    QSet<QString> requested;
    QList<HmiObject> selected;
    HmiObject object;
    QString id;

    if (objects == nullptr || ids.isEmpty() || ids.size() > 100 || !source.validateModel(error))
    {
        return editError(error, QStringLiteral("请选择当前画面的1..100个有效控件。%1").arg(error == nullptr ? QString() : *error));
    }
    Q_FOREACH (id, ids)
    {
        if (id.isEmpty() || requested.contains(id)) { return editError(error, QStringLiteral("选中控件ID为空或重复。")); }
        requested.insert(id);
    }
    Q_FOREACH (object, source.objects) { if (requested.contains(object.id)) { selected.append(object); } }
    if (selected.size() != ids.size()) { return editError(error, QStringLiteral("部分选中控件不属于当前画面。")); }
    *objects = selected;
    return true;
}

/******************************************************************************
 * Function Name   : validateObjectAddresses
 * Description     : Validate active addresses with each field's role and complete numeric transfer count.
 * Input Parameters: object, project, error
 * Return Value    : True when all active addresses are supported and span-safe.
 * Date            : 2026.11.10
 ******************************************************************************/
bool validateObjectAddresses(HmiObject object, const HmiProject &project, QString *error) //20261110-8 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress address;
    QList<AddressSlot> addressSlots;
    AddressSlot slot;

    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = project.connections;
    addressSlots = activeSlots(&object);
    Q_FOREACH (slot, addressSlots)
    {
        context.role = slot.role;
        context.transferCount = slot.count;
        if (!parseQualifiedPlcAddressText(*slot.value, context, catalog, &address, error))
        {
            return editError(error, QStringLiteral("控件%1的活动地址无效：%2").arg(object.id, error == nullptr ? QString() : *error));
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : matchingConnections
 * Description     : Reject implicit connection remapping across documents, including channel and protocol changes.
 * Input Parameters: left, right
 * Return Value    : True when both ordered connection contracts are identical.
 * Date            : 2026.11.10
 ******************************************************************************/
bool matchingConnections(const HmiProject &left, const HmiProject &right) //20261110-8 add
{
    int index;

    if (left.connections.size() != right.connections.size()) { return false; }
    for (index = 0; index < left.connections.size(); ++index)
    {
        if (left.connections.at(index).toJson() != right.connections.at(index).toJson()) { return false; }
    }
    return true;
}

/******************************************************************************
 * Function Name   : mergePicture
 * Description     : Reuse exact decoded image bytes or derive a deterministic collision-safe resource name.
 * Input Parameters: candidate, requestedName, bytes, resolvedName, error
 * Return Value    : True with an exact-content resource binding; equal pixels alone do not merge different bytes.
 * Date            : 2026.11.10
 ******************************************************************************/
bool mergePicture(HmiProject *candidate, const QString &requestedName, const QByteArray &bytes,
                  QString *resolvedName, QString *error) //20261110-8 add
{
    QMap<QString, QByteArray>::const_iterator picture;
    QString base;
    QString key;
    QString digest;
    int suffixLength;

    if (!validateButtonImage(bytes)) { return editError(error, QStringLiteral("剪贴板包含不能完整解码的图片，粘贴已取消。")); }
    for (picture = candidate->pictures.constBegin(); picture != candidate->pictures.constEnd(); ++picture)
    {
        if (picture.value() == bytes) { *resolvedName = picture.key(); return true; }
    }
    base = requestedName.trimmed().isEmpty() ? QStringLiteral("image") : requestedName;
    key = base;
    digest = QString::fromLatin1(QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex());
    suffixLength = 12;
    while (candidate->pictures.contains(key) && candidate->pictures.value(key) != bytes)
    {
        key = base + QStringLiteral("~") + digest.left(suffixLength);
        if (suffixLength == digest.size() && candidate->pictures.contains(key) && candidate->pictures.value(key) != bytes)
        {
            return editError(error, QStringLiteral("图片内容哈希与既有名称冲突，无法安全重命名。"));
        }
        suffixLength = qMin(suffixLength + 4, int(digest.size()));
    }
    candidate->pictures.insert(key, bytes);
    *resolvedName = key;
    return true;
}

/******************************************************************************
 * Function Name   : mergeStatePicture
 * Description     : Bind a state's effective embedded image by actual bytes instead of its possibly reused display name.
 * Input Parameters: candidate, state, error
 * Return Value    : True with embedded bytes preserved; gallery-only states retain their original gallery identity.
 * Date            : 2026.11.10
 ******************************************************************************/
bool mergeStatePicture(HmiProject *candidate, BitButtonState *state, QString *error) //20261110-8 add
{
    QString name;

    if (state->picturePng.isEmpty()) { return true; }
    if (!validateButtonImage(state->picturePng)) { return editError(error, QStringLiteral("控件内嵌图片损坏。")); }
    if (state->picture.isEmpty()) { return true; }
    if (!mergePicture(candidate, state->picture, state->picturePng, &name, error)) { return false; }
    state->picture = name;
    return true;
}

/******************************************************************************
 * Function Name   : mergeObjectPictures
 * Description     : Remap all shared and extra lamp state images through their typed fields.
 * Input Parameters: candidate, object, error
 * Return Value    : True without changing text, inactive address drafts or unknown extension metadata.
 * Date            : 2026.11.10
 ******************************************************************************/
bool mergeObjectPictures(HmiProject *candidate, HmiObject *object, QString *error) //20261110-8 add
{
    int index;
    QString resolved;
    if (object->type == QStringLiteral("multifunction")) //20261112-0 add
    {
        for (index = 0; index < 2; ++index)
        { if (!object->multiFunctionButton.common.states[index].picturePng.isEmpty()) { if (!mergePicture(candidate, object->multiFunctionButton.common.states[index].picture, object->multiFunctionButton.common.states[index].picturePng, &resolved, error)) { return false; } object->multiFunctionButton.common.states[index].picture = resolved; } }
    }
    if (object->type == QStringLiteral("multistate")) //20261112-0 add
    {
        for (index = 0; index < object->multiStateButton.stateCount; ++index)
        { if (!object->multiStateButton.state(index)->picturePng.isEmpty()) { if (!mergePicture(candidate, object->multiStateButton.state(index)->picture, object->multiStateButton.state(index)->picturePng, &resolved, error)) { return false; } object->multiStateButton.state(index)->picture = resolved; } }
    }
    if (object->type == QStringLiteral("screen")) //20261112-0 add
    {
        for (index = 0; index < 2; ++index)
        { if (!object->screenButton.common.states[index].picturePng.isEmpty()) { if (!mergePicture(candidate, object->screenButton.common.states[index].picture, object->screenButton.common.states[index].picturePng, &resolved, error)) { return false; } object->screenButton.common.states[index].picture = resolved; } }
    }
    if (object->type == QStringLiteral("track") && !object->moveTrack.appearance.picturePng.isEmpty()) //20261112-0 add: Preserve image identity during resource-conflict copies.
    {
        if (!mergePicture(candidate, object->moveTrack.appearance.picture, object->moveTrack.appearance.picturePng, &resolved, error)) { return false; }
        object->moveTrack.appearance.picture = resolved;
    }
    if (object->type == QStringLiteral("wheel") && !object->wheel.highlight.picturePng.isEmpty()) //20261112-0 add: Preserve exact highlight image identity during cross-project copies.
    {
        if (!mergePicture(candidate, object->wheel.highlight.picture, object->wheel.highlight.picturePng, &resolved, error)) { return false; }
        object->wheel.highlight.picture = resolved;
    }
    if (object->type == QStringLiteral("time") && !object->timeDisplay.appearance.picturePng.isEmpty()) //20261112-0 add
    {
        if (!mergePicture(candidate, object->timeDisplay.appearance.picture, object->timeDisplay.appearance.picturePng, &resolved, error)) { return false; }
        object->timeDisplay.appearance.picture = resolved;
    }
    if (object->type == QStringLiteral("ascii") && !object->asciiDisplay.common.appearance.picturePng.isEmpty()) //20261112-0 add
    {
        if (!mergePicture(candidate, object->asciiDisplay.common.appearance.picture, object->asciiDisplay.common.appearance.picturePng, &resolved, error)) { return false; }
        object->asciiDisplay.common.appearance.picture = resolved;
    }
    if (object->hasNumberDisplayProperties && !object->numberDisplay.appearance.picturePng.isEmpty()) //20261112-0 add
    {
        if (!mergePicture(candidate, object->numberDisplay.appearance.picture, object->numberDisplay.appearance.picturePng, &resolved, error)) { return false; }
        object->numberDisplay.appearance.picture = resolved;
    }

    if (object->type == QStringLiteral("gif")) //20261112-0 add: Cross-project paste binds original animation bytes independently of PNG states.
    {
        if (!mergeAnimation(candidate, object->gifDisplay.animation, object->gifDisplay.gifData, &resolved, error)) { return false; }
        object->gifDisplay.animation = resolved;
    }

    if (object->hasImageDisplayProperties) //20261112-0 add: Remap exact-content picture bindings before cloning image states.
    {
        for (index = 0; index < object->imageDisplay.states.size(); ++index)
        {
            if (object->imageDisplay.states[index].picturePng.isEmpty()) { continue; }
            if (!mergePicture(candidate, object->imageDisplay.states[index].picture, object->imageDisplay.states[index].picturePng, &resolved, error)) { return false; }
            object->imageDisplay.states[index].picture = resolved;
        }
    }
    if (!object->hasBitButtonProperties) { return true; }
    for (index = 0; index < 2; ++index)
    {
        if (!mergeStatePicture(candidate, &object->bitButton.states[index], error)) { return false; }
    }
    if (object->hasLampProperties)
    {
        for (index = 0; index < object->lamp.extraStates.size(); ++index)
        {
            if (!mergeStatePicture(candidate, &object->lamp.extraStates[index], error)) { return false; }
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : mergeMacro
 * Description     : Preserve macro source and reconcile stable identity only when the same name has the same source.
 * Input Parameters: source, candidate, name, mappedIds, error
 * Return Value    : True after a safe merge; same-name different-source conflicts fail explicitly.
 * Date            : 2026.11.10
 ******************************************************************************/
bool mergeMacro(const HmiProject &source, HmiProject *candidate, const QString &name,
                 QMap<QString, QString> *mappedIds, QString *error) //20261110-8 add
{
    QString oldId;
    QString newId;

    if (!source.macros.contains(name)) { return editError(error, QStringLiteral("剪贴板引用的宏%1不存在。").arg(name)); }
    if (candidate->macros.contains(name) && candidate->macros.value(name) != source.macros.value(name))
    {
        return editError(error, QStringLiteral("宏%1同名但源码不同，粘贴已取消，未覆盖既有宏。").arg(name));
    }
    candidate->macros.insert(name, source.macros.value(name));
    oldId = source.macroIds.value(name);
    if (oldId.isEmpty()) { return true; }
    newId = candidate->macroIds.value(name);
    if (newId.isEmpty())
    {
        newId = oldId;
        if (candidate->macroIds.values().contains(newId)) { newId = QUuid::createUuid().toString(QUuid::WithoutBraces); }
        candidate->macroIds.insert(name, newId);
    }
    mappedIds->insert(QStringLiteral("macro/") + oldId, newId);
    return true;
}

/******************************************************************************
 * Function Name   : entityJson
 * Description     : Resolve an unchanged external dependency without treating equal row numbers as stable identities.
 * Input Parameters: project, kind, id, value
 * Return Value    : True only for an existing supported entity.
 * Date            : 2026.11.10
 ******************************************************************************/
bool entityJson(const HmiProject &project, const QString &kind, const QString &id, QJsonObject *value) //20261110-8 add
{
    HmiPage page;
    HmiObject object;
    HmiConfigurationNode node;
    PlcConnection connection;

    if (kind == QStringLiteral("project") && id == project.projectId) { *value = project.toJson(); return true; }
    Q_FOREACH (page, project.allPages())
    {
        if (kind == QStringLiteral("page") && page.id == id) { *value = page.toJson(); return true; }
        Q_FOREACH (object, page.objects)
        {
            if (kind == QStringLiteral("object") && object.id == id) { *value = object.toJson(); return true; }
        }
    }
    Q_FOREACH (node, project.configurationNodes)
    {
        if (kind == QStringLiteral("configuration") && node.id == id) { *value = node.toJson(); return true; }
    }
    Q_FOREACH (connection, project.connections)
    {
        if (kind == QStringLiteral("connection") && connection.id == id) { *value = connection.toJson(); return true; }
    }
    return false;
}

/******************************************************************************
 * Function Name   : mergeReferenceTarget
 * Description     : Remap selected objects and resources, or require an exact existing external dependency.
 * Input Parameters: source, target, candidate, reference, mappedIds, error
 * Return Value    : True for a fully resolved reference; missing pages/configurations are never invented.
 * Date            : 2026.11.10
 ******************************************************************************/
bool mergeReferenceTarget(const HmiProject &source, const HmiProject &target, HmiProject *candidate,
                           HmiReference *reference, QMap<QString, QString> *mappedIds, QString *error) //20261110-8 add
{
    QString key;
    QString name;
    QJsonObject sourceEntity;
    QJsonObject targetEntity;

    key = reference->targetKind + QLatin1Char('/') + reference->targetId;
    if (mappedIds->contains(key)) { reference->targetId = mappedIds->value(key); return true; }
    if (reference->targetKind == QStringLiteral("animation")) //20261112-0 add
    {
        if (!source.animations.contains(reference->targetId) || !mergeAnimation(candidate, reference->targetId, source.animations.value(reference->targetId), &name, error)) { return false; }
        mappedIds->insert(key, name); reference->targetId = name; return true;
    }
    if (reference->targetKind == QStringLiteral("picture"))
    {
        if (!source.pictures.contains(reference->targetId) ||
            !mergePicture(candidate, reference->targetId, source.pictures.value(reference->targetId), &name, error)) { return false; }
        mappedIds->insert(key, name);
        reference->targetId = name;
        return true;
    }
    if (reference->targetKind == QStringLiteral("macro"))
    {
        name = source.macroIds.key(reference->targetId);
        if (name.isEmpty() || !mergeMacro(source, candidate, name, mappedIds, error)) { return false; }
        reference->targetId = mappedIds->value(key);
        return !reference->targetId.isEmpty();
    }
    if (!entityJson(source, reference->targetKind, reference->targetId, &sourceEntity) ||
        !entityJson(target, reference->targetKind, reference->targetId, &targetEntity) || sourceEntity != targetEntity)
    {
        return editError(error, QStringLiteral("引用%1的外部依赖%2/%3在目标工程缺失或不同，粘贴已取消。").arg(reference->id, reference->targetKind, reference->targetId));
    }
    return true;
}

/******************************************************************************
 * Function Name   : appendSelection
 * Description     : Append one clone set on a detached project, remapping identities and complete owned references.
 * Input Parameters: target, clipboard, originals, offsetX, offsetY, candidate, createdIds, error
 * Return Value    : True after one complete clone set has been staged; caller owns final validation/publication.
 * Date            : 2026.11.10
 ******************************************************************************/
bool appendSelection(const HmiProject &target, const Stage3Clipboard &clipboard, const QList<HmiObject> &originals,
                       qint64 offsetX, qint64 offsetY, HmiProject *candidate, QStringList *createdIds, QString *error) //20261110-8 add
{
    QMap<QString, QString> mappedIds;
    QMap<QString, QString> groups;
    QSet<QString> selected;
    QList<HmiObject> copies;
    HmiObject object;
    HmiReference reference;
    QString id;
    qint64 x;
    qint64 y;

    Q_FOREACH (object, originals)
    {
        selected.insert(object.id);
        id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        mappedIds.insert(QStringLiteral("object/") + object.id, id);
        object.id = id;
        x = qint64(object.geometry.x()) + offsetX;
        y = qint64(object.geometry.y()) + offsetY;
        if (x < 0 || y < 0 || x + object.geometry.width() > target.pageWidth || y + object.geometry.height() > target.pageHeight)
        {
            return editError(error, QStringLiteral("复制后控件超出目标画面边界，整个事务已取消。"));
        }
        object.geometry.moveTo(int(x), int(y));
        if (!object.group.isEmpty())
        {
            if (!groups.contains(object.group)) { groups.insert(object.group, QUuid::createUuid().toString(QUuid::WithoutBraces)); }
            object.group = groups.value(object.group);
        }
        if (object.type == QStringLiteral("alarmbar") || object.type == QStringLiteral("alarmdisplay")) //20261113-2 add: Copy global alarm definitions only into a compatible project and validate navigation identities.
        {
            HmiConfigurationNode required;
            HmiConfigurationNode peer;
            AlarmRegistrationProperties alarms;
            AlarmRegistrationProperties peerAlarms;
            AlarmRegistrationRow row;
            QString pageId;
            bool reused;
            Q_FOREACH (required, clipboard.source.configurationNodes)
            {
                if (required.kind != QStringLiteral("alarm") || !required.active || !required.implemented) { continue; }
                if (!AlarmRegistrationProperties::fromJson(required.properties, &alarms, error)) { return false; }
                reused = false;
                Q_FOREACH (peer, candidate->configurationNodes)
                {
                    if (peer.id == required.id && peer.toJson() == required.toJson()) { reused = true; }
                    else if (peer.id == required.id || (peer.kind == QStringLiteral("alarm") && peer.active && peer.implemented && AlarmRegistrationProperties::fromJson(peer.properties, &peerAlarms) && peerAlarms.family == alarms.family)) { return editError(error, QStringLiteral("目标工程已有不同的报警登录，粘贴已取消。")); }
                }
                if (alarms.family == 0) { Q_FOREACH (row, alarms.rows) { if (!row.used) { continue; } Q_FOREACH (pageId, (QStringList{row.jumpPageId, row.backPageId})) { if (pageId.isEmpty()) { continue; } reference = HmiReference(); reference.id = QStringLiteral("alarm-navigation"); reference.targetKind = QStringLiteral("page"); reference.targetId = pageId; if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; } } } }
                if (!reused) { candidate->configurationNodes.append(required); }
            }
        }
        if (object.type == QStringLiteral("historytrend")) //20261112-1 add: Reuse the exact collection or copy it only into an empty target; never overwrite a conflicting collector.
        {
            HmiConfigurationNode required;
            HmiConfigurationNode peer;
            bool found = false;
            bool reused = false;
            Q_FOREACH (peer, clipboard.source.configurationNodes) { if (peer.id == object.historyTrend.collectionId && peer.kind == QStringLiteral("dataCollection") && peer.active && peer.implemented) { required = peer; found = true; break; } }
            if (!found) { return editError(error, QStringLiteral("剪贴板缺少历史趋势图的数据采集定义。")); }
            Q_FOREACH (peer, candidate->configurationNodes)
            {
                if (peer.id == required.id && peer.toJson() == required.toJson()) { reused = true; }
                else if (peer.id == required.id || (peer.kind == QStringLiteral("dataCollection") && peer.active && peer.implemented)) { return editError(error, QStringLiteral("目标工程已有不同的数据采集，粘贴已取消。")); }
            }
            if (!reused) { candidate->configurationNodes.append(required); }
        }
        if (object.type == QStringLiteral("slider") && !object.slider.macroName.isEmpty() && clipboard.source.macros.contains(object.slider.macroName)) { if (!mergeMacro(clipboard.source, candidate, object.slider.macroName, &mappedIds, error)) { return false; } } //20261112-0 add
        if (object.type == QStringLiteral("dropdown") && !object.dropdown.macroName.isEmpty() && clipboard.source.macros.contains(object.dropdown.macroName)) { if (!mergeMacro(clipboard.source, candidate, object.dropdown.macroName, &mappedIds, error)) { return false; } } //20261112-0 add
        if (object.hasBitButtonProperties && !object.bitButton.macroName.isEmpty() && clipboard.source.macros.contains(object.bitButton.macroName))
        {
            if (!mergeMacro(clipboard.source, candidate, object.bitButton.macroName, &mappedIds, error)) { return false; }
        }
        if (object.type == QStringLiteral("multifunction")) //20261112-0 add: Cross-document paste requires exact dependency identity/content for every navigation command.
        {
            for (MultiFunctionCommand &command : object.multiFunctionButton.commands)
            {
                if (command.activeTarget().isEmpty()) { continue; }
                reference = HmiReference(); reference.id = QStringLiteral("mf-navigation"); reference.targetKind = QStringLiteral("page"); reference.targetId = command.activeTarget();
                if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; }
                if (command.operation == 0) { command.screenPageId = reference.targetId; } else { command.windowPageId = reference.targetId; }
            }
        }
        if (object.type == QStringLiteral("screen") && !object.screenButton.activeTarget().isEmpty()) //20261112-0 add
        {
            reference = HmiReference(); reference.id = QStringLiteral("screen-navigation"); reference.targetKind = QStringLiteral("page"); reference.targetId = object.screenButton.activeTarget();
            if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; }
            if (object.screenButton.action == 0) { object.screenButton.screenPageId = reference.targetId; } else { object.screenButton.windowPageId = reference.targetId; }
        }
        if ((object.hasNumberDisplayProperties && object.numberDisplay.allowInput && object.numberDisplay.customKeyboard) ||
            (object.type == QStringLiteral("ascii") && object.asciiDisplay.common.allowInput && object.asciiDisplay.common.customKeyboard)) //20261112-0 change: A keyboard ID cannot silently bind to a different target document's page.
        {
            reference = HmiReference(); reference.id = QStringLiteral("value-keyboard"); reference.targetKind = QStringLiteral("page"); reference.targetId = object.type == QStringLiteral("ascii") ? object.asciiDisplay.common.keyboardPageId : object.numberDisplay.keyboardPageId;
            if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; }
        }
        if (!mergeObjectPictures(candidate, &object, error) || !validateObjectAddresses(object, *candidate, error)) { return false; }
        copies.append(object);
        createdIds->append(object.id);
    }
    Q_FOREACH (reference, clipboard.source.references)
    {
        if (reference.ownerKind != QStringLiteral("object") || !selected.contains(reference.ownerId)) { continue; }
        reference.id = QUuid::createUuid().toString(QUuid::WithoutBraces);
        reference.ownerId = mappedIds.value(QStringLiteral("object/") + reference.ownerId);
        if (!mergeReferenceTarget(clipboard.source, target, candidate, &reference, &mappedIds, error)) { return false; }
        candidate->references.append(reference);
    }
    candidate->objects.append(copies);
    return true;
}
}

/******************************************************************************
 * Function Name   : captureStage3Selection
 * Description     : Capture complete selected model values and their source resource/dependency contracts in memory.
 * Input Parameters: source, objectIds, clipboard, error
 * Return Value    : True on atomic capture; no operating-system clipboard or external file is touched.
 * Date            : 2026.11.10
 ******************************************************************************/
bool captureStage3Selection(const HmiProject &source, const QStringList &objectIds,
                            Stage3Clipboard *clipboard, QString *error) //20261110-8 add
{
    Stage3Clipboard candidate;
    QList<HmiObject> objects;
    HmiObject object;

    if (clipboard == nullptr || !selectedObjects(source, objectIds, &objects, error)) { return false; }
    Q_FOREACH (object, objects)
    {
        if (!validateObjectAddresses(object, source, error)) { return false; }
        candidate.objectIds.append(object.id);
    }
    candidate.source = source;
    *clipboard = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : pasteStage3Selection
 * Description     : Paste a full selected object set with typed resources and references as one project transaction.
 * Input Parameters: target, clipboard, offset, draft, newIds, error
 * Return Value    : True only after the entire candidate validates; every failure leaves both outputs untouched.
 * Date            : 2026.11.10
 ******************************************************************************/
bool pasteStage3Selection(const HmiProject &target, const Stage3Clipboard &clipboard,
                          const QPoint &offset, HmiProject *draft, QStringList *newIds, QString *error) //20261110-8 add
{
    HmiProject candidate;
    QList<HmiObject> objects;
    QStringList ids;

    if (draft == nullptr || !target.validateModel(error) || !selectedObjects(clipboard.source, clipboard.objectIds, &objects, error)) { return false; }
    if (target.languages != clipboard.source.languages || !matchingConnections(target, clipboard.source))
    {
        return editError(error, QStringLiteral("源与目标的语言顺序或PLC连接不一致；当前粘贴不猜测语言和外部协议映射。"));
    }
    if (target.objects.size() + objects.size() > 100) { return editError(error, QStringLiteral("粘贴后超过当前画面100控件保护上限。")); }
    candidate = target;
    if (!appendSelection(target, clipboard, objects, offset.x(), offset.y(), &candidate, &ids, error) || !candidate.validateModel(error)) { return false; }
    *draft = candidate;
    if (newIds != nullptr) { *newIds = ids; }
    return true;
}

/******************************************************************************
 * Function Name   : deleteStage3Selection
 * Description     : Remove selected objects and their owned references after checking every inbound dependency.
 * Input Parameters: source, objectIds, draft, error
 * Return Value    : True on atomic removal; shared resource pools remain available for history and other consumers.
 * Date            : 2026.11.10
 ******************************************************************************/
bool deleteStage3Selection(const HmiProject &source, const QStringList &objectIds, HmiProject *draft, QString *error) //20261110-8 add
{
    HmiProject candidate;
    QList<HmiObject> objects;
    QSet<QString> ids;
    HmiObject object;
    HmiReference reference;
    int index;

    if (draft == nullptr || !selectedObjects(source, objectIds, &objects, error)) { return false; }
    Q_FOREACH (object, objects) { ids.insert(object.id); }
    Q_FOREACH (reference, source.references)
    {
        if (reference.targetKind == QStringLiteral("object") && ids.contains(reference.targetId) &&
            !(reference.ownerKind == QStringLiteral("object") && ids.contains(reference.ownerId)))
        {
            return editError(error, QStringLiteral("控件仍被引用%1使用，删除或剪切已取消。").arg(reference.id));
        }
    }
    candidate = source;
    for (index = candidate.objects.size() - 1; index >= 0; --index)
    {
        if (ids.contains(candidate.objects.at(index).id)) { candidate.objects.removeAt(index); }
    }
    for (index = candidate.references.size() - 1; index >= 0; --index)
    {
        reference = candidate.references.at(index);
        if (reference.ownerKind == QStringLiteral("object") && ids.contains(reference.ownerId)) { candidate.references.removeAt(index); }
    }
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    return true;
}

/******************************************************************************
 * Function Name   : cutStage3Selection
 * Description     : Publish the clipboard capture and complete project removal only when both succeed.
 * Input Parameters: source, objectIds, clipboard, draft, error
 * Return Value    : True after an atomic two-output transaction.
 * Date            : 2026.11.10
 ******************************************************************************/
bool cutStage3Selection(const HmiProject &source, const QStringList &objectIds,
                        Stage3Clipboard *clipboard, HmiProject *draft, QString *error) //20261110-8 add
{
    Stage3Clipboard captured;
    HmiProject candidate;

    if (clipboard == nullptr || draft == nullptr || !captureStage3Selection(source, objectIds, &captured, error) ||
        !deleteStage3Selection(source, objectIds, &candidate, error)) { return false; }
    *clipboard = captured;
    *draft = candidate;
    return true;
}

namespace
{
/******************************************************************************
 * Function Name   : incrementAddress
 * Description     : Increment a verified scalar address component and revalidate its complete transfer span.
 * Input Parameters: value, role, count, delta, project, error
 * Return Value    : True with a canonical shifted address; bank/bit composite grammars are not guessed.
 * Date            : 2026.11.10
 ******************************************************************************/
bool incrementAddress(QString *value, PlcAddressRole role, u32 count, qint64 delta,
                        const HmiProject &project, QString *error) //20261110-8 add
{
    PlcCatalog catalog;
    PlcAddressContext context;
    PlcAddress parsed;
    PlcAddress shifted;
    PlcNativeFamilyRule rule;
    const PlcConnection *connection;
    const PlcProfile *profile;
    qint64 number;
    qint64 result;
    int radix;
    bool valid;

    if (delta == 0) { return true; }
    if (!catalog.load(QStringLiteral(":/plc/plc-catalog.json"), error)) { return false; }
    context.connections = project.connections;
    context.role = role;
    context.transferCount = count;
    if (!parseQualifiedPlcAddressText(*value, context, catalog, &parsed, error)) { return false; }
    radix = 10;
    if (parsed.space == PlcAddressSpace::External)
    {
        connection = plcConnection(context, parsed.connectionId);
        profile = connection == nullptr ? nullptr : catalog.profile(connection->profileId);
        if (profile == nullptr) { return editError(error, QStringLiteral("递增地址的外部连接不存在。")); }
        if (!plcProfileHasModbusCodec(*profile))
        {
            if (!plcNativeFamilyRule(*profile, parsed.family, &rule, error) ||
                rule.grammar != PlcNativeGrammar::Integer || rule.componentCount != 1)
            {
                return editError(error, QStringLiteral("此地址的分段、位或其他特殊递增语法尚未验证，复制已取消。"));
            }
            radix = rule.majorRadix;
        }
    }
    number = parsed.value.toLongLong(&valid, radix);
    if (!valid || number < 0) { return editError(error, QStringLiteral("此地址不是已验证的单一整数递增格式。")); }
    result = number + delta;
    if (result < 0 || result > std::numeric_limits<u32>::max())
    {
        return editError(error, QStringLiteral("递增后的地址数值溢出，整个复制事务已取消。"));
    }
    if (!parsePlcAddress(parsed.family, QString::number(result, radix).toUpper(), parsed.connectionId,
                         context, catalog, &shifted, error)) { return false; }
    *value = plcQualifiedAddressText(shifted);
    return true;
}

/******************************************************************************
 * Function Name   : validateIncrements
 * Description     : Reject duplicate/unavailable address roles before constructing any grid cell.
 * Input Parameters: objects, increments, error
 * Return Value    : True for explicit independent rules over active fields only.
 * Date            : 2026.11.10
 ******************************************************************************/
bool validateIncrements(const QList<HmiObject> &objects, const QList<Stage3AddressIncrement> &increments, QString *error) //20261110-8 add
{
    QSet<int> fields;
    Stage3AddressIncrement increment;
    HmiObject object;
    AddressSlot slot;
    bool found;

    Q_FOREACH (increment, increments)
    {
        if (int(increment.field) < int(Stage3AddressField::Operation) || int(increment.field) > int(Stage3AddressField::SamplingWord) ||
            fields.contains(int(increment.field)))
        {
            return editError(error, QStringLiteral("递增规则包含未知或重复的地址角色。"));
        }
        fields.insert(int(increment.field));
        if (increment.columnStep == 0 && increment.rowStep == 0) { continue; }
        found = false;
        Q_FOREACH (object, objects)
        {
            Q_FOREACH (slot, activeSlots(&object)) { if (slot.field == increment.field) { found = true; } }
        }
        if (!found) { return editError(error, QStringLiteral("所选控件没有启用指定的地址角色；未改动其禁用草稿。")); }
    }
    return true;
}

/******************************************************************************
 * Function Name   : shiftedObjects
 * Description     : Apply only requested active address roles to one detached grid cell.
 * Input Parameters: source, objects, increments, row, column, shifted, error
 * Return Value    : True with complete per-object drafts, including every unchanged inactive field.
 * Date            : 2026.11.10
 ******************************************************************************/
bool shiftedObjects(const HmiProject &source, const QList<HmiObject> &objects,
                      const QList<Stage3AddressIncrement> &increments, int row, int column,
                      QList<HmiObject> *shifted, QString *error) //20261110-8 add
{
    QList<HmiObject> candidate;
    QList<AddressSlot> addressSlots;
    HmiObject object;
    AddressSlot slot;
    Stage3AddressIncrement increment;
    qint64 delta;

    Q_FOREACH (object, objects)
    {
        addressSlots = activeSlots(&object);
        Q_FOREACH (increment, increments)
        {
            delta = qint64(column) * increment.columnStep + qint64(row) * increment.rowStep;
            Q_FOREACH (slot, addressSlots)
            {
                if (slot.field == increment.field && !incrementAddress(slot.value, slot.role, slot.count, delta, source, error)) { return false; }
            }
        }
        candidate.append(object);
    }
    *shifted = candidate;
    return true;
}
}

/******************************************************************************
 * Function Name   : duplicateStage3Selection
 * Description     : Stage the complete two-dimensional clone grid while retaining its original cell unchanged.
 * Input Parameters: source, objectIds, options, draft, newIds, error
 * Return Value    : True on one complete project transaction; no geometry/address/resource failure partially publishes.
 * Date            : 2026.11.10
 ******************************************************************************/
bool duplicateStage3Selection(const HmiProject &source, const QStringList &objectIds,
                              const Stage3DuplicateOptions &options, HmiProject *draft,
                              QStringList *newIds, QString *error) //20261110-8 add
{
    HmiProject candidate;
    Stage3Clipboard clipboard;
    QList<HmiObject> objects;
    QList<HmiObject> shifted;
    QStringList ids;
    qint64 additionalCount;
    int row;
    int column;

    if (draft == nullptr || options.rows < 1 || options.rows > 99 || options.columns < 1 || options.columns > 99)
    {
        return editError(error, QStringLiteral("行列总数必须为1..99，且包含原对象所在的首格。"));
    }
    if (!captureStage3Selection(source, objectIds, &clipboard, error) || !selectedObjects(source, objectIds, &objects, error) ||
        !validateIncrements(objects, options.increments, error)) { return false; }
    additionalCount = (qint64(options.rows) * options.columns - 1) * objects.size();
    if (source.objects.size() + additionalCount > 100)
    {
        return editError(error, QStringLiteral("整个复制网格将超过当前画面100控件保护上限。"));
    }
    candidate = source;
    for (row = 0; row < options.rows; ++row)
    {
        for (column = 0; column < options.columns; ++column)
        {
            if (row == 0 && column == 0) { continue; }
            if (!shiftedObjects(source, objects, options.increments, row, column, &shifted, error) ||
                !appendSelection(source, clipboard, shifted, qint64(column) * options.offsetX,
                                 qint64(row) * options.offsetY, &candidate, &ids, error)) { return false; }
        }
    }
    if (!candidate.validateModel(error)) { return false; }
    *draft = candidate;
    if (newIds != nullptr) { *newIds = ids; }
    return true;
}

/******************************************************************************
 * Function Name   : Stage3DuplicateDialog::Stage3DuplicateDialog
 * Description     : Preview an explicit coordinate-step grid without creating history or changing the input project.
 * Input Parameters: project, objectIds, parent
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
Stage3DuplicateDialog::Stage3DuplicateDialog(const HmiProject &project, const QStringList &objectIds,
                                             QWidget *parent) //20261110-8 add
    : QDialog(parent), m_source(project), m_working(project), m_preview(project), m_selection(objectIds)
{
    QVBoxLayout *layout;
    QFormLayout *form;
    QFormLayout *nativeForm;
    QFormLayout *advancedForm;
    QLabel *policy;

    setObjectName(QStringLiteral("stage3DuplicateDialog"));
    setWindowTitle(QStringLiteral("多重复制"));
    resize(690, 540);
    layout = new QVBoxLayout(this);
    form = new QFormLayout;
    m_columns = new QSpinBox(this);
    m_columns->setObjectName(QStringLiteral("duplicateColumnsSpin"));
    m_columns->setRange(1, 99);
    m_columns->setValue(5);
    m_rows = new QSpinBox(this);
    m_rows->setObjectName(QStringLiteral("duplicateRowsSpin"));
    m_rows->setRange(1, 99);
    m_rows->setValue(5);
    m_offsetX = new QSpinBox(this);
    m_offsetX->setObjectName(QStringLiteral("duplicateOffsetXSpin"));
    m_offsetX->setRange(0, 800);
    m_offsetX->setValue(10);
    m_offsetY = new QSpinBox(this);
    m_offsetY->setObjectName(QStringLiteral("duplicateOffsetYSpin"));
    m_offsetY->setRange(0, 480);
    m_offsetY->setValue(10);
    m_incrementEnabled = new QCheckBox(QStringLiteral("地址自动递增"), this);
    m_incrementEnabled->setObjectName(QStringLiteral("duplicateAddressIncrementCheck"));
    m_incrementEnabled->setChecked(false);
    m_incrementMode = new QComboBox(this);
    m_incrementMode->setObjectName(QStringLiteral("duplicateIncrementModeCombo"));
    m_incrementMode->addItem(QStringLiteral("原厂已验证的操作／独立监视地址"), 0);
    m_incrementMode->addItem(QStringLiteral("高级：明确指定地址角色"), 1);
    m_nativeIncrementGroup = new QGroupBox(QStringLiteral("地址递增"), this);
    m_nativeIncrementGroup->setObjectName(QStringLiteral("duplicateNativeIncrementGroup"));
    nativeForm = new QFormLayout(m_nativeIncrementGroup);
    m_incrementDirection = new QComboBox(m_nativeIncrementGroup);
    m_incrementDirection->setObjectName(QStringLiteral("duplicateAddressDirectionCombo"));
    m_incrementDirection->addItem(QStringLiteral("向右递增"), 0);
    m_incrementDirection->addItem(QStringLiteral("向下递增"), 1);
    m_incrementStep = new QSpinBox(m_nativeIncrementGroup);
    m_incrementStep->setObjectName(QStringLiteral("duplicateAddressIncrementSpin"));
    m_incrementStep->setRange(0, 256);
    m_incrementStep->setValue(1);
    nativeForm->addRow(QStringLiteral("方向"), m_incrementDirection);
    nativeForm->addRow(QStringLiteral("递增间隔"), m_incrementStep);
    m_advancedIncrementGroup = new QGroupBox(QStringLiteral("高级地址角色（产品扩展）"), this);
    m_advancedIncrementGroup->setObjectName(QStringLiteral("duplicateAdvancedIncrementGroup"));
    advancedForm = new QFormLayout(m_advancedIncrementGroup);
    m_addressField = new QComboBox(m_advancedIncrementGroup);
    m_addressField->setObjectName(QStringLiteral("duplicateAddressFieldCombo"));
    m_addressField->addItem(QStringLiteral("地址保持不变"), -1);
    m_addressField->addItem(QStringLiteral("操作地址"), int(Stage3AddressField::Operation));
    m_addressField->addItem(QStringLiteral("位监视地址"), int(Stage3AddressField::MonitorBit));
    m_addressField->addItem(QStringLiteral("字监视地址"), int(Stage3AddressField::MonitorWord));
    m_addressField->addItem(QStringLiteral("多态灯监视地址"), int(Stage3AddressField::LampMultiMonitor));
    m_addressField->addItem(QStringLiteral("位通知地址"), int(Stage3AddressField::NoticeBit));
    m_addressField->addItem(QStringLiteral("字通知地址"), int(Stage3AddressField::NoticeWord));
    m_addressField->addItem(QStringLiteral("显现位地址"), int(Stage3AddressField::VisibilityBit));
    m_addressField->addItem(QStringLiteral("显现字地址"), int(Stage3AddressField::VisibilityWord));
    m_addressField->addItem(QStringLiteral("触控位地址"), int(Stage3AddressField::TouchBit));
    m_addressField->addItem(QStringLiteral("触控字地址"), int(Stage3AddressField::TouchWord));
    m_addressField->addItem(QStringLiteral("偏移字地址"), int(Stage3AddressField::OffsetWord)); //20261112-0 add
    m_addressField->addItem(QStringLiteral("输入下限地址"), int(Stage3AddressField::InputMinimumWord));
    m_addressField->addItem(QStringLiteral("输入上限地址"), int(Stage3AddressField::InputMaximumWord));
    m_addressField->addItem(QStringLiteral("显示下限地址"), int(Stage3AddressField::DisplayMinimumWord));
    m_addressField->addItem(QStringLiteral("显示上限地址"), int(Stage3AddressField::DisplayMaximumWord));
    m_addressField->addItem(QStringLiteral("采样触发位地址"), int(Stage3AddressField::TriggerBit)); //20261112-1 add
    m_addressField->addItem(QStringLiteral("采样清除位地址"), int(Stage3AddressField::ClearBit));
    m_addressField->addItem(QStringLiteral("数据群组采样地址"), int(Stage3AddressField::SamplingWord)); //20261113-1 add
    m_columnAddressStep = new QSpinBox(m_advancedIncrementGroup);
    m_columnAddressStep->setObjectName(QStringLiteral("duplicateColumnAddressStepSpin"));
    m_columnAddressStep->setRange(-256, 256);
    m_rowAddressStep = new QSpinBox(m_advancedIncrementGroup);
    m_rowAddressStep->setObjectName(QStringLiteral("duplicateRowAddressStepSpin"));
    m_rowAddressStep->setRange(-256, 256);
    form->addRow(QStringLiteral("列数（含原对象）"), m_columns);
    form->addRow(QStringLiteral("行数（含原对象）"), m_rows);
    form->addRow(QStringLiteral("列边缘间隔"), m_offsetX);
    form->addRow(QStringLiteral("行边缘间隔"), m_offsetY);
    form->addRow(m_incrementEnabled);
    form->addRow(QStringLiteral("递增模式"), m_incrementMode);
    advancedForm->addRow(QStringLiteral("明确递增的地址角色"), m_addressField);
    advancedForm->addRow(QStringLiteral("每列地址增量"), m_columnAddressStep);
    advancedForm->addRow(QStringLiteral("每行地址增量"), m_rowAddressStep);
    layout->addLayout(form);
    layout->addWidget(m_nativeIncrementGroup);
    layout->addWidget(m_advancedIncrementGroup);
    policy = new QLabel(QStringLiteral("行列包含原对象，按选区范围加边缘间隔排列。自动递增默认关闭；原厂模式仅递增操作和独立监视地址，通知、显现、触控地址保持不变。多选复制和高级地址角色为产品扩展。"), this);
    policy->setObjectName(QStringLiteral("duplicatePolicyLabel"));
    policy->setWordWrap(true);
    layout->addWidget(policy);
    m_summary = new QLabel(this);
    m_summary->setObjectName(QStringLiteral("duplicatePreviewSummary"));
    layout->addWidget(m_summary);
    m_table = new QTableWidget(0, 7, this);
    m_table->setObjectName(QStringLiteral("duplicatePreviewTable"));
    m_table->setHorizontalHeaderLabels({QStringLiteral("名称"), QStringLiteral("X"), QStringLiteral("Y"),
                                      QStringLiteral("宽"), QStringLiteral("高"), QStringLiteral("操作地址"), QStringLiteral("ID")});
    m_table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_table->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    layout->addWidget(m_table);
    m_error = new QLabel(this);
    m_error->setObjectName(QStringLiteral("validationErrorLabel"));
    m_error->setWordWrap(true);
    layout->addWidget(m_error);
    m_buttons = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    m_buttons->setObjectName(QStringLiteral("buttonBox"));
    layout->addWidget(m_buttons);
    connect(m_buttons, &QDialogButtonBox::accepted, this, &Stage3DuplicateDialog::accept);
    connect(m_buttons, &QDialogButtonBox::rejected, this, &QDialog::reject);
    connect(m_rows, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    connect(m_columns, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    connect(m_offsetX, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    connect(m_offsetY, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    connect(m_incrementEnabled, &QCheckBox::toggled, this, [this](bool) { refreshPreview(); });
    connect(m_incrementMode, &QComboBox::currentIndexChanged, this, [this](int) { refreshPreview(); });
    connect(m_incrementDirection, &QComboBox::currentIndexChanged, this, [this](int) { refreshPreview(); });
    connect(m_incrementStep, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    connect(m_addressField, &QComboBox::currentIndexChanged, this, [this](int) { refreshPreview(); });
    connect(m_columnAddressStep, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    connect(m_rowAddressStep, &QSpinBox::valueChanged, this, [this](int) { refreshPreview(); });
    refreshPreview();
}

/******************************************************************************
 * Function Name   : Stage3DuplicateDialog::draftProject
 * Description     : Return the committed detached project only after Accepted; preview never changes this value.
 * Input Parameters: None
 * Return Value    : Accepted project or unchanged input before acceptance.
 * Date            : 2026.11.10
 ******************************************************************************/
HmiProject Stage3DuplicateDialog::draftProject() const //20261110-8 add
{
    return m_working;
}

/******************************************************************************
 * Function Name   : Stage3DuplicateDialog::newObjectIds
 * Description     : Return only committed new IDs for the caller's selection and single history transaction.
 * Input Parameters: None
 * Return Value    : Accepted new object IDs, empty during preview or after cancellation.
 * Date            : 2026.11.10
 ******************************************************************************/
QStringList Stage3DuplicateDialog::newObjectIds() const //20261110-8 add
{
    return m_newIds;
}

/******************************************************************************
 * Function Name   : Stage3DuplicateDialog::refreshPreview
 * Description     : Rebuild a complete provisional grid and expose its geometry without touching accepted state.
 * Input Parameters: None
 * Return Value    : True for a valid preview; invalid input clears stale preview rows and disables acceptance.
 * Date            : 2026.11.10
 ******************************************************************************/
bool Stage3DuplicateDialog::refreshPreview() //20261110-8 add
{
    Stage3DuplicateOptions options;
    Stage3AddressIncrement increment;
    HmiProject candidate;
    QStringList ids;
    QString error;
    HmiObject object;
    QStringList values;
    QRect bounds;
    QSet<int> availableFields;
    AddressSlot addressSlot;
    int row;
    int column;
    bool valid;
    bool nativeMode;
    bool incrementEnabled;

    options.rows = m_rows->value();
    options.columns = m_columns->value();
    Q_FOREACH (object, m_source.objects)
    {
        if (m_selection.contains(object.id))
        {
            bounds = bounds.isNull() ? object.geometry : bounds.united(object.geometry);
            Q_FOREACH (addressSlot, activeSlots(&object)) { availableFields.insert(int(addressSlot.field)); }
        }
    }
    // The UI exposes observed edge gaps; the transaction API receives explicit coordinate steps.
    options.offsetX = bounds.width() + m_offsetX->value();
    options.offsetY = bounds.height() + m_offsetY->value();
    incrementEnabled = m_incrementEnabled->isChecked();
    nativeMode = m_incrementMode->currentData().toInt() == 0;
    m_incrementMode->setEnabled(incrementEnabled);
    m_nativeIncrementGroup->setVisible(incrementEnabled && nativeMode);
    m_advancedIncrementGroup->setVisible(incrementEnabled && !nativeMode);
    if (incrementEnabled && nativeMode)
    {
        // Native row-major/right and column-major/down order uses literal address units, including U32 words.
        for (column = int(Stage3AddressField::Operation); column <= int(Stage3AddressField::LampMultiMonitor); ++column)
        {
            if (!availableFields.contains(column)) { continue; }
            increment.field = Stage3AddressField(column);
            increment.columnStep = m_incrementDirection->currentData().toInt() == 0 ? m_incrementStep->value() : options.rows * m_incrementStep->value();
            increment.rowStep = m_incrementDirection->currentData().toInt() == 0 ? options.columns * m_incrementStep->value() : m_incrementStep->value();
            options.increments.append(increment);
        }
    }
    else if (incrementEnabled && m_addressField->currentData().toInt() >= 0)
    {
        increment.field = Stage3AddressField(m_addressField->currentData().toInt());
        increment.columnStep = m_columnAddressStep->value();
        increment.rowStep = m_rowAddressStep->value();
        options.increments.append(increment);
    }
    m_columnAddressStep->setEnabled(m_addressField->currentData().toInt() >= 0);
    m_rowAddressStep->setEnabled(m_addressField->currentData().toInt() >= 0);
    valid = duplicateStage3Selection(m_source, m_selection, options, &candidate, &ids, &error);
    m_table->setRowCount(0);
    m_previewIds.clear();
    m_error->setText(error);
    m_buttons->button(QDialogButtonBox::Ok)->setEnabled(valid);
    if (!valid) { m_summary->setText(QStringLiteral("预览无效，原工程保持不变。")); return false; }
    m_preview = candidate;
    m_previewIds = ids;
    m_summary->setText(QStringLiteral("保留%1个原控件，新增%2个控件。" ).arg(m_selection.size()).arg(ids.size()));
    row = 0;
    Q_FOREACH (object, candidate.objects)
    {
        if (!ids.contains(object.id)) { continue; }
        m_table->insertRow(row);
        values = {object.name, QString::number(object.geometry.x()), QString::number(object.geometry.y()),
                  QString::number(object.geometry.width()), QString::number(object.geometry.height()), object.address, object.id};
        for (column = 0; column < values.size(); ++column) { m_table->setItem(row, column, new QTableWidgetItem(values.at(column))); }
        ++row;
    }
    return true;
}

/******************************************************************************
 * Function Name   : Stage3DuplicateDialog::accept
 * Description     : Publish exactly one complete validated preview for the main window's project-level undo command.
 * Input Parameters: None
 * Return Value    : None.
 * Date            : 2026.11.10
 ******************************************************************************/
void Stage3DuplicateDialog::accept() //20261110-8 add
{
    if (!refreshPreview()) { return; }
    m_working = m_preview;
    m_newIds = m_previewIds;
    QDialog::accept();
}
