// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2012 The Bitcoin developers
// Distributed under the MIT/X11 software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "ui_interface.h"
#include "init.h"
#include "bitcoinrpc.h"

#include <string>

static bool noui_ThreadSafeMessageBox(const std::string& message, const std::string& caption, unsigned int style)
{
    std::string strCaption;
    // Check for usage of predefined caption
    switch (style) {
    case CClientUIInterface::MSG_ERROR:
        strCaption += _("Error");
        break;
    case CClientUIInterface::MSG_WARNING:
        strCaption += _("Warning");
        break;
    case CClientUIInterface::MSG_INFORMATION:
        strCaption += _("Information");
        break;
    default:
        strCaption += caption; // Use supplied caption (can be empty)
    }

    printf("%s: %s\n", strCaption.c_str(), message.c_str());
    fprintf(stderr, "%s: %s\n", strCaption.c_str(), message.c_str());
    return false;
}

static void noui_ThreadSafeShowGeneratedI2PAddress(const std::string& caption, const std::string& pub, const std::string& priv, const std::string& b32, const std::string& configFileName)
{
    std::string msg = "\nIf you want to use a permanent I2P-address you have to set a \'mydestination\' option in the configuration file:\n";
    msg += configFileName;

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

static bool noui_ThreadSafeAskFee(int64 /*nFeeRequired*/)
{
    return true;
}

static void noui_InitMessage(const std::string &message)
{
    printf("init message: %s\n", message.c_str());
}

void noui_connect()
{
    // Connect bitcoind signal handlers
    uiInterface.ThreadSafeMessageBox.connect(noui_ThreadSafeMessageBox);
    uiInterface.ThreadSafeAskFee.connect(noui_ThreadSafeAskFee);
    uiInterface.InitMessage.connect(noui_InitMessage);
}
