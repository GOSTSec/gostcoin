#include "optionsdialog.h"
#include "ui_optionsdialog.h"

#include "bitcoinunits.h"
#include "monitoreddatamapper.h"
#include "netbase.h"
#include "optionsmodel.h"

#include "showi2paddresses.h"
#include "util.h"

#include "clientmodel.h"

#include <QDir>
#include <QIntValidator>
#include <QLocale>
#include <QMessageBox>

OptionsDialog::OptionsDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::OptionsDialog),
    model(0),
    mapper(0),
    fRestartWarningDisplayed_Proxy(false),
    fRestartWarningDisplayed_Lang(false),
    fProxyIpValid(true),
    fRestartWarningDisplayed_I2P(false)
{
    ui->setupUi(this);

    /* Network elements init */

    ui->proxyIp->setEnabled(false);
    ui->proxyPort->setEnabled(false);
    ui->proxyPort->setValidator(new QIntValidator(1, 65535, this));

    ui->socksVersion->setEnabled(false);
    ui->socksVersion->addItem("5", 5);
    ui->socksVersion->addItem("4", 4);
    ui->socksVersion->setCurrentIndex(0);

    connect(ui->connectSocks                 , SIGNAL(toggled(bool))       , ui->proxyIp, SLOT(setEnabled(bool)));
    connect(ui->connectSocks                 , SIGNAL(toggled(bool))       , ui->proxyPort, SLOT(setEnabled(bool)));
    connect(ui->connectSocks                 , SIGNAL(toggled(bool))       , ui->socksVersion, SLOT(setEnabled(bool)));
    connect(ui->connectSocks                 , SIGNAL(clicked(bool))       , this, SLOT(showRestartWarning_Proxy()));

    connect(ui->pushButtonCurrentI2PAddress  , SIGNAL(clicked())           , this, SLOT(ShowCurrentI2PAddress()));
    connect(ui->pushButtonGenerateI2PAddress , SIGNAL(clicked())           , this, SLOT(GenerateNewI2PAddress()));

    ui->proxyIp->installEventFilter(this);

    /* Window elements init */
#ifdef Q_OS_MAC
    /* remove Window tab on Mac */
    ui->tabWidget->removeTab(ui->tabWidget->indexOf(ui->tabWindow));
#endif

    /* Display elements init */
    QDir translations(":translations");
    ui->lang->addItem(QString("(") + tr("default") + QString(")"), QVariant(""));
    foreach(const QString &langStr, translations.entryList())
    {
        QLocale locale(langStr);

        /** check if the locale name consists of 2 parts (language_country) */
        if(langStr.contains("_"))
        {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language - native country (locale name)", e.g. "Deutsch - Deutschland (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" - ") + locale.nativeCountryName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language - country (locale name)", e.g. "German - Germany (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" - ") + QLocale::countryToString(locale.country()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
        else
        {
#if QT_VERSION >= 0x040800
            /** display language strings as "native language (locale name)", e.g. "Deutsch (de)" */
            ui->lang->addItem(locale.nativeLanguageName() + QString(" (") + langStr + QString(")"), QVariant(langStr));
#else
            /** display language strings as "language (locale name)", e.g. "German (de)" */
            ui->lang->addItem(QLocale::languageToString(locale.language()) + QString(" (") + langStr + QString(")"), QVariant(langStr));
#endif
        }
    }

    ui->unit->setModel(new BitcoinUnits(this));

    /* Widget-to-option mapper */
    mapper = new MonitoredDataMapper(this);
    mapper->setSubmitPolicy(QDataWidgetMapper::ManualSubmit);
    mapper->setOrientation(Qt::Vertical);

    /* enable apply button when data modified */
    connect(mapper, SIGNAL(viewModified()), this, SLOT(enableApplyButton()));
    /* disable apply button when new data loaded */
    connect(mapper, SIGNAL(currentIndexChanged(int)), this, SLOT(disableApplyButton()));
    /* setup/change UI elements when proxy IP is invalid/valid */
    connect(this, SIGNAL(proxyIpValid(QValidatedLineEdit *, bool)), this, SLOT(handleProxyIpValid(QValidatedLineEdit *, bool)));
}

OptionsDialog::~OptionsDialog()
{
    delete ui;
}

void OptionsDialog::setModel(OptionsModel *model)
{
    this->model = model;

    if(model)
    {
        connect(model, SIGNAL(displayUnitChanged(int)), this, SLOT(updateDisplayUnit()));

        mapper->setModel(model);
        setMapper();
        mapper->toFirst();
    }

    /* update the display unit, to not use the default ("BTC") */
    updateDisplayUnit();

    /* warn only when changed by user action (placed here so init via mapper doesn't trigger this) */
    connect(ui->lang, SIGNAL(valueChanged()), this, SLOT(showRestartWarning_Lang()));
    connect(ui->checkBoxAllowZeroHop         , SIGNAL(stateChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->checkBoxInboundAllowZeroHop  , SIGNAL(stateChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->checkBoxUseI2POnly           , SIGNAL(stateChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->lineEditSAMHost              , SIGNAL(textChanged(QString)), this, SLOT(showRestartWarning_I2P()));
    connect(ui->lineEditTunnelName           , SIGNAL(textChanged(QString)), this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxInboundBackupQuality  , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxInboundIPRestriction  , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxInboundLength         , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxInboundLengthVariance , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxInboundQuantity       , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxOutboundBackupQuantity, SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxOutboundIPRestriction , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxOutboundLength        , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxOutboundLengthVariance, SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxOutboundPriority      , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxOutboundQuantity      , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));
    connect(ui->spinBoxSAMPort               , SIGNAL(valueChanged(int))   , this, SLOT(showRestartWarning_I2P()));

    /* disable apply button after settings are loaded as there is nothing to save */
    disableApplyButton();
}

void OptionsDialog::setMapper()
{
    /* Main */
    mapper->addMapping(ui->transactionFee, OptionsModel::Fee);
    mapper->addMapping(ui->bitcoinAtStartup, OptionsModel::StartAtStartup);

    /* Network */
    mapper->addMapping(ui->connectSocks, OptionsModel::ProxyUse);
    mapper->addMapping(ui->proxyIp, OptionsModel::ProxyIP);
    mapper->addMapping(ui->proxyPort, OptionsModel::ProxyPort);
    mapper->addMapping(ui->socksVersion, OptionsModel::ProxySocksVersion);

    /* Window */
#ifndef Q_OS_MAC
    mapper->addMapping(ui->minimizeToTray, OptionsModel::MinimizeToTray);
    mapper->addMapping(ui->minimizeOnClose, OptionsModel::MinimizeOnClose);
#endif
    mapper->addMapping(ui->silentMode, OptionsModel::SilentMode);

    /* Display */
    mapper->addMapping(ui->lang, OptionsModel::Language);
    mapper->addMapping(ui->unit, OptionsModel::DisplayUnit);
    mapper->addMapping(ui->displayAddresses, OptionsModel::DisplayAddresses);
    mapper->addMapping(ui->coinControlFeatures, OptionsModel::CoinControlFeatures);

    /* I2P */
    mapper->addMapping(ui->checkBoxUseI2POnly           , OptionsModel::I2PUseI2POnly);
    mapper->addMapping(ui->lineEditSAMHost              , OptionsModel::I2PSAMHost);
    mapper->addMapping(ui->spinBoxSAMPort               , OptionsModel::I2PSAMPort);
    mapper->addMapping(ui->lineEditTunnelName           , OptionsModel::I2PSessionName);
    mapper->addMapping(ui->spinBoxInboundQuantity       , OptionsModel::I2PInboundQuantity);
    mapper->addMapping(ui->spinBoxInboundLength         , OptionsModel::I2PInboundLength);
    mapper->addMapping(ui->spinBoxInboundLengthVariance , OptionsModel::I2PInboundLengthVariance);
    mapper->addMapping(ui->spinBoxInboundBackupQuality  , OptionsModel::I2PInboundBackupQuantity);
    mapper->addMapping(ui->checkBoxInboundAllowZeroHop  , OptionsModel::I2PInboundAllowZeroHop);
    mapper->addMapping(ui->spinBoxInboundIPRestriction  , OptionsModel::I2PInboundIPRestriction);
    mapper->addMapping(ui->spinBoxOutboundQuantity      , OptionsModel::I2POutboundQuantity);
    mapper->addMapping(ui->spinBoxOutboundLength        , OptionsModel::I2POutboundLength);
    mapper->addMapping(ui->spinBoxOutboundLengthVariance, OptionsModel::I2POutboundLengthVariance);
    mapper->addMapping(ui->spinBoxOutboundBackupQuantity, OptionsModel::I2POutboundBackupQuantity);
    mapper->addMapping(ui->checkBoxAllowZeroHop         , OptionsModel::I2POutboundAllowZeroHop);
    mapper->addMapping(ui->spinBoxOutboundIPRestriction , OptionsModel::I2POutboundIPRestriction);
    mapper->addMapping(ui->spinBoxOutboundPriority      , OptionsModel::I2POutboundIPRestriction);
}

void OptionsDialog::enableApplyButton()
{
    ui->applyButton->setEnabled(true);
}

void OptionsDialog::disableApplyButton()
{
    ui->applyButton->setEnabled(false);
}

void OptionsDialog::enableSaveButtons()
{
    /* prevent enabling of the save buttons when data modified, if there is an invalid proxy address present */
    if(fProxyIpValid)
        setSaveButtonState(true);
}

void OptionsDialog::disableSaveButtons()
{
    setSaveButtonState(false);
}

void OptionsDialog::setSaveButtonState(bool fState)
{
    ui->applyButton->setEnabled(fState);
    ui->okButton->setEnabled(fState);
}

void OptionsDialog::on_resetButton_clicked()
{
    if(model)
    {
        // confirmation dialog
        QMessageBox::StandardButton btnRetVal = QMessageBox::question(this, tr("Confirm options reset"),
            tr("Some settings may require a client restart to take effect.") + "<br><br>" + tr("Do you want to proceed?"),
            QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel);

        if(btnRetVal == QMessageBox::Cancel)
            return;

        disableApplyButton();

        /* disable restart warning messages display */
        fRestartWarningDisplayed_Lang = fRestartWarningDisplayed_Proxy = true;
        fRestartWarningDisplayed_I2P = true;

        /* reset all options and save the default values (QSettings) */
        model->Reset();
        mapper->toFirst();
        mapper->submit();

        /* re-enable restart warning messages display */
        fRestartWarningDisplayed_Lang = fRestartWarningDisplayed_Proxy = false;
        fRestartWarningDisplayed_I2P = false;
    }
}

void OptionsDialog::on_okButton_clicked()
{
    mapper->submit();
    accept();
}

void OptionsDialog::on_cancelButton_clicked()
{
    reject();
}

void OptionsDialog::on_applyButton_clicked()
{
    mapper->submit();
    disableApplyButton();
}

void OptionsDialog::showRestartWarning_Proxy()
{
    if(!fRestartWarningDisplayed_Proxy)
    {
        QMessageBox::warning(this, tr("Warning"), tr("This setting will take effect after restarting Gostcoin."), QMessageBox::Ok);
        fRestartWarningDisplayed_Proxy = true;
    }
}

void OptionsDialog::showRestartWarning_I2P()
{
    if(!fRestartWarningDisplayed_I2P)
    {
        QMessageBox::warning(this, tr("Warning"), tr("This setting will take effect after restarting Gostcoin."), QMessageBox::Ok);
        fRestartWarningDisplayed_I2P = true;
    }
}

void OptionsDialog::showRestartWarning_Lang()
{
    if(!fRestartWarningDisplayed_Lang)
    {
        QMessageBox::warning(this, tr("Warning"), tr("This setting will take effect after restarting Gostcoin."), QMessageBox::Ok);
        fRestartWarningDisplayed_Lang = true;
    }
}

void OptionsDialog::ShowCurrentI2PAddress()
{
    if (this->model)
    {
        const QString pub = this->model->getPublicI2PKey();
        const QString priv = this->model->getPrivateI2PKey();
        const QString b32 = this->model->getB32Address(pub);
        const QString configFile = QString::fromStdString(GetConfigFile().string());

        ShowI2PAddresses i2pCurrDialog(tr("Your current I2P-address"), pub, priv, b32, configFile, this);
        i2pCurrDialog.exec();
    }
}

void OptionsDialog::GenerateNewI2PAddress()
{
    if (this->model)
    {
        QString pub, priv;
        this->model->generateI2PDestination(pub, priv);
        const QString b32 = this->model->getB32Address(pub);
        const QString configFile = QString::fromStdString(GetConfigFile().string());

        ShowI2PAddresses i2pCurrDialog(tr("Generated I2P address"), pub, priv, b32, configFile, this);
        i2pCurrDialog.exec();
    }
}

void OptionsDialog::updateDisplayUnit()
{
    if(model)
    {
        /* Update transactionFee with the current unit */
        ui->transactionFee->setDisplayUnit(model->getDisplayUnit());
    }
}

void OptionsDialog::handleProxyIpValid(QValidatedLineEdit *object, bool fState)
{
    // this is used in a check before re-enabling the save buttons
    fProxyIpValid = fState;

    if(fProxyIpValid)
    {
        enableSaveButtons();
        ui->statusLabel->clear();
    }
    else
    {
        disableSaveButtons();
        object->setValid(fProxyIpValid);
        ui->statusLabel->setStyleSheet("QLabel { color: red; }");
        ui->statusLabel->setText(tr("The supplied proxy address is invalid."));
    }
}

bool OptionsDialog::eventFilter(QObject *object, QEvent *event)
{
    if(event->type() == QEvent::FocusOut)
    {
        if(object == ui->proxyIp)
        {
            CService addr;
            /* Check proxyIp for a valid IPv4/IPv6 address and emit the proxyIpValid signal */
            emit proxyIpValid(ui->proxyIp, LookupNumeric(ui->proxyIp->text().toStdString().c_str(), addr));
        }
    }
    return QDialog::eventFilter(object, event);
}
