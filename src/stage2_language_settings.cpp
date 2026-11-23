#include "main_window.h"
#include "local_help.h"

#include <QComboBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QJsonArray>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QSet>
#include <QSignalBlocker>

namespace {

/******************************************************************************
 * Function Name   : renameLanguageKeys
 * Description     : Move per-language values with their slots, including simultaneous swaps.
 * Input Parameters: values, renames, error
 * Return Value    : True when each former label maps to a destination without losing text.
 * Date            : 2026.11.23
 ******************************************************************************/
bool renameLanguageKeys(QJsonObject *values, const QMap<QString, QString> &renames, QString *error) //20261123-19 add
{
    const QJsonObject previous = *values;
    QMap<QString, QString>::const_iterator name;

    for (name = renames.cbegin(); name != renames.cend(); ++name)
    {
        values->remove(name.key());
    }
    for (name = renames.cbegin(); name != renames.cend(); ++name)
    {
        if (!previous.contains(name.key()))
        {
            continue;
        }
        if (values->contains(name.value()) && values->value(name.value()) != previous.value(name.key()))
        {
            *error = QStringLiteral("语言“%1”已有保留的译文，请换一个名称。").arg(name.value());
            return false;
        }
        values->insert(name.value(), previous.value(name.key()));
    }
    return true;
}

/******************************************************************************
 * Function Name   : renameLocalizedFields
 * Description     : Update typed localization maps while retaining opaque extension data.
 * Input Parameters: value, renames, inHistoryData, inMessageDisplay, error
 * Return Value    : True when the localized draft has been remapped without collisions.
 * Date            : 2026.11.23
 ******************************************************************************/
bool renameLocalizedFields(QJsonValue *value, const QMap<QString, QString> &renames,
                           bool inHistoryData, bool inMessageDisplay, QString *error) //20261123-19 add
{
    QJsonObject object;
    QJsonObject localized;
    QJsonArray entries;
    QJsonValue child;
    int index;

    if (value->isArray())
    {
        entries = value->toArray();
        for (index = 0; index < entries.size(); ++index)
        {
            child = entries.at(index);
            if (!renameLocalizedFields(&child, renames, inHistoryData, inMessageDisplay, error))
            {
                return false;
            }
            entries.replace(index, child);
        }
        *value = entries;
    }
    if (!value->isObject())
    {
        return true;
    }
    object = value->toObject();
    for (const QString &key : object.keys())
    {
        if (key == QStringLiteral("extensions"))
        {
            continue;
        }
        child = object.value(key);
        if (child.isObject() && (key == QStringLiteral("texts") || key == QStringLiteral("captions") ||
            key == QStringLiteral("messages") || key == QStringLiteral("languageAppearances") ||
            key == QStringLiteral("languageFrameColors") || (inHistoryData && key == QStringLiteral("names"))))
        {
            localized = child.toObject();
            if (!renameLanguageKeys(&localized, renames, error))
            {
                return false;
            }
            child = localized;
            if (key == QStringLiteral("languageAppearances") && !renameLocalizedFields(&child, renames, false, false, error))
            {
                return false;
            }
        }
        else if (inMessageDisplay && key == QStringLiteral("states"))
        {
            entries = child.toArray();
            for (index = 0; index < entries.size(); ++index)
            {
                localized = entries.at(index).toObject();
                if (!renameLanguageKeys(&localized, renames, error))
                {
                    return false;
                }
                entries.replace(index, localized);
            }
            child = entries;
        }
        else if (!renameLocalizedFields(&child, renames, inHistoryData || key == QStringLiteral("historyData"),
                                        key == QStringLiteral("messageDisplay"), error))
        {
            return false;
        }
        object.insert(key, child);
    }
    *value = object;
    return true;
}

/******************************************************************************
 * Function Name   : applyLanguageRows
 * Description     : Relabel existing slots before changing the count and editor selection.
 * Input Parameters: source, names, selected, draft, error
 * Return Value    : True only if the full language edit can be saved.
 * Date            : 2026.11.23
 ******************************************************************************/
bool applyLanguageRows(const HmiProject &source, const QStringList &names, int selected,
                       HmiProject *draft, QString *error) //20261123-19 add
{
    QStringList renamed = source.languages;
    QMap<QString, QString> aliases;
    HmiProject candidate = source;
    QJsonObject json;
    QJsonValue fields;
    QString field;
    int index;

    if (selected < 0 || selected >= names.size())
    {
        *error = QStringLiteral("请选择当前语言。");
        return false;
    }
    if (names.contains(QString()) || QSet<QString>(names.cbegin(), names.cend()).size() != names.size())
    {
        *error = QStringLiteral("语言名称不能为空或重复。");
        return false;
    }
    for (index = 0; index < qMin(names.size(), renamed.size()); ++index)
    {
        if (renamed.at(index) != names.at(index))
        {
            aliases.insert(renamed.at(index), names.at(index));
            renamed[index] = names.at(index);
        }
    }
    if (!aliases.isEmpty())
    {
        json = source.toJson();
        json.insert(QStringLiteral("languages"), QJsonArray::fromStringList(renamed));
        json.insert(QStringLiteral("activeLanguage"), aliases.value(source.activeLanguage, source.activeLanguage));
        if (!source.startupLanguage.isEmpty())
        {
            json.insert(QStringLiteral("startupLanguage"), aliases.value(source.startupLanguage, source.startupLanguage));
        }
        Q_FOREACH (field, (QStringList{QStringLiteral("pages"), QStringLiteral("configurationNodes")}))
        {
            if (json.contains(field))
            {
                fields = json.value(field);
                if (!renameLocalizedFields(&fields, aliases, false, false, error))
                {
                    return false;
                }
                json.insert(field, fields);
            }
        }
        if (!HmiProject::fromJson(json, &candidate, error))
        {
            return false;
        }
        candidate.filePassword = source.filePassword;
    }
    if (candidate.languages != names && !candidate.setLanguages(names, error))
    {
        return false;
    }
    candidate.activeLanguage = names.at(selected);
    if (!candidate.validateModel(error))
    {
        return false;
    }
    *draft = candidate;
    return true;
}

} // namespace

