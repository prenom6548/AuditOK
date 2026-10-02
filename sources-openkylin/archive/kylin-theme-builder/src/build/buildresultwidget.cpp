#include "buildresultwidget.h"

buildResultWidget::buildResultWidget(QWidget *parent) :
    kdk::KDialog(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setFixedSize(QSize(424,170));
    this->setFocus();

    QWidget*mainwindow = mainWidget();
    QVBoxLayout *m_buildResultLayout = new QVBoxLayout(mainwindow);
    QHBoxLayout *tipLayout = new QHBoxLayout();
    QHBoxLayout *btnLayout = new QHBoxLayout();
    m_buildResultLayout->setContentsMargins(23,0,23,23);
    QLabel *iconlabel = new QLabel();
    iconlabel->setPixmap(QIcon::fromTheme("ukui-dialog-success").pixmap(16,16));
    QLabel *m_buildResult = new QLabel;
    m_buildResult->setText(tr("Theme finished!"));

    QLabel *guideLabel = new QLabel();
    QString str = "<a href=\"https://app-distribute.kylinos.cn\">" + tr("Upload theme platform") + "</a>";
    guideLabel->setText(str);
    guideLabel->setTextFormat(Qt::RichText);
    guideLabel->setCursor(Qt::PointingHandCursor);
    guideLabel->setOpenExternalLinks(true); // 将此属性设置为 true

    QPushButton *m_backHomeBtn = new QPushButton();
    m_backHomeBtn->setText(tr("OK"));
    tipLayout->addWidget(iconlabel);
    tipLayout->addWidget(m_buildResult);
    tipLayout->addStretch(1);

    btnLayout->addWidget(guideLabel);
    btnLayout->addStretch(1);
    btnLayout->addWidget(m_backHomeBtn);
    m_buildResultLayout->addLayout(tipLayout);
    m_buildResultLayout->addLayout(btnLayout);

    QPushButton* closebtn = closeButton();

    connect(closebtn,&QPushButton::clicked,m_backHomeBtn,&QPushButton::click);
    connect(m_backHomeBtn, &QPushButton::clicked, this,[=](){
        emit gohomesignals();
        this->close();
    });

}
