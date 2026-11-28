/******************************************************************************
 * Function Name   : supportsNativeSystemDialogProfile
 * Description     : Admit the two complete unassigned-port configurations emitted by the native system dialog.
 * Input Parameters: parsed native document, normalized configuration digest
 * Return Value    : True for static local text with the captured inactive configuration.
 * Date            : 2026.11.28
 ******************************************************************************/
static bool supportsNativeSystemDialogProfile(const SourceDocument &document, const QByteArray &digest) //20261128-10 add
{
    if ((digest != QByteArrayLiteral("2e4607316f881af776e1bc9d5992c28cbb4253d4fe2a948627587fe5908bc282") &&
         digest != QByteArrayLiteral("31f411f41c760b5fdb97ee4c21a361c54fe21ca5a4ca2da79e9bbbffab60fa39")) ||
        document.value(QStringLiteral("system.174.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.178.count")).toInt() != 0 ||
        document.value(QStringLiteral("system.194.count")).toInt() != 0) {
        return false;
    }
    for (const QString &prefix : document.objectPrefixes) {
        // Neither snapshot contains a PLC consumer: retain the independent type and visibility gates.
        if (document.value(prefix + QStringLiteral("type")).toInt() != 28 ||
            document.value(prefix + QStringLiteral("common.visibility.0")).toInt() != 0 ||
            document.value(prefix + QStringLiteral("common.touch.0")).toInt() != 0) {
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
