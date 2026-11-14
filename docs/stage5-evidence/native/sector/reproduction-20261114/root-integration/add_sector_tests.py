"""Bind the G07 gate to source-verified frames before enabling production."""
from pathlib import Path

root = Path(__file__).resolve().parent
work = root.parent / 'kinseal-stage4-20261112/worktree'
p = work / 'tests/stage5_compiler_tests.cpp'
text = p.read_text(encoding='utf-8')
assert 'sectorCanonicalFrames' not in text
slots = '''    void sectorCanonicalFrames_data(); //20261114-3 add
    void sectorCanonicalFrames();
    void sectorUnprovedFieldsPreserveOutput_data();
    void sectorUnprovedFieldsPreserveOutput();
    void sectorSourceAndPackageGates();
'''
text = text.replace('    void ellipseSharedPrecisionGuards();', '    void ellipseSharedPrecisionGuards();\n' + slots)
start = text.index('/******************************************************************************\n * Function Name   : ellipseProject')
end = text.index('/******************************************************************************\n * Function Name   : lineProject', start)
factory = text[start:end].replace('ellipse', 'sector').replace('Ellipse', 'Sector').replace('//20261114-2', '//20261114-3')
factory = factory.replace('圆/椭圆', '扇形').replace('200,100)', '200,200)')
factory = factory.replace('object.graphic.foreground=Qt::black;', 'object.graphic.lineStyle=0;object.graphic.foreground=Qt::black;')
factory = factory.replace('black-outline, white-fill', 'open default-angle, white-fill')
text = text[:end] + factory + text[end:]
start = text.index('/******************************************************************************\n * Function Name   : Stage5CompilerTests::ellipseCanonicalFrames_data')
end = text.index('/******************************************************************************\n * Function Name   : Stage5CompilerTests::ellipseSharedPrecisionGuards', start)
tests = text[start:end].replace('ellipse', 'sector').replace('Ellipse', 'Sector').replace('//20261114-2', '//20261114-3')
tests = tests.replace('B-S-M-sector-cell-area-r2.json', 'B-S-M-sector-cell-area-r1.json')
tests = tests.replace('including both formerly failing families', 'including local paths mapped by their verified integer translations')
tests = tests.replace('variant<27', 'variant<30')
tests = tests.replace('geometry.moveTop(279)', 'geometry.moveTop(179)')
tests = tests.replace('    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue));QCOMPARE(plan.staticSectorCount,77);', '''    if (variant==27) { project.objects[0].geometry.setSize(QSize(241,241)); }
    if (variant==28) { project.objects[0].geometry.setHeight(199); }
    if (variant==29) { peer=project.objects[0];peer.id=QStringLiteral("patch-edge-contact");peer.geometry.moveLeft(303);project.objects.append(peer); }
    QVERIFY(!prepareDeviceCompilePlan(project,&plan,&error,&issue));QCOMPARE(plan.staticSectorCount,77);''')
text = text.replace('QTEST_MAIN(Stage5CompilerTests)', tests + 'QTEST_MAIN(Stage5CompilerTests)')
text = text.replace('QStringLiteral("sector"); } //20261114-2 change: Ellipses now have their own source-verified slice; keep this unknown-type rejection meaningful.', 'QStringLiteral("unrecovered-control"); } //20261114-3 change: Keep the unknown-type case independent of newly admitted graphical families.')
text = text.replace('    HmiObject ellipse; //20261114-2 add:', '    HmiObject sector; //20261114-3 add: A ninth static family must preserve all existing control records.\n    HmiObject ellipse; //20261114-2 add:')
text = text.replace('rectangle.geometry.moveTopLeft(QPoint(100,300))', 'rectangle.geometry.moveTopLeft(QPoint(130,300))')
text = text.replace('    QCOMPARE(project.objects.size(),13);', '    sector=sectorProject().objects.first();sector.geometry=QRect(10,205,100,100);project.objects.append(sector); //20261114-3 add\n    QCOMPARE(project.objects.size(),14);')
p.write_text(text, encoding='utf-8')
p = work / 'src/device_compile_plan.h'
text = p.read_text(encoding='utf-8')
assert 'staticSectorCount' not in text
text = text.replace('    int staticEllipseCount=0;', '    int staticSectorCount=0; //20261114-3 add: Default-angle sectors allocate only verified background pixels.\n    int staticEllipseCount=0;')
p.write_text(text, encoding='utf-8')
print('Added sector fixture tests, 30 negative cases and package gates; production remains closed.')
