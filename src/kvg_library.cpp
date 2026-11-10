#include "kvg_library.h"
#include <QCache>
#include <QBuffer> //20261109-1 add
#include <QFile>
#include <QImageReader> //20261109-1 add
#include <QJsonArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QPainter>
#include <QSvgRenderer>
#include <QXmlStreamReader>

/******************************************************************************
 * Function Name   : galleryCatalog
 * Description     : Cache the small immutable index independently of the on-demand SVG cache.
 * Input Parameters: None
 * Return Value    : Catalog metadata, or an empty object if packaging is incomplete.
 * Date            : 2026.11.09
 ******************************************************************************/
static const QJsonObject &galleryCatalog() //20261109-0 add
{
    static const QJsonObject catalog = []() {
        QFile file(QStringLiteral(":/system-gallery/catalog.json"));
        if (!file.open(QIODevice::ReadOnly)) {
            return QJsonObject();
        }
        return QJsonDocument::fromJson(file.readAll()).object();
    }();
    return catalog;
}

/******************************************************************************
 * Function Name   : kvgLibraryCategories
 * Description     : Load only the gallery index; SVG bodies are opened on demand.
 * Input Parameters: None
 * Return Value    : Ordered categories with stable resource IDs.
 * Date            : 2026.11.09
 ******************************************************************************/
QList<KvgLibraryCategory> kvgLibraryCategories() //20261109-0 add
{
    QList<KvgLibraryCategory> categories;
    QJsonArray entries;
    QJsonObject object;
    KvgLibraryCategory category;
    int i;
    int j;

    entries = galleryCatalog().value(QStringLiteral("categories")).toArray();
    for (i = 0; i < entries.size(); ++i) {
        object = entries.at(i).toObject();
        category = KvgLibraryCategory();
        category.id = object.value(QStringLiteral("id")).toString();
        category.group = object.value(QStringLiteral("group")).toString();
        category.name = object.value(QStringLiteral("name")).toString();
        for (j = 0; j < object.value(QStringLiteral("pictures")).toArray().size(); ++j) {
            category.pictures.append(object.value(QStringLiteral("pictures")).toArray().at(j).toString());
        }
        categories.append(category);
    }
    return categories;
}

/******************************************************************************
 * Function Name   : kvgLibraryBranches
 * Description     : Read the verified native root and leaf order without filtering the full index.
 * Input Parameters: None
 * Return Value    : Ordered peer roots with their ordered stable category identifiers.
 * Date            : 2026.11.09
 ******************************************************************************/
QList<KvgLibraryBranch> kvgLibraryBranches() //20261109-3 add
{
    QList<KvgLibraryBranch> branches;
    QJsonArray entries;
    QJsonArray categories;
    QJsonObject object;
    KvgLibraryBranch branch;
    int i;
    int j;

    entries = galleryCatalog().value(QStringLiteral("nativeTree")).toArray();
    for (i = 0; i < entries.size(); ++i)
    {
        object = entries.at(i).toObject();
        branch = KvgLibraryBranch();
        branch.name = object.value(QStringLiteral("name")).toString();
        categories = object.value(QStringLiteral("categoryIds")).toArray();
        for (j = 0; j < categories.size(); ++j)
        {
            branch.categoryIds.append(categories.at(j).toString());
        }
        branches.append(branch);
    }
    return branches;
}

/******************************************************************************
 * Function Name   : kvgPictureResource
 * Description     : Resolve legacy names and qualified SVG/KVG or PNG gallery resources.
 * Input Parameters: picture: persisted picture identifier.
 * Return Value    : Resource path, or an empty string for unknown identifiers.
 * Date            : 2026.11.09
 ******************************************************************************/
QString kvgPictureResource(const QString &picture) //20261109-0 add
{
    QString resource;
    QStringList legacy;

    legacy = {QStringLiteral("switch_OFF.kvg"), QStringLiteral("switch_ON.kvg"),
              QStringLiteral("default.kvg"), QStringLiteral("gray.kvg")};
    if (legacy.contains(picture)) {
        resource = QStringLiteral(":/system-gallery/default/") + picture;
    } else if (picture.startsWith(QStringLiteral(":/system-gallery/")) &&
               (picture.endsWith(QStringLiteral(".kvg"), Qt::CaseInsensitive) ||
                picture.endsWith(QStringLiteral(".svg"), Qt::CaseInsensitive) ||
                picture.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive)) && //20261109-1 change
               !picture.contains(QStringLiteral("/../")) && !picture.contains(QChar(u'\0'))) {
        resource = picture;
    }
    return !resource.isEmpty() && QFile::exists(resource) ? resource : QString();
}

/******************************************************************************
 * Function Name   : kvgPictureWarning
 * Description     : Report verified missing vendor sidecars without inventing replacement pixels.
 * Input Parameters: picture: persisted picture identifier.
 * Return Value    : User-facing limitation for the selected asset, or an empty string.
 * Date            : 2026.11.09
 ******************************************************************************/
