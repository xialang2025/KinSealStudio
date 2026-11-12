"""Reuse the audited typed-control wiring with trend-specific common read and two independent bit roles."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent
work=root/'worktree'
sources={}
def edit(path,before,after):
 s=sources.get(path,(work/path).read_text(encoding='utf-8'))
 assert s.count(before)==1,(path,before,s.count(before))
 sources[path]=s.replace(before,after)
tree=ast.parse((root/'integrate_chart.py').read_text(encoding='utf-8'))
for node in tree.body:
 if not isinstance(node,ast.Expr) or not isinstance(node.value,ast.Call) or not isinstance(node.value.func,ast.Name) or node.value.func.id!='edit': continue
 path,before,after=map(ast.literal_eval,node.value.args)
 after=after.replace('chart','trend').replace('Chart','Trend').replace('图表','趋势图')
 if path=='CMakeLists.txt' and before.endswith('properties.cpp)'):
  before='src/chart_properties.cpp)'; after='src/chart_properties.cpp src/trend_properties.cpp)'
 if path=='src/project_model.cpp' and before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")':
  before='QStringLiteral("chart"), QStringLiteral("extensions")'; after='QStringLiteral("chart"), QStringLiteral("trend"), QStringLiteral("extensions")'
 if path=='src/project_model.cpp' and before=='value.toObject().contains(QStringLiteral("moveTrack")))':
  before='value.toObject().contains(QStringLiteral("chart")))'; after='value.toObject().contains(QStringLiteral("chart")) || value.toObject().contains(QStringLiteral("trend")))'
 if path=='src/stage3_project_edits.cpp':
  after='''    if (object->type == QStringLiteral("trend")) //20261112-1 add: Sampling uses a shared full-span read carrier and independent trigger/clear bits.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->trend.readAddress, PlcAddressRole::Word, TrendProperties::wordCount(object->trend.dataType) * object->trend.channelCount});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->trend.triggerAddress, PlcAddressRole::Bit, 1U});
        addressSlots.append({Stage3AddressField::ClearBit, &object->trend.clearAddress, PlcAddressRole::Bit, 1U});
        addConditionSlots(&object->trend.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("track")) //'''
 if path=='src/native_shell_ui.cpp': after=after.replace('graphicsitem','action_trendchart').replace('btn_metre','btn_trend')
 if path=='src/hmi_canvas.cpp' and before.startswith('object.name = '):
  before='object.name = (object.type == QStringLiteral("chart")'
  after='object.name = (object.type == QStringLiteral("trend") ? QStringLiteral("趋势图%1") : object.type == QStringLiteral("chart")'
 if path=='src/native_shell_ui.cpp' and before.startswith('!selected ||'):
  before='!selected || object.type == QStringLiteral("chart")'
  after='!selected || object.type == QStringLiteral("trend") || object.type == QStringLiteral("chart")'
 if path in ('src/main_window.cpp','src/static_text_actions.cpp') and before=='|| object.type == QStringLiteral("track")))':
  before='|| object.type == QStringLiteral("chart")))'
  after='|| object.type == QStringLiteral("chart") || object.type == QStringLiteral("trend")))'
 after=after.replace('object.trend.setAddress(object.address.trimmed());','object.trend.readAddress = object.address.trimmed();').replace('object.trend.address()','object.trend.readAddress').replace('QRect(0, 0, 170, 130)','QRect(0, 0, 200, 120)')
 edit(path,before,after)
edit('src/stage3_project_edits.h','    OffsetWord, InputMinimumWord, InputMaximumWord, DisplayMinimumWord, DisplayMaximumWord //20261112-0 add','    OffsetWord, InputMinimumWord, InputMaximumWord, DisplayMinimumWord, DisplayMaximumWord, //20261112-0 add\n    TriggerBit, ClearBit //20261112-1 add: Independently increment trend sampling-control bits.')
edit('src/stage3_project_edits.cpp','int(increment.field) > int(Stage3AddressField::DisplayMaximumWord)','int(increment.field) > int(Stage3AddressField::ClearBit)')
edit('src/stage3_project_edits.cpp','    m_addressField->addItem(QStringLiteral("显示上限地址"), int(Stage3AddressField::DisplayMaximumWord));','    m_addressField->addItem(QStringLiteral("显示上限地址"), int(Stage3AddressField::DisplayMaximumWord));\n    m_addressField->addItem(QStringLiteral("采样触发位地址"), int(Stage3AddressField::TriggerBit)); //20261112-1 add\n    m_addressField->addItem(QStringLiteral("采样清除位地址"), int(Stage3AddressField::ClearBit));')
for path,s in sources.items(): (work/path).write_text(s,encoding='utf-8',newline='\n')
print('Integrated typed trend model, editor transactions, static canvas and three independent address carriers.')
