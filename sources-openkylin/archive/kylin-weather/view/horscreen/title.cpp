/*
 * Copyright (C) 2023, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */
#include "title.h"
#include "horscreen.h"

const static QSize ICON_SIZE_PC = QSize(32, 32);
const static QSize ICON_SIZE_PAD = QSize(48, 48);


title::title(QWidget *parent) : QWidget(parent)
{
    m_titleLayout = new QHBoxLayout(this);
    //左上角图标
    logoBtn = new QPushButton(this);
    logoBtn->setAccessibleName("kylin-weather_QPushButton_title_titleLogoBtn");
    logoBtn->setIcon(QIcon::fromTheme("kylin-weather", QIcon(":/res/contorl_icons/icon.png")));
    logoBtn->setIconSize(QSize(24, 24));
    logoBtn->setFocusPolicy(Qt::NoFocus);
    //左上角文字
    m_label = new QLabel(this);
    m_label->setAccessibleName("kylin-weather_QLabel_title_titleLabel");
    m_label->setText(tr("Weather"));
    m_label->setStyleSheet("color: white");
    m_label->setAlignment(Qt::AlignVCenter);
    QPalette palText = m_label->palette();
    palText.setColor(QPalette::WindowText, Qt::white);
    m_label->setPalette(palText);

    // 搜索框
    m_searchLineEdit = new KSearchLineEdit();
    m_searchLineEdit->setAccessibleName("kylin-weather_KSearchLineEdit_title_searchLineEdit");
    m_searchLineEdit->setPlaceholderText(tr("Search City/Region"));
    m_searchLineEdit->setCompleter(nullptr);
    m_searchLineEdit->setClearButtonEnabled(false);
    m_searchLineEdit->clearFocus();
    m_searchLineEdit->setEnabled(false);
    m_searchLineEdit->setTranslucent(true);
//    m_searchLineEdit->setContextMenuPolicy(Qt::NoContextMenu);
    this->setFocusPolicy(Qt::ClickFocus);

    //窗口四联--菜单
    g_menu = new menuModule(this);
    g_menu->setAccessibleName("kylin-weather_menuModule_title_Menu");
    g_menu->setFocusPolicy(Qt::NoFocus);
    g_menu->setAttribute(Qt::WA_TranslucentBackground);

    //刷新按钮
    btnRefresh = new QPushButton(this);
    btnRefresh->setAccessibleName("kylin-weather_QPushButton_title_btnRefresh");
    btnRefresh->setToolTip(tr("Refresh"));
    btnRefresh->setIcon(QIcon::fromTheme("view-refresh-symbolic"));
    btnRefresh->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    btnRefresh->setProperty("isWindowButton", 0x1);
    btnRefresh->setProperty("useIconHighlightEffect", 0x2);
    btnRefresh->setFlat(true);
    btnRefresh->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));

    //窗口四联--最小化按钮
    btnMinimize = new QPushButton(this);
    btnMinimize->setAccessibleName("kylin-weather_QPushButton_title_BtnMinimize");
    btnMinimize->setToolTip(tr("Minimize"));
    btnMinimize->setFocusPolicy(Qt::NoFocus);                           //设置焦点类型
    btnMinimize->setIcon(QIcon::fromTheme("window-minimize-symbolic")); //主题库的最小化图标
    btnMinimize->setProperty("isWindowButton", 0x1);
    btnMinimize->setProperty("useIconHighlightEffect", 0x2);
    btnMinimize->setFlat(true);
    btnMinimize->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));

    //窗口四联--最大化/还原按钮
    //全屏按钮注释代码
    btnMaximum = new QPushButton(this);
    btnMaximum->setAccessibleName("kylin-weather_QPushButton_title_BtnMaximum");
    btnMaximum->setIcon(QIcon::fromTheme("window-maximize-symbolic"));
    btnMaximum->setToolTip(tr("full screen"));
    btnMaximum->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    btnMaximum->setProperty("isWindowButton", 0x1);
    btnMaximum->setProperty("useIconHighlightEffect", 0x2);
    btnMaximum->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    btnMaximum->setFlat(true);
    btnMaximum->hide();

    //窗口四联--关闭按钮
    btnCancel = new QPushButton(this);
    btnCancel->setAccessibleName("kylin-weather_QPushButton_title_BtnCancel");
    btnCancel->setToolTip(tr("Close"));
    btnCancel->setIcon(QIcon::fromTheme("window-close-symbolic"));
    btnCancel->setFocusPolicy(Qt::NoFocus); //设置焦点类型
    btnCancel->setProperty("isWindowButton", 0x2);
    btnCancel->setProperty("useIconHighlightEffect", 0x8);
    btnCancel->setFlat(true);
    btnCancel->setProperty("setIconHighlightEffectDefaultColor", QColor(Qt::white));
    switch (ViewVar::getInstance()->m_currentMode)
    {
        case CurrentMode::PCMode:
        slotChangePCMode();        
            break;
        case CurrentMode::PadHMode:
        slotChangeHScreenMode();
            break;
        case CurrentMode::PadVMode:
        slotChangeVScreenMode();
            break;
    }

    this->initConnection();
    this->initLayout();
    this->initQss();
    this->installEventFilter(this);
    m_searchLineEdit->installEventFilter(this);
}



