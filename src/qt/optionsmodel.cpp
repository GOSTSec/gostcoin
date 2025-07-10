#include "optionsmodel.h"

#include "bitcoinunits.h"
#include "init.h"
#include "walletdb.h"
#include "guiutil.h"

#include <QSettings>

#include "i2p.h"
#include <sstream>

#define I2P_OPTIONS_SECTION_NAME    "I2P"

class ScopeGroupHelper
{
public:
    ScopeGroupHelper(QSettings& settings, const QString& groupName)
        : settings_(settings)
    {
        settings_.beginGroup(groupName);
    }
    ~ScopeGroupHelper()
    {
        settings_.endGroup();
    }

private:
    QSettings& settings_;
};

OptionsModel::OptionsModel(QObject *parent) :
    QAbstractListModel(parent)
{
    Init();
}

bool static ApplyProxySettings()
{
    QSettings settings;
    CService addrProxy(settings.value("addrProxy", "127.0.0.1:9050").toString().toStdString());
    int nSocksVersion(settings.value("nSocksVersion", 5).toInt());
    if (!settings.value("fUseProxy", false).toBool()) {
        addrProxy = CService();
        nSocksVersion = 0;
        return false;
    }
    if (nSocksVersion && !addrProxy.IsValid())
        return false;
    if (!IsLimited(NET_IPV4))
        SetProxy(NET_IPV4, addrProxy, nSocksVersion);
    if (nSocksVersion > 4) {
#ifdef USE_IPV6
        if (!IsLimited(NET_IPV6))
            SetProxy(NET_IPV6, addrProxy, nSocksVersion);
#endif
        SetNameProxy(addrProxy, nSocksVersion);
    }
    return true;
}

std::string& FormatI2POptionsString(
        std::string& options,
        const std::string& name,
        const std::pair<bool, std::string>& value)
{
    if (value.first)
    {
        if (!options.empty())
            options += " ";
        options += name + "=" + value.second;
    }
    return options;
}

std::string& FormatI2POptionsString(
        std::string& options,
        const std::string& name,
        const std::pair<bool, bool>& value)
{
    if (value.first)
    {
        if (!options.empty())
            options += " ";
        options += name + "=" + (value.second ? "true" : "false");
    }
    return options;
}

std::string& FormatI2POptionsString(
        std::string& options,
        const std::string& name,
        const std::pair<bool, int>& value)
{
    if (value.first)
    {
        if (!options.empty())
            options += " ";
        std::ostringstream oss;
        oss << value.second;
        options += name + "=" + oss.str();
    }
    return options;
}

