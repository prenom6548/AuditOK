/*
 * Copyright (C) 2022, KylinSoft Co., Ltd.
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: LiKe <like1@kylinos.cn>
 *
 */
#include "historywidget.h"

/**
 * @brief 历史记录窗口
 *
 * 历史记录窗口类的构造函数，用于初始化历史记录窗口。
 *
 * @param parent 父窗口指针
 */
HistoryWidget::HistoryWidget(QWidget *parent) : QWidget(parent)
{
    QVBoxLayout *alllayout = new QVBoxLayout();

    QHBoxLayout *linelayout = new QHBoxLayout();

    QPushButton *line1 = new QPushButton();
    line1->setDisabled(true);
    line1->setFixedHeight(1);
    line1->setFixedWidth(1040);
    linelayout->addWidget(line1);

    QWidget *textWidget = new QWidget();
    QHBoxLayout *textlayout = new QHBoxLayout();
    QLabel *historyLabel = new QLabel(textWidget);
    historyLabel->setText(tr("Historical record"));
    QLabel *tipsLabel = new QLabel(textWidget);
    tipsLabel->setText(tr("Records are kept for 30 days only"));

    textlayout->addWidget(historyLabel);
    textlayout->addStretch(1);
    textlayout->addWidget(tipsLabel);
    textWidget->setLayout(textlayout);
    alllayout->addLayout(linelayout);
    alllayout->addWidget(textWidget);
    QDir builderDir = QDir(QDir::homePath() + "/.cache/theme-build/");
    QStringList folders = builderDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QList<QString> validFolders;

    QDateTime currentDate = QDateTime::currentDateTime();

    foreach (QString folder, folders) {
        QString yearStr = folder.mid(0, 4);
        QString monthStr = folder.mid(4, 2);
        QString dayStr = folder.mid(6, 2);
        QString hourStr = folder.mid(8, 2);
        QString minuteStr = folder.mid(10, 2);
        QString secondStr = folder.mid(12, 2);

        QDateTime folderDateTime = QDateTime::fromString(yearStr + monthStr + dayStr + hourStr + minuteStr + secondStr, "yyyyMMddhhmmss");

        if (folderDateTime.isValid() && folderDateTime.daysTo(currentDate) <= 30) {
//            qDebug() << "Processing folder: " << folder;
            validFolders.prepend(folder);
        }
    }
    QGridLayout *layout = new QGridLayout;
    layout->setSpacing(24);
    int row = 1;
    int col = 1;

    layout->setColumnStretch(0, 1);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(5, 1);
    layout->setRowStretch(5, 1);
    foreach (QString folder, validFolders) {
        HistoryButton *button = new HistoryButton(this);
        HistoryInfoLoad *info = new HistoryInfoLoad(folder);

        button->setHistoryTime(folder);
        button->setCover(info->getCover());
        button->setThemeName(info->getThemeName());
        button->setThemeType(info->getThemeType());
        button->initUI();
        connect(button,&HistoryButton::clicked,this,[=](){

            FileProcess::g_date = folder;            
            FileProcess::g_themeCNName = info->getThemeCNName();
            FileProcess::g_themeENName = info->getThemeName();
            FileProcess::g_builderName = info->getThemeBuilder();
            FileProcess::g_builderMailName = info->getThemeMail();
            FileProcess::g_createThemeType = info->getThemeType();
            emit startSecondEdit(info->getInfoData());
        });
        layout->addWidget(button, row, col);
        col++;
        if (col == 5) {
            col = 1;
            row++;
        }
        connect(this,&HistoryWidget::updateInfo,info,&HistoryInfoLoad::updateHistoryInfo);
    }

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);
    alllayout->addWidget(widget);
    alllayout->addStretch(1);
    this->setLayout(alllayout);
}

/**
 * @brief 历史记录窗口
 *
 * 历史记录窗口类的构造函数，用于初始化历史记录窗口。
 *
 * @param parent 父窗口指针
 */
