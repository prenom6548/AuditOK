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
#include "searchview.h"

SearchView::SearchView(QWidget *parent) : QWidget(parent)
{
    this->initInteraction();
    this->initWidget();
    this->initStyle();
    this->initConnect();
}

void SearchView::initWidget()
{
    m_stackedWidget = new QStackedWidget(this);

    m_collectCity = new CollectCity(this);
    m_vCollectLayout = new QVBoxLayout();

    m_searchWidget = new QWidget(this);
    m_searchLineEdit = new kdk::KSearchLineEdit(m_searchWidget);
    m_searchCityList = new SearchCityLIst(m_searchWidget);
    m_backLabel = new QLabel(m_searchWidget);
    m_hSearchLayout = new QHBoxLayout();
    m_vSearchLayout = new QVBoxLayout();

    m_stackedWidget->setAccessibleName("kylin-weather_QStackedWidget_SearchView_stackedWidget");
    m_searchWidget->setAccessibleName("kylin-weather_QWidget_SearchView_searchWidget");
    m_collectCity->setAccessibleName("kylin-weather_CollectCity");
    m_searchCityList->setAccessibleName("kylin-weather_SearchCityList");
    m_searchLineEdit->setAccessibleName("kylin-weather_KSearchLineEdit_search");
    m_backLabel->setAccessibleName("kylin-weather_QLabel_SearchView_backLabel");

    return;
}

void SearchView::initStyle()
{
    this->setFixedSize(ViewVar::SEARCH);

    m_searchLineEdit->setClearButtonEnabled(false);
    m_searchLineEdit->setFixedSize(ViewVar::SEARCH_BOX);

    m_backLabel->setText("<   " + tr("Back"));
    m_backLabel->setCursor(Qt::PointingHandCursor);
    m_backLabel->setAlignment(Qt::AlignLeft | Qt::AlignVCenter);
    m_backLabel->hide();

    m_hSearchLayout->addSpacing(10);
    m_hSearchLayout->addWidget(m_backLabel);
    m_hSearchLayout->addStretch();
    m_hSearchLayout->addWidget(m_searchLineEdit);
    m_hSearchLayout->addSpacing(190);

    m_vSearchLayout->addLayout(m_hSearchLayout);
    m_vSearchLayout->addWidget(m_searchCityList);
    m_vSearchLayout->addStretch();
    m_vSearchLayout->setContentsMargins(15, 0, 0, 0);
    m_searchWidget->setLayout(m_vSearchLayout);

    m_stackedWidget->addWidget(m_collectCity);
    m_stackedWidget->addWidget(m_searchWidget);
    m_stackedWidget->setCurrentIndex(0);

    m_vCollectLayout->addWidget(m_stackedWidget);
    this->setLayout(m_vCollectLayout);

    return;
}

void SearchView::initInteraction()
{
    connect(Interaction::getInstance(), &Interaction::searchUiShow, this, &SearchView::searchListUi);
}

void SearchView::initConnect()
{
    connect(m_searchLineEdit, &kdk::KSearchLineEdit::textChanged,    this, &SearchView::sendSearchInput);        //处理输入字符串，处理后返回给后端
    connect(m_searchCityList, &SearchCityLIst::addCollectCity,       this, &SearchView::addCollectCity);         //添加收藏城市
    connect(m_searchCityList, &SearchCityLIst::changeCityFromSearch, this, &SearchView::changeCityFromSearch);   //从搜索界面切换城市
    connect(m_searchCityList, &SearchCityLIst::cityListNull,         this, &SearchView::searchListUi);           //处理搜索为空界面显示
    connect(m_collectCity,    &CollectCity::changeCityFromCollect,   this, &SearchView::changeCityFromCollect);  //从收藏城市列表改变当前城市
    connect(m_collectCity,    &CollectCity::sigChangWidget,   this, &SearchView::changeStackedWidget);           // 切换搜索城市界面

    m_backLabel->installEventFilter(this);
}
//搜索界面各部分显隐和位置确定
void SearchView::searchListUi(bool hasCity)
{
    if (ViewVar::m_isTitleSearch) {
        return;
    }
    // if (hasCity) {
    //     m_collectCity->hide();
    //     m_searchCityList->move(ViewVar::SEARCHLIST_POS);
    // } else {
    //     m_collectCity->show();
    // }
    m_searchCityList->searchUiChange(hasCity);
    m_searchCityList->setViewModel(hasCity);
}

