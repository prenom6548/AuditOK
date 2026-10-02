#include "cmyfiledialog.h"
#include <locale.h>
#include <libintl.h>
#include <QAbstractItemView>
//#define APP_LANGUAGE "ksc-defender"
//#define _(STRING) dgettext(APP_LANGUAGE, STRING)

CMyFileDialog::CMyFileDialog(QString titleName, QWidget *parent) : QFileDialog(parent)
{
    char *charTitle;
    QByteArray title = titleName.toLocal8Bit();
    charTitle = title.data();
//    setWindowTitle(_(charTitle));

    setWindowIcon(QIcon::fromTheme("ksc-defender"));
}

void CMyFileDialog::showEvent(QShowEvent *event)
{
    if(this->isVisible())
    {
        for(QWidget *widget : this->parent()->findChildren<QWidget*>())
        {
            if(widget->objectName() == "acceptButton")
            {
                QDialogButtonBox *button = (QDialogButtonBox *)widget;
                disconnect(button,0,0,0);//使链接失效
                connect(button,SIGNAL(clicked()),this,SLOT(slot_myAccetp()));//改成自己的槽
            }
        }
    }
}

void CMyFileDialog::slot_myAccetp()
{
    QDialog::accept();
}

