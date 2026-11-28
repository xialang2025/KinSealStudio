#include <QFontMetricsF>
#include <QTextLayout>
#include <QGlyphRun>
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <QRawFont>
#include <QtEndian>

/******************************************************************************
 * Function Name   : nativeKernDistance
 * Description     : Read a horizontal TrueType pair and reproduce Qt 4 Windows pixel rounding.
 * Input Parameters: sorted six-byte pairs, adjacent glyph indices, 26.6 design-to-device divisor
 * Return Value    : Signed kerning distance in pixels.
 * Date            : 2026.11.28
 ******************************************************************************/
static int nativeKernDistance(QByteArrayView pairs, quint32 left, quint32 right, int divisor) //20261128-10 add
{
    const quint32 key = (left << 16) | right;
    qsizetype low = 0, high = pairs.size() / 6;
    qsizetype middle;
    const char *record;
    int design, fixed;

    while (low < high) {
        middle = low + (high - low) / 2;
        record = pairs.data() + middle * 6;
        if (qFromBigEndian<quint32>(record) < key) { low = middle + 1; }
        else { high = middle; }
    }
    if (low == pairs.size() / 6) { return 0; }
    record = pairs.data() + low * 6;
    if (qFromBigEndian<quint32>(record) != key) { return 0; }
    design = qFromBigEndian<qint16>(record + 4);
    fixed = (qAbs(design) * 4096 + divisor / 2) / divisor;
    return ((design < 0 ? -fixed : fixed) + 32) >> 6;
}

/******************************************************************************
 * Function Name   : layoutNativeKernedLine
 * Description     : Position a simple Latin glyph run using the native Windows font's full cell height.
 * Input Parameters: line, text, cellHeight, output glyph run, output width, hasKern
 * Return Value    : True when every visible character has a supported glyph and advance.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool layoutNativeKernedLine(const QTextLine &line, QStringView text, int cellHeight,
                                   QGlyphRun &run, qreal &advance, bool &hasKern) //20261128-10 add
{
    QList<QGlyphRun> runs;
    QRawFont face;
    QList<quint32> glyphs;
    QList<QPointF> positions, advances;
    QByteArray table;
    QByteArrayView pairs;
    qsizetype size, count;
    int adjustment, divisor;
    advance = 0;

    // Keep the shaping engine for fallback fonts, ligatures, bidirectional runs and complex scripts.
    if (text.isEmpty() || cellHeight <= 0 || std::any_of(text.begin(), text.end(), [](QChar c) {
            return c.unicode() < 0x20 || c.unicode() > 0x7e;
        })) { return false; }
    runs = line.glyphRuns();
    if (runs.size() != 1 || runs.first().isRightToLeft()) { return false; }
    run = runs.first();
    face = run.rawFont();
    glyphs = run.glyphIndexes();
    if (glyphs.size() != text.size() || glyphs != face.glyphIndexesForString(text.toString())) { return false; }
    table = face.fontTable("kern");
    // Only a single version-zero horizontal pair table has this representation.
    if (table.size() < 18 || qFromBigEndian<quint32>(table.constData()) != 1 ||
        qFromBigEndian<quint16>(table.constData() + 4) != 0 ||
        qFromBigEndian<quint16>(table.constData() + 8) != 1) { return false; }
    size = qFromBigEndian<quint16>(table.constData() + 6);
    count = qFromBigEndian<quint16>(table.constData() + 10);
    if (size < 14 + count * 6 || size > table.size() - 4) { return false; }
    pairs = QByteArrayView(table).sliced(18, count * 6);
    divisor = int(face.unitsPerEm() * 64) / cellHeight;
    if (divisor <= 0) { return false; }
    advances = face.advancesForGlyphIndexes(glyphs, QRawFont::SeparateAdvances);
    positions = run.positions();
    if (advances.size() != glyphs.size() || positions.size() != glyphs.size()) { return false; }
    for (qsizetype i = 0; i < glyphs.size(); ++i) {
        positions[i].setX(advance);
        adjustment = i + 1 < glyphs.size() ? nativeKernDistance(pairs, glyphs[i], glyphs[i + 1], divisor) : 0;
        hasKern |= adjustment != 0;
        advance += qRound(advances[i].x()) + adjustment;
    }
    run.setPositions(positions);
    return true;
}

/******************************************************************************
 * Function Name   : paintNativeKernedText
 * Description     : Draw unwrapped Latin text with native GDI cell metrics when the font applies pair kerning.
 * Input Parameters: painter, rect, text, properties, resolved font
 * Return Value    : True after drawing every line, false to use the general layout.
 * Date            : 2026.11.28
 ******************************************************************************/
