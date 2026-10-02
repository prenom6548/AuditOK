#include <QDebug>
#include <QTimer>
#include <QGraphicsDropShadowEffect>
#include <QDir>
#include <gsettingmonitor.h>
#include <ukuiwindowhelper/ukuiwindowhelper.h>
#include "mainwindow.h"
#include "toolmodel.h"
#include "cal.h"

const int TAG_INFO = 100;

using namespace std;

ToolModelOutput::ToolModelOutput(QWidget *parent) : QWidget(parent)
{
    /* 初始化组件 */
    this->setWidgetUi();

    /* 设置组件样式 */
    this->setWidgetStyle();
}


// 初始化组件
void ToolModelOutput::setWidgetUi()
{
    /* 汇率刷新 */
    toolLabUpdate = new QLabel(this);
    toolLabUpdate->setAccessibleName("kylin-calculator_QLabel_toolLabUpdate");
    toolIconUpdate = new QToolButton(this);
    toolIconUpdate->setAccessibleName("kylin-calculator_QToolButton_toolIconUpdate");
    toolLabTime = new QLabel(this);
    toolLabTime->setAccessibleName("kylin-calculator_QLabel_toolLabTime");
    toolLabRate = new QLabel(this);
    toolLabRate->setAccessibleName("kylin-calculator_QLabel_toolLabRate");
    toolLabUpdate->setContentsMargins(4, 0, 4, 0);
    toolIconUpdate->setFixedSize(24, 24);

    toolUpdateLayout = new QVBoxLayout(this);
    timeRateLayout = new QVBoxLayout(this);

    timeRateLayout->addWidget(toolLabTime, 0, Qt::AlignCenter);
    timeRateLayout->addWidget(toolLabRate, 0, Qt::AlignCenter);
    timeRateLayout->setSpacing(0);
    timeRateLayout->setMargin(0);

    toolUpdateLayout->setSpacing(0);
    toolUpdateLayout->addSpacing(5);
    toolUpdateLayout->addWidget(toolLabUpdate, 0, Qt::AlignCenter);
    toolUpdateLayout->addWidget(toolIconUpdate, 0, Qt::AlignCenter);
    toolUpdateLayout->addLayout(timeRateLayout, 0);
    toolUpdateLayout->addSpacing(5);
    toolUpdateLayout->setMargin(0);

    toolUpdateWid = new QWidget(this);
    toolUpdateWid->setAccessibleName("kylin-calculator_QWidget_toolUpdateWid");
    toolUpdateWid->setLayout(toolUpdateLayout);
    toolUpdateWid->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    /* 换算前单位标识 */
    toolLabUnitBef = new QLabel(this);
    toolLabUnitBef->setAccessibleName("kylin-calculator_QLabel_toolLabUnitBef");
    toolLabIconBef = new QLabel(this);
    toolLabIconBef->setAccessibleName("kylin-calculator_QLabel_toolLabIconBef");

    toolLabUnitBef->setFixedWidth(64);
    toolLabUnitBef->setWordWrap(true);

    toolUnitBefLayout = new QHBoxLayout(this);
    toolUnitBefLayout->setSpacing(0);
    toolUnitBefLayout->addWidget(toolLabUnitBef);
    toolUnitBefLayout->addWidget(toolLabIconBef);
    toolUnitBefLayout->addSpacing(16);
    toolUnitBefLayout->setMargin(0);

    toolUnitBefWid = new QPushButton(this);
    toolUnitBefWid->setAccessibleName("kylin-calculator_QPushButton_toolUnitBefWid");
    toolUnitBefWid->setLayout(toolUnitBefLayout);
    toolUnitBefWid->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    /* 换算后单位标识 */
    toolLabUnitAft = new QLabel(this);
    toolLabUnitAft->setAccessibleName("kylin-calculator_QLabel_toolLabUnitAft");
    toolLabIconAft = new QLabel(this);
    toolLabIconAft->setAccessibleName("kylin-calculator_QLabel_toolLabIconAft");

    toolLabUnitAft->setFixedWidth(64);
    toolLabUnitAft->setWordWrap(true);

    toolUnitAftLayout = new QHBoxLayout(this);
    toolUnitAftLayout->setSpacing(0);
    toolUnitAftLayout->addWidget(toolLabUnitAft);
    toolUnitAftLayout->addWidget(toolLabIconAft);
    toolUnitAftLayout->addSpacing(16);
    toolUnitAftLayout->setMargin(0);

    toolUnitAftWid = new QPushButton(this);
    toolUnitAftWid->setAccessibleName("kylin-calculator_QPushButton_toolUnitAftWid");
    toolUnitAftWid->setLayout(toolUnitAftLayout);
    toolUnitAftWid->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);

    /* 汇率刷新 */
    toolLabUpdate->setText(tr("Rate update"));
    toolIconUpdate->setIcon(QIcon::fromTheme("transform-rotate", QIcon(":/image/update.png")));
    toolIconUpdate->setIconSize(QSize(16, 16));
    toolIconUpdate->setProperty("isWindowButton", 0x1);
    toolIconUpdate->setProperty("useIconHighlightEffect", 0x2);
    toolIconUpdate->setAutoRaise(true);

    /* 默认汇率 */
    toolDouRate = 0.15;
    toolLabTime->setText("2020.09.03 09:30");
    toolLabRate->setText("1 CNY = 0.15 USD");

    connect(toolIconUpdate, &QToolButton::clicked, this, &ToolModelOutput::updateRate);

    /* 换算前单位标识 */
    toolRateNameBef = tr("Chinese Yuan");
    toolRateSymbBef = "CNY";
    toolLabUnitBef->setText(toolRateNameBef + "\n" + toolRateSymbBef);
    toolLabUnitBef->setAlignment(Qt::AlignCenter);
    toolLabIconBef->setPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16));
    toolLabIconBef->setFixedSize(16, 16);

    /* 换算后单位标识 */
    toolRateNameAft = tr("US Dollar");
    toolRateSymbAft = "USD";
    toolLabUnitAft->setText(toolRateNameAft + "\n" + toolRateSymbAft);
    toolLabUnitAft->setAlignment(Qt::AlignCenter);
    toolLabIconAft->setPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16));
    toolLabIconAft->setFixedSize(16, 16);

    connect(toolUnitBefWid, SIGNAL(clicked()), this, SLOT(unitListBefShow()));
    connect(toolUnitAftWid, SIGNAL(clicked()), this, SLOT(unitListAftShow()));

    // 数据输出界面
    this->toolLabHis = new QLabel(this);
    toolLabHis->setAccessibleName("kylin-calculator_QLabel_toolLabHis");
    this->toolLabBef = new QLabel(this);
    toolLabBef->setAccessibleName("kylin-calculator_QLabel_toolLabBef");
    this->toolLabAft = new QLabel(this);
    toolLabAft->setAccessibleName("kylin-calculator_QLabel_toolLabAft");

    this->toolLabAft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->toolLabBef->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    this->toolLabAft->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    QFont toolLabHisFont("SourceHanSansCN-Light", 40, 15);
    this->toolLabHis->setAlignment(Qt::AlignRight | Qt::AlignTop);
    this->toolLabHis->setFont(toolLabHisFont);
    this->toolLabHis->setText("");
    this->toolLabHis->setContentsMargins(0, 0, 20, 0);
    this->toolLabHis->show();

    QFont toolLabBefFont("SourceHanSansCN-Normal;", 48, 15);
    this->toolLabBef->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
    this->toolLabBef->setFont(toolLabBefFont);
    this->toolLabBef->setText("0");
    this->toolLabBef->setContentsMargins(0, 0, 20, 0);
    this->toolLabBef->show();

    QFont toolLabAftFont("SourceHanSansCN-Normal", 48, 15);
    this->toolLabAft->setAlignment(Qt::AlignRight | Qt::AlignBottom);
    this->toolLabAft->setFont(toolLabAftFont);
    this->toolLabAft->setText("0");
    this->toolLabAft->setContentsMargins(0, 0, 20, 0);
    this->toolLabAft->show();

    // 将数据输出组件和数据单位列表加入布局
    QGridLayout *toolOutputLayout = new QGridLayout(this);

    toolOutputLayout->addWidget(toolUpdateWid, 0, 0, 1, 1);
    toolOutputLayout->addWidget(toolUnitBefWid, 1, 0, 1, 1);
    toolOutputLayout->addWidget(toolUnitAftWid, 2, 0, 1, 1);
    toolOutputLayout->addWidget(toolLabHis, 0, 1, 1, 3);
    toolOutputLayout->addWidget(toolLabBef, 1, 1, 1, 3);
    toolOutputLayout->addWidget(toolLabAft, 2, 1, 1, 3);


    toolOutputLayout->setSpacing(2);
    toolOutputLayout->setContentsMargins(4, 1, 1, 2);

    this->setLayout(toolOutputLayout);

    // 数据单位列表
    this->unitListBef = new UnitListWidget(this->parentWidget());
    unitListBef->setAccessibleName("kylin-calculator_UnitListWidget_unitListBef");
    this->unitListAft = new UnitListWidget(this->parentWidget());
    unitListAft->setAccessibleName("kylin-calculator_UnitListWidget_unitListAft");

    connect(this->unitListBef, &UnitListWidget::itemClicked, this, &ToolModelOutput::listItemClicked);
    connect(this->unitListAft, &UnitListWidget::itemClicked, this, &ToolModelOutput::listItemClicked);

    this->unitListBef->hide();
    this->unitListAft->hide();

    m_picturetowhite = new PictureToWhite();
    m_picturetowhite->setObjectName("kylin-calculator_PictureToWhite_m_picturetowhite");

    rateMap = {{"Algerian dinar", "DZD"},      {"Australian dollar", "AUD"},
               {"Bahrain Dinar", "BHD"},       {"Bolivar Fuerte", "VEF"},
               {"Botswana pula", "BWP"},       {"Brazilian real", "BRL"},
               {"Brunei dollar", "BND"},       {"Canadian dollar", "CAD"},
               {"Chilean peso", "CLP"},        {"Chinese yuan", "CNY"},
               {"Colombian peso", "COP"},      {"Czech koruna", "CZK"},
               {"Danish krone", "DKK"},        {"Euro", "EUR"},
               {"Hungarian Forint", "HUF"},    {"Icelandic Krona", "ISK"},
               {"Indian rupee", "INR"},        {"Indonesian Rupiah", "IDR"},
               {"Iranian Rial", "IRR"},        {"Israeli New Shekel", "ILS"},
               {"Japanese yen", "JPY"},        {"Kazakhstani Tenge", "KZT"},
               {"Korean won", "KRW"},          {"Kuwaiti dinar", "KWD"},
               {"Libyan Dinar", "LYD"},        {"Malaysian ringgit", "MYR"},
               {"Mauritian rupee", "MUR"},     {"Mexican peso", "MXN"},
               {"Nepalese Rupee", "NPR"},      {"New Zealand dollar", "NZD"},
               {"Norwegian krone", "NOK"},     {"Nuevo Sol", "PEN"},
               {"Omani rial", "OMR"},          {"Pakistani Rupee", "PKR"},
               {"Peruvian sol", "PEN"},        {"Philippine peso", "PHP"},
               {"Polish zloty", "PLN"},        {"Qatari riyal", "QAR"},
               {"Rial Omani", "OMR"},          {"Russian ruble", "RUB"},
               {"Saudi Arabian riyal", "SAR"}, {"Singapore dollar", "SGD"},
               {"South African rand", "ZAR"},  {"Sri Lanka Rupee", "LKR"},
               {"Swedish krona", "SEK"},       {"Swiss franc", "CHF"},
               {"Thai baht", "THB"},           {"Trinidadian dollar", "TTD"},
               {"Tunisian Dinar", "TND"},      {"U.A.E. dirham", "AED"},
               {"U.K. pound", "GBP"},          {"Uruguayan peso", "UYU"},
               {"U.S. dollar", "USD"},         {NULL, NULL}};

    // 没有ARS
    currencyInfoUS = {{"AED", tr("UAE Dirham")},         {"ARS", tr("Argentinian peso")},
                      {"AUD", tr("Australian Dollar")},  {"BGN", tr("Bulgarian Lev")},
                      {"BHD", tr("Bahraini Dinar")},     {"BND", tr("Brunei Dollar")},
                      {"BRL", tr("Brazilian Real")},     {"BSD", tr("Bahaman Dollar")},
                      {"BWP", tr("Botswana Pula")},      {"CAD", tr("Canadian Dollar")},
                      {"CFA", tr("CFA Franc")},          {"CHF", tr("Swiss Franc")},
                      {"CLP", tr("Chilean Peso")},       {"CNY", tr("Chinese Yuan")},
                      {"COP", tr("Colombian Peso")},     {"CZK", tr("Czech Koruna")},
                      {"DKK", tr("Danish Krone")},       {"DOP", tr("Dominican peso")},
                      {"DZD", tr("Algerian Dinar")},     {"EEK", tr("Estonian Kroon")},
                      {"EGP", tr("Egyptian pound")},     {"EUR", tr("Euro")},
                      {"FJD", tr("Fijian dollar")},      {"GBP", tr("Pound Sterling")},
                      {"GTQ", tr("Guatemalan Quetzal")}, {"HKD", tr("Hong Kong Dollar")},
                      {"HRK", tr("Croatian Kuna")},      {"HUF", tr("Hungarian Forint")},
                      {"IDR", tr("Indonesian Rupiah")},  {"ILS", tr("Israeli New Shekel")},
                      {"INR", tr("Indian Rupee")},       {"IRR", tr("Iranian Rial")},
                      {"ISK", tr("Icelandic Krona")},    {"JPY", tr("Japanese Yen")},
                      {"KRW", tr("South Korean Won")},   {"KWD", tr("Kuwaiti Dinar")},
                      {"KZT", tr("Kazakhstani Tenge")},  {"LKR", tr("Sri Lankan Rupee")},
                      {"LTL", tr("Lithuanian Litas")},   {"LVL", tr("Latvian Lats")},
                      {"LYD", tr("Libyan Dinar")},       {"MUR", tr("Mauritian Rupee")},
                      {"MVR", tr("Maldivian Rupee")},    {"MXN", tr("Mexican Peso")},
                      {"MYR", tr("Malaysian Ringgit")},  {"NOK", tr("Norwegian Krone")},
                      {"NPR", tr("Nepalese Rupee")},     {"NZD", tr("New Zealand Dollar")},
                      {"OMR", tr("Omani Rial")},         {"PAB", tr("Panamanian balbos")},
                      {"PEN", tr("Peruvian Nuevo Sol")}, {"PHP", tr("Philippine Peso")},
                      {"PKR", tr("Pakistani Rupee")},    {"PLN", tr("Polish Zloty")},
                      {"PYG", tr("Paraguayan Guaran")},  {"QAR", tr("Qatari Riyal")},
                      {"RON", tr("New Romanian Leu")},   {"RUB", tr("Russian Rouble")},
                      {"SAR", tr("Saudi Riyal")},        {"SEK", tr("Swedish Krona")},
                      {"SGD", tr("Singapore Dollar")},   {"THB", tr("Thai Baht")},
                      {"TND", tr("Tunisian Dinar")},     {"TRY", tr("New Turkish Lira")},
                      {"TTD", tr("T&T Dollar (TTD)")},   {"TWD", tr("Taiwan Dollar")},
                      {"UAH", tr("Ukrainian Hryvnia")},  {"USD", tr("US Dollar")},
                      {"UYU", tr("Uruguayan Peso")},     {"VEF", tr("Venezuelan Bolívar")},
                      {"ZAR", tr("South African Rand")}, {NULL, NULL}};

    // 新增ARS、BSD、DOP、EGP、FJD、GTQ、MVR、PAB、PYG、TWD、UAH
    currencyInfoCN = {{"AED", "阿联酋迪拉姆"},
                      {"ARS", "阿根廷比索"},
                      {"AUD", "澳大利亚元"},
                      {"BGN", "保加利亚列弗"},
                      {"BHD", "巴林第纳尔"},
                      {"BND", "文莱元"},
                      {"BRL", "巴西雷亚尔"},
                      {"BSD", "巴哈马元"},
                      {"BWP", "博茨瓦纳普拉"},
                      {"CAD", "加拿大元"},
                      {"CFA", "中非法郎"},
                      {"CHF", "瑞士法郎"},
                      {"CLP", "智利比索"},
                      {"CNY", "人民币"},
                      {"COP", "哥伦比亚比索"},
                      {"CZK", "捷克克朗"},
                      {"DKK", "丹麦克朗"},
                      {"DOP", "多米尼加比索"},
                      {"DZD", "阿尔及利亚第纳尔"},
                      {"EEK", "爱沙尼亚克朗"},
                      {"EGP", "埃及镑"},
                      {"EUR", "欧元"},
                      {"FJD", "斐济元"},
                      {"GBP", "英镑"},
                      {"GTQ", "危地马拉格查尔"},
                      {"HKD", "港币"},
                      {"HRK", "克罗地亚库纳"},
                      {"HUF", "匈牙利福林"},
                      {"IDR", "印度尼西亚卢比"},
                      {"ILS", "以色列新谢克尔"},
                      {"INR", "印度卢比"},
                      {"IRR", "伊朗里亚尔"},
                      {"ISK", "冰岛克朗"},
                      {"JPY", "日元"},
                      {"KRW", "韩元"},
                      {"KWD", "科威特第纳尔"},
                      {"KZT", "哈萨克斯坦腾格"},
                      {"LKR", "斯里兰卡卢比"},
                      {"LTL", "立陶宛立特"},
                      {"LVL", "拉脱维亚拉特"},
                      {"LYD", "利比亚第纳尔"},
                      {"MUR", "毛里求斯卢比"},
                      {"MVR", "马尔代夫卢比"},
                      {"MXN", "墨西哥比索"},
                      {"MYR", "马来西亚林吉特"},
                      {"NOK", "挪威克朗"},
                      {"NPR", "尼泊尔卢比"},
                      {"NZD", "新西兰元"},
                      {"OMR", "阿曼里亚尔"},
                      {"PAB", "巴拿马巴波亚"},
                      {"PEN", "秘鲁新索尔"},
                      {"PHP", "菲律宾比索"},
                      {"PKR", "巴基斯坦卢比"},
                      {"PLN", "波兰兹罗提"},
                      {"PYG", "巴拉圭瓜拉尼"},
                      {"QAR", "卡塔尔里亚尔"},
                      {"RON", "新罗马尼亚列伊"},
                      {"RUB", "俄罗斯卢布"},
                      {"SAR", "沙特里亚尔"},
                      {"SEK", "瑞典克朗"},
                      {"SGD", "新加坡元"},
                      {"THB", "泰铢"},
                      {"TND", "突尼斯第纳尔"},
                      {"TRY", "新土尔其里拉"},
                      {"TTD", "特立尼达和多巴哥元"},
                      {"TWD", "台币"},
                      {"UAH", "乌克兰格里夫纳"},
                      {"USD", "美元"},
                      {"UYU", "乌拉圭比索"},
                      {"VEF", "委内瑞拉玻利瓦尔"},
                      {"ZAR", "南非兰特"},
                      {NULL, NULL}};

    m_firstLoad = true;

    defUpdateRate();

    updateRate();

    this->setAutoFillBackground(true);
    this->setBackgroundRole(QPalette::Base);

    toolUnitBefWid->setProperty("isWindowButton", 0x1);
    toolUnitBefWid->setProperty("useIconHighlightEffect", 0x2);

    toolUnitAftWid->setProperty("isWindowButton", 0x1);
    toolUnitAftWid->setProperty("useIconHighlightEffect", 0x2);
}

