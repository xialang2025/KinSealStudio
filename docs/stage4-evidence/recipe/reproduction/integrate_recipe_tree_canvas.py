"""Complete the single-recipe tree lifecycle and nonpersistent resolved canvas headers."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';sources={};tree=ast.parse(Path(__file__).with_name('make_recipe_models.py').read_text(encoding='utf-8'));ns={};exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='f'],type_ignores=[]),'<format>','exec'),ns);f=ns['f']
def edit(path,before,after):
    s=sources.get(path,(root/path).read_text(encoding='utf-8'));assert s.count(before)==1,(path,before,s.count(before));sources[path]=s.replace(before,after)
edit('src/recipe_display_properties.cpp','        if (names.contains(node.name)', '        if (!names.isEmpty()) { return recipeDisplayError(error, QStringLiteral("已经达到最大配方数量，请删除后再新建。")); } //20261113-2 add: The target supports one definition with multiple recipe columns.\n        if (names.contains(node.name)')
edit('src/main_window.cpp','        else if (role == QStringLiteral("dataCollection"))','        else if (role == QStringLiteral("recipe")) { configureRecipe(pageId); } //20261113-2 add\n        else if (role == QStringLiteral("dataCollection"))')
edit('src/main_window.cpp','        if (item == nullptr || item->data(0, Qt::UserRole).toString() != QStringLiteral("dataCollection")) { return; }','''        if (item != nullptr && item->data(0, Qt::UserRole).toString() == QStringLiteral("recipe")) //20261113-2 add
        {
            id = item->data(0, Qt::UserRole + 1).toString(); if (id.isEmpty()) { menu.addAction(QStringLiteral("新增"), this, [this]() { configureRecipe(); }); } else { menu.addAction(QStringLiteral("编辑"), this, [this, id]() { configureRecipe(id); }); menu.addAction(QStringLiteral("删除"), this, [this, id]() { deleteRecipe(id); }); } menu.exec(m_systemSettingsTree->viewport()->mapToGlobal(position)); return;
        }
        if (item == nullptr || item->data(0, Qt::UserRole).toString() != QStringLiteral("dataCollection")) { return; }''')
edit('src/main_window_workflow.cpp','        item = new QTreeWidgetItem(settings, {label}); item->setData(0, Qt::UserRole, QStringLiteral("unsupported"));\n    }\n    decorateNativeSystemTree();','''        item = new QTreeWidgetItem(settings, {label}); item->setData(0, Qt::UserRole, label == QStringLiteral("配方") ? QStringLiteral("recipe") : QStringLiteral("unsupported")); //20261113-2 change
        if (label == QStringLiteral("配方")) { Q_FOREACH (collection, m_project.configurationNodes) { if (collection.kind != QStringLiteral("recipe") || !collection.active || !collection.implemented) { continue; } collectionItem = new QTreeWidgetItem(item, {collection.name}); collectionItem->setData(0, Qt::UserRole, QStringLiteral("recipe")); collectionItem->setData(0, Qt::UserRole + 1, collection.id); } }
    }
    decorateNativeSystemTree();''')
edit('src/hmi_canvas.h','    void rebuildPage(int width, int height, bool preserveItems);','    QMap<QString, QStringList> m_recipeTitles; //20261113-2 add: Transient resolved context is never copied into HmiObject or KSP.\n    QMap<QString, int> m_recipeCounts;\n    HmiControlItem *makeControlItem(const HmiObject &object);\n    void bindRecipeContext(HmiControlItem *item);\n    void rebuildPage(int width, int height, bool preserveItems);')
edit('src/hmi_canvas.cpp','    QList<HmiObject>::const_iterator object_iterator;\n\n    cancelInteraction();','''    QList<HmiObject>::const_iterator object_iterator;
    HmiConfigurationNode recipe;
    RecipeProperties properties;
    QStringList titles;

    m_recipeTitles.clear(); m_recipeCounts.clear(); //20261113-2 add: Resolve live definitions once for this page rebuild.
    Q_FOREACH (recipe, project.configurationNodes) { if (recipe.kind != QStringLiteral("recipe") || !recipe.active || !recipe.implemented || !RecipeProperties::fromJson(recipe.properties, &properties)) { continue; } titles = QStringList{QStringLiteral("序号"), recipe.name}; titles.append(properties.itemNames.mid(0, properties.length)); m_recipeTitles.insert(recipe.id, titles); m_recipeCounts.insert(recipe.id, properties.count); }
    cancelInteraction();''')
p=root/'src/hmi_canvas.cpp';s=sources[str(p.relative_to(root)).replace('\\','/')]
for before,after in [('new HmiControlItem(*object_iterator)','makeControlItem(*object_iterator)'),('new HmiControlItem(object)','makeControlItem(object)'),('new HmiControlItem(previous_objects.at(item_index))','makeControlItem(previous_objects.at(item_index))')]:
    assert before in s,before;s=s.replace(before,after)
s=s.replace('        control_item->setObject(object);','        control_item->setObject(object); bindRecipeContext(control_item); //20261113-2 change')
s+=f('ScreenCanvas::bindRecipeContext','Attach resolved headers and bounded illustrative row counts without modifying persistent object fields.','void ScreenCanvas::bindRecipeContext(HmiControlItem *item)', '''    HmiObject object = item->toHmiObject();
    item->setData(12, m_recipeTitles.value(object.recipeDisplay.recipeId)); item->setData(13, m_recipeCounts.value(object.recipeDisplay.recipeId));''','item')
s+=f('ScreenCanvas::makeControlItem','Create every page/history/placement item through one transient recipe-context binder.','HmiControlItem *ScreenCanvas::makeControlItem(const HmiObject &object)', '''    HmiControlItem *item = new HmiControlItem(object);
    item->setData(10, m_language); bindRecipeContext(item); return item;''','object','New owned control item.')
sources['src/hmi_canvas.cpp']=s
p=root/'src/stage4_control_actions.cpp';s=sources.get('src/stage4_control_actions.cpp',p.read_text(encoding='utf-8'))
s+=f('MainWindow::configureRecipe','Commit one accepted recipe definition as a complete undoable project transaction.','void MainWindow::configureRecipe(const QString &id)', '''    HmiProject candidate = currentEditorProject();
    HmiConfigurationNode node;
    HmiConfigurationNode peer;
    RecipeDialog *dialog;
    QString error;
    int index;
    int existing = -1;
    if (!m_hasProject) { return; }
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { peer = candidate.configurationNodes[index]; if (!id.isEmpty() && peer.id == id && peer.kind == QStringLiteral("recipe")) { node = peer; existing = index; } if (id.isEmpty() && peer.kind == QStringLiteral("recipe") && peer.active && peer.implemented) { QMessageBox::warning(this, QStringLiteral("KinSealStudio"), QStringLiteral("已经达到最大配方数量，请删除后再新建。")); return; } }
    if (!id.isEmpty() && existing < 0) { return; } if (id.isEmpty()) { node.kind = QStringLiteral("recipe"); }
    dialog = new RecipeDialog(node, candidate, this); if (dialog->exec() != QDialog::Accepted) { delete dialog; return; }
    if (existing < 0) { candidate.configurationNodes.append(dialog->node()); } else { candidate.configurationNodes[existing] = dialog->node(); } delete dialog;
    if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("配方未提交"), error); }''','id')
s+=f('MainWindow::deleteRecipe','Refuse removal while any active or inactive page consumes the stable recipe definition.','void MainWindow::deleteRecipe(const QString &id)', '''    HmiProject candidate = currentEditorProject();
    QString error;
    int index;
    for (index = 0; index < candidate.configurationNodes.size(); ++index) { if (candidate.configurationNodes[index].id == id && candidate.configurationNodes[index].kind == QStringLiteral("recipe")) { candidate.configurationNodes.removeAt(index); if (!commitProjectDraft(candidate, &error)) { QMessageBox::warning(this, QStringLiteral("配方正在使用"), error); } return; } }''','id')
sources['src/stage4_control_actions.cpp']=s
for path,s in sources.items():(root/path).write_text(s,encoding='utf-8',newline='\n')
print('Connected single-recipe tree actions, whole-project deletion guards and transient canvas headers across rebuild/history/placement.')
