#ifndef PROJECT_MODEL_H
#define PROJECT_MODEL_H

#include <QJsonObject>
#include <QList>
#include <QRect>
#include <QString>
#include "static_text_properties.h" //20261108-14 add
#include "bit_button_properties.h" //20261108-15 add
#include "word_button_properties.h" //20261109-6 add
#include "lamp_properties.h" //20261109-8 add
#include "function_button_properties.h" //20261110-0 add
#include "plc_address.h" //20261109-0 add
#include "message_display_properties.h" //20261112-0 add
#include "scale_properties.h" //20261112-0 add
#include "qr_code_properties.h" //20261112-0 add
#include "image_display_properties.h" //20261112-0 add
#include "gif_display_properties.h" //20261112-0 add
#include "number_display_properties.h" //20261112-0 add
#include "ascii_display_properties.h" //20261112-0 add
#include "time_display_properties.h" //20261112-0 add
#include "screen_button_properties.h" //20261112-0 add
#include "multi_state_button_properties.h" //20261112-0 add

struct HmiObject
{
    QString id;
    QString type;
    QString name;
    QString text;
    QString address;
    QRect geometry;
    bool hasStaticTextProperties = false; //20261108-14 add: Legacy projects retain their validated raster path.
    StaticTextProperties staticText;
    bool hasBitButtonProperties = false; //20261108-15 add
    BitButtonProperties bitButton;
    bool hasWordButtonProperties = false; //20261109-6 add: Word operations share artwork, macros and access rules with switches.
    WordButtonProperties wordButton;
    bool hasLampProperties = false; //20261109-8 add: A lamp monitors addresses without a touch-operation address.
    LampProperties lamp;
    bool hasFunctionButtonProperties = false; //20261110-0 add: System functions have no PLC operation address.
    FunctionButtonProperties functionButton;
    MessageDisplayProperties messageDisplay; //20261112-0 add: Message states use their own source/device family.
    ScaleProperties scale; //20261112-0 add: A scale is a static graphic with no PLC role.
    QrCodeProperties qrCode; //20261112-0 add: A QR monitor is independent of buttons and static text.
    bool hasImageDisplayProperties = false; //20261112-0 add: Legacy image placeholders stay loadable until configured.
    ImageDisplayProperties imageDisplay;
    GifDisplayProperties gifDisplay; //20261112-0 add
    bool hasNumberDisplayProperties = false; //20261112-0 add: Preserve old number placeholders until configured.
    NumberDisplayProperties numberDisplay;
    AsciiDisplayProperties asciiDisplay; //20261112-0 add: ASCII options remain distinct from numeric enum/range semantics.
    TimeDisplayProperties timeDisplay; //20261112-0 add: System clock formatting has no PLC monitor carrier.
    ScreenButtonProperties screenButton; //20261112-0 add: Navigation targets are stable identities rather than switch addresses.
    MultiStateButtonProperties multiStateButton; //20261112-0 add: Multi-state input does not use the lamp native subtype.
    double layer = 1;
    QString group;
    QJsonObject extensions; //20261110-5 add: Versioned, explicitly inactive editor metadata only.

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HmiObject *object, QString *error);
};

//20261110-5 add: These are editor entities; page kinds do not claim a recovered native layout.
struct HmiPage
{
    QString id;
    QString kind = QStringLiteral("normal");
    QString name;
    int width = 800;
    int height = 480;
    QList<HmiObject> objects;
    QJsonObject extensions;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HmiPage *page, QString *error = nullptr);
};

struct HmiConfigurationNode
{
    QString id;
    QString kind;
    QString name;
    QString ownerPageId;
    bool implemented = false;
    bool active = false;
    QJsonObject properties;
    QJsonObject extensions;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HmiConfigurationNode *node, QString *error = nullptr);
};

struct HmiReference
{
    QString id;
    QString ownerKind;
    QString ownerId;
    QString field;
    QString targetKind;
    QString targetId;

    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HmiReference *reference, QString *error = nullptr);
};

struct HmiProject
{
    QString name;
    QString targetModel;
    QString pageName;
    int pageWidth;
    int pageHeight;
    QList<HmiObject> objects;
    QStringList languages = {QStringLiteral("中文")}; //20261108-14 add
    QString activeLanguage = QStringLiteral("中文");
    QMap<QString, QString> macros; //20261108-15 add: Sources are saved without claiming native macro compilation.
    QMap<QString, QByteArray> pictures;
    QMap<QString, QByteArray> animations; //20261112-0 add: Exact GIF bytes, never PNG-normalized.
    QVector<PlcConnection> connections; //20261109-0 add: Stable connection IDs qualify external object addresses.
    QByteArray nativeSourceSnapshot; //20261109-0 add: Preserve bounded native metadata for verified source reserialization.
    QStringList nativeSourceWarnings; //20261109-0 add: Explain dormant native values preserved outside the editable model.
    //20261110-5 add: Legacy fields above are the only storage of the active page, avoiding stale object mirrors.
    QString projectId = QStringLiteral("project-1");
    QString activePageId = QStringLiteral("page-1");
    QString activePageKind = QStringLiteral("normal");
    QJsonObject activePageExtensions;
    QList<HmiPage> inactivePages;
    QStringList pageOrder = {QStringLiteral("page-1")};
    QString startupPageId = QStringLiteral("page-1");
    QString startupLanguage; // Empty inherits the verified native snapshot or the first language for a new project.
    QList<HmiConfigurationNode> configurationNodes;
    QList<HmiReference> references;
    QMap<QString, QString> macroIds; // Optional name -> stable ID; legacy macro names are never asserted to be IDs.
    QJsonObject extensions;

    HmiProject();
    QList<HmiPage> allPages() const;
    int pageCount() const;
    bool selectPage(const QString &id, QString *error = nullptr);
    bool addPage(const HmiPage &page, QString *error = nullptr);
    bool copyPage(const QString &id, const QString &newName, QString *newId = nullptr, QString *error = nullptr);
    bool renamePage(const QString &id, const QString &newName, QString *error = nullptr);
    bool removePage(const QString &id, QString *error = nullptr);
    bool renameMacro(const QString &oldName, const QString &newName, QString *error = nullptr);
    bool setLanguages(const QStringList &newLanguages, QString *error = nullptr);
    bool validateModel(QString *error = nullptr) const;
    bool validateSinglePageBackend(QString *error = nullptr) const;
    bool validatePlcConfiguration(QString *error) const; //20261109-0 add
    QJsonObject toJson() const;
    static bool fromJson(const QJsonObject &json, HmiProject *project, QString *error);
    bool save(const QString &path, QString *error) const;
    static bool load(const QString &path, HmiProject *project, QString *error);
};

#endif