// 设置组件样式
void ToolModelOutput::setWidgetStyle(bool resetFontSize)
{
    QFont font = toolLabTime->font();
    font.setPixelSize(12);
    toolLabTime->setFont(font);
    toolLabRate->setFont(font);
    if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::LIGHT) {
//        toolLabUpdate->setStyleSheet("font-size:16px;color:#272A2D;");

//        toolLabTime->setStyleSheet("font-size:12px;color:#272A2D;");
//        toolLabRate->setStyleSheet("font-size:12px;color:#272A2D;");
//        toolUpdateWid->setStyleSheet("background:#F3F3F3;opacity:1;");

//        QString unitLabStyle = "font-family:SourceHanSansCN-ExtraLight;font-size:18px;color:#272A2D;line-height:18px;";
//        toolLabUnitBef->setStyleSheet(unitLabStyle);
        toolLabIconBef->setPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16));
//        toolUnitBefWid->setStyleSheet("QPushButton{background:#F3F3F3;color:#272A2D;opacity:1;}"
//                                      "QPushButton:hover{background-color:#F3F3F3;}");
//        toolLabUnitAft->setStyleSheet(unitLabStyle);
        toolLabIconAft->setPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16));
//        toolUnitAftWid->setStyleSheet("QPushButton{background:#F3F3F3;color:#272A2D;opacity:1;}"
//                                      "QPushButton:hover{background-color:#F3F3F3;}");
        // 数据输出界面
        this->toolLabHis->setStyleSheet("color:#8C8C8C;font-size:12px;font-weight:15px;line-height:50px;");

        if (resetFontSize) {
            this->toolLabAft->setStyleSheet("color:#272A2D;font-size:40px;font-weight:15px;line-height:50px;");
            this->toolLabBef->setStyleSheet("color:#272A2D;font-size:48px;font-weight:15px;");
        } else {
            QString fontSizeStr = QString::number(toolLabAft->font().pixelSize());
            this->toolLabAft->setStyleSheet("color:#272A2D;font-size:" + fontSizeStr
                                            + "px;font-weight:15px;line-height:50px;");
            fontSizeStr = QString::number(toolLabBef->font().pixelSize());
            this->toolLabBef->setStyleSheet("color:#272A2D;font-size:" + fontSizeStr + "px;font-weight:15px;");
        }

    } else if (WidgetStyle::themeColor == WidgetStyle::ThemeColor::DARK) {
//        toolLabUpdate->setStyleSheet("font-family:SourceHanSansCN-ExtraLight;font-size:16px;color:#FFFFFF;");

//        toolLabTime->setStyleSheet("font-family:NotoSansHans-Light;font-size:12px;color:#A6A6A6;line-height:17px;");
//        toolLabRate->setStyleSheet("font-family:NotoSansHans-Light;font-size:12px;color:#A6A6A6;line-height:17px;");
//        toolUpdateWid->setStyleSheet("background:#18181A;opacity:0.75;");

//        QString unitLabStyle = "font-family:SourceHanSansCN-ExtraLight;font-size:18px;color:#FFFFFF;line-height:22px;";
//        toolLabUnitBef->setStyleSheet(unitLabStyle);
        toolLabIconBef->setPixmap(
            m_picturetowhite->drawSymbolicColoredPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16)));
