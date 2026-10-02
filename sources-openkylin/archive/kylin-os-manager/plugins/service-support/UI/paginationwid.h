#ifndef PAGINATIONWID_H
#define PAGINATIONWID_H


#include <QObject>
#include <QWidget>
#include <QToolButton>
#include <QButtonGroup>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>
#include <QListView>
#include <QKeyEvent>
#define BTN_SIZE 36
#define BTN_TEXT_ELF 10

#define MINIMUM_PAGE 1
#define MAX_PAGE 10

#define MAX_BUTTON_COUNT 4

#define PAGE_STEP 5


// class EachPageItem;
class GotoPageItem;
class btnz : public QToolButton
{
    Q_OBJECT
public:
    btnz(QWidget *parent, int num)
    {
        m_pageNum = num;
        changIcon(m_pageNum);
        setCheckable(true);
        connect(this, &btnz::clicked, this, &btnz::currentChange);
    }
    int getNum()
    {
        return m_pageNum;
    }
    void setNum(int num)
    {
        m_pageNum = num;
        changIcon(m_pageNum);
    }

private:
    int m_pageNum = -1;
    void changIcon(int pageNum)
    {
        if (pageNum < 0) {
            this->setText("...");
        } else {
            this->setText(QString::number(pageNum));
        }
    }
Q_SIGNALS:
    void currentChange(int currentNum);
};
class PaginationWid : public QWidget
{
    Q_OBJECT
public:
    explicit PaginationWid(QWidget *parent = nullptr, int totalPage = 1);

    void updatePageNum(int totalPage = 1);

    /**
     * @brief SetItemPerPage 设置每页显示多少条目
     * @param args 条目数组，数组元素必须大于0，且后面数必须大于前面
     * @param size 数组大小
     */
    void setItemPerPage();


    void refresh();
    void setTotalItem(int nTotal);
    int getTotalItemNum();

private:
    QToolButton *m_backBtn = nullptr;
    QToolButton *m_nextBtn = nullptr;
    QToolButton *m_btnLeft = nullptr;
    QToolButton *m_btnRight = nullptr;
    QButtonGroup *m_btnGroup = nullptr;
    QHBoxLayout *m_layout = nullptr;
    QWidget *m_btnWid = nullptr;
    QLabel *m_pageNum = nullptr;
    QLabel *m_jumpLabel = nullptr;
    QLabel *m_jumpPage = nullptr;
    QLineEdit *m_pageLine = nullptr;


    QHBoxLayout *m_pageWidLayout = nullptr;
    int m_totalItems = 1;
    int m_totalPage = 1;
    int m_currentPage = 1;

    void initMainWid();
    QList<btnz *> m_btnList;

    QString m_totalText;
    QString m_pagesText;

    std::vector<QToolButton *> m_vecPageBtn;

    int m_maxPage;         // 最大页数
    int m_displayPage = 0; // 显示的页数
                           //    EachPageItem *m_pItemEachPage;
    GotoPageItem *m_pGotoItem;

    void btnPageClicked();
    void changeView(int currentPage);
    void clearPage();
    void reload();
    QToolButton *findPage(int nIndex) const;
    QToolButton *getPageItem(int nPos);
    void dealBtnText(QToolButton *btn);

Q_SIGNALS:
    void pageChanged(int nPage);           // 页码变化
    void numberPerpageChanged(int nCount); // 每页条码变化
};

// class EachPageItem : public QWidget
//{
//    Q_OBJECT
// public:
//    explicit EachPageItem(QWidget *parent = nullptr);
//    ~EachPageItem() override;
//    void SetItemEachPage(int *args, int size);
//    int GetCurrent();

// protected:
//    //    void paintEvent(QPaintEvent* event) override;

// signals:
//    void EachPageItemChanged(int nCount);

// private:
//    QComboBox *m_pCbx;
//    std::vector<int> m_vecPage;

//};
class GotoPageItem : public QWidget
{
    Q_OBJECT
public:
    explicit GotoPageItem(QWidget *parent = nullptr);
    ~GotoPageItem() override;
    void setBtnString(const QString &jump, const QString &page);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

Q_SIGNALS:
    void GotoPage(int nPage);

private:
    QHBoxLayout *m_pHMainLayout;
    QLabel *m_pLabel1;
    QLineEdit *m_pEditPage;
    QLabel *m_pLabel2;
};
#endif // PAGINATIONWID_H
