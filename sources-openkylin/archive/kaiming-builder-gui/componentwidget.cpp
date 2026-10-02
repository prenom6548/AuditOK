#include "componentwidget.h"
#include "logic/KMBuilderManifest.h"
#include "buildoptionsdialog.h"
#include <QTimer>
#include <qglobal.h>

#include "messageboxutils.h"

#define INIT_LINE_EDIT(labelText, widget) \
    { \
        QLabel *label = new QLabel(labelText); \
        label->setFixedWidth(150); \
        widget = new QLineEdit(q_ptr); \
        widget->setFixedHeight(35); \
        widget->setPlaceholderText(labelText); \
    }
#define INIT_TEXT_EDIT(labelText, widget) \
    { \
        QLabel *label = new QLabel(labelText); \
        label->setFixedWidth(150); \
        widget = new QTextEdit(q_ptr); \
        widget->setFixedHeight(75); \
        widget->setPlaceholderText(labelText); \
    }
#define INIT_LABEL_TEXTEDIT(edit, placeholder) \
    { \
        edit = new QTextEdit(q_ptr); \
        edit->setFixedHeight(300); \
        edit->setPlaceholderText(placeholder); \
    }

class ComponentWidgetPrivate : public QObject
{
    Q_OBJECT

public:
    ComponentWidgetPrivate(ComponentWidget *parent)
            : q_ptr(parent), m_mode(ComponentWidget::AddMode) {}

    void setMode(ComponentWidget::Mode mode) {
        m_mode = mode;
    }

    ComponentWidget::Mode mode() const {
        return m_mode;
    }

    virtual ~ComponentWidgetPrivate();

    void setupUi();

private slots:
    void addSource();
    void addConfigopts();
    void removeConfigopts(QWidget *configoptsRow);

protected:
    void updateGeometry();

private:
    Q_DECLARE_PUBLIC(ComponentWidget)
    ComponentWidget * q_ptr;
    ComponentWidget::Mode m_mode;

    QVBoxLayout * m_componentLayout;

    QLineEdit *m_nameEdit;
    QComboBox *m_buildSystemCombo;

    QTabWidget *m_tabWidget ;
    QWidget * m_buildScriptTab;
    QWidget * m_preInstallTab;
    QWidget * m_postInstallTab;
    QWidget * m_cleanupTab;
    QTextEdit *m_buildScriptEdit;
    QTextEdit *m_preInstallEdit;
    QTextEdit *m_postInstallEdit;
    QLabel *m_labelAnnotationConfigOpts;
    QTextEdit *m_cleanupEdit;

    QLabel *m_labelConfigopts;
    QWidget *m_configoptsWidget;
    QVBoxLayout *m_configoptsLayout;

    QPushButton *m_addConfigoptsButton;
    QPushButton *m_buildOptionsDialogBtn;
    KMBuilderManifest::Component::BuildOptions m_buildOptions;
    // 功能区
    QPushButton * m_btnCancel;
    QPushButton * m_btnOk;

    QVBoxLayout *m_sourcesLayout;  // 源列表布局
    QPushButton *m_addSourceButton;

    void addSourceToUI(const KMBuilderManifest::Component::Source &source);
    void clearAllData();
    void saveAllData();
};

