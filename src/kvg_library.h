#ifndef KVG_LIBRARY_H
#define KVG_LIBRARY_H

#include <QByteArray>
#include <QImage>
#include <QList>
#include <QRectF>
#include <QStringList>

class QPainter;

//20261109-0 add: Stable resource IDs distinguish equal filenames in different vendor categories.
struct KvgLibraryCategory
{
    QString id;
    QString group;
    QString name;
    QStringList pictures;
};

//20261109-3 add: Native visible tree order is independent of the complete resource index.
struct KvgLibraryBranch
{
    QString name;
    QStringList categoryIds;
};

QList<KvgLibraryCategory> kvgLibraryCategories();
QList<KvgLibraryBranch> kvgLibraryBranches(); //20261109-3 add
QString kvgPictureResource(const QString &picture);
QString kvgPictureWarning(const QString &picture); //20261109-0 add
QImage kvgPictureImage(const QString &picture, const QSize &size);
bool paintKvgPicture(QPainter *painter, const QRectF &rect, const QString &picture, int imageMode);
QImage importKvgImage(const QByteArray &bytes, QString *error);

//20261109-1 add: Legacy KVG API names also accept catalog-backed SVG and PNG resources.

#endif