//        toolUnitBefWid->setStyleSheet("QPushButton{background:#18181A;color:#FFFFFF;opacity:0.75;}"
//                                      "QPushButton:hover{background-color:#18181A;}");
//        toolLabUnitAft->setStyleSheet(unitLabStyle);
        toolLabIconAft->setPixmap(
            m_picturetowhite->drawSymbolicColoredPixmap(QIcon::fromTheme("ukui-down-symbolic").pixmap(16, 16)));
//        toolUnitAftWid->setStyleSheet("QPushButton{background:#18181A;color:#FFFFFF;opacity:0.75;}"
//                                      "QPushButton:hover{background-color:#18181A;}");

        // 数据输出界面
        this->toolLabHis->setStyleSheet("color:#8C8C8C;font-size:12px;font-weight:15px;line-height:50px;");

        if (resetFontSize) {
            this->toolLabAft->setStyleSheet("color:#FFFFFF;font-size:40px;font-weight:15px;line-height:50px;");
            this->toolLabBef->setStyleSheet("color:#FFFFFF;font-size:48px;font-weight:15px;");
        } else {
            QString fontSizeStr = QString::number(toolLabAft->font().pixelSize());
            this->toolLabAft->setStyleSheet("color:#FFFFFF;font-size:" + fontSizeStr
                                            + "px;font-weight:15px;line-height:50px;");
            fontSizeStr = QString::number(toolLabBef->font().pixelSize());
            this->toolLabBef->setStyleSheet("color:#FFFFFF;font-size:" + fontSizeStr + "px;font-weight:15px;");
        }
    }
}

