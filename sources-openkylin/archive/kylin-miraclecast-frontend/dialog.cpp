#include "dialog.h"
#include "ui_dialog.h"
#include "xatom-helper.h"
#include <QApplication>
#include <QDebug>


Dialog::Dialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog)
{
    ui->setupUi(this);

    ui->pushButton_allow->setProperty("isImportant",true);
    QPalette pe;
    QColor color(55,144,250);
    pe.setColor(QPalette::ButtonText, Qt::white);
    pe.setColor(QPalette::Inactive, QPalette::Button,color);
    ui->pushButton_allow->setPalette(pe);
    ui->pushButton_allow->setText(tr("allow"));

    ui->label_pin->setStyleSheet("QLabel{font-size: 24px}");
    ui->label_pin->setText("12345678");
   ui->warningLabel->setFont(QFont("Timers", 14, QFont::Bold));
   QPalette pa;
   pa.setColor(QPalette::WindowText, Qt::red);
   ui->warningLabel->setPalette(pa);
    this->setWindowFlags(Qt::WindowFlags()|Qt::WindowStaysOnTopHint);
    setWindowFlags(Qt::Dialog);
    this->setFixedSize(this->width(),this->height());
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButton_allow_clicked()
{
    qDebug()<<"allow";
    qDebug()<<ui->pushButton_allow->text();
    if(ui->pushButton_allow->text()==tr("ok"))//允许(连接时)
    {
        this->hide();
        return;
    }
    this->hide();
    emit allow_btn_signal();
}

void Dialog::on_pushButton_refuse_clicked()
{
    qDebug()<<"refuse";
    this->hide();
    emit refuse_btn_signal();
}
void Dialog::set_label_text(QString msg)
{
    ui->label->setText(msg);
    ui->label->setWordWrap(true);
}
void Dialog::set_label_pin_text(QString msg)
{
    ui->label_pin->setText(msg);
}

void Dialog::setLabelText(QLabel *label, QString text)
{
    QFontMetrics  fontMetrics(label->font());
    int fontSize = fontMetrics.width(text);
    if (fontSize > label->width()) {
        label->setText(fontMetrics.elidedText(text, Qt::ElideRight, label->width()));
        label->setToolTip(text);
    } else {
        label->setText(text);
        label->setToolTip("");
    }
}

void Dialog::set_warningLabel_text(QString msg)
{
   setLabelText(ui->warningLabel, msg);
}
void Dialog::set_button(int a)
{
    if(a==0)
    {
        ui->pushButton_allow->setVisible(false);
        ui->pushButton_refuse->setVisible(false);
    }
    else{
         ui->pushButton_allow->setVisible(true);
        ui->pushButton_refuse->setVisible(true);
    }
        
}
void Dialog::set_countdown(int a)
{
   if(a==0)
   {
       ui->warningLabel->setVisible(false);
   }
   else{
       ui->warningLabel->setVisible(true);
   }
}
void Dialog::set_label_pin(int a)
{
    if(a==0)
    {
        ui->label_pin->setVisible(false);
    }
    else{
        ui->label_pin->setVisible(true);
    }      
}
void Dialog::set_button_2()
{
    ui->pushButton_allow->setText(tr("ok"));
    ui->pushButton_allow->setVisible(true);
}

void Dialog::paintEvent(QPaintEvent * event){
    Q_UNUSED(event)

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    QPalette pa;
    pa.setColor(QPalette::Text, Qt::red);
    ui->warningLabel->setPalette(pa);

    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing);
    QPainterPath rectPath;
    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);

    QPixmap pixmap(this->rect().size());
    pixmap.fill(Qt::transparent);
    QPainter pixmapPainter(&pixmap);
    pixmapPainter.setRenderHint(QPainter::Antialiasing);
    pixmapPainter.setPen(Qt::transparent);
    pixmapPainter.setBrush(Qt::white);
    pixmapPainter.drawPath(rectPath);
    pixmapPainter.end();

    QImage img = pixmap.toImage();

    pixmap = QPixmap::fromImage(img);
    QPainter pixmapPainter2(&pixmap);
    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
    pixmapPainter2.setPen(Qt::transparent);
    pixmapPainter2.setBrush(Qt::transparent);
    pixmapPainter2.drawPath(rectPath);

    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

    p.save();
    p.fillPath(rectPath,palette().color(QPalette::Base));
    p.restore();
}

void Dialog::set_visible(int a)
{
    if(a==0)
    {
        this->setVisible(false);
    }
    else{
         this->setVisible(true);
    }
}
