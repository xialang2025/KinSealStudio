#include <QCoreApplication>
#include <QDir>
#include <QFile>
#include <QFileInfo>
#include <QThread>
#include <cstdio>

//20261114-7 add: This is a synthetic process/ELF fixture, never an ARM compiler or ADS oracle.
/******************************************************************************
 * Function Name   : fixtureWord
 * Description     : Write one little-endian synthetic ELF field.
 * Input Parameters: bytes, offset, value, count
 * Return Value    : None.
 * Date            : 2026.11.14
 ******************************************************************************/
static void fixtureWord(QByteArray *bytes, int offset, unsigned int value, int count)
{
    int index;

    for (index = 0; index < count; ++index) { (*bytes)[offset + index] = char((value >> (index * 8)) & 255U); }
}

/******************************************************************************
 * Function Name   : fixtureArgument
 * Description     : Find the output or address following one exact SDK-shaped argument.
 * Input Parameters: arguments, name
 * Return Value    : Following value, or empty text.
 * Date            : 2026.11.14
 ******************************************************************************/
static QString fixtureArgument(const QStringList &arguments, const QString &name)
{
    int index;

    index = int(arguments.indexOf(name));
    return index >= 0 && index + 1 < arguments.size() ? arguments.at(index + 1) : QString();
}

/******************************************************************************
 * Function Name   : main
 * Description     : Model bounded child output, delays, failures and a minimal declared synthetic ELF.
 * Input Parameters: argc, argv
 * Return Value    : Process exit status.
 * Date            : 2026.11.14
 ******************************************************************************/
int main(int argc, char **argv)
{
    QCoreApplication application(argc, argv);
    QStringList arguments = application.arguments();
    QString role = QFileInfo(application.applicationFilePath()).baseName().toLower();
    QString mode = qEnvironmentVariable("KINSEAL_MACRO_FIXTURE_MODE");
    QString selected = qEnvironmentVariable("KINSEAL_MACRO_FIXTURE_STAGE");
    QByteArray bytes;
    QByteArray chunk(64 * 1024, 'X');
    QFile file;
    QString output;
    unsigned int entry;
    int index;

    std::fputs("SYNTHETIC PROCESS FIXTURE; NOT ADS OR ARM ORACLE\n", stdout);
    std::fflush(stdout);
    if (selected.isEmpty() || role == selected)
    {
        if (mode == QStringLiteral("sleep"))
        { QThread::msleep(2000); }
        if (mode == QStringLiteral("flood"))
        {
            for (index = 0; index < 16; ++index) { std::fwrite(chunk.constData(), 1, size_t(chunk.size()), stdout); std::fflush(stdout); }
        }
        if (mode == QStringLiteral("fail")) { std::fputs("synthetic requested failure\n", stderr); return 7; }
    }
    if (role == QStringLiteral("armcc"))
    {
        output = fixtureArgument(arguments, QStringLiteral("-o"));
        bytes = QByteArray("synthetic-object");
    }
    else if (role == QStringLiteral("armlink"))
    {
        output = fixtureArgument(arguments, QStringLiteral("-o"));
        entry = fixtureArgument(arguments, QStringLiteral("-entry")).toUInt(nullptr, 0);
        bytes = QByteArray(217, char(0));
        bytes.replace(0, 7, QByteArray::fromHex("7f454c46010101"));
        fixtureWord(&bytes, 16, 2, 2);
        fixtureWord(&bytes, 18, 40, 2);
        fixtureWord(&bytes, 24, entry, 4);
        fixtureWord(&bytes, 32, 52, 4);
        fixtureWord(&bytes, 46, 40, 2);
        fixtureWord(&bytes, 48, 3, 2);
        fixtureWord(&bytes, 92 + 4, 2, 4);
        fixtureWord(&bytes, 92 + 16, 172, 4);
        fixtureWord(&bytes, 92 + 20, 32, 4);
        fixtureWord(&bytes, 92 + 24, 2, 4);
        fixtureWord(&bytes, 92 + 36, 16, 4);
        fixtureWord(&bytes, 132 + 4, 3, 4);
        fixtureWord(&bytes, 132 + 16, 204, 4);
        fixtureWord(&bytes, 132 + 20, 13, 4);
        fixtureWord(&bytes, 188, 1, 4);
        fixtureWord(&bytes, 192, entry, 4);
        fixtureWord(&bytes, 202, 1, 2);
        bytes.replace(204, 13, QByteArray("\0Macro_Entry\0", 13));
        if (mode == QStringLiteral("invalid-elf")) { bytes = QByteArray("synthetic-invalid-elf"); }
    }
    else if (role == QStringLiteral("fromelf"))
    {
        output = fixtureArgument(arguments, QStringLiteral("-output"));
        bytes = QByteArray("SYNTHETIC-NOT-ARM-ORACLE!");
        if (mode == QStringLiteral("raw-overflow")) { bytes = QByteArray(65536, 'X'); }
    }
    else { return 0; }
    if (output.isEmpty()) { return 8; }
    file.setFileName(QDir::current().filePath(output));
    if (!file.open(QIODevice::WriteOnly) || file.write(bytes) != bytes.size()) { return 9; }
    return 0;
}

