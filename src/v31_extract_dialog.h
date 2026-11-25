#pragma once

#include <QString>
#include <functional>

class QWidget;

//20261125-81 add: V31 archives contain an editable project and a separate gallery.
bool extractV31Configuration(const QString &archivePath, const QString &projectPath,
                             const QString &galleryBase, bool replaceProject, QString *error,
                             const std::function<void(int)> &progress = {});
void showV31ExtractDialog(QWidget *parent);
