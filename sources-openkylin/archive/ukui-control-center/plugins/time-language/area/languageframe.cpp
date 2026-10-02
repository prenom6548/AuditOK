#include "languageframe.h"

#include "settinggroup.h"

#include <QHBoxLayout>
#include <QTimer>
#include <kysdk/applications/accessinfohelper.h>

using namespace kdk;

LanguageFrame::LanguageFrame(const QString &languageCode, const QString &showName, QWidget *parent)
    : UkccFrame(parent, BorderRadiusStyle::None, false)
    , m_languageCode(languageCode)
{
    setContainer(true);
    setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(1);

    m_headerFrame = new UkccFrame(this, BorderRadiusStyle::None, true);
    QHBoxLayout *headerLayout = new QHBoxLayout(m_headerFrame);
    headerLayout->setContentsMargins(16, 0, 16, 0);

    nameLabel = new QLabel(m_headerFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(nameLabel, "Area", "", "");
    nameLabel->setText(showName);
    nameLabel->setAccessibleName(showName);

    expandBtn = new QToolButton(m_headerFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(expandBtn, "Area", "", "");
    expandBtn->setAutoRaise(true);
    expandBtn->setProperty("useButtonPalette", true);
    expandBtn->setVisible(false);
    connect(expandBtn, &QToolButton::clicked, this, [=]() {
        if (m_expandClickLocked) {
            return;
        }
        m_expandClickLocked = true;
        setExpanded(!m_expanded);
        QTimer::singleShot(150, this, [this]() {
            m_expandClickLocked = false;
        });
    });

    deleteBtn = new QToolButton(m_headerFrame);
    KDK_EXTEND_ALL_INFO_FORMAT(deleteBtn, "Area", "", "");
    deleteBtn->setAutoRaise(true);
    deleteBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    deleteBtn->setProperty("useButtonPalette", true);
    deleteBtn->setToolTip(tr("Delete"));
    deleteBtn->setAccessibleName(tr("Delete"));
    nameLabel->setBuddy(deleteBtn);

    headerLayout->addWidget(nameLabel);
    headerLayout->addStretch();
    headerLayout->addWidget(deleteBtn);
    headerLayout->addWidget(expandBtn);

    mainLayout->addWidget(m_headerFrame);

    connect(deleteBtn, &QToolButton::clicked, this, [=]() {
        Q_EMIT deleteRequested();
    });

    setObjectName("LanguageFrame");
}

LanguageFrame::~LanguageFrame()
{
}

// 获取语言显示名称
QString LanguageFrame::getShowName() const
{
    return nameLabel ? nameLabel->text() : QString();
}

// 获取语言编码
QString LanguageFrame::getLanguageCode() const
{
    return m_languageCode;
}

// 设置删除按钮可用状态
void LanguageFrame::setDeleteEnabled(bool enabled)
{
    if (deleteBtn) {
        deleteBtn->setEnabled(enabled);
    }
}

void LanguageFrame::reloadIcon()
{
    if (deleteBtn) {
        deleteBtn->setIcon(QIcon::fromTheme("edit-delete-symbolic"));
    }
    updateExpandIcon();
}

// 设置语言特性展开状态
void LanguageFrame::setExpanded(bool expanded)
{
    if (m_expanded == expanded) {
        updateExpandIcon();
        return;
    }

    SettingGroup *group = qobject_cast<SettingGroup *>(parentWidget());
    const bool groupUpdatesEnabled = group && group->updatesEnabled();
    if (groupUpdatesEnabled) {
        group->setUpdatesEnabled(false);
    }

    m_expanded = expanded;
    const bool showDetail = expanded && hasFeatureItems();
    for (LanguageFeatureItem *item : m_featureItems) {
        item->setVisible(showDetail);
    }
    updateExpandIcon();
    updateLayoutHeight();

    if (groupUpdatesEnabled) {
        group->setUpdatesEnabled(true);
    }
}

// 更新语言项布局高度
void LanguageFrame::updateLayoutHeight()
{
    if (!mainLayout) {
        return;
    }
    mainLayout->invalidate();
    mainLayout->activate();

    const int layoutHeight = mainLayout->sizeHint().height();
    if (height() != layoutHeight || minimumHeight() != layoutHeight || maximumHeight() != layoutHeight) {
        setFixedHeight(layoutHeight);
    }
    updateGeometry();

    if (QWidget *parent = parentWidget()) {
        if (QLayout *parentLayout = parent->layout()) {
            parentLayout->invalidate();
            parentLayout->activate();
        }
        parent->updateGeometry();
    }
}

// 判断语言特性是否展开
bool LanguageFrame::isExpanded() const
{
    return m_expanded;
}

// 添加语言特性项
LanguageFeatureItem *LanguageFrame::addLanguageFeature(LanguageFeatureType type,
                                                       const QString &featureName,
                                                       LanguageFeatureState state)
{
    if (m_featureItems.contains(type)) {
        return m_featureItems.value(type);
    }

    LanguageFeatureItem *item = new LanguageFeatureItem(type, featureName, state, this);
    item->setVisible(m_expanded);
    mainLayout->addWidget(item);
    m_featureItems.insert(type, item);
    expandBtn->setVisible(true);
    setExpanded(m_expanded);
    connect(item, &LanguageFeatureItem::installRequested, this, [=](LanguageFeatureType featureType) {
        Q_EMIT featureInstallRequested(m_languageCode, featureType);
    });
    updateLayoutHeight();
    notifyGroupShapeUpdate();
    return item;
}

// 获取指定语言特性项
LanguageFeatureItem *LanguageFrame::featureItem(LanguageFeatureType type) const
{
    return m_featureItems.value(type, nullptr);
}

// 判断是否存在语言特性项
bool LanguageFrame::hasFeatureItems() const
{
    return !m_featureItems.isEmpty();
}

// 更新展开按钮图标
void LanguageFrame::updateExpandIcon()
{
    if (!expandBtn) {
        return;
    }
    const QString iconName = m_expanded ? "ukui-up-symbolic" : "ukui-down-symbolic";
    expandBtn->setIcon(QIcon::fromTheme(iconName));
}

// 通知所属分组刷新外形
void LanguageFrame::notifyGroupShapeUpdate()
{
    for (QWidget *widget = parentWidget(); widget; widget = widget->parentWidget()) {
        if (SettingGroup *group = qobject_cast<SettingGroup *>(widget)) {
            group->updateShape();
            break;
        }
    }
}