// 初始化单位列表
void ToolModelOutput::initUnitList(QString listStr)
{
    QStringList unitStrList = listStr.split(",");

    unitListBef->clear();
    unitListAft->clear();

    QListWidgetItem *unitBeforeItem[unitStrList.size()];
    QListWidgetItem *unitAfterItem[unitStrList.size()];

    for (int i = 0; i < unitStrList.size(); i++) {

        // 获取汇率名称
        QMap<QString, QString>::iterator it;
        QString itemName;

        if (currencyInfoUS.contains(unitStrList[i])) {

            it = currencyInfoUS.find(unitStrList[i]);
            if (it != currencyInfoUS.end()) {
                itemName = it.value();
            } else {
                continue;
            }
        } else {
            continue;
        }

        unitBeforeItem[i] = new QListWidgetItem(unitListBef->getListWidget(), i);
        unitAfterItem[i] = new QListWidgetItem(unitListAft->getListWidget(), i);
        unitBeforeItem[i]->setSizeHint(QSize(240, 36));
        unitAfterItem[i]->setSizeHint(QSize(240, 36));
        unitBeforeItem[i]->setText(itemName + " " + unitStrList[i]);
        unitAfterItem[i]->setText(itemName + " " + unitStrList[i]);
        unitBeforeItem[i]->setData(Qt::DisplayRole + TAG_INFO, unitStrList[i]);
        unitAfterItem[i]->setData(Qt::DisplayRole + TAG_INFO, unitStrList[i]);

        unitListBef->insertItem(i, unitBeforeItem[i]);
        unitListAft->insertItem(i, unitAfterItem[i]);
    }

    unitListBef->setSpacing(2);
    unitListAft->setSpacing(2);

    // 默认汇率选择人民币和美元
    unitListBef->setCurrentRow(12);
    unitListAft->setCurrentRow(63);

    return;
}

/* 将正常计算结果 乘以汇率 进行转换 */
void ToolModelOutput::unitConversion()
{
    QString strBefore = this->toolLabBef->text().remove(QRegExp(","));
    strBefore.replace("×", "*");
    strBefore.replace("÷", "/");
    strBefore.replace(SUB, "-");

    /* 判断获取的表达式是否正常 */
    /* 之前的计算已经进行判断 , 此处判断表达式中含有数字 则认为正常 , 目的在与 排除 inf Error 等表示错误的表达式 */
    bool flag = false;
    std::string stdStr = strBefore.toStdString();
    const char *cStr = stdStr.c_str();
    while (*cStr != '\0') {
        if (*cStr >= '0' && *cStr <= '9') {
            flag = true;
            break;
        }
        cStr++;
    }

    if (!flag) {
        return;
    }

    /* 乘以汇率 */
    double douResult = calculator(calculator(strBefore) + "*" + QString::number(toolDouRate)).toDouble();

    QString strResult = QString::number(douResult);

    if (strResult.contains("inf")) {
        strResult = tr("Error!");
    }

    this->toolLabAft->setText(strResult);

    return;
}

// 单位换算历史记录转换
QString ToolModelOutput::unitConvHistory(QString hisText)
{
    QString resHisText;

    hisText.replace("×", "*");
    hisText.replace("÷", "/");
    hisText.replace(SUB, "-");

    /* 默认使用人民币/美元汇率 */
    if (toolDouRate == 0) {
        toolDouRate = 0.15;
    }

    QStringList singerHisList = (hisText).split("=");
    QString dataBef = singerHisList[singerHisList.size() - 1].remove(QRegExp(",")).remove("\n");
    double douResult = calculator(calculator(dataBef) + "*" + QString::number(toolDouRate)).toDouble();
    QString newHisText =
        dataBef + " " + toolRateSymbBef + " = " + QString::number(douResult) + " " + toolRateSymbAft + "\n";

    toolDisHistory.append(newHisText);

    // 获取历史记录
    int size = toolDisHistory.size();
    int hisIndex = (size - 4 >= 0) ? (size - 4) : 0;
    for (int i = hisIndex; i < size; i++) {
        resHisText = resHisText + toolDisHistory.at(i);
    }

    // 去除末尾换行符
    resHisText.chop(1);

    return resHisText;
}