QString kvgPictureWarning(const QString &picture) //20261109-0 add
{
    QString resource;
    resource = kvgPictureResource(picture);
    return galleryCatalog().value(QStringLiteral("warnings")).toObject().value(resource).toString();
}

/******************************************************************************
 * Function Name   : cachedKvgRenderer
 * Description     : Cache a bounded number of parsed SVGs rather than loading the full library.
 * Input Parameters: picture: persisted picture identifier.
 * Return Value    : Valid cached renderer, or nullptr when the resource cannot be decoded.
 * Date            : 2026.11.09
 ******************************************************************************/
static QSvgRenderer *cachedKvgRenderer(const QString &picture) //20261109-0 add
{
    static QCache<QString, QSvgRenderer> cache(64);
    QString resource;
    QSvgRenderer *renderer;
    QFile file;
    QSize size; //20261109-1 add

    resource = kvgPictureResource(picture);
    if (resource.isEmpty()) {
        return nullptr;
    }
    renderer = cache.object(resource);
    if (renderer != nullptr) {
        return renderer;
    }
    file.setFileName(resource);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 16 * 1024 * 1024) {
        return nullptr;
    }
    renderer = new QSvgRenderer(file.readAll());
    if (!renderer->isValid()) {
        delete renderer;
        return nullptr;
    }
    size = renderer->defaultSize();
    if (!size.isValid() || size.width() > 4096 || size.height() > 4096)
    {
        delete renderer;
        return nullptr; //20261109-1 add: Vector intrinsic dimensions follow the same import limits.
    }
    cache.insert(resource, renderer);
    return renderer;
}

/******************************************************************************
 * Function Name   : boundedPngImage
 * Description     : Check encoded bytes and declared dimensions before allocating PNG pixels.
 * Input Parameters: bytes, error
 * Return Value    : A decoded image within 16 MiB and 4096 by 4096 limits, or a null image.
 * Date            : 2026.11.09
 ******************************************************************************/
static QImage boundedPngImage(const QByteArray &bytes, QString *error) //20261109-1 add
{
    QBuffer buffer;
    QImageReader reader;
    QImage image;
    QSize size;

    if (bytes.isEmpty() || bytes.size() > 16 * 1024 * 1024)
    {
        if (error != nullptr) { *error = QStringLiteral("图片超过16 MiB输入限制。"); }
        return image;
    }
    buffer.setData(bytes);
    buffer.open(QIODevice::ReadOnly);
    reader.setDevice(&buffer);
    reader.setFormat(QByteArrayLiteral("PNG"));
    size = reader.size();
    if (!size.isValid() || size.width() > 4096 || size.height() > 4096)
    {
        if (error != nullptr) { *error = QStringLiteral("本版支持不超过4096×4096的PNG图片。"); }
        return image;
    }
    image = reader.read();
    if (image.isNull() || image.width() > 4096 || image.height() > 4096)
    {
        if (error != nullptr) { *error = QStringLiteral("无法解析PNG图片。"); }
        return QImage();
    }
    return image.convertToFormat(QImage::Format_ARGB32_Premultiplied); //20261109-1 change: Cache raster pixels in a bounded four-byte format.
}

/******************************************************************************
 * Function Name   : cachedGalleryPng
 * Description     : Decode raster resources separately from SVG with a bounded pixel cache.
 * Input Parameters: resource
 * Return Value    : Cached PNG pixels, or nullptr for invalid or oversized resources.
 * Date            : 2026.11.09
 ******************************************************************************/
static const QImage *cachedGalleryPng(const QString &resource) //20261109-1 add
{
    static QCache<QString, QImage> cache(64 * 1024); // Cache cost is KiB, at most one maximum-sized raster.
    QImage *image;
    QFile file;
    int cost;

    image = cache.object(resource);
    if (image != nullptr)
    {
        return image;
    }
    file.setFileName(resource);
    if (!file.open(QIODevice::ReadOnly) || file.size() > 16 * 1024 * 1024)
    {
        return nullptr;
    }
    image = new QImage(boundedPngImage(file.readAll(), nullptr));
    if (image->isNull())
    {
        delete image;
        return nullptr;
    }
    cost = int((image->sizeInBytes() + 1023) / 1024);
    if (!cache.insert(resource, image, cost))
    {
        return nullptr; // QCache owns and deletes rejected entries.
    }
    return image;
}

/******************************************************************************
 * Function Name   : paintKvgPicture
 * Description     : Render vector or raster gallery artwork in stretch, fit or original-size mode.
 * Input Parameters: painter, rect, picture, imageMode: 0 stretch, 1 aspect fit, 2 original size.
 * Return Value    : True when a valid gallery resource was rendered.
 * Date            : 2026.11.09
 ******************************************************************************/