void title::initConnection()
{
    //    connect(btnMinimize, &QPushButton::clicked, horscreen::mutual, &horscreen::showMinimized);
    connect(btnMinimize, &QPushButton::clicked, [=] {
        horscreen::mutual->showMinimized();
        ViewVar::widgetIsShow = false;
    });
    //    connect(btnMaximum,&QPushButton::clicked,horscreen::mutual,&horscreen::showMaximized);
    //    connect(btnCancel, &QPushButton::clicked, horscreen::mutual, &horscreen::hide);
    connect(btnCancel, &QPushButton::clicked, [=] {
        horscreen::mutual->hide();
        ViewVar::widgetIsShow = false;
    });
    connect(btnRefresh, &QPushButton::clicked, this, &title::sigRefreshWeather);
    //全屏按钮注释代码
    connect(btnMaximum, &QPushButton::clicked, this, &title::recovery);

    // 首次允许定位弹窗点击“是”更改菜单内允许定位为“是”
    connect(this, &title::setMenuModuleAllowLocationYes, g_menu, &menuModule::setMenuModuleAllowLocationYes);
    connect(m_searchLineEdit, &KSearchLineEdit::textChanged, this, &title::sendSearchInput);

    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPc, this, &title::slotChangePCMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadH, this, &title::slotChangeHScreenMode);
    connect(ViewVar::getInstance(), &ViewVar::sigChangeModeToPadV, this, &title::slotChangeVScreenMode);

}

void title::initLayout()
{
    m_titleLayout->setMargin(0);
    m_titleLayout->setSpacing(0);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(logoBtn);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(m_label, 0, Qt::AlignVCenter);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(m_searchLineEdit);
    m_titleLayout->addStretch();
    m_titleLayout->addWidget(btnRefresh);
    m_titleLayout->addSpacing(8);
    m_titleLayout->addWidget(g_menu->menuButton);
    m_titleLayout->addSpacing(2);
    m_titleLayout->addWidget(btnMinimize);
    m_titleLayout->addSpacing(2);
    if (ViewVar::platFormList.contains(ViewVar::platForm)) {
        btnMaximum->show();
        //全屏按钮注释代码
        m_titleLayout->addWidget(btnMaximum);
        m_titleLayout->addSpacing(2);
    } else {
        btnMaximum->hide();
    }
    m_titleLayout->addWidget(btnCancel);
    m_titleLayout->addSpacing(4);
    this->setLayout(m_titleLayout);
}

void title::initQss()
{
    logoBtn->setStyleSheet("QPushButton{border:0px;border-radius:4px;background:transparent;}"
                           "QPushButton::hover{border:0px;border-radius:4px;background:transparent;}"
                           "QPushButton::pressed{border:0px;border-radius:4px;background:transparent;}");
}