// 汇率更新函数
void ToolModelOutput::updateRate()
{
    toolIconUpdate->setEnabled(false);
    updateRateThread = new UpdateRateThread();
    updateRateThread->setObjectName("kylin-calculator_UpdateRateThread_updateRateThread");
    connect(updateRateThread, &UpdateRateThread::isDone, this, &ToolModelOutput::dealDone);
    updateRateThread->start();
    // 更新界面的数据 时间 汇率
    QDateTime date = QDateTime::currentDateTime();
    QString strTime = date.toString("yyyy.MM.dd hh:mm");

    toolLabTime->setText(strTime);
}

void ToolModelOutput::defUpdateRate()
{
    if (m_firstLoad) {
        m_firstLoad = false;

        //从配置文件中读
        QStringList getConfig;
        UpdateRateThread testUpdateRateThread;
        getConfig = testUpdateRateThread.getConfigData();
        //配置文件不在
        if (getConfig.isEmpty()) {
            // 2023-01-09
            QVector<QString> stringVector = {
                "CNY", "1",        "USD", "0.146488", "EUR", "0.137518", "JPY", "19.3412",  "HKD", "1.14376",
                "GBP", "0.121055", "MYR", "0.64286",  "RUB", "10.6215",  "AUD", "0.212689", "CAD", "0.196889",
                "NZD", "0.230712", "SGD", "0.195393", "CHF", "0.135827", "ZAR", "2.5086",   "KRW", "183.87",
                "AED", "0.53813",  "SAR", "0.55064",  "HUF", "54.2628",  "PLN", "0.64547",  "DKK", "1.0228",
                "SEK", "1.5425",   "NOK", "1.4661",   "TRY", "2.7498",   "MXN", "2.8046",   "THB", "4.9303"};
            getConfig = QStringList::fromVector(stringVector);
        }
        strRateList = getConfig;
        // 获取汇率单位标识
        QString strRateKey = strRateList.at(0);
        for (int i = 2; i < strRateList.size(); i++) {
            if (i % 2 == 0) {
                strRateKey.append(",");
                strRateKey.append(strRateList.at(i));
            }
        }

        // 初始化选择列表的汇率选项
        initUnitList(strRateKey);
    }

    // 更新界面的数据 时间 汇率
    QDateTime date = QDateTime::currentDateTime();
    QString strTime = date.toString("yyyy.MM.dd hh:mm");

    QString rateNameBef = toolRateSymbBef;
    QString rateNameAft = toolRateSymbAft;

    QString labelRate = "1 " + rateNameBef + " = " + QString::number(toolDouRate, 'f', 2) + " " + rateNameAft;
    toolLabTime->setText(strTime);
    toolLabRate->setText(labelRate);

    updateRateName();
}

/* 控制换算前的单位列表 */
void ToolModelOutput::unitListBefShow()
{
    if (!unitListAft->isHidden()) {
        unitListAft->hide();
    }
    if (unitListBef->isHidden()) {
        int b_x = toolUnitBefWid->x() + toolUnitBefWid->width();
        int b_y = toolUnitBefWid->y();
        if (this->parentWidget()) {
            QPoint globalPos = this->parentWidget()->mapToGlobal(QPoint(0,0));
            unitListBef->setGeometry(QRect(globalPos.x()+b_x, globalPos.y()+b_y, 280, 410));
        } else {
            unitListBef->setGeometry(QRect(b_x, b_y, 280, 410));
        }

        unitListBef->show();
//        unitListBef->raise();
//        unitListBef->setFocus();

        toolUnitBefWid->setBackgroundRole(QPalette::Window);
    } else {
        unitListBef->hide();
        toolUnitBefWid->setBackgroundRole(QPalette::Base);
    }
}

/* 控制换算后的单位列表 */
void ToolModelOutput::unitListAftShow()
{
    if (!unitListBef->isHidden()) {
        unitListBef->hide();
    }
    if (unitListAft->isHidden()) {
        int a_x = toolUnitAftWid->x() + toolUnitAftWid->width();
        int a_y = toolUnitAftWid->y();
        if (this->parentWidget()) {
            QPoint globalPos = this->parentWidget()->mapToGlobal(QPoint(0,0));
            unitListAft->setGeometry(QRect(globalPos.x()+a_x, globalPos.y()+a_y, 280, 410));
        } else {
            unitListAft->setGeometry(QRect(a_x, a_y, 280, 410));
        }
        unitListAft->show();
//        unitListAft->raise();
//        unitListAft->setFocus();
    } else {
        unitListAft->hide();
    }
}

void ToolModelOutput::listItemClicked(QListWidgetItem *item)
{
    UnitListWidget *sender = dynamic_cast<UnitListWidget *>(QObject::sender());
    if (sender == unitListAft) {
        toolRateSymbAft = item->data(Qt::DisplayRole + TAG_INFO).toString();
        updateRateName();
        unitListAft->hide();
    } else {
        toolRateSymbBef = item->data(Qt::DisplayRole + TAG_INFO).toString();
        updateRateName();
        unitListBef->hide();
    }
}

void ToolModelOutput::dealDone(QStringList list)
{
    if (list.isEmpty()) {
        return;
    }
    strRateList = list;
    // 获取汇率单位标识
    QString strRateKey = strRateList.at(0);
    for (int i = 2; i < strRateList.size(); i++) {
        if (i % 2 == 0) {
            strRateKey.append(",");
            strRateKey.append(strRateList.at(i));
        }
    }
    // 初始化选择列表的汇率选项
    initUnitList(strRateKey);

    // 更新界面的数据 时间 汇率
    QDateTime date = QDateTime::currentDateTime();
    QString strTime = date.toString("yyyy.MM.dd hh:mm");


    QString rateNameBef = toolRateSymbBef;
    QString rateNameAft = toolRateSymbAft;

    QString labelRate = "1 " + rateNameBef + " = " + QString::number(toolDouRate, 'f', 2) + " " + rateNameAft;
    toolLabTime->setText(strTime);
    toolLabRate->setText(labelRate);

    updateRateName();

    toolIconUpdate->setEnabled(true);

    //停止线程
    updateRateThread->quit();

    //等待线程处理完手头工作
    updateRateThread->wait();
}

