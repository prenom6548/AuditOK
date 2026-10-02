#include "baseinfoframe.h"
#include "basedialog.h"
#include "logic/KMBuilderStringUtils.h"
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QVBoxLayout>
#include <QLineEdit>
#include <QPushButton>

#include "selectdependsdialog.h"

BaseInfoFrame::BaseInfoFrame(QWidget *parent)
    : QFrame(parent),
      m_baseInput(new QLineEdit(this)),
      m_baseSearchButton(new QPushButton("🔍", this)),
      m_runtimeInput(new QLineEdit(this)),
      m_runtimeSearchButton(new QPushButton("🔍", this)),
      m_addDependencyButton(new QPushButton(tr("Add depend"), this)),
      m_dependencyContainer(new QWidget(this)),
      m_dependencyLayout(new QVBoxLayout(m_dependencyContainer))
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);

    // Base 部分
    QHBoxLayout *baseLayout = new QHBoxLayout;
    m_baseInput->setPlaceholderText(tr("The basic environment needed to build the package."
                                    "Format: {channel}:{id}/{version}.  "
                                    "Example:  "
                                    "stable:top.openkylin.base/1.0.0"));
    m_baseInput->setToolTip(tr("The basic environment needed to build the package.\n"
                               "Format: {channel}:{id}/{version}\n"
                               "Example:\n"
                               "stable:top.openkylin.base/1.0.0"));
//    m_baseInput->setText("stable:top.openkylin.base/1.0.0");
    m_baseInput->setText("");
    baseLayout->addWidget(m_baseInput);
    baseLayout->addWidget(m_baseSearchButton);
    connect(m_baseSearchButton, &QPushButton::clicked, this, [this]() {
        openSearchDialog("base","","");
    });

    // Runtime 部分
    QHBoxLayout *runtimeLayout = new QHBoxLayout;
    m_runtimeInput->setPlaceholderText(tr("Specific environments that complement the base environment to build package,"
                                       "such as cmake,qmake, etc. "
                                       "Format: {channel}:{id}/{version}. "
                                       "Example: "
                                       "stable:top.openkylin.ukui/1.0.0"));
    m_runtimeInput->setToolTip(tr("Specific environments that complement the base environment to build package,"
                                  "such as cmake,qmake, etc\n"
                                  "Format: {channel}:{id}/{version}\n"
                                  "Example:\n"
                                  "stable:top.openkylin.ukui/1.0.0"));
//    m_runtimeInput->setText("stable:top.openkylin.ukui/1.0.0");
    m_runtimeInput->setText("");
    runtimeLayout->addWidget(m_runtimeInput);
    runtimeLayout->addWidget(m_runtimeSearchButton);
    connect(m_runtimeSearchButton, &QPushButton::clicked, this, [this]() {
        openSearchDialog("runtime","","");
    });

    QFormLayout *formLayout = new QFormLayout;
    formLayout->addRow(tr("base") + "<font color=red>*</font>", baseLayout);
    formLayout->addRow(tr("runtime"), runtimeLayout);

    mainLayout->addLayout(formLayout);

    // 依赖项管理
    QLabel *dependenciesLabel = new QLabel("depends", this);
    QLabel *annotationLabel = new QLabel(tr("Dependencies needed to build packages. "
                                         "Example: "
                                         " top.openkylin.depend"
                                         ));
    annotationLabel->setStyleSheet("color: gray");
    annotationLabel->setWordWrap(true);
    dependenciesLabel->setToolTip(tr("Dependencies needed to build packages.\n"
                                     "Example:\n"
                                     " top.openkylin.depend"));
    mainLayout->addWidget(dependenciesLabel);
    mainLayout->addWidget(annotationLabel);

    // 滚动区域包裹依赖项列表
    mainLayout->addWidget(m_dependencyContainer);

    // 调整容器和布局的尺寸策略
    m_dependencyContainer->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Maximum);
    m_dependencyLayout->setAlignment(Qt::AlignTop);

    // 添加依赖项按钮
    mainLayout->addWidget(m_addDependencyButton);
    connect(m_addDependencyButton, &QPushButton::clicked, this, [=]() {
        QString baseValue = m_baseInput->text(); // 获取 base 输入框的值
        QString runtimeValue = m_runtimeInput->text(); // 获取 runtime 输入框的值

        SelectDependsDialog dialog(this, baseValue, runtimeValue, m_selectedDepends);
        if (dialog.exec() == QDialog::Accepted) {
            const QVector<SelectedDepend>& depends = dialog.getSelected();
            for (auto& depend : depends) {
                QString selectedDepend = depend.id;
                if (!depend.version.isEmpty())
                {
                    selectedDepend = selectedDepend + "/" + depend.version;
                }
                if (m_selectedDepends.contains(selectedDepend))
                {
                    continue;
                }
                m_selectedDepends.append(selectedDepend);
                this->addDependency(selectedDepend);
            }
        }
    });

    setLayout(mainLayout);
    // 设置整个窗口的尺寸策略
    this->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Preferred);
}

