//20261127-11 add: Alternate editor for a fully parsed native stream with unmapped canvas properties.
class NativeRootDelegate final : public QStyledItemDelegate
{
public:
    using QStyledItemDelegate::QStyledItemDelegate;
    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override
    {
        if (index.column() != 2 || !index.parent().isValid()) { return nullptr; }
        return QStyledItemDelegate::createEditor(parent, option, index);
    }
};

class NativeSourceDialog final : public QDialog
{
public:
    NativeSourceDialog(QWidget *parent, const QString &fileName, const QByteArray &bytes, const QString &ownerPassword,
                       const NativeSourceIndex &index, const QString &diagnostic)
        : QDialog(parent), path(fileName), source(bytes), password(ownerPassword)
    {
        QVBoxLayout *layout = new QVBoxLayout(this);
        QLabel *details = new QLabel(dialogButtonText(QStringLiteral("部分画布属性尚未映射。可修改工程名、图元层级和移动锁。"), "Edit the project name, layer and movement lock while retaining all native records."), this);
        QFormLayout *form = new QFormLayout();
        QDialogButtonBox *buttons = new QDialogButtonBox(this);
        QPushButton *saveButton;
        QPushButton *copyButton;
        QPushButton *closeButton;
        QTreeWidgetItem *top;
        QTreeWidgetItem *item;

        setObjectName(QStringLiteral("nativeSourceEditor"));
        refreshTitle();
        resize(700, 450);
        details->setWordWrap(true);
        details->setToolTip(diagnostic);
        layout->addWidget(details);
        nameEdit = new QLineEdit(index.title, this);
        nameEdit->setObjectName(QStringLiteral("nativeProjectName"));
        nameEdit->setMaxLength(256);
        form->addRow(dialogButtonText(QStringLiteral("工程名称"), "Project name"), nameEdit);
        form->addRow(dialogButtonText(QStringLiteral("原厂型号"), "Model"), new QLabel(index.model, this));
        layout->addLayout(form);
        tree = new QTreeWidget(this);
        tree->setObjectName(QStringLiteral("nativeSourceRoots"));
        tree->setHeaderLabels({dialogButtonText(QStringLiteral("画面 / 元件"), "Screen / object"), dialogButtonText(QStringLiteral("原厂类型"), "Native type"),
                               dialogButtonText(QStringLiteral("层级"), "Layer"), dialogButtonText(QStringLiteral("允许移动"), "Movable")});
        tree->setItemDelegate(new NativeRootDelegate(tree));
        tree->setAlternatingRowColors(true);
        for (const NativeSourcePage &page : index.pages) {
            top = new QTreeWidgetItem(tree, {page.number + QLatin1Char(' ') + page.name});
            for (const NativeSourceRoot &root : page.roots) {
                item = new QTreeWidgetItem(top, {root.name.isEmpty() ? root.prefix.chopped(1) : root.name,
                                                QString::number(root.type), QString::number(root.layer, 'g', 17)});
                item->setData(0, Qt::UserRole, root.prefix);
                item->setFlags(item->flags() | Qt::ItemIsEditable | Qt::ItemIsUserCheckable);
                item->setCheckState(3, root.movable ? Qt::Checked : Qt::Unchecked);
            }
        }
        if (tree->topLevelItemCount()) { tree->topLevelItem(0)->setExpanded(true); }
        tree->header()->setSectionResizeMode(0, QHeaderView::Stretch);
        layout->addWidget(tree);
        saveButton = buttons->addButton(dialogButtonText(QStringLiteral("保存"), "Save"), QDialogButtonBox::ActionRole);
        copyButton = buttons->addButton(dialogButtonText(QStringLiteral("另存为"), "Save As"), QDialogButtonBox::ActionRole);
        closeButton = buttons->addButton(dialogButtonText(QStringLiteral("关闭"), "Close"), QDialogButtonBox::RejectRole);
        saveButton->setObjectName(QStringLiteral("nativeSourceSave"));
        layout->addWidget(buttons);
        connect(tree, &QTreeWidget::itemChanged, this, [this]() { dirty = true; });
        connect(nameEdit, &QLineEdit::textChanged, this, [this]() { dirty = true; });
        connect(saveButton, &QPushButton::clicked, this, [this]() { save(false); });
        connect(copyButton, &QPushButton::clicked, this, [this]() { save(true); });
        connect(closeButton, &QPushButton::clicked, this, &NativeSourceDialog::reject);
    }

