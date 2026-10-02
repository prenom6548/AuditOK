/*
 * UKUI Intelligent Data Manager
 *
 * Copyright (C) 2024, KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 *
 * Authors: Yue Lan <lanyue@kylinos.cn>
 *
 */

#include "file/fileinfo.h"

#include "conditioneditor.h"
#include "conditionedit.h"
#include "condition.h"
#include "model/searchresultmodel.h"

#include "modelcontroller.h"

#include "fileop/fileoperationmanager.h"

#include <QLayout>
#include <QHBoxLayout>
#include <QLabel>
#include <QMouseEvent>

#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QHeaderView>

#include <QStandardPaths>
#include <QPushButton>
#include <kysdk/applications/ktag.h>

#include <QDir>
#include <QFile>

#include <QVBoxLayout>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>

#include <QDialogButtonBox>
#include <QToolBar>
#include <QLineEdit>

#include <QApplication>

#include <QDebug>

static ConditionEditor *currentEditor = nullptr;

class ExpandableLabel : public QWidget
{
    Q_OBJECT
public:
    explicit ExpandableLabel(const QString &text, bool expandable)
    {
        setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);

        auto layout = new QHBoxLayout;
        auto label = new QLabel(text);
        label->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        layout->addWidget(label, Qt::AlignLeft);
        layout->addStretch(100);
        auto label2 = new QLabel;
        label2->setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Minimum);
        layout->addWidget(label2, Qt::AlignRight);
        rightLabel = label2;
        rightLabel->setVisible(expandable);

        rightLabel->installEventFilter(this);

        setLayout(layout);
        setAutoFillBackground(true);
    }

    bool eventFilter(QObject *watched, QEvent *event)
    {
        if (event->type() == QEvent::MouseButtonPress) {
            auto mouseEvent = static_cast<QMouseEvent *>(event);
            if (mouseEvent->button() == Qt::LeftButton) {
                Q_EMIT this->expandedChanged(!m_expanded);
            }
        }
        return QWidget::eventFilter(watched, event);
    }

signals:
    void expandedChanged(bool expanded);

public slots:
    void setExpanded(bool expanded) {
        m_expanded = expanded;
        rightLabel->setText(expanded? tr("Collspand ↑"): tr("Expand ↓"));
    }

private:
    QLabel *rightLabel;
    bool m_expanded = false;
};

class BasicConditionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit BasicConditionEditor(const QString &configPath, QWidget *parent = nullptr) : QWidget(parent)
    {
        setMinimumHeight(400);

        auto vboxLayout = new QVBoxLayout;
        vboxLayout->setAlignment(Qt::AlignTop|Qt::AlignLeft);
        setLayout(vboxLayout);

        QFile file(configPath);
        if (file.open(QFile::ReadOnly|QFile::Text)) {
            auto byteArray = file.readAll();
            file.close();
            auto jsonDocument = QJsonDocument::fromJson(byteArray);
            if (jsonDocument.isObject()) {
                auto rootObj = jsonDocument.object();
                int count = rootObj["conditions count"].toInt();
                if (count > 0) {
                    hasCondition = true;
                    initConditionCount = count;
                }
                for (int i = 0; i < count; i++) {
                    auto conditionObj = rootObj[QString::number(i)].toObject();
                    auto condition = Condition::fromJsonObject(conditionObj);
                    auto edit = new ConditionEdit(condition);
                    vboxLayout->addWidget(edit);
                }
            }
        }
    }

    bool hasCondition = false;
    int initConditionCount = 0;

