"""Enable only the formally compared default-angle G07 background slice."""
from pathlib import Path
import json

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
report = json.loads((work / 'docs/stage5-evidence/native/sector/B-S-M-sector-cell-area-r1.json').read_text(encoding='utf-8-sig'))
assert report['allRasterComparisonsEqual'] and len(report['cases']) == 27
red = json.loads((work / 'docs/stage5-evidence/sector-red-r1/run.json').read_text(encoding='utf-8-sig'))
assert red['expectedFailure'] and red['totals'] == dict(passed=2, failed=15, skipped=0)
p = work / 'src/device_compile_plan.cpp'
text = p.read_text(encoding='utf-8')
assert 'prepareDeviceSector' not in text
start = text.index('/******************************************************************************\n * Function Name   : prepareDeviceEllipse')
end = text.index('/******************************************************************************\n * Function Name   : prepareDeviceCompilePlan', start)
function = text[start:end].replace('Ellipse', 'Sector').replace('ellipse', 'sector').replace('椭圆', '扇形').replace('//20261114-2', '//20261114-3')
function = function.replace('object.geometry.height()<100 ||', 'object.geometry.height()<100 || object.geometry.width()!=object.geometry.height() || object.geometry.width()>240 ||')
function = function.replace('宽高至少100，右端', '边长100到240的正方形，右端')
text = text[:end] + function + text[end:]
marker = '        if (object.type==QStringLiteral("ellipse"))'
branch = '''        if (object.type==QStringLiteral("sector")) //20261114-3 add: Only the native default-angle square path may reach independent background composition.
        {
            if (!prepareDeviceSector(project,object,prepared.objects,error,issue)) { return false; }
            ++prepared.staticSectorCount;
            continue;
        }
'''
assert marker in text
text = text.replace(marker, branch + marker, 1)
p.write_text(text, encoding='utf-8')
p = work / 'src/independent_downloader.cpp'
text = p.read_text(encoding='utf-8')
assert 'device_sector_raster.h' not in text
text = text.replace('#include "device_ellipse_raster.h"', '#include "device_sector_raster.h" //20261114-3 add\n#include "device_ellipse_raster.h"', 1)
marker = '        else if (object.type==QStringLiteral("ellipse"))'
branch = '''        else if (object.type==QStringLiteral("sector")) //20261114-3 add: Publish no caller payload until both open-path masks and the isolated patch succeed.
        {
            if (!paintDeviceSectorGraphic(&painter,object.geometry,object.graphic,error))
            {
                painter.end();
                return false;
            }
        }
'''
assert marker in text
text = text.replace(marker, branch + marker, 1)
p.write_text(text, encoding='utf-8')
print('Enabled verified sector projection and transactional raster call site.')
