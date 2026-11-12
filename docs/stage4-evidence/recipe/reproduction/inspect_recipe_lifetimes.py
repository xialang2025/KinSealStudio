from pathlib import Path
import json
base=Path(__file__).resolve().parent/'worktree/docs/stage4-evidence/recipe/native'
d=json.loads((base/'native-probe-recipe-lifetimes-r12.json').read_text(encoding='utf-8'))
out={k:d.get(k) for k in ['status','error','nativeTableStates','tableHeaders']};(base/'observed-lifetimes-r12.json').write_text(json.dumps(out,ensure_ascii=False,indent=2)+'\n',encoding='utf-8')
print(json.dumps(out,ensure_ascii=True,indent=2))
