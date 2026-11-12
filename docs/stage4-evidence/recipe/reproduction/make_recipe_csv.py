"""Implement the observed bounded GBK recipe exchange format without guessing CSV escaping."""
from pathlib import Path
import ast
root=Path(__file__).resolve().parent/'worktree';tree=ast.parse(Path(__file__).with_name('make_recipe_models.py').read_text(encoding='utf-8'));ns={};exec(compile(ast.Module(body=[n for n in tree.body if isinstance(n,ast.FunctionDef) and n.name=='f'],type_ignores=[]),'<format>','exec'),ns);f=ns['f']
def put(path,text):(root/path).write_text(text,encoding='utf-8',newline='\n')
put('src/recipe_csv.h','''#ifndef RECIPE_CSV_H
#define RECIPE_CSV_H
#include "recipe_properties.h"
//20261113-2 add: Native CSV omits item captions and retained hidden rows/columns.
bool exportRecipeCsv(const QString &name, const RecipeProperties &properties, QByteArray *output, QString *error = nullptr);
bool importRecipeCsv(const QByteArray &bytes, QString *name, RecipeProperties *output, QString *error = nullptr);
#endif
''')
s='''#include "recipe_csv.h"
#ifdef Q_OS_WIN
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#endif
'''
s+=f('csvError','Report an unsupported or malformed native recipe exchange without publishing partial output.','static bool csvError(QString *error, const QString &message)','    if (error != nullptr) { *error = message; } return false;','error, message','False.')
s+=f('recipeGbkEncode','Encode native CP936 bytes exactly and reject best-fit character replacement.','static bool recipeGbkEncode(const QString &text, QByteArray *output)', '''#ifdef Q_OS_WIN
    int size;
    BOOL substituted = FALSE;
    QByteArray bytes;
    size = WideCharToMultiByte(936, WC_NO_BEST_FIT_CHARS, reinterpret_cast<const wchar_t *>(text.utf16()), int(text.size()), nullptr, 0, nullptr, &substituted);
    if (size <= 0 || substituted) { return false; } bytes.resize(size); substituted = FALSE;
    if (WideCharToMultiByte(936, WC_NO_BEST_FIT_CHARS, reinterpret_cast<const wchar_t *>(text.utf16()), int(text.size()), bytes.data(), size, nullptr, &substituted) != size || substituted) { return false; }
    *output = bytes; return true;
#else
    Q_UNUSED(text); Q_UNUSED(output); return false;
#endif''','text, output','True after exact CP936 encoding.')
s+=f('recipeGbkDecode','Decode bounded CP936 input and round-trip bytes before parsing untrusted exchange fields.','static bool recipeGbkDecode(const QByteArray &bytes, QString *output)', '''#ifdef Q_OS_WIN
    int size;
    QString text;
    QByteArray encoded;
    if (bytes.isEmpty() || bytes.size() > 8 * 1024 * 1024 || bytes.contains(char(0))) { return false; }
    size = MultiByteToWideChar(936, MB_ERR_INVALID_CHARS, bytes.constData(), int(bytes.size()), nullptr, 0); if (size <= 0) { return false; } text.resize(size);
    if (MultiByteToWideChar(936, MB_ERR_INVALID_CHARS, bytes.constData(), int(bytes.size()), reinterpret_cast<wchar_t *>(text.data()), size) != size || !recipeGbkEncode(text, &encoded) || encoded != bytes) { return false; }
    *output = text; return true;
#else
    Q_UNUSED(bytes); Q_UNUSED(output); return false;
#endif''','bytes, output','True for exact bounded CP936 input.')
s+=f('exportRecipeCsv','Write the ten measured metadata lines, recipe names and active numeric matrix using native trailing commas.','bool exportRecipeCsv(const QString &name, const RecipeProperties &properties, QByteArray *output, QString *error)', '''    QString text;
    QString caption;
    QByteArray bytes;
    int row;
    int column;
    if (output == nullptr || name.trimmed().isEmpty() || name.size() > 48 || !properties.validate(error)) { return false; }
    Q_FOREACH (caption, (QStringList{name, properties.description, properties.writeAddress, properties.readAddress})) { if (caption.contains(QLatin1Char('\\r')) || caption.contains(QLatin1Char('\\n'))) { return csvError(error, QStringLiteral("原厂配方CSV元信息不支持换行。")); } }
    for (column = 0; column < properties.count; ++column) { caption = properties.recipeNames[column]; if (caption.contains(QLatin1Char(',')) || caption.contains(QLatin1Char('\\r')) || caption.contains(QLatin1Char('\\n'))) { return csvError(error, QStringLiteral("原厂配方CSV未转义逗号/换行；请修改配方列名称后导出。")); } }
    text = QStringLiteral("名称:%1\\r\\n描述:%2\\r\\n配方长度:%3\\r\\n配方总数:%4\\r\\n数据类型:%5\\r\\n是否写配方到PLC:%6\\r\\n写入地址:%7\\r\\n是否从PLC中读配方:%8\\r\\n是否与写地址相同:%9\\r\\n读取地址:%10\\r\\n").arg(name, properties.description).arg(properties.length).arg(properties.count).arg(RecipeProperties::dataTypes().value(properties.dataType)).arg(properties.writeEnabled ? 1 : 0).arg(properties.writeAddress).arg(properties.readEnabled ? 1 : 0).arg(properties.sameReadWrite ? 1 : 0).arg(properties.readAddress);
    for (column = 0; column < properties.count; ++column) { text += properties.recipeNames[column] + QLatin1Char(','); } text += QStringLiteral("\\r\\n");
    for (row = 0; row < properties.length; ++row) { for (column = 0; column < properties.count; ++column) { text += properties.values[row][column] + QLatin1Char(','); } text += QStringLiteral("\\r\\n"); }
    if (!recipeGbkEncode(text, &bytes)) { return csvError(error, QStringLiteral("配方CSV需要可无损表示的GBK字符及Windows编码支持。")); } *output = bytes; return true;''','name, properties, output, error','True after complete exact-format encoding.')
