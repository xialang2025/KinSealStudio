"""Reuse the retained private GUI-thread probe for a freshly cleared, single move-track native sample."""
from pathlib import Path
import json

root = Path(__file__).resolve().parent
owner = root / 'native-track'
owner.mkdir(exist_ok=True)
(owner / 'temp').mkdir(exist_ok=True)
(root / 'worktree/docs/stage4-evidence/move-track/native').mkdir(parents=True, exist_ok=True)
source = (root / 'native-wheel/wheel_probe.py').read_text(encoding='utf-8')
source = source.replace('stage4-evidence/wheel/native', 'stage4-evidence/move-track/native').replace('native-wheel', 'native-track').replace('KinSealStage4Wheel20261112', 'KinSealStage4Track20261112')
(owner / 'track_probe.py').write_text(source, encoding='utf-8', newline='\n')
scenario = json.loads((root / 'native-wheel/wheel-default-r2.json').read_text(encoding='utf-8'))
for step in scenario:
    for key, value in list(step.items()):
        if isinstance(value, str):
            step[key] = value.replace('wheel-', 'track-').replace('action_wheelView', 'action_moveTrack')
(owner / 'track-default-r1.json').write_text(json.dumps(scenario, ensure_ascii=False, indent=2), encoding='utf-8')
