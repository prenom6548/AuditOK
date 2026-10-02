#include "infocreatewidget.h"

//create theme info graphical interface
InfoCreateWidget::InfoCreateWidget(QWidget *parent, const QString &m_state) :
    kdk::KDialog(parent)
{
    this->setWindowIcon(QIcon::fromTheme("kylin-theme-builder"));
    this->setWindowTitle(tr("Package Information"));
    QWidget*mainwindow = mainWidget();
    this->setFixedSize(424,444);
    this->setWindowModality(Qt::ApplicationModal);
    this->setFocus();

    InitThemeName();
    InitBuilderInfo();
//    InitDefaultInstallment();
    InitInfoBtn(m_state);
    InfoProcess();

    m_warningMessage = new QLabel();
    m_warningMessage->setText("");
    m_warningMessage2 = new QLabel();
    m_warningMessage2->setText("");
    m_warningMessage3 = new QLabel();
    m_warningMessage3->setText("");
    m_warningMessage4 = new QLabel();
    m_warningMessage4->setText("");
    m_warningMessage->setFixedHeight(20);
    m_warningMessage2->setFixedHeight(20);
    m_warningMessage3->setFixedHeight(20);
    m_warningMessage4->setFixedHeight(20);
    QFont font("Arial", 12);
    m_warningMessage->setFont(font);
    m_warningMessage2->setFont(font);
    m_warningMessage3->setFont(font);
    m_warningMessage4->setFont(font);

    QPushButton *line = new QPushButton();
    line->setDisabled(true);
    line->setFixedHeight(1);
    QPushButton *line1 = new QPushButton();
    line1->setDisabled(true);
    line1->setFixedHeight(1);
    QPushButton *line2 = new QPushButton();
    line2->setDisabled(true);
    line2->setFixedHeight(1);

    QVBoxLayout *m_infoCreateLayout = new QVBoxLayout(mainwindow);
    m_infoCreateLayout->addLayout(m_themeNameCNLayout);
    m_infoCreateLayout->addWidget(m_warningMessage);
    m_infoCreateLayout->addLayout(m_themeNameENLayout);
    m_infoCreateLayout->addWidget(m_warningMessage2);
    m_infoCreateLayout->addWidget(line);
    m_infoCreateLayout->addLayout(m_builderNameLayout);
    m_infoCreateLayout->addWidget(m_warningMessage3);
    m_infoCreateLayout->addLayout(m_builderMailLayout);
    m_infoCreateLayout->addWidget(m_warningMessage4);
//    m_infoCreateLayout->addWidget(line1);
//    m_infoCreateLayout->addLayout(m_defaultInstallLayout);
//    m_infoCreateLayout->addWidget(line2);
    m_infoCreateLayout->addLayout(m_infoBtnLayout);

    mainwindow->setContentsMargins(24,0,24,24);
//    this->setLayout(m_infoCreateLayout);
}

void InfoCreateWidget::InitThemeName(){

    m_themeNameCNLayout = new QHBoxLayout();
    m_themeNameCNLabel = new QLabel();
    m_themeNameCNLineEdit = new QLineEdit();
    m_themeNameCNLineEdit->setFixedSize(248,36);
    m_themeNameCNLabel->setFixedWidth(100);
    m_themeNameCNLineEdit->setEchoMode(QLineEdit::Normal);

    m_themeNameCNLayout->addWidget(m_themeNameCNLabel);
    m_themeNameCNLayout->addWidget(m_themeNameCNLineEdit);

    m_themeNameENLayout = new QHBoxLayout();
    m_themeNameENLabel = new QLabel();
    m_themeNameENLineEdit = new QLineEdit();
    m_themeNameENLineEdit->setFixedSize(248,36);
    m_themeNameENLabel->setFixedWidth(100);
    m_themeNameENLineEdit->setEchoMode(QLineEdit::Normal);

    m_themeNameENLayout->addWidget(m_themeNameENLabel);
    m_themeNameENLayout->addWidget(m_themeNameENLineEdit);

    QFont f(m_themeNameCNLabel->font());
    QFontMetrics fm(f);
    QString str1= fm.elidedText(tr("ThemeName-CN"), Qt::ElideRight, 100);
    QString str2= fm.elidedText(tr("ThemeName-EN"), Qt::ElideRight, 100);
    m_themeNameCNLabel->setText(str1);
    m_themeNameCNLabel->setToolTip(tr("ThemeName-CN"));
    m_themeNameENLabel->setText(str2);
    m_themeNameENLabel->setToolTip(tr("ThemeName-EN"));

    if (QGSettings::isSchemaInstalled("org.ukui.style")) {
        QGSettings *gSetting = new QGSettings("org.ukui.style", QByteArray(), this);
        connect(gSetting, &QGSettings::changed, this, [=](const QString &key) {
            if ("systemFontSize" == key) {
                updateLabelShow();
            }
        });
    }
}

