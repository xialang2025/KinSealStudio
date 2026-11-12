"""Connect the C25 independent property family through the existing strict typed-control path."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';sources={}
def edit(path,before,after):
 s=sources.get(path,(root/path).read_text(encoding='utf-8'));assert s.count(before)==1,(path,before,s.count(before));sources[path]=s.replace(before,after)
tree=ast.parse((root.parent/'integrate_chart.py').read_text(encoding='utf-8'))
for node in tree.body:
 if not isinstance(node,ast.Expr) or not isinstance(node.value,ast.Call) or not isinstance(node.value.func,ast.Name) or node.value.func.id!='edit':continue
 path,before,after=map(ast.literal_eval,node.value.args)
 after=after.replace('chart_properties','xy_curve_properties').replace('chart_dialog','xy_curve_dialog').replace('ChartProperties','XyCurveProperties').replace('ChartDialog','XyCurveDialog').replace('paintChart','paintXyCurve').replace('object->chart','object->xyCurve').replace('object.chart','object.xyCurve').replace('m_record.chart','m_record.xyCurve').replace('chart.toJson()','xyCurve.toJson()').replace('XyCurveProperties chart','XyCurveProperties xyCurve').replace('QStringLiteral("chart")','QStringLiteral("xycurve")').replace('QStringLiteral("图表','QStringLiteral("X-Y曲线').replace('//20261112-1','//20261113-1')
 if path=='CMakeLists.txt' and before.endswith('properties.cpp)'):
  before='src/data_collection_properties.cpp)';after='src/data_collection_properties.cpp src/xy_curve_properties.cpp)'
 if path=='src/project_model.cpp' and before=='QStringLiteral("moveTrack"), QStringLiteral("extensions")':
  before='QStringLiteral("historytrend"), QStringLiteral("extensions")';after='QStringLiteral("historytrend"), QStringLiteral("xycurve"), QStringLiteral("extensions")'
 if path=='src/project_model.cpp' and before=='value.toObject().contains(QStringLiteral("moveTrack")))':
  before='value.toObject().contains(QStringLiteral("historytrend")))';after='value.toObject().contains(QStringLiteral("historytrend")) || value.toObject().contains(QStringLiteral("xycurve")))'
 if path=='src/stage3_project_edits.cpp':
  after='''    if (object->type == QStringLiteral("xycurve")) //20261113-1 add: Guard paired X/Y coordinates and keep trigger/clear increments independent.
    {
        addressSlots.append({Stage3AddressField::Operation, &object->xyCurve.readAddress, PlcAddressRole::Word, XyCurveProperties::wordCount(object->xyCurve.dataType) * u32(object->xyCurve.channelCount) * 2U});
        addressSlots.append({Stage3AddressField::TriggerBit, &object->xyCurve.triggerAddress, PlcAddressRole::Bit, 1U}); addressSlots.append({Stage3AddressField::ClearBit, &object->xyCurve.clearAddress, PlcAddressRole::Bit, 1U}); addConditionSlots(&object->xyCurve.visibility, false, &addressSlots);
    }
    if (object->type == QStringLiteral("track")) //'''
 if path=='src/native_shell_ui.cpp':after=after.replace('graphicsitem','action_X_Y').replace('btn_metre','btn_XYtrend')
 if path=='src/hmi_canvas.cpp' and before.startswith('object.name = '):
  before='object.name = (object.type == QStringLiteral("historytrend")';after='object.name = (object.type == QStringLiteral("xycurve") ? QStringLiteral("X-Y曲线%1") : object.type == QStringLiteral("historytrend")'
 if path=='src/native_shell_ui.cpp' and before.startswith('!selected ||'):
  before='!selected || object.type == QStringLiteral("historytrend")';after='!selected || object.type == QStringLiteral("xycurve") || object.type == QStringLiteral("historytrend")'
 if path in ('src/main_window.cpp','src/static_text_actions.cpp') and before=='|| object.type == QStringLiteral("track")))':
  before='|| object.type == QStringLiteral("trend")))' if path=='src/main_window.cpp' else '|| object.type == QStringLiteral("historytrend")))'
  after=before[:-2]+' || object.type == QStringLiteral("xycurve")))'
 after=after.replace('object.xyCurve.setAddress(object.address.trimmed());','object.xyCurve.readAddress = object.address.trimmed();').replace('object.xyCurve.address()','object.xyCurve.readAddress').replace('QRect(0, 0, 170, 130)','QRect(0, 0, 240, 160)')
 edit(path,before,after)
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
print('Connected C25 typed persistence, real action, compact read field and paired-address copy roles.')