void OptionsModel::Init()
{
    QSettings settings;

    // These are Qt-only settings:
    nDisplayUnit = settings.value("nDisplayUnit", BitcoinUnits::BTC).toInt();
    bDisplayAddresses = settings.value("bDisplayAddresses", false).toBool();
    fMinimizeToTray = settings.value("fMinimizeToTray", false).toBool();
    fSilentMode = settings.value("fSilentMode", false).toBool();
    fMinimizeOnClose = settings.value("fMinimizeOnClose", false).toBool();
    nTransactionFee = settings.value("nTransactionFee").toLongLong();
    language = settings.value("language", "").toString();
    fCoinControlFeatures = settings.value("fCoinControlFeatures", false).toBool();

    // These are shared with core Bitcoin; we want
    // command-line options to override the GUI settings:
    if (settings.contains("addrProxy") && settings.value("fUseProxy").toBool())
        SoftSetArg("-proxy", settings.value("addrProxy").toString().toStdString());
    if (settings.contains("nSocksVersion") && settings.value("fUseProxy").toBool())
        SoftSetArg("-socks", settings.value("nSocksVersion").toString().toStdString());
    if (!language.isEmpty())
        SoftSetArg("-lang", language.toStdString());

    ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);

    if (settings.value("useI2POnly", false).toBool())
    {
        mapArgs["-onlynet"] = NATIVE_I2P_NET_STRING;
        std::vector<std::string>& onlyNets = mapMultiArgs["-onlynet"];
        if (std::find(onlyNets.begin(), onlyNets.end(), NATIVE_I2P_NET_STRING) == onlyNets.end())
            onlyNets.push_back(NATIVE_I2P_NET_STRING);
    }

    if (settings.contains("samhost"))
        SoftSetArg(I2P_SAM_HOST_PARAM, settings.value("samhost").toString().toStdString());

    if (settings.contains("samport"))
        SoftSetArg(I2P_SAM_PORT_PARAM, settings.value("samport").toString().toStdString());

    if (settings.contains("sessionName"))
        SoftSetArg(I2P_SESSION_NAME_PARAM, settings.value("sessionName").toString().toStdString());

    i2pInboundQuantity        = settings.value(SAM_NAME_INBOUND_QUANTITY       , SAM_DEFAULT_INBOUND_QUANTITY       ).toInt();
    i2pInboundLength          = settings.value(SAM_NAME_INBOUND_LENGTH         , SAM_DEFAULT_INBOUND_LENGTH         ).toInt();
    i2pInboundLengthVariance  = settings.value(SAM_NAME_INBOUND_LENGTHVARIANCE , SAM_DEFAULT_INBOUND_LENGTHVARIANCE ).toInt();
    i2pInboundBackupQuantity  = settings.value(SAM_NAME_INBOUND_BACKUPQUANTITY , SAM_DEFAULT_INBOUND_BACKUPQUANTITY ).toInt();
    i2pInboundAllowZeroHop    = settings.value(SAM_NAME_INBOUND_ALLOWZEROHOP   , SAM_DEFAULT_INBOUND_ALLOWZEROHOP   ).toBool();
    i2pInboundIPRestriction   = settings.value(SAM_NAME_INBOUND_IPRESTRICTION  , SAM_DEFAULT_INBOUND_IPRESTRICTION  ).toInt();
    i2pOutboundQuantity       = settings.value(SAM_NAME_OUTBOUND_QUANTITY      , SAM_DEFAULT_OUTBOUND_QUANTITY      ).toInt();
    i2pOutboundLength         = settings.value(SAM_NAME_OUTBOUND_LENGTH        , SAM_DEFAULT_OUTBOUND_LENGTH        ).toInt();
    i2pOutboundLengthVariance = settings.value(SAM_NAME_OUTBOUND_LENGTHVARIANCE, SAM_DEFAULT_OUTBOUND_LENGTHVARIANCE).toInt();
    i2pOutboundBackupQuantity = settings.value(SAM_NAME_OUTBOUND_BACKUPQUANTITY, SAM_DEFAULT_OUTBOUND_BACKUPQUANTITY).toInt();
    i2pOutboundAllowZeroHop   = settings.value(SAM_NAME_OUTBOUND_ALLOWZEROHOP  , SAM_DEFAULT_OUTBOUND_ALLOWZEROHOP  ).toBool();
    i2pOutboundIPRestriction  = settings.value(SAM_NAME_OUTBOUND_IPRESTRICTION , SAM_DEFAULT_OUTBOUND_IPRESTRICTION ).toInt();
    i2pOutboundPriority       = settings.value(SAM_NAME_OUTBOUND_PRIORITY      , SAM_DEFAULT_OUTBOUND_PRIORITY      ).toInt();

    std::string i2pOptionsTemp;
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_INBOUND_QUANTITY       , std::make_pair(settings.contains(SAM_NAME_INBOUND_QUANTITY       ), i2pInboundQuantity));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_INBOUND_LENGTH         , std::make_pair(settings.contains(SAM_NAME_INBOUND_LENGTH         ), i2pInboundLength));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_INBOUND_LENGTHVARIANCE , std::make_pair(settings.contains(SAM_NAME_INBOUND_LENGTHVARIANCE ), i2pInboundLengthVariance));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_INBOUND_BACKUPQUANTITY , std::make_pair(settings.contains(SAM_NAME_INBOUND_BACKUPQUANTITY ), i2pInboundBackupQuantity));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_INBOUND_ALLOWZEROHOP   , std::make_pair(settings.contains(SAM_NAME_INBOUND_ALLOWZEROHOP   ), i2pInboundAllowZeroHop));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_INBOUND_IPRESTRICTION  , std::make_pair(settings.contains(SAM_NAME_INBOUND_IPRESTRICTION  ), i2pInboundIPRestriction));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_QUANTITY      , std::make_pair(settings.contains(SAM_NAME_OUTBOUND_QUANTITY      ), i2pOutboundQuantity));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_LENGTH        , std::make_pair(settings.contains(SAM_NAME_OUTBOUND_LENGTH        ), i2pOutboundLength));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_LENGTHVARIANCE, std::make_pair(settings.contains(SAM_NAME_OUTBOUND_LENGTHVARIANCE), i2pOutboundLengthVariance));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_BACKUPQUANTITY, std::make_pair(settings.contains(SAM_NAME_OUTBOUND_BACKUPQUANTITY), i2pOutboundBackupQuantity));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_ALLOWZEROHOP  , std::make_pair(settings.contains(SAM_NAME_OUTBOUND_ALLOWZEROHOP  ), i2pOutboundAllowZeroHop));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_IPRESTRICTION , std::make_pair(settings.contains(SAM_NAME_OUTBOUND_IPRESTRICTION ), i2pOutboundIPRestriction));
    FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_OUTBOUND_PRIORITY      , std::make_pair(settings.contains(SAM_NAME_OUTBOUND_PRIORITY      ), i2pOutboundPriority));
	FormatI2POptionsString(i2pOptionsTemp, SAM_NAME_I2CP_LEASESET_ENC_TYPE, std::make_pair(true, std::string (SAM_DEFAULT_I2CP_LEASESET_ENC_TYPE))); // i2cp.leaseSetEncType=6,4,0 TODO: make it configurable

    if (!i2pOptionsTemp.empty())
        SoftSetArg(I2P_SAM_I2P_OPTIONS_PARAM, i2pOptionsTemp);

    i2pOptions = QString::fromStdString(i2pOptionsTemp);
}

