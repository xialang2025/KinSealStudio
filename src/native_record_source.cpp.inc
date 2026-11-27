//20261127-11 add: Record layouts follow the V122 writers and their matching native files.

/******************************************************************************
 * Function Name   : nativeRecordPattern
 * Description     : Read primitives, complete addresses (A) and QString lists (L) by field index.
 * Input Parameters: source document, named field block, V122 grammar
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
static void nativeRecordPattern(SourceDocument &d, const QString &prefix, const QByteArray &pattern)
{
    QString name;
    int i;

    for (i = 0; i < pattern.size() && d.error.isEmpty(); ++i) {
        name = prefix + QLatin1Char('.') + QString::number(i);
        if (pattern.at(i) == 'A') { d.address(name); }
        else if (pattern.at(i) == 'L') { d.list(name); }
        else { d.read(pattern.at(i), name); }
    }
}

/******************************************************************************
 * Function Name   : nativeRecordRows
 * Description     : Consume a counted private collection that owns the following rows.
 * Input Parameters: document, path, grammar of one row
 * Return Value    : None
 * Date            : 2026.11.27
 ******************************************************************************/
static void nativeRecordRows(SourceDocument &d, const QString &name, const QByteArray &pattern)
{
    const int count = d.count(name, 4096);
    int i;

    for (i = 0; i < count && d.error.isEmpty(); ++i) {
        nativeRecordPattern(d, SourceDocument::indexed(name, i), pattern);
    }
}

/******************************************************************************
 * Function Name   : retainedContainer
 * Description     : Select known V122 control bodies that can be preserved by the source editor.
 * Input Parameters: root type
 * Return Value    : True if its private reader is defined below.
 * Date            : 2026.11.27
 ******************************************************************************/
static bool retainedContainer(int type)
{
    static const QSet<int> types = {11,14,18,19,20,21,30,31,32,33,35,38,39,40,47,50,51,113};
    return types.contains(type);
}

/******************************************************************************
 * Function Name   : readRetainedRecord
 * Description     : Traverse a V122 control through its final private field without searching for a subsequent root tag.
 * Input Parameters: source document, object prefix, root type
 * Return Value    : None; the document records the first invalid field.
 * Date            : 2026.11.27
 ******************************************************************************/
