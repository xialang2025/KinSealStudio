"""Apply findings from the completed read-only C30 review before building once."""
from pathlib import Path
root=Path(__file__).resolve().parent/'worktree'
p=root/'src/data_collection_dialog.cpp';s=p.read_text(encoding='utf-8')
old='    if (existing < 0) { candidate.configurationNodes.append(result); } else { candidate.configurationNodes[existing] = result; }\n    if (!candidate.validateModel(&error)) { QMessageBox::warning(this, QStringLiteral("配置未提交"), error); return; } QDialog::accept();'
assert s.count(old)==1
s=s.replace(old,'    if (!candidate.replaceDataCollection(result, &error)) { QMessageBox::warning(this, QStringLiteral("配置未提交"), error); return; } QDialog::accept(); //20261113-2 change: Validate source and every dependent table together.')
# The explicit duplicate-collector loop remains; the replaced index variable is no longer needed.
s=s.replace('    int existing = -1;\n    if (!m_initialValid)', '    if (!m_initialValid)').replace('        if (peer.id == result.id) { existing = index; }\n        else if (peer.kind ==', '        if (peer.id != result.id && peer.kind ==')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/history_data_properties.cpp';s=p.read_text(encoding='utf-8')
s=s.replace('    QString initial;\n    Q_FOREACH', '    QString initial;\n    int index;\n    Q_FOREACH').replace('    QString name;\n    for (column', '    QString name;\n    int index;\n    for (column')
s=s.replace('    for (column = columns.begin(); column != columns.end(); ++column)\n    {', '    for (index = 0; index < columns.size(); ++index)\n    {\n        column = &columns[index];')
p.write_text(s,encoding='utf-8',newline='\n')
p=root/'CMakeLists.txt';s=p.read_text(encoding='utf-8').replace('src/history_data_properties.h #20261112-1','src/history_data_properties.h #20261113-2');p.write_text(s,encoding='utf-8',newline='\n')
p=root/'src/stage3_project_edits.cpp';s=p.read_text(encoding='utf-8').replace('剪贴板缺少历史趋势图的数据采集定义。','剪贴板缺少历史数据控件的数据采集定义。');p.write_text(s,encoding='utf-8',newline='\n')
print('Applied collector-acceptance reconciliation and Qt 6 iterator compatibility findings.')
