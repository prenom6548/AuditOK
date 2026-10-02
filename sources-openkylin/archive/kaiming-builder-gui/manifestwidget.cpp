
#include "manifestwidget.h"

#include <QFileDialog>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QScrollArea>
#include <QPushButton>
#include <QLabel>
#include <QUrl>
#include <QDir>
#include <QMessageBox>
#include <QFileInfo>
#include <QDebug>
#include <QTimer>
#include <QPointer>

#include "component/clicklabel.h"
#include "component/chooselineedit.h"
#include "stackedwidgetmanager.h"
#include "messageboxutils.h"
#include "globalinfo.h"
#include "common/subprocess.h"
#include "logic/BuildSettingManager.h"
#include "logic/HistoryManager.h"

#define INIT_GROUPBOX(variableName, title, parent, widget) \
    QGroupBox *variableName = new QGroupBox(title, parent); \
    QVBoxLayout *variableName##Layout = new QVBoxLayout; \
    variableName##Layout->addWidget(widget); \
    variableName->setLayout(variableName##Layout);


class ManifestWidgetPrivate : public QObject
{
    Q_OBJECT

public:
    ManifestWidgetPrivate(ManifestWidget *parent)
            : q_ptr(parent), m_editingIndex(-1) {}
    virtual ~ManifestWidgetPrivate();

    void setupUi();

public:
    void addComponent(const KMBuilderManifest::Component &component);
    void updateComponent(int index, const KMBuilderManifest::Component &component);
    void setEditingIndex(int index) { m_editingIndex = index; }
    int editingIndex() const { return m_editingIndex; }
    void saveToYaml();
    void loadFromYaml();

private:
    Q_DECLARE_PUBLIC(ManifestWidget)
    ManifestWidget * q_ptr;
    int m_editingIndex; // 当前编辑的组件索引，-1 表示添加模式
    QVBoxLayout * m_manifestLayout;

    // 功能区
    QLineEdit * m_projectDir;
    QString m_fileName;
    QPushButton * m_btnSave;
    QPushButton * m_btnRefresh;
    QPushButton * m_btnBuild;
    QPushButton * m_btnBack;
    PackageFrame * m_packageFrame;
    BaseInfoFrame * m_baseInfoFrame;
    SetupInfoFrame * m_setupInfoFrame;
    QPushButton * m_btnAddComponent;
    QVBoxLayout * m_componentsLayout;
    QVector<KMBuilderManifest::Component> m_components; // 组件列表
    KMBuilderManifest::Permission m_permission;
    KMBuilderManifest::Annotations m_annotations;
    QCheckBox *m_kylinPrivilegeCheckBox;
    QCheckBox *m_kylinSysappCheckBox;
    QTextEdit *m_kylinDependAppsEdit;
    OperationFlag m_flag;
    void updateComponentIndices();

};

ManifestWidget::ManifestWidget(QWidget *parent)
    : QWidget(parent),
      d_ptr(new ManifestWidgetPrivate(this))
{
    Q_D(ManifestWidget);
    d->setupUi();
    setAutoFillBackground(true);

    connect(this, &ManifestWidget::editComponentClicked, this, &ManifestWidget::setEditingIndex);
}

ManifestWidget::~ManifestWidget()
{
}

void ManifestWidget::mousePressEvent(QMouseEvent *event)
{
    Q_D(ManifestWidget);
    if (d->m_projectDir->geometry().contains(event->pos()) == false && d->m_projectDir->hasFocus()) {
        d->m_projectDir->clearFocus(); // 移除焦点
    }

    QWidget::mousePressEvent(event);
}

void ManifestWidget::setOperationFlag(OperationFlag flag)
{
    Q_D(ManifestWidget);
    d->m_flag = flag;
}

bool ManifestWidget::fetchSources()
{
    Q_D(ManifestWidget);

    QString workDir = GlobelInfo::instance().getProjectDir();
    //TODO：确定拷贝的目录是那个，是否还需要projectDir和id

    for (auto& component : d->m_components) {
        for (auto& source : component.m_sources) {
            if (source.m_type == KMBuilderManifest::SOURCEType::DIR && !source.m_dir.m_originalPath.isEmpty()) {
                // 检查原始路径是否是项目目录的子目录
                if (source.m_dir.m_originalPath.startsWith(workDir)) {
                    // 是子目录，跳过拷贝
                    continue;
                }
                QString destDir = workDir + "/" + source.m_dir.m_path;
                QDir dir(destDir);
                dir.mkpath(destDir);
                // 拷贝原始源码目录到项目中对应的目录中
                SubProcess child;
                child.start("rsync", { "-av", "--delete", source.m_dir.m_originalPath + "/", destDir + "/" }, true);

                child.start("sync", {}, true);
            } else if (source.m_type == KMBuilderManifest::SOURCEType::FILE && !source.m_file.m_originalPath.isEmpty()){
                // 检查原始路径是否是项目目录的子目录
                if (source.m_file.m_originalPath.startsWith(workDir)) {
                    // 是子目录，跳过拷贝
                    continue;
                }
                QString destFile = workDir + "/" + source.m_file.m_path;
                QDir dir(destFile);
                QString filename =  "/" + dir.dirName();
                destFile = destFile.mid(0, destFile.length() - filename.length());
                dir.mkpath(destFile);
                // 拷贝原始源码目录到项目中对应的目录中
                SubProcess child;
                child.start("cp", { source.m_file.m_originalPath, workDir + "/" + source.m_file.m_path }, true);

                child.start("sync", {}, true);
            }
        }
    }

    return true;
}

ManifestWidgetPrivate::~ManifestWidgetPrivate()
{}

void ManifestWidgetPrivate::setupUi()
{
    Q_Q(ManifestWidget);

    QScrollArea *scrollArea = new QScrollArea;
    scrollArea->setWidgetResizable(true);
    scrollArea->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);

    QWidget *areaWidget = new QWidget;
    scrollArea->setWidget(areaWidget);
    m_manifestLayout = new QVBoxLayout;
    m_manifestLayout->setSpacing(24);
    areaWidget->setLayout(m_manifestLayout);

    // 设置项目路径
    QWidget *headerWidget = new QWidget;
    QHBoxLayout * projectLayout = new QHBoxLayout;
    projectLayout->setContentsMargins(0, 0, 0, 0);
    {
        QLabel * lableProjectPath = new QLabel;
        lableProjectPath->setText(tr("Project Path"));
        lableProjectPath->setToolTip(tr("Create a project under this path."
                                        "Click the search button on the right to select from the local"));

        m_projectDir = new  QLineEdit;
        m_projectDir->setReadOnly(true);
        m_projectDir->setStyleSheet("background-color: #f0f0f0;");
        m_projectDir->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

        projectLayout->addWidget(lableProjectPath);
        projectLayout->addWidget(m_projectDir);
    }
    headerWidget->setLayout(projectLayout);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(headerWidget);
    mainLayout->addWidget(scrollArea);
    mainLayout->setContentsMargins(24, 0, 24, 24);
    mainLayout->setSpacing(24);
    q->setLayout(mainLayout);

    // Package Info
    {
        m_packageFrame = new PackageFrame;
        INIT_GROUPBOX(packageGroupBox, tr("Package Info"), q_ptr, m_packageFrame)
        m_manifestLayout->addWidget(packageGroupBox);
    }

    //Base Select
    {
        m_baseInfoFrame = new BaseInfoFrame;
        INIT_GROUPBOX(baseGroupBox, tr("Base Select"), q_ptr, m_baseInfoFrame)
        m_manifestLayout->addWidget(baseGroupBox);
    }

    //Setup Info
    {
        m_setupInfoFrame = new SetupInfoFrame;
        INIT_GROUPBOX(setupGroupBox, tr("Setup Info"), q_ptr, m_setupInfoFrame)
        m_manifestLayout->addWidget(setupGroupBox);
    }

    // 其它配置
    {
        // TODO
    }

    //Component List
    {
        QWidget *componentContainer = new QWidget;
        QVBoxLayout *componentLayout = new QVBoxLayout(componentContainer);

        QHBoxLayout *componentHeader = new QHBoxLayout;
        QLabel *componentLabel = new QLabel(tr("components")+ "<font color = red>*</font>");
        QLabel *annotationComponentLabel = new QLabel(tr("The software package itself and the components it depends on."));
        componentLabel->setToolTip(tr("The software package itself and the components it depends on."));
        annotationComponentLabel->setStyleSheet("color: gray");
        annotationComponentLabel->setWordWrap(true);
        m_btnAddComponent = new QPushButton(tr("Add component"));
        connect(m_btnAddComponent, &QPushButton::clicked, q, &ManifestWidget::addComponentClicked);
        componentHeader->addWidget(componentLabel);
        componentHeader->addStretch();
        componentHeader->addWidget(m_btnAddComponent);

        componentLayout->addLayout(componentHeader);
        componentLayout->addWidget(annotationComponentLabel);

        // 组件列表容器
        QWidget *componentsContainer = new QWidget;
        componentsContainer->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
        m_componentsLayout = new QVBoxLayout(componentsContainer);
        m_componentsLayout->setContentsMargins(0, 5, 0, 0);
        m_componentsLayout->setSpacing(3);
        componentLayout->addWidget(componentsContainer);

        INIT_GROUPBOX(componentGroupBox, tr("Component List"), q_ptr, componentContainer)
        m_manifestLayout->addWidget(componentGroupBox);
    }

    //Annotations
    {
        QWidget *annotationsContainer = new QWidget;
        QGridLayout *annotationsLayout = new QGridLayout(annotationsContainer);
        INIT_GROUPBOX(annotationsGroupBox, tr("Annotations"), q_ptr, annotationsContainer)
        m_manifestLayout->addWidget(annotationsGroupBox);

        m_kylinPrivilegeCheckBox = new QCheckBox(tr("The application requires sudo to run"));
        annotationsLayout->addWidget(m_kylinPrivilegeCheckBox, 0, 0);

        m_kylinSysappCheckBox = new QCheckBox(tr("Install data to path /usr"));
        annotationsLayout->addWidget(m_kylinSysappCheckBox, 0, 1);
//        m_kylinSysappCheckBox->setVisible(false);  // 隐藏控件

        QLabel *kylinDependAppsLabel = new QLabel(tr("Programs that depend on other applications"));
        annotationsLayout->addWidget(kylinDependAppsLabel, 1, 0);
        m_kylinDependAppsEdit = new QTextEdit;
        m_kylinDependAppsEdit->setPlaceholderText(tr("Enter dependent executables (absolute paths or names), separated by commas\n"
                                          "Example 1 (absolute paths): /usr/bin/b, /opt/app/a\n"
                                          "Example 2 (program names): b, a"));
        m_kylinDependAppsEdit->setToolTip(tr("Configure dependent executable programs\n\n"
                                   "Can be absolute paths (e.g., /usr/bin/b) or program names (e.g., b)\n"
                                   "Multiple entries should be comma-separated\n\n"
                                   "Recommendation: Match the calling style used in this application:\n"
                                   "- If called with absolute paths, specify absolute paths\n"
                                   "- If called with program names, specify program names"));
        annotationsLayout->addWidget(m_kylinDependAppsEdit, 1, 1);
    }

    m_manifestLayout->addStretch();

    QHBoxLayout * operationLayout = new QHBoxLayout;
    {
        m_btnSave = new QPushButton(tr("Save"));
        connect(m_btnSave, &QPushButton::clicked, this, &ManifestWidgetPrivate::saveToYaml);

        m_btnRefresh = new QPushButton(tr("Refresh"));
        connect(m_btnRefresh, &QPushButton::clicked, this, &ManifestWidgetPrivate::loadFromYaml);

        m_btnBuild = new QPushButton;
        m_btnBuild->setText(tr("Next"));
        connect(m_btnBuild, &QPushButton::clicked, this, [=](){
            QString projectDir = m_projectDir->text();
            if (projectDir.isEmpty()) {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set project path"),
                                                                  QObject::tr("OK"));
                m_projectDir->setFocus();

                return ;
            }

            //package.id
            QString id = m_packageFrame->id();
            if (id.isEmpty()) {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set project id"),
                                                                  QObject::tr("OK"));

                m_packageFrame->setIdFocus();

                return ;
            }

            //package.name
            QString name = m_packageFrame->name();
            if (name.isEmpty()){
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set project name"),
                                                                  QObject::tr("OK"));
                m_packageFrame->setNameFocus();
                return ;
            }

            //package.version
            QString version = m_packageFrame->version();
            if(version.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set package version"),
                                                                  QObject::tr("OK"));
                m_packageFrame->setVersionFocus();
                return ;
            }

            //base
            QString base = m_baseInfoFrame->base();
            if (base.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set base"),
                                                                  QObject::tr("OK"));
                m_baseInfoFrame->setBaseFocus();
                return;
            }

            //command
            QString command = m_setupInfoFrame->command();
            if (command.isEmpty())
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set command"),
                                                                  QObject::tr("OK"));
                m_setupInfoFrame->setCommandFocus();
                return;
            }

            //component
            int component_size = m_componentsLayout->count();
            if (component_size == 0)
            {
                MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                                  QObject::tr("Please set components"),
                                                                  QObject::tr("OK"));
                return;
            }

            // 跳转到下一页
            {
                q->getManifest();
                KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
                QString workDir = GlobelInfo::instance().getProjectDir();

                QString yamlPath;
                if (m_flag == OperationFlag::Load) {
                    yamlPath = KMBuilderStringUtils::buildFilename(workDir, m_fileName);
                } else if (m_flag == OperationFlag::New) {
                    yamlPath = KMBuilderStringUtils::buildFilename(workDir, id + ".km.yaml");
                }
                GlobelInfo::instance().setYamlPath(yamlPath);

                // 生成YAML文件
                manifest->toYaml(yamlPath);

                // 缓存
                HistoryManager historyManager;
                historyManager.addHistory(manifest->m_package.m_id, yamlPath);

                // 提取本地源到项目目录中
                q->fetchSources();

                // 检查YAML文件是否存在
                if (!QFileInfo(yamlPath).exists()) {
                    MessageBoxUtils::QMESSAGE_BOX_WARNING(areaWidget, QObject::tr("Warning"),
                                                          QObject::tr("Not found project file : ") + yamlPath,
                                                          QObject::tr("OK"));
                    return;
                }

                emit q->next();
            }

        });

        m_btnBack = new QPushButton(tr("Back"));
        connect(m_btnBack, &QPushButton::clicked, q, &ManifestWidget::back);
        operationLayout->addStretch();
        operationLayout->addWidget(m_btnSave);
        operationLayout->addWidget(m_btnRefresh);
        operationLayout->addWidget(m_btnBack);
        operationLayout->addWidget(m_btnBuild);
        operationLayout->setSpacing(8);
    }

    mainLayout->addLayout(operationLayout);
}