ComponentWidget::ComponentWidget(QWidget *parent)
    : QWidget(parent),
      d_ptr(new ComponentWidgetPrivate(this))
{
    Q_D(ComponentWidget);
    d->setupUi();
    //初始化清空数据
    d->clearAllData();

    connect(d->m_buildSystemCombo, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](int index) {
            QString buildsystem = d->m_buildSystemCombo->itemText(index);

            if (buildsystem == "simple") {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                d->m_tabWidget->setTabVisible(0, true);  // Show build-script tab
                d->m_tabWidget->setTabVisible(1, false); // Hide pre-install tab
                d->m_tabWidget->setTabVisible(2, false); // Hide post-install tab
                d->m_tabWidget->setTabVisible(3, false); // Hide cleanup tab
#else
                d->m_tabWidget->clear();
                d->m_tabWidget->addTab(d->m_buildScriptTab, "build-script");
#endif

                d->m_labelConfigopts->setVisible(false);
                d->m_labelAnnotationConfigOpts->setVisible(false);
                d->m_configoptsWidget->setVisible(false);
                d->m_addConfigoptsButton->setVisible(false);
                d->m_buildOptionsDialogBtn->setVisible(false);
            } else {
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
                d->m_tabWidget->setTabVisible(0, false); // Hide build-script tab
                d->m_tabWidget->setTabVisible(1, true);  // Show pre-install tab
                d->m_tabWidget->setTabVisible(2, true);  // Show post-install tab
                d->m_tabWidget->setTabVisible(3, true);  // Show cleanup tab
#else
                d->m_tabWidget->clear();
                d->m_tabWidget->addTab(d->m_preInstallTab, "pre-install");
                d->m_tabWidget->addTab(d->m_postInstallTab, "post-install");
                d->m_tabWidget->addTab(d->m_cleanupTab, "cleanup");
#endif

                d->m_labelConfigopts->setVisible(true);
                d->m_labelAnnotationConfigOpts->setVisible(true);
                d->m_configoptsWidget->setVisible(true);
                d->m_addConfigoptsButton->setVisible(true);
                d->m_buildOptionsDialogBtn->setVisible(true);
            }
        });
}

ComponentWidget::~ComponentWidget()
{
}

void ComponentWidget::setMode(Mode mode)
{
    Q_D(ComponentWidget);
    d->setMode(mode);
}


ComponentWidgetPrivate::~ComponentWidgetPrivate()
{}

