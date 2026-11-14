#include "simulation_window.h"
#include "bit_button_renderer.h"
#include "static_text_renderer.h"

#include <QCloseEvent>
#include <QComboBox>
#include <QEvent>
#include <QFocusEvent>
#include <QFormLayout>
#include <QHBoxLayout>
#include <QInputDialog>
#include <QLabel>
#include <QLineEdit>
#include <QMouseEvent>
#include <QPainter>
#include <QPushButton>
#include <QPointer>
#include <QSignalBlocker>
#include <QSpinBox>
#include <QVBoxLayout>
#include <algorithm>

/******************************************************************************
 * Function Name   : orderedSimulationObjects
 * Description     : Retain stable insertion order for equal layers in both painting and hit testing.
 * Input Parameters: runtime
 * Return Value    : Ordered immutable object pointers.
 * Date            : 2026.11.14
 ******************************************************************************/
static QList<const HmiObject *> orderedSimulationObjects(const SimulationRuntime &runtime) //20261114-6 add
{
    QList<const HmiObject *> ordered;

    for (const HmiObject &entry : runtime.project().objects) { ordered.append(&entry); }
    std::stable_sort(ordered.begin(), ordered.end(), [](const HmiObject *left, const HmiObject *right) {
        return left->layer < right->layer;
    });
    return ordered;
}

/******************************************************************************
 * Function Name   : simulationFrame
 * Description     : Render the runtime snapshot with the same five-family text and artwork painters as the editor.
 * Input Parameters: runtime
 * Return Value    : One 800 by 480 PC image without device pixel packing or physical I/O.
 * Date            : 2026.11.14
 ******************************************************************************/
QImage simulationFrame(const SimulationRuntime &runtime) //20261114-6 add
{
    QImage frame(800, 480, QImage::Format_ARGB32_Premultiplied);
    QPainter painter;
    QList<const HmiObject *> ordered;
    const HmiObject *object;
    const BitButtonState *appearance;
    int state;

    frame.fill(QColor(197, 222, 255));
    painter.begin(&frame);
    painter.setClipRect(QRect(0, 0, 800, 480));
    ordered = orderedSimulationObjects(runtime);
    Q_FOREACH (object, ordered)
    {
        if (!runtime.isVisible(*object)) { continue; }
        if (object->type == QStringLiteral("text"))
        {
            paintStaticText(&painter, QRectF(object->geometry), staticTextContent(*object, runtime.language()), object->staticText);
            continue;
        }
        state = runtime.visualState(*object);
        if (state < 0) { continue; }
        appearance = object->hasLampProperties ? object->lamp.state(object->bitButton, state)
                                               : &object->bitButton.states[qBound(0, state, 1)];
        if (appearance != nullptr)
        {
            paintBitButton(&painter, QRectF(object->geometry), *appearance, runtime.language(), runtime.isPressed(object->id));
        }
    }
    painter.end();
    return frame;
}

/******************************************************************************
 * Function Name   : SimulationViewport::SimulationViewport
 * Description     : Create a fixed logical screen which cannot select, move, or mutate editor items.
 * Input Parameters: runtime, parent
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationViewport::SimulationViewport(SimulationRuntime *runtime, QWidget *parent) //20261114-6 add
    : QWidget(parent), m_runtime(runtime)
{
    setObjectName(QStringLiteral("simulationViewport"));
    setFixedSize(800, 480);
    setFocusPolicy(Qt::StrongFocus);
    setMouseTracking(true);
}

/******************************************************************************
 * Function Name   : SimulationViewport::setChangedCallback
 * Description     : Notify the owner after an atomic runtime input and optional keypad request.
 * Input Parameters: callback
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::setChangedCallback(const std::function<void(bool)> &callback) //20261114-6 add
{
    m_changed = callback;
}

/******************************************************************************
 * Function Name   : SimulationViewport::changed
 * Description     : Redraw a whole shared frame so hidden or transparent controls leave no stale pixels.
 * Input Parameters: inputReady
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::changed(bool inputReady) //20261114-6 add
{
    update();
    if (m_changed) { m_changed(inputReady); }
}

/******************************************************************************
 * Function Name   : SimulationViewport::hitObject
 * Description     : Resolve the top visible object before deciding whether its touch action is allowed.
 * Input Parameters: point
 * Return Value    : Top immutable object, or nullptr.
 * Date            : 2026.11.14
 ******************************************************************************/
const HmiObject *SimulationViewport::hitObject(const QPoint &point) const //20261114-6 add
{
    QList<const HmiObject *> ordered;
    int index;

    ordered = orderedSimulationObjects(*m_runtime);
    for (index = int(ordered.size()) - 1; index >= 0; --index)
    {
        if (m_runtime->isVisible(*ordered.at(index)) && ordered.at(index)->geometry.contains(point)) { return ordered.at(index); }
    }
    return nullptr;
}

