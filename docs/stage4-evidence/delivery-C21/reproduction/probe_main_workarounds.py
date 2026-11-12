"""Test one selection-cache reset and one font-layout hypothesis; all touched source is restored afterward."""
from pathlib import Path
import hashlib
import json
root = Path(__file__).resolve().parent
main = root.parents[1] / 'KinSealStudio'
backup = root / 'diagnose-test-originals'
records = json.loads((backup / 'manifest.json').read_text(encoding='utf-8'))
canvas = main / 'tests/stage3_canvas_tests.cpp'
text = canvas.read_text(encoding='utf-8')
assert hashlib.sha256(canvas.read_bytes()).hexdigest() == records[0]['instrumentedSha256']
before = '    QCOMPARE(canvas.scene()->selectedItems().size(), 2);\n    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(objectRecords(canvas), locked);'
assert text.count(before) == 1
text = text.replace(before, '''    Q_FOREACH (graphics, canvas.scene()->items()) { if (graphics->parentItem() == nullptr && graphics->isSelected()) { graphics->setSelected(false); graphics->setSelected(true); } }
    qInfo() << "[DEBUG-c21-group] after-explicit-root-reset" << canvas.scene()->selectedItems().size();
''' + before)
canvas.write_text(text, encoding='utf-8', newline='\n')
records[0]['instrumentedSha256'] = hashlib.sha256(canvas.read_bytes()).hexdigest()
path = main / 'src/static_text_renderer.cpp'
data = path.read_bytes()
assert not (backup / path.name).exists()
(backup / path.name).write_bytes(data)
text = data.decode('utf-8-sig').replace('\r\n', '\n')
before = '    layout.setText(content); layout.setFont(font);'
assert text.count(before) == 1
text = text.replace(before, '    layout.setText(content); // [DEBUG-c21-dpi] Keep the constructor font bound to its explicit paint device.')
path.write_text(text, encoding='utf-8', newline='\n')
records.append(dict(relative='src/static_text_renderer.cpp', sha256=hashlib.sha256(data).hexdigest(), instrumentedSha256=hashlib.sha256(path.read_bytes()).hexdigest()))
(backup / 'manifest.json').write_text(json.dumps(records, indent=2), encoding='utf-8')
