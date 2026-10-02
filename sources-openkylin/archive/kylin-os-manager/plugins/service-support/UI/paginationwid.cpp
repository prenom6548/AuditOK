#include "paginationwid.h"

#include <QDebug>
// PaginationWid::PaginationWid(QWidget *parent) : QWidget(parent) {}

PaginationWid::PaginationWid(QWidget *parent, int totalPage)
    : QWidget(parent), m_totalPage(totalPage), m_currentPage(MINIMUM_PAGE), m_maxPage(MINIMUM_PAGE), m_displayPage(0)
{
    initMainWid();
    m_totalText = tr("total");
    m_pagesText = tr("pages");
}

void PaginationWid::updatePageNum(int totalPage)
{
    m_pageNum->setText(m_totalText + " " + QString::number(totalPage) + " " + m_pagesText);
}

void PaginationWid::initMainWid()
{
    m_nextBtn = new QToolButton(this);
    m_nextBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_nextBtn->setIcon(QIcon::fromTheme("ukui-end.symbolic"));
    m_nextBtn->setProperty("useButtonPalette", true);
    m_backBtn = new QToolButton(this);
    m_backBtn->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_backBtn->setIcon(QIcon::fromTheme("ukui-start.symbolic"));
    m_backBtn->setProperty("useButtonPalette", true);

    m_vecPageBtn.reserve(10);
    m_pGotoItem = new GotoPageItem(this);
    //    m_pItemEachPage = new EachPageItem(this);
    //    m_pItemEachPage->hide();
    m_btnWid = new QWidget(this);
    m_btnWid->setFixedHeight(40);
    //    updateBtnGroups(10);
    m_pageNum = new QLabel(this);

    m_jumpLabel = new QLabel(this);
    m_jumpLabel->setText(tr("Jump to"));
    m_jumpLabel->adjustSize();
    m_jumpPage = new QLabel(this);
    m_jumpPage->setText(tr("page"));
    m_pageLine = new QLineEdit(this);
    m_pageLine->setText(QString::number(m_totalItems));
    m_pageLine->setFixedWidth(72);
    m_pageLine->setAlignment(Qt::AlignCenter);

    m_jumpLabel->hide();
    m_jumpPage->hide();
    m_pageLine->hide();
    m_btnGroup = new QButtonGroup(this);

    m_layout = new QHBoxLayout(this);

    m_pageWidLayout = new QHBoxLayout();
    m_pageWidLayout->setContentsMargins(0, 0, 0, 0);
    m_btnWid->setLayout(m_pageWidLayout);
    m_currentPage = 1;


    m_btnGroup->setExclusive(true);
    m_layout->setContentsMargins(0, 0, 0, 0);

    m_layout->setSpacing(0);
    m_layout->addStretch();
    m_layout->addWidget(m_backBtn);
    m_layout->addSpacing(8);
    m_layout->addWidget(m_btnWid);
    m_layout->addSpacing(8);
    m_layout->addWidget(m_nextBtn);
    m_layout->addSpacing(16);
    m_layout->addWidget(m_pageNum);
    m_layout->addSpacing(16);
    m_layout->addWidget(m_pGotoItem);
    m_layout->addSpacing(0);
    this->setLayout(m_layout);


    // 上一页
    connect(m_backBtn, &QToolButton::clicked, this, [this]() {
        if (m_currentPage - 1 < 0) {
            m_currentPage = 1;
        } else {
            --m_currentPage;
        }
        changeView(m_currentPage);
        Q_EMIT pageChanged(m_currentPage);
    });

    // 下一页
    connect(m_nextBtn, &QToolButton::clicked, this, [&]() {
        if (m_currentPage + 1 > m_maxPage) {
            m_currentPage = m_maxPage;
        } else {
            ++m_currentPage;
        }
        changeView(m_currentPage);
        Q_EMIT pageChanged(m_currentPage);
    });


    // 前往第N页
    connect(m_pGotoItem, &GotoPageItem::GotoPage, this, [&](int nPage) {
        if (nPage > m_maxPage) {
            nPage = m_maxPage;
        }
        if (nPage < MINIMUM_PAGE) {
            nPage = MINIMUM_PAGE;
        }
        if (nPage + 1 > m_maxPage) {
            m_currentPage = m_maxPage;
        } else {
            m_currentPage = nPage;
        }
        changeView(m_currentPage);
        Q_EMIT pageChanged(m_currentPage);
    });

    //    // 每页条码变化
    //    connect(m_pItemEachPage, &EachPageItem::EachPageItemChanged, this, [&](int nCount) {
    //        int EachPage = m_pItemEachPage->GetCurrent();
    //        if (EachPage > 0) {

    //            m_maxPage = m_totalItems / EachPage;
    //            if (m_totalItems % EachPage > 0)
    //                ++m_maxPage;
    //            if (m_maxPage <= 0)
    //                m_maxPage = MINIMUM_PAGE;
    //            // 如果m_maxPage小于MAX_BUTTON_COUNT，则不变。

    //            // 如果m_maxPage大于MAX_BUTTON_COUNT，则只构建7个按钮，头、尾，中间5个
    //            if (m_maxPage > MAX_BUTTON_COUNT)
    //                m_displayPage = MAX_BUTTON_COUNT + 1;
    //            else
    //                m_displayPage = m_maxPage;
    //        }
    //        reload();
    //        Q_EMIT numberPerpageChanged(nCount);
    //    });
}

