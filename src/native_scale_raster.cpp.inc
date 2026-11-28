#include <QPainter>
#ifdef Q_OS_WIN
#include <qt_windows.h>
#endif

/******************************************************************************
 * Function Name   : nativeScaleRasterStyle
 * Description     : Check the original child pen, font, text alignment and drawing flags before rasterization.
 * Input Parameters: parsed V122 document, child path, text and top-side flags
 * Return Value    : True for the styles independently observed in the original scale pages.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool nativeScaleRasterStyle(const SourceDocument &doc, const QString &child, bool text, bool top) //20261128-34 add
{
    QByteArray style;
    const auto appendRange = [&](const QString &first, const QString &last) {
        const auto a = doc.values.constFind(child + first);
        const auto b = doc.values.constFind(child + last);
        if (a == doc.values.cend() || b == doc.values.cend() || b->end <= a->start) { return false; }
        style.append(doc.bytes.constData() + a->start, b->end - a->start);
        return true;
    };
    if (text) {
        if (!appendRange(QStringLiteral("font.family"), QStringLiteral("text.tail.7")) ||
            !appendRange(QStringLiteral("text.tail.9"), QStringLiteral("text.tail.9")) ||
            !appendRange(QStringLiteral("text.tail.12"), QStringLiteral("text.end.1"))) { return false; }
    } else if (!appendRange(QStringLiteral("path.tail.0"), QStringLiteral("stroke.2")) ||
               !appendRange(QStringLiteral("stroke.5"), QStringLiteral("stroke.5")) ||
               !appendRange(QStringLiteral("stroke.7"), QStringLiteral("stroke.end"))) { return false; }
    const QByteArray digest = QCryptographicHash::hash(style, QCryptographicHash::Sha256).toHex();
    if (!text) { return digest == "6a9edd07df6b614e0f223c16fb745b9388059a80c4a853574780e6892535bd16"; }
    return digest == (top ? "0d0dff97801962db4d427fe64daf453c72830831db81891803b5afb12c768fc3" :
                            "fadf3645f1ef0e81b3e69740f8b57fffe645780b43344536d6575b6bc2e6fccd");
}

/******************************************************************************
 * Function Name   : nativeScaleLabelMask
 * Description     : Draw the embedded monochrome SimSun strike into a private Windows memory bitmap.
 * Input Parameters: decimal caption, output mask
 * Return Value    : True if the requested native font and its verified 16-pixel metrics were available.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool nativeScaleLabelMask(const QString &caption, QImage *mask) //20261128-34 add
{
#ifdef Q_OS_WIN
    const int width = caption.size() * 8;
    BITMAPINFO info = {};
    TEXTMETRICW metrics = {};
    SIZE extent = {};
    wchar_t actual[LF_FACESIZE] = {};
    HDC dc = CreateCompatibleDC(nullptr);
    HFONT font = nullptr;
    HBITMAP bitmap = nullptr;
    HGDIOBJ oldFont = nullptr, oldBitmap = nullptr;
    void *pixels = nullptr;
    bool ok = false;
    if (!dc || width <= 0) { if (dc) { DeleteDC(dc); } return false; }
    info.bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
    info.bmiHeader.biWidth = width;
    info.bmiHeader.biHeight = -16;
    info.bmiHeader.biPlanes = 1;
    info.bmiHeader.biBitCount = 32;
    font = CreateFontW(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, DEFAULT_PITCH, L"SimSun");
    bitmap = CreateDIBSection(dc, &info, DIB_RGB_COLORS, &pixels, nullptr, 0);
    if (font && bitmap && pixels) {
        oldFont = SelectObject(dc, font);
        oldBitmap = SelectObject(dc, bitmap);
        const bool selected = oldFont && oldFont != HGDI_ERROR && oldBitmap && oldBitmap != HGDI_ERROR;
        const auto data = reinterpret_cast<const wchar_t *>(caption.utf16());
        if (selected && GetTextFaceW(dc, LF_FACESIZE, actual) &&
            (QString::fromWCharArray(actual).compare(QStringLiteral("SimSun"), Qt::CaseInsensitive) == 0 ||
             QString::fromWCharArray(actual) == QStringLiteral("宋体")) &&
            GetTextMetricsW(dc, &metrics) && metrics.tmHeight == 16 && metrics.tmAscent == 14 &&
            GetTextExtentPoint32W(dc, data, caption.size(), &extent) && extent.cx == width &&
            PatBlt(dc, 0, 0, width, 16, WHITENESS) && SetBkMode(dc, TRANSPARENT) &&
            SetTextColor(dc, RGB(0,0,0)) != CLR_INVALID && TextOutW(dc, 0, 0, data, caption.size()) && GdiFlush()) {
            *mask = QImage(static_cast<const u8 *>(pixels), width, 16, width * 4, QImage::Format_RGB32).copy();
            ok = !mask->isNull();
        }
    }
    if (oldBitmap && oldBitmap != HGDI_ERROR) { SelectObject(dc, oldBitmap); }
    if (oldFont && oldFont != HGDI_ERROR) { SelectObject(dc, oldFont); }
    if (bitmap) { DeleteObject(bitmap); }
    if (font) { DeleteObject(font); }
    DeleteDC(dc);
    return ok;
#else
    Q_UNUSED(caption);
    Q_UNUSED(mask);
    return false;
#endif
}

/******************************************************************************
 * Function Name   : hasReopenedDefaultScaleGeometry
 * Description     : Match the translated V122 right-side tick and label layout measured on both reopened originals.
 * Input Parameters: parsed source, scale object prefix, verified scale model
 * Return Value    : True when the seven horizontal endpoint changes have an original layout witness.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool hasReopenedDefaultScaleGeometry(const SourceDocument &doc, const QString &root,
                                            const HmiObject &object) //20261128-37 add
{
    const ScaleProperties &scale = object.scale;
    const double sx = 200.0 / 124.0, sy = 100.0 / 87.0;
    const QPointF origin = object.geometry.topLeft();
    QString child;
    double x, y, right, bottom, row;
    int i, label;
    const auto value = [&](const QString &field) { return doc.value(child + field).toDouble(); };
    const auto close = [](double a, double b) { return std::abs(a - b) <= 0.00000001; };

    if (object.geometry.size() != QSize(200, 100) || !object.subpixelOffset.isNull() ||
        scale.position != 3 || scale.direction != 0 || scale.minimum != 0 || scale.maximum != 100 ||
        scale.majorCount != 4 || scale.minorCount != 1 || scale.fontSize != 16 || scale.decimals != 0 ||
        !scale.showValues || scale.textColor != Qt::black || scale.lineColor != Qt::black ||
        doc.value(root + QStringLiteral("children.count")).toInt() != 12) { return false; }
    for (i = 0; i < 12; ++i) {
        child = SourceDocument::indexed(root + QStringLiteral("child"), i) + QLatin1Char('.');
        if (i > 0 && i <= 8 && i % 2 == 0) {
            label = i / 2 - 1;
            x = value(QStringLiteral("text.head.0")) + value(QStringLiteral("text.tail.10")) - origin.x();
            y = value(QStringLiteral("text.head.1")) + value(QStringLiteral("text.tail.11")) - origin.y();
            // The four original label boxes contain 100, 67, 33 and 0, in that order.
            if (!close(x, 68.0 * sx) || !close(y, label * 25.0 * sy) ||
                !close(value(QStringLiteral("text.head.2")), (label == 0 ? 56.0 : label == 3 ? 24.0 : 40.0) * sx) ||
                !close(value(QStringLiteral("text.head.3")), 12.0 * sy)) { return false; }
        } else {
            x = value(QStringLiteral("stroke.3")) - origin.x();
            y = value(QStringLiteral("stroke.4")) - origin.y();
            right = x + value(QStringLiteral("element[1].1"));
            bottom = y + value(QStringLiteral("element[1].2"));
            x += value(QStringLiteral("element[0].1"));
            y += value(QStringLiteral("element[0].2"));
            // The major ticks are 50 units long. The three halfway minor ticks are 12.5 units long.
            row = i == 0 ? 0.0 : i < 9 ? (i - 1) / 2.0 : (i - 9) + 0.5;
            if (!close(x, 0.0) || !close(y, (6.75 + 25.0 * row) * sy) ||
                !close(right, (i == 0 ? 0.0 : i < 9 ? 50.0 : 12.5) * sx) ||
                !close(bottom, i == 0 ? (6.75 + 75.0) * sy : y)) { return false; }
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : paintNativeScaleSource
 * Description     : Bake verified V122 scale child lines and captions onto the ordinary page in source order.
 * Input Parameters: current project including original scale layout, active page painter, diagnostic
 * Return Value    : True when every cached element was rendered.
 * Date            : 2026.11.28
 ******************************************************************************/