void ManifestWidgetPrivate::addComponent(const KMBuilderManifest::Component &component)
{
    Q_Q(ManifestWidget);

    // 创建条目容器
    QWidget *componentRow = new QWidget(q);
    QHBoxLayout *rowLayout = new QHBoxLayout(componentRow);
    rowLayout->setContentsMargins(10, 5, 10, 5); // 设置边距
    rowLayout->setSpacing(10); // 设置控件间距

    // 显示 component 的 name
    QLabel *nameLabel = new QLabel(component.m_name);
    nameLabel->setStyleSheet("font-size: px; color: #333;");
    rowLayout->addWidget(nameLabel);
    rowLayout->addStretch();

    // 上移按钮
    QPushButton *upBtn = new QPushButton("↑");
    upBtn->setFixedSize(24, 24);
    connect(upBtn, &QPushButton::clicked, [this, componentRow] {
        int currentIndex = m_componentsLayout->indexOf(componentRow);
        if (currentIndex > 0) {
            // 取出当前项
            QLayoutItem *item = m_componentsLayout->takeAt(currentIndex);
            QWidget *widget = item->widget();

            // 更新数据顺序
            m_components.move(currentIndex, currentIndex - 1);

            // 重新插入到前一个位置
            m_componentsLayout->insertWidget(currentIndex - 1, widget);
            delete item;

            // 触发更新
            QTimer::singleShot(0, this, [this] { updateComponentIndices(); });
        }
    });

    // 下移按钮
    QPushButton *downBtn = new QPushButton("↓");
    downBtn->setFixedSize(24, 24);
    connect(downBtn, &QPushButton::clicked, [this, componentRow] {
        int currentIndex = m_componentsLayout->indexOf(componentRow);
        if (currentIndex < m_componentsLayout->count() - 1) {
            // 取出当前项
            QLayoutItem *item = m_componentsLayout->takeAt(currentIndex);
            QWidget *widget = item->widget();

            // 更新数据顺序
            m_components.move(currentIndex, currentIndex + 1);

            // 重新插入到后一个位置
            m_componentsLayout->insertWidget(currentIndex + 1, widget);
            delete item;

            // 触发更新
            QTimer::singleShot(0, this, [this] { updateComponentIndices(); });
        }
    });

    // 编辑按钮
    QPushButton *editBtn = new QPushButton(tr("Edit"));
    editBtn->setFixedSize(60, 24);
    connect(editBtn, &QPushButton::clicked, [this, componentRow] {
        Q_Q(ManifestWidget);
        int index = m_componentsLayout->indexOf(componentRow); // 获取当前行的索引
        // 验证一致性
        int propIndex = componentRow->property("componentIndex").toInt();
        if (index != propIndex) {
            qWarning() << "Index mismatch! Layout:" << index
                       << "Property:" << propIndex;
            // 强制更新属性
            componentRow->setProperty("componentIndex", index);
        }
        if (index >= 0 && index < m_components.size()) {
            emit q->editComponentClicked(index, m_components[index]); // 发射编辑信号，传递索引和组件数据
        }
     });

    // 删除按钮
    QPushButton *removeBtn = new QPushButton("×");
    removeBtn->setFixedSize(24, 24);
    removeBtn->setStyleSheet("color: red;");
    connect(removeBtn, &QPushButton::clicked, [this, componentRow]{
        // 获取索引
        int index = m_componentsLayout->indexOf(componentRow);
        // 验证一致性
        int propIndex = componentRow->property("componentIndex").toInt();
        if (index != propIndex) {
            qWarning() << "Index mismatch! Layout:" << index
                       << "Property:" << propIndex;
            // 强制更新属性
            componentRow->setProperty("componentIndex", index);
        }
        // 检查索引是否有效
        if (index >= 0 && index < m_components.size()) {
            m_components.remove(index);
            m_componentsLayout->removeWidget(componentRow);
            componentRow->deleteLater();

             updateComponentIndices();
        } else {
            qWarning() << "Invalid index:" << index;
        }

    });
    rowLayout->addWidget(upBtn);
    rowLayout->addWidget(downBtn);
    rowLayout->addWidget(editBtn);
    rowLayout->addWidget(removeBtn);

    // 存储原始数据
    componentRow->setProperty("componentData", QVariant::fromValue(component));

    // 添加到布局
    m_componentsLayout->addWidget(componentRow);
    // 将组件添加到列表
    m_components.append(component);
}

