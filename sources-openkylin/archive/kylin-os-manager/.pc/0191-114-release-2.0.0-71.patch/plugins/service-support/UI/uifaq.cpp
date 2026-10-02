#include "uifaq.h"
#include <cmath>
#include <QDebug>
#include <QBoxLayout>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlRecord>
#include <QSettings>
#include <QTextCodec>
#include <QScrollArea>
#include <QLabel>
#include <QApplication>
#include <QTextDocument>
#include <kborderlessbutton.h>

UiFaq::UiFaq(QWidget *parent) : QWidget(parent)
{
    setObjectName(NAME_FAQ);
    readSettings();
    initDataBase();
    initUI();
}

void UiFaq::initUI()
{
    QWidget *homepage = new QWidget(this);
    QWidget *detailsPage = new QWidget(this);

    m_stackWidget = new QStackedWidget(this);
    m_stackWidget->insertWidget(StackWidget::kHomepage, homepage);
    m_stackWidget->insertWidget(StackWidget::kDetails, detailsPage);
    m_stackWidget->setCurrentIndex(StackWidget::kHomepage);
    auto *svl = new QVBoxLayout;
    svl->setSpacing(0);
    svl->setMargin(0);
    svl->addWidget(m_stackWidget);
    setLayout(svl);

    initHomePageUI(homepage);
    initDetailsPageUI(detailsPage);
}

void UiFaq::initHomePageUI(QWidget *page)
{
    m_tabBar = new kdk::KTabBar(kdk::KTabBarStyle::SegmentDark, page);
    m_tabBar->addTab(tr("Application usage exception"));
    m_tabBar->addTab(tr("Network connection exception"));
    m_tabBar->addTab(tr("Hardware exception"));
    m_tabBar->addTab(tr("System exception"));
    m_tabBar->setFixedSize(m_tabBar->count() * 116, 36);
    connect(m_tabBar,&kdk::KTabBar::currentChanged,page,[=](int index){
        QString cmd = creatPageSql(index + 1);
        runSqlCmd(cmd);
    });

    m_searchLineEdit =new kdk::KSearchLineEdit(page);
    m_searchLineEdit->setFixedWidth(240);
    m_searchLineEdit->setClearButtonEnabled(true);
    m_searchLineEdit->setPlaceholderText(tr("Enter the problem you encountered"));
    connect(m_searchLineEdit,&kdk::KSearchLineEdit::textChanged,this,&UiFaq::searcTextChanged);

    auto *thl = new QHBoxLayout;
    thl->setSpacing(40);
    thl->setMargin(0);
    thl->addSpacing(40);
    thl->addWidget(m_tabBar);
    thl->addWidget(m_searchLineEdit);
    thl->addStretch();
    thl->addSpacing(40);

    m_faqListWidget = new QListWidget(page);
    m_faqListWidget->setFrameShape(QFrame::NoFrame);
    m_faqListWidget->setSizeAdjustPolicy(QListWidget::AdjustToContents);
    QScrollArea *sa = new  QScrollArea(page);
    sa->setWidget(m_faqListWidget);
    sa->setWidgetResizable(true);
    sa->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    sa->setFrameShape(QFrame::NoFrame);
    sa->setContentsMargins(0, 0, 0, 0);

    auto *hll = new QHBoxLayout;
    hll->setSpacing(0);
    hll->setContentsMargins(40,0,20,0);
    hll->addWidget(sa);

    auto *tvl = new QVBoxLayout;
    tvl->setSpacing(0);
    tvl->setMargin(0);
    tvl->addLayout(thl);
    tvl->addSpacing(20);
    tvl->addLayout(hll);
    page->setLayout(tvl);
}