void PaginationWid::setItemPerPage()
{
    int EachPage = MAX_PAGE;
    if (EachPage > 0) {
        m_maxPage = m_totalItems / EachPage;
        if (m_totalItems % EachPage > 0) {
            ++m_maxPage;
        }
        if (m_maxPage <= 0) {
            m_maxPage = MINIMUM_PAGE;
        }
        if (m_maxPage > MAX_BUTTON_COUNT) {
            m_displayPage = MAX_BUTTON_COUNT + 1;
        } else {
            m_displayPage = m_maxPage;
        }
    }
}
void PaginationWid::btnPageClicked()
{
    QToolButton *btn = qobject_cast<QToolButton *>(sender());
    if (btn && btn != m_btnLeft && btn != m_btnRight) {
        QString str = btn->text();
        if (str.contains("…")) {
            str = btn->toolTip();
        }
        int nIndex = str.toInt();                    // 要跳转的页数
        QToolButton *page = findPage(m_currentPage); // 找到当前页数按钮
        if (page) {
            page->setChecked(false); // 取消选中
        }
        QToolButton *nPage = findPage(nIndex);
        if (nPage) {
            nPage->setChecked(true);
        }
        m_currentPage = nIndex;
        changeView(m_currentPage);
        Q_EMIT pageChanged(nIndex);
    }
}
void PaginationWid::changeView(int currentPage)
{
    if (currentPage == MINIMUM_PAGE) {
        m_backBtn->setDisabled(true); // 当前第一页，禁用左按钮
    } else {
        m_backBtn->setDisabled(false); // 开启右按钮
    }
    if (currentPage == m_maxPage) {
        m_nextBtn->setDisabled(true); // 最后一页，禁用右按钮
    } else {
        m_nextBtn->setDisabled(false); // 开启左按钮
    }
    // 全部显示
    if (m_displayPage <= MAX_BUTTON_COUNT) {

        for (unsigned long long i = 0; i < m_vecPageBtn.size(); i++) {
            QToolButton *btn = m_vecPageBtn[i];
            if (btn->isHidden()) {
                btn->show();
            }
            if (i == m_vecPageBtn.size() - 1) {
                btn->setText(QString::number(m_maxPage));
            } else {
                btn->setText(QString::number(i + 1));
            }
            dealBtnText(btn);
            btn->adjustSize();
            if (btn->text().toInt() == currentPage) {
                btn->setChecked(true);
            } else {
                btn->setChecked(false);
            }
        }
        // 屏蔽省略
        if (!m_btnLeft->isHidden()) {
            m_btnLeft->hide();
        }
        if (m_btnRight->isHidden()) {
            m_btnRight->hide();
        }
        return;
    }
    if (currentPage < MAX_BUTTON_COUNT && currentPage != MAX_BUTTON_COUNT - 1) {

        // 页面情况 1 2 3 4 5 6 ... max
        for (unsigned long long i = 0; i < m_vecPageBtn.size(); i++) {
            QToolButton *btn = m_vecPageBtn[i];
            if (btn->isHidden()) {
                btn->show();
            }
            if (i == m_vecPageBtn.size() - 1) {
                btn->setText(QString::number(m_maxPage));
            } else {
                btn->setText(QString::number(i + 1));
            }
            dealBtnText(btn);

            btn->adjustSize();
            if (btn->text().toInt() == currentPage) {
                btn->setChecked(true);
            } else {
                btn->setChecked(false);
            }
        }

        // 显示后省略号，屏蔽前省略
        if (!m_btnLeft->isHidden()) {
            m_btnLeft->hide();
        }
        if (m_btnRight->isHidden()) {
            if (m_maxPage - 1 == MAX_BUTTON_COUNT) {
                m_btnRight->hide();
            } else {
                m_btnRight->show();
            }
        }
    } else {
        if (currentPage < m_maxPage - 1) {
            int value = -1;
            for (unsigned long long i = 1; i <= MAX_BUTTON_COUNT - 1; i++) {
                QToolButton *btn = m_vecPageBtn[i];
                btn->setText(QString::number(currentPage + value++));
                dealBtnText(btn);

                btn->adjustSize();
                if (btn->text().toInt() == currentPage) {
                    btn->setChecked(true);
                } else {
                    btn->setChecked(false);
                }
            }
            // 开启两边省略号
            if (currentPage == 3) {
                m_btnLeft->hide();
            } else {
                m_btnLeft->show();
            }

            if (m_maxPage > 2 && currentPage == m_maxPage - 2) {
                m_btnRight->hide();
            } else {
                m_btnRight->show();
            }

        } else {
            // 页面情况 1 ... max - 5, max - 4, [max - 3], max - 2, max - 1, max
            for (unsigned long long i = 1; i < m_vecPageBtn.size(); i++) {
                QToolButton *btn = m_vecPageBtn[i];
                btn->setText(QString::number(m_maxPage - MAX_BUTTON_COUNT + i));
                dealBtnText(btn);

                btn->adjustSize();
                if (btn->text().toInt() == currentPage) {
                    btn->setChecked(true);
                } else {
                    btn->setChecked(false);
                }
            }
            // 显示前省略号，屏蔽后省略
            if (m_btnLeft->isHidden()) {
                if (m_maxPage - 1 == MAX_BUTTON_COUNT) {
                    m_btnLeft->hide();
                } else {
                    m_btnLeft->show();
                }
            }
            if (!m_btnRight->isHidden()) {
                m_btnRight->hide();
            }
        }
    }
}

