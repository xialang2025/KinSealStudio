"""Finalize reviewed C27 test names without rerunning any generator."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');s=s.replace('QStringLiteral("schema")','QStringLiteral("schemaVersion")');s=s.replace('HmiProject mixed = historyProject();\n    QJsonObject json;\n    QByteArray source = QByteArrayLiteral("keep");','HmiProject mixed = historyProject();\n    QJsonObject json;\n    QByteArray source = QByteArrayLiteral("keep");')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/stage4_control_actions.cpp';p.write_text(p.read_text(encoding='utf-8').rstrip()+'\n',encoding='utf-8',newline='\n')
p=root/'src/alarm_bar_properties.h';p.write_text(p.read_text(encoding='utf-8').replace('frameColor = QStringLiteral("#000000")','frameColor = QStringLiteral("#e0e0e0")'),encoding='utf-8',newline='\n')
print('Updated C27 schemaVersion assertions in final test source.')
