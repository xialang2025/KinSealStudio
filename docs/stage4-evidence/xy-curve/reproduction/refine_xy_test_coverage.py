"""Strengthen the existing C25 persistence method before its first compilation; no slots or product logic change."""
from pathlib import Path
p=Path(__file__).resolve().parent/'worktree/tests/stage4_controls_tests.cpp';s=p.read_text(encoding='utf-8');i=s.index('void Stage4ControlTests::xyStrictPersistenceAndBackendGates()');j=s.index('/******************************************************************************',i)
section=s[i:j];assert section.count('    object.xyCurve.channelCount = 3;')==1
section=section.replace('    object.xyCurve.channelCount = 3;','    object.xyCurve.xMinimum = QStringLiteral("-100"); object.xyCurve.xMaximum = QStringLiteral("4000"); object.xyCurve.xTotalDigits = 6; object.xyCurve.xFractionalDigits = 2;\n    object.xyCurve.channelCount = 3;')
s=s[:i]+section+s[j:];i=s.index(' * Function Name   : Stage4ControlTests::xyStaticRasterAndClipping');s=s[:i]+s[i:].replace('deterministic direction','independent numeric axis labels',1);p.write_text(s,encoding='utf-8',newline='\n')