void BaseInfoFrame::addDependency(const QString& ref)
{
    QWidget *dependRow = new QWidget(this);
    QHBoxLayout *layout = new QHBoxLayout(dependRow);
    QLineEdit *packageInput = new QLineEdit(dependRow);
    packageInput->setPlaceholderText("top.openkylin.depend");
    packageInput->setText(ref);
    // 检查是否包含 {}，设置不同字体颜色
    if (ref.contains('{') && ref.contains('}')) {
        packageInput->setStyleSheet("color: red;");  // 红色字体
    } else {
        packageInput->setStyleSheet("color: black;"); // 默认黑色
    }
    QPushButton *searchButton = new QPushButton("🔍", dependRow);
    QPushButton *removeButton = new QPushButton("×", dependRow);
    removeButton->setFixedSize(24, 24);

    layout->addWidget(packageInput);
    layout->addWidget(searchButton);
    layout->addWidget(removeButton);
    dependRow->setLayout(layout);
    m_dependencyLayout->addWidget(dependRow);
    updateGeometry(); // 添加后更新布局

    // 动态监听文本变化，更新字体颜色
    connect(packageInput, &QLineEdit::textChanged, [=](const QString &text) {
        if (text.contains('{') && text.contains('}')) {
            packageInput->setStyleSheet("color: red;");
        } else {
            packageInput->setStyleSheet("color: black;");
        }
    });

    connect(searchButton, &QPushButton::clicked, this, [=]() {
        QString baseValue = m_baseInput->text(); // 获取 base 输入框的值
        QString runtimeValue = m_runtimeInput->text(); // 获取 runtime 输入框的值

        openSearchDialog("depend", baseValue, runtimeValue);
    });

    // 绑定删除事件
    connect(removeButton, &QPushButton::clicked, this, [=]() {
        m_selectedDepends.removeAll(ref);
        removeDependency(dependRow);
    });
}

void BaseInfoFrame::openSearchDialog(const QString &kind, const QString &base, const QString &runtime)
{
    // TODO: 实现搜索弹窗，用户选择后填充 targetInput
    BaseDialog dialog(this);
    dialog.loadData(kind, base, runtime);
    if (dialog.exec() == QDialog::Accepted) {
        KMRef selectedRef = dialog.getSelectedRef();
        if (kind == "base")
        {
            QString selectedText = KMBuilderStringUtils::buildRef(selectedRef);
            m_baseInput->setText(selectedText);
        } else if (kind == "runtime")
        {
            QString selectedText = KMBuilderStringUtils::buildRef(selectedRef);
            m_runtimeInput->setText(selectedText);
        } else if (kind == "depend")
        {
            QString selectedText = KMBuilderStringUtils::buildRef(selectedRef,true);
            // 找到当前点击的依赖项输入框并设置值
            QWidget *dependRow = qobject_cast<QWidget *>(sender()->parent());
            if (dependRow) {
                QLineEdit *packageInput = dependRow->findChild<QLineEdit *>();
                if (packageInput) {
                    packageInput->setText(selectedText);
                }
            }
        }
        qDebug() << "Selected Ref:"
                 << "Channel:" << selectedRef.m_channel
                 << "ID:" << selectedRef.m_id
                 << "Version:" << selectedRef.m_version;
    }
}

void BaseInfoFrame::removeDependency(QWidget *dependRow)
{
    m_dependencyLayout->removeWidget(dependRow);
    delete dependRow;
    updateGeometry(); // 添加后更新布局
}

void BaseInfoFrame::getData()
{
    KMBuilderManifest * manifest = GlobelInfo::instance().getManifest();
    manifest->m_base = m_baseInput->text().trimmed();
    manifest->m_runtime = m_runtimeInput->text().trimmed();
    // 提取 Depends
    QVector<QString> dependsList;
    for (int i = 0; i < m_dependencyLayout->count(); ++i) {
        QWidget *dependRow = m_dependencyLayout->itemAt(i)->widget();
        if (dependRow) {
            QLineEdit *packageInput = dependRow->findChild<QLineEdit *>();
            if (packageInput && !packageInput->text().trimmed().isEmpty()) {
                dependsList.append(packageInput->text().trimmed());
            }
        }
    }
    manifest->m_depends.clear();
    manifest->m_depends.append(dependsList);
}

void BaseInfoFrame::clear()
{
    // m_baseInput->clear();
    // m_runtimeInput->clear();

    while (QLayoutItem *item = m_dependencyLayout->takeAt(0)) {
        if (QWidget *widget = item->widget()) {
            widget->deleteLater(); // 删除依赖项行
        }
        delete item; // 删除布局项
    }
}


void BaseInfoFrame::updateGeometry() {
    // 更新容器高度
    m_dependencyContainer->adjustSize();

    // 更新整个窗口
    adjustSize();
    if (parentWidget()) {
        parentWidget()->adjustSize();
    }
}

void BaseInfoFrame::load(const QString &base, const QString &runtime, const QVector<QString> &depends)
{
    m_baseInput->setText(base);
    m_runtimeInput->setText(runtime);
    m_selectedDepends.clear();

    // 加载依赖项
    for (const auto &depend : depends) {
        // 调用 addDependency() 添加新行
        if (!m_selectedDepends.contains(depend))
        {
            m_selectedDepends.append(depend);
        }
        addDependency(depend);
    }
}
