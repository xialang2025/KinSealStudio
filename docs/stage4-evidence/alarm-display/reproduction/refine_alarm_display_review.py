"""Prevent copying hidden caption languages into a project that cannot edit them."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'src/project_model.cpp';s=p.read_text(encoding='utf-8');a=s.index('bool HmiProject::validatePlcConfiguration(');head=s[:a];tail=s[a:]
before='    const StaticTextProperties *condition;';assert tail.count(before)==1;tail=tail.replace(before,'    QString captionLanguage; //20261113-2 add: Caption languages must remain editable in the destination project.\n'+before)
before='        if (object.type == QStringLiteral("alarmdisplay") &&';assert tail.count(before)==1
tail=tail.replace(before,'''        if (object.type == QStringLiteral("alarmdisplay")) //20261113-2 add: Reject missing destination languages instead of dropping their captions on the next property edit.
        {
            Q_FOREACH (captionLanguage, object.alarmDisplay.captions.keys()) { if (!languages.contains(captionLanguage)) { return modelError(error, QStringLiteral("报警显示列名语言%1未在目标工程启用，事务已取消。").arg(captionLanguage)); } }
        }
'''+before);p.write_text(head+tail,encoding='utf-8',newline='\n')
p=root/'tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');a=s.index('void Stage4ControlTests::alarmDisplaySharedPrerequisitesCopyAndVisibility()');b=s.index('\n/******************************************************************************',a);part=s[a:b];part=part.replace('    Stage3Clipboard clipboard;','    Stage3Clipboard clipboard;\n    HmiProject sentinel;')
before='target.languages = project.languages;';assert part.count(before)==1;part=part.replace(before,'sentinel = result; QVERIFY(!pasteStage3Selection(target, clipboard, QPoint(10,10), &result, nullptr, &error)); QCOMPARE(result.toJson(), sentinel.toJson()); '+before);p.write_text(s[:a]+part+s[b:],encoding='utf-8',newline='\n')
print('Added atomic refusal for caption languages absent from the destination and covered recovery by matching project languages.')