void ComponentWidgetPrivate::setupUi()
{
    Q_Q(ComponentWidget);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *areaWidget = new QWidget;
    scrollArea->setWidget(areaWidget);
    m_componentLayout = new QVBoxLayout;
    m_componentLayout->setSpacing(15);
    areaWidget->setLayout(m_componentLayout);

    // 操作按钮组
    QWidget *headerWidget = new QWidget;
    QHBoxLayout * operationLayout = new QHBoxLayout;
    {
        m_btnCancel = new QPushButton;
        m_btnCancel->setText(tr("Cancel"));
        connect(m_btnCancel, &QPushButton::clicked, this, [this]() {
            clearAllData();                // 先清除数据
            Q_EMIT q_ptr->cancelComponentClicked(); // 再触发信号切换页面
        });

        m_btnOk = new QPushButton;
        m_btnOk->setText(tr("OK"));

        connect(m_btnOk, &QPushButton::clicked, this, [=]() {
            if (m_nameEdit->text().isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set component name"),
                                                                  QObject::tr("OK"));
                return;
            }

            if (m_buildSystemCombo->currentText() == "simple" && m_buildScriptEdit->toPlainText().isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("When buildsystem is simple, must set build-script"),
                                                                  QObject::tr("OK"));
                return;
            }
            int sources_size = m_sourcesLayout->count();
            if (sources_size == 0){
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set sources"),
                                                                  QObject::tr("OK"));
                return;
            }
            saveAllData();                 // 先保存数据
            Q_EMIT q_ptr->okComponentClicked();    // 再触发信号切换页面
        });
        operationLayout->addStretch();
        operationLayout->addWidget(m_btnCancel);
        operationLayout->addWidget(m_btnOk);
        operationLayout->setSpacing(8);
        operationLayout->setContentsMargins(0,0,0,0);
    }
    headerWidget->setLayout(operationLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(scrollArea);
    mainLayout->addWidget(headerWidget);
    q->setLayout(mainLayout);
    mainLayout->setContentsMargins(24, 0, 24, 24);
    mainLayout->setSpacing(24);

    {
        QFormLayout *formLayout = new QFormLayout;

        INIT_LINE_EDIT("Name", m_nameEdit);
        m_nameEdit->setPlaceholderText(tr("The name of building component. "
                                          "Example: "
                                          "demo"));
        m_nameEdit->setToolTip(tr("The name of building component.\n"
                                  "Example:\n"
                                  "demo"));
        formLayout->addRow(tr("name") + "<font color = red>*</font>", m_nameEdit);

        m_buildSystemCombo = new QComboBox;
        m_buildSystemCombo->addItems({"autotools", "cmake", "cmake-ninja", "meson", "qmake", "simple"});
        m_buildSystemCombo->setToolTip(tr("1. The source code root directory corresponding to the building component contains the 'CMakeLists. txt' file, which is suitable for selecting 'cmake' and 'cmake ninja' to build the system. \n"
                                          "2. There is a 'meson. build' file in the root directory of the source code corresponding to the building component, which is suitable for selecting 'meson' to build the system.\n"
                                          "3. There is a '*. pro' file in the root directory of the source code (there can only be one. pro file), which is suitable for choosing 'qmake' to build the system.\n"
                                          "4. There are 'configure' and 'autogen' in the root directory of the source code One of the files' autogen. sh ',' bootstrap ', or' bootstrap. sh 'is suitable for choosing' autotools' to build a system.\n"
                                          "5. In all other cases, you can choose 'implement' to build the system and customize the software build and installation behavior in the 'build script' property.\n"
                                          ));
        formLayout->addRow(tr("buildsystem") + "<font color = red>*</font>", m_buildSystemCombo);

        m_tabWidget =new QTabWidget;
        m_tabWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        m_tabWidget->setFixedHeight(360);  // 设置整个控件高度
        {
            m_buildScriptTab = new QWidget;
            QVBoxLayout *buildScriptLayout = new QVBoxLayout(m_buildScriptTab);

            QString placeholderText1 = tr("Custom build and installation behavior implemented through shell scripts.\n"
                                          "Note: The software installation path should be in the /opt/apps/${APP-ID}/files directory, not in /usr. \n"
                                           "Eaxmple:\n"
                                           "set -e\n"
                                           "./configure --prefix==${APP_INSTALL_PREFIX}\n"
                                           "make -j8\n"
                                           "make install");
            INIT_LABEL_TEXTEDIT( m_buildScriptEdit, placeholderText1);
            m_buildScriptEdit->setToolTip(tr("Custom build and installation behavior implemented through shell scripts.\n"
                                             "Note: The software installation path should be in the /opt/apps/${APP-ID}/files directory, not in /usr. \n"
                                              "Eaxmple:\n"
                                              "set -e\n"
                                              "./configure --prefix==${APP_INSTALL_PREFIX}\n"
                                              "make -j8\n"
                                              "make install"));
            buildScriptLayout->addWidget(m_buildScriptEdit);
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
            m_tabWidget->addTab(m_buildScriptTab, "build-script");
#endif
        }
        {
            m_preInstallTab = new QWidget;
            QVBoxLayout *preInstallLayout = new QVBoxLayout(m_preInstallTab);
            QString placeholderText2 = tr("After make, execute a shell script before make install. "
                                          "Example: \n"
                                          "set -e\n"
                                          "mv hello.sh hello");
            INIT_LABEL_TEXTEDIT(m_preInstallEdit, placeholderText2);
            m_preInstallEdit->setToolTip(tr("After make, execute a shell script before make install. "
                                            "Example: \n"
                                            "set -e\n"
                                            "mv hello.sh hello"));
            preInstallLayout->addWidget(m_preInstallEdit);
            m_tabWidget->addTab(m_preInstallTab, "pre-install");
        }
        {
            m_postInstallTab = new QWidget;
            QVBoxLayout *postInstallLayout = new QVBoxLayout(m_postInstallTab);
            QString placeholderText3 = tr("A shell script executed after make install or ninja install. \n"
                                          "Note: The software installation path should be in the /opt/apps/${APP-ID}/files directory, not in /usr. \n"
                                          "Example: \n"
                                          "set -e\n"
                                          "install -D code.sh ${APP_INSTALL_PREFIX}/bin/code"
                                          );
            INIT_LABEL_TEXTEDIT(m_postInstallEdit, placeholderText3);
            m_postInstallEdit->setToolTip(tr("A shell script executed after make install or ninja install. \n"
                                             "Note: The software installation path should be in the /opt/apps/${APP-ID}/files directory, not in /usr. \n"
                                             "Example: \n"
                                             "set -e\n"
                                             "install -D code.sh ${APP_INSTALL_PREFIX}/bin/code"
                                             ));
            postInstallLayout->addWidget(m_postInstallEdit);
            m_tabWidget->addTab(m_postInstallTab, "post-install");
        }
        {
            m_cleanupTab = new QWidget;
            QVBoxLayout *cleanupLayout = new QVBoxLayout(m_cleanupTab);
            QString placeholderText = tr("Files to be deleted after the software package is builded. \n"
                                         "Example: \n"
                                         "/include \n "
                                         "*.h");
            INIT_LABEL_TEXTEDIT(m_cleanupEdit, placeholderText);
            m_cleanupEdit->setToolTip(tr("Files to be deleted after the software package is builded.\n "
                                         "Wildcard characters are supported. Separated by newlines.\n"
                                         "Example:\n"
                                         "/include \n "
                                         "*.h"));
            cleanupLayout->addWidget(m_cleanupEdit);
            m_tabWidget->addTab(m_cleanupTab, "cleanup");
        }
#if QT_VERSION >= QT_VERSION_CHECK(5, 15, 0)
        m_tabWidget->setTabVisible(0, false); // Hide build-script tab
#endif
        formLayout->addRow(m_tabWidget);

        m_componentLayout->addLayout(formLayout);
    }
    {
        // ConfigOpts section
        m_labelConfigopts = new QLabel(tr("config-opts"));
        m_labelConfigopts->setToolTip(tr("An array of options passed to the configuration session.\n"
                                    "For example, the configuration part of the cmake build is cmake .\n"
                                    "Example:\n"
                                    " -DBUILD_TESTING=OFF\n"
                                    " -DCMAKE_INSTALL_PREFIX:PATH=/opt/apps/myappid/files\n"
                                    " --disable-static"));

        m_componentLayout->addWidget(m_labelConfigopts);

        m_labelAnnotationConfigOpts = new QLabel(tr("An array of options passed to the configuration session(such as qmake xxx.pro | ./configure). "
                                                    "Example: "
                                                    "-DBUILD_TESTING=OFF"
                                                    ));
        m_labelAnnotationConfigOpts->setStyleSheet("color: gray");
        m_labelAnnotationConfigOpts->setWordWrap(true);
        m_componentLayout->addWidget(m_labelAnnotationConfigOpts);

        m_configoptsWidget = new QWidget();
        m_componentLayout->addWidget(m_configoptsWidget);

        m_configoptsLayout = new QVBoxLayout(m_configoptsWidget);
        // 调整容器和布局的尺寸策略
        m_configoptsWidget->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
        m_configoptsLayout->setAlignment(Qt::AlignTop);

        {
            m_addConfigoptsButton = new QPushButton(tr("Add config-opts"));
            connect(m_addConfigoptsButton, &QPushButton::clicked, this, &ComponentWidgetPrivate::addConfigopts);
            m_componentLayout->addWidget(m_addConfigoptsButton);
        }
    }
    {
        m_buildOptionsDialogBtn = new QPushButton(tr("Set build-options"));
        m_componentLayout->addWidget(m_buildOptionsDialogBtn);

        // 连接按钮点击信号
        connect(m_buildOptionsDialogBtn, &QPushButton::clicked, [this]() {
            BuildOptionsDialog *dialog = new BuildOptionsDialog(q_ptr);
            dialog->setBuildOptionsData(m_buildOptions);
            if (dialog->exec() == QDialog::Accepted) {
                m_buildOptions = dialog->getBuildOptionsData();
                qDebug() << "构建配置已更新";
            }
        });
    }

    // 新增源管理区域
    {
        QHBoxLayout *sourceHeaderLayout = new QHBoxLayout;
        QLabel *sourceLabel = new QLabel(tr("sources")+ "<font color = red>*</font>");
        QLabel *annotationSourceLabel = new QLabel(tr("A list of source codes to be downloaded and extracted for the component.\n"
                                                   "During the build process, "
                                                   "the source code corresponding to all sources of the same component will be copied to the same directory within the project directory."));

        annotationSourceLabel->setStyleSheet("color: gray");
        annotationSourceLabel->setWordWrap(true);
        sourceLabel->setToolTip(tr("A list of source codes to be downloaded and extracted for the component.\n"
                                   "During the build process, "
                                   "the source code corresponding to all sources of the same component will be copied to the same directory within the project directory."));
        m_addSourceButton = new QPushButton(tr("Add source"));
        connect(m_addSourceButton, &QPushButton::clicked, this, &ComponentWidgetPrivate::addSource);
        sourceHeaderLayout->addWidget(sourceLabel);
        sourceHeaderLayout->addStretch();
        sourceHeaderLayout->addWidget(m_addSourceButton);
        m_componentLayout->addLayout(sourceHeaderLayout);
        m_componentLayout->addWidget(annotationSourceLabel);

        // 创建直接承载源列表的容器
        QWidget *sourceContainer = new QWidget;
        sourceContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred); // 允许横向拉伸
        m_sourcesLayout = new QVBoxLayout(sourceContainer);
        m_sourcesLayout->setContentsMargins(0, 5, 0, 0); // 添加5px顶部间距
        m_sourcesLayout->setSpacing(3);    // 设置条目间距
        m_componentLayout->addWidget(sourceContainer);
    }
    q_ptr->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
    // 添加这行保证初始布局正确
    QTimer::singleShot(0, this, &ComponentWidgetPrivate::updateGeometry);
}

