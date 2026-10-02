#include "buildWidget.h"

buildWidget::buildWidget(QWidget *parent):
    kdk::KDialog(parent)
{
    this->setFixedSize(384,110);
    this->setWindowModality(Qt::ApplicationModal);
    this->closeButton()->setVisible(false);

    QWidget*mainwindow = mainWidget();
    m_buildLayout = new QVBoxLayout(mainwindow);
    m_buildMessage = new QLabel(mainwindow);
    m_buildMessage->setText("正在制作主题...");
    m_buildProgressBar = new QProgressBar(mainwindow);
    m_buildLayout->addWidget(m_buildMessage);
    m_buildLayout->addWidget(m_buildProgressBar);
    m_buildProgressBar->setMinimum(0);
    m_buildProgressBar->setMaximum(100);
    m_buildProgressBar->setValue(0);

    this->setLayout(m_buildLayout);

    build *thread = new build();
    connect(thread, &build::progressChanged, m_buildProgressBar, &QProgressBar::setValue);
    thread->start();

    connect(thread, &build::progressChanged, this, [=](){
        if(m_buildProgressBar->value()==100){
            this->close();
            //拷贝deb包至用户目录中
            QString sourceDirPath = QDir::homePath()+"/.cache/theme-build/"+FileProcess::g_date;
            QDir sourceDir = sourceDirPath;
            QDir homePath = FileProcess::g_debPath;

            QStringList debFiles = sourceDir.entryList(QStringList() << FileProcess::g_themeENName+"_*.deb", QDir::Files);
            // 遍历文件列表并复制文件到目标目录
            foreach (const QString& debFile, debFiles) {
                QString sourceFilePath = sourceDir.absoluteFilePath(debFile);
                QString homeFilePath = homePath.absoluteFilePath(debFile);

                // 创建源文件QFile对象
                QFile sourceFile(sourceFilePath);

                // 创建目标文件QFile对象
                QFile targetFile(homeFilePath);

                // 检查目标文件是否已存在
                if (targetFile.exists()) {
                    targetFile.remove();
                    qDebug() << "Target file already exists:" << debFile;
                }

                // 使用QFile的copy方法将源文件复制到目标文件
                if (!sourceFile.copy(homeFilePath)) {
                    qDebug() << "Failed to copy file:" << debFile;
                    continue;
                }

//                qDebug() << "File copied successfully:" << debFile;
            }
            //完成返回界面
            buildResultWidget *m_buildResult = new buildResultWidget();
            m_buildResult->show();

            connect(m_buildResult,&buildResultWidget::gohomesignals, this,[=](){
                //发送返回主界面信号buildcheckwidget
                emit gohomesignals();
            });
        }
    });
    connect(thread, &build::progressError, this, [=](QString errorinfo){
        QMessageBox*errorBox = new QMessageBox();
        errorBox->setText(errorinfo);
        errorBox->exec();
        this->deleteLater();
    });
}
