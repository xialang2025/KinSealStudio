#include "native_test_project.h"

#include <QByteArray>
#include <QCryptographicHash>
#include <QFile>
#include <QFileInfo> //20261108-9 add
#include <QSaveFile>

using u8 = unsigned char; //20261108-9 add
using u16 = unsigned short; //20261108-7 add

/******************************************************************************
 * Function Name   : exportNativeTestProject
 * Description     : Export the verified one-text native project with a fixed four-character label.
 * Input Parameters: project, path, error
 * Return Value    : True when the native .sm file was written atomically.
 * Date            : 2026.11.08
 ******************************************************************************/
bool exportNativeTestProject(const HmiProject &project, const QString &path,
                             QString *error) //20261108-7 add
{
    QFile template_file(QStringLiteral(":/samples/native_text.sm"));
    QSaveFile output_file(path);
    QByteArray project_bytes;
    QByteArray original_text;
    QByteArray replacement_text;
    QString display_text;
    int character_index;
    int occurrence_count;
    int search_position;
    int match_position;
    u16 character_code;

    if (!project.validateSinglePageBackend(error)) //20261110-5 add: The old seed is never a multi-page source exporter.
    {
        return false;
    }
    if (project.languages != QStringList{QStringLiteral("中文")} || !project.connections.isEmpty() ||
        !project.macros.isEmpty() || !project.pictures.isEmpty()) //20261110-5 add: A fixed fixture cannot silently remove additional project configuration.
    {
        if (error != nullptr) { *error = QStringLiteral("固定原厂测试模板仅支持默认中文语言且无额外连接/宏/图片资源；请使用完整原厂源导出。"); }
        return false;
    }
    if (project.targetModel != QStringLiteral("AMT070W124-L800X480R") ||
        project.pageWidth != 800 || project.pageHeight != 480 ||
        project.objects.size() != 1 ||
        project.objects.first().type != QStringLiteral("text") ||
        project.objects.first().hasStaticTextProperties || //20261108-14 add: Never silently discard appearance or permission properties in legacy export.
        project.objects.first().geometry != QRect(244, 112, 150, 50))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("原生测试工程仅支持目标机型、一个画面和固定位置的单个静态文本（244,112,150,50）。");
        }
        return false;
    }

    display_text = project.objects.first().text;
    if (display_text.size() != 4)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("原生测试工程的文本必须恰好为 4 个 ASCII 字母或数字。");
        }
        return false;
    }
    original_text = QByteArray::fromHex("0054004500580054"); // UTF-16BE "TEXT" in the verified template.
    for (character_index = 0; character_index < display_text.size(); ++character_index)
    {
        character_code = display_text.at(character_index).unicode();
        if (!((character_code >= 'A' && character_code <= 'Z') ||
              (character_code >= 'a' && character_code <= 'z') ||
              (character_code >= '0' && character_code <= '9')))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("原生测试工程的文本必须恰好为 4 个 ASCII 字母或数字。");
            }
            return false;
        }
        replacement_text.append(char(0));
        replacement_text.append(char(character_code));
    }

    if (!template_file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("内置原生工程模板无法读取。");
        }
        return false;
    }
    project_bytes = template_file.readAll();
    if (QCryptographicHash::hash(project_bytes, QCryptographicHash::Sha256).toHex() !=
        QByteArray("336ea4ca4d0883a58eea5342113f53884d9416692f93dfd7dd8c4ea2c1af7ef7"))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("原生模板校验失败，已停止导出。");
        }
        return false;
    }
    occurrence_count = 0;
    search_position = 0;
    while ((match_position = project_bytes.indexOf(original_text, search_position)) >= 0)
    {
        project_bytes.replace(match_position, original_text.size(), replacement_text);
        search_position = match_position + replacement_text.size();
        ++occurrence_count;
    }
    if (occurrence_count != 3 || project_bytes.size() != 3887)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("原生模板结构与已验证版本不一致，已停止导出。");
        }
        return false;
    }
    if (!project.nativeSourceSnapshot.isEmpty() && project.nativeSourceSnapshot != project_bytes) //20261110-5 add: Preserve an existing source by refusing unrepresented template metadata.
    {
        if (error != nullptr) { *error = QStringLiteral("现有原厂源与固定测试模板不一致，已拒绝覆盖；请使用完整原厂源导出。"); }
        return false;
    }
    if (!output_file.open(QIODevice::WriteOnly) ||
        output_file.write(project_bytes) != project_bytes.size() ||
        !output_file.commit())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("原生测试工程写入失败：%1").arg(output_file.errorString());
        }
        return false;
    }
    return true;
}

