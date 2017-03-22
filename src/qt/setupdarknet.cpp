// Copyright 2013 The Anoncoin Developers

#include <QApplication>
#include <QMessageBox>
#include <QPushButton>

#include "setupdarknet.h"
#include "bitcoingui.h"
#include "util.h"


void runFirstRunWizard()
{
   QString strMessage = BitcoinGUI::tr("Do you run Tor or I2P on your computer?\n"
                        "If so, press yes to let Anoncoin configure it's "
                        "connection to the prefered darknet of yours.");
    QMessageBox::StandardButton isRunningDarknet = QMessageBox::question(NULL, 
        BitcoinGUI::tr("Anoncoin Wizard - Step #1"), strMessage,
        QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
    if (isRunningDarknet == QMessageBox::No)
    {
        // Tell user to download
        strMessage = BitcoinGUI::tr("Do you want to download Tor or I2P, and "
                     "continue darknet setup?\nIf you select yes Anoncoin will quit, "
                     "so you can continue the wizard after installing a darknet.\n"
                     "If you select no, Anoncoin will write a default clearnet "
                     "(regular internet) config file for you. (unsafe mode)\n");
        QMessageBox::StandardButton wantDownloadDarknet = QMessageBox::question(NULL,
            BitcoinGUI::tr("Anoncoin Wizard - Step #2"), strMessage,
            QMessageBox::Yes|QMessageBox::No, QMessageBox::Yes);
        if (wantDownloadDarknet == QMessageBox::No)
        {
            writeFirstConfig(false, false, false, false);
            return;
        }
        QApplication::quit();
        exit(7); // Exit code 7 = awaiting darknet
    }
    // Step #2 passed
    QMessageBox msgBox;
    msgBox.setText(BitcoinGUI::tr("Great! Then which darknet do you run? Or maybe both?"));
    QPushButton *i2pButton  = msgBox.addButton(BitcoinGUI::tr("I2P"), QMessageBox::ActionRole);
    QPushButton *torButton  = msgBox.addButton(BitcoinGUI::tr("Tor"), QMessageBox::ActionRole);
    QPushButton *bothButton = msgBox.addButton(BitcoinGUI::tr("Both"), QMessageBox::ActionRole);
    msgBox.exec();
    if (msgBox.clickedButton() == i2pButton)
    {
        FinishWizard(1);
        return;
    }
    else if (msgBox.clickedButton() == torButton)
    {
        FinishWizard(2);
        return;
    }
    else if (msgBox.clickedButton() == bothButton)
    {
        FinishWizard(3);
        return;
    }
}

void FinishWizard(int darknet)
{
    QMessageBox msgBox;
    QPushButton *darknetOnlyButton;
    QPushButton *sharedButton = msgBox.addButton(BitcoinGUI::tr("Shared"), QMessageBox::ActionRole);
    switch (darknet)
    {
      case 1:
        darknetOnlyButton =  msgBox.addButton(BitcoinGUI::tr("I2P only"), QMessageBox::ActionRole);
      break;
      case 2:
        darknetOnlyButton =  msgBox.addButton(BitcoinGUI::tr("Tor only"), QMessageBox::ActionRole);
      break;
    }
    msgBox.setText(BitcoinGUI::tr("Ok, last question!\nDo you want to run shared "
                                  "(which means you work as a bridge between the "
                                  "darknet and internet) or only darknet?"));
    msgBox.exec();
    if (msgBox.clickedButton() == darknetOnlyButton)
    {
        if (darknet == 1) // I2P
            writeFirstConfig(true, false, false, false);
        if (darknet == 2) // Tor
            writeFirstConfig(false, true, false, false);
    }
    else if (msgBox.clickedButton() == sharedButton)
    {
        if (darknet == 1) // I2P
            writeFirstConfig(false, false, true, false);
        if (darknet == 2) // Tor
            writeFirstConfig(false, false, false, true);
    }
    if (darknet == 3) // Both 
        writeFirstConfig(false, false, true, true);
    QMessageBox msgBoxThanks;
    msgBoxThanks.setText(BitcoinGUI::tr("Thanks! That was all. Enjoy your wallet :)"));
    msgBoxThanks.exec();
    return;
}


