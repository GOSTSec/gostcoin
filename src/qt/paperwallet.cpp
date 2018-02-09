#include <stdio.h>
#include <QPainter>
#include <QTextStream>
#include <QTextDocument>
#include <QUrl>
#include <QFile>
#include <QPrintDialog>
#include <QPrinter>

#include <qrencode.h>

#include "key.h"
#include "base58.h"
#include "paperwallet.h"

static void printAsQR(QPainter &painter, QString &vchKey, int shift)
{
    QRcode *qr = QRcode_encodeString(vchKey.toStdString().c_str(), 1, QR_ECLEVEL_L, QR_MODE_8, 1);
    if (qr) 
	{
        QPaintDevice *pd = painter.device(); 
        const double w = pd->width();
        const double h = pd->height();
        QColor bg("white");
	    QColor fg("black");
	    painter.setBrush(bg);
	    painter.fillRect(0, 0, w, h, bg);
	    painter.setPen(Qt::SolidLine);
	    painter.setPen(fg);
	    painter.setBrush(fg);
        const int s=qr->width > 0 ? qr->width : 1;
        const double aspect = w / h;
        const double scale = ((aspect > 1.0) ? h : w) / s;// * 0.3;
        for(int y = 0; y < s; y++)
		{
            const int yy = y*s;
            for(int x = 0; x < s; x++)
			{
                const int xx = yy + x;
                const unsigned char b = qr->data[xx];
                if(b & 0x01)
				{
                    const double rx1 = x*scale, ry1 = y*scale;
                    QRectF r(rx1 + shift, ry1, scale, scale);
                    painter.drawRects(&r, 1);
                }
            }
        }
        QRcode_free(qr);
    }
}

static bool readHtmlTemplate(const QString &res_name, QString &htmlContent)
{
    QFile htmlFile(res_name);
    if (!htmlFile.open(QIODevice::ReadOnly | QIODevice::Text))
	{
        printf("Cant open %s\n", res_name.toStdString().c_str ());
        return false;
    }

    QTextStream in(&htmlFile);
    in.setCodec("UTF-8");
    htmlContent = in.readAll();
    return true;
}

void PrintPaperWallet (QWidget * parent)
{
	RandAddSeedPerfmon();
	CKey secret;
    secret.MakeNewKey(true);

	CPrivKey privkey = secret.GetPrivKey();
    CPubKey pubkey = secret.GetPubKey();
    CKeyID keyid = pubkey.GetID();	

	std::string secret_str = CBitcoinSecret(secret).ToString();
    std::string address = CBitcoinAddress(keyid).ToString();
    
    QString qsecret = QString::fromStdString(secret_str);
    QString qaddress = QString::fromStdString(address);

	QPrinter printer(QPrinter::HighResolution);
    printer.setPageMargins(0, 10, 0, 0, QPrinter::Millimeter);
    
    QPrintDialog dlg(&printer, parent);
    if(dlg.exec() == QDialog::Accepted) 
	{
		QImage img1(200, 200, QImage::Format_ARGB32);
	    QImage img2(200, 200, QImage::Format_ARGB32);
	    QPainter painter(&img1);
	    painter.setRenderHint(QPainter::Antialiasing, false);
	    painter.setRenderHint(QPainter::TextAntialiasing, false);
	    painter.setRenderHint(QPainter::SmoothPixmapTransform, false);
	    painter.setRenderHint(QPainter::HighQualityAntialiasing, false);
	    painter.setRenderHint(QPainter::NonCosmeticDefaultPen, false);
	    printAsQR(painter, qaddress, 0);
		painter.end();

        painter.begin(&img2);
        printAsQR(painter, qsecret, 0);
        painter.end();

		QString html;
        readHtmlTemplate(":/html/paperwallet", html);
        
        html.replace("__ADDRESS__", qaddress);
        html.replace("__PRIVATE__", qsecret);
        
        QTextDocument document;
        document.addResource(QTextDocument::ImageResource, QUrl(":qr1.png" ), img1);
        document.addResource(QTextDocument::ImageResource, QUrl(":qr2.png" ), img2);
        document.setHtml(html);
        document.setPageSize(QSizeF(printer.pageRect(QPrinter::Point).size()));
        document.print(&printer);
	}	
}
