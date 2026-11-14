#ifndef SIMULATION_WINDOW_H
#define SIMULATION_WINDOW_H

#include "simulation_runtime.h"
#include <QDialog>
#include <QImage>
#include <QWidget>
#include <functional>

class QLabel;
class QLineEdit;
class QComboBox;
class QSpinBox;
class QCloseEvent;
class QMouseEvent;
class QFocusEvent;
class QPaintEvent;

QImage simulationFrame(const SimulationRuntime &runtime); //20261114-6 add

class SimulationViewport : public QWidget //20261114-6 add
{
    Q_OBJECT
public:
    explicit SimulationViewport(SimulationRuntime *runtime, QWidget *parent = nullptr);
    void setChangedCallback(const std::function<void(bool)> &callback);
    void cancelInteraction(const QString &reason);
    bool pointerDown() const;
protected:
    void paintEvent(QPaintEvent *event) override;
    void mousePressEvent(QMouseEvent *event) override;
    void mouseMoveEvent(QMouseEvent *event) override;
    void mouseReleaseEvent(QMouseEvent *event) override;
    void focusOutEvent(QFocusEvent *event) override;
    bool event(QEvent *event) override;
private:
    const HmiObject *hitObject(const QPoint &point) const;
    void changed(bool inputReady = false);
    SimulationRuntime *m_runtime;
    QString m_activeObject;
    std::function<void(bool)> m_changed;
};

class SimulationWindow : public QDialog //20261114-6 add
{
    Q_OBJECT
public:
    explicit SimulationWindow(QWidget *parent = nullptr);
    ~SimulationWindow() override;
    bool initialize(const HmiProject &project, QString *error = nullptr);
    SimulationRuntime &runtime();
    const SimulationRuntime &runtime() const;
    void refreshRuntime();
protected:
    void closeEvent(QCloseEvent *event) override;
    void changeEvent(QEvent *event) override;
private:
    void writeInternalValue();
    void requestNumericInput();
    SimulationRuntime m_runtime;
    SimulationViewport *m_viewport;
    QLabel *m_status;
    QLabel *m_value;
    QLineEdit *m_address;
    QLineEdit *m_input;
    QComboBox *m_type;
    QSpinBox *m_level;
};

#endif
