#include "uiservicesupport.h"
#include <QTranslator>
#include <QApplication>
#include <QLibraryInfo>
#include <QStackedWidget>
#include <QBoxLayout>
#include <ktabbar.h>
#include <QScrollArea>
#include <QDebug>
#include "uiproblemfeedback.h"
#include "uiselfservice.h"
#include "uihistoryfeedback.h"
#include "kom_buriedpoint.h"
#include "../constant.h"

UiServiceSupport::UiServiceSupport(QWidget *parent) : QWidget(parent)
{
    //加载翻译
    translations();
    setFixedSize(824, 600);
    //标签栏
    m_mainTabBar = new kdk::KTabBar(kdk::SegmentLight, this);
    m_mainTabBar->hide();
    m_mainTabBar->addTab(tr("Feedback"));
    m_mainTabBar->addTab(tr("Self service"));
    if (Settings::isUpload()) {
        m_mainTabBar->addTab(tr("History"));
    }
    m_mainTabBar->setFixedSize(400, 36);
    QHBoxLayout *hlTabBar = new QHBoxLayout;
    hlTabBar->setMargin(0);
    hlTabBar->addStretch(9);
    hlTabBar->addWidget(m_mainTabBar);
    hlTabBar->addStretch(9);
    //内容区域
    m_stackWidget = new QStackedWidget(this);
    auto *tab1 = new UiProblemFeedback(this);
    QScrollArea *scrollArea = new QScrollArea(this);
    scrollArea->setObjectName(NAME_PROBLEM_FEEDBACK);
    scrollArea->setFrameShape(QScrollArea::NoFrame);
    scrollArea->setWidget(tab1);
    connect(this, &UiServiceSupport::indexChanged, tab1, &UiProblemFeedback::indexChanged);
    m_stackWidget->addWidget(scrollArea);
    auto *tab3 = new UiSelfService(this);
    m_stackWidget->addWidget(tab3);
    if (Settings::isUpload()) {
        auto *tab4 = new UiHistoryFeedback(this);
        m_stackWidget->addWidget(tab4);
        connect(this, &UiServiceSupport::indexChanged, tab4, &UiHistoryFeedback::indexChanged);
    }

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->setMargin(0);
    vl->setSpacing(0);
    vl->addSpacing(16);
    vl->addLayout(hlTabBar);
    vl->addWidget(m_stackWidget);

    //    connect(m_mainTabBar, &kdk::KTabBar::currentChanged, this, &UiServiceSupport::itemIndexChange);
}

void UiServiceSupport::itemIndexChange(int index)
{
    m_stackWidget->setCurrentIndex(index);
    kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupport,
                                    kom::BuriedPoint::EventCode::kSwitchTab,
                                    {{"tabName", m_stackWidget->currentWidget()->objectName()}});
    emit indexChanged(m_stackWidget->currentWidget()->objectName());
}

void UiServiceSupport::itemIndexChangeFromString(const QString &str)
{
    for (int i = 0; i < m_stackWidget->count(); i++) {
        QString objName = m_stackWidget->widget(i)->objectName();
        if (str == objName) {
            m_stackWidget->setCurrentIndex(i);
            kom::BuriedPoint::uploadMessage(kom::BuriedPoint::BuriedPointPage::kServiceSupport,
                                            kom::BuriedPoint::EventCode::kSwitchTab,
                                            {{"tabName", m_stackWidget->currentWidget()->objectName()}});
            m_mainTabBar->setCurrentIndex(i);
            emit indexChanged(objName);
            return;
        }
    }
}

void UiServiceSupport::translations()
{
    QLocale locale;
    // 加载应用翻译文件
    QString projectName = "kom-service-support";
    QString localeFileCrush = "translations";
    QString kylinFileCrush = "/usr/share/kylin-os-manager/translations/";
    QTranslator *trans_global = new QTranslator(this);
    if (trans_global->load(locale, projectName, "_", localeFileCrush)) {
        QApplication::installTranslator(trans_global);
    } else {
        if (!trans_global->load(QLocale(), projectName, "_", kylinFileCrush)) {
            qWarning() << "main Load global translations file" << QLocale() << "failed!";
        } else {
            QApplication::installTranslator(trans_global);
        }
    }
    // 加载Qt翻译文件
    QTranslator *trans_qt = new QTranslator(this);
    QString qtTransPath = QLibraryInfo::location(QLibraryInfo::TranslationsPath);
    if (!trans_qt->load(locale, "qt", "_", qtTransPath)) {
        qWarning() << "main Load qt translations file" << QLocale() << "failed!";
    } else {
        QApplication::installTranslator(trans_qt);
    }
    QTranslator *trans_qtwebengine = new QTranslator(this);
    if (!trans_qtwebengine->load(locale, "qtwebengine", "_", qtTransPath)) {
        qWarning() << "main Load qtwebengine translations file" << QLocale() << "failed!";
    } else {
        QApplication::installTranslator(trans_qtwebengine);
    }
    // 加载sdk控件翻译
    QTranslator *trans = new QTranslator(this);
    if (trans->load(":/translations/gui_" + locale.name() + ".qm")) {
        QApplication::installTranslator(trans);
    }
}
