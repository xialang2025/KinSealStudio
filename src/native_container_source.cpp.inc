//20261127-11 add: V122 ContainerItem and GraphicsGroup records use counted children.

/******************************************************************************
 * Function Name   : readNativeGroupChildren
 * Description     : Read the complete, nested preview group before consuming object-specific data.
 * Input Parameters: doc, group prefix, recursive nesting depth
 * Return Value    : None; the first malformed or unknown field is recorded in doc.
 * Date            : 2026.11.27
 ******************************************************************************/
static void readNativeGroupChildren(SourceDocument &doc, const QString &prefix, int depth)
{
    int count;
    int elements;
    int type;
    int i;
    int j;
    QString child;

    if (depth > 32) {
        doc.reject(prefix, QStringLiteral("子图元嵌套超过 32 层"));
        return;
    }
    count = doc.count(prefix + QStringLiteral("children"), 16384);
    for (i = 0; i < count && doc.error.isEmpty(); ++i) {
        child = SourceDocument::indexed(prefix + QStringLiteral("child"), i) + QLatin1Char('.');
        type = doc.read('i', child + QStringLiteral("type")).toInt();
        if (doc.value(QStringLiteral("header.1")).toInt() == 113 &&
            type != 0x1000f && type != 13 && type != 0x10018 && type != 0x10016) {
            doc.reject(child, QStringLiteral("尚未核实的 V113 子图元类型 %1").arg(type));
            return;
        }
        switch (type) {
        case 0x1000f:
            doc.fields("ddddcicciddibqii", child + QStringLiteral("rectangle"));
            doc.list(child + QStringLiteral("rectangle.labels"));
            doc.read('i', child + QStringLiteral("rectangle.1c"));
            break;
        case 13:
            doc.fields("qdddddddbi", child + QStringLiteral("pixmap"));
            doc.list(child + QStringLiteral("pixmap.labels"));
            if (doc.value(QStringLiteral("header.1")).toInt() > 114) { doc.read('i', child + QStringLiteral("pixmap.1c")); }
            break;
        case 0x10018:
            doc.fields("dddddqddbi", child + QStringLiteral("project-picture"));
            doc.list(child + QStringLiteral("project-picture.labels"));
            doc.read('i', child + QStringLiteral("project-picture.1c"));
            break;
        case 0x10016:
            doc.fields("ddddq", child + QStringLiteral("text.head"));
            doc.read('q', child + QStringLiteral("font.family"));
            doc.fields("diaaaaasaii", child + QStringLiteral("font.tail"));
            doc.fields("ccciibciqiddbi", child + QStringLiteral("text.tail"));
            doc.list(child + QStringLiteral("text.labels"));
            doc.fields("bd", child + QStringLiteral("text.end"));
            break;
        case 0x10010:
            doc.fields("ciddddqiiidccii", child + QStringLiteral("ellipse"));
            doc.list(child + QStringLiteral("ellipse.labels"));
            doc.fields("ddbi", child + QStringLiteral("ellipse.pos"));
            doc.list(child + QStringLiteral("ellipse.names"));
            doc.fields("di", child + QStringLiteral("ellipse.tail"));
            break;
        case 0x10011:
        case 0x10013:
        case 0x10014:
            elements = doc.count(child + QStringLiteral("path"), 16384);
            for (j = 0; j < elements && doc.error.isEmpty(); ++j) {
                doc.fields("idd", SourceDocument::indexed(child + QStringLiteral("element"), j));
            }
            if (elements > 0) { doc.fields("ii", child + QStringLiteral("path.tail")); }
            doc.fields("iicddbqiiii", child + QStringLiteral("stroke"));
            doc.list(child + QStringLiteral("stroke.labels"));
            doc.read('i', child + QStringLiteral("stroke.end"));
            break;
        case 0x10012:
            doc.read('q', child + QStringLiteral("polygon.name"));
            elements = doc.count(child + QStringLiteral("polygon.points"));
            for (j = 0; j < elements && doc.error.isEmpty(); ++j) {
                doc.fields("dd", SourceDocument::indexed(child + QStringLiteral("point"), j));
            }
            doc.fields("iiiccciddbi", child + QStringLiteral("polygon.style"));
            doc.list(child + QStringLiteral("polygon.labels"));
            doc.read('i', child + QStringLiteral("polygon.end"));
            break;
        case 100:
            readNativeGroupChildren(doc, child, depth + 1);
            break;
        default:
            doc.reject(child, QStringLiteral("尚未映射的预览子图元类型 %1").arg(type));
            break;
        }
    }
    type = doc.read('i', prefix + QStringLiteral("group.metadata-type")).toInt();
    doc.read('q', prefix + QStringLiteral("group.metadata-name"));
    if (type == 100 || type == 101) { doc.fields("iiii", prefix + QStringLiteral("group.bounds")); }
    doc.read('i', prefix + QStringLiteral("group.1c"));
}

/******************************************************************************
 * Function Name   : SourceDocument::container
 * Description     : Read the preview group and the common permission, visibility and notification fields.
 * Input Parameters: field path prefix
 * Return Value    : None; failures are latched on the document.
 * Date            : 2026.11.27
 ******************************************************************************/
void SourceDocument::container(const QString &prefix) //20261127-11 add
{
    readNativeGroupChildren(*this, prefix, 0);
    commonControl(prefix);
}