// 更新组件索引
void ManifestWidgetPrivate::updateComponentIndices()
{
    for (int i = 0; i < m_componentsLayout->count(); ++i) {
        if (QWidget *widget = m_componentsLayout->itemAt(i)->widget()) {
            widget->setProperty("componentIndex", i);
        }
    }
}

void ManifestWidgetPrivate::updateComponent(int index, const KMBuilderManifest::Component &component)
{
    if (index >= 0 && index < m_components.size()) {
        // 更新组件数据
        m_components[index] = component;
        qDebug() << "ManifestWidget update component:" << component.m_name <<index; // 调试输出

        // 更新 UI 显示
        QWidget *row = m_componentsLayout->itemAt(index)->widget();
        if (row) {
            QLabel *nameLabel = row->findChild<QLabel *>();
            if (nameLabel) {
                nameLabel->setText(component.m_name);
            }
            row->setProperty("componentData", QVariant::fromValue(component));
        }
    }
}

void ManifestWidget::handleComponentSaved(const KMBuilderManifest::Component &component)
{
    Q_D(ManifestWidget);
    if (d->editingIndex() == -1) {
        d->addComponent(component); // 添加新组件
    } else {
        d->updateComponent(d->editingIndex(),component); // 更新现有组件
    }
    d->setEditingIndex(-1); // 重置编辑索引
}

