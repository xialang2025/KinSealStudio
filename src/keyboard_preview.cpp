#include "keyboard_preview.h"
#include "bit_button_renderer.h"
#include "control_rendering.h"
#include "project_model.h"
#include "static_text_renderer.h"

#include <QDialog>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollArea>
#include <QVBoxLayout>
#include <algorithm>
#include <utility>

namespace {

class KeyboardPreviewSurface : public QWidget
{
public:
    /******************************************************************************
     * Function Name   : KeyboardPreviewSurface
     * Description     : Freeze one keyboard page and retain the editor layer order for painting and touch.
     * Input Parameters: project, input, dialog
     * Return Value    : None.
     * Date            : 2026.11.23
     ******************************************************************************/
    KeyboardPreviewSurface(const HmiProject &project, QLineEdit *input, QDialog *dialog) //20261123-26 add
        : QWidget(dialog), m_project(project), m_input(input), m_dialog(dialog)
    {
        const QString bitmapName = m_project.activePageBackground.bitmapName;

        setObjectName(QStringLiteral("KeyboardPreviewSurface"));
        setFixedSize(m_project.pageWidth, m_project.pageHeight);
        if (!bitmapName.isEmpty())
        {
            m_bitmap = decodedControlPng(m_project.pictures.value(bitmapName));
        }
        for (const HmiObject &object : std::as_const(m_project.objects))
        {
            m_ordered.append(&object);
        }
        std::stable_sort(m_ordered.begin(), m_ordered.end(), [](const HmiObject *left, const HmiObject *right) {
            return left->layer < right->layer;
        });
    }

protected:
    /******************************************************************************
     * Function Name   : paintEvent
     * Description     : Reuse the editor renderers for the preview canvas at the saved positions.
     * Input Parameters: event
     * Return Value    : None.
     * Date            : 2026.11.23
     ******************************************************************************/
    void paintEvent(QPaintEvent *event) override //20261123-26 add
    {
        QPainter painter(this);
        QRectF area;
        bool pressed;
        int state;

        Q_UNUSED(event);
        painter.fillRect(rect(), m_project.activePageBackground.brush(rect(), m_bitmap));
        for (const HmiObject *object : std::as_const(m_ordered))
        {
            area = areaFor(*object);
            painter.save(); //20261124-15 add
            painter.setWorldTransform(object->appearanceTransform(area), true);
            if (object->type == QStringLiteral("text"))
            {
                paintStaticText(&painter, area, staticTextContent(*object, m_project.activeLanguage), object->staticText);
            }
            else if (object->type == QStringLiteral("switch") && object->hasBitButtonProperties)
            {
                pressed = object == m_pressed;
                state = (pressed && object->hasFunctionButtonProperties && object->functionButton.pressedEffect) ||
                    (m_caps && isCapsButton(*object)) ? 1 : 0;
                paintBitButton(&painter, area, object->bitButton.states[state], m_project.activeLanguage, pressed);
            }
            else if (isSupportedGraphic(object->type))
            {
                paintGraphic(&painter, object->type, area, object->graphic);
            }
            painter.restore(); //20261124-15 add
        }
    }

    /******************************************************************************
     * Function Name   : mousePressEvent
     * Description     : Capture the topmost keyboard component beneath the pointer.
     * Input Parameters: event
     * Return Value    : None.
     * Date            : 2026.11.23
     ******************************************************************************/
    void mousePressEvent(QMouseEvent *event) override //20261123-26 add
    {
        if (event->button() != Qt::LeftButton)
        {
            return;
        }
        m_pressed = keyAt(event->position());
        update();
    }

    /******************************************************************************
     * Function Name   : mouseReleaseEvent
     * Description     : Dispatch one action after releasing over the same keyboard component.
     * Input Parameters: event
     * Return Value    : None.
     * Date            : 2026.11.23
     ******************************************************************************/
    void mouseReleaseEvent(QMouseEvent *event) override //20261123-26 add
    {
        const HmiObject *pressed;

        if (event->button() != Qt::LeftButton)
        {
            return;
        }
        pressed = m_pressed;
        m_pressed = nullptr;
        update();
        if (pressed != nullptr && pressed == keyAt(event->position()))
        {
            if (isCapsButton(*pressed))
            {
                m_caps = !m_caps;
            }
            else
            {
                applyKey(pressed->functionButton);
            }
        }
    }

private:
    /******************************************************************************
     * Function Name   : areaFor
     * Description     : Apply saved subpixel coordinates equally to painting and hit testing.
     * Input Parameters: object
     * Return Value    : Object rectangle in keyboard page coordinates.
     * Date            : 2026.11.23
     ******************************************************************************/
    static QRectF areaFor(const HmiObject &object) //20261123-26 add
    {
        return QRectF(object.geometry).translated(object.subpixelOffset);
    }

    /******************************************************************************
     * Function Name   : isCapsButton
     * Description     : Recognize the original full ASCII template's toggle without operating its address.
     * Input Parameters: object
     * Return Value    : True for the observed Caps indicator button.
     * Date            : 2026.11.23
     ******************************************************************************/
    static bool isCapsButton(const HmiObject &object) //20261123-26 add
    {
        return object.type == QStringLiteral("switch") && object.hasBitButtonProperties &&
            !object.hasFunctionButtonProperties && !object.hasWordButtonProperties && !object.hasLampProperties &&
            object.address == QStringLiteral("LB4014") && object.bitButton.operation == 3 && object.bitButton.indicator;
    }

