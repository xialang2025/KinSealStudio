#ifndef DIALOG_BUTTON_TEXT_H
#define DIALOG_BUTTON_TEXT_H

#include <QCoreApplication>
#include <QString>
#include <QVariant>

inline constexpr char EnglishDialogButtonsProperty[] = "kinSealEnglishDialogButtons"; //20261124-8 add

/******************************************************************************
 * Function Name   : dialogButtonText
 * Description     : Use the selected UI language for an explicitly captioned dialog button.
 * Input Parameters: chinese, english
 * Return Value    : Button caption in the editor's language.
 * Date            : 2026.11.24
 ******************************************************************************/
inline QString dialogButtonText(const QString &chinese, const char *english) //20261124-8 add
{
    const QCoreApplication *application = QCoreApplication::instance();

    if (application != nullptr && application->property(EnglishDialogButtonsProperty).toBool())
    {
        return QString::fromLatin1(english);
    }
    return chinese;
}

#endif