void InfoCreateWidget::InitBuilderInfo(){

    m_builderNameLayout = new QHBoxLayout();
    m_builderNameLabel = new QLabel();
    m_builderNameLineEdit = new QLineEdit();
    m_builderNameLineEdit->setFixedSize(248,36);
    m_builderNameLabel->setText(tr("Builder Name"));
    m_builderNameLineEdit->setEchoMode(QLineEdit::Normal);

    m_builderNameLayout->addWidget(m_builderNameLabel);
    m_builderNameLayout->addStretch(1);
    m_builderNameLayout->addWidget(m_builderNameLineEdit);

    m_builderMailLayout = new QHBoxLayout();
    m_builderMailLabel = new QLabel();
    m_builderMailLineEdit = new QLineEdit();
    m_builderMailLineEdit->setFixedSize(248,36);
    m_builderMailLabel->setText(tr("Builder Mail"));
    m_builderMailLineEdit->setEchoMode(QLineEdit::Normal);

    m_builderMailLayout->addWidget(m_builderMailLabel);
    m_builderMailLayout->addStretch(1);
    m_builderMailLayout->addWidget(m_builderMailLineEdit);
}

void InfoCreateWidget::InitInfoBtn(const QString &m_state){

    m_infoBtnLayout = new QHBoxLayout();
    m_exitBtn = new QPushButton();
    m_recogniseBtn = new QPushButton();
    m_createDebBtn = new QPushButton();
    m_exitBtn->setText(tr("Back"));
    m_exitBtn->setFixedSize(88,33);
    m_recogniseBtn->setText(tr("OK"));
    m_recogniseBtn->setFixedSize(88,33);
    m_createDebBtn->setText(tr("Create"));
    m_createDebBtn->setFixedSize(88,33);
    QShortcut *shortcut = new QShortcut(QKeySequence(Qt::Key_Return), m_recogniseBtn);
    QObject::connect(shortcut, &QShortcut::activated, m_recogniseBtn, &QPushButton::click);
    QShortcut *shortcut1 = new QShortcut(QKeySequence(Qt::Key_Return), m_createDebBtn);
    QObject::connect(shortcut1, &QShortcut::activated, m_createDebBtn, &QPushButton::click);
    m_infoBtnLayout->addStretch(1);
    m_infoBtnLayout->addWidget(m_exitBtn);
    m_infoBtnLayout->addWidget(m_recogniseBtn);
    m_infoBtnLayout->addWidget(m_createDebBtn);

    if(m_state == "recognise"){
        m_recogniseBtn->setHidden(false);
        m_recogniseBtn->setEnabled(false);
        m_createDebBtn->setHidden(true);
    } else if(m_state == "create"){
        if (m_builderNameLineEdit->text() != nullptr && m_themeNameENLineEdit->text() != nullptr
                && m_builderMailLineEdit->text() != nullptr ){
            m_recogniseBtn->setHidden(true);
            m_createDebBtn->setHidden(false);
            m_createDebBtn->setEnabled(true);
        } else {
            m_recogniseBtn->setHidden(true);
            m_createDebBtn->setHidden(false);
            m_createDebBtn->setEnabled(false);
        }
    } else {
        qDebug() << m_state;
    }
}

