"""Temporarily add focused diagnostics to the two failing tests, preserving byte-identical originals for restoration."""
from pathlib import Path
import hashlib
import json
root = Path(__file__).resolve().parent
main = root.parents[1] / 'KinSealStudio'
backup = root / 'diagnose-test-originals'
backup.mkdir(exist_ok=True)
records = []
for relative in ('tests/stage3_canvas_tests.cpp', 'tests/stage3_integration_tests.cpp'):
    path = main / relative
    peer = backup / path.name
    current = path.read_bytes()
    data = peer.read_bytes() if peer.exists() else current
    assert current == data or b'[DEBUG-c21-' in current, 'Concurrent test changes must be preserved'
    if not peer.exists():
        peer.write_bytes(data)
    records.append(dict(relative=relative, sha256=hashlib.sha256(data).hexdigest()))
    text = data.decode('utf-8-sig').replace('\r\n', '\n')
    if 'canvas' in path.name:
        before = '    QCOMPARE(canvas.scene()->selectedItems().size(), 2);\n    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(objectRecords(canvas), locked);'
        after = '''    qInfo() << "[DEBUG-c21-group] immediate" << canvas.selectedObjectIds() << canvas.scene()->selectedItems().size();
    Q_FOREACH (graphics, canvas.scene()->items()) { qInfo() << "[DEBUG-c21-group] item" << graphics->data(0) << graphics->data(20) << graphics->parentItem() << graphics->flags() << graphics->isSelected(); }
    QCoreApplication::processEvents(); qInfo() << "[DEBUG-c21-group] after-events" << canvas.selectedObjectIds() << canvas.scene()->selectedItems().size();
    QCOMPARE(canvas.scene()->selectedItems().size(), 2);
    canvas.executeEdit(QStringLiteral("undo")); QCOMPARE(objectRecords(canvas), locked);'''
    else:
        text = text.replace('#include <QPainter>', '#include <QPainter>\n#include <QFontInfo>\n#include <QFontMetricsF>')
        before = '    QCOMPARE(first, second);\n    QCOMPARE(controlPixelFont(properties, 96).pointSizeF(), 13.125);'
        after = '''    qInfo() << "[DEBUG-c21-dpi] device" << first.logicalDpiY() << second.logicalDpiY();
    qInfo() << "[DEBUG-c21-dpi] metrics" << QFontMetricsF(controlPixelFont(properties, first.logicalDpiY()), &first).height() << QFontMetricsF(controlPixelFont(properties, second.logicalDpiY()), &second).height();
    qInfo() << "[DEBUG-c21-dpi] font" << QFontInfo(controlPixelFont(properties, first.logicalDpiY())).pixelSize() << QFontInfo(controlPixelFont(properties, second.logicalDpiY())).pixelSize();
    first.save(QStringLiteral("Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-stage4-20261112/dpi-first.png")); second.save(QStringLiteral("Z:/Project/vscode_prj_temp/REA/.scratch/kinseal-stage4-20261112/dpi-second.png"));
    QCOMPARE(first, second);
    QCOMPARE(controlPixelFont(properties, 96).pointSizeF(), 13.125);'''
    assert text.count(before) == 1
    text = text.replace(before, after)
    path.write_text(text, encoding='utf-8', newline='\n')
    records[-1]['instrumentedSha256'] = hashlib.sha256(path.read_bytes()).hexdigest()
(backup / 'manifest.json').write_text(json.dumps(records, indent=2), encoding='utf-8')