bool paintNativeScaleSource(const HmiProject &project, QPainter *painter, QString *error, bool reopenedProject) //20261128-37 change
{
    QByteArray source;
    QString child, element, caption;
    QImage mask;
    HmiObject verified;
    QRectF rect;
    QPoint pts[2], origin;
    int index, point, x, y;
    bool trimTickEnds;
    if (project.objects.size() != 1 ||
        !exportNativeProjectSource(project, &source, error)) { return false; }
    SourceDocument doc(source);
    if (!doc.parse() || doc.objectPrefixes.size() != 1) { return fail(error, QStringLiteral("刻度的原厂缓存子图元不完整。")); }
    const QString root = doc.objectPrefixes.first();
    if (!sourceNativeScale(doc, root, 0, &verified, error)) { return false; }
    trimTickEnds = reopenedProject && hasReopenedDefaultScaleGeometry(doc, root, verified); //20261128-37 add
    const bool top = verified.scale.position == 0;
    const int count = doc.value(root + QStringLiteral("children.count")).toInt();
    const auto number = [&](const QString &suffix) { return doc.value(child + suffix).toDouble(); };
    // Axis-aligned cosmetic paths use the Qt4 subpixel grid before the inclusive pixel span is written.
    const auto grid = [](double v) { return int(std::floor((v * 64.0 + 31.0) / 64.0)); };
    for (index = 0; index < count; ++index) {
        child = SourceDocument::indexed(root + QStringLiteral("child"), index) + QLatin1Char('.');
        const bool text = doc.value(child + QStringLiteral("type")).toInt() == 0x10016;
        if (!nativeScaleRasterStyle(doc, child, text, top)) { return fail(error, QStringLiteral("刻度子图元的笔型、字体或对齐样式尚无原厂像素对照。")); }
        if (!text) {
            for (point = 0; point < 2; ++point) {
                element = SourceDocument::indexed(QStringLiteral("element"), point);
                pts[point] = QPoint(grid(number(QStringLiteral("stroke.3")) + number(element + QStringLiteral(".1"))),
                                    grid(number(QStringLiteral("stroke.4")) + number(element + QStringLiteral(".2"))));
            }
            if (pts[0].x() != pts[1].x() && pts[0].y() != pts[1].y()) { return fail(error, QStringLiteral("刻度缓存包含非水平、垂直的刻线。")); }
            // In both independent reopened V5.1 pages only the seven rightmost horizontal pixels are absent.
            if (trimTickEnds && index != 0) { --pts[1].rx(); } //20261128-37 add
            painter->fillRect(QRect(pts[0], pts[1]).normalized(), verified.scale.lineColor);
            continue;
        }
        rect = QRectF(number(QStringLiteral("text.head.0")) + number(QStringLiteral("text.tail.10")),
                      number(QStringLiteral("text.head.1")) + number(QStringLiteral("text.tail.11")),
                      number(QStringLiteral("text.head.2")), number(QStringLiteral("text.head.3")));
        caption = doc.value(child + QStringLiteral("text.head.4")).toString();
        if (!nativeScaleLabelMask(caption, &mask)) { return fail(error, QStringLiteral("当前系统无法生成原厂 16 像素宋体刻度字形。")); }
        origin = QPoint(qRound(rect.x() + (top ? (rect.width() - mask.width()) / 2 : 0)),
                        qRound(rect.center().y()) - mask.height() / 2);
        for (y = 0; y < mask.height(); ++y) {
            for (x = 0; x < mask.width(); ++x) {
                if (qRed(mask.pixel(x, y)) == 0) { painter->fillRect(QRect(origin + QPoint(x,y), QSize(1,1)), verified.scale.textColor); }
            }
        }
    }
    return true;
}