s+=f('importRecipeCsv','Parse the observed metadata and matrix atomically; reject ambiguous headers, malformed dimensions and out-of-range scalars.','bool importRecipeCsv(const QByteArray &bytes, QString *name, RecipeProperties *output, QString *error)', '''    RecipeProperties candidate;
    QString text;
    QString nextName;
    QStringList lines;
    QStringList metadata;
    QStringList fields;
    QStringList prefixes = {QStringLiteral("名称:"), QStringLiteral("描述:"), QStringLiteral("配方长度:"), QStringLiteral("配方总数:"), QStringLiteral("数据类型:"), QStringLiteral("是否写配方到PLC:"), QStringLiteral("写入地址:"), QStringLiteral("是否从PLC中读配方:"), QStringLiteral("是否与写地址相同:"), QStringLiteral("读取地址:")};
    bool valid;
    int index;
    int row;
    int column;
    int length;
    int count;
    if (name == nullptr || output == nullptr || !recipeGbkDecode(bytes, &text)) { return csvError(error, QStringLiteral("配方CSV编码无效或文件超限。")); }
    text.replace(QStringLiteral("\\r\\n"), QStringLiteral("\\n")); if (text.contains(QLatin1Char('\\r'))) { return false; } lines = text.split(QLatin1Char('\\n')); if (!lines.isEmpty() && lines.last().isEmpty()) { lines.removeLast(); } if (lines.size() < 12) { return csvError(error, QStringLiteral("配方CSV缺少完整元信息和数据。")); }
    for (index = 0; index < prefixes.size(); ++index) { if (!lines[index].startsWith(prefixes[index])) { return csvError(error, QStringLiteral("配方CSV元信息顺序或名称不匹配。")); } metadata.append(lines[index].mid(prefixes[index].size())); }
    nextName = metadata[0]; if (nextName.trimmed().isEmpty() || nextName.size() > 48) { return false; } length = metadata[2].toInt(&valid); if (!valid || QString::number(length) != metadata[2]) { return false; } count = metadata[3].toInt(&valid); if (!valid || QString::number(count) != metadata[3]) { return false; }
    candidate.dataType = RecipeProperties::dataTypes().indexOf(metadata[4]); if (candidate.dataType < 0 || length < 1 || length > RecipeProperties::maximumLength(candidate.dataType) || count < 1 || count > 200 || lines.size() != 11 + length) { return csvError(error, QStringLiteral("配方CSV的类型、尺寸或数据行数无效。")); }
    for (index = 5; index <= 8; ++index) { if (index == 6) { continue; } if (metadata[index] != QStringLiteral("0") && metadata[index] != QStringLiteral("1")) { return false; } }
    candidate.description = metadata[1]; candidate.writeEnabled = metadata[5] == QStringLiteral("1"); candidate.writeAddress = metadata[6]; candidate.readEnabled = metadata[7] == QStringLiteral("1"); candidate.sameReadWrite = metadata[8] == QStringLiteral("1"); candidate.readAddress = metadata[9]; candidate.itemNames.clear(); candidate.recipeNames.clear(); candidate.values.clear(); candidate.length = 0; candidate.resizeTable(length, count);
    fields = lines[10].split(QLatin1Char(',')); if (fields.size() != count + 1 || !fields.last().isEmpty()) { return csvError(error, QStringLiteral("配方CSV列名数量与配方总数不一致。")); } fields.removeLast(); candidate.recipeNames = fields;
    for (row = 0; row < length; ++row) { fields = lines[11 + row].split(QLatin1Char(',')); if (fields.size() != count + 1 || !fields.last().isEmpty()) { return csvError(error, QStringLiteral("配方CSV数据列数不一致。")); } fields.removeLast(); for (column = 0; column < count; ++column) { candidate.values[row][column] = fields[column]; } }
    if (!candidate.validate(error)) { return false; } *name = nextName; *output = candidate; return true;''','bytes, name, output, error','True after complete bounded exchange validation.')
put('src/recipe_csv.cpp',s)
print('Created strict atomic CP936 recipe exchange codec for observed native metadata and active values.')
