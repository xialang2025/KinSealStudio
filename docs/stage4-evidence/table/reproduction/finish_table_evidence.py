"""Persist the actual table ROI comparison and explicit original-page human observation."""
from pathlib import Path
import hashlib
import json
from PIL import Image

root = Path(__file__).resolve().parent
work = root / 'worktree'
base = work / 'docs/stage4-evidence/table'
read = lambda path: json.loads(path.read_text(encoding='utf-8-sig'))
def write(path, data):
    path.write_text(json.dumps(data, ensure_ascii=False, indent=2) + '\n', encoding='utf-8')

physical = base / 'physical/table-baseline-r1'
assert not (physical / 'screen-result.json').exists()
run = read(physical / 'run.json')
assert run['completed'] and run['nativeHardware']['usbCompletion']['ready']
write(physical / 'screen-result.json', dict(source='explicit_user_reply', answer='已显示 4×4 表格，没有宽度提示',
    sourceSha256=run['sourceSha256'], nativeBaselineDefaultDisplayConfirmed=True, widthWarningAbsent=True,
    screenTouchConfirmed=False, fullRenderingNotClaimed=True, liteIndependentEncoderVerified=False,
    scope='Reply to the table-baseline-r1 question; only the default native 4-by-4 table page.'))
local_path = base / 'windows-r1/table-default-raster.png'
native_path = base / 'native/sheet-table-default-r1.png'
local = Image.open(local_path).convert('RGB')
native = Image.open(native_path).convert('RGB')
assert local.size == native.size
box = (90, 90, 311, 211)
lp = list(local.crop(box).getdata())
np = list(native.crop(box).getdata())
different = sum(a != b for a, b in zip(lp, np))
mask_diff = sum((a == (0, 0, 0)) != (b == (0, 0, 0)) for a, b in zip(lp, np))
tests = (base / 'windows-r1/KinSealStage4ControlTests.txt').read_text(encoding='utf-8-sig')
assert 'PASS   : Stage4ControlTests::tableEightNativeSwitchRasterMasks()' in tests
comparison = dict(roiExclusiveRightBottom=list(box), width=box[2]-box[0], height=box[3]-box[1],
    local=str(local_path.relative_to(work)).replace('\\', '/'), native=str(native_path.relative_to(work)).replace('\\', '/'),
    localSha256=hashlib.sha256(local_path.read_bytes()).hexdigest(), nativeSha256=hashlib.sha256(native_path.read_bytes()).hexdigest(),
    fullRgbDifferentPixels=different, defaultBlackMaskDifferentPixels=mask_diff,
    eightNativeSwitchBlackMaskCases=8, eightNativeSwitchBlackMaskDifferentPixels=[0]*8,
    switchResultEvidence='windows-r1/KinSealStage4ControlTests.txt::tableEightNativeSwitchRasterMasks',
    localBackgroundRgb=[224,224,224], nativeRgb565DecodedBackground=[230,226,230],
    completeRasterEqualityClaimed=False)
write(base / 'raster-comparison.json', comparison)
spec_path = root / 'table-delivery.json'
spec = read(spec_path)
spec['screenObservation'] = '用户对本次表格问题明确回复“已显示 4×4 表格，没有宽度提示”。仅确认当前原厂默认基线显示，完整变体/触控E和Lite独立设备编码仍未通过。[人工确认](stage4-evidence/table/physical/table-baseline-r1/screen-result.json)。'
spec['protocol'] += ' 用户确认同一基线“已显示 4×4 表格，没有宽度提示”；仅默认显示观察，完整变体/触控E和Lite独立编码保持未验。'
spec['renderLimit'] += f' 实测221×121 ROI完整RGB差异{different}像素；默认黑线掩码差异{mask_diff}像素，全部8开关组合黑线掩码差异均0。详见[raster-comparison.json](stage4-evidence/table/raster-comparison.json)。'
spec['reviewFindings'].append('最终实际窗口截图已检查：600×500窗口、外观页、行列/三色/透明度/三开关均可见，无裁切；字体与组合框图标未宣称逐像素相同。')
spec['unresolvedText'][-1] = 'Q-S04-G10-003：用户已确认默认4×4原厂基线显示且无宽度提示，完整变体/触控E与Lite独立设备输出仍未验收。'
write(spec_path, spec)
print(json.dumps(comparison, ensure_ascii=False))