void InfoCreateWidget::InfoProcess(){
    m_themeENNameState = false;
    m_builderMailState = false;
    m_builderNameState = false;
    // 监听文本变化
    QObject::connect(m_themeNameCNLineEdit, &QLineEdit::textChanged, [this]() {

        // 获取当前文本
        FileProcess::g_themeCNName = m_themeNameCNLineEdit->text();

        QRegExp englishRegexp("[\\x4e00-\\x9fa5]+");
        if(m_themeNameENLineEdit != nullptr){
            if (englishRegexp.exactMatch(FileProcess::g_themeCNName)){
                // 文本长度不超过15
                if (m_themeNameCNLineEdit->text().length() > 15) {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage->setPalette(m_fontqpe);
                    m_warningMessage->setText(tr("Please enter less than 15 characters"));

                } else {
                    m_warningMessage->setText("");
                }
            } else {
                if (m_themeNameCNLineEdit->text().isEmpty()) {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText, Qt::red);
                    m_warningMessage->setPalette(m_fontqpe);
                    m_warningMessage->setText(tr("Input is empty!"));
                }else{
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage->setPalette(m_fontqpe);
                    m_warningMessage->setText(tr("Input format error!"));
                }
            }
        } else {
            QPalette m_fontqpe;
            m_fontqpe.setColor(QPalette::WindowText,Qt::red);
            m_warningMessage->setPalette(m_fontqpe);
            m_warningMessage->setText(tr("Input is empty!"));
        }
    });

    QObject::connect(m_themeNameENLineEdit, &QLineEdit::textChanged, [this]() {
        FileProcess::g_themeENName = m_themeNameENLineEdit->text();
        QRegExp englishRegexp("[a-z0-9]+$");
        QRegExp capsEnglishRegexp("[A-Za-z0-9]+$");
        if(m_themeNameENLineEdit->text() != nullptr){
            if (englishRegexp.exactMatch(FileProcess::g_themeENName)){
                // 文本长度不超过15,不少于1
                if (m_themeNameENLineEdit->text().length() > 15) {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage2->setPalette(m_fontqpe);
                    m_warningMessage2->setText(tr("Please enter less than 15 characters"));
                    m_themeENNameState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                } else if(m_themeNameENLineEdit->text().length() <= 1){
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage2->setPalette(m_fontqpe);
                    m_warningMessage2->setText(tr("Please enter more than 1 characters"));
                    m_themeENNameState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                } else {
                    if (!FileCheck::isLegalThemeName(m_themeNameENLineEdit->text())){
                        QPalette m_fontqpe;
                        m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                        m_warningMessage2->setPalette(m_fontqpe);
                        m_warningMessage2->setText(tr("Please don't duplicate the name of a package that already exists!"));
                        m_themeENNameState = false;
                        m_recogniseBtn->setEnabled(false);
                        m_createDebBtn->setEnabled(false);
                    } else {
                        m_warningMessage2->setText("");
                        m_themeENNameState = true;
                    }
                }


                if (m_themeENNameState && m_builderMailState && m_builderNameState){
                    m_recogniseBtn->setEnabled(true);
                    m_createDebBtn->setEnabled(true);
                } else {
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                }
            } else {
                if(capsEnglishRegexp.exactMatch(FileProcess::g_themeENName)){
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage2->setPalette(m_fontqpe);
                    m_warningMessage2->setText(tr("Please keep theme names in lowercase!"));
                    m_themeENNameState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                } else {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage2->setPalette(m_fontqpe);
                    m_warningMessage2->setText(tr("Input format error!"));
                    m_themeENNameState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                }
            }
        } else {
            QPalette m_fontqpe;
            m_fontqpe.setColor(QPalette::WindowText,Qt::red);
            m_warningMessage2->setPalette(m_fontqpe);
            m_warningMessage2->setText(tr("Input is empty!"));
            m_themeENNameState = false;
            m_recogniseBtn->setEnabled(false);
            m_createDebBtn->setEnabled(false);
        }
    });

    QObject::connect(m_builderNameLineEdit, &QLineEdit::textChanged, [this]() {

        FileProcess::g_builderName = m_builderNameLineEdit->text();
        QRegExp englishRegexp("[A-Za-z0-9]+$");
        if(m_builderNameLineEdit->text() != nullptr){
            if (englishRegexp.exactMatch(FileProcess::g_builderName)){
                // 文本长度不超过15
                if (m_builderNameLineEdit->text().length() > 15) {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage3->setPalette(m_fontqpe);
                    m_warningMessage3->setText(tr("Please enter less than 15 characters"));
                    m_builderNameState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                } else {
                    m_warningMessage3->setText("");
                    m_builderNameState = true;
                }

                if (m_themeENNameState && m_builderMailState && m_builderNameState){
                    m_recogniseBtn->setEnabled(true);
                    m_createDebBtn->setEnabled(true);
                } else {
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                }
            } else {
                QPalette m_fontqpe;
                m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                m_warningMessage3->setPalette(m_fontqpe);
                m_warningMessage3->setText(tr("Input format error!"));
                m_builderNameState = false;
                m_recogniseBtn->setEnabled(false);
                m_createDebBtn->setEnabled(false);
            }
        } else {
            QPalette m_fontqpe;
            m_fontqpe.setColor(QPalette::WindowText,Qt::red);
            m_warningMessage3->setPalette(m_fontqpe);
            m_warningMessage3->setText(tr("Input is empty!"));
            m_builderNameState = false;
            m_recogniseBtn->setEnabled(false);
            m_createDebBtn->setEnabled(false);
        }
    });

    QObject::connect(m_builderMailLineEdit, &QLineEdit::textChanged, [this]() {

        FileProcess::g_builderMailName = m_builderMailLineEdit->text();
        QRegExp mailRegexp("\\b[A-Za-z0-9._%+-]+@[A-Za-z0-9.-]+\\.[a-z]{2,}\\b");
        if(m_builderMailLineEdit->text() != nullptr){
            if (mailRegexp.exactMatch(FileProcess::g_builderMailName)){

                // 文本长度不超过35
                if (m_builderMailLineEdit->text().length() > 35) {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage4->setPalette(m_fontqpe);
                    m_warningMessage4->setText(tr("Please enter less than 35 characters"));
                    m_builderMailState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                } else if (m_builderMailLineEdit->text().length() <= 6) {
                    QPalette m_fontqpe;
                    m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                    m_warningMessage4->setPalette(m_fontqpe);
                    m_warningMessage4->setText(tr("Please enter more than 6 characters"));
                    m_builderMailState = false;
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                } else {
                    m_warningMessage4->setText("");
                    m_builderMailState = true;
                }

                if (m_themeENNameState && m_builderNameState && m_builderMailState){
                    m_recogniseBtn->setEnabled(true);
                    m_createDebBtn->setEnabled(true);
                } else {
                    m_recogniseBtn->setEnabled(false);
                    m_createDebBtn->setEnabled(false);
                }
            } else {
                QPalette m_fontqpe;
                m_fontqpe.setColor(QPalette::WindowText,Qt::red);
                m_warningMessage4->setPalette(m_fontqpe);
                m_warningMessage4->setText(tr("Input format error!"));
                m_builderMailState = false;
                m_recogniseBtn->setEnabled(false);
                m_createDebBtn->setEnabled(false);
            }
        } else {
            QPalette m_fontqpe;
            m_fontqpe.setColor(QPalette::WindowText,Qt::red);
            m_warningMessage4->setPalette(m_fontqpe);
            m_warningMessage4->setText(tr("Input is empty!"));
            m_builderMailState = false;
            m_recogniseBtn->setEnabled(false);
            m_createDebBtn->setEnabled(false);
        }
    });

    //退出信息收集页面
    connect(m_exitBtn, &QPushButton::clicked,this,[=](){
        this->close();
    });

    //创建control/changelog文件
    connect(m_recogniseBtn, &QPushButton::clicked,this,[=](){
        //判断是否为空，不为空设置值为未命名x

        createControlChangelog(FileProcess::g_date);
        createMeson(FileProcess::g_date);
        emit readytoSave();
    });

    //获取已输入信息，再次确认后开始编译
    connect(m_createDebBtn,&QPushButton::clicked,this,[=](){
        createControlChangelog(FileProcess::g_date);
        createMeson(FileProcess::g_date);
        //修改focal.xml
        emit hasGetThemeName();
        //发送文件复制信号
        emit readytoBuild();
        //编包前检查
        buildCheckWidget *m_buildCheckWidget = new buildCheckWidget();
        m_buildCheckWidget->buildCheck(QDir::homePath() + "/.cache/theme-build/" + FileProcess::g_date + "/src");

        connect(m_buildCheckWidget, &buildCheckWidget::gohomesignals, this,[=](){
            //发送返回主界面信号给maininterface
            emit gohomesignals();
        });
    });
}

