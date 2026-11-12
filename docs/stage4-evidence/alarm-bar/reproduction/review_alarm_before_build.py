"""Apply concrete findings from the read-only C27 API/schema/transaction review."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8').replace('exportNativeProject(project,','exportNativeProjectSource(project,').replace('compileIndependentDevicePayloads(project,','buildProjectPayload(project,').replace('copyStage3Selection(project,','captureStage3Selection(project,')
s=s.replace('page.id = QStringLiteral("alarm-target");','page.width = 800; page.height = 480; page.id = QStringLiteral("alarm-target");')
p.write_text(s.replace('QStringLiteral("schema")', 'QStringLiteral("schemaVersion")'),encoding='utf-8',newline='\n')
p=root/'src/project_model.cpp';s=p.read_text(encoding='utf-8');before='''        Q_FOREACH (value, json.value(QStringLiteral("configurationNodes")).toArray())
        {''';after=before+'''
            if (schema < 12 && value.toObject().value(QStringLiteral("kind")).toString() == QStringLiteral("alarm") && (value.toObject().value(QStringLiteral("active")).toBool() || value.toObject().value(QStringLiteral("implemented")).toBool())) { return modelError(error, QStringLiteral("报警登录专有数据必须使用schema12。")); } //20261113-2 add''';assert s.count(before)==1;s=s.replace(before,after);p.write_text(s,encoding='utf-8',newline='\n')
print('Corrected test API names, explicit page dimensions and registration-only schema12 downgrade guard.')