void ComponentWidgetPrivate::addSourceToUI(const KMBuilderManifest::Component::Source &source)
{
    // 创建 YAML 节点并转换
    YAML::Node yamlNode;
    KMBuilderManifest::sourceToYaml(yamlNode, source); // 直接调用静态方法
    QString yamlStr = KMBuilderManifest::nodeToString(yamlNode);

    // 创建显示控件
    QWidget *sourceRow = new QWidget;
    QHBoxLayout *rowLayout = new QHBoxLayout(sourceRow);

    // 显示YAML内容
    QTextEdit *yamlView = new QTextEdit();
    yamlView->setPlainText(yamlStr.isEmpty() ? "No data" : yamlStr);
    yamlView->setReadOnly(true);
    rowLayout->addWidget(yamlView);

    // 新增上移下移按钮
    QPushButton *upBtn = new QPushButton("↑");
    upBtn->setFixedSize(24, 24);

    QPushButton *downBtn = new QPushButton("↓");
    downBtn->setFixedSize(24, 24);

    // 上移按钮处理
    connect(upBtn, &QPushButton::clicked, [=]() {
        QWidget *currentRow = sourceRow;  // 直接使用捕获的源行对象
        int currentIndex = m_sourcesLayout->indexOf(currentRow);

        if (currentIndex > 0) {
            // 取出并删除旧布局项
            QLayoutItem *oldItem = m_sourcesLayout->takeAt(currentIndex);
            QWidget *currentWidget = oldItem->widget();
            currentWidget->setParent(nullptr);
            delete oldItem;

            // 交换位置
            m_sourcesLayout->insertWidget(currentIndex - 1, currentWidget);

            // 确保控件归属正确
            currentWidget->setParent(m_sourcesLayout->parentWidget());
        }
    });

    // 下移按钮处理
    connect(downBtn, &QPushButton::clicked, [=]() {
        QWidget *currentRow = sourceRow;  // 直接使用捕获的源行对象
        int currentIndex = m_sourcesLayout->indexOf(currentRow);

        if (currentIndex < m_sourcesLayout->count() - 1) {
            // 取出并删除旧布局项
            QLayoutItem *oldItem = m_sourcesLayout->takeAt(currentIndex);
            QWidget *currentWidget = oldItem->widget();
            currentWidget->setParent(nullptr);
            delete oldItem;

            // 交换位置
            m_sourcesLayout->insertWidget(currentIndex + 1, currentWidget);

            // 确保控件归属正确
            currentWidget->setParent(m_sourcesLayout->parentWidget());
        }
    });


    // 编辑按钮
    QPushButton *editBtn = new QPushButton(tr("Edit"));
    editBtn->setFixedSize(60, 24);

    connect(editBtn, &QPushButton::clicked, [=](){
        // 获取当前源数据
        QVariant var = sourceRow->property("sourceData");
        if (var.canConvert<KMBuilderManifest::Component::Source>()) {
            KMBuilderManifest::Component::Source source = var.value<KMBuilderManifest::Component::Source>();

            // 弹出编辑对话框
            SourceDialog dlg(m_nameEdit->text(), q_ptr);
            dlg.setSourceData(source); // 设置当前源数据
            if (dlg.exec() == QDialog::Accepted) {
                // 获取编辑后的源数据
                KMBuilderManifest::Component::Source updatedSource = dlg.getSourceData();

                // 更新源数据
                sourceRow->setProperty("sourceData", QVariant::fromValue(updatedSource));

                // 更新 YAML 显示
                YAML::Node yamlNode;
                KMBuilderManifest::sourceToYaml(yamlNode, updatedSource);
                QString yamlStr = KMBuilderManifest::nodeToString(yamlNode);
                yamlView->setPlainText(yamlStr.isEmpty() ? "No data" : yamlStr);
            }
        }
    });

    // 删除按钮
    QPushButton *removeBtn = new QPushButton(tr("×"));
    removeBtn->setFixedSize(24, 24);
    removeBtn->setStyleSheet("color: red;");

    connect(removeBtn, &QPushButton::clicked, [=]{
        m_sourcesLayout->removeWidget(sourceRow);
        sourceRow->deleteLater();
    });
    rowLayout->addWidget(upBtn);
    rowLayout->addWidget(downBtn);
    rowLayout->addWidget(editBtn);
    rowLayout->addWidget(removeBtn);

    m_sourcesLayout->addWidget(sourceRow);

    // 存储原始数据
    sourceRow->setProperty("sourceData", QVariant::fromValue(source));
}

