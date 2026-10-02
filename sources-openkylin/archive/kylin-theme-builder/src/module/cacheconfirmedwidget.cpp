#include "cacheconfirmedwidget.h"

//是否保存记录界面
cacheConfirmedWidget::cacheConfirmedWidget(QWidget *parent):
    kdk::KDialog(parent)
{
    this->setWindowModality(Qt::ApplicationModal);
    this->setFixedSize(424,172);
    this->setFocus();

    QWidget*mainwindow = mainWidget();

//    m_cacheConfirmedWidget = new QVBoxLayout;
    m_questionMessage = new QLabel();
    m_questionMessage->setText(tr("Is it saved as edit history?"));
    m_questionMessage->setToolTip(tr("Is it saved as edit history?"));

    m_saveCache = new QPushButton();
    m_cleanCache = new QPushButton();
    m_cancel = new QPushButton();

    m_saveCache->setFixedSize(96,36);
    m_cleanCache->setFixedSize(96,36);
    m_cancel->setFixedSize(96,36);

    m_saveCache->setText(tr("Save"));
    m_cleanCache->setText(tr("Clean"));
    m_cancel->setText(tr("cancel"));
//    m_cacheConfirmedBtn = new QHBoxLayout();
//    m_cacheConfirmedBtn->addWidget(m_saveCache);
//    m_cacheConfirmedBtn->addWidget(m_cleanCache);
//    m_cacheConfirmedBtn->addWidget(m_cancel);

//    m_cacheConfirmedWidget->addWidget(m_questionMessage);
//    m_cacheConfirmedWidget->addLayout(m_cacheConfirmedBtn);
//    this->setLayout(m_cacheConfirmedWidget);

//    QGridLayout * layout = new QGridLayout(mainwindow);
//    layout->addWidget(m_questionMessage,0,0);
//    layout->addWidget(m_saveCache,1,0);
//    layout->addWidget(m_cleanCache,1,1);
//    layout->addWidget(m_cancel,1,2);
    QVBoxLayout * layout = new QVBoxLayout(mainwindow);
    QHBoxLayout * btnlayout = new QHBoxLayout();
    btnlayout->addWidget(m_saveCache);
    btnlayout->addStretch(1);
    btnlayout->addWidget(m_cleanCache);
    btnlayout->addWidget(m_cancel);
    layout->addWidget(m_questionMessage);
    layout->addLayout(btnlayout);

    layout->setContentsMargins(24, 0, 24, 24);

    QObject::connect(m_saveCache,&QPushButton::clicked,this,[=](){
        InfoCreateWidget* m_infoCreateWidget = new InfoCreateWidget(nullptr,"recognise");
        m_infoCreateWidget->m_themeNameCNLineEdit->setText(FileProcess::g_themeCNName);
        m_infoCreateWidget->m_themeNameENLineEdit->setText(FileProcess::g_themeENName);
        m_infoCreateWidget->m_builderMailLineEdit->setText(FileProcess::g_builderMailName);
        m_infoCreateWidget->m_builderNameLineEdit->setText(FileProcess::g_builderName);
        m_infoCreateWidget->show();
        connect(m_infoCreateWidget,&InfoCreateWidget::readytoSave,this,[=](){
            emit cacheSave();
            this->close();
        });
    });
    QObject::connect(m_cleanCache,&QPushButton::clicked,this,[=](){
        FileProcess m_fileClean;
        m_fileClean.FileClean(FileProcess::g_date);
        emit cacheClean();
        this->close();
    });
    QObject::connect(m_cancel,&QPushButton::clicked,this,[=](){
        emit cacheCancel();
        this->close();
    });
}

void cacheConfirmedWidget::closeEvent(QCloseEvent *event)
{
    emit visibilityChanged(false);
    QWidget::closeEvent(event);
}