void PaginationWid::clearPage()
{
    int sz = m_pageWidLayout->count();
    for (int i = 0; i < sz; i++) {
        QLayoutItem *item = m_pageWidLayout->takeAt(0);
        QWidget *tmp = item->widget();
        m_pageWidLayout->removeWidget(tmp);
        delete tmp;
        tmp = nullptr;
        delete item;
        item = nullptr;
    }
    m_vecPageBtn.clear();
}
void PaginationWid::reload()
{
    clearPage();
    m_btnGroup->deleteLater();
    m_btnGroup = new QButtonGroup(this);
    int lastPage = m_currentPage;
    m_currentPage = 1;
    m_btnLeft = new QToolButton(this);
    m_btnLeft->setText("…");
    m_btnRight = new QToolButton(this);
    m_btnRight->setText("…");
    m_btnLeft->setProperty("useButtonPalette", true);
    m_btnRight->setProperty("useButtonPalette", true);
    m_btnLeft->setStyleSheet("padding:2px");
    m_btnRight->setStyleSheet("padding:2px");
    m_btnLeft->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_btnRight->setFixedSize(BTN_SIZE, BTN_SIZE);
    m_btnLeft->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnRight->setCursor(QCursor(Qt::PointingHandCursor));
    m_btnLeft->hide();
    m_btnRight->hide();
    //    m_btnLeft->setText("...");
    //    m_btnRight->setText("...");

    for (int i = 0; i < m_displayPage; i++) {
        QToolButton *btn = new QToolButton(this);
        btn->setText(QString::number(i + 1));
        dealBtnText(btn);
        btn->setCheckable(true);
        m_pageWidLayout->addWidget(btn);
        btn->setProperty("useButtonPalette", true);
        btn->setFixedSize(BTN_SIZE, BTN_SIZE);
        btn->setCursor(QCursor(Qt::PointingHandCursor));

        connect(btn, &QToolButton::clicked, this, &PaginationWid::btnPageClicked);
        m_vecPageBtn.emplace_back(btn);
        m_btnGroup->addButton(btn);
    }
    m_btnGroup->setExclusive(true);
    m_pageWidLayout->insertWidget(1, m_btnLeft);
    m_pageWidLayout->insertWidget(m_pageWidLayout->count() - 1, m_btnRight);

    // 初始化一次界面
    changeView(m_currentPage);
    // 上次停留的页数大于现在的最大页数
    if (lastPage > m_maxPage) {
        m_currentPage = m_maxPage; // 直接变成当前最大页数
    } else {
        // 否则还是保持当前页数
        m_currentPage = lastPage;
    }
    changeView(m_currentPage); // 再次刷新下当前界面

    // 左省略号，当作快退，即退PAGE_STEP页
    connect(m_btnLeft, &QToolButton::clicked, this, [&]() {
        if (m_currentPage - PAGE_STEP < MINIMUM_PAGE) {
            m_currentPage = MINIMUM_PAGE;
        } else {
            m_currentPage -= PAGE_STEP;
        }
        changeView(m_currentPage);
        Q_EMIT pageChanged(m_currentPage);
    });

    // 右省略号，当作快进，即进PAGE_STEP页
    connect(m_btnRight, &QToolButton::clicked, this, [&]() {
        if (m_currentPage + PAGE_STEP > m_maxPage) {
            m_currentPage = m_maxPage;
        } else {
            m_currentPage += PAGE_STEP;
        }
        changeView(m_currentPage);
        Q_EMIT pageChanged(m_currentPage);
    });
}
void PaginationWid::refresh()
{
    reload();
}
QToolButton *PaginationWid::findPage(int nIndex) const
{
    for (int i = 0; i < m_pageWidLayout->count(); i++) {
        QLayoutItem *item = m_pageWidLayout->itemAt(i);
        if (item) {
            QToolButton *btn = qobject_cast<QToolButton *>(item->widget());
            if (btn && btn->text().compare(QString::number(nIndex)) == 0) {
                return btn;
            }
        }
    }
    return nullptr;
}
QToolButton *PaginationWid::getPageItem(int nPos)
{
    return dynamic_cast<QToolButton *>(m_pageWidLayout->itemAt(nPos)->widget());
}