public slots:
    void onAddCondition()
    {
        auto vboxLayout = static_cast<QVBoxLayout *>(layout());
        int rowCount = vboxLayout->count();
        bool isSearch = false;
        if (rowCount > 0) {
            auto widget = vboxLayout->itemAt(0)->widget();
            isSearch = !widget->isEnabled();
        }
        if (isSearch) {
            rowCount -= 3;
        }
        auto edit = new ConditionEdit(Condition::Type(rowCount <= 5? rowCount: Condition::FileName));
        vboxLayout->addWidget(edit);
    }

    void addCondition(QSharedPointer<Condition> condition, bool enable = true)
    {
        auto vboxLayout = static_cast<QVBoxLayout *>(layout());
        auto edit = new ConditionEdit(condition);
        edit->setEnabled(enable);
        vboxLayout->addWidget(edit);
    }

    void onRemoveCondition()
    {
        auto vboxLayout = static_cast<QVBoxLayout *>(layout());
        int rowCount = vboxLayout->count();
        if (rowCount > 0) {
            auto item = vboxLayout->itemAt(rowCount - 1);
            auto widget = item->widget();
            if (!widget->isEnabled())
                return;
            vboxLayout->removeItem(item);
            item->widget()->deleteLater();
        }
    }

    QList<QSharedPointer<Condition>> getCurrentConditions(bool containsDisabled = false)
    {
        QList<QSharedPointer<Condition>> conditions;
        auto vboxLayout = static_cast<QVBoxLayout *>(layout());
        int rowCount = vboxLayout->count();
        for (int row = 0; row < rowCount; row++) {
            auto conditionEdit = qobject_cast<ConditionEdit *>(vboxLayout->itemAt(row)->widget());
            if (!containsDisabled && !conditionEdit->isEnabled())
                continue;
            conditions << conditionEdit->getCondition();
        }
        return conditions;
    }
};

class FlowLayout : public QLayout
{
public:
    explicit FlowLayout(QWidget *parent, int margin = -1, int hSpacing = -1, int vSpacing = -1);
    explicit FlowLayout(int margin = -1, int hSpacing = -1, int vSpacing = -1);
    ~FlowLayout();

    void addItem(QLayoutItem *item) override;
    int horizontalSpacing() const;
    int verticalSpacing() const;
    Qt::Orientations expandingDirections() const override;
    bool hasHeightForWidth() const override;
    int heightForWidth(int) const override;
    int count() const override;
    QLayoutItem *itemAt(int index) const override;
    QSize minimumSize() const override;
    void setGeometry(const QRect &rect) override;
    QSize sizeHint() const override;
    QLayoutItem *takeAt(int index) override;

private:
    int doLayout(const QRect &rect, bool testOnly) const;
    int smartSpacing(QStyle::PixelMetric pm) const;

    QList<QLayoutItem *> itemList;
    int m_hSpace;
    int m_vSpace;
};

class LabelConditionEditor : public QWidget
{
    Q_OBJECT
public:
    explicit LabelConditionEditor(const QString &configPath, QWidget *parent = nullptr) : QWidget(parent)
    {
        QJsonArray labelArray;
        QFile file(configPath);
        if (file.open(QFile::ReadOnly|QFile::Text)) {
            auto byteArray = file.readAll();
            file.close();
            auto jsonDocument = QJsonDocument::fromJson(byteArray);
            if (jsonDocument.isObject()) {
                auto rootObj = jsonDocument.object();
                labelArray = rootObj["labels"].toArray();
            }
        }

        FlowLayout *flowLayout = new FlowLayout;
        this->flowLayout = flowLayout;

        bool shouldAddDocument = true;
        bool shouldAddDownload = true;
        for (auto value : labelArray) {
            auto text = value.toString();
            if (text == tr("Document for Work")) {
                shouldAddDocument = false;
            } else if (text == tr("Download Polymerization")) {
                shouldAddDownload = false;
            }
            auto tag = new QPushButton;
            tag->setCheckable(true);
            tag->setChecked(true);
            tag->setText(text);
            flowLayout->addWidget(tag);
        }

        if (shouldAddDocument) {
            auto button = new QPushButton(tr("Document for Work"));
            button->setCheckable(true);
            flowLayout->addWidget(button);
        }

        if (shouldAddDownload) {
            auto button = new QPushButton(tr("Download Polymerization"));
            button->setCheckable(true);
            flowLayout->addWidget(button);
        }

        setLayout(flowLayout);
    }

    FlowLayout *flowLayout;

    QStringList getLabels()
    {
        QStringList labels;
        for (int i = 0; i < flowLayout->count(); i++) {
            auto tag = qobject_cast<QPushButton *>(flowLayout->itemAt(i)->widget());
            if (tag->isChecked())
                labels << tag->text();
        }
        return labels;
    }
};