// 根据符号更新货币名称并显示
void ToolModelOutput::updateRateName()
{
    /* 获取汇率索引值 */
    int idxRateBef = strRateList.indexOf(toolRateSymbBef) + 1;
    int idxRateAft = strRateList.indexOf(toolRateSymbAft) + 1;

    /* 获取汇率比率 */
    toolDouRate = (strRateList[idxRateAft]).toDouble() / (strRateList[idxRateBef]).toDouble();

    /* 获取汇率名称 */
    QMap<QString, QString>::iterator it;


    it = currencyInfoUS.find(toolRateSymbBef);
    if (it != currencyInfoUS.end()) {
        toolRateNameBef = it.value();
    }

    it = currencyInfoUS.find(toolRateSymbAft);
    if (it != currencyInfoUS.end()) {
        toolRateNameAft = it.value();
    }

    // 更新汇率比率显示
    QString labelRate = toolLabRate->text();
    labelRate = "1 " + toolRateSymbBef + " = " + QString::number(toolDouRate, 'f', 2) + " " + toolRateSymbAft;
    toolLabRate->setText(labelRate);

    // 更新汇率名称和符号显示
    toolLabUnitBef->setText(toolRateNameBef + "\n" + toolRateSymbBef);
    toolLabUnitAft->setText(toolRateNameAft + "\n" + toolRateSymbAft);

//    QFont font(toolLabUnitBef->font());
//    font.setPixelSize(18); // QFont像素大小设置为18，然后再次设置到QLabel中
//    toolLabUnitBef->setFont(font);
//    toolLabUnitAft->setFont(font);

    QFontMetrics fontmts = toolLabUnitBef->fontMetrics();
    int dif = fontmts.width(toolRateNameBef) - toolLabUnitBef->width();
    if (dif > 0) {
        QString str = fontmts.elidedText(toolRateNameBef, Qt::ElideRight, toolLabUnitBef->width());
        toolLabUnitBef->setText(str + "\n" + toolRateSymbBef);
        toolLabUnitBef->setToolTip(toolRateNameBef);
    }

    fontmts = toolLabUnitAft->fontMetrics();
    dif = fontmts.width(toolRateNameAft) - toolLabUnitAft->width();
    if (dif > 0) {
        QString str = fontmts.elidedText(toolRateNameAft, Qt::ElideRight, toolLabUnitAft->width());
        toolLabUnitAft->setText(str + "\n" + toolRateSymbAft);
        toolLabUnitAft->setToolTip(toolRateNameAft);
    }

    // 更新汇率换算结果
    unitConversion();
}

// 换算器按钮界面
ToolModelButton::ToolModelButton(QWidget *parent) : QWidget(parent)
{
    // 初始化组件
    this->setWidgetUi();
}

// 初始化组件
void ToolModelButton::setWidgetUi()
{
    for (int i = 0; i < 10; i++) {
        btnNum[i] = new BasicButton(this);
        btnNum[i]->setText(QString::number(i));
        btnNum[i]->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
        btnNum[i]->setIconSize(QSize(106, 62));
        btnNum[i]->setProperty("useButtonPalette", true);
        btnNum[i]->setAccessibleName(QString("kylin_calculator_BasicButton_btnNum%1").arg(i));
    }

    btnClear = new BasicButton(this);
    btnClear->setAccessibleName("kylin-calculator_BasicButton_btnClear");
    btnPer = new BasicButton(this);
    btnPer->setAccessibleName("kylin-calculator_BasicButton_btnPer");
    btnDelete = new BasicButton(this);
    btnDelete->setAccessibleName("kylin-calculator_BasicButton_btnDelete");

    btnDiv = new BasicButton(this);
    btnDiv->setAccessibleName("kylin-calculator_BasicButton_btnDiv");
    btnMulti = new BasicButton(this);
    btnMulti->setAccessibleName("kylin-calculator_BasicButton_btnMulti");
    btnSub = new BasicButton(this);
    btnSub->setAccessibleName("kylin-calculator_BasicButton_btnSub");
    btnAdd = new BasicButton(this);
    btnAdd->setAccessibleName("kylin-calculator_BasicButton_btnAdd");
    btnEqual = new BasicButton(this);
    btnEqual->setAccessibleName("kylin-calculator_BasicButton_btnEqual");

    btnPoint = new BasicButton(this);
    btnPoint->setAccessibleName("kylin-calculator_BasicButton_btnPoint");

    m_picturetowhite = new PictureToWhite();
    m_picturetowhite->setObjectName("kylin-calculator_PictureToWhite_m_picturetowhite");

    // 设置按钮的显示文本
    QString btnList = "C,÷,×,B,ｰ,+,=,%,.";
    QStringList btnNameList = btnList.split(",");
    int index = 0;

    btnClear->setText(btnNameList[index++]);
    btnDiv->setText(btnNameList[index++]);
    btnMulti->setText(btnNameList[index++]);
    btnDelete->setText(btnNameList[index++]);
    btnSub->setText(btnNameList[index++]);
    btnAdd->setText(btnNameList[index++]);
    btnEqual->setText(btnNameList[index++]);
    btnPer->setText(btnNameList[index++]);
    btnPoint->setText(btnNameList[index++]);

    for (int i = 0; i < 10; i++) {
        btnNum[i]->setIcon(QIcon(":/image/newIcon/standard/btnNum" + QString::number(i) + ".svg"));
    }

    btnClear->setIcon(QIcon(":/image/newIcon/standard/btnClear.svg"));
    btnDiv->setIcon(QIcon(":/image/newIcon/standard/btnDiv.svg"));
    btnMulti->setIcon(QIcon(":/image/newIcon/standard/btnMulti.svg"));
    btnDelete->setIcon(QIcon(":/image/newIcon/standard/btnDelete.svg"));
    btnSub->setIcon(QIcon(":/image/newIcon/standard/btnSub.svg"));
    btnAdd->setIcon(QIcon(":/image/newIcon/standard/btnAdd.svg"));
    btnEqual->setIcon(QIcon(":/image/newIcon/standard/btnEqual.svg"));
    btnPer->setIcon(QIcon(":/image/newIcon/standard/btnPer.svg"));
    btnPoint->setIcon(QIcon(":/image/newIcon/standard/btnPoint.svg"));

    btnClear->setIconSize(QSize(106, 62));
    btnDiv->setIconSize(QSize(106, 62));
    btnMulti->setIconSize(QSize(106, 62));
    btnDelete->setIconSize(QSize(85, 50));
    btnSub->setIconSize(QSize(106, 62));
    btnAdd->setIconSize(QSize(106, 62));
    btnEqual->setIconSize(QSize(106, 62));
    btnPer->setIconSize(QSize(106, 62));
    btnPoint->setIconSize(QSize(106, 62));

    // 设置按钮自适应放缩
    btnClear->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnDiv->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnMulti->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnDelete->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnSub->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnAdd->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnEqual->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    btnPoint->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

    btnClear->setProperty("useButtonPalette", true);
    btnDiv->setProperty("useButtonPalette", true);
    btnMulti->setProperty("useButtonPalette", true);
    btnDelete->setProperty("useButtonPalette", true);
    btnSub->setProperty("useButtonPalette", true);
    btnAdd->setProperty("useButtonPalette", true);
    btnPer->setProperty("useButtonPalette", true);
    btnPoint->setProperty("useButtonPalette", true);

    btnEqual->setProperty("isImportant", true);

    // 将按钮进行网格布局
    QGridLayout *btnLayout = new QGridLayout();
    btnLayout->addWidget(btnClear, 0, 0, 1, 1);
    btnLayout->addWidget(btnPer, 0, 1, 1, 1);
    btnLayout->addWidget(btnDelete, 0, 2, 1, 1);
    btnLayout->addWidget(btnDiv, 0, 3, 1, 1);
    btnLayout->addWidget(btnMulti, 1, 3, 1, 1);
    btnLayout->addWidget(btnSub, 2, 3, 1, 1);
    btnLayout->addWidget(btnAdd, 3, 3, 1, 1);
    btnLayout->addWidget(btnPoint, 4, 2, 1, 1);
    btnLayout->addWidget(btnEqual, 4, 3, 1, 1);

    // 数字按钮布局
    // btnLayout->addWidget(btnDZero, 4, 0, 1, 1);
    btnLayout->addWidget(btnNum[0], 4, 0, 1, 2);
    for (int i = 1; i < 10; i++) {
        btnLayout->addWidget(btnNum[i], 3 - (i - 1) / 3, (i - 1) % 3, 1, 1);
    }

    // 设置间距和背景样式
    btnLayout->setSpacing(2);
    btnLayout->setMargin(1);
    btnLayout->setContentsMargins(4, 0, 4, 4);

    // this->setFixedHeight(320);
    this->setLayout(btnLayout);
}