/******************************************************************************
 * Function Name   : MainWindow::configureLanguages
 * Description     : Edit numbered language slots and the current editing language as one draft.
 * Input Parameters: None.
 * Return Value    : None.
 * Date            : 2026.11.23
 ******************************************************************************/
void MainWindow::configureLanguages() //20261123-19 change
{
    const HmiProject source = currentEditorProject();
    HmiProject draft;
    QDialog dialog(this);
    QLabel countLabel(QStringLiteral("语言总数："), &dialog);
    QLabel currentLabel(QStringLiteral("当前语言："), &dialog);
    QComboBox count(&dialog);
    QComboBox current(&dialog);
    QDialogButtonBox buttons(QDialogButtonBox::Ok | QDialogButtonBox::Cancel | QDialogButtonBox::Help, &dialog);
    QLabel validation(&dialog);
    QList<QLabel *> rowLabels;
    QList<QLineEdit *> rowNames;
    const QStringList presets = {QStringLiteral("中文"), QStringLiteral("英语"), QStringLiteral("德语"),
        QStringLiteral("韩语"), QStringLiteral("日语"), QStringLiteral("法语")};
    const int rowCount = qMax(int(presets.size()), int(source.languages.size()));
    const int extraHeight = (rowCount - presets.size()) * 30;
    QLabel *label;
    QLineEdit *edit;
    QString error;
    int row;

    if (!m_hasProject)
    {
        return;
    }
    dialog.setObjectName(QStringLiteral("Kns_MulLanguageDlg"));
    dialog.setWindowTitle(QStringLiteral("语言设置"));
    dialog.setWindowFlag(Qt::WindowContextHelpButtonHint, true);
    dialog.setFixedSize(400, 370 + extraHeight);
    dialog.setProperty("localHelpTopic", QStringLiteral("language-settings"));
    dialog.setStyleSheet(QStringLiteral("QDialog#Kns_MulLanguageDlg { background: #efefef; }"
        "QComboBox, QLineEdit { background: #caeace; }"));
    countLabel.setGeometry(60, 30, 80, 21);
    currentLabel.setGeometry(60, 60, 80, 21);
    count.setObjectName(QStringLiteral("lanComboBox"));
    current.setObjectName(QStringLiteral("curLanComboBox"));
    count.setGeometry(140, 30, 141, 22);
    current.setGeometry(141, 60, 142, 22);
    for (row = 0; row < rowCount; ++row)
    {
        count.addItem(QString::number(row + 1), row + 1);
        label = new QLabel(QString::number(row + 1), &dialog);
        edit = new QLineEdit(source.languages.value(row, presets.value(row)), &dialog);
        label->setObjectName(QStringLiteral("lanNumLabel_%1").arg(row + 1));
        edit->setObjectName(QStringLiteral("lineEdit_%1").arg(row + 1));
        label->setGeometry(60, 90 + row * 30, 25, 21);
        edit->setGeometry(142, 90 + row * 30, 141, 21);
        edit->setMaxLength(256);
        rowLabels.append(label);
        rowNames.append(edit);
        connect(edit, &QLineEdit::textChanged, &dialog, [&current, row](const QString &text) {
            if (row < current.count())
            {
                current.setItemText(row, text);
            }
        });
    }
    auto updateRows = [&]() {
        const QSignalBlocker blocked(&current);
        const int visibleRows = count.currentData().toInt();
        const int previouslySelected = current.currentIndex();
        int index;

        current.clear();
        for (index = 0; index < rowNames.size(); ++index)
        {
            rowLabels.at(index)->setVisible(index < visibleRows);
            rowNames.at(index)->setVisible(index < visibleRows);
            if (index < visibleRows)
            {
                current.addItem(rowNames.at(index)->text());
            }
        }
        current.setCurrentIndex(previouslySelected >= 0 && previouslySelected < visibleRows ? previouslySelected : 0);
    };
    count.setCurrentIndex(source.languages.size() - 1);
    updateRows();
    current.setCurrentIndex(source.languages.indexOf(source.activeLanguage));
    validation.setGeometry(20, 278 + extraHeight, 360, 31);
    validation.setWordWrap(true);
    buttons.setObjectName(QStringLiteral("buttonBox"));
    buttons.setGeometry(135, 310 + extraHeight, 235, 32);
    buttons.button(QDialogButtonBox::Ok)->setText(QStringLiteral("确定"));
    buttons.button(QDialogButtonBox::Cancel)->setText(QStringLiteral("取消"));
    buttons.button(QDialogButtonBox::Help)->setText(QStringLiteral("帮助"));
    connect(&count, &QComboBox::currentIndexChanged, &dialog, updateRows);
    connect(&buttons, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    connect(&buttons, &QDialogButtonBox::helpRequested, &dialog, [this, &dialog]() {
        m_localHelp->showTopic(QStringLiteral("language-settings"), &dialog);
    });
    connect(&buttons, &QDialogButtonBox::accepted, &dialog, [&]() {
        QStringList names;
        int index;

        for (index = 0; index < count.currentData().toInt(); ++index)
        {
            names.append(rowNames.at(index)->text().trimmed());
        }
        if (!applyLanguageRows(source, names, current.currentIndex(), &draft, &error))
        {
            validation.setText(error);
            return;
        }
        dialog.accept();
    });
    if (dialog.exec() == QDialog::Accepted)
    {
        commitProjectDraft(draft);
    }
}