void ComponentWidgetPrivate::addSource()
{
    Q_Q(ComponentWidget);
    QString name = m_nameEdit->text();
    if (name.isEmpty()) {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(q, QObject::tr("Warning"),
                                                 QObject::tr("Please set component name first"),
                                                 QObject::tr("OK"));
        m_nameEdit->setFocus();

        return ;
    }

    SourceDialog dlg(name, q_ptr);
    if (dlg.exec() == QDialog::Accepted) {
        KMBuilderManifest::Component::Source source = dlg.getSourceData();
        addSourceToUI(source); // 调用复用函数
    }
}

void ComponentWidgetPrivate::addConfigopts()
{
    QWidget *configoptsRow = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(configoptsRow);

    QLineEdit *configoptsInput = new QLineEdit(configoptsRow);
    configoptsInput->setPlaceholderText(tr("--disable-static"));

    QPushButton *removeButton = new QPushButton("×", configoptsRow);
    removeButton->setFixedSize(24, 24);

    layout->addWidget(configoptsInput);
    layout->addWidget(removeButton);
    configoptsRow->setLayout(layout);
    m_configoptsLayout->addWidget(configoptsRow);
    updateGeometry();

    // 绑定删除事件
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        removeConfigopts(configoptsRow);
    });
}

void ComponentWidgetPrivate::removeConfigopts(QWidget *configoptsRow)
{
    m_configoptsLayout->removeWidget(configoptsRow);
    delete configoptsRow;

    // 添加延迟更新防止界面闪烁
    QTimer::singleShot(10, this, &ComponentWidgetPrivate::updateGeometry);
}

