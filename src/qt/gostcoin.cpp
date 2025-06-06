/*
 * W.J. van der Laan 2011-2012
 */
// Copyright (c) 2013 The Anoncoin Developers
// Copyright (c) 2017-2018 The Gostcoin Developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.
//
// I2P-patch
// Copyright (c) 2012-2013 giv

#include <QApplication>
#include <QMessageBox>
#include <QFile>
#include <QDir>
#include <QTextStream>
#include <QDebug>

#include "bitcoingui.h"
#include "clientmodel.h"
#include "walletmodel.h"
#include "optionsmodel.h"
#include "guiutil.h"
#include "guiconstants.h"
#include "init.h"
#include "util.h"
#include "ui_interface.h"
#include "paymentserver.h"
#include "splashscreen.h"
#include "setupdarknet.h"

#ifdef ANDROID
  //for setenv("QT_USE_ANDROID_NATIVE_DIALOGS", GOSTCOIN_NO_NATIVE_ANDROID_DIALOGS, GOSTCOIN_SETENV_OVERWRITE);
# include <stdlib.h>
# define GOSTCOIN_NO_NATIVE_ANDROID_DIALOGS "0"
# define GOSTCOIN_SETENV_OVERWRITE 1
#endif

#include <QMessageBox>
#if QT_VERSION < 0x050000
#include <QTextCodec>
#endif
#include <QLocale>
#include <QTimer>
#include <QTranslator>
#include <QLibraryInfo>

#ifdef Q_OS_MAC
#include "macdockiconhandler.h"
#endif

#if defined(BITCOIN_NEED_QT_PLUGINS) && !defined(_BITCOIN_QT_PLUGINS_INCLUDED)
#define _BITCOIN_QT_PLUGINS_INCLUDED
#define __INSURE__
#include <QtPlugin>
Q_IMPORT_PLUGIN(qcncodecs)
Q_IMPORT_PLUGIN(qjpcodecs)
Q_IMPORT_PLUGIN(qtwcodecs)
Q_IMPORT_PLUGIN(qkrcodecs)
Q_IMPORT_PLUGIN(qtaccessiblewidgets)
#endif

// Declare meta types used for QMetaObject::invokeMethod
Q_DECLARE_METATYPE(bool*)

// Need a global reference for the notifications to find the GUI
static BitcoinGUI *guiref;
static SplashScreen *splashref;

static void ThreadSafeShowGeneratedI2PAddress(const std::string& caption, const std::string& pub, const std::string& priv, const std::string& b32, const std::string& configFileName)
{
    if(guiref)
    {
        QMetaObject::invokeMethod(guiref, "showGeneratedI2PAddr",
                                   GUIUtil::blockingGUIThreadConnection(),
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(pub)),
                                   Q_ARG(QString, QString::fromStdString(priv)),
                                   Q_ARG(QString, QString::fromStdString(b32)),
                                   Q_ARG(QString, QString::fromStdString(configFileName)));
    }
    else
    {
        std::string msg = "\nIf you want to use a permanent I2P-address you have to set a \'mydestination\' option in the configuration file: ";
        msg += configFileName;
        msg += "\nGenerated address:\n";

        msg += "\nAddress + private key (save this text in the configuration file and keep it secret):\n";
        msg += priv;

        msg += "\n\nAddress (you can make it public):\n";
        msg += pub;

        msg += "\n\nShort base32-address:\n";
        msg += b32;
        msg += "\n\n";

        printf("%s: %s\n", caption.c_str(), msg.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), msg.c_str());
    }
}

static bool ThreadSafeMessageBox(const std::string& message, const std::string& caption, unsigned int style)
{
    // Message from network thread
    if(guiref)
    {
        bool modal = (style & CClientUIInterface::MODAL);
        bool ret = false;
        // In case of modal message, use blocking connection to wait for user to click a button
        QMetaObject::invokeMethod(guiref, "message",
                                   modal ? GUIUtil::blockingGUIThreadConnection() : Qt::QueuedConnection,
                                   Q_ARG(QString, QString::fromStdString(caption)),
                                   Q_ARG(QString, QString::fromStdString(message)),
                                   Q_ARG(unsigned int, style),
                                   Q_ARG(bool*, &ret));
        return ret;
    }
    else
    {
        printf("%s: %s\n", caption.c_str(), message.c_str());
        fprintf(stderr, "%s: %s\n", caption.c_str(), message.c_str());
        return false;
    }
}

static bool ThreadSafeAskFee(int64 nFeeRequired)
{
    if(!guiref)
        return false;
    if(nFeeRequired < CTransaction::nMinTxFee || nFeeRequired <= nTransactionFee || fDaemon)
        return true;

    bool payFee = false;

    QMetaObject::invokeMethod(guiref, "askFee", GUIUtil::blockingGUIThreadConnection(),
                               Q_ARG(qint64, nFeeRequired),
                               Q_ARG(bool*, &payFee));

    return payFee;
}

