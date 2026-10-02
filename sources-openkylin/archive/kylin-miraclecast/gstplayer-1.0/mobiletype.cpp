#include "mobiletype.h"
#include "ui_mobiletype.h"

MobileType::MobileType(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::MobileType)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_DeleteOnClose);
    QPalette pa;
    pa.setColor(QPalette::WindowText, Qt::red);
    ui->label->setPalette(pa);

    QLocale locale;
    if (locale.language() == QLocale::Chinese){
        ui->ChangeMobileType->setText("请确认设备类型");
        ui->ignoreBtn->setText("取消");
        ui->label->setText("注意：重新更换设备型号需删除该历史设备记录，先关闭再开启投屏功能。");
        ui->label->setWordWrap(true);
        setWindowTitle("投屏");
    }
    else if (locale.language() == QLocale::English){
        ui->ChangeMobileType->setText("Please confirm the device type");
        ui->ignoreBtn->setText("Cancel");
        QString newStrMsg = "Note: to replace the device model again, you need to delete the historical device record. Turn off and then turn on the screen projection function.";
        QFontMetrics fontWidth(ui->label->font());//得到每个字符的宽度
        QString elideNote = fontWidth.elidedText(newStrMsg, Qt::ElideRight, 429);//最大宽度150像素
        ui->label->setText(elideNote);//显示省略好的字符串
        ui->label->setToolTip(newStrMsg);//设置tooltips
        setWindowTitle("cast");
    }

    connect(ui->ignoreBtn, &QPushButton::clicked, [=]{
        close();
    });
    connect(ui->confirmBtn, &QPushButton::clicked, [=]{
        emit sendMobileType(ui->comboBox->currentText());
        close();
    });
}

MobileType::~MobileType()
{
    delete ui;
}