ConditionEditor::ConditionEditor(int spaceId, const QString &spaceName, ConditionEditor::Mode mode, QWidget *parent) : QMainWindow(parent)
{
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlag(Qt::Dialog);

    if (currentEditor) {
        ModelController::globalInstance()->warningWithDialog(nullptr, tr("A condition dialog is running, please apply or cancel last condition settings"));
        close();
        currentEditor->activateWindow();
        return;
    }

    currentEditor = this;

    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    setMinimumWidth(640);
    setMinimumHeight(480);

    m_mode = mode;

    // 加载标签和条件配置
    QString configPath = QStandardPaths::writableLocation(QStandardPaths::AppConfigLocation);
    QDir d;
    d.mkpath(configPath + "/spaces");
    QString jsonPath = QString("%1/spaces/%2.json").arg(configPath).arg(spaceId);

    QTreeWidget *treeView = new QTreeWidget(this);
    treeView->setFrameShape(QFrame::NoFrame);
    treeView->setIndentation(0);
    treeView->setExpandsOnDoubleClick(false);
    treeView->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOn);

    treeView->setHeaderHidden(true);
    treeView->header()->setStretchLastSection(true);
    treeView->setColumnCount(1);

    treeView->setAnimated(true);

    bool isCreate = false;
    bool shouldShowSaveToSpaceButton = false;
    switch (mode) {
    case Create: {
        auto item0 = new QTreeWidgetItem();
        item0->setFlags(Qt::NoItemFlags);
        auto spaceNameItem = new QTreeWidgetItem();
        spaceNameItem->setFlags(Qt::NoItemFlags);
        treeView->addTopLevelItem(item0);
        item0->setExpanded(true);
        item0->addChild(spaceNameItem);
        treeView->setItemWidget(item0, 0, new ExpandableLabel(tr("Create Space"), false));
        auto spaceNameEdit = new QLineEdit;
        connect(spaceNameEdit, &QLineEdit::textChanged, this, [=]{
            m_spaceName = spaceNameEdit->text();
        });
        spaceNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        spaceNameEdit->setPlaceholderText(tr("Please input space name"));
        treeView->setItemWidget(spaceNameItem, 0, spaceNameEdit);
        isCreate = true;
        break;
    }
    case Search: {
        auto searchModel = SearchResultModel::globalInstance();
        m_spaceName = searchModel->getSearchKeyword();
        if (!searchModel->getSearchInSpace()) {
            shouldShowSaveToSpaceButton = true;
        }
        auto item0 = new QTreeWidgetItem();
        item0->setFlags(Qt::NoItemFlags);
        item0->setChildIndicatorPolicy(QTreeWidgetItem::DontShowIndicator);
        treeView->addTopLevelItem(item0);
        treeView->setItemWidget(item0, 0, new ExpandableLabel(tr("Edit Conditions"), false));
        break;
    }
    default: {
        m_spaceId = spaceId;
        m_spaceName = spaceName;
        setProperty("currentSpaceName", spaceName);
        auto item0 = new QTreeWidgetItem();
        item0->setFlags(Qt::NoItemFlags);
        auto spaceNameItem = new QTreeWidgetItem();
        spaceNameItem->setFlags(Qt::NoItemFlags);
        treeView->addTopLevelItem(item0);
        item0->setExpanded(true);
        item0->addChild(spaceNameItem);
        treeView->setItemWidget(item0, 0, new ExpandableLabel(tr("Space's Condtions of %1").arg(spaceName), false));
        auto spaceNameEdit = new QLineEdit;
        connect(spaceNameEdit, &QLineEdit::textChanged, this, [=]{
            m_spaceName = spaceNameEdit->text();
        });
        spaceNameEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
        spaceNameEdit->setPlaceholderText(tr("Please input space name"));
        spaceNameEdit->setText(spaceName);
        treeView->setItemWidget(spaceNameItem, 0, spaceNameEdit);
        break;
    }
    }

    QTreeWidgetItem *item1 = new QTreeWidgetItem();
    item1->setFlags(Qt::NoItemFlags);
    item1->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    item1->setText(0, tr("Label"));
    auto labelChildItem = new QTreeWidgetItem();
    labelChildItem->setFlags(Qt::NoItemFlags);
    item1->addChild(labelChildItem);
    //fixme: 实现一个flowlayout，读取标签配置并加载
    auto labelConditionEditor = new LabelConditionEditor(jsonPath, treeView);
    treeView->setItemWidget(labelChildItem, 0, labelConditionEditor);

    QTreeWidgetItem *item2 = new QTreeWidgetItem();
    item2->setFlags(Qt::NoItemFlags);
    item2->setChildIndicatorPolicy(QTreeWidgetItem::ShowIndicator);
    item2->setText(0, tr("Condition"));
    auto conditionItem = new QTreeWidgetItem();
    conditionItem->setFlags(Qt::NoItemFlags);
    item2->addChild(conditionItem);
    // fixme: 实现一个动态添加条件的界面
    auto basicConditionEditor = new BasicConditionEditor(jsonPath, treeView);
    treeView->setItemWidget(conditionItem, 0, basicConditionEditor);

    if (mode == Search) {
        // 从search model中获取结果填充到条件编辑界面
        auto searchModel = SearchResultModel::globalInstance();
        for (auto condition : searchModel->reversedConditions) {
            basicConditionEditor->addCondition(condition, false);
        }

        for (auto condition : searchModel->searchConditions) {
            basicConditionEditor->addCondition(condition);
        }
    }

    treeView->addTopLevelItem(item1);
    treeView->addTopLevelItem(item2);

    auto label1 = new ExpandableLabel(tr("Intelligent Labels"), true);
    auto label2 = new ExpandableLabel(tr("Regular Conditions"), true);

    connect(treeView, &QTreeWidget::itemExpanded, this, [=](QTreeWidgetItem *item){
        auto label = qobject_cast<ExpandableLabel *>(treeView->itemWidget(item, 0));
        label->setExpanded(true);
    });
    connect(treeView, &QTreeWidget::itemCollapsed, this, [=](QTreeWidgetItem *item){
        auto label = qobject_cast<ExpandableLabel *>(treeView->itemWidget(item, 0));
        label->setExpanded(false);
    });
    connect(label1, &ExpandableLabel::expandedChanged, this, [=](bool expanded){
        item1->setExpanded(expanded);
    });
    connect(label2, &ExpandableLabel::expandedChanged, this, [=](bool expanded){
        item2->setExpanded(expanded);
    });

    treeView->setItemWidget(item1, 0, label1);
    treeView->setItemWidget(item2, 0, label2);

    if (labelConditionEditor->flowLayout->count() > 0) {
        item1->setExpanded(true);
        label1->setExpanded(true);
    } else {
        label1->setExpanded(false);
    }

    if (/*basicConditionEditor->hasCondition || isCreate*/true) {
        item2->setExpanded(true);
        label2->setExpanded(true);
    } else {
        label2->setExpanded(false);
    }

    treeView->adjustSize();

    setCentralWidget(treeView);

    adjustSize();

    // button
    auto buttonGroup = new QDialogButtonBox;
    buttonGroup->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Minimum);
    auto saveAsSpace = new QPushButton(tr("Create as Space"), this);
    if (shouldShowSaveToSpaceButton)
        buttonGroup->addButton(saveAsSpace, QDialogButtonBox::ActionRole);
    else
        saveAsSpace->hide();

    auto addCondition = new QPushButton(tr("Add Condition"));
    buttonGroup->addButton(addCondition, QDialogButtonBox::ActionRole);
    auto removeCondition = new QPushButton(tr("Remove Condition"));
    buttonGroup->addButton(removeCondition, QDialogButtonBox::ActionRole);

    auto cancelButton = new QPushButton(tr("Cancel"));
    buttonGroup->addButton(cancelButton, QDialogButtonBox::NoRole);
    auto applyButton = new QPushButton(tr("Apply"));

    buttonGroup->addButton(applyButton, QDialogButtonBox::YesRole);
    buttonGroup->setContentsMargins(10, 10, 10, 10);

    auto toolBar = new QToolBar;
    toolBar->addWidget(buttonGroup);
    toolBar->setMovable(false);

    addToolBar(Qt::BottomToolBarArea, toolBar);

    connect(saveAsSpace, &QPushButton::clicked, this, [=](){
        bool ok = false;
        QString spaceName = FileOperationManager::getNewSpaceName(nullptr, this->windowHandle(), &ok);
        if (!ok)
            return;

        QJsonObject configObj;
        // 获取标记配置
        auto labels = labelConditionEditor->getLabels();
        configObj["labels"] = QJsonArray::fromStringList(labels);

        // 获取条件配置
        auto conditions = basicConditionEditor->getCurrentConditions(true);
        configObj["conditions count"] = conditions.count();
        for (int i = 0; i < conditions.count(); i++) {
            configObj[QString::number(i)] = conditions[i]->conditionObject();
        }

        QList<QSharedPointer<UKUI::IDM::FileInfo>> infos;
        auto nameMatchFiles = SearchResultModel::globalInstance()->getNameMatchFiles();
        auto contentMatchFiles = SearchResultModel::globalInstance()->getContentMatchFiles();
        infos << nameMatchFiles;
        infos << contentMatchFiles;

        Q_EMIT conditionCreatedRequest(spaceName.isEmpty()? m_spaceName: spaceName, QVariant(configObj), true, infos);
        this->close();
    });
    connect(cancelButton, &QPushButton::clicked, this, &ConditionEditor::close);
    connect(addCondition, &QPushButton::clicked, basicConditionEditor, &BasicConditionEditor::onAddCondition);
    connect(removeCondition, &QPushButton::clicked, basicConditionEditor, &BasicConditionEditor::onRemoveCondition);
    connect(applyButton, &QPushButton::clicked, basicConditionEditor, [=]{
        switch (m_mode) {
        case Edit: {
            QJsonObject configObj;
            // 获取标记配置
            auto labels = labelConditionEditor->getLabels();
            configObj["labels"] = QJsonArray::fromStringList(labels);

            // 获取条件配置
            auto conditions = basicConditionEditor->getCurrentConditions();
            configObj["conditions count"] = conditions.count();
            for (int i = 0; i < conditions.count(); i++) {
                configObj[QString::number(i)] = conditions[i]->conditionObject();
            }

            QJsonDocument doc(configObj);

            QFile file(jsonPath);
            if (file.open(QFile::WriteOnly|QFile::Text)) {
                file.write(doc.toJson());
                file.close();

                if (!m_spaceName.isEmpty() && (property("currentSpaceName").toString() != m_spaceName)) {
                    // 更新空间名称
                    ModelController::globalInstance()->renameSpace(spaceId, m_spaceName);
                }
            } else {
                ModelController::globalInstance()->criticalWithDialog(nullptr, tr("Failed to save space condition for %1").arg(m_spaceName));
            }

            Q_EMIT conditionChanged(spaceId, spaceName, configObj);
            this->close();
            break;
        }
        case Create: {
            QJsonObject configObj;
            // 获取标记配置
            auto labels = labelConditionEditor->getLabels();
            configObj["labels"] = QJsonArray::fromStringList(labels);

            // 获取条件配置
            auto conditions = basicConditionEditor->getCurrentConditions();
            configObj["conditions count"] = conditions.count();
            for (int i = 0; i < conditions.count(); i++) {
                configObj[QString::number(i)] = conditions[i]->conditionObject();
            }

            Q_EMIT conditionCreatedRequest(m_spaceName, QVariant(configObj), false);
            this->close();
            break;
        }
        case Search: {
            auto conditions = basicConditionEditor->getCurrentConditions();
            auto searchModel = SearchResultProxyModel::globalInstance();
            searchModel->setSearchConditions(conditions);
            this->close();
            break;
        }
        default:
            break;
        }
    });

    applyButton->setDefault(true);
    applyButton->setFocus();
}