static void readRetainedRecord(SourceDocument &d, const QString &prefix, int type)
{
    const QString body = prefix + QStringLiteral("private");
    QString row;
    QByteArray pattern;
    int mode;
    int count;
    int i;
    int j;
    int languages;
    int language;

    d.container(prefix);
    if (!d.error.isEmpty()) { return; }
    switch (type) {
    case 11:
        mode = d.read('i', body + QStringLiteral(".mode")).toInt();
        if (mode == 0) { pattern = "iiqAAAssiqqsbbisqidsqqbddibbsbsqqbssiiiiiqqqiiiiiibibsqqqqAAsssbbqd"; }
        else if (mode == 2) { pattern = "iiqsssqqbddbqqqssdbbisssssbsqqqqAAAAAbi"; }
        else if (mode == 3) { pattern = "iqsiiiiiicAbddi"; }
        else { d.reject(body, QStringLiteral("尚未核实图表模式 %1").arg(mode)); }
        break;
    case 14: case 40: case 47: case 113:
        pattern = type == 47 ? "ccqiii" : type == 14 ? "biiiAAAibbccc" : "biiAAAiibbccc";
        nativeRecordPattern(d, body, pattern);
        count = d.value(body + (type == 47 ? QStringLiteral(".5") : type == 14 ? QStringLiteral(".2") : QStringLiteral(".1"))).toInt();
        if (count < 0 || count > 4096) { d.reject(body, QStringLiteral("通道数量超出范围")); return; }
        for (i = 0; i < count && d.error.isEmpty(); ++i) {
            nativeRecordPattern(d, SourceDocument::indexed(body + QStringLiteral(".channel"), i), type == 47 ? "ciddq" : "cidd");
        }
        nativeRecordPattern(d, body + QStringLiteral(".tail"), type == 47 ? "bbccssbsddssbssssbbccssbsddss" :
                            type == 14 ? "bbccssbsddssbbccssbssAddss" : "bbccssbsddssbbccssbsddss");
        return;
    case 18: case 20:
        nativeRecordRows(d, body + QStringLiteral(".columns"), "iqqqqqq");
        pattern = type == 18 ? "bbbbciiqccccLii" : "bbbbciiqcccc";
        break;
    case 19: pattern = "icbiicciiiiicL"; break;
    case 21: pattern = "biiiisqqqqqqsqssssqqqAbss"; break;
    case 30: pattern = "qaaassqqqqaccaaaaaaaccccaa"; break;
    case 31: pattern = "bqisiisqsqqssiiqsq"; break;
    case 32:
        nativeRecordPattern(d, body + QStringLiteral(".header"), "ccsscbbccbq");
        nativeRecordRows(d, body + QStringLiteral(".visible"), "b");
        nativeRecordPattern(d, body + QStringLiteral(".style"), "bbcii");
        nativeRecordRows(d, body + QStringLiteral(".names"), "qqs");
        nativeRecordRows(d, body + QStringLiteral(".translations"), "L");
        return;
    case 33: pattern = "aAaaqAcccaaaccciaaccaiiaqaaaaaiciiiciicqiiacAAqq"; break;
    case 35:
        nativeRecordPattern(d, body + QStringLiteral(".head"), "iqbAsss");
        count = d.count(body + QStringLiteral(".states"), 4096);
        for (j = 0; j < 2; ++j) {
            if (j) { d.read('s', body + QStringLiteral(".extended")); }
            for (i = 0; i < count && d.error.isEmpty(); ++i) {
                row = SourceDocument::indexed(body + (j ? QStringLiteral(".extendedState") : QStringLiteral(".state")), i);
                languages = d.count(row + QStringLiteral(".languages"), 32);
                nativeRecordPattern(d, row + QStringLiteral(".head"), j ? "cccii" : "ccci");
                for (language = 0; language < languages && d.error.isEmpty(); ++language) {
                    nativeRecordPattern(d, SourceDocument::indexed(row + QStringLiteral(".language"), language), j ? "sqscqcccii" : "sqsc");
                }
            }
        }
        return;
    case 38:
        nativeRecordPattern(d, body + QStringLiteral(".head"), "bqqi");
        count = d.count(body + QStringLiteral(".text"));
        for (i = 0; i < count && d.error.isEmpty(); ++i) {
            nativeRecordRows(d, SourceDocument::indexed(body + QStringLiteral(".text"), i), "cqiiq");
        }
        nativeRecordRows(d, body + QStringLiteral(".image"), "iqiic");
        count = d.count(body + QStringLiteral(".command"));
        for (i = 0; i < count && d.error.isEmpty(); ++i) {
            row = SourceDocument::indexed(body + QStringLiteral(".command"), i);
            nativeRecordPattern(d, row + QStringLiteral(".op"), "sAsAssdsqbb");
            if (d.value(row + QStringLiteral(".op.10")).toInt()) { d.address(row + QStringLiteral(".thirdAddress")); }
            d.fields("bb", row + QStringLiteral(".flags"));
        }
        nativeRecordPattern(d, body + QStringLiteral(".tail"), "Abibiii");
        nativeRecordRows(d, body + QStringLiteral(".values"), "d");
        nativeRecordPattern(d, body + QStringLiteral(".end"), "iibiibbb");
        return;
    case 39:
        nativeRecordPattern(d, body, "iiAccsibqissc");
        nativeRecordRows(d, body + QStringLiteral(".items"), "iqq");
        return;
    case 50: pattern = "qAissqAqAiqiic"; break;
    case 51:
        nativeRecordPattern(d, body + QStringLiteral(".head"), "bbaiiqAqqqqiiqiiqbbbbqiiiqqq");
        nativeRecordRows(d, body + QStringLiteral(".items"), "qqis");
        nativeRecordPattern(d, body + QStringLiteral(".tail"), "qbbbiiiiqqis");
        return;
    default:
        d.reject(body, QStringLiteral("未映射的容器类型 %1").arg(type));
        return;
    }
    nativeRecordPattern(d, body, pattern);
}