/******************************************************************************
 * Function Name   : SimulationViewport::paintEvent
 * Description     : Paint the complete PC frame without reusing editable graphics-scene items.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::paintEvent(QPaintEvent *event) //20261114-6 add
{
    QPainter painter(this);

    Q_UNUSED(event);
    painter.drawImage(QPoint(0, 0), simulationFrame(*m_runtime));
}

/******************************************************************************
 * Function Name   : SimulationViewport::mousePressEvent
 * Description     : Start a single permitted topmost touch and retain its identity until release or cancellation.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::mousePressEvent(QMouseEvent *event) //20261114-6 add
{
    const HmiObject *object;
    QString error;

    if (event->button() != Qt::LeftButton || !m_runtime->running()) { return; }
    cancelInteraction(QStringLiteral("new-pointer-press"));
    object = hitObject(event->position().toPoint());
    if (object != nullptr && m_runtime->press(object->id, &error)) { m_activeObject = object->id; }
    changed();
}

/******************************************************************************
 * Function Name   : SimulationViewport::mouseMoveEvent
 * Description     : Cancel a held touch as soon as it leaves its original control or loses visibility.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::mouseMoveEvent(QMouseEvent *event) //20261114-6 add
{
    const HmiObject *object;
    bool inside;

    if (m_activeObject.isEmpty()) { return; }
    object = hitObject(event->position().toPoint());
    inside = object != nullptr && object->id == m_activeObject && m_runtime->touchAllowed(*object);
    if (!inside) { cancelInteraction(QStringLiteral("pointer-left-control")); }
}

/******************************************************************************
 * Function Name   : SimulationViewport::mouseReleaseEvent
 * Description     : Release the held touch before presenting any runtime numeric input dialog.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::mouseReleaseEvent(QMouseEvent *event) //20261114-6 add
{
    const HmiObject *object;
    bool inside;
    QString error;

    if (event->button() != Qt::LeftButton || m_activeObject.isEmpty()) { return; }
    object = hitObject(event->position().toPoint());
    inside = object != nullptr && object->id == m_activeObject;
    m_activeObject.clear();
    m_runtime->release(inside, &error);
    changed(inside);
}

/******************************************************************************
 * Function Name   : SimulationViewport::cancelInteraction
 * Description     : Clear a held momentary bit on pointer loss without cancelling an already released keypad request.
 * Input Parameters: reason
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::cancelInteraction(const QString &reason) //20261114-6 add
{
    if (m_activeObject.isEmpty()) { return; }
    m_activeObject.clear();
    m_runtime->cancel(reason);
    changed();
}

/******************************************************************************
 * Function Name   : SimulationViewport::pointerDown
 * Description     : Expose only the live pointer ownership needed by lifecycle tests.
 * Input Parameters: None
 * Return Value    : True while a runtime control owns the pointer.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationViewport::pointerDown() const //20261114-6 add
{
    return !m_activeObject.isEmpty();
}

/******************************************************************************
 * Function Name   : SimulationViewport::focusOutEvent
 * Description     : Release live momentary touches before focus moves to the inspector or another window.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationViewport::focusOutEvent(QFocusEvent *event) //20261114-6 add
{
    cancelInteraction(QStringLiteral("focus-lost"));
    QWidget::focusOutEvent(event);
}

/******************************************************************************
 * Function Name   : SimulationViewport::event
 * Description     : Handle lost capture independently of normal mouse-release delivery.
 * Input Parameters: event
 * Return Value    : QWidget event result.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationViewport::event(QEvent *event) //20261114-6 add
{
    if (event->type() == QEvent::UngrabMouse || event->type() == QEvent::WindowDeactivate)
    {
        cancelInteraction(QStringLiteral("capture-lost"));
    }
    return QWidget::event(event);
}

/******************************************************************************
 * Function Name   : SimulationWindow::SimulationWindow
 * Description     : Present the immutable runtime screen with explicitly local session input tools.
 * Input Parameters: parent
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationWindow::SimulationWindow(QWidget *parent) : QDialog(parent, Qt::Window) //20261114-6 add
{
    QVBoxLayout *outer;
    QHBoxLayout *content;
    QFormLayout *inspector;
    QVBoxLayout *side;
    QLabel *scope;
    QPushButton *write;
    QPushButton *close;

    setObjectName(QStringLiteral("offlineSimulationWindow"));
    setWindowTitle(QStringLiteral("离线模拟 — 内部地址"));
    outer = new QVBoxLayout(this);
    scope = new QLabel(QStringLiteral("单画面内部LB/LW模拟；初值为0，仅本次会话有效。宏不执行，不连接设备或PLC。"), this);
    scope->setObjectName(QStringLiteral("simulationScope"));
    scope->setWordWrap(true);
    outer->addWidget(scope);
    content = new QHBoxLayout();
    m_viewport = new SimulationViewport(&m_runtime, this);
    content->addWidget(m_viewport);
    side = new QVBoxLayout();
    inspector = new QFormLayout();
    m_address = new QLineEdit(QStringLiteral("LB0"), this);
    m_address->setObjectName(QStringLiteral("simulationAddress"));
    m_type = new QComboBox(this);
    m_type->setObjectName(QStringLiteral("simulationValueType"));
    m_type->addItem(QStringLiteral("位"));
    m_type->addItems(WordButtonProperties::dataTypes());
    m_input = new QLineEdit(QStringLiteral("0"), this);
    m_input->setObjectName(QStringLiteral("simulationWriteValue"));
    m_value = new QLabel(this);
    m_value->setObjectName(QStringLiteral("simulationReadValue"));
    m_level = new QSpinBox(this);
    m_level->setObjectName(QStringLiteral("simulationUserLevel"));
    m_level->setRange(0, 8);
    inspector->addRow(QStringLiteral("内部地址"), m_address);
    inspector->addRow(QStringLiteral("数据类型"), m_type);
    inspector->addRow(QStringLiteral("当前值"), m_value);
    inspector->addRow(QStringLiteral("输入值"), m_input);
    inspector->addRow(QStringLiteral("会话用户等级"), m_level);
    side->addLayout(inspector);
    write = new QPushButton(QStringLiteral("写入内部值"), this);
    write->setObjectName(QStringLiteral("simulationWrite"));
    side->addWidget(write);
    side->addStretch();
    close = new QPushButton(QStringLiteral("退出模拟"), this);
    close->setObjectName(QStringLiteral("simulationClose"));
    side->addWidget(close);
    content->addLayout(side);
    outer->addLayout(content);
    m_status = new QLabel(this);
    m_status->setObjectName(QStringLiteral("simulationStatus"));
    m_status->setWordWrap(true);
    outer->addWidget(m_status);
    connect(write, &QPushButton::clicked, this, &SimulationWindow::writeInternalValue);
    connect(close, &QPushButton::clicked, this, &QWidget::close);
    connect(m_address, &QLineEdit::textChanged, this, [this]() { refreshRuntime(); });
    connect(m_type, &QComboBox::currentIndexChanged, this, [this]() { refreshRuntime(); });
    connect(m_level, &QSpinBox::valueChanged, this, [this](int level) {
        QString error;

        bool ok;

        ok = m_runtime.setUserLevel(level, &error);
        refreshRuntime();
        if (!ok) { m_status->setText(error); }
    });
    m_viewport->setChangedCallback([this](bool inputReady) {
        refreshRuntime();
        if (inputReady && !m_runtime.pendingInputObjectId().isEmpty()) { requestNumericInput(); }
    });
}

/******************************************************************************
 * Function Name   : SimulationWindow::~SimulationWindow
 * Description     : Finalize even direct owner destruction without leaving a held runtime operation.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationWindow::~SimulationWindow() //20261114-6 add
{
    m_viewport->setChangedCallback({});
    m_viewport->cancelInteraction(QStringLiteral("owner-destroyed"));
    delete m_viewport; //20261114-6 change: Destroy the callback-bearing child while the value-owned runtime is still alive.
    m_viewport = nullptr;
    m_runtime.stop(QStringLiteral("window-destroyed"));
}

/******************************************************************************
 * Function Name   : SimulationWindow::initialize
 * Description     : Reject unsupported semantics before exposing a running simulation window.
 * Input Parameters: project, error
 * Return Value    : True for a complete supported immutable snapshot.
 * Date            : 2026.11.14
 ******************************************************************************/
