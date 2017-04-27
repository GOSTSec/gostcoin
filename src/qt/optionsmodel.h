#ifndef OPTIONSMODEL_H
#define OPTIONSMODEL_H

#include <QAbstractListModel>

/** Interface from Qt to configuration data structure for Bitcoin client.
   To Qt, the options are presented as a list with the different options
   laid out vertically.
   This can be changed to a tree once the settings become sufficiently
   complex.
 */
class OptionsModel : public QAbstractListModel
{
    Q_OBJECT

public:
    explicit OptionsModel(QObject *parent = 0);

    enum OptionID {
        StartAtStartup,    // bool
        MinimizeToTray,    // bool
        MinimizeOnClose,   // bool
        ProxyUse,          // bool
        ProxyIP,           // QString
        ProxyPort,         // int
        ProxySocksVersion, // int
        Fee,               // qint64
        DisplayUnit,       // BitcoinUnits::Unit
        DisplayAddresses,  // bool
        Language,          // QString
        CoinControlFeatures, // bool
        I2PUseI2POnly,              // bool
        I2PSAMHost,                 // QString
        I2PSAMPort,                 // int
        I2PSessionName,             // QString

        I2PInboundQuantity,         // int
        I2PInboundLength,           // int
        I2PInboundLengthVariance,   // int
        I2PInboundBackupQuantity,   // int
        I2PInboundAllowZeroHop,     // bool
        I2PInboundIPRestriction,    // int

        I2POutboundQuantity,        // int
        I2POutboundLength,          // int
        I2POutboundLengthVariance,  // int
        I2POutboundBackupQuantity,  // int
        I2POutboundAllowZeroHop,    // bool
        I2POutboundIPRestriction,   // int
        I2POutboundPriority,        // int
        SilentMode,    // bool
        OptionIDRowCount,
    };

    void Init();
    void Reset();

    /* Migrate settings from wallet.dat after app initialization */
    bool Upgrade(); /* returns true if settings upgraded */

    int rowCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex & index, int role = Qt::DisplayRole) const;
    bool setData(const QModelIndex & index, const QVariant & value, int role = Qt::EditRole);

    /* Explicit getters */
    qint64 getTransactionFee();
    bool getMinimizeToTray() { return fMinimizeToTray; }
    bool isSilentMode() { return fSilentMode; }
    bool getMinimizeOnClose() { return fMinimizeOnClose; }
    int getDisplayUnit() { return nDisplayUnit; }
    bool getDisplayAddresses() { return bDisplayAddresses; }
    QString getLanguage() { return language; }
    bool getCoinControlFeatures();

private:
    int nDisplayUnit;
    bool bDisplayAddresses;
    bool fMinimizeToTray;
    bool fSilentMode;
    bool fMinimizeOnClose;
    QString language;
    bool fCoinControlFeatures;
    int i2pInboundQuantity;
    int i2pInboundLength;
    int i2pInboundLengthVariance;
    int i2pInboundBackupQuantity;
    bool i2pInboundAllowZeroHop;
    int i2pInboundIPRestriction;
    int i2pOutboundQuantity;
    int i2pOutboundLength;
    int i2pOutboundLengthVariance;
    int i2pOutboundBackupQuantity;
    bool i2pOutboundAllowZeroHop;
    int i2pOutboundIPRestriction;
    int i2pOutboundPriority;
    QString i2pOptions;

signals:
    void displayUnitChanged(int unit);
    void transactionFeeChanged(qint64);
    void coinControlFeaturesChanged(bool);
};

#endif // OPTIONSMODEL_H
