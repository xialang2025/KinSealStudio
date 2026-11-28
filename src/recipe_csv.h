#ifndef RECIPE_CSV_H
#define RECIPE_CSV_H
#include "recipe_properties.h"
#include <QByteArray>
bool encodeNativeCp936(const QString &text, QByteArray *output); //20261128-23 add
//20261113-2 add: Native CSV omits item captions and retained hidden rows/columns.
bool exportRecipeCsv(const QString &name, const RecipeProperties &properties, QByteArray *output, QString *error = nullptr);
bool importRecipeCsv(const QByteArray &bytes, QString *name, RecipeProperties *output, QString *error = nullptr);
#endif