void ManifestWidget::setEditingIndex(int index)
{
    Q_D(ManifestWidget);
    d->setEditingIndex(index);
}

int ManifestWidget::editingIndex() const
{
    Q_D(const ManifestWidget);
    return d->editingIndex();
}

const KMBuilderManifest::Permission& ManifestWidget::getPermissionData()
{
    Q_D(const ManifestWidget);
    return d->m_permission;
}

void ManifestWidget::setPermissionData(const KMBuilderManifest::Permission& permisson)
{
    Q_D(ManifestWidget);
    d->m_permission = permisson;
}

void ManifestWidget::getManifest()
{
    Q_D(const ManifestWidget);
    d->m_packageFrame->getData();
    d->m_setupInfoFrame->getData();
    d->m_baseInfoFrame->getData();
    KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
    manifest->m_components.clear();
    manifest->m_components.append(d->m_components);
    manifest->m_permission = d->m_permission;
    //annotations
    manifest->m_annotations.m_kylinPrivilege = d->m_kylinPrivilegeCheckBox->isChecked();
    manifest->m_annotations.m_kylinSysapp = d->m_kylinSysappCheckBox->isChecked();

#if (QT_VERSION >= QT_VERSION_CHECK(5, 14, 0))
    manifest->m_annotations.m_kylinDependApps = d->m_kylinDependAppsEdit->toPlainText().split(",", Qt::SkipEmptyParts);
#else
    manifest->m_annotations.m_kylinDependApps = d->m_kylinDependAppsEdit->toPlainText().split(",", QString::SkipEmptyParts);
#endif
    for (QString &item : manifest->m_annotations.m_kylinDependApps) {
        item = item.trimmed();
    }
}