static void InitMessage(const std::string &message)
{
    if(splashref)
    {
        splashref->showMessage(QString::fromStdString(message), Qt::AlignBottom|Qt::AlignHCenter, QColor(55,55,55));
        qApp->processEvents();
    }
    printf("init message: %s\n", message.c_str());
}

/*
   Translate string to current locale using Qt.
 */
static std::string Translate(const char* psz)
{
    return QCoreApplication::translate("bitcoin-core", psz).toStdString();
}

/* Handle runaway exceptions. Shows a message box with the problem and quits the program.
 */
static void handleRunawayException(std::exception *e)
{
    PrintExceptionContinue(e, "Runaway exception");
    QMessageBox::critical(0, "Runaway exception", BitcoinGUI::tr("A fatal error occurred. Gostcoin can no longer continue safely and will quit.") + QString("\n\n") + QString::fromStdString(strMiscWarning));
    exit(1);
}

#ifndef BITCOIN_QT_TEST
int main(int argc, char *argv[])
{
    // Command-line options take precedence:
    ParseParameters(argc, argv);

#if QT_VERSION < 0x050000
    // Internal string conversion is all UTF-8
    QTextCodec::setCodecForTr(QTextCodec::codecForName("UTF-8"));
    QTextCodec::setCodecForCStrings(QTextCodec::codecForTr());
#endif

    Q_INIT_RESOURCE(gostcoin);
    QApplication app(argc, argv);

#ifdef ANDROID
    //workaround for https://bugreports.qt.io/browse/QTBUG-35545
    setenv("QT_USE_ANDROID_NATIVE_DIALOGS", GOSTCOIN_NO_NATIVE_ANDROID_DIALOGS, GOSTCOIN_SETENV_OVERWRITE);
#endif

    // Register meta types used for QMetaObject::invokeMethod
    qRegisterMetaType< bool* >();

    // Do this early as we don't want to bother initializing if we are just calling IPC
    // ... but do it after creating app, so QCoreApplication::arguments is initialized:
    if (PaymentServer::ipcSendCommandLine())
        exit(0);
    PaymentServer* paymentServer = new PaymentServer(&app);

    // Install global event filter that makes sure that long tooltips can be word-wrapped
    app.installEventFilter(new GUIUtil::ToolTipToRichTextFilter(TOOLTIP_WRAP_THRESHOLD, &app));

    // ... then bitcoin.conf:
    if (!boost::filesystem::is_directory(GetDataDir(false)))
    {
        // This message can not be translated, as translation is not initialized yet
        // (which not yet possible because lang=XX can be overridden in bitcoin.conf in the data directory)
        QMessageBox::critical(0, "Gostcoin",
                              QString("Error: Specified data directory \"%1\" does not exist.").arg(QString::fromStdString(mapArgs["-datadir"])));
        return 1;
    }
    // Gostcoin
    if (!boost::filesystem::exists(GetConfigFile().string()))
    {
       	// Run wizard
        runFirstRunWizard();
    }
    // Read config after it's potentional written by the wizard.
    ReadConfigFile(mapArgs, mapMultiArgs);

    // Application identification (must be set before OptionsModel is initialized,
    // as it is used to locate QSettings)
    QApplication::setOrganizationName("GOSTSec");
    QApplication::setOrganizationDomain("gostcoin.net");
    if(GetBoolArg("-testnet")) // Separate UI settings for testnet
        QApplication::setApplicationName("GOSTcoin-Qt-testnet");
    else
        QApplication::setApplicationName("GOSTcoin-Qt");

    // ... then GUI settings:
    OptionsModel optionsModel;

    // Get desired locale (e.g. "de_DE") from command line or use system locale
    QString lang_territory = QString::fromStdString(GetArg("-lang", QLocale::system().name().toStdString()));
    QString lang = lang_territory;
    // Convert to "de" only by truncating "_DE"
    lang.truncate(lang_territory.lastIndexOf('_'));

    QTranslator qtTranslatorBase, qtTranslator, translatorBase, translator;
    // Load language files for configured locale:
    // - First load the translator for the base language, without territory
    // - Then load the more specific locale translator

    // Load e.g. qt_de.qm
#ifdef Q_OS_MAC
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
#else
    if (qtTranslatorBase.load("qt_" + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
#endif
        app.installTranslator(&qtTranslatorBase);

    // Load e.g. qt_de_DE.qm
#ifdef Q_OS_MAC
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::path(QLibraryInfo::TranslationsPath)))
#else
    if (qtTranslator.load("qt_" + lang_territory, QLibraryInfo::location(QLibraryInfo::TranslationsPath)))
#endif
        app.installTranslator(&qtTranslator);

    // Load e.g. bitcoin_de.qm (shortcut "de" needs to be defined in gostcoin.qrc)
    if (translatorBase.load(lang, ":/translations/"))
        app.installTranslator(&translatorBase);

    // Load e.g. bitcoin_de_DE.qm (shortcut "de_DE" needs to be defined in gostcoin.qrc)
    if (translator.load(lang_territory, ":/translations/"))
        app.installTranslator(&translator);

    // Subscribe to global signals from core
    uiInterface.ThreadSafeMessageBox.connect(ThreadSafeMessageBox);
    uiInterface.ThreadSafeAskFee.connect(ThreadSafeAskFee);
    uiInterface.ThreadSafeShowGeneratedI2PAddress.connect(ThreadSafeShowGeneratedI2PAddress);
    uiInterface.InitMessage.connect(InitMessage);
    uiInterface.Translate.connect(Translate);

    // Show help message immediately after parsing command-line options (for "-lang") and setting locale,
    // but before showing splash screen.
    if (mapArgs.count("-?") || mapArgs.count("--help"))
    {
        GUIUtil::HelpMessageBox help;
        help.showOrPrint();
        return 1;
    }

#ifdef Q_OS_MAC
    // on mac, also change the icon now because it would look strange to have a testnet splash (green) and a std app icon (orange)
    if(GetBoolArg("-testnet")) {
        MacDockIconHandler::instance()->setIcon(QIcon(":icons/gostcoin_testnet"));
    }
#endif

    SplashScreen splash(QPixmap(), Qt::Widget);
    if (GetBoolArg("-splash", true) && !GetBoolArg("-min"))
    {
        splash.show();
        splash.setAutoFillBackground(true);
        splashref = &splash;
    }

    if (mapArgs.count("-style"))
    {
        QString filename = QString::fromStdString((std::string)GetDataDir().string());
        filename = filename.append(QDir::separator()).append(QString::fromStdString((std::string)mapArgs["-style"]));
        QFile file(filename);
        if (file.exists())
        {
            if (file.open(QIODevice::ReadOnly | QIODevice::Text))
            {
                QTextStream in(&file);
                QString content = "";
                while (!in.atEnd())
                {
                    content.append(in.readLine());
                }
                app.setStyleSheet(content);
            }
            else
            {
                QMessageBox::warning(NULL, BitcoinGUI::tr("Failed to load style!"),
                    BitcoinGUI::tr("Failed to load the stylesheet provided."),
                    QMessageBox::Ok, QMessageBox::Ok);
            }
        }
    }

    app.processEvents();
    app.setQuitOnLastWindowClosed(false);

    try
    {
#ifndef Q_OS_MAC
        // Regenerate startup link, to fix links to old versions
        // OSX: makes no sense on mac and might also scan/mount external (and sleeping) volumes (can take up some secs)
        if (GUIUtil::GetStartOnSystemStartup())
            GUIUtil::SetStartOnSystemStartup(true);
#endif

        boost::thread_group threadGroup;

        BitcoinGUI window;
        guiref = &window;

        QTimer* pollShutdownTimer = new QTimer(guiref);
        QObject::connect(pollShutdownTimer, SIGNAL(timeout()), guiref, SLOT(detectShutdown()));
        pollShutdownTimer->start(200);

        if(AppInit2(threadGroup))
        {
            {
                // Put this in a block, so that the Model objects are cleaned up before
                // calling Shutdown().

                optionsModel.Upgrade(); // Must be done after AppInit2

                if (splashref)
                    splash.finish(&window);

                ClientModel clientModel(&optionsModel);
                WalletModel *walletModel = 0;
                if(pwalletMain)
                    walletModel = new WalletModel(pwalletMain, &optionsModel);

                window.setClientModel(&clientModel);
                if(walletModel)
                {
                    window.addWallet("~Default", walletModel);
                    window.setCurrentWallet("~Default");
                }

                // If -min option passed, start window minimized.
                if(GetBoolArg("-min"))
                {
                    window.showMinimized();
                }
                else
                {
                    window.show();
                }

                // Now that initialization/startup is done, process any command-line
                // bitcoin: URIs
                QObject::connect(paymentServer, SIGNAL(receivedURI(QString)), &window, SLOT(handleURI(QString)));
                QTimer::singleShot(100, paymentServer, SLOT(uiReady()));

                app.exec();

                window.hide();
                window.setClientModel(0);
                window.removeAllWallets();
                guiref = 0;
                delete walletModel;
            }
            // Shutdown the core and its threads, but don't exit Bitcoin-Qt here
            threadGroup.interrupt_all();
            threadGroup.join_all();
            Shutdown();
        }
        else
        {
            threadGroup.interrupt_all();
            threadGroup.join_all();
            Shutdown();
            return 1;
        }
    } catch (std::exception& e) {
        qDebug() << "error:" << e.what();
        handleRunawayException(&e);
    } catch (...) {
        handleRunawayException(NULL);
    }
    return 0;
}
#endif // BITCOIN_QT_TEST
