/******************************************************************************
 * Function Name   : hasLocalStaticSystemPage
 * Description     : Check the separately verified object and macro restrictions on system-dialog snapshots.
 * Input Parameters: parsed native document, the permitted static object type
 * Return Value    : True for a local static page without schedule or external-address dependencies.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool hasLocalStaticSystemPage(const SourceDocument &document, int type = 28) //20261128-17 change
{
    if (document.value(QStringLiteral("system.174.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.178.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.194.count")).toInt() != 0) {
        return false;
    }
    for (const QString &prefix : document.objectPrefixes) {
        // Neither snapshot contains a PLC consumer: retain the independent type and visibility gates.
        if (document.value(prefix + QStringLiteral("type")).toInt() != type ||
            document.value(prefix + QStringLiteral("common.visibility.0")).toInt() != 0 ||
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
 * Function Name   : supportsNativeSystemDialogProfile
 * Description     : Admit the complete known static-text and static-table configuration profiles.
 * Input Parameters: parsed native document, normalized configuration digest
 * Return Value    : True for a known digest and a page without external consumers.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool supportsNativeSystemDialogProfile(const SourceDocument &document, const QByteArray &digest) //20261128-17 change
{
    // Seven saved table configurations share this complete fingerprint; their inactive clock descriptors
    // and remembered page name differ from the original draw-only profile. The runtime gets only a page image.
    if (digest == QByteArrayLiteral("2c9d9ef3322d35d79d24112f89911d1522e81d213218292714a1617e39349e64")) {
        return document.objectPrefixes.size() == 1 && hasLocalStaticSystemPage(document, 36);
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
