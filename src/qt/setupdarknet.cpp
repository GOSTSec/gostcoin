// Copyright 2013 The Anoncoin Developers
// Copyright 2017 The Gostcoin Developers

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>

#include "setupdarknet.h"
#include "bitcoingui.h"
#include "util.h"


void runFirstRunWizard()
{
   QString strMessage = BitcoinGUI::tr("Do you run I2P on your computer?\n"
                        "If so, press yes to let Gostcoin configure it's "
                        "connection to the I2P");
    QMessageBox::StandardButton isRunningDarknet = QMessageBox::question(NULL, 
        BitcoinGUI::tr("Gostcoin Wizard - Step #1"), strMessage,
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    if (isRunningDarknet == QMessageBox::No)
    {
        // Tell user to download
        strMessage = BitcoinGUI::tr("Do you want to install I2P, and "
                     "continue I2P setup?\nIf you select yes Gostcoin will quit, "
                     "so you can continue the wizard after installing I2P.\n"
                     "If you select no, Gostcoin will write a default clearnet "
                     "(regular internet) config file for you. (unsafe mode)\n");
        QMessageBox::StandardButton wantDownloadDarknet = QMessageBox::question(NULL,
            BitcoinGUI::tr("Gostcoin Wizard - Step #2"), strMessage,
            QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if (wantDownloadDarknet == QMessageBox::No)
        {
            writeFirstConfig(false, false);
            return;
        }
        QApplication::quit();
        exit(7); // Exit code 7 = awaiting darknet
    }
    // Step #2 passed. Use I2P 
    FinishWizard();
}

void FinishWizard()
{
    QMessageBox msgBox;
    QPushButton *sharedButton = msgBox.addButton(BitcoinGUI::tr("Shared"), QMessageBox::ActionRole);
	QPushButton * i2pOnlyButton =  msgBox.addButton(BitcoinGUI::tr("I2P only"), QMessageBox::ActionRole);
    msgBox.setText(BitcoinGUI::tr("Ok, last question!\nDo you want to run shared "
                                  "(which means you work as a bridge between the "
                                  "I2P and internet) or only I2P?"));
    msgBox.exec();
    if (msgBox.clickedButton() == i2pOnlyButton)
    	writeFirstConfig (true, true); 
    else if (msgBox.clickedButton() == sharedButton)
        writeFirstConfig (false, true);
    QMessageBox msgBoxThanks;
    msgBoxThanks.setText(BitcoinGUI::tr("Thanks! That was all. Enjoy your wallet :)"));
    msgBoxThanks.exec();
    return;
}


