"""Adapt the audited typed-control integration to C26's one buffer, sample word and dynamic maximum roles."""
from pathlib import Path
root=Path(__file__).resolve().parent
s=(root/'integrate_xy.py').read_text(encoding='utf-8')
s=s.replace('C25','C26').replace('xy_curve','data_group').replace('XyCurve','DataGroup').replace('xyCurve','dataGroup').replace('xycurve','datagroup').replace('paintXyCurve','paintDataGroup').replace('X-Y曲线','数据群组').replace('action_X_Y','action_dataGroup').replace('btn_XYtrend','btn_dataGroup')
s=s.replace("before='src/data_collection_properties.cpp)';after='src/data_collection_properties.cpp src/data_group_properties.cpp)'", "before='src/xy_curve_properties.cpp)';after='src/xy_curve_properties.cpp src/data_group_properties.cpp)'")
s=s.replace("before='QStringLiteral(\"historytrend\"), QStringLiteral(\"extensions\")';after='QStringLiteral(\"historytrend\"), QStringLiteral(\"datagroup\"), QStringLiteral(\"extensions\")'", "before='QStringLiteral(\"xycurve\"), QStringLiteral(\"extensions\")';after='QStringLiteral(\"xycurve\"), QStringLiteral(\"datagroup\"), QStringLiteral(\"extensions\")'")
s=s.replace("before='value.toObject().contains(QStringLiteral(\"historytrend\")))';after='value.toObject().contains(QStringLiteral(\"historytrend\")) || value.toObject().contains(QStringLiteral(\"datagroup\")))'", "before='value.toObject().contains(QStringLiteral(\"xycurve\")))';after='value.toObject().contains(QStringLiteral(\"xycurve\")) || value.toObject().contains(QStringLiteral(\"datagroup\")))'")
s=s.replace("before='object.name = (object.type == QStringLiteral(\"historytrend\")';after='object.name = (object.type == QStringLiteral(\"datagroup\") ? QStringLiteral(\"数据群组%1\") : object.type == QStringLiteral(\"historytrend\")'", "before='object.name = (object.type == QStringLiteral(\"xycurve\")';after='object.name = (object.type == QStringLiteral(\"datagroup\") ? QStringLiteral(\"数据群组%1\") : object.type == QStringLiteral(\"xycurve\")'")
s=s.replace("before='!selected || object.type == QStringLiteral(\"historytrend\")';after='!selected || object.type == QStringLiteral(\"datagroup\") || object.type == QStringLiteral(\"historytrend\")'", "before='!selected || object.type == QStringLiteral(\"xycurve\")';after='!selected || object.type == QStringLiteral(\"datagroup\") || object.type == QStringLiteral(\"xycurve\")'")
s=s.replace("before='|| object.type == QStringLiteral(\"trend\")))' if path=='src/main_window.cpp' else '|| object.type == QStringLiteral(\"historytrend\")))'", "before='|| object.type == QStringLiteral(\"xycurve\")))'")
a=s.index('  after=\'\'\'    if (object->type == QStringLiteral("datagroup"))');b=s.index("'''",a+11)
# Replace the entire family-specific address block, retaining the following track branch anchor.
b=s.index("'''",s.index('    if (object->type == QStringLiteral("track")) //',a))+3
s=s[:a]+'''  after=\'\'\'    if (object->type == QStringLiteral("datagroup")) //20261113-1 add: A group owns one full point buffer and independent active sample/maximum words.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->dataGroup.readAddress, PlcAddressRole::Word, object->dataGroup.readWordSpan()});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->dataGroup.triggerAddress, PlcAddressRole::Bit, 1U});
        if (object->dataGroup.sampleMode == 1) { addressSlots.append({Stage3AddressField::SamplingWord, &object->dataGroup.sampleAddress, PlcAddressRole::Word, 1U}); }
        if (object->dataGroup.rangeSource == 1) { addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->dataGroup.maximumAddress, PlcAddressRole::Word, DataGroupProperties::wordCount(object->dataGroup.dataType)}); }
        addConditionSlots(&object->dataGroup.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("track")) //\'\'\''''+s[b:]
s=s.replace("for path,s in sources.items():",'''edit('src/stage3_project_edits.h', '    TriggerBit, ClearBit //20261112-1 add:', '    TriggerBit, ClearBit, SamplingWord //20261113-1 change:')
edit('src/stage3_project_edits.cpp', 'int(increment.field) > int(Stage3AddressField::ClearBit)', 'int(increment.field) > int(Stage3AddressField::SamplingWord)')
edit('src/stage3_project_edits.cpp', '    m_addressField->addItem(QStringLiteral("采样清除位地址"), int(Stage3AddressField::ClearBit));', '    m_addressField->addItem(QStringLiteral("采样清除位地址"), int(Stage3AddressField::ClearBit));\\n    m_addressField->addItem(QStringLiteral("数据群组采样地址"), int(Stage3AddressField::SamplingWord)); //20261113-1 add')
for path,s in sources.items():''')
s=s.replace('DataGroupProperties dataGroup; //20261113-1 add: Separate bar, meter and donut drafts.','DataGroupProperties dataGroup; //20261113-1 add: One buffered curve with independent sample and maximum source drafts.')
(root/'integrate_group.py').write_text(s,encoding='utf-8',newline='\n')
print('Prepared C26 integration with an appended SamplingWord enum and no fabricated clear carrier.')