void PaginationWid::dealBtnText(QToolButton *btn)
{
    QString fullText = btn->text();
    QFontMetrics elidfont(btn->font());

    QString elideNote = elidfont.elidedText(btn->text(), Qt::ElideRight, btn->width() - BTN_TEXT_ELF);
    btn->setText(elideNote);
    if (btn->text() != "…") {
        if (btn->text().contains("…")) {
            btn->setToolTip(fullText);
        } else {
            btn->setToolTip("");
        }
    }
    //    btn->setStyleSheet("padding:2px");
}
void PaginationWid::setTotalItem(int nTotal)
{
    //        m_pTotalItem->SetTotal(nTotal);
    m_totalItems = nTotal;
    int EachPage = MAX_PAGE;
    if (EachPage > 0) {
        m_maxPage = nTotal / EachPage;
        if (nTotal % EachPage > 0)
            ++m_maxPage;
        if (m_maxPage <= 0)
            m_maxPage = MINIMUM_PAGE;
        // 如果m_nMaxPage大于MAX_BUTTON_COUNT，则只构建7个按钮，头、尾，中间5个
        if (m_maxPage > MAX_BUTTON_COUNT)
            m_displayPage = MAX_BUTTON_COUNT + 1;
        else
            m_displayPage = m_maxPage;
    }
    updatePageNum(m_maxPage);
}