void HistoryWidget::updateHistoryDir()
{
    QDir builderDir = QDir(QDir::homePath() + "/.cache/theme-build/");
    QStringList folders = builderDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    QList<QString> validFolders;

    QDateTime currentDate = QDateTime::currentDateTime();

    foreach (QString folder, folders) {
        QString yearStr = folder.mid(0, 4);
        QString monthStr = folder.mid(4, 2);
        QString dayStr = folder.mid(6, 2);
        QString hourStr = folder.mid(8, 2);
        QString minuteStr = folder.mid(10, 2);
        QString secondStr = folder.mid(12, 2);

        QDateTime folderDateTime = QDateTime::fromString(yearStr + monthStr + dayStr + hourStr + minuteStr + secondStr, "yyyyMMddhhmmss");

        if (folderDateTime.isValid() && folderDateTime.daysTo(currentDate) <= 30) {
            validFolders.prepend(folder);
        }
    }

    QGridLayout *layout = new QGridLayout;
    layout->setSpacing(24);
    int row = 0;
    int col = 1;
    layout->setColumnStretch(0, 1);
    layout->setRowStretch(0, 1);
    layout->setColumnStretch(5, 1);
    layout->setRowStretch(5, 1);

    foreach (QString folder, validFolders) {
        HistoryButton *button = new HistoryButton(this);
        HistoryInfoLoad *info = new HistoryInfoLoad(folder);

        button->setHistoryTime(folder);
        button->setCover(info->getCover());
        button->setThemeName(info->getThemeName());
        button->setThemeType(info->getThemeType());
        button->initUI();
        connect(button,&HistoryButton::clicked,this,[=](){

            FileProcess::g_date = folder;
            FileProcess::g_themeCNName = info->getThemeCNName();
            FileProcess::g_themeENName = info->getThemeName();
            FileProcess::g_builderName = info->getThemeBuilder();
            FileProcess::g_builderMailName = info->getThemeMail();
            FileProcess::g_createThemeType = info->getThemeType();
            emit startSecondEdit(info->getInfoData());
        });
        layout->addWidget(button, row, col);

        col++;
        if (col == 5) {
            col = 1;
            row++;
        }
    }

    QWidget *widget = new QWidget(this);
    widget->setLayout(layout);

    QVBoxLayout *alllayout = qobject_cast<QVBoxLayout*>(this->layout());
    if (alllayout) {
        // 删除原来的widget
        while (QWidget* w = alllayout->takeAt(2)->widget()) {  // 从索引2开始，删除了所有的widget,保留提示和分隔线
            delete w;
        }
        alllayout->addWidget(widget);
        alllayout->addStretch(1);
    }
}

/**
 * @brief 历史按钮构造函数
 *
 * 创建一个历史按钮的构造函数。
 *
 * @param parent 父窗口指针
 */
HistoryButton::HistoryButton(QWidget* parent): QPushButton(parent)
{
//    this->setBackgroundRole(QPalette::Base);
    this->setFixedSize(240, 280);
    m_menu = new QMenu(this);
    this->installEventFilter(this);
}

void HistoryButton::setHistoryTime(const QString &text)
{
    m_historytime = text;
    QDateTime dateTime = QDateTime::fromString(text, "yyyyMMddhhmmss");
    m_showhistorytime = dateTime.toString("yyyy/MM/dd/hh:mm:ss");
}

void HistoryButton::setCover(const QString & covePath)
{
    m_coverpath = covePath;
}

void HistoryButton::setThemeCNName(const QString &themeCNName)
{
    m_themecnname = themeCNName;
}


void HistoryButton::setThemeName(const QString &themeName)
{
    m_themename = themeName;
}

void HistoryButton::setThemeBuilder(const QString &themeBuilder)
{
    m_themebuilder = themeBuilder;
}

void HistoryButton::setThemeMail(const QString &thememail)
{
    m_thememail = thememail;
}

void HistoryButton::setThemeType(const QString &themeType)
{
    m_themetype = themeType;
}

/**
 * @brief 初始化 UI
 *
 * 初始化 HistoryButton 的 UI。
 */