void title::sendSearchInput()
{
    ViewVar::m_isTitleSearch = true;
    CoreVar::CURRENT_TITLE_SEARCH = true;
    QString inputText = m_searchLineEdit->text().trimmed().toLower();
    if (inputText != "") {
        inputText.remove(QRegExp("\\s")).remove("'");
        if (m_searchText == inputText) {
            return;
        }
    } else {
        inputText = m_searchLineEdit->text();
    }
    m_searchText = inputText;
    emit sigTitleSearchCity();

    Interaction::getInstance()->searchResult(inputText);
    //城市搜索-埋点
    kdk::kabase::BuriedPoint buriedPointCitySearch;
    if (buriedPointCitySearch.functionBuriedPoint(kdk::kabase::AppName::KylinWeather,
                                                  kdk::kabase::BuriedPoint::PT::KylinWeatherCitySearch)) {
        qCritical() << "Error : buried point fail CitySearch!";
    };
}

void title::slotSearchLineEditClear()
{
    ViewVar::m_isTitleSearch = false;
    CoreVar::CURRENT_TITLE_SEARCH = false;
    m_searchLineEdit->clear();
    m_searchLineEdit->clearFocus();
    return;
}

void title::slotChangeLineEditStyle()
{
    m_searchLineEdit->setEnabled(true);
    return;
}

int title::getSearchLinePositionY()
{
    QPoint globalPox = m_searchLineEdit->mapToGlobal(QPoint(0, 0));
    int y = m_searchLineEdit->height() + globalPox.y() + 3;
    return y;
}

int title::getSearchLinePositionX()
{
    QPoint globalPox = m_searchLineEdit->mapToGlobal(QPoint(0, 0));
    int x = globalPox.x();
    return x;
}

int title::getSearchLineWidth()
{
    return m_searchLineEdit->width();
}

bool title::eventFilter(QObject *watch, QEvent *e)
{
    if (e->type() == QEvent::FocusIn && watch == m_searchLineEdit) {
        emit sigLineEditGetFocus();
    }
    return QObject::eventFilter(watch, e);
}

void title::slotChangeHScreenMode()
{
    this->setFixedHeight(64);
    logoBtn->setFixedSize(ICON_SIZE_PAD);
    m_searchLineEdit->setFixedSize(300, 48);
    btnMinimize->setFixedSize(ICON_SIZE_PAD);
    btnMaximum->setFixedSize(ICON_SIZE_PAD);
    btnCancel->setFixedSize(ICON_SIZE_PAD);
    btnRefresh->setFixedSize(ICON_SIZE_PAD);
    g_menu->menuButton->setFixedSize(ICON_SIZE_PAD);
    return;
}

void title::slotChangeVScreenMode()
{
    this->setFixedHeight(64);
    logoBtn->setFixedSize(ICON_SIZE_PAD);
    m_searchLineEdit->setFixedSize(300, 48);
    btnMinimize->setFixedSize(ICON_SIZE_PAD);
    btnMaximum->setFixedSize(ICON_SIZE_PAD);
    btnCancel->setFixedSize(ICON_SIZE_PAD);
    btnRefresh->setFixedSize(ICON_SIZE_PAD);
    g_menu->menuButton->setFixedSize(ICON_SIZE_PAD);
    return;
}

void title::slotChangePCMode()
{
    this->setFixedHeight(40);
    logoBtn->setFixedSize(24, 24);
    m_searchLineEdit->setFixedSize(240, 32);
    btnMinimize->setFixedSize(ICON_SIZE_PC);
    btnMaximum->setFixedSize(ICON_SIZE_PC);
    btnCancel->setFixedSize(ICON_SIZE_PC);
    btnRefresh->setFixedSize(ICON_SIZE_PC);
    g_menu->menuButton->setFixedSize(ICON_SIZE_PC);
    return;
}
void title::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        this->m_searchLineEdit->setFocus();
    }
    return QWidget::keyPressEvent(event);
}