void UiFaq::initDetailsPageUI(QWidget *page)
{
    m_detailsPageTitle = new kom::KomLabel(page);
    m_detailsPageTitle->setFontSize(18);

    auto *textBase = new kdk::KBackgroundGroup();
    textBase->setBorderRadius(6);
    textBase->setBackgroundRole(QPalette::ColorRole::Button);
    textBase->setFixedWidth(744);
    textBase->setFixedHeight(471);

    auto *scrollArea = new QScrollArea(textBase);
    scrollArea->setFixedWidth(744);
    scrollArea->setFixedHeight(471);
    scrollArea->setStyleSheet("QScrollArea { background-color: transparent; border: 0px; }");
    scrollArea->viewport()->setStyleSheet("background-color: transparent;");

    auto *myVersionLabel = new kom::KomLabel(textBase);
    myVersionLabel->setFixedWidth(700);
    myVersionLabel->setFontSize(12);
    myVersionLabel->setText("本机系统：银河麒麟操作系统V11"); //todo:改为动态获取
    myVersionLabel->move(24,430);

    auto *tbvl = new QVBoxLayout;
    tbvl->setSpacing(0);
    tbvl->setMargin(0);
    tbvl->addWidget(scrollArea);
    tbvl->addStretch();
    textBase->setLayout(tbvl);

    m_detailsPageText = new QLabel(scrollArea);
    m_detailsPageText->setWordWrap(true);
    m_detailsPageTextVersion = new QLabel(scrollArea);
    auto *tsvl = new QVBoxLayout;
    tsvl->setSpacing(0);
    tsvl->setContentsMargins(24,16,24,16);
    tsvl->addWidget(m_detailsPageText);
    tsvl->addWidget(m_detailsPageTextVersion);
    tsvl->addStretch();
    scrollArea->setLayout(tsvl);

    auto *tvl = new QVBoxLayout;
    tvl->setSpacing(0);
    tvl->setContentsMargins(40,17,40,0);
    tvl->addWidget(m_detailsPageTitle);
    tvl->addSpacing(16);
    tvl->addWidget(textBase);
    tvl->addStretch();

    page->setLayout(tvl);
}

void UiFaq::showDetails(FaqItemData itemData)
{
    m_detailsPageTitle->setText(itemData.question);
    m_detailsPageText->setText(itemData.answer);
    QString detailsPageTextVersionText = itemData.version;
    if (detailsPageTextVersionText.startsWith('|')) {
        detailsPageTextVersionText = detailsPageTextVersionText.mid(1);
    }
    if (detailsPageTextVersionText.endsWith('|')) {
        detailsPageTextVersionText.chop(1);
    }
    detailsPageTextVersionText = "\n【适应系统】\n" + detailsPageTextVersionText;
    m_detailsPageTextVersion->setText(detailsPageTextVersionText);
    m_stackWidget->setCurrentIndex(StackWidget::kDetails);
}

void UiFaq::searchFaq(QString text)
{
    m_searchLineEdit->setFocus();
    m_searchLineEdit->setText(text);
    m_searchLineEdit->clearFocus();
}

// 首页点击问题分类右侧箭头后执行
void UiFaq::changeIssueType(QString text)
{
    m_stackWidget->setCurrentIndex(StackWidget::kHomepage);
    if (!m_searchLineEdit->text().isEmpty()) {
        searchFaq("");
    }

    for (int i = 0; i < m_tabBar->count(); i++) {
        if (text == m_tabBar->tabText(i)) {
            m_tabBar->setCurrentIndex(i);
        }
    }
}

void UiFaq::searcTextChanged(QString text)
{
    m_stackWidget->setCurrentIndex(StackWidget::kHomepage);
    if (text.isEmpty()) {
        m_isSearchResult = false;
        m_loaded = false;
        indexChanged(m_objectName);//初始化页面，复用首次切入页面的初始化函数
        m_tabBar->show();
        m_tabBar->setCurrentIndex(0);
        return;
    }
    m_tabBar->hide();
    m_isSearchResult = true;
    QString sql = "select f_question,f_answer,f_category,f_version,tags from zsk_faq where f_question like '%";
    sql += text + "%'";
    runSqlCmd(sql);
}