ConditionEditor::~ConditionEditor()
{
    if (currentEditor == this)
        currentEditor = nullptr;
}

QVariant ConditionEditor::pharseConditions(const QList<QSharedPointer<Condition>> &conditions)
{
    QJsonObject configObj;

    configObj["conditions count"] = conditions.count();
    for (int i = 0; i < conditions.count(); i++) {
        configObj[QString::number(i)] = conditions[i]->conditionObject();
    }

    return configObj;
}

QList<QSharedPointer<Condition>> ConditionEditor::generateContitionsFromDescription(const QVariant &description)
{
    QList<QSharedPointer<Condition>> result;

    QJsonObject jsonObj = description.toJsonObject();
    int count = jsonObj["conditions count"].toInt();
    for (int i = 0; i < count; i++) {
        auto conditionObj = jsonObj[QString::number(i)].toObject();
        auto condition = Condition::fromJsonObject(conditionObj);
        result << condition;
    }

    return result;
}

FlowLayout::FlowLayout(QWidget *parent, int margin, int hSpacing, int vSpacing)
    : QLayout(parent), m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::FlowLayout(int margin, int hSpacing, int vSpacing)
    : m_hSpace(hSpacing), m_vSpace(vSpacing)
{
    setContentsMargins(margin, margin, margin, margin);
}