void OptionsModel::Reset()
{
    QSettings settings;

    // Remove all entries in this QSettings object
    settings.clear();

    // default setting for OptionsModel::StartAtStartup - disabled
    if (GUIUtil::GetStartOnSystemStartup())
        GUIUtil::SetStartOnSystemStartup(false);

    // Re-Init to get default values
    Init();

    // Ensure Upgrade() is not running again by setting the bImportFinished flag
    settings.setValue("bImportFinished", true);
}

bool OptionsModel::Upgrade()
{
    QSettings settings;

    if (settings.contains("bImportFinished"))
        return false; // Already upgraded

    settings.setValue("bImportFinished", true);

    // Move settings from old wallet.dat (if any):
    CWalletDB walletdb("wallet.dat");

    QList<QString> intOptions;
    intOptions << "nDisplayUnit" << "nTransactionFee";
    foreach(QString key, intOptions)
    {
        int value = 0;
        if (walletdb.ReadSetting(key.toStdString(), value))
        {
            settings.setValue(key, value);
            walletdb.EraseSetting(key.toStdString());
        }
    }
    QList<QString> boolOptions;
    boolOptions << "bDisplayAddresses" << "fMinimizeToTray" << "fSilentMode" << "fMinimizeOnClose" << "fUseProxy";
    foreach(QString key, boolOptions)
    {
        bool value = false;
        if (walletdb.ReadSetting(key.toStdString(), value))
        {
            settings.setValue(key, value);
            walletdb.EraseSetting(key.toStdString());
        }
    }
    try
    {
        CAddress addrProxyAddress;
        if (walletdb.ReadSetting("addrProxy", addrProxyAddress))
        {
            settings.setValue("addrProxy", addrProxyAddress.ToStringIPPort().c_str());
            walletdb.EraseSetting("addrProxy");
        }
    }
    catch (std::ios_base::failure &e)
    {
        // 0.6.0rc1 saved this as a CService, which causes failure when parsing as a CAddress
        CService addrProxy;
        if (walletdb.ReadSetting("addrProxy", addrProxy))
        {
            settings.setValue("addrProxy", addrProxy.ToStringIPPort().c_str());
            walletdb.EraseSetting("addrProxy");
        }
    }
    ApplyProxySettings();
    Init();

    return true;
}