void ManifestWidget::clearAll(const QString &path, const QString &id)
{
    Q_D(ManifestWidget);

    // 项目路径
    d->m_projectDir->setText(path);

    d->m_packageFrame->clear(id);
    d->m_baseInfoFrame->clear();
    d->m_setupInfoFrame->clear();

    // 清理组件列表
    while (QLayoutItem *item = d->m_componentsLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater();
        }
        delete item;
    }
    d->m_components.clear();

    // 清理权限信息
    d->m_permission = KMBuilderManifest::Permission();

    //清理annotations
    d->m_kylinPrivilegeCheckBox->setChecked(false);
    d->m_kylinSysappCheckBox->setChecked(false);
    d->m_kylinDependAppsEdit->clear();

    // 重置编辑索引
    d->setEditingIndex(-1);

    //单例初始化
    KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
    manifest->cleanup();
}

void ManifestWidget::load(const KMBuilderManifest &manifest, const QString &fileName)
{
    Q_D(ManifestWidget);

    d->m_fileName = fileName;
    // 加载包信息
    d->m_packageFrame->load(manifest.m_package);

    // 加载基础信息
    d->m_baseInfoFrame->load(manifest.m_base, manifest.m_runtime, manifest.m_depends);

    // 加载设置信息
    d->m_setupInfoFrame->load(manifest.m_channel, manifest.m_commands, manifest.m_cleanups, manifest.m_modules, manifest.m_envs);

    // 加载组件列表
    for (const auto &component : manifest.m_components) {
        d->addComponent(component);
    }

    // 加载权限信息
    d->m_permission = manifest.m_permission;

    // 加载annotation
    d->m_kylinPrivilegeCheckBox->setChecked(manifest.m_annotations.m_kylinPrivilege);
    d->m_kylinSysappCheckBox->setChecked(manifest.m_annotations.m_kylinSysapp);
    d->m_kylinDependAppsEdit->setText(manifest.m_annotations.m_kylinDependApps.join(","));

    // 重置编辑索引
    d->setEditingIndex(-1);
}