    void reject() override
    {
        if (dirty) {
            QMessageBox prompt(QMessageBox::Question, windowTitle(),
                               dialogButtonText(QStringLiteral("是否保存对工程的修改？"), "Save your changes?"), QMessageBox::NoButton, this);
            auto saveChoice = prompt.addButton(dialogButtonText(QStringLiteral("保存"), "Save"), QMessageBox::AcceptRole);
            auto discardChoice = prompt.addButton(dialogButtonText(QStringLiteral("放弃修改"), "Discard"), QMessageBox::DestructiveRole);
            prompt.addButton(dialogButtonText(QStringLiteral("取消"), "Cancel"), QMessageBox::RejectRole);
            prompt.exec();
            if (prompt.clickedButton() == saveChoice) { if (!save(false)) { return; } }
            else if (prompt.clickedButton() != discardChoice) { return; }
        }
        QDialog::reject();
    }

private:
    void refreshTitle()
    {
        setWindowTitle(dialogButtonText(QStringLiteral("原厂源工程"), "Native source project") + QStringLiteral(" — ") + QFileInfo(path).fileName());
    }

    bool save(bool choose)
    {
        QList<NativeSourceEdit> edits;
        QByteArray candidate;
        QString destination = path;
        QString error;
        QTreeWidgetItem *item;
        bool ok;
        double layer;
        int p;
        int i;

        if (choose) {
            destination = QFileDialog::getSaveFileName(this, dialogButtonText(QStringLiteral("原厂工程另存为"), "Save native project as"), path,
                                                        dialogButtonText(QStringLiteral("原厂工程 (*.sm)"), "Native projects (*.sm)"));
            if (destination.isEmpty()) { return false; }
            if (!destination.endsWith(QStringLiteral(".sm"), Qt::CaseInsensitive)) { destination += QStringLiteral(".sm"); }
        }
        for (p = 0; p < tree->topLevelItemCount() && error.isEmpty(); ++p) {
            for (i = 0; i < tree->topLevelItem(p)->childCount(); ++i) {
                item = tree->topLevelItem(p)->child(i);
                layer = item->text(2).toDouble(&ok);
                if (!ok || !std::isfinite(layer)) { error = dialogButtonText(QStringLiteral("层级必须是有限数值。"), "Layer must be a finite number."); break; }
                edits.append({item->data(0, Qt::UserRole).toString(), item->checkState(3) == Qt::Checked, layer});
            }
        }
        if (error.isEmpty() && QFileInfo(destination) == QFileInfo(path)) {
            QFile previous(path);
            if (!previous.open(QIODevice::ReadOnly) || previous.size() != source.size() || previous.readAll() != source) {
                error = dialogButtonText(QStringLiteral("原文件已被更改或移除，请另存为新文件。"), "The original file has changed. Save to a different file.");
            }
        }
        if (!error.isEmpty() || !editNativeSource(source, password, nameEdit->text(), edits, &candidate, &error) ||
            !saveNativeSourceFile(destination, candidate, &error)) {
            QMessageBox::warning(this, windowTitle(), error);
            return false;
        }
        source = candidate;
        path = destination;
        dirty = false;
        refreshTitle();
        return true;
    }

    QString path;
    QByteArray source;
    QString password;
    QLineEdit *nameEdit;
    QTreeWidget *tree;
    bool dirty = false;
};