int OptionsModel::rowCount(const QModelIndex & parent) const
{
    return OptionIDRowCount;
}

QVariant OptionsModel::data(const QModelIndex & index, int role) const
{
    if(role == Qt::EditRole)
    {
        QSettings settings;
        switch(index.row())
        {
        case StartAtStartup:
            return QVariant(GUIUtil::GetStartOnSystemStartup());
        case MinimizeToTray:
            return QVariant(fMinimizeToTray);
        case SilentMode:
            return QVariant(fSilentMode);
        case MinimizeOnClose:
            return QVariant(fMinimizeOnClose);
        case ProxyUse: {
            proxyType proxy;
            return QVariant(GetProxy(NET_IPV4, proxy));
        }
        case ProxyIP: {
            proxyType proxy;
            if (GetProxy(NET_IPV4, proxy))
                return QVariant(QString::fromStdString(proxy.first.ToStringIP()));
            else
                return QVariant(QString::fromStdString("127.0.0.1"));
        }
        case ProxyPort: {
            proxyType proxy;
            if (GetProxy(NET_IPV4, proxy))
                return QVariant(proxy.first.GetPort());
            else
                return QVariant(9050);
        }
        case ProxySocksVersion: {
            proxyType proxy;
            if (GetProxy(NET_IPV4, proxy))
                return QVariant(proxy.second);
            else
                return QVariant(5);
        }
        case Fee:
            return QVariant(nTransactionFee);
        case DisplayUnit:
            return QVariant(nDisplayUnit);
        case DisplayAddresses:
            return QVariant(bDisplayAddresses);
        case Language:
            return settings.value("language", "");
        case I2PUseI2POnly:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            bool useI2POnly = false;
            if (mapArgs.count("-onlynet"))
            {
                const std::vector<std::string>& onlyNets = mapMultiArgs["-onlynet"];
                if (std::find(onlyNets.begin(), onlyNets.end(), NATIVE_I2P_NET_STRING) != onlyNets.end())
                    useI2POnly = true;
            }
            return settings.value("useI2POnly", useI2POnly);
        }
        case I2PSAMHost:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            return settings.value("samhost", QString::fromStdString(GetArg(I2P_SAM_HOST_PARAM, I2P_SAM_HOST_DEFAULT)));
        }
        case I2PSAMPort:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            return settings.value("samport", GetArg(I2P_SAM_PORT_PARAM, I2P_SAM_PORT_DEFAULT));
        }
        case I2PSessionName:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            return settings.value("sessionName", QString::fromStdString(GetArg(I2P_SESSION_NAME_PARAM, I2P_SESSION_NAME_DEFAULT)));
        }
        case I2PInboundQuantity:
            return QVariant(i2pInboundQuantity);
        case I2PInboundLength:
            return QVariant(i2pInboundLength);
        case I2PInboundLengthVariance:
            return QVariant(i2pInboundLengthVariance);
        case I2PInboundBackupQuantity:
            return QVariant(i2pInboundBackupQuantity);
        case I2PInboundAllowZeroHop:
            return QVariant(i2pInboundAllowZeroHop);
        case I2PInboundIPRestriction:
            return QVariant(i2pInboundIPRestriction);
        case I2POutboundQuantity:
            return QVariant(i2pOutboundQuantity);
        case I2POutboundLength:
            return QVariant(i2pOutboundLength);
        case I2POutboundLengthVariance:
            return QVariant(i2pOutboundLengthVariance);
        case I2POutboundBackupQuantity:
            return QVariant(i2pOutboundBackupQuantity);
        case I2POutboundAllowZeroHop:
            return QVariant(i2pOutboundAllowZeroHop);
        case I2POutboundIPRestriction:
            return QVariant(i2pOutboundIPRestriction);
        case I2POutboundPriority:
            return QVariant(i2pOutboundPriority);
        case CoinControlFeatures:
            return QVariant(fCoinControlFeatures);
        default:
            return QVariant();
        }
    }
    return QVariant();
}