FlowLayout::~FlowLayout()
{
    QLayoutItem *item;
    while ((item = takeAt(0)))
        delete item;
}

void FlowLayout::addItem(QLayoutItem *item)
{
    itemList.append(item);
}

int FlowLayout::horizontalSpacing() const
{
    if (m_hSpace >= 0) {
        return m_hSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutHorizontalSpacing);
    }
}

int FlowLayout::verticalSpacing() const
{
    if (m_vSpace >= 0) {
        return m_vSpace;
    } else {
        return smartSpacing(QStyle::PM_LayoutVerticalSpacing);
    }
}

int FlowLayout::count() const
{
    return itemList.size();
}

QLayoutItem *FlowLayout::itemAt(int index) const
{
    return itemList.value(index);
}

QLayoutItem *FlowLayout::takeAt(int index)
{
    if (index >= 0 && index < itemList.size())
        return itemList.takeAt(index);
    else
        return 0;
}

Qt::Orientations FlowLayout::expandingDirections() const
{
    return 0;
}

bool FlowLayout::hasHeightForWidth() const
{
    return true;
}

int FlowLayout::heightForWidth(int width) const
{
    int height = doLayout(QRect(0, 0, width, 0), true);
    return height;
}

void FlowLayout::setGeometry(const QRect &rect)
{
    QLayout::setGeometry(rect);
    doLayout(rect, false);
}