int PaginationWid::getTotalItemNum()
{
    return m_totalItems;
}
///////////////////////////////////
/// \brief PaginationWid::pageBtnClicked
// EachPageItem::EachPageItem(QWidget *parent) : QWidget(parent)
//{
//    setObjectName("per_page_item");
//    m_pCbx = new QComboBox(this);
//    m_pCbx->setFixedSize(100, 27);
//    setFixedSize(105, 30);
//    m_pCbx->setView(new QListView(this));
//    m_pCbx->view()->setFocusPolicy(Qt::NoFocus);
//    m_pCbx->setCursor(QCursor(Qt::PointingHandCursor));
//    m_pCbx->move(width() / 2 - m_pCbx->width() / 2, height() / 2 - m_pCbx->height() / 2);
//    m_vecPage.reserve(5);

//    connect(m_pCbx, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [&](int index) {
//        Q_EMIT EachPageItemChanged(m_vecPage[index]);
//    });
//}

// EachPageItem::~EachPageItem() {}

// void EachPageItem::SetItemEachPage(int *args, int size)
//{
//    m_vecPage.clear();
//    m_pCbx->clear();
//    m_vecPage.reserve(5);
//    for (int i = 0; i < size; i++) {
//        int n = args[i];
//        if (n > 0) {
//            if (i > 0) {
//                if (n <= m_vecPage[i - 1])
//                    continue;
//            }
//            m_vecPage.emplace_back(n);
//            m_pCbx->addItem(QString::number(n) + "条/页");
//        }
//    }
//}

// int EachPageItem::GetCurrent()
//{
//    unsigned long long idx = m_pCbx->currentIndex();
//    if (m_vecPage.size() >= idx + 1) {
//        return m_vecPage[idx];
//    } else
//        return 0;
//}


//////////////////////////GotoPageItem///////////////////////////////////

GotoPageItem::GotoPageItem(QWidget *parent) : QWidget(parent)
{
    setObjectName("goto_page_item");
    this->setFixedHeight(36);
    m_pHMainLayout = new QHBoxLayout(this);
    m_pLabel1 = new QLabel(tr("Jump to"), this);
    m_pEditPage = new QLineEdit(this);
    m_pLabel2 = new QLabel(tr("Page"), this);

    m_pEditPage->setAlignment(Qt::AlignCenter);
    m_pLabel1->adjustSize();
    m_pLabel2->adjustSize();
    m_pEditPage->setFixedSize(72, 36);
    QRegExp reg("[0-9]+$");
    QValidator *va = new QRegExpValidator(reg, m_pEditPage);
    m_pEditPage->setValidator(va);
    m_pHMainLayout->addWidget(m_pLabel1);
    m_pHMainLayout->addWidget(m_pEditPage);
    m_pHMainLayout->addWidget(m_pLabel2);
    m_pHMainLayout->addStretch();
    m_pHMainLayout->setContentsMargins(0, 0, 0, 0);
    m_pEditPage->installEventFilter(this);

    m_pEditPage->setFocusPolicy(Qt::ClickFocus);
}

GotoPageItem::~GotoPageItem() {}



bool GotoPageItem::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QKeyEvent::KeyRelease) {
        Qt::Key k = static_cast<Qt::Key>((dynamic_cast<QKeyEvent *>(event)->key()));
        if (k == Qt::Key_Return || k == Qt::Key_Enter) {
            QString str = m_pEditPage->text();
            int nPage = 1;
            if (!str.isEmpty())
                nPage = str.toInt();
            Q_EMIT GotoPage(nPage);
            return true;
        }
    }
    return QWidget::eventFilter(watched, event);
}