bool OptionsModel::setData(const QModelIndex & index, const QVariant & value, int role)
{
    bool successful = true; /* set to false on parse error */
    if(role == Qt::EditRole)
    {
        QSettings settings;
        switch(index.row())
        {
        case StartAtStartup:
            successful = GUIUtil::SetStartOnSystemStartup(value.toBool());
            break;
        case MinimizeToTray:
            fMinimizeToTray = value.toBool();
            settings.setValue("fMinimizeToTray", fMinimizeToTray);
            break;
        case SilentMode:
            fSilentMode = value.toBool();
            settings.setValue("fSilentMode", fSilentMode);
            break;
        case MinimizeOnClose:
            fMinimizeOnClose = value.toBool();
            settings.setValue("fMinimizeOnClose", fMinimizeOnClose);
            break;
        case ProxyUse:
            settings.setValue("fUseProxy", value.toBool());
            successful = ApplyProxySettings();
            break;
        case ProxyIP: {
            proxyType proxy;
            proxy.first = CService("127.0.0.1", 9050);
            GetProxy(NET_IPV4, proxy);

            CNetAddr addr(value.toString().toStdString());
            proxy.first.SetIP(addr);
            settings.setValue("addrProxy", proxy.first.ToStringIPPort().c_str());
            successful = ApplyProxySettings();
        }
        break;
        case ProxyPort: {
            proxyType proxy;
            proxy.first = CService("127.0.0.1", 9050);
            GetProxy(NET_IPV4, proxy);

            proxy.first.SetPort(value.toInt());
            settings.setValue("addrProxy", proxy.first.ToStringIPPort().c_str());
            successful = ApplyProxySettings();
        }
        break;
        case ProxySocksVersion: {
            proxyType proxy;
            proxy.second = 5;
            GetProxy(NET_IPV4, proxy);

            proxy.second = value.toInt();
            settings.setValue("nSocksVersion", proxy.second);
            successful = ApplyProxySettings();
        }
        break;
        case Fee:
            nTransactionFee = value.toLongLong();
            settings.setValue("nTransactionFee", nTransactionFee);
            emit transactionFeeChanged(nTransactionFee);
            break;
        case DisplayUnit:
            nDisplayUnit = value.toInt();
            settings.setValue("nDisplayUnit", nDisplayUnit);
            emit displayUnitChanged(nDisplayUnit);
            break;
        case DisplayAddresses:
            bDisplayAddresses = value.toBool();
            settings.setValue("bDisplayAddresses", bDisplayAddresses);
            break;
        case Language:
            settings.setValue("language", value);
            break;
        case I2PUseI2POnly:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            settings.setValue("useI2POnly", value.toBool());
            break;
        }
        case I2PSAMHost:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            settings.setValue("samhost", value.toString());
            break;
        }
        case I2PSAMPort:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            settings.setValue("samport", value.toString());
            break;
        }
        case I2PSessionName:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            settings.setValue("sessionName", value.toString());
            break;
        }
        case I2PInboundQuantity:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pInboundQuantity = value.toInt();
            settings.setValue(SAM_NAME_INBOUND_QUANTITY, i2pInboundQuantity);
            break;
        }
        case I2PInboundLength:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pInboundLength = value.toInt();
            settings.setValue(SAM_NAME_INBOUND_LENGTH, i2pInboundLength);
            break;
        }
        case I2PInboundLengthVariance:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pInboundLengthVariance = value.toInt();
            settings.setValue(SAM_NAME_INBOUND_LENGTHVARIANCE, i2pInboundLengthVariance);
            break;
        }
        case I2PInboundBackupQuantity:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pInboundBackupQuantity = value.toInt();
            settings.setValue(SAM_NAME_INBOUND_BACKUPQUANTITY, i2pInboundBackupQuantity);
            break;
        }
        case I2PInboundAllowZeroHop:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pInboundAllowZeroHop = value.toBool();
            settings.setValue(SAM_NAME_INBOUND_ALLOWZEROHOP, i2pInboundAllowZeroHop);
            break;
        }
        case I2PInboundIPRestriction:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pInboundIPRestriction = value.toInt();
            settings.setValue(SAM_NAME_INBOUND_IPRESTRICTION, i2pInboundIPRestriction);
            break;
        }
        case I2POutboundQuantity:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundQuantity = value.toInt();
            settings.setValue(SAM_NAME_OUTBOUND_QUANTITY, i2pOutboundQuantity);
            break;
        }
        case I2POutboundLength:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundLength = value.toInt();
            settings.setValue(SAM_NAME_OUTBOUND_LENGTH, i2pOutboundLength);
            break;
        }
        case I2POutboundLengthVariance:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundLengthVariance = value.toInt();
            settings.setValue(SAM_NAME_OUTBOUND_LENGTHVARIANCE, i2pOutboundLengthVariance);
            break;
        }
        case I2POutboundBackupQuantity:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundBackupQuantity = value.toInt();
            settings.setValue(SAM_NAME_OUTBOUND_BACKUPQUANTITY, i2pOutboundBackupQuantity);
            break;
        }
        case I2POutboundAllowZeroHop:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundAllowZeroHop = value.toBool();
            settings.setValue(SAM_NAME_OUTBOUND_ALLOWZEROHOP, i2pOutboundAllowZeroHop);
            break;
        }
        case I2POutboundIPRestriction:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundIPRestriction = value.toInt();
            settings.setValue(SAM_NAME_OUTBOUND_IPRESTRICTION, i2pOutboundIPRestriction);
            break;
        }
        case I2POutboundPriority:
        {
            ScopeGroupHelper s(settings, I2P_OPTIONS_SECTION_NAME);
            i2pOutboundPriority = value.toInt();
            settings.setValue(SAM_NAME_OUTBOUND_PRIORITY, i2pOutboundPriority);
            break;
        }
        case CoinControlFeatures: {
            fCoinControlFeatures = value.toBool();
            settings.setValue("fCoinControlFeatures", fCoinControlFeatures);
            emit coinControlFeaturesChanged(fCoinControlFeatures);
        }
        break;
        default:
            break;
        }
    }
    emit dataChanged(index, index);

    return successful;
}

qint64 OptionsModel::getTransactionFee()
{
    return nTransactionFee;
}

bool OptionsModel::getCoinControlFeatures()
{
    return fCoinControlFeatures;
}

QString OptionsModel::getPublicI2PKey() const
{
    return QString::fromStdString(I2PSession::Instance().getMyDestination().pub);
}

QString OptionsModel::getPrivateI2PKey() const
{
    return QString::fromStdString(I2PSession::Instance().getMyDestination().priv);
}

QString OptionsModel::getB32Address(const QString& destination) const
{
    return QString::fromStdString(I2PSession::GenerateB32AddressFromDestination(destination.toStdString()));
}

void OptionsModel::generateI2PDestination(QString& pub, QString& priv) const
{
    const SAM::FullDestination generatedDest = I2PSession::Instance().destGenerate();
    pub = QString::fromStdString(generatedDest.pub);
    priv = QString::fromStdString(generatedDest.priv);
}
