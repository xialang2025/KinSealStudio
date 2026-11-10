#ifndef PLC_NATIVE_CATALOG_H
#define PLC_NATIVE_CATALOG_H

#include "plc_address.h"

#include <array>

//20261109-0 add: Static rules recovered from the isolated Common.dll V5.1 codec.
enum class PlcNativeGrammar
{
    Integer,
    Bit,
    BankInteger,
    BankBit,
    PackedDecimalBit,
    DecimalHexNibble,
    LgLength
};

struct PlcNativeFamilyRule
{
    QString profileId;
    QString family;
    QString evidence;
    PlcNativeGrammar grammar = PlcNativeGrammar::Integer;
    u8 typeByte = 0;
    u16 ordinal = 0;
    int componentCount = 0;
    int majorRadix = 10;
    int minorRadix = 10;
    int minorWidth = 1;
    std::array<u32, 3> minimum = {};
    std::array<u32, 3> maximum = {};
    std::array<qint64, 4> numericTerms = {};    // Constant, then each parsed component coefficient.
    std::array<qint64, 4> countTerms = {};      // Inclusive N maximum in the same coordinates.
    bool exactCountBoundary = false;          // False identifies a verified strict arithmetic subset.
    bool hasExcludedNumericRange = false;
    u32 excludedNumericMinimum = 0;
    u32 excludedNumericMaximum = 0;
};

struct PlcNativeParsedAddress
{
    QString canonicalValue;
    u8 typeByte = 0;
    u16 ordinal = 0;
    u32 numeric = 0;
    u32 maximumCount = 0;
    bool exactCountBoundary = false;
};

/******************************************************************************
 * Function Name   : plcNativeFamilyRule
 * Description     : Query an immutable rule bound to an exact observed profile and source hash.
 * Input Parameters: profile, family, rule, error
 * Return Value    : True with a copied rule; outputs are unchanged on failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool plcNativeFamilyRule(const PlcProfile &profile, const QString &family,
                         PlcNativeFamilyRule *rule, QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : parseNativePlcAddress
 * Description     : Parse strict canonical syntax and validate the explicit native transfer count.
 * Input Parameters: profile, family, value, transferCount, address, error
 * Return Value    : True with native type, ordinal and numeric address; no DLL is loaded.
 * Date            : 2026.11.09
 ******************************************************************************/
bool parseNativePlcAddress(const PlcProfile &profile, const QString &family,
                           const QString &value, u32 transferCount,
                           PlcNativeParsedAddress *address,
                           QString *error = nullptr); //20261109-0 add

/******************************************************************************
 * Function Name   : plcNativeInitialAddressValue
 * Description     : Format the verified component minima and require a valid native N-one address.
 * Input Parameters: profile, family, value, error
 * Return Value    : True with a valid initial value; outputs are unchanged on failure.
 * Date            : 2026.11.09
 ******************************************************************************/
bool plcNativeInitialAddressValue(const PlcProfile &profile, const QString &family,
                                  QString *value, QString *error = nullptr); //20261109-0 add

#endif
