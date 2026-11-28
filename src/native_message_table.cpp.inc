#include "recipe_csv.h"
#include <QTemporaryFile>
#include <QtEndian>

/******************************************************************************
 * Function Name   : buildNativeMessageRecords
 * Description     : Encode the same language-major CP936 records for project, media and standalone outputs.
 * Input Parameters: message control, project language order, output bytes, diagnostic
 * Return Value    : True when all rows can be encoded with the native 100-byte copy rule.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool buildNativeMessageRecords(const MessageDisplayProperties &message, const QStringList &languages,
                                      QByteArray *result, QString *error) //20261128-24 add
{
    QByteArray records, text;
    QColor color;
    QString value;
    int state;
    for (const QString &language : languages) {
        color = message.languageAppearance(language).textColor;
        for (state = 0; state < message.stateCount; ++state) {
            value = message.text(state, language);
            text.clear();
            if (!color.isValid() || value.contains(QChar(0)) || (!value.isEmpty() && !encodeNativeCp936(value, &text))) {
                if (error) { *error = QStringLiteral("消息文本须能以简体中文代码页表示，不可含内嵌零字符，并须指定有效文字颜色。"); }
                return false;
            }
            // V5.1 writes R,G,B,0 and copies at most 100 encoded bytes, even across a two-byte character.
            records.append(char(color.red()));
            records.append(char(color.green()));
            records.append(char(color.blue()));
            records.append(char(0));
            text.truncate(100);
            records.append(text);
            records.append(100 - text.size(), char(0));
        }
    }
    *result = records;
    return true;
}

/******************************************************************************
 * Function Name   : writeNativeMessageTable
 * Description     : Write the measured V122 single-message layout, with states contiguous inside each language.
 * Input Parameters: loaded project, new DAT destination, optional native simulator session profile, error
 * Return Value    : True after all rows and encodings have passed validation and the new file is published.
 * Date            : 2026.11.28
 ******************************************************************************/
bool writeNativeMessageTable(const HmiProject &project, const QString &path, const QString &session, QString *error) //20261128-23 add
{
    const QFileInfo target(path);
    QTemporaryFile output(target.absolutePath() + QStringLiteral("/.kinseal-message-XXXXXX"));
    QByteArray records, header;
    if (!project.validateSinglePageBackend(error, false)) { return false; }
    if (project.objects.size() != 1 || project.objects.first().type != QStringLiteral("message") ||
        project.languages.isEmpty() || target.suffix().compare(QStringLiteral("dat"), Qt::CaseInsensitive) ||
        target.exists() || target.isSymLink()) {
        *error = QStringLiteral("消息表导出需要单消息控件画面和新的 .dat 文件名。"); return false;
    }
    if (!buildNativeMessageRecords(project.objects.first().messageDisplay, project.languages, &records, error)) {
        return false;
    }
    if (!session.isEmpty()) {
        // The V5.1 type-35 writer rounds the scene bounds, then accumulates width * height * 2 at +0x844.
        // The simulator's 512-byte copy begins at +0x690, placing that RGB565 size at file offset 436.
        const bool initialized = session == QStringLiteral("download-defaults");
        const QSize size = project.objects.first().geometry.size();
        if ((!initialized && session != QStringLiteral("fresh")) || records.size() > 65535 ||
            size.width() < 1 || size.height() < 1 || size.width() > 4096 || size.height() > 4096 ||
            project.pageWidth != 800 || project.pageHeight != 480 ||
            !project.macros.isEmpty() || !project.pictures.isEmpty() || !project.animations.isEmpty()) {
            *error = QStringLiteral("模拟消息头支持 800×480 单页单消息及 fresh/download-defaults 会话。"); return false;
        }
        header = QByteArray(512, char(0));
        header[0] = 1;
        qToLittleEndian<quint16>(quint16(records.size()), header.data() + 2);
        for (int offset : {404, 408, 412}) { qToLittleEndian<quint32>(initialized ? 1U : 0U, header.data() + offset); }
        qToLittleEndian<quint32>(1U, header.data() + 428);
        qToLittleEndian<quint32>(1U, header.data() + 432);
        qToLittleEndian<quint32>(quint32(size.width() * size.height() * 2), header.data() + 436);
    }
    if (!output.open() || output.write(header) != header.size() || output.write(records) != records.size() ||
        !output.flush() || !output.rename(target.absoluteFilePath())) {
        *error = output.errorString(); return false;
    }
    output.setAutoRemove(false);
    return true;
}
