#ifndef MAIN_WINDOW_H
#define MAIN_WINDOW_H

#include "project_model.h"
#include "stage3_project_edits.h" //20261110-8 add: Keep complete object/resource clipboard values across document navigation.

#include <QMainWindow>
#include <QPoint>
#include <QString>

class QLabel;
class QCloseEvent;
class QLineEdit;
class QPlainTextEdit; //20261108-12 add
class QPushButton;
class QProcess;
class QSpinBox;
class QTabWidget;
class QTemporaryDir;
class QTreeWidget;
class QTreeWidgetItem;
class ScreenCanvas;
class QComboBox;
class QAction; //20261110-7 add: Share command instances across all main-window surfaces.
class QMenu;

class MainWindow : public QMainWindow
{
    Q_OBJECT
    friend class Stage1BackendTests; //20261110-5 add: Verify page switching and persistence through the actual editor model.
    friend class Stage2WorkflowTests; //20261110-7 add: Exercise whole-project transactions without operating user windows.
    friend class Stage3IntegrationTests; //20261110-8 add: Verify resource placement and project clipboard transactions.
    friend class Stage4ControlTests; //20261112-0 add: Exercise real control actions and transaction history.

public:
    explicit MainWindow(QWidget *parent = nullptr);
    bool openProjectPath(const QString &path, QString *error = nullptr); //20261111-1 add: Reuse the validated open transaction for file arguments and owned captures.
    ~MainWindow() override; //20261110-7 add: Detach application-wide signals before derived state is destroyed.

private slots:
    void createProject();
    void openProject();
    void saveProject();
    void saveProjectAs();
    void closeProject(); //20261110-7 add
    void configureProject();
    void configureSystemSettings(); //20261110-7 add
    void showGlobalTool(int kind);
    void showMacroSource(const QString &name, int line);
    void replaceProjectAddresses();
    void exportNativeTest();
    void addSelectedControl();
    void applySelectedProperties();
    void refreshDeviceState();
    void downloadToDevice(); //20261108-8 change
    void updateCoordinates(const QPoint &point);
    void updateSelection(const HmiObject &object, bool selected);
    void selectTreeObject(QTreeWidgetItem *current, QTreeWidgetItem *previous);
    void createStaticText(); //20261108-14 add
    void editStaticText();
    void createStage4Control(const QString &type); //20261112-0 add
    void editStage4Control();
    void configureDataCollection(const QString &id = QString()); //20261112-1 add
    void deleteDataCollection(const QString &id);
    void configureLanguages();
    void configureConnections(); //20261109-0 add
    void configureMacroSdk(); //20261109-0 add
    void createBitButton(); //20261108-15 add
    void createWordButton(); //20261109-6 add
    void createLamp(); //20261109-8 add
    void createFunctionButton(); //20261110-0 add
    void editBitButton();
    void manageProjectPictures(); //20261110-8 add

private:
    HmiProject m_project;
    QString m_currentPath;
    bool m_dirty;
    QProcess *m_downloadProcess; //20261108-8 add
    QTemporaryDir *m_downloadTemp; //20261108-8 add
    QTabWidget *m_pageTabs;
    ScreenCanvas *m_canvas;
    QTreeWidget *m_projectTree;
    QTabWidget *m_leftTabs;
    QTabWidget *m_rightTabs;
    QLineEdit *m_nameEdit;
    QPlainTextEdit *m_textEdit; //20261108-12 change: Preserve multiline labels like the native editor.
    QLineEdit *m_addressEdit;
    QSpinBox *m_xSpin;
    QSpinBox *m_ySpin;
    QSpinBox *m_widthSpin;
    QSpinBox *m_heightSpin;
    QLabel *m_connectionLabel;
    QLabel *m_coordinateLabel;
    QLabel *m_projectLabel;
    QString m_selectedId;
    QComboBox *m_languageCombo = nullptr; //20261108-14 add
    QComboBox *m_pageCombo = nullptr; //20261110-7 add
    QTreeWidget *m_systemSettingsTree = nullptr;
    QMenu *m_recentProjectsMenu = nullptr;
    QMap<QString, QAction *> m_commands;
    QList<HmiProject> m_projectHistory;
    int m_projectHistoryIndex = -1;
    QJsonObject m_savedContent;
    bool m_hasProject = true;
    bool m_restoringProject = false;
    bool m_updatingNavigation = false;
    Stage3Clipboard m_objectClipboard; //20261110-8 add
    HmiProject m_pendingControlResources;
    bool m_hasPendingControlResources = false;

    void createActionsAndMenus();
    void initializeNativeShell(); //20261111-1 add: Bind the recovered shell resources to existing command transactions.
    void createNativeStatusBar();
    QWidget *createNativePalette(const QString &group);
    void updateNativeStatusBar();
    void decorateNativeSystemTree();
    void createToolbars();
    QWidget *createToolbox();
    QWidget *createPropertyEditor();
    QWidget *createSystemSettingsTree();
    void rebuildProjectTree();
    bool applyControlDraft(const HmiObject &object, const QMap<QString, QString> &macros,
                           const QMap<QString, QByteArray> &pictures, bool editing,
                           QString *error, const QMap<QString, QByteArray> *animations = nullptr); //20261112-0 change
    void beginControlDraftPlacement(const HmiObject &object, const QMap<QString, QString> &macros,
                                    const QMap<QString, QByteArray> &pictures, const QMap<QString, QByteArray> *animations = nullptr); //20261112-0 change
    void setProject(const HmiProject &project, const QString &path);
    void setDirty(bool dirty);
    bool confirmDiscardChanges();
    bool writeProject(const QString &path);
    HmiProject currentEditorProject() const; //20261110-7 add
    bool commitProjectDraft(const HmiProject &project, QString *error = nullptr);
    bool activatePageId(const QString &pageId, const QString &objectId = QString());
    void refreshPageNavigation();
    void rebuildSystemSettings();
    void dispatchEditCommand(const QString &command);
    void updateCommandStates();
    void resetProjectHistory();
    void recordProjectHistory();
    bool projectIsModified() const;
    void storeSavedCheckpoint(); //20261110-7 add
    void restoreProjectHistory(int delta);
    void updateRecentProjects(const QString &path = QString());
    void fillProperties(const HmiObject &object, bool selected);
    QString connectedDownloadDevice() const;

protected:
    void closeEvent(QCloseEvent *event) override;
};

#endif