void UiFaq::runSqlCmd(QString cmd)
{
    m_faqItemDataList.clear();
    QSqlQuery query(*m_db);
    query.exec(QString(cmd));
    while(query.next()) {
        FaqItemData tmp;
        tmp.question = query.value("f_question").toString();
        tmp.answer = query.value("f_answer").toString();
        tmp.category = query.value("f_category").toString();
        tmp.version = query.value("f_version").toString();
        tmp.tags = query.value("tags").toString();
        m_faqItemDataList.append(tmp);
    }
    creatFaqWidgetList();
}

void UiFaq::initDataBase()
{
    m_db =new QSqlDatabase(QSqlDatabase::addDatabase("QSQLITE", "faq_database_conn")) ;
    m_db->setDatabaseName("/usr/share/kylin-os-manager/data/service_support_faq.db");
    if (!m_db->open()) {
        qDebug()<<"open database failed";
        return;
    }
}

void UiFaq::readSettings()
{
    QSettings settings("/usr/share/kylin-os-manager/data/service_support_faq.ini", QSettings::IniFormat);
    settings.setIniCodec(QTextCodec::codecForName("UTF-8"));
    settings.beginGroup("KeyMap");
    m_typeMap1 = settings.value("type1").toStringList();
    m_typeMap2 = settings.value("type2").toStringList();
    m_typeMap3 = settings.value("type3").toStringList();
    m_typeMap4 = settings.value("type4").toStringList();
    settings.endGroup();
}

QString UiFaq::creatPageSql(int i)
{
    QString out = "select f_question,f_answer,f_category,f_version,tags from zsk_faq where ";
    QStringList list;
    if (i == 1) {
        list = m_typeMap1;
    } else if (i == 2) {
        list = m_typeMap2;
    } else if (i == 3) {
        list = m_typeMap3;
    } else if (i == 4) {
        list = m_typeMap4;
    }
    for (QString &str : list) {
        out += "f_category = '" + str + "' ";
        out += "or ";
    }
    out.chop(3);
    return out;
}

void UiFaq::indexChanged(QString name)
{
    if (name == m_objectName) {
        // 判断是否已经初始化过
        if (m_loaded) {
            return;
        }
        m_loaded = true;
        QString cmd = creatPageSql();
        runSqlCmd(cmd);
    }
}

void UiFaq::creatFaqWidgetList()
{
    m_faqListWidget->clear();

    int max = 100;
    if (max > m_faqItemDataList.length()) {
        max = m_faqItemDataList.length();
    }
    for (int i=0; i<max; i++) {
        auto *item = new QListWidgetItem();
        FaqItemData tmp = m_faqItemDataList.at(i);
        auto *faqItem = new UiFaqItem(tmp,m_searchLineEdit->text());
        //设置item高度时顺便指定间隔，如果用m_faqListWidget->setSpacing(4)会影响水平间隔，不符合设计要求
        item->setSizeHint(QSize(0, faqItem->height() + 4));
        item->setFlags(item->flags() & ~Qt::ItemIsSelectable & ~Qt::ItemIsEnabled);
        m_faqListWidget->addItem(item);
        m_faqListWidget->setItemWidget(item, faqItem);
        connect(faqItem,&UiFaqItem::heightChange,this,[=](int h){
            item->setSizeHint(QSize(0, h + 4));
        });
        connect(faqItem,&UiFaqItem::showDetails,this,&UiFaq::showDetails);
        //规避SDK背景框初始状态下无颜色的问题
        QEvent e(QEvent::Leave);
        QApplication::sendEvent(faqItem,&e);
    }
}

bool UiFaq::GoBack() {
    if (m_stackWidget->currentIndex() == StackWidget::kDetails) {
        m_stackWidget->setCurrentIndex(StackWidget::kHomepage);
        return true;
    }
    return false;
}