void InfoCreateWidget::InitDefaultInstallment()
{
    m_defaultInstallLayout = new QHBoxLayout();
    m_defaultInstallLabel = new QLabel();
    m_defaultInstallCheckBox = new QCheckBox();
    m_defaultInstallLabel->setText(tr("Set as a theme for system use during installation"));
    m_defaultInstallLayout->addWidget(m_defaultInstallCheckBox);
    m_defaultInstallLayout->addWidget(m_defaultInstallLabel);
    m_defaultInstallLayout->addStretch(1);

    connect(m_defaultInstallCheckBox,&QCheckBox::clicked,this,[=]{
        if(m_defaultInstallCheckBox->checkState()){
            FileProcess::g_isSetup = true;
        }else{
            //仅安装
            FileProcess::g_isSetup = false;
        }
    });
}

void InfoCreateWidget::renameDesktop()
{
    QString m_date= FileProcess::g_date;
    //修改desktop文件名为theme-name.desktop
    QFile m_desktop(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/application/theme.desktop");
    m_desktop.rename(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/application/"+FileProcess::g_themeENName+".desktop");

}

void InfoCreateWidget::createControlChangelog(const QString &m_date){
    //点击生成制作主题包按钮时，m_date将设为空
//    qDebug()<<m_date;
    if(m_date != nullptr){
        QFile m_control(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/debian/control");

        if (m_control.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {

//            qDebug()<< m_control;
            QTextStream m_controlStream(&m_control);
            m_controlStream << QStringLiteral("Source: ") + FileProcess::g_themeENName;
            m_controlStream << endl << QStringLiteral("Section: misc");
            m_controlStream << endl << QStringLiteral("Priority: optional");
            m_controlStream << endl << QStringLiteral("Maintainer: ") + FileProcess::g_builderName + QStringLiteral(" <") + FileProcess::g_builderMailName + QStringLiteral(">");
            m_controlStream << endl << QStringLiteral("Build-Depends: debhelper-compat (=12),meson,ninja-build");
            m_controlStream << endl << QStringLiteral("Standards-Version: 4.6.0");
            m_controlStream << endl << QStringLiteral("Rules-Requires-Root: no");
            m_controlStream << endl;
            m_controlStream << endl << QStringLiteral("Package: ") + FileProcess::g_themeENName;
            m_controlStream << endl << QStringLiteral("Architecture: all");
            m_controlStream << endl << QStringLiteral("Depends: ${misc:Depends},${shlibs:Depends}");
            m_controlStream << endl << QStringLiteral("Description: Theme Package ") + FileProcess::g_themeCNName;
        }

        QFile m_changelog(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/debian/changelog");

        if (m_changelog.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            QTextStream m_changelogStream(&m_changelog);
            m_changelogStream << FileProcess::g_themeENName + QStringLiteral(" (1.0) UNRELEASED; urgency=medium");
            m_changelogStream << endl;
            m_changelogStream << endl << QStringLiteral("  * Initial release");
            m_changelogStream << endl;
            m_changelogStream << endl << QStringLiteral(" -- ") + FileProcess::g_builderName + QStringLiteral(" <") + FileProcess::g_builderMailName
                                 + QStringLiteral(">  Wed, 30 Aug 2023 14:42:06 +0800");
        }

        this->close();
    }
}

//创建meson相关文件
void InfoCreateWidget::createMeson(const QString &m_date){
    //点击生成制作主题包按钮时，m_date将设为空
//    qDebug()<<m_date;
    if(m_date != nullptr){
        QFile m_meson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/meson.build");

        if (m_meson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)) {

//            qDebug()<< m_meson;
            QTextStream m_mesonStream(&m_meson);
            m_mesonStream << QStringLiteral("project('")+ FileProcess::g_themeENName +QStringLiteral("','c',default_options: ['prefix=/usr/'])");
            m_mesonStream << endl << QStringLiteral("components_d = ['globalTheme', 'iconTheme', 'cursorTheme','plymouthTheme','grubTheme','setupConfig','application']");
            m_mesonStream << endl;
            m_mesonStream << endl << QStringLiteral("foreach component: components_d");
            m_mesonStream << endl << QStringLiteral("if get_option(component)");
            m_mesonStream << endl << QStringLiteral("subdir(component)");
            m_mesonStream << endl << QStringLiteral("endif");
            m_mesonStream << endl << QStringLiteral("endforeach");
        }

        QFile m_mesonOption(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/meson_options.txt");

        if (m_mesonOption.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            QTextStream m_mesonOptionStream(&m_mesonOption);

            if(FileProcess::g_isSetup){
                if(FileProcess::g_createThemeType == "globalTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: true, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: true, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: true, description:'build ukui cursor theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: true, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: true, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: true, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "iconTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: true, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: false, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: false, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: false, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: true, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "cursorTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: false, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: true, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: false, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: false, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: true, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "plymouthTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: false, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: false, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: true, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: false, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: true, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "grubTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: false, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: false, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: false, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: true, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: true, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else {
                    qDebug() << FileProcess::g_createThemeType;
                }
            } else {
                if(FileProcess::g_createThemeType == "globalTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: true, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: true, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: true, description:'build ukui cursor theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: true, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: true, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: false, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "iconTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: true, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: false, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: false, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: false, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: false, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "cursorTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: false, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: true, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: false, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: false, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: false, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "plymouthTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: false, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: false, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: true, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: false, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: false, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else if(FileProcess::g_createThemeType == "grubTheme"){
                    m_mesonOptionStream << QStringLiteral("option('globalTheme', type: 'boolean', value: false, description:'build ukui gtk theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('iconTheme', type: 'boolean', value: false, description:'build ukui icons theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('cursorTheme', type: 'boolean', value: false, description:'build ukui sounds theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('plymouthTheme', type: 'boolean', value: false, description:'build ukui plymouth theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('grubTheme', type: 'boolean', value: true, description:'build ukui grub theme')");
                    m_mesonOptionStream << endl << QStringLiteral("option('setupConfig', type: 'boolean', value: false, description:'Set to default during installation')");
                    m_mesonOptionStream << endl << QStringLiteral("option('application', type: 'boolean', value: true, description:'theme desktop')");
                } else {
                    qDebug() << FileProcess::g_createThemeType;
                }
            }

         }

        if(FileProcess::g_createThemeType == "globalTheme"){
            QFile m_globalMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/globalTheme/meson.build");

            if (m_globalMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_globalMesonStream(&m_globalMeson);
                m_globalMesonStream << QStringLiteral("globaltheme_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/')");
                m_globalMesonStream << endl << QStringLiteral("platformconfig_dir = join_paths(get_option('prefix'), 'share/qt5-ukui-platformtheme/themeconfig/')");
                m_globalMesonStream << endl;
                QDir directory(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/globalTheme");
                QStringList fileList = directory.entryList(QDir::Files);
                if (fileList.contains("1-openkylin.png")) {
                    m_globalMesonStream << endl << QStringLiteral("global_d = ['theme.conf','preview.png','1-openkylin.png']");
                } else {
                    m_globalMesonStream << endl << QStringLiteral("global_d = ['theme.conf','preview.png','1-openkylin.jpg']");
                }
                m_globalMesonStream << endl;
                m_globalMesonStream << endl << QStringLiteral("foreach global: global_d");
                m_globalMesonStream << endl << QStringLiteral("  install_data(global, install_dir: globaltheme_dir/'")+ FileProcess::g_themeENName +QStringLiteral("')");
                m_globalMesonStream << endl << QStringLiteral("endforeach");
                m_globalMesonStream << endl;
                m_globalMesonStream << endl << QStringLiteral("install_data('")+FileProcess::g_themeENName+(".json', install_dir: platformconfig_dir)");

                m_globalMesonStream << endl;
                m_globalMesonStream << endl << QStringLiteral("subdir('wallpaper')");
            }
            QFile m_wallPaperXmlMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/globalTheme/wallpaper/meson.build");
            if (m_wallPaperXmlMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_wallPaperXmlMesonStream(&m_wallPaperXmlMeson);
                m_wallPaperXmlMesonStream << QStringLiteral("wallpaper_xml_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/")+ FileProcess::g_themeENName +QStringLiteral("')");
                m_wallPaperXmlMesonStream << endl<< QStringLiteral("focalwallpaper_xml_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/")+ FileProcess::g_themeENName +QStringLiteral("')");
                m_wallPaperXmlMesonStream << endl << QStringLiteral("install_data('kylin-theme-builder.xml', install_dir: wallpaper_xml_dir)");
                m_wallPaperXmlMesonStream << endl << QStringLiteral("install_data('kylin-theme-builder-focal.xml', install_dir: focalwallpaper_xml_dir)");
            }
        }

        if (FileProcess::g_createThemeType == "globalTheme" || FileProcess::g_createThemeType == "iconTheme"){
            QFile m_iconMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/iconTheme/meson.build");

            if (m_iconMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_iconMesonStream(&m_iconMeson);
                m_iconMesonStream << QStringLiteral("icons_dir = join_paths(get_option('prefix'), 'share/icons')");
                m_iconMesonStream << endl;
                m_iconMesonStream << endl << QStringLiteral("  install_subdir(");
                m_iconMesonStream << endl << QStringLiteral("    '")+FileProcess::g_themeENName+QStringLiteral("',");
                m_iconMesonStream << endl << QStringLiteral("    install_dir: icons_dir/'")+ FileProcess::g_themeENName +QStringLiteral("',");
                m_iconMesonStream << endl << QStringLiteral("    strip_directory: true");
                m_iconMesonStream << endl << QStringLiteral("  )");
            }
        }

        if(FileProcess::g_createThemeType == "globalTheme" || FileProcess::g_createThemeType == "cursorTheme"){
            QFile m_cursorbuildMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/cursorTheme/meson.build");

            if (m_cursorbuildMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_cursorbuildMesonStream(&m_cursorbuildMeson);
                m_cursorbuildMesonStream << QStringLiteral("subdir('")+FileProcess::g_themeENName+QStringLiteral("-cursor')");
            }
            QDir m_cursorMesonDir(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/cursorTheme/"+FileProcess::g_themeENName + "-cursor");
            m_cursorMesonDir.mkdir(m_cursorMesonDir.absolutePath());
            QFile m_cursorMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/cursorTheme/"+FileProcess::g_themeENName+"-cursor/meson.build");

            if (m_cursorMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_cursorMesonStream(&m_cursorMeson);
                m_cursorMesonStream << QStringLiteral("cursors_dir = join_paths(get_option('prefix'), 'share/icons')");
                m_cursorMesonStream << endl << QStringLiteral("run_command('cursor.sh')");
                m_cursorMesonStream << endl << QStringLiteral("install_data('index.theme', install_dir: cursors_dir/'")+FileProcess::g_themeENName+QStringLiteral("-cursor')");
                m_cursorMesonStream << endl << QStringLiteral("install_data('cursor.theme', install_dir: cursors_dir/'")+FileProcess::g_themeENName+QStringLiteral("-cursor')");
                m_cursorMesonStream << endl << QStringLiteral("install_subdir('cursors', install_dir: cursors_dir/'")+FileProcess::g_themeENName+QStringLiteral("-cursor')");

            }
        }

        if(FileProcess::g_createThemeType == "globalTheme" || FileProcess::g_createThemeType == "grubTheme"){
            QFile m_grubMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/grubTheme/meson.build");

            if (m_grubMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_grubMesonStream(&m_grubMeson);
                m_grubMesonStream << QStringLiteral("grub_dir = join_paths(get_option('prefix'), 'share/grub/themes/UKUI')");
                m_grubMesonStream << endl << QStringLiteral("install_data('background-")+FileProcess::g_themeENName+(".png', install_dir: grub_dir)");

            }
        }

        if(FileProcess::g_createThemeType == "globalTheme" || FileProcess::g_createThemeType == "plymouthTheme"){
            QFile m_plymouthMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/plymouthTheme/meson.build");

            if (m_plymouthMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
                QTextStream m_plymouthMesonStream(&m_plymouthMeson);
                m_plymouthMesonStream << QStringLiteral("plymouth_dir = join_paths(get_option('prefix'), 'share/plymouth/themes/")
                                         +FileProcess::g_themeENName+QStringLiteral("-logo')");
                m_plymouthMesonStream << endl << QStringLiteral("install_data('other/uk.png',install_dir: plymouth_dir)");
                m_plymouthMesonStream << endl << QStringLiteral("install_data('other/password_field.png',install_dir: plymouth_dir)");
                m_plymouthMesonStream << endl << QStringLiteral("install_data('other/progress_dot_off.png',install_dir: plymouth_dir)");
                m_plymouthMesonStream << endl << QStringLiteral("install_data('other/")+FileProcess::g_themeENName+("-logo.script',install_dir: plymouth_dir)");
                m_plymouthMesonStream << endl << QStringLiteral("install_data('other/")+FileProcess::g_themeENName+("-logo.plymouth',install_dir: plymouth_dir)");

                for (int var = 1; var <= 104; ++var) {
                    m_plymouthMesonStream << endl << QStringLiteral("install_data('")+QString::number(var)+QStringLiteral(".png',install_dir: plymouth_dir)");
                }
            }
        }
        QFile m_setupMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/setupConfig/meson.build");
        if (m_setupMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            QTextStream m_setupMesonStream(&m_setupMeson);
            m_setupMesonStream << endl << QStringLiteral("setupdesktop_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/")+ FileProcess::g_themeENName +QStringLiteral("')");
            m_setupMesonStream << endl << QStringLiteral("setupshell_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/")+ FileProcess::g_themeENName +QStringLiteral("')");
            m_setupMesonStream << endl << QStringLiteral("setupconf_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/")+ FileProcess::g_themeENName +QStringLiteral("')");
            m_setupMesonStream << endl << QStringLiteral("setupschemas_dir = join_paths(get_option('prefix'), 'share/config/globaltheme/")+ FileProcess::g_themeENName +QStringLiteral("')")<< endl;

            m_setupMesonStream << endl << QStringLiteral("install_data('kylin-theme-builder-change.desktop', install_dir: setupdesktop_dir)");
            m_setupMesonStream << endl << QStringLiteral("install_data('kylin-theme-builder-change', install_dir: setupshell_dir)");

            m_setupMesonStream << endl << QStringLiteral("install_data('theme-builder-change.conf', install_dir: setupconf_dir)");
            m_setupMesonStream << endl << QStringLiteral("install_data('40_kylin-theme-builder.gschema.override', install_dir: setupschemas_dir)");
        }

        renameDesktop();

        QFile m_applicationMeson(QDir::homePath() + "/.cache/theme-build/" + m_date + "/src/application/meson.build");
        if (m_applicationMeson.open(QIODevice::WriteOnly | QIODevice::Truncate | QIODevice::Text)){
            QTextStream m_applicationStream(&m_applicationMeson);
            m_applicationStream << endl << QStringLiteral("app_dir = join_paths('/usr/share/kylin-theme-builder/desktops/')");
            m_applicationStream << endl << QStringLiteral("install_data('")+FileProcess::g_themeENName+(".desktop', install_dir: app_dir)");
        }
        this->close();
    }
}

void InfoCreateWidget::getFileInfo(const QMap<QString, QVariant> &m_packageInfo){
    m_themeNameCNLineEdit->setText(m_packageInfo.value("key4").toString());
    m_themeNameENLineEdit->setText(m_packageInfo.value("key1").toString());
    m_builderNameLineEdit->setText(m_packageInfo.value("key2").toString());
    m_builderMailLineEdit->setText(m_packageInfo.value("key3").toString());
}

void InfoCreateWidget::updateLabelShow()
{
    QFont f(m_themeNameCNLabel->font());
    QFontMetrics fm(f);
    QString str1= fm.elidedText(tr("ThemeName-CN"), Qt::ElideRight, 100);
    QString str2= fm.elidedText(tr("ThemeName-EN"), Qt::ElideRight, 100);
    m_themeNameCNLabel->setText(str1);
    m_themeNameCNLabel->setToolTip(str1);
    m_themeNameENLabel->setText(str2);
    m_themeNameCNLabel->setToolTip(str2);
}