/* 键盘响应事件 */
void ToolModelButton::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Period:
        btnPoint->click();
        break;
    case Qt::Key_0:
        btnNum[0]->click();
        break;
    case Qt::Key_1:
        btnNum[1]->click();
        break;
    case Qt::Key_2:
        btnNum[2]->click();
        break;
    case Qt::Key_3:
        btnNum[3]->click();
        break;
    case Qt::Key_4:
        btnNum[4]->click();
        break;
    case Qt::Key_5:
        btnNum[5]->click();
        break;
    case Qt::Key_6:
        btnNum[6]->click();
        break;
    case Qt::Key_7:
        btnNum[7]->click();
        break;
    case Qt::Key_8:
        btnNum[8]->click();
        break;
    case Qt::Key_9:
        btnNum[9]->click();
        break;
    case Qt::Key_Plus:
        btnAdd->click();
        break;
    case Qt::Key_Minus:
        btnSub->click();
        break;
    case Qt::Key_Asterisk:
        btnMulti->click();
        break;
    case Qt::Key_Slash:
        btnDiv->click();
        break;
    case Qt::Key_Escape:
        btnClear->click();
        break;
    case Qt::Key_Percent:
        btnPer->click();
        break;
    case Qt::Key_Backspace:
        btnDelete->click();
        break;
    case Qt::Key_Enter:
        btnEqual->click();
        break;
    case 16777220:
        btnEqual->click();
        break;
    case 0x3d:
        btnEqual->click();
        break;
    default:
        return QWidget::keyPressEvent(event);
    }

    return;
}

UnitListWidget::UnitListWidget(QWidget *parent)
    : QWidget(parent)
    , m_listwidget(new QListWidget)
{
//    QGraphicsDropShadowEffect *shadowEffect = new QGraphicsDropShadowEffect(this);
//    shadowEffect->setOffset(0, 0);
//    shadowEffect->setColor(QGuiApplication::palette().color(QPalette::BrightText));
//    shadowEffect->setBlurRadius(8);
//    setGraphicsEffect(shadowEffect);

    m_listwidget->setFrameShape(QFrame::NoFrame);
    QPalette palette = m_listwidget->palette();
    palette.setColor(QPalette::Base, QGuiApplication::palette().color(QPalette::Base));
    m_listwidget->setPalette(palette);
    connect(m_listwidget, &QListWidget::itemClicked, this, &UnitListWidget::itemClicked);

    QHBoxLayout *hBoxLayout = new QHBoxLayout;
    hBoxLayout->setContentsMargins(5, 10, 5, 10);
    hBoxLayout->addWidget(m_listwidget);
    setLayout(hBoxLayout);
//    setAttribute(Qt::WA_TranslucentBackground, true);
    UkuiWindowHelper *windowhelper = new UkuiWindowHelper(this);
    UkuiWindowHelper::DecorationComponents comps;
    uint32_t decorationState = 0;
    comps |=  UkuiWindowHelper::DecorationComponent::RoundCorner;
    comps |=  UkuiWindowHelper::DecorationComponent::Shadow;
    windowhelper->setDecorationCompoents(comps);
    this->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
}

UnitListWidget::~UnitListWidget()
{
    /* nothing to do */
}

void UnitListWidget::insertItem(int row, QListWidgetItem *item)
{
    m_listwidget->insertItem(row, item);
}

void UnitListWidget::clear(void)
{
    m_listwidget->clear();
}

void UnitListWidget::setSpacing(int space)
{
    m_listwidget->setSpacing(space);
}

void UnitListWidget::setCurrentRow(int row)
{
    m_listwidget->setCurrentRow(row);
}

QListWidget *UnitListWidget::getListWidget(void)
{
    return m_listwidget;
}

void UnitListWidget::paintEvent(QPaintEvent *event)
{
    Q_UNUSED(event);

    QStyleOption opt;
    opt.init(this);
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);
    style()->drawPrimitive(QStyle::PE_FrameMenu, &opt, &painter, this);
    QPalette palette = m_listwidget->palette();
    palette.setColor(QPalette::Base, QGuiApplication::palette().color(QPalette::Base));
    m_listwidget->setPalette(palette);
}

//void UnitListWidget::focusOutEvent(QFocusEvent *event)
//{
//    if (hasFocus())
//        return;
//    hide();
//    if (m_win->hasFocus()) {
//        return;
//    }
//    this->hide();
//}

void UpdateRateThread::run()
{
    //读配置文件
    if (getConfigDataSuccess()) {
        return;
    }
    // 访问汇算换算api
    //    QString strUrl = "https://api.exchangerate-api.com/v4/latest/CNY";
    QString strUrl = "http://www.safe.gov.cn/AppStructured/hlw/RMBQuery.do";
    QNetworkAccessManager manager;
    QNetworkRequest netRequest;
    QNetworkReply *netReply;
    QEventLoop loop;
    m_timer = new QTimer();
    m_timer->setObjectName("kylin-calculator_QTimer_m_timer");
    connect(m_timer, SIGNAL(timeout()), &loop, SLOT(quit()));
    m_timer->start(10000);

    netRequest.setHeader(QNetworkRequest::ContentTypeHeader, "application/x-www-form-urlencoded");
    netRequest.setUrl(QUrl(strUrl));
    netReply = manager.get(netRequest);

    connect(netReply, SIGNAL(finished()), &loop, SLOT(quit()));
    loop.exec();

    m_timer->stop();

    if (netReply->error() != QNetworkReply::NoError) {
        return;
    }

    QString strRateAll = netReply->readAll();

    if (strRateAll == "") {
        return;
    }

    QStringList strList;
    QByteArray rateJsonBuf = strRateAll.toUtf8();

    //    QJsonDocument rateJsonDoc = QJsonDocument::fromJson(rateJsonBuf);
    //    if (rateJsonDoc.isObject()) {
    //        QJsonValue rateJsonValue = rateJsonDoc.object().value("rates");
    //        QJsonObject rateJsonObj = rateJsonValue.toObject();
    //        QStringList rateKeyList = rateJsonObj.keys();

    //        for (int i = 0; i < rateKeyList.size(); i++) {
    //            strList << rateKeyList.value(i);
    //            double rate = rateJsonObj.value(rateKeyList.value(i)).toDouble();
    //            strList << QString::number(rate);
    //        }
    //    }

    strList = getHtmlData(rateJsonBuf);

    emit isDone(strList);

    manager.deleteLater();
    netReply->deleteLater();
    loop.deleteLater();
}