bool paintKvgPicture(QPainter *painter, const QRectF &rect, const QString &picture, int imageMode) //20261109-1 change
{
    QSvgRenderer *renderer;
    const QImage *image;
    QString resource;
    QSizeF size;
    QRectF target;

    if (painter == nullptr || rect.isEmpty() || imageMode < 0 || imageMode > 2) {
        return false;
    }
    resource = kvgPictureResource(picture);
    if (resource.isEmpty()) { return false; }
    image = nullptr;
    renderer = nullptr;
    if (resource.endsWith(QStringLiteral(".png"), Qt::CaseInsensitive))
    {
        image = cachedGalleryPng(resource);
        if (image == nullptr) { return false; }
    }
    else
    {
        renderer = cachedKvgRenderer(picture);
        if (renderer == nullptr) { return false; }
    }
    target = rect;
    if (imageMode != 0) {
        size = image != nullptr ? QSizeF(image->size()) : QSizeF(renderer->defaultSize()); //20261109-1 change
        if (size.isEmpty()) {
            size = renderer->viewBoxF().size();
        }
        if (imageMode == 1) {
            size.scale(rect.size(), Qt::KeepAspectRatio);
        }
        target = QRectF(rect.center() - QPointF(size.width() / 2, size.height() / 2), size);
    }
    painter->save();
    painter->setClipRect(rect, Qt::IntersectClip); //20261109-1 add: Original-size artwork stays inside its control.
    if (image != nullptr)
    {
        painter->drawImage(target, *image);
    }
    else
    {
        renderer->render(painter, target);
    }
    painter->restore();
    return true;
}

/******************************************************************************
 * Function Name   : kvgPictureImage
 * Description     : Render a bounded transparent thumbnail for either vector or raster artwork.
 * Input Parameters: picture, size: requested thumbnail bounds.
 * Return Value    : Transparent thumbnail, or a null image when decoding fails.
 * Date            : 2026.11.09
 ******************************************************************************/
QImage kvgPictureImage(const QString &picture, const QSize &size) //20261109-0 add
{
    QImage image;
    QPainter painter;
    bool rendered;

    if (size.isEmpty() || size.width() > 4096 || size.height() > 4096) {
        return image;
    }
    image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    painter.begin(&image);
    rendered = paintKvgPicture(&painter, image.rect(), picture, 1);
    painter.end();
    return rendered ? image : QImage();
}

/******************************************************************************
 * Function Name   : importKvgImage
 * Description     : Decode standalone SVG/KVG or PNG input within project image limits.
 * Input Parameters: bytes: source image bytes; error: optional diagnostic output.
 * Return Value    : Decoded raster, or a null image for invalid, external or oversized input.
 * Date            : 2026.11.09
 ******************************************************************************/
QImage importKvgImage(const QByteArray &bytes, QString *error) //20261109-1 change
{
    QXmlStreamReader xml;
    QSvgRenderer renderer;
    QImage image;
    QPainter painter;
    QString reference;
    QSize size;
    int elements;
    int i;
    bool rootSeen;
    QXmlStreamAttributes attributes;

    if (error != nullptr) {
        error->clear();
    }
    if (bytes.isEmpty() || bytes.size() > 16 * 1024 * 1024) {
        if (error != nullptr) { *error = QStringLiteral("图片超过16 MiB输入限制。"); }
        return image;
    }
    if (bytes.startsWith(QByteArray::fromHex(QByteArrayLiteral("89504e470d0a1a0a"))))
    {
        return boundedPngImage(bytes, error); //20261109-1 add: PNG bytes never enter QSvgRenderer.
    }
    xml.addData(bytes);
    rootSeen = false;
    elements = 0;
    while (!xml.atEnd()) {
        xml.readNext();
        if (!xml.isStartElement()) {
            continue;
        }
        ++elements;
        if ((!rootSeen && xml.name() != QStringLiteral("svg")) || elements > 100000) {
            if (error != nullptr) { *error = QStringLiteral("KVG/SVG内容无效或结构过大。"); }
            return image;
        }
        rootSeen = true;
        attributes = xml.attributes();
        for (i = 0; i < attributes.size(); ++i) {
            if (attributes.at(i).name() == QStringLiteral("href")) {
                reference = attributes.at(i).value().toString().trimmed();
                if (!reference.isEmpty() && !reference.startsWith(QLatin1Char('#')) &&
                    !reference.startsWith(QStringLiteral("data:"), Qt::CaseInsensitive)) {
                    if (error != nullptr) { *error = QStringLiteral("请导入已嵌入图片内容的独立KVG/SVG文件。"); }
                    return image;
                }
            }
        }
    }
    if (!rootSeen || xml.hasError() || !renderer.load(bytes)) {
        if (error != nullptr) { *error = QStringLiteral("无法解析KVG/SVG图片。"); }
        return image;
    }
    size = renderer.defaultSize();
    if (size.isEmpty() || size.width() > 4096 || size.height() > 4096) {
        if (error != nullptr) { *error = QStringLiteral("本版支持不超过4096×4096的图片。"); }
        return image;
    }
    image = QImage(size, QImage::Format_ARGB32_Premultiplied);
    image.fill(Qt::transparent);
    painter.begin(&image);
    renderer.render(&painter);
    painter.end();
    return image;
}
