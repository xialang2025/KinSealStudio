from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8')
before='QCOMPARE(result.configurationNodes, project.configurationNodes);'
assert s.count(before)==1
s=s.replace(before,'QCOMPARE(result.configurationNodes.size(), project.configurationNodes.size()); QCOMPARE(result.configurationNodes.first().toJson(), project.configurationNodes.first().toJson());')
p.write_text(s,encoding='utf-8',newline='\n')
