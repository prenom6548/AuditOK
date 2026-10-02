#include "widget.h"

Widget::Widget(QWidget *parent)
    : QWidget(parent)
{
    /* 初始化桌面全屏并填充图片背景 */
    initTranslation();
    initWidgetFullDesktop();
    initUI();
    initConnectSlots();
}

Widget::~Widget()
{
}

void Widget::initWidgetFullDesktop()
{
    // 无边框 全屏
    this->setWindowFlags(Qt::FramelessWindowHint);
    setAttribute(Qt::WA_TranslucentBackground);
    this->setWindowState(Qt::WindowFullScreen);
}

void Widget::initTranslation()
{
    m_ptranslator = new QTranslator;
    QTranslator *translator_qt = new QTranslator;
    //获取系统语言环境 选择翻译文件
    if (m_ptranslator->load(QLocale(), QLatin1String("kylin-fix-boot-grub"), QLatin1String("_"), QLatin1String("/usr/share/kylin-fix-boot-grub/")))
        QApplication::installTranslator(m_ptranslator);
    else
        qDebug() << "cannot load translator kylin-fix-boot-grub" << QLocale::system().name() << ".qm!";

    if (translator_qt->load("/usr/share/qt5/translations/qt_"+QLocale::system().name()))
        QApplication::installTranslator(translator_qt);
    else
        qDebug() << "cannot load translator kylin-fix-boot-grub" << QLocale::system().name() << ".qm!";
}

void Widget::initUI()
{
    m_pCeterStackWidget = new centerStackWidget();
    m_pCenterWidget = new centerWidget();
    m_pRepairingWidget = new repairingWidget();
    m_pSuccessWidget = new successWidget();
    m_pFixFailedWidget = new fixFailedWidget();

    m_pCeterStackWidget->m_pStackWidget->addWidget(m_pCenterWidget);
    m_pCeterStackWidget->m_pStackWidget->addWidget(m_pRepairingWidget);
    m_pCeterStackWidget->m_pStackWidget->addWidget(m_pSuccessWidget);
    m_pCeterStackWidget->m_pStackWidget->addWidget(m_pFixFailedWidget);
    m_pCeterStackWidget->m_pStackWidget->setCurrentIndex(0);
    int cx = (QApplication::desktop()->width() - m_pCenterWidget->width()) / 2;
    int cy = (QApplication::desktop()->height() - m_pCenterWidget->height()) / 2;
    m_pMainVboxLayout = new QVBoxLayout();
    m_pMainVboxLayout->setContentsMargins(cx, 0, 0, 0);
    m_pMainVboxLayout->setSpacing(0);
    m_pMainVboxLayout->addSpacerItem(new QSpacerItem(10, cy, QSizePolicy::Fixed));
    m_pMainVboxLayout->addWidget(m_pCeterStackWidget, Qt::AlignTop);
    m_pMainVboxLayout->addSpacerItem(new QSpacerItem(10, cy, QSizePolicy::Fixed));
    this->setLayout(m_pMainVboxLayout);
}

void Widget::initConnectSlots()
{
//    connect(m_pCenterWidget, &centerWidget::execFixBashStatus, this, &Widget::switchStackWidget);
    connect(m_pCenterWidget, &centerWidget::execSwitchWidgetFixing, this, &Widget::switchFixingWidget);
}

void Widget::switchStackWidget(bool status)
{
    if (status) {
        m_pCeterStackWidget->m_pStackWidget->setCurrentIndex(2);
    } else {
        m_pCeterStackWidget->m_pStackWidget->setCurrentIndex(3);
    }
    m_pRepairingWidget->m_pTime->stop();
}

void Widget::switchFixingWidget(QString diskName)
{
    if (diskName.length() == 0) {
        qDebug() << "执行修复grub时，传入分区名称为空";
        return ;
    }
    qDebug() << "执行修复grub时，传入分区名称: " << diskName;
    m_pCeterStackWidget->m_pStackWidget->setCurrentIndex(1);
    this->update();

    m_pBashFixProcess = new QProcess(0);
    connect(m_pBashFixProcess, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), this, &Widget::bashFixProcessFinishSlots);
    connect(m_pBashFixProcess, &QProcess::readyReadStandardOutput, this, [=]() {
        QString outputString = m_pBashFixProcess->readAllStandardOutput();
        qDebug() << "   ------------------执行脚本开始------------------    ";
        qDebug() << outputString;
        qDebug() << "   ------------------执行脚本结束------------------    ";
    });
    QString cmdString = "/bin/bash";

    QStringList argvList;
    argvList.clear();
    argvList.append("/usr/share/kylin-fix-boot-grub/fix-bash/fix-boot-bash");
    argvList.append(diskName);
    argvList.append("/");
    m_pBashFixProcess->start(cmdString, argvList);
}

void Widget::bashFixProcessFinishSlots(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::NormalExit && exitCode == 0) {
        qDebug() << "执行脚本成功";
        m_pCeterStackWidget->m_pStackWidget->setCurrentIndex(2);
        m_pSuccessWidget->m_pTime->start();
    } else {
        qDebug() << "执行脚本失败";
        m_pCeterStackWidget->m_pStackWidget->setCurrentIndex(3);
    }
    m_pRepairingWidget->m_pTime->stop();
    m_pBashFixProcess->deleteLater();
}

void Widget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QImage image(":/data/svg/backgrand.png");
    QPainter painter(this);
    painter.drawImage(0, 0, image.scaled(this->size()));
}