void ManifestWidgetPrivate::saveToYaml()
{
    Q_Q(ManifestWidget);
    QString workDir = GlobelInfo::instance().getProjectDir();
    QString id = m_packageFrame->id();
    QString yamlPath = KMBuilderStringUtils::buildFilename(workDir, id + ".km.yaml");

    q->getManifest();
    KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
    //存储
    HistoryManager historyManager;
    historyManager.addHistory(manifest->m_package.m_id, yamlPath);

    // 生成YAML文件
    if (manifest->toYaml(yamlPath)) {
        MessageBoxUtils::QMESSAGE_BOX_INFORMATION(q, QObject::tr("Success"),
                                                QObject::tr("Manifest saved successfully to:\n%1").arg(yamlPath),
                                                QObject::tr("OK"));
    } else {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(q, QObject::tr("Error"),
                                            QObject::tr("Failed to save manifest to:\n%1").arg(yamlPath),
                                            QObject::tr("OK"));
    }
}

void ManifestWidgetPrivate::loadFromYaml()
{
    Q_Q(ManifestWidget);
    QString workDir = GlobelInfo::instance().getProjectDir();
    QString id = m_packageFrame->id();
    QString fileName = (m_flag == OperationFlag::Load) ? m_fileName : (id + ".km.yaml");

    QString yamlPath = KMBuilderStringUtils::buildFilename(workDir, fileName);

    if (QFileInfo(yamlPath).exists())
    {
        try
        {
            q->clearAll(workDir, id);
            KMBuilderManifest* manifest = KMBuilderApi::loadFromYaml(yamlPath);
            q->load(*manifest, fileName);
        } catch (const std::exception &e)
        {
            MessageBoxUtils::QMESSAGE_BOX_WARNING(q, QObject::tr("Error"),
                                                QObject::tr("Failed to load manifest from:\n%1\n%2")
                                                .arg(yamlPath, e.what()),
                                                QObject::tr("OK"));
        }
    } else {
        MessageBoxUtils::QMESSAGE_BOX_WARNING(q, QObject::tr("File Not Found"),
                                            QObject::tr("Please save the manifest first to:\n%1")
                                            .arg(yamlPath),
                                            QObject::tr("OK"));
    }
}

#include "manifestwidget.moc"
#include "moc_manifestwidget.cpp"