void HistoryButton::initUI()
{
    this->setProperty("useButtonPalette", true);
    QVBoxLayout* layout = new QVBoxLayout(this);

    QLabel* imageLabel = new QLabel;
    imageLabel->setAlignment(Qt::AlignCenter);

    layout->addWidget(imageLabel);

    QString imagePath = nullptr;
    if(nullptr == m_coverpath){
        imagePath = ":/resource/background/custom-preview.png";
    }else{
        imagePath = m_coverpath;
    };

    QImage image(imagePath);
    if (!image.isNull())
    {
        QImage scaledImage = image.scaled(QSize(208, 137), Qt::KeepAspectRatio);
        imageLabel->setPixmap(QPixmap::fromImage(scaledImage));
    }
    QLabel* label1 = new QLabel();
    if(m_themename != nullptr){
        label1->setText(m_themename);
    }else{
        label1->setText("未命名");
    }
    layout->addWidget(label1);

    QLabel* label2 = new QLabel(m_showhistorytime);
    layout->addWidget(label2);

    QWidget * btnwidget = new QWidget(this);
    QHBoxLayout * btnwidgetlayout = new QHBoxLayout(btnwidget);
    btnwidgetlayout->setContentsMargins(0,0,0,0);

    QString buttonText;
    if (m_themetype == "globalTheme") {
        buttonText = tr("globalTheme");
    } else if (m_themetype == "iconTheme") {
        buttonText = tr("iconTheme");
    } else if (m_themetype == "cursorTheme") {
        buttonText = tr("cursorTheme");
    } else if (m_themetype == "plymouthTheme") {
        buttonText = tr("plymouthTheme");
    } else {
        buttonText = tr("grubTheme");
    }
    m_typebutton = new QPushButton(buttonText, this);
    m_typebutton->setProperty("useButtonPalette", true);
    m_typebutton->setProperty("setRadius", 18);

    m_menubutton = new QToolButton(this);
    m_menubutton->setProperty("useButtonPalette", true);
    m_menubutton->setFixedWidth(50);
    m_menubutton->setText("···");
    m_menubutton->setPopupMode(QToolButton::InstantPopup);

    btnwidgetlayout->addWidget(m_typebutton);
    btnwidgetlayout->addStretch(1);
    btnwidgetlayout->addWidget(m_menubutton);
    btnwidget->setLayout(btnwidgetlayout);
    layout->addWidget(btnwidget);

    this->setLayout(layout);
    // 创建并添加菜单选项
    QAction* action1 = m_menu->addAction("删除");
    QAction* action2 = m_menu->addAction("导出");
    m_menubutton->setMenu(m_menu);
    if(m_themename != nullptr){
        action2->setDisabled(false);
    }else{
        action2->setDisabled(true);
    }
//    connect(btn_menu, &QPushButton::clicked, this, [=]() {
//        m_menu->exec(btn_menu->mapToGlobal(btn_menu->rect().bottomLeft()));
//    });
//    connect(action2,&QAction::triggered,this,[=](){
//       emit  startBuild(m_historytime);
//    });
    // 连接菜单选项的槽函数
    connect(action1, &QAction::triggered, this, [=]() {
        DirOperation *dop = new DirOperation();
        dop->DeleteHistoryDir(m_historytime);
        this->deleteLater();
//        emit updateHistoryRequested();
        this->parentWidget()->layout()->update();
    });

    connect(action2, &QAction::triggered, this, [=](){
        FileProcess::g_date = m_historytime;
        FileProcess::g_themeCNName = m_themecnname;
        FileProcess::g_themeENName = m_themename;
        FileProcess::g_builderName = m_themebuilder;
        FileProcess::g_builderMailName = m_thememail;
        FileProcess::g_createThemeType = m_themetype;

        InfoCreateWidget *m_infoWidget =new InfoCreateWidget(nullptr,nullptr);
        m_infoWidget->createControlChangelog(m_historytime);
        m_infoWidget->createMeson(m_historytime);

        //编包前检查
        buildCheckWidget *m_buildCheckWidget = new buildCheckWidget();
        m_buildCheckWidget->buildCheck(QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src");

        connect(m_buildCheckWidget, &buildCheckWidget::gohomesignals, this,[=](){
//            DirOperation *dop = new DirOperation();
//            dop->DeleteHistoryDir(m_historytime);
//            this->deleteLater();
//            this->parentWidget()->layout()->update();
            FileProcess::g_date = nullptr;
            FileProcess::g_themeCNName = nullptr;
            FileProcess::g_themeENName = nullptr;
            FileProcess::g_builderName = nullptr;
            FileProcess::g_builderMailName = nullptr;
            FileProcess::g_createThemeType = nullptr;
        });
    });
    //FIX ME:现在是三个button，当historybutton hover的时候，button和btn_menu没有悬浮，导致有色差，用qss的话一样会出现这个问题。暂时没有好的解决方式
    connect(this, &HistoryButton::hoverStateChanged, this, [&](bool ishover) {
        if(ishover){
            m_typebutton->setAttribute(Qt::WA_UnderMouse, true);
            m_menubutton->setAttribute(Qt::WA_UnderMouse, true);
        }else{
            m_typebutton->setAttribute(Qt::WA_UnderMouse, false);
            m_menubutton->setAttribute(Qt::WA_UnderMouse, false);
        }
    });
}

bool HistoryButton::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == this) {
        if (event->type() == QEvent::Enter) {
            emit hoverStateChanged(true);
        } else if (event->type() == QEvent::Leave) {
            emit hoverStateChanged(false);
        }
    }
    return QPushButton::eventFilter(obj, event);
}


void HistoryWidget::handleButtonClick()
{

}
