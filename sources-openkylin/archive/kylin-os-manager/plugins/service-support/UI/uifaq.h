#ifndef UIFAQ_H
#define UIFAQ_H

#include <QWidget>
#include <QListWidget>
#include <QStackedWidget>
#include <QMetaType>
#include <kbackgroundgroup.h>
#include <ksearchlineedit.h>
#include <ktabbar.h>
#include "../constant.h"
#include "kom-label.h"
#include "service_support_base.h"

struct FaqItemData
{
    QString question;
    QString answer;
    QString category;
    QString version;
    QString tags;
};
Q_DECLARE_METATYPE(FaqItemData);

class UiFaqItem: public kdk::KBackgroundGroup {
    Q_OBJECT
public:
    UiFaqItem(FaqItemData itemData, QString highLight = "", QWidget *parent = nullptr);

signals:
    void heightChange(int);
    void showDetails(FaqItemData);

private:
    void adjustHtmlTextForLabel(QLabel *label, const QString &html);
    bool m_fold = true;
};


class QSqlDatabase;
class UiFaq: public QWidget, public ServiceSupportBase {
    Q_OBJECT
public:
    UiFaq(QWidget *parent = nullptr);
    bool GoBack() override;

public slots:
    void indexChanged(QString name);
    void showDetails(FaqItemData itemData);
    void searchFaq(QString text);
    void changeIssueType(QString text);

private slots:
    void searcTextChanged(QString text);

private:
    enum StackWidget {
        kHomepage,
        kDetails
    };

    void initUI();
    void initHomePageUI(QWidget *page);
    void initDetailsPageUI(QWidget *page);
    void initDataBase();
    void readSettings();
    QString creatPageSql(int i = 1);
    void creatFaqWidgetList();
    void runSqlCmd(QString cmd);

    QSqlDatabase *m_db = nullptr;
    QStackedWidget *m_stackWidget = nullptr;
    kdk::KTabBar *m_tabBar = nullptr;
    kdk::KSearchLineEdit *m_searchLineEdit = nullptr;
    QStringList m_typeMap1;//考虑到以后分类可能会变，就不绑定变量名了
    QStringList m_typeMap2;
    QStringList m_typeMap3;
    QStringList m_typeMap4;
    QList<FaqItemData> m_faqItemDataList;
    bool m_loaded = false;
    QString m_objectName = NAME_FAQ;
    QListWidget *m_faqListWidget = nullptr;
    kom::KomLabel *m_detailsPageTitle = nullptr;
    QLabel *m_detailsPageText = nullptr;
    QLabel *m_detailsPageTextVersion = nullptr;
    bool m_isSearchResult = false;
};


#endif // UIFAQ_H
