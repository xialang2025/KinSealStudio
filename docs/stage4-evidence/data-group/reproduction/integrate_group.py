"""Connect the C26 independent property family through the existing strict typed-control path."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';sources={}
def edit(path,before,after):
 s=sources.get(path,(root/path).read_text(encoding='utf-8'));assert s.count(before)==1,(path,before,s.count(before));sources[path]=s.replace(before,after)
tree=ast.parse((root.parent/'integrate_chart.py').read_text(encoding='utf-8'))
for node in tree.body:
 if not isinstance(node,ast.Expr) or not isinstance(node.value,ast.Call) or not isinstance(node.value.func,ast.Name) or node.value.func.id!='edit':continue
 path,before,after=map(ast.literal_eval,node.value.args)
 after=after.replace('chart_properties','data_group_properties').replace('chart_dialog','data_group_dialog').replace('ChartProperties','DataGroupProperties').replace('ChartDialog','DataGroupDialog').replace('paintChart','paintDataGroup').replace('object->chart','object->dataGroup').replace('object.chart','object.dataGroup').replace('m_record.chart','m_record.dataGroup').replace('chart.toJson()','dataGroup.toJson()').replace('DataGroupProperties chart','DataGroupProperties dataGroup').replace('QStringLiteral("chart")','QStringLiteral("datagroup")').replace('QStringLiteral("图表','QStringLiteral("数据群组').replace('//20261112-1','//20261113-1')
 if path=='CMakeLists.txt' and before.endswith('properties.cpp)'):
  before='src/xy_curve_properties.cpp)';after='src/xy_curve_properties.cpp src/data_group_properties.cpp)'
 if path=='src/project_model.cpp' and before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")':
  before='QStringLiteral("xycurve"), QStringLiteral("extensions")';after='QStringLiteral("xycurve"), QStringLiteral("datagroup"), QStringLiteral("extensions")'
 if path=='src/project_model.cpp' and before=='value.toObject().contains(QStringLiteral("moveTrack")))':
  before='value.toObject().contains(QStringLiteral("xycurve")))';after='value.toObject().contains(QStringLiteral("xycurve")) || value.toObject().contains(QStringLiteral("datagroup")))'
 if path=='src/stage3_project_edits.cpp':
  after='''    if (object->type == QStringLiteral("datagroup")) //20261113-1 add: A group owns one full point buffer and independent active sample/maximum words.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->dataGroup.readAddress, PlcAddressRole::Word, object->dataGroup.readWordSpan()});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->dataGroup.triggerAddress, PlcAddressRole::Bit, 1U});
        if (object->dataGroup.sampleMode == 1) { addressSlots.append({Stage3AddressField::SamplingWord, &object->dataGroup.sampleAddress, PlcAddressRole::Word, 1U}); }
        if (object->dataGroup.rangeSource == 1) { addressSlots.append({Stage3AddressField::DisplayMaximumWord, &object->dataGroup.maximumAddress, PlcAddressRole::Word, DataGroupProperties::wordCount(object->dataGroup.dataType)}); }
        addConditionSlots(&object->dataGroup.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("track")) //'''
 if path=='src/native_shell_ui.cpp':after=after.replace('graphicsitem','action_dataGroup').replace('btn_metre','btn_dataGroup')
 if path=='src/hmi_canvas.cpp' and before.startswith('object.name = '):
  before='object.name = (object.type == QStringLiteral("xycurve")';after='object.name = (object.type == QStringLiteral("datagroup") ? QStringLiteral("数据群组%1") : object.type == QStringLiteral("xycurve")'
 if path=='src/native_shell_ui.cpp' and before.startswith('!selected ||'):
  before='!selected || object.type == QStringLiteral("xycurve")';after='!selected || object.type == QStringLiteral("datagroup") || object.type == QStringLiteral("xycurve")'
 if path in ('src/main_window.cpp','src/static_text_actions.cpp') and before=='|| object.type == QStringLiteral("track")))':
  before='|| object.type == QStringLiteral("xycurve")))'
  after=before[:-2]+' || object.type == QStringLiteral("datagroup")))'
 after=after.replace('object.dataGroup.setAddress(object.address.trimmed());','object.dataGroup.readAddress = object.address.trimmed();').replace('object.dataGroup.address()','object.dataGroup.readAddress').replace('QRect(0, 0, 170, 130)','QRect(0, 0, 240, 160)')
 edit(path,before,after)
edit('src/stage3_project_edits.h', '    TriggerBit, ClearBit //20261112-1 add:', '    TriggerBit, ClearBit, SamplingWord //20261113-1 change:')
edit('src/stage3_project_edits.cpp', 'int(increment.field) > int(Stage3AddressField::ClearBit)', 'int(increment.field) > int(Stage3AddressField::SamplingWord)')
edit('src/stage3_project_edits.cpp', '    m_addressField->addItem(QStringLiteral("采样清除位地址"), int(Stage3AddressField::ClearBit));', '    m_addressField->addItem(QStringLiteral("采样清除位地址"), int(Stage3AddressField::ClearBit));\n    m_addressField->addItem(QStringLiteral("数据群组采样地址"), int(Stage3AddressField::SamplingWord)); //20261113-1 add')
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
print('Connected C26 typed persistence, real action, compact read field and paired-address copy roles.')