QStringList UpdateRateThread::getConfigData()
{
    QFile file(m_configPath + "/" + m_configFileName);
    QString config;
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        config = file.readAll();
        file.close();
    }
    if (!config.isEmpty()) {
        QString nowStr = QDateTime::currentDateTime().toString("yyyy-MM-dd");
        QStringList configData = config.split("\n");
        if (configData.length() > m_moneyType) {
            if (nowStr == configData.first()) {
                configData.removeFirst();
                return configData;
            }
        }
    }
    return QStringList();
}

bool UpdateRateThread::getConfigDataSuccess()
{
    QStringList list = getConfigData();
    if (!list.isEmpty()) {
        emit isDone(list);
        return true;
    }
    return false;
}

QStringList UpdateRateThread::getHtmlData(const QString &str)
{
    QStringList header;
    header.append("日期");
    bool isHeader = false;
    QStringList value;
    bool isValue = false;
    QString data = str;
    data.remove('\t');
    data.remove('\r');
    QStringList list = data.split('\n');
    list.removeAll("");
    list.removeAll("</div>");
    list.removeAll("<tr>");
    list.removeAll("<th width=\"8%\" class=\"table_head\" style=\"cursor:'default'\" id=\"comtemplatename\" >");
    list.removeAll("<!-- <s:property  value=\"currency\"/>&nbsp; -->");
    list.removeAll("<td td width=\"8%\" align=\"center\" >");
    list.removeAll("</td>");
    list.removeAll("</th>");
    for (const QString &str : list) {
        if (str.contains(">日期<")) {
            isHeader = true;
        }
        if (isHeader && header.length() < m_moneyType) {
            if (isTableText(str)) {
                header.append(str);
            }
        }
        if (header.length() == m_moneyType && isDateTimeText(str) && isHeader) {
            isValue = true;
            isHeader = false;
            value.append(str);
        }
        if (isValue && value.length() < m_moneyType) {
            if (isNumberText(str)) {
                value.append(str);
            }
        }
        if (value.length() == m_moneyType) {
            isValue = false;
            break;
        }
    }
    return specificationFormat(header, value);
}

bool UpdateRateThread::isTableText(const QString &str)
{
    if (str.length() < 2) {
        return false;
    }
    for (const QChar &ch : str) {
        if (ch.isNumber() || ch.isLower()) {
            return false;
        }
    }
    return true;
}

bool UpdateRateThread::isDateTimeText(const QString &str)
{
    if (!str.contains('-') || str.length() != 10) {
        return false;
    }
    if (str.at(4) == '-' && str.at(7) == '-') {
        return true;
    }
    return false;
}

bool UpdateRateThread::isNumberText(const QString &str)
{
    for (const QChar &ch : str) {
        if (!ch.isNumber() && ch != '.') {
            return false;
        }
    }
    return true;
}

QStringList UpdateRateThread::specificationFormat(const QStringList &head, const QStringList &value)
{
    if (head.length() != value.length() || head.length() != m_moneyType) {
        return QStringList();
    }
    QStringList indirectPricing = CreatIndirectPricing();
    QMap<QString, QString> moneyTypeMap = creatMoneyTypeMap();
    QStringList result;
    if (!value.contains("CNY")) {
        result.append("CNY");
        result.append(QString::number(1));
    }
    for (int i = 1; i < head.length(); i++) {
        QString moneyType = head.at(i);
        QString abbreviation = moneyTypeMap.value(moneyType);
        if (abbreviation.isEmpty()) {
            continue;
        }
        QString rateStr = value.at(i);
        bool ok = false;
        double rate = rateStr.toDouble(&ok);
        if (!ok || rate == 0) {
            continue;
        }
        result.append(abbreviation);
        if (indirectPricing.contains(moneyType)) {
            rate = rate / 100;
        } else {
            rate = 100 / rate;
        }
        result.append(QString::number(rate));
    }
    saveToConfig(value.at(0), result);
    return result;
}

QStringList UpdateRateThread::CreatIndirectPricing()
{
    QStringList indirectPricing;
    indirectPricing << "林吉特"
                    << "卢布"
                    << "兰特"
                    << "韩元"
                    << "迪拉姆"
                    << "里亚尔"
                    << "福林"
                    << "兹罗提"
                    << "丹麦克朗"
                    << "瑞典克朗"
                    << "挪威克朗"
                    << "里拉"
                    << "比索"
                    << "泰铢";
    return indirectPricing;
}

QMap<QString, QString> UpdateRateThread::creatMoneyTypeMap()
{
    QMap<QString, QString> moneyTypeMap;
    moneyTypeMap.insert("美元", "USD");
    moneyTypeMap.insert("欧元", "EUR");
    moneyTypeMap.insert("日元", "JPY");
    moneyTypeMap.insert("港元", "HKD");
    moneyTypeMap.insert("英镑", "GBP");
    moneyTypeMap.insert("林吉特", "MYR");
    moneyTypeMap.insert("卢布", "RUB");
    moneyTypeMap.insert("澳元", "AUD");
    moneyTypeMap.insert("加元", "CAD");
    moneyTypeMap.insert("新西兰元", "NZD");
    moneyTypeMap.insert("新加坡元", "SGD");
    moneyTypeMap.insert("瑞士法郎", "CHF");
    moneyTypeMap.insert("兰特", "ZAR");
    moneyTypeMap.insert("韩元", "KRW");
    moneyTypeMap.insert("迪拉姆", "AED");
    moneyTypeMap.insert("里亚尔", "SAR");
    moneyTypeMap.insert("福林", "HUF");
    moneyTypeMap.insert("兹罗提", "PLN");
    moneyTypeMap.insert("丹麦克朗", "DKK");
    moneyTypeMap.insert("瑞典克朗", "SEK");
    moneyTypeMap.insert("挪威克朗", "NOK");
    moneyTypeMap.insert("里拉", "TRY");
    moneyTypeMap.insert("比索", "MXN");
    moneyTypeMap.insert("泰铢", "THB");
    return moneyTypeMap;
}

void UpdateRateThread::saveToConfig(const QString &dateTime, const QStringList &result)
{
    QDir().mkpath(m_configPath);
    QFile file(m_configPath + "/" + m_configFileName);
    if (file.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {
        file.write(dateTime.toLocal8Bit());
        for (const QString &str : result) {
            file.write("\n");
            file.write(str.toLocal8Bit());
        }
        file.close();
    }
}