bool SimulationWindow::initialize(const HmiProject &project, QString *error) //20261114-6 add
{
    if (!m_runtime.start(project, error)) { return false; }
    refreshRuntime();
    return true;
}

/******************************************************************************
 * Function Name   : SimulationWindow::runtime
 * Description     : Expose the owned session for controlled input and read-only diagnostics.
 * Input Parameters: None
 * Return Value    : Runtime reference.
 * Date            : 2026.11.14
 ******************************************************************************/
SimulationRuntime &SimulationWindow::runtime() //20261114-6 add
{
    return m_runtime;
}

/******************************************************************************
 * Function Name   : SimulationWindow::runtime
 * Description     : Expose a const snapshot reader without access to editor state.
 * Input Parameters: None
 * Return Value    : Const runtime reference.
 * Date            : 2026.11.14
 ******************************************************************************/
const SimulationRuntime &SimulationWindow::runtime() const //20261114-6 add
{
    return m_runtime;
}

/******************************************************************************
 * Function Name   : SimulationWindow::refreshRuntime
 * Description     : Synchronize language, access level, inspected value, and the shared PC frame.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationWindow::refreshRuntime() //20261114-6 add
{
    QString error;
    QString value;
    bool bit = false;
    bool ok;
    QSignalBlocker blocker(m_level);

    m_level->setValue(m_runtime.userLevel());
    if (m_type->currentIndex() == 0)
    {
        ok = m_runtime.readBit(m_address->text(), &bit, &error);
        value = bit ? QStringLiteral("1") : QStringLiteral("0");
    }
    else { ok = m_runtime.readWord(m_address->text(), m_type->currentIndex() - 1, &value, &error); }
    m_value->setText(ok ? value : error);
    m_status->setText(QStringLiteral("%1 · 语言：%2 · 等级：%3").arg(
        m_runtime.running() ? QStringLiteral("模拟运行中") : QStringLiteral("模拟已停止"), m_runtime.language()).arg(m_runtime.userLevel()));
    m_viewport->update();
}

/******************************************************************************
 * Function Name   : SimulationWindow::writeInternalValue
 * Description     : Apply one explicit inspector value through the same typed store as control actions.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationWindow::writeInternalValue() //20261114-6 add
{
    QString error;
    bool ok;

    if (m_type->currentIndex() == 0)
    {
        if (m_input->text() != QStringLiteral("0") && m_input->text() != QStringLiteral("1"))
        {
            m_status->setText(QStringLiteral("位值只能输入0或1。"));
            return;
        }
        ok = m_runtime.writeBit(m_address->text(), m_input->text() == QStringLiteral("1"), &error);
    }
    else { ok = m_runtime.writeWord(m_address->text(), m_type->currentIndex() - 1, m_input->text(), &error); }
    refreshRuntime();
    if (!ok) { m_status->setText(error); }
}

/******************************************************************************
 * Function Name   : SimulationWindow::requestNumericInput
 * Description     : Collect a PC session value without invoking firmware keyboards or changing the editor.
 * Input Parameters: None
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationWindow::requestNumericInput() //20261114-6 add
{
    const HmiObject *object = nullptr;
    QString id;
    QString value;
    QString error;
    QString address;
    bool accepted;
    bool password;
    QPointer<SimulationWindow> lifetime(this);

    id = m_runtime.pendingInputObjectId();
    for (const HmiObject &entry : m_runtime.project().objects)
    {
        if (entry.id == id) { object = &entry; break; }
    }
    if (object == nullptr) { return; }
    address = object->address;
    password = object->wordButton.operation == 2;
    m_runtime.readWord(object->address, object->wordButton.dataType, &value, &error);
    error.clear();
    while (m_runtime.running() && m_runtime.pendingInputObjectId() == id) //20261114-6 change: Preserve a visible correction or cancel path after invalid input.
    {
        value = QInputDialog::getText(this, QStringLiteral("模拟输入"),
            (error.isEmpty() ? QString() : error + QLatin1Char('\n')) + QStringLiteral("输入内部地址 %1 的值").arg(address),
            password ? QLineEdit::Password : QLineEdit::Normal, value, &accepted);
        if (lifetime.isNull()) { return; }
        if (!accepted) { m_runtime.cancel(QStringLiteral("numeric-input-cancelled")); break; }
        if (m_runtime.submitInput(value, &error)) { break; }
    }
    refreshRuntime();
}

/******************************************************************************
 * Function Name   : SimulationWindow::closeEvent
 * Description     : Stop the private session and release pending touches and keypad state before exit.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationWindow::closeEvent(QCloseEvent *event) //20261114-6 add
{
    m_viewport->cancelInteraction(QStringLiteral("window-closed"));
    m_runtime.stop(QStringLiteral("window-closed"));
    event->accept();
}

/******************************************************************************
 * Function Name   : SimulationWindow::changeEvent
 * Description     : Clear a live touch on top-level deactivation while preserving already released input dialogs.
 * Input Parameters: event
 * Return Value    : None
 * Date            : 2026.11.14
 ******************************************************************************/
void SimulationWindow::changeEvent(QEvent *event) //20261114-6 add
{
    if (event->type() == QEvent::WindowDeactivate) { m_viewport->cancelInteraction(QStringLiteral("window-deactivated")); }
    QDialog::changeEvent(event);
}
