"""Keep the mixed test collector identical to both historical consumers."""
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/tests/stage4_controls_tests.cpp'
s=p.read_text(encoding='utf-8');start=s.index('void Stage4ControlTests::historyDataStrictRoundTripAndBackendGates()');end=s.index('\n/******************************************************************************', start)
chunk=s[start:end];chunk=chunk.replace('    QString error;','    QString error;\n    int index;')
before='mixed.languages = project.languages; mixed.objects.append(project.objects.first());'
after='mixed.languages = project.languages; mixed.configurationNodes = project.configurationNodes; for (index = 0; index < 8; ++index) { mixed.objects[0].historyTrend.curves[index].sourceItemId = project.objects.first().historyData.columns.first().sourceItemId; } mixed.objects.append(project.objects.first());'
assert chunk.count(before)==1;chunk=chunk.replace(before,after);s=s[:start]+chunk+s[end:];p.write_text(s,encoding='utf-8',newline='\n')
print('Aligned mixed history fixture with the exact same stable collection items.')
