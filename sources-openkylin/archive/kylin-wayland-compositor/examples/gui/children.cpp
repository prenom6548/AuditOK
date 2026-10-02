// SPDX-FileCopyrightText: 2025 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#include <QApplication>
#include <QCloseEvent>
#include <QLabel>
#include <QMainWindow>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QTextEdit>
#include <QVBoxLayout>

class SubWindow : public QMainWindow
{
    Q_OBJECT

  public:
    SubWindow(QWidget *parent = nullptr, int level = 1, int windowNumber = 1)
        : QMainWindow(parent), m_level(level), m_subWindowCount(0)
    {
        // 设置窗口属性
        setWindowTitle(QString("子窗口 - 第 %1 层第 %2 号").arg(level).arg(windowNumber));
        resize(400, 300);

        // 创建中央部件
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // 主布局
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // 窗口信息标签
        QLabel *infoLabel =
            new QLabel(QString("这是第 %1 层\n第 %2 号子窗口").arg(level).arg(windowNumber), this);
        infoLabel->setAlignment(Qt::AlignCenter);
        QFont font = infoLabel->font();
        font.setPointSize(16);
        infoLabel->setFont(font);

        // 文本编辑区
        QTextEdit *textEdit = new QTextEdit(this);
        textEdit->setPlainText(QString("窗口层级: %1\n窗口序号: %2").arg(level).arg(windowNumber));

        // 创建子窗口按钮
        QPushButton *createBtn = new QPushButton("创建子窗口", this);
        createBtn->setFixedHeight(40);
        connect(createBtn, &QPushButton::clicked, this, &SubWindow::createSubWindow);

        // 关闭窗口按钮
        QPushButton *closeBtn = new QPushButton("关闭窗口", this);
        closeBtn->setFixedHeight(40);
        connect(closeBtn, &QPushButton::clicked, this, &SubWindow::close);

        // 添加到布局
        mainLayout->addWidget(infoLabel);
        mainLayout->addWidget(textEdit);
        mainLayout->addWidget(createBtn);
        mainLayout->addWidget(closeBtn);

        // 状态栏显示
        statusBar()->showMessage(QString("已创建 0 个子窗口"), 2000);
    }

  signals:
    void windowClosed(); // 新增信号，窗口关闭时发出

  protected:
    void closeEvent(QCloseEvent *event) override
    {
        if (m_subWindowCount > 0) {
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(
                this, "确认关闭",
                QString("有 %1 个子窗口正在打开，确定要关闭吗？").arg(m_subWindowCount),
                QMessageBox::Yes | QMessageBox::No);
            if (reply == QMessageBox::No) {
                event->ignore();
                return;
            }
        }
        emit windowClosed(); // 发出窗口关闭信号
        event->accept();
    }

  private slots:
    void createSubWindow()
    {
        m_subWindowCount++;
        SubWindow *subWindow = new SubWindow(this, m_level + 1, m_subWindowCount);
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(subWindow, &SubWindow::windowClosed, this,
                &SubWindow::onSubWindowClosed); // 连接子窗口关闭信号
        subWindow->show();

        // 更新状态栏
        statusBar()->showMessage(QString("已创建 %1 个子窗口").arg(m_subWindowCount), 2000);
    }

    void onSubWindowClosed()
    {
        m_subWindowCount--; // 子窗口关闭，计数器减1
        statusBar()->showMessage(QString("已创建 %1 个子窗口").arg(m_subWindowCount), 2000);
    }

  private:
    int m_level;
    int m_subWindowCount;
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

  public:
    MainWindow(QWidget *parent = nullptr) : QMainWindow(parent), m_subWindowCount(0)
    {
        // 设置主窗口属性
        setWindowTitle("主窗口");
        resize(500, 400);

        // 创建中央部件
        QWidget *centralWidget = new QWidget(this);
        setCentralWidget(centralWidget);

        // 主布局
        QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

        // 标题标签
        QLabel *titleLabel = new QLabel("主窗口控制面板", this);
        titleLabel->setAlignment(Qt::AlignCenter);
        QFont titleFont = titleLabel->font();
        titleFont.setPointSize(20);
        titleLabel->setFont(titleFont);

        // 信息文本编辑区
        QTextEdit *infoText = new QTextEdit(this);
        infoText->setPlainText("使用下方按钮创建和管理子窗口\n"
                               "每个子窗口可以继续创建更多子窗口");
        infoText->setReadOnly(true);

        // 创建子窗口按钮
        QPushButton *createBtn = new QPushButton("创建子窗口", this);
        createBtn->setFixedHeight(50);
        connect(createBtn, &QPushButton::clicked, this, &MainWindow::createSubWindow);

        // 退出按钮
        QPushButton *exitBtn = new QPushButton("退出程序", this);
        exitBtn->setFixedHeight(50);
        connect(exitBtn, &QPushButton::clicked, this, &MainWindow::close);

        // 添加到布局
        mainLayout->addWidget(titleLabel);
        mainLayout->addWidget(infoText);
        mainLayout->addWidget(createBtn);
        mainLayout->addWidget(exitBtn);

        // 初始化状态栏
        statusBar()->showMessage("准备就绪", 2000);
    }

  private slots:
    void createSubWindow()
    {
        m_subWindowCount++;
        SubWindow *subWindow = new SubWindow(this, 1, m_subWindowCount);
        subWindow->setAttribute(Qt::WA_DeleteOnClose);
        connect(subWindow, &SubWindow::windowClosed, this,
                &MainWindow::onSubWindowClosed); // 连接子窗口关闭信号
        subWindow->show();

        // 更新状态栏
        statusBar()->showMessage(QString("已创建 %1 个子窗口").arg(m_subWindowCount), 2000);
    }

    void onSubWindowClosed()
    {
        m_subWindowCount--; // 子窗口关闭，计数器减1
        statusBar()->showMessage(QString("已创建 %1 个子窗口").arg(m_subWindowCount), 2000);
    }

  private:
    int m_subWindowCount;
};

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}

#include "children.moc"