void ComponentWidgetPrivate::saveAllData()
{
    Q_Q(ComponentWidget);
    KMBuilderManifest::Component component;
    component.m_name = m_nameEdit->text();
    component.m_buildSystem = m_buildSystemCombo->currentText();
    component.m_buildScript = m_buildScriptEdit->toPlainText();
    component.m_preInstall = m_preInstallEdit->toPlainText();
    component.m_postInstall = m_postInstallEdit->toPlainText();
#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    component.m_cleanup = QVector<QString>::fromList(m_cleanupEdit->toPlainText().split('\n', Qt::SkipEmptyParts));
#else
    component.m_cleanup = QVector<QString>::fromList(m_cleanupEdit->toPlainText().split('\n', QString::SkipEmptyParts));
#endif

    // 收集配置选项
    for (int i = 0; i < m_configoptsLayout->count(); ++i) {
        if (QWidget *row = m_configoptsLayout->itemAt(i)->widget()) {
            if (QLineEdit *edit = row->findChild<QLineEdit*>()) {
                if (!edit->text().isEmpty()) {
                    component.m_configOpts.append(edit->text());
                }
            }
        }
    }

    component.m_buildOptions =m_buildOptions;

    for (int i = 0; i < m_sourcesLayout->count(); ++i) {
        if (QWidget *row = m_sourcesLayout->itemAt(i)->widget()) {
            QVariant var = row->property("sourceData");
            if (var.canConvert<KMBuilderManifest::Component::Source>()) {
                component.m_sources.append(var.value<KMBuilderManifest::Component::Source>());
            }
        }
    }
    emit q->componentSaved(component);
}