UiFaqItem::UiFaqItem(FaqItemData itemData, QString highLight, QWidget *parent) : kdk::KBackgroundGroup(parent)
{
    setBorderRadius(6);
    setBackgroundRole(QPalette::ColorRole::Button);
    setFixedWidth(746);
    setFixedHeight(53);
    setStateEnable(this,true);

    auto *widget = new QWidget(this);
    addWidget(widget);

    auto *t1 = new QLabel(widget);
    t1->setAttribute(Qt::WA_TransparentForMouseEvents);
    QString titleText = itemData.question;
    if (!highLight.isEmpty()) {
        QColor highlightColor = qApp->palette().color(QPalette::Highlight);
        QString color = highlightColor.name();
        QString replaceText = "<span style=\"color:"+ color +";\">" + highLight + "</span>";
        titleText.replace(highLight,replaceText);
    }
    t1->setText(titleText);
    t1->setAlignment(Qt::AlignVCenter);
    auto *t2 = new QLabel(widget);
    t2->setPixmap(QIcon::fromTheme("ukui-end.symbolic").pixmap(16,16));
    auto *t3 = new QLabel(widget);
    t3->setContentsMargins(0,0,20,0);
    t3->setFixedHeight(84);
    t3->setWordWrap(true);
    adjustHtmlTextForLabel(t3, itemData.answer);
    t3->hide();
    auto *b1 = new kdk::KBorderlessButton(tr("查看详情"),widget);
    connect(b1, &kdk::KBorderlessButton::clicked, b1, [=] {
        emit showDetails(itemData);
    });
    b1->hide();

    auto *hl1 = new QHBoxLayout();
    hl1->setSpacing(0);
    hl1->setMargin(0);
    hl1->addWidget(t1);
    hl1->addStretch();
    hl1->addWidget(t2);

    auto *hl2 = new QHBoxLayout();
    hl2->setSpacing(0);
    hl2->setMargin(0);
    hl2->addWidget(b1);
    hl2->addStretch();

    auto *vl = new QVBoxLayout();
    vl->setSpacing(0);
    vl->setContentsMargins(24,0,24,8);
    vl->addLayout(hl1);
    vl->addWidget(t3);
    vl->addLayout(hl2);
    widget->setLayout(vl);

    connect(this,&UiFaqItem::clicked,this,[=](QWidget *w){
        int height = 53;
        if (m_fold) {
            t2->setPixmap(QIcon::fromTheme("ukui-down.symbolic").pixmap(16,16));
            height = 174;
            t3->show();
            b1->show();
        } else {
            t2->setPixmap(QIcon::fromTheme("ukui-end.symbolic").pixmap(16,16));
            t3->hide();
            b1->hide();
        }
        widget->setFixedHeight(height);
        setFixedHeight(height);
        emit heightChange(height);
        m_fold = !m_fold;
    });
}

void UiFaqItem::adjustHtmlTextForLabel(QLabel *label, const QString &html) {
    // 确保 QLabel 已设置字体和尺寸, 获取单位文本高度
    QFontMetrics fm(label->font());
    int lineHeight = fm.height();

    // 计算可显示的最大行数（考虑padding）
    int availablewidth = label->width() - 40;
    int availableHeight = label->height() - 2; // 减去边框padding
    int maxLines = qMax(1, availableHeight / lineHeight);

    // 将HTML转换为纯文本以计算行数
    QTextDocument doc;
    doc.setHtml(html);
    QString plainText = doc.toPlainText().trimmed();

    // 按行分割文本
    QStringList lines = plainText.split('\n');

    // 构造自适应行数的HTML文本
    QString adaptedHtml;
    int count = 0;

    // 重新组装不超过最大行数的HTML内容
    for (const QString& line: lines) {
        if (count >= maxLines) {
            break;
        }

        adaptedHtml += line + "<br>";

        auto text_width = fm.width(line);
        if (text_width > availablewidth) {
            count += std::ceil(static_cast<double>(text_width) / availablewidth);
        } else {
            count++;
        }
    }

    label->setText(adaptedHtml);
}