/******************************************************************************
 * Function Name   : importNativeTestProject
 * Description     : Import only the verified native one-text template without accepting unknown binary layouts.
 * Input Parameters: path, project, error
 * Return Value    : True when the native file matches the verified template and text slots.
 * Date            : 2026.11.08
 ******************************************************************************/
bool importNativeTestProject(const QString &path, HmiProject *project,
                             QString *error) //20261108-9 add
{
    QFile template_file(QStringLiteral(":/samples/native_text.sm"));
    QFile source_file(path);
    QByteArray template_bytes;
    QByteArray expected_bytes;
    QByteArray source_bytes;
    QByteArray original_text;
    QByteArray found_text;
    QByteArray first_text;
    HmiProject imported_project;
    HmiObject object;
    QString display_text;
    int search_position;
    int match_position;
    int occurrence_count;
    int character_index;
    u8 character_code;

    if (project == nullptr)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("工程接收位置为空。");
        }
        return false;
    }
    if (!template_file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("内置原生工程模板无法读取。");
        }
        return false;
    }
    if (!source_file.open(QIODevice::ReadOnly))
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("无法读取原生工程：%1").arg(source_file.errorString());
        }
        return false;
    }

    template_bytes = template_file.readAll();
    source_bytes = source_file.readAll();
    // 20261108-9 add: Anchor the importer to the same immutable vendor template as the exporter.
    if (QCryptographicHash::hash(template_bytes, QCryptographicHash::Sha256).toHex() !=
            QByteArray("336ea4ca4d0883a58eea5342113f53884d9416692f93dfd7dd8c4ea2c1af7ef7") ||
        template_bytes.size() != 3887 || source_bytes.size() != template_bytes.size())
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("当前只支持已验证的单文本 .sm 工程；其他原厂工程尚不能导入。");
        }
        return false;
    }

    expected_bytes = template_bytes;
    original_text = QByteArray::fromHex("0054004500580054");
    search_position = 0;
    occurrence_count = 0;
    while ((match_position = template_bytes.indexOf(original_text, search_position)) >= 0)
    {
        found_text = source_bytes.mid(match_position, original_text.size());
        if (occurrence_count == 0)
        {
            first_text = found_text;
        }
        if (found_text != first_text)
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("原生工程的文本字段不一致，无法安全导入。");
            }
            return false;
        }
        expected_bytes.replace(match_position, original_text.size(), found_text);
        search_position = match_position + original_text.size();
        ++occurrence_count;
    }
    if (occurrence_count != 3 || expected_bytes != source_bytes)
    {
        if (error != nullptr)
        {
            *error = QStringLiteral("当前只支持已验证的单文本 .sm 工程；其他原厂工程尚不能导入。");
        }
        return false;
    }

    // 20261108-9 add: Permit only four ASCII letters or digits in all three UTF-16BE text slots.
    for (character_index = 0; character_index < 4; ++character_index)
    {
        character_code = u8(first_text.at(character_index * 2 + 1));
        if (first_text.at(character_index * 2) != 0 ||
            !((character_code >= 'A' && character_code <= 'Z') ||
              (character_code >= 'a' && character_code <= 'z') ||
              (character_code >= '0' && character_code <= '9')))
        {
            if (error != nullptr)
            {
                *error = QStringLiteral("原生工程文本必须是四个 ASCII 字母或数字。");
            }
            return false;
        }
        display_text.append(QChar(u16(character_code)));
    }

    imported_project.name = QFileInfo(path).completeBaseName();
    object.id = QStringLiteral("native-test-text");
    object.type = QStringLiteral("text");
    object.name = QStringLiteral("测试文字");
    object.text = display_text;
    object.address = QString();
    object.geometry = QRect(244, 112, 150, 50);
    imported_project.objects.append(object);
    *project = imported_project;
    return true;
}