QSize FlowLayout::sizeHint() const
{
    return minimumSize();
}

QSize FlowLayout::minimumSize() const
{
    QSize size;
    QLayoutItem *item;
    foreach (item, itemList)
        size = size.expandedTo(item->minimumSize());

    size += QSize(2*margin(), 2*margin());
    return size;
}

int FlowLayout::doLayout(const QRect &rect, bool testOnly) const
{
    int left, top, right, bottom;
    getContentsMargins(&left, &top, &right, &bottom);
    QRect effectiveRect = rect.adjusted(+left, +top, -right, -bottom);
    int x = effectiveRect.x();
    int y = effectiveRect.y();
    int lineHeight = 0;

    QLayoutItem *item;
    foreach (item, itemList) {
        QWidget *wid = item->widget();
        int spaceX = horizontalSpacing();
        if (spaceX == -1)
            spaceX = wid->style()->layoutSpacing(
                        QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Horizontal);
        int spaceY = verticalSpacing();
        if (spaceY == -1)
            spaceY = wid->style()->layoutSpacing(
                        QSizePolicy::PushButton, QSizePolicy::PushButton, Qt::Vertical);
        int nextX = x + item->sizeHint().width() + spaceX;
        if (nextX - spaceX > effectiveRect.right() && lineHeight > 0) {
            x = effectiveRect.x();
            y = y + lineHeight + spaceY;
            nextX = x + item->sizeHint().width() + spaceX;
            lineHeight = 0;
        }

        if (!testOnly)
            item->setGeometry(QRect(QPoint(x, y), item->sizeHint()));

        x = nextX;
        lineHeight = qMax(lineHeight, item->sizeHint().height());
    }
    return y + lineHeight - rect.y() + bottom;
}
int FlowLayout::smartSpacing(QStyle::PixelMetric pm) const
{
    QObject *parent = this->parent();
    if (!parent) {
        return -1;
    } else if (parent->isWidgetType()) {
        QWidget *pw = static_cast<QWidget *>(parent);
        return pw->style()->pixelMetric(pm, 0, pw);
    } else {
        return static_cast<QLayout *>(parent)->spacing();
    }
}

#include "conditioneditor.moc"