    /******************************************************************************
     * Function Name   : keyAt
     * Description     : Resolve an interactive keyboard component without dispatching other page controls.
     * Input Parameters: point
     * Return Value    : Topmost matching keyboard component, or null.
     * Date            : 2026.11.23
     ******************************************************************************/
    const HmiObject *keyAt(const QPointF &point) const //20261123-26 add
    {
        for (auto item = m_ordered.crbegin(); item != m_ordered.crend(); ++item)
        {
            const HmiObject &object = **item;

            if (areaFor(object).contains(point) && (isCapsButton(object) ||
                (object.type == QStringLiteral("switch") && object.hasBitButtonProperties &&
                 object.hasFunctionButtonProperties && object.functionButton.function == 14)))
            {
                return &object;
            }
        }
        return nullptr;
    }

    /******************************************************************************
     * Function Name   : characterFor
     * Description     : Mirror the device's two-way ASCII character pairs while the local Caps state is active.
     * Input Parameters: text
     * Return Value    : One key's input text with the matching variant applied.
     * Date            : 2026.11.23
     ******************************************************************************/
    QString characterFor(const QString &text) const //20261123-26 add
    {
        static const QString unshifted = QStringLiteral("\x60" "1234567890-=qwertyuiop[]\\asdfghjkl;'zxcvbnm,./");
        static const QString shifted = QStringLiteral("~!@#$%^&*()_+QWERTYUIOP{}|ASDFGHJKL:\"ZXCVBNM<>?");
        qsizetype index;

        if (!m_caps || text.size() != 1)
        {
            return text;
        }
        // AMZ070W01 firmware selects the counterpart from the paired tables only when LB4014 equals one.
        index = unshifted.indexOf(text.at(0));
        if (index >= 0)
        {
            return QString(shifted.at(index));
        }
        index = shifted.indexOf(text.at(0));
        return index >= 0 ? QString(unshifted.at(index)) : text;
    }

    /******************************************************************************
     * Function Name   : applyKey
     * Description     : Route the saved Function Button key enum to the local preview input only.
     * Input Parameters: key
     * Return Value    : None.
     * Date            : 2026.11.23
     ******************************************************************************/
    void applyKey(const FunctionButtonProperties &key) //20261123-26 add
    {
        m_input->setFocus(Qt::MouseFocusReason);
        // Qt's line editor keeps cursor movement and selection replacement consistent with typed input.
        switch (key.keyboardKey)
        {
        case 0: m_dialog->accept(); break;
        case 1: m_input->backspace(); break;
        case 2: m_input->clear(); break;
        case 3: m_dialog->reject(); break;
        case 4: m_input->del(); break;
        case 5: m_caps = !m_caps; break;
        case 6: m_input->cursorBackward(false); break;
        case 7: m_input->cursorForward(false); break;
        case 8: m_input->insert(QStringLiteral(" ")); break;
        case 9: m_input->insert(characterFor(key.keyboardText)); break;
        }
    }

    const HmiProject m_project;
    QLineEdit *m_input;
    QDialog *m_dialog;
    QList<const HmiObject *> m_ordered;
    QImage m_bitmap;
    const HmiObject *m_pressed = nullptr;
    bool m_caps = false;
};

} // namespace

/******************************************************************************
 * Function Name   : showStage2KeyboardPreview
 * Description     : Observe the active keyboard layout and test its actions in a private text editor.
 * Input Parameters: parent, project
 * Return Value    : Accepted text, or no value after cancellation or for a non-keyboard page.
 * Date            : 2026.11.23
 ******************************************************************************/
std::optional<QString> showStage2KeyboardPreview(QWidget *parent, const HmiProject &project) //20261123-26 add
{
    QDialog dialog(parent);
    QVBoxLayout *layout;
    QLineEdit *input;
    QScrollArea *scroll;
    KeyboardPreviewSurface *surface;

    if (project.activePageKind != QStringLiteral("keyboard"))
    {
        return std::nullopt;
    }
    dialog.setObjectName(QStringLiteral("Stage2KeyboardPreview"));
    dialog.setWindowTitle(QStringLiteral("键盘预览 — %1").arg(project.pageName));
    layout = new QVBoxLayout(&dialog);
    input = new QLineEdit(&dialog);
    input->setObjectName(QStringLiteral("KeyboardPreviewInput"));
    input->setPlaceholderText(QStringLiteral("在此预览键盘输入"));
    layout->addWidget(input);
    scroll = new QScrollArea(&dialog);
    scroll->setFrameShape(QFrame::NoFrame);
    surface = new KeyboardPreviewSurface(project, input, &dialog);
    scroll->setWidget(surface);
    layout->addWidget(scroll);
    dialog.resize(qMin(project.pageWidth + 24, 824), qMin(project.pageHeight + 54, 534));
    input->setFocus();
    QObject::connect(input, &QLineEdit::returnPressed, &dialog, &QDialog::accept);
    if (dialog.exec() != QDialog::Accepted)
    {
        return std::nullopt;
    }
    return input->text();
}
