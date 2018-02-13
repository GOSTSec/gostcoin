#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "clientmodel.h"
#include "clientversion.h"

AboutDialog::AboutDialog(QWidget *parent) :
	QDialog(parent),
	ui(new Ui::AboutDialog)
{
	ui->setupUi(this);

	// Set current copyright year
	ui->copyrightLabel->setText(
		tr("Copyright") + QString(" &copy; 2009-%1 ").arg(COPYRIGHT_YEAR) + tr("The Bitcoin developers")  + QString("<br>") + 
		tr("Copyright") + QString(" &copy; 2011-%1 ").arg(COPYRIGHT_YEAR) + tr("The Litecoin developers") + QString("<br>") + 
		tr("Copyright") + QString(" &copy; 2013-%1 ").arg(COPYRIGHT_YEAR) + tr("The Anoncoin developers") + QString("<br>") + 
		tr("Copyright") + QString(" &copy; 2015-%1 ").arg(COPYRIGHT_YEAR) + tr("The i2pd developers")     + QString("<br>") + 
		tr("Copyright") + QString(" &copy; 2017-%1 ").arg(COPYRIGHT_YEAR) + tr("The Gostcoin developers")
	);
}

void AboutDialog::setModel(ClientModel *model)
{
	if(model)
	{
		ui->versionLabel->setText(model->formatFullVersion());
	}
}

AboutDialog::~AboutDialog()
{
	delete ui;
}

void AboutDialog::on_buttonBox_accepted()
{
	close();
}