void clearLayout(QLayout* layout) {
    if (!layout) return;

    QLayoutItem *item;
    while ((item = layout->takeAt(0))) {
        if (QWidget* widget = item->widget()) {
            widget->setParent(nullptr); // 解除父子关系
            widget->deleteLater(); // 安全地删除控件
        }
        if (QLayout* childLayout = item->layout()) {
            clearLayout(childLayout); // 递归清理子布局
        }
        delete item; // 删除布局项
    }
}
//编辑后更新Component
void ComponentWidget::loadComponentData(const KMBuilderManifest::Component &component)
{
    Q_D(ComponentWidget);
    // 加载基本字段
    d->m_nameEdit->setText(component.m_name);
    d->m_buildSystemCombo->setCurrentText(component.m_buildSystem);
    d->m_buildScriptEdit->setPlainText(component.m_buildScript);
    d->m_preInstallEdit->setPlainText(component.m_preInstall);
    d->m_postInstallEdit->setPlainText(component.m_postInstall);

    // 加载 cleanup 字段
    QStringList cleanupList = component.m_cleanup.toList();
    QString cleanupText = cleanupList.join("\n");
    d->m_cleanupEdit->setPlainText(cleanupText);

    // 加载 configOpts 字段
    clearLayout(d->m_configoptsLayout); // 先清除现有的配置选项
    for (const QString &configOpt : component.m_configOpts) {
        QWidget *configoptsRow = new QWidget();
        QHBoxLayout *layout = new QHBoxLayout(configoptsRow);

        QLineEdit *configoptsInput = new QLineEdit(configoptsRow);
        configoptsInput->setText(configOpt);

        QPushButton *removeButton = new QPushButton("×", configoptsRow);
        removeButton->setFixedSize(24, 24);

        layout->addWidget(configoptsInput);
        layout->addWidget(removeButton);
        configoptsRow->setLayout(layout);
        d->m_configoptsLayout->addWidget(configoptsRow);

        // 绑定删除事件
        connect(removeButton, &QPushButton::clicked, d, [=]() {
            d->removeConfigopts(configoptsRow);
        });
    }
    // 加载buildOptions
    d->m_buildOptions = component.m_buildOptions;

    // 加载 sources 字段
    clearLayout(d->m_sourcesLayout); // 先清除现有的源
    for (const KMBuilderManifest::Component::Source &source : component.m_sources)
    {
        d->addSourceToUI(source);
    }
}

void ComponentWidgetPrivate::clearAllData()
{
    // 清除所有输入框内容（补充遗漏字段）
    m_nameEdit->clear();
    m_buildSystemCombo->setCurrentText("autotools");
    m_buildScriptEdit->clear();
    m_preInstallEdit->clear();
    m_postInstallEdit->clear();
    m_cleanupEdit->clear();

    // 检查布局是否有效
    if (m_configoptsLayout && m_configoptsLayout->count() > 0) {
        clearLayout(m_configoptsLayout);
    }

    m_buildOptions = KMBuilderManifest::Component::BuildOptions();

    if (m_sourcesLayout && m_sourcesLayout->count() > 0) {
        clearLayout(m_sourcesLayout);
    }

}
void ComponentWidget::cleanup()
{
    Q_D(ComponentWidget);
    d->clearAllData(); // 调用私有类的清理函数
}

void ComponentWidgetPrivate::updateGeometry() {
    // 更新容器高度
    m_configoptsWidget->adjustSize();

    // 更新整个窗口
    q_ptr->layout()->activate();
    q_ptr->updateGeometry();

    // 向上传递尺寸变化
    QWidget *parent = q_ptr->parentWidget();
    if (parent) {
        parent->adjustSize();
        parent->updateGeometry();
    }
}
#include "componentwidget.moc"
#include "moc_componentwidget.cpp"
