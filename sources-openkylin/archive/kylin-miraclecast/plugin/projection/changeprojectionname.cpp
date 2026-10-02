#include "changeprojectionname.h"
#include "ui_changeprojectionname.h"

#include <QDebug>


//extern void qt_blurImage(QImage &blurImage, qreal radius, bool quality, int transposed);

ChangeProjectionName::ChangeProjectionName(QWidget *parent) 
:
    QDialog(parent),
    ui(new Ui::ChangeProjectionName)
{
    ui->setupUi(this);
    //setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);
    // setWindowFlags(Qt::WindowCloseButtonHint);
//    setAttribute(Qt::WA_TranslucentBackground);
    setAttribute(Qt::WA_DeleteOnClose);

    QPalette pa;
    pa.setColor(QPalette::WindowText, Qt::red);
    ui->tipLabel_2->setPalette(pa);
    ui->lineEdit->setMaxLength(32);
    ui->lineEdit->setAttribute(Qt::WA_InputMethodEnabled, false);

    ui->saveBtn->setEnabled(false);
    QLocale locale;
    if (locale.language() == QLocale::English){
        QString newStrMsg = "Device Name";
        QFontMetrics fontWidth(ui->ChangeProjectionname->font());//得到每个字符的宽度
        QString elideNote = fontWidth.elidedText(newStrMsg, Qt::ElideRight, 74);//最大宽度74像素
        ui->ChangeProjectionname->setText(elideNote);//显示省略好的字符串
        ui->ChangeProjectionname->setToolTip(newStrMsg);//设置tooltips
    }

    connect(ui->lineEdit, &QLineEdit::textChanged, this, [=](QString txt){
        if (txt.toLocal8Bit().length() > 31){
            ui->lineEdit->setFocusPolicy(Qt::NoFocus);
            QString StrMsg = tr("The length must be 1-32 characters");
            QFontMetrics fontWidth(ui->tipLabel_2->font());//得到每个字符的宽度
            QString elideNote_2 = fontWidth.elidedText(StrMsg, Qt::ElideRight, 298);//最大宽度74像素
            ui->tipLabel_2->setText(elideNote_2);
            ui->tipLabel_2->setToolTip(StrMsg);
//            ui->tipLabel_2->setText(tr("The length must be 1-32 characters"));
//            ui->saveBtn->setEnabled(false);
        }else {
            ui->tipLabel_2->setText(tr(""));
        }

        if (!txt.isEmpty()){
            ui->saveBtn->setEnabled(true);
        } else {
            ui->saveBtn->setEnabled(false);
        }
    });

    connect(ui->cancelBtn, &QPushButton::clicked, [=]{
        close();
    });

    connect(ui->saveBtn, &QPushButton::clicked, [=]{
        emit sendNewProjectionName(ui->lineEdit->text());
        close();
    });
}

ChangeProjectionName::~ChangeProjectionName()
{
    delete ui;
}

//void ChangeProjectionName::paintEvent(QPaintEvent *event) {
//    Q_UNUSED(event)

//    QPainter p(this);
//    p.setRenderHint(QPainter::Antialiasing);
//    QPainterPath rectPath;
//    rectPath.addRoundedRect(this->rect().adjusted(10, 10, -10, -10), 6, 6);

//    // 画一个黑底
//    QPixmap pixmap(this->rect().size());
//    pixmap.fill(Qt::transparent);
//    QPainter pixmapPainter(&pixmap);
//    pixmapPainter.setRenderHint(QPainter::Antialiasing);
//    pixmapPainter.setPen(Qt::transparent);
//    pixmapPainter.setBrush(Qt::black);
//    pixmapPainter.setOpacity(0.65);
//    pixmapPainter.drawPath(rectPath);
//    pixmapPainter.end();

//    // 模糊这个黑底
//    QImage img = pixmap.toImage();
//    qt_blurImage(img, 10, false, false);

//    // 挖掉中心
//    pixmap = QPixmap::fromImage(img);
//    QPainter pixmapPainter2(&pixmap);
//    pixmapPainter2.setRenderHint(QPainter::Antialiasing);
//    pixmapPainter2.setCompositionMode(QPainter::CompositionMode_Clear);
//    pixmapPainter2.setPen(Qt::transparent);
//    pixmapPainter2.setBrush(Qt::transparent);
//    pixmapPainter2.drawPath(rectPath);

//    // 绘制阴影
//    p.drawPixmap(this->rect(), pixmap, pixmap.rect());

//    // 绘制一个背景
//    p.save();
//    p.fillPath(rectPath,palette().color(QPalette::Base));
//    p.restore();
//}