void SearchView::sendSearchInput()
{
    ViewVar::m_isTitleSearch = false;
    CoreVar::CURRENT_TITLE_SEARCH = false;
    QString inputText = m_searchLineEdit->text().trimmed().toLower();
    if (inputText != "") {
        inputText.remove(QRegExp("\\s")).remove("'");
        if (m_backInputText == inputText) {
            return;
        }
    } else {
        inputText = m_searchLineEdit->text();
    }
    m_backInputText = inputText;
    m_searchCityList->setViewModel(false);
    Interaction::getInstance()->searchResult(inputText);
    //城市搜索-埋点
    kdk::kabase::BuriedPoint buriedPointCitySearch;
    if (buriedPointCitySearch.functionBuriedPoint(kdk::kabase::AppName::KylinWeather,
                                                  kdk::kabase::BuriedPoint::PT::KylinWeatherCitySearch)) {
        qCritical() << "Error : buried point fail CitySearch!";
    };
}
//添加收藏城市
void SearchView::addCollectCity(QModelIndex index)
{
    Interaction::getInstance()->addCollectCity(index);

    m_searchLineEdit->clear();
    m_searchLineEdit->clearFocus();
    m_stackedWidget->setCurrentIndex(0);
    m_collectCity->changeEditBtnStyle();

    //城市收藏-埋点
    kdk::kabase::BuriedPoint buriedPointCollection;
    if (buriedPointCollection.functionBuriedPoint(kdk::kabase::AppName::KylinWeather,
                                                  kdk::kabase::BuriedPoint::PT::KylinWeatherCollection)) {
        qCritical() << "Error : buried point fail Collection!";
    };
}
//切换主界面城市-从搜索界面
void SearchView::changeCityFromSearch(QModelIndex index)
{
    emit searchWidState();
    Interaction::getInstance()->changeCityFromSearch(index);
}
//切换主界面城市-从收藏界面
void SearchView::changeCityFromCollect(QModelIndex index)
{
    changeUiShow();
    Interaction::getInstance()->changeCityFromCollect(index);
}
//切换城市--界面处理
void SearchView::changeUiShow()
{
    searchReset();
    emit searchWidState();
}
//搜索-收藏隐藏--界面处理
void SearchView::searchReset()
{
    if (m_searchCityList->isHidden()) {
        return;
    }
    m_searchCityList->hide();
    m_collectCity->show();
    // m_hotCity->show();
    m_searchLineEdit->clear();
}
//画圆角
void SearchView::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);
    QPainter painter(this);
    QStyleOption opt;
    opt.init(this);
    painter.setRenderHint(QPainter::Antialiasing); // 反锯齿;
    QColor color = qApp->palette().color(QPalette::Base);
    QBrush brush = QBrush(color);
    painter.setBrush(brush);
    painter.setPen(Qt::transparent);
    int radius = ViewVar::SAME_ITEMRADIUS;
    QRect rect = this->rect();
    rect.setWidth(rect.width());
    rect.setHeight(rect.height());
    painter.drawRoundedRect(rect, radius, radius);
}

bool SearchView::event(QEvent *event)
{
    //点击别处，使搜索框失去焦点
    if (event->type() == QEvent::MouseButtonPress) {
        qDebug()<<__LINE__<<__func__<<"fffffffffffffffffffffffffffffffffffffffffffffffffff";
        m_searchLineEdit->clearFocus();
    }

    return QWidget::event(event);
}

bool SearchView::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == m_backLabel && event->type() == QEvent::MouseButtonPress) {
        QMouseEvent *mouseEvent = static_cast<QMouseEvent*>(event);
        if (mouseEvent->button() == Qt::LeftButton) {
            onBackLabelClicked();
            return true;
        }
    }
    return QWidget::eventFilter(obj, event);
}

void SearchView::changeStackedWidget()
{
    if (CoreVar::CURRENT_COLLECT_LIST.size() == 17) {
        QMessageBox *msg = new QMessageBox((QWidget*)this->parent());
        msg->setAccessibleName("kylin-weather_QMessageBox_SearchView_msg");
        msg->setWindowTitle(tr(""));
        msg->setText(tr("Add favorite cities to reach the upper limit !"));
        msg->setIcon(QMessageBox::Information);
        QScreen *screen = QGuiApplication::primaryScreen();
        msg->exec();

    } else {
        m_stackedWidget->setCurrentIndex(1);
        m_backLabel->show();
    }

    return;
}

void SearchView::onBackLabelClicked()
{
    // 返回收藏城市界面
    m_stackedWidget->setCurrentIndex(0);
    m_backLabel->hide();  // 隐藏返回标签
    m_searchLineEdit->clear();
    m_searchLineEdit->clearFocus();

    // 如果收藏城市界面处于编辑状态，退出编辑状态
    if (ViewVar::m_editStyle) {
        m_collectCity->slotClickButton();
    }
}

void SearchView::hideEvent(QHideEvent *event)
{
    m_searchLineEdit->clear();
    m_searchLineEdit->clearFocus();
    m_stackedWidget->setCurrentIndex(0);
    m_backLabel->hide();
    if (ViewVar::m_editStyle) {
        m_collectCity->slotClickButton();
    }

    return;
}
void SearchView::keyPressEvent(QKeyEvent *event)
{
    if(event->modifiers() == Qt::ControlModifier&&event->key()==Qt::Key_E){
        qDebug()<<__LINE__<<__func__;
        m_searchLineEdit->setFocus();
    }
    return QWidget::keyPressEvent(event);
}