bool paintNativeKernedText(QPainter *painter, const QRectF &rect, const QString &text, //20261128-20 change
                                  const StaticTextProperties &properties, const QFont &font) //20261128-10 add
{
    LOGFONTW description = {};
    TEXTMETRICW metrics = {};
    HDC dc;
    HFONT handle;
    HGDIOBJ old;
    QList<QGlyphRun> runs;
    QList<qreal> widths;
    QList<QPointF> positions;
    const auto lines = QStringView(text).split(QLatin1Char('\n'));
    qreal x, y, width, scale;
    bool valid, hasKern = false;

    if (properties.adaptive || properties.spacing != 0 || properties.underline || properties.italic ||
        font.pixelSize() <= 0 || text.isEmpty() || std::any_of(text.begin(), text.end(), [](QChar c) {
            return c != QLatin1Char('\n') && (c.unicode() < 0x20 || c.unicode() > 0x7e);
        })) { return false; }
    description.lfHeight = -font.pixelSize();
    description.lfWeight = properties.bold ? FW_BOLD : FW_NORMAL;
    description.lfCharSet = DEFAULT_CHARSET;
    description.lfQuality = NONANTIALIASED_QUALITY;
    font.family().left(LF_FACESIZE - 1).toWCharArray(description.lfFaceName);
    dc = CreateCompatibleDC(nullptr);
    if (!dc) { return false; }
    handle = CreateFontIndirectW(&description);
    old = handle ? SelectObject(dc, handle) : nullptr;
    valid = old && old != HGDI_ERROR && GetTextMetricsW(dc, &metrics) && metrics.tmHeight > 0;
    if (old && old != HGDI_ERROR) { SelectObject(dc, old); }
    if (handle) { DeleteObject(handle); }
    DeleteDC(dc);
    if (!valid) { return false; }

    // Validate all lines before touching the painter so a fallback cannot leave a partially painted object.
    for (const QStringView part : lines) {
        QGlyphRun run;
        width = 0;
        if (!part.isEmpty()) {
            QTextLayout layout(part.toString(), font, painter->device());
            layout.beginLayout();
            QTextLine line = layout.createLine();
            if (!line.isValid()) { layout.endLayout(); return false; }
            line.setLineWidth(0x01000000);
            layout.endLayout();
            if (!layoutNativeKernedLine(line, part, metrics.tmHeight, run, width, hasKern)) { return false; }
            // QTextLayout positions include its own ascent. Align the same glyphs to the native GDI baseline.
            positions = run.positions();
            for (QPointF &position : positions) { position.ry() += metrics.tmAscent - line.ascent(); }
            run.setPositions(positions);
        }
        runs.append(run);
        widths.append(width);
    }
    // A lone Latin glyph still uses the GDI ascent and whole-cell vertical placement.
    if (!hasKern && text.size() != 1) { return false; }
    y = (rect.height() - (lines.size() * (metrics.tmHeight + metrics.tmExternalLeading) - metrics.tmExternalLeading)) * 0.5;
    scale = painter->transform().m22();
    if (painter->transform().type() <= QTransform::TxScale && scale != 0) {
        y = -std::floor(-y * scale + 0.5) / scale;
    }
    for (qsizetype i = 0; i < runs.size(); ++i) {
        x = properties.alignment == 1 ? 0 : (rect.width() - widths[i]) / (properties.alignment == 2 ? 1 : 2);
        if (!runs[i].isEmpty()) { painter->drawGlyphRun(rect.topLeft() + QPointF(x, y), runs[i]); }
        y += metrics.tmHeight + metrics.tmExternalLeading;
    }
    return true;
}
