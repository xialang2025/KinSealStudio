/******************************************************************************
 * Function Name   : hasLocalStaticSystemPage
 * Description     : Check the separately verified object and macro restrictions on system-dialog snapshots.
 * Input Parameters: parsed native document, permitted object types, optional local message visibility
 * Return Value    : True for a local static page without schedule or external-address dependencies.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool hasLocalStaticSystemPage(const SourceDocument &document, const QList<int> &permitted = {28},
                                     bool localMessageVisibility = false) //20261128-40 change
{
    int type;

    if (document.value(QStringLiteral("system.174.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.178.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.194.count")).toInt() != 0) {
        return false;
    }
    for (const QString &prefix : document.objectPrefixes) {
        // The original message probe enables the local bit and level tests together.
        type = document.value(prefix + QStringLiteral("type")).toInt();
        if (!permitted.contains(type) ||
            (permitted.size() > 1 && document.value(prefix + QStringLiteral("common.58.2")).toInt() != 0) ||
            (document.value(prefix + QStringLiteral("common.visibility.0")).toInt() != 0 &&
             !(localMessageVisibility && type == 35 &&
               document.value(prefix + QStringLiteral("common.visibility.0")).toInt() == 1 &&
               document.value(prefix + QStringLiteral("common.visibility.1")).toInt() == 5 &&
               document.value(prefix + QStringLiteral("common.a8.0")).toInt() == 1 &&
               document.value(prefix + QStringLiteral("common.a8.1")).toInt() == 2)) ||
            document.value(prefix + QStringLiteral("common.touch.0")).toInt() != 0) {
            return false;
        }
        if (type == 36 && (document.value(prefix + QStringLiteral("table.10")).toInt() != 0 ||
                           document.value(prefix + QStringLiteral("table.columnControlled")).toInt() != 0 ||
                           document.value(prefix + QStringLiteral("table.rowAddress.1")).toInt() != 0 ||
                           document.value(prefix + QStringLiteral("table.columnAddress.1")).toInt() != 0 ||
                           document.value(prefix + QStringLiteral("common.58.2")).toInt() != 0)) {
            return false;
        }
        // A dormant external address is still a project dependency when the source is reopened for editing.
        for (const char *field : {"common.40.1", "common.e4.1", "common.90.1", "common.60.1",
                                  "common.b0.1", "common.fc.1"}) {
            if (document.value(prefix + QLatin1String(field)).toInt() != 0) {
                return false;
            }
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : hasLocalDisplaySystemPage
 * Description     : Verify the local connections of a single display before admitting its serial configuration.
 * Input Parameters: parsed document, whether this configuration was also observed with number, ASCII and time
 * Return Value    : True for a witnessed display family without a scheduled macro or an external address.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool hasLocalDisplaySystemPage(const SourceDocument &document, bool dataDisplays) //20261128-22 add
{
    QString prefix, address, family, digits;
    QStringList addresses;
    QList<int> addressFields;
    int type;
    bool ok;
    u32 number;

    if (document.objectPrefixes.size() != 1) {
        return false;
    }
    prefix = document.objectPrefixes.first();
    type = document.value(prefix + QStringLiteral("type")).toInt();
    if (type != 35 && !(dataDisplays && (type == 12 || type == 16 || type == 17))) {
        return false;
    }
    if (!hasLocalStaticSystemPage(document, {type}, type == 35) ||
        document.value(prefix + QStringLiteral("common.58.2")).toInt() != 0) {
        return false;
    }
    // Enumerate every address token in the V122 display records, including hidden limit and input addresses.
    addresses = {QStringLiteral("common.40"), QStringLiteral("common.e4"), QStringLiteral("common.90"),
                 QStringLiteral("common.60"), QStringLiteral("common.b0"), QStringLiteral("common.fc")};
    if (type == 35) {
        addresses.append(QStringLiteral("private.head.3"));
    } else if (type == 12 || type == 16) {
        if (document.value(prefix + (type == 12 ? QStringLiteral("display.18") : QStringLiteral("display.16"))).toBool()) {
            return false;
        }
        addressFields = type == 12 ? QList<int>{17, 23, 24, 28, 35, 36, 56} : QList<int>{13, 15, 21, 40};
        for (int index : addressFields) {
            addresses.append(QStringLiteral("display.") + QString::number(index));
        }
    }
    for (const QString &relative : addresses) {
        address = prefix + relative + QLatin1Char('.');
        family = document.value(address + QLatin1Char('5')).toString();
        digits = document.value(address + QLatin1Char('4')).toString();
        number = digits.toUInt(&ok, 10);
        if (!ok || number > 65535 || digits != QString::number(number) ||
            (family != QStringLiteral("LW") && family != QStringLiteral("LB")) ||
            document.value(address + QLatin1Char('0')).toString() != family + digits ||
            document.value(address + QLatin1Char('1')).toInt() != 0 ||
            !document.value(address + QLatin1Char('8')).toString().isEmpty()) {
            return false;
        }
    }
    return true;
}

/******************************************************************************
 * Function Name   : supportsReopenedStaticDrawing
 * Description     : Verify that a local drawing restored by V5.1 differs from the base profile only in its remembered page and two descriptor fields.
 * Input Parameters: parsed V122 document, validated normalization edits, byte range of the configuration.
 * Return Value    : True for local static objects without a scan, touch condition or macro dependency.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool supportsReopenedStaticDrawing(const SourceDocument &document, const SourceUpdates &validated,
                                          qsizetype start, qsizetype end) //20261128-40 add
{
    const QString remembered = document.value(QStringLiteral("config.head.12")).toString();
    SourceUpdates base(validated);
    QByteArray bytes;

    if (remembered.isEmpty() || remembered != document.value(QStringLiteral("page.head.18")).toString() ||
        document.value(QStringLiteral("project.pages.count")).toInt() != 1 || document.objectPrefixes.isEmpty() ||
        document.value(QStringLiteral("config.200.7")).toInt() != 7 ||
        document.value(QStringLiteral("config.240.7")).toInt() != 7 ||
        !hasLocalStaticSystemPage(document, {28, 36, 0x1002a})) {
        return false;
    }
    // Restore the null QString and the two original words for comparison only. Keep the saved SM intact.
    base.set(QStringLiteral("config.head.12"), QString());
    base.set(QStringLiteral("config.200.7"), 1);
    base.set(QStringLiteral("config.240.7"), 1);
    bytes = base.render(start, end);
    return base.error.isEmpty() && QCryptographicHash::hash(bytes, QCryptographicHash::Sha256).toHex() ==
           QByteArrayLiteral("ca3da07ec299bf5bedab28b73682a1b8ea0ee4867413dfc1a1ca4e91fda3c6ba");
}

/******************************************************************************
 * Function Name   : supportsNativeSystemDialogProfile
 * Description     : Admit the complete known static-text and static-table configuration profiles.
 * Input Parameters: parsed native document, normalized configuration digest
 * Return Value    : True for a known digest and a page without external consumers.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool supportsNativeSystemDialogProfile(const SourceDocument &document, const QByteArray &digest) //20261128-17 change
{
    const bool defaultPage = digest == QByteArrayLiteral("d44eb75f7476affc81162f95f805302932771ed3b716b225c0ae3c39fd692ae9");
    const bool rememberedPage = digest == QByteArrayLiteral("2c9d9ef3322d35d79d24112f89911d1522e81d213218292714a1617e39349e64");
    if ((defaultPage || rememberedPage) && hasLocalDisplaySystemPage(document, defaultPage)) { //20261128-22 add
        return true;
    }
    //20261128-21 add: The original picture-only probes use both complete serial profiles.
    // Neither profile has an external consumer in these projects, including the inactive address slots.
    if ((defaultPage || rememberedPage) &&
        !document.objectPrefixes.isEmpty() && (hasLocalStaticSystemPage(document, {15}) ||
        (defaultPage && hasLocalStaticSystemPage(document, {34})))) {
        for (const QString &prefix : document.objectPrefixes) {
            if (document.value(prefix + QStringLiteral("image.address.1")).toInt() != 0 ||
                document.value(prefix + QStringLiteral("gif.address.1")).toInt() != 0 ||
                document.value(prefix + QStringLiteral("common.58.2")).toInt() != 0) {
                return false;
            }
        }
        return true;
    }
    // Seven saved table configurations share this complete fingerprint; their inactive clock descriptors
    // and remembered page name differ from the original draw-only profile. The runtime gets only a page image.
    if (rememberedPage) {
        return document.objectPrefixes.size() == 1 && hasLocalStaticSystemPage(document, {36});
    }
    return (digest == QByteArrayLiteral("2e4607316f881af776e1bc9d5992c28cbb4253d4fe2a948627587fe5908bc282") ||
            digest == QByteArrayLiteral("31f411f41c760b5fdb97ee4c21a361c54fe21ca5a4ca2da79e9bbbffab60fa39")) &&
           hasLocalStaticSystemPage(document);
}

/******************************************************************************
 * Function Name   : applyNativeLocalPageIoProfile
 * Description     : Compile the current-page reporting and page-switch rows in the original single-page dialog profile.
 * Input Parameters: parsed document, previous normalization, configuration limits, optional 4998-byte output segment
 * Return Value    : True for two fully validated local LW records; the optional segment changes only on success.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool applyNativeLocalPageIoProfile(const SourceDocument &document, SourceUpdates normalized,
                                         qsizetype first, qsizetype end, QByteArray *segment) //20261128-17 add
{
    const QString prefixes[] = {QStringLiteral("config.140."), QStringLiteral("config.18c.")};
    const QString flags[] = {QStringLiteral("config.13c.0"), QStringLiteral("config.188.0")};
    const int offsets[] = {352, 752};
    int enabled[2], stations[2], word[2], row, byte;
    u32 address[2];
    QString digits;
    bool ok;
    QByteArray digest;

    for (row = 0; row < 2; ++row) {
        enabled[row] = document.value(flags[row]).toInt();
        stations[row] = document.value(prefixes[row] + QStringLiteral("3")).toInt();
        digits = document.value(prefixes[row] + QStringLiteral("4")).toString();
        address[row] = digits.toUInt(&ok, 10);
        word[row] = row == 0 ? 1 : enabled[row];
        // Both original address discriminators change together. The HMI record keeps its selected word type when disabled.
        if ((enabled[row] != 0 && enabled[row] != 1) || !ok || address[row] > 65535 ||
            digits != QString::number(address[row]) ||
            document.value(prefixes[row] + QStringLiteral("0")).toString() != QStringLiteral("LW") + digits ||
            document.value(prefixes[row] + QStringLiteral("2")).toInt() != word[row] ||
            document.value(prefixes[row] + QStringLiteral("6")).toInt() != word[row] ||
            (stations[row] != 100 && (stations[row] != 0 || address[row] != 0)) ||
            (word[row] == 0 && (address[row] != 0 || stations[row] != 0))) {
            return false;
        }
        normalized.set(flags[row], 0);
        normalized.set(prefixes[row] + QStringLiteral("0"), QStringLiteral("LW0"));
        normalized.set(prefixes[row] + QStringLiteral("2"), row == 0 ? 1 : 0);
        normalized.set(prefixes[row] + QStringLiteral("3"), 0);
        normalized.set(prefixes[row] + QStringLiteral("4"), QStringLiteral("0"));
        normalized.set(prefixes[row] + QStringLiteral("6"), row == 0 ? 1 : 0);
    }
    // All other fields, including device/driver, register family and the remaining I/O rows, must still match.
    digest = QCryptographicHash::hash(normalized.render(first, end), QCryptographicHash::Sha256).toHex();
    if (!normalized.error.isEmpty() ||
        digest != QByteArrayLiteral("457a4f7ddf257d18588d566a678a72904f46cdffa1d6489e3de3be9b84cb85d1") ||
        !hasLocalStaticSystemPage(document)) {
        return false;
    }
    if (segment) {
        for (row = 0; row < 2; ++row) {
            (*segment)[offsets[row]] = char(enabled[row]);
            (*segment)[offsets[row] + 4] = 0;  // Local register space in the firmware address descriptor.
            (*segment)[offsets[row] + 5] = word[row] ? 'W' : '_';
            (*segment)[offsets[row] + 6] = char(stations[row]);
            for (byte = 0; byte < 4; ++byte) {
                (*segment)[offsets[row] + 12 + byte] = char((address[row] >> (byte * 8)) & 0xff);
            }
        }
    }
    return true;
}
