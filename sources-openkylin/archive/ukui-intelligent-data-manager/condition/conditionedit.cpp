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

#include "conditionedit.h"

#include <QGridLayout>
#include <QComboBox>
#include <QLineEdit>
#include <QAction>
#include <QFileDialog>
#include <QStandardPaths>
#include <QDateTimeEdit>
#include <QMessageBox>

#include <QtGlobal>

#include <QDebug>
#include <QJsonObject>

ConditionEdit::ConditionEdit(Condition::Type type, QWidget *parent) : QWidget(parent)
{
    m_type = type;

    initUi();

    switchType(m_type);
}

ConditionEdit::ConditionEdit(QSharedPointer<Condition> condition, QWidget *parent) : QWidget(parent)
{
    m_type = condition->type();
    m_isAndCondition = condition->isAndCondition();

    initUi();

    switchType(m_type);
    auto gridLayout = static_cast<QGridLayout *>(layout());
    auto typeComboBox = qobject_cast<QComboBox *>(gridLayout->itemAtPosition(0, 0)->widget());
    typeComboBox->blockSignals(true);
    typeComboBox->setCurrentIndex(int(m_type));
    typeComboBox->blockSignals(false);
    QWidget *widget1 = gridLayout->itemAtPosition(0, 1)->widget();
    QWidget *widget2 = gridLayout->itemAtPosition(0, 2)->widget();
    switch (m_type) {
    case Condition::FileSize: {
        bool contains = condition->getFileSizeIsContains();
        int type = condition->getFileSizeType();
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        auto typeComboBox = qobject_cast<QComboBox *>(widget2);
        includeExculdeCombobox->setCurrentIndex(contains? 0: 1);
        typeComboBox->setCurrentIndex(type);
        break;
    }
    case Condition::FileType: {
        int type = condition->getFileTypeSelectionType();
        bool isSuffixType = type == -1;
        QStringList suffixes = condition->getFileTypeSuffixes();
        auto typeComboBox = qobject_cast<QComboBox *>(widget1);
        auto edit = qobject_cast<QLineEdit *>(widget2);
        typeComboBox->setCurrentIndex(isSuffixType? 0: type);
        edit->setText(isSuffixType? suffixes.join(','): nullptr);
        break;
    }
    case Condition::FileContent: {
        QString content = condition->getFileContentContent();
        auto edit = qobject_cast<QLineEdit *>(widget2);
        edit->setText(content);
        break;
    }
    case Condition::SearchLocation: {
        bool contains = condition->getSearchLocationIsContains();
        QString path = condition->getSearchLocationPath();
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        auto edit = qobject_cast<QLineEdit *>(widget2);
        includeExculdeCombobox->setCurrentIndex(contains? 0: 1);
        edit->setText(path);
        break;
    }
    case Condition::DateRange: {
        quint64 start = condition->getDateRangeStartMsec();
        quint64 end = condition->getDateRangeEndMsec();
        auto startEdit = qobject_cast<QDateTimeEdit *>(widget1);
        auto endEdit = qobject_cast<QDateTimeEdit *>(widget2);
        startEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(start));
        endEdit->setDateTime(QDateTime::fromMSecsSinceEpoch(end));
        break;
    }
    case Condition::ConditionMatchRule: {
        auto matchRule = condition->getConditionMatchRule();
        auto typeComboBox = qobject_cast<QComboBox *>(widget2);
        typeComboBox->setCurrentIndex(int(matchRule));
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        includeExculdeCombobox->setCurrentIndex(condition->getConditionMatchRuleIsContains()? 0: 1);
        break;
    }
    default: {
        bool contains = condition->getFileNameIsContains();
        QStringList keywords = condition->getFileNameKeywords();
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        auto edit = qobject_cast<QLineEdit *>(widget2);
        includeExculdeCombobox->setCurrentIndex(contains? 0: 1);
        edit->setText(keywords.join(','));
        break;
    }
    }
}

QSharedPointer<Condition> ConditionEdit::getCondition()
{
    auto gridLayout = static_cast<QGridLayout *>(layout());
    if (gridLayout->columnCount() < 3)
        return nullptr;
    auto condition = QSharedPointer<Condition>(new Condition(m_type));
    QWidget *widget1 = gridLayout->itemAtPosition(0, 1)->widget();
    QWidget *widget2 = gridLayout->itemAtPosition(0, 2)->widget();
    switch (m_type) {
    case Condition::FileName: {
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        bool contains = includeExculdeCombobox->currentIndex() == 0;
        auto edit = qobject_cast<QLineEdit *>(widget2);
        QString text = edit->text();
        QStringList keywords = text.split(',');
        condition->setFileNameCondition(contains, keywords);
        break;
    }
    case Condition::FileType: {
        auto typeComboBox = qobject_cast<QComboBox *>(widget1);
        bool isSuffixType = typeComboBox->currentIndex() == 0;
        if (isSuffixType) {
            auto edit = qobject_cast<QLineEdit *>(widget2);
            QString text = edit->text();
            QStringList suffixes = text.split(',');
            condition->setFileTypeCondition(suffixes);
        } else {
            condition->setFileTypeCondition(typeComboBox->currentIndex());
        }
        break;
    }
    case Condition::FileSize: {
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        bool contains = includeExculdeCombobox->currentIndex() == 0;
        auto typeComboBox = qobject_cast<QComboBox *>(widget2);
        int type = typeComboBox->currentIndex();
        condition->setFileSizeCondition(contains, type);
        break;
    }
    case Condition::FileContent: {
        auto edit = qobject_cast<QLineEdit *>(widget2);
        QString text = edit->text();
        condition->setFileContentCondition(text);
        break;
    }
    case Condition::SearchLocation: {
        auto includeExculdeCombobox = qobject_cast<QComboBox *>(widget1);
        bool contains = includeExculdeCombobox->currentIndex() == 0;
        auto edit = qobject_cast<QLineEdit *>(widget2);
        QString path = edit->text();
        condition->setSearchLocationCondition(contains, path);
        break;
    }
    case Condition::DateRange: {
        auto startEdit = qobject_cast<QDateTimeEdit *>(widget1);
        quint64 start = startEdit->dateTime().toMSecsSinceEpoch();
        auto endEdit = qobject_cast<QDateTimeEdit *>(widget2);
        quint64 end = endEdit->dateTime().toMSecsSinceEpoch();
        condition->setDateRangeCondition(0, start, end);
        break;
    }
    case Condition::ConditionMatchRule: {
        auto containsComboBox = qobject_cast<QComboBox *>(widget1);
        bool contains = containsComboBox->currentIndex() == 0;
        auto typeComboBox = qobject_cast<QComboBox *>(widget2);
        condition->setConditionMatchRule(Condition::MatchRule(typeComboBox->currentIndex()), contains);
        break;
    }
    default:
        break;
    }

    condition->setIsAndCondition(m_isAndCondition);
    return condition;
}

void ConditionEdit::initUi()
{
    setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);

    auto layout = new QGridLayout;

    auto typeCombobox = new QComboBox;
    QStringList types = {tr("File Name"), tr("File Type"), tr("File Size"), tr("File Content"), tr("Search Location"), tr("Date Range"), tr("Condition Match Rule")};
    typeCombobox->addItems(types);
    typeCombobox->setCurrentIndex(int(m_type));

    layout->addWidget(typeCombobox, 0, 0);
    setLayout(layout);

    connect(typeCombobox, QOverload<int>::of(&QComboBox::currentIndexChanged), this, [=](){
        switchType(Condition::Type(typeCombobox->currentIndex()));
    });
}

void ConditionEdit::switchType(Condition::Type type)
{
    QGridLayout *layout = static_cast<QGridLayout *>(this->layout());

    m_type = type;
    if (layout->columnCount() > 1) {
        auto widget1 = layout->itemAtPosition(0, 1)->widget();
        auto widget2 = layout->itemAtPosition(0, 2)->widget();
        layout->removeItem(layout->itemAtPosition(0, 1));
        layout->removeItem(layout->itemAtPosition(0, 2));
        widget1->deleteLater();
        widget2->deleteLater();
    }
    switch (type) {
    case Condition::Type::FileType: {
        auto fileTypeCombobox = new QComboBox(this);
        QStringList selections = {tr("By suffix"), tr("Folder"), tr("Plain Text"), tr("WPS Document"), tr("PDF"), tr("Image"), tr("Video"), tr("Audio")};
        fileTypeCombobox->addItems(selections);
        layout->addWidget(fileTypeCombobox, 0, 1);

        auto nameEdit = new QLineEdit(this);
        nameEdit->setPlaceholderText(tr("Input file suffix name, use \",\" to split"));
        layout->addWidget(nameEdit, 0, 2);

        connect(fileTypeCombobox, &QComboBox::currentTextChanged, nameEdit, [=]{
            if (fileTypeCombobox->currentIndex() != 0) {
                nameEdit->setEnabled(false);
            } else {
                nameEdit->setEnabled(true);
            }
        });
        break;
    }
    case Condition::Type::FileSize: {
        auto includeExcludeCombobox = new QComboBox;
        QStringList selections = {tr("Contains"), tr("Not Contains")};
        includeExcludeCombobox->addItems(selections);
        layout->addWidget(includeExcludeCombobox, 0, 1);

        auto sizeComboBox = new QComboBox;
        QStringList sizes = {tr("Empty(0K)"), tr("Tiny(0-16K)"), tr("Small(16K-1M)"), tr("Medium(1M-128M)"), tr("Big(128M-1G)"), tr("Large(1-4G)"), tr("Great(>4G)")};
        sizeComboBox->addItems(sizes);
        layout->addWidget(sizeComboBox, 0, 2);
        break;
    }
    case Condition::Type::FileContent: {
        auto includeExcludeCombobox = new QComboBox;
        QStringList selections = {tr("Contains")/*, tr("Not Contains")*/};
        includeExcludeCombobox->addItems(selections);
        layout->addWidget(includeExcludeCombobox, 0, 1);

        auto nameEdit = new QLineEdit;
        nameEdit->setPlaceholderText(tr("Input content"));
        layout->addWidget(nameEdit, 0, 2);
        break;
    }
    case Condition::Type::SearchLocation: {
        auto includeExcludeCombobox = new QComboBox;
        QStringList selections = {tr("Contains"), tr("Not Contains")};
        includeExcludeCombobox->addItems(selections);
        layout->addWidget(includeExcludeCombobox, 0, 1);

        auto nameEdit = new QLineEdit;
        nameEdit->setReadOnly(true);
        nameEdit->setPlaceholderText(tr("Please add file path"));
        auto folderAction = new QAction(QIcon::fromTheme("folder-symbolic"), tr("Select file path"));
        nameEdit->addAction(folderAction, QLineEdit::TrailingPosition);
        connect(folderAction, &QAction::triggered, this, [=]{
            QString userHome = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
            QString directory = QFileDialog::getExistingDirectory(0, tr("Select file path"), userHome);
            if (directory.isEmpty()) {
                return;
            }
            if (directory.startsWith(userHome + "/") || directory == userHome) {
                nameEdit->setText(directory);
            } else {
                QMessageBox::warning(parentWidget()->topLevelWidget(), tr("Invalid Path"), tr("Please select a directory under user home directory."));
            }
        });
        layout->addWidget(nameEdit, 0, 2);
        break;
    }
    case Condition::Type::DateRange: {
//            auto selectionsComboBox = new QComboBox;
//            QStringList selections = {tr("Custom"), tr("Today"), tr("Yestoday"), tr("This Week"), tr("Last Week"), tr("This Month"), tr("Last Month"), tr("This Year"), tr("Last Year")};
//            selectionsComboBox->addItems(selections);

        auto startTime = new QDateTimeEdit;
        startTime->setDateTime(QDateTime::currentDateTime());
        startTime->setCalendarPopup(true);
        layout->addWidget(startTime, 0, 1);

        auto endTime = new QDateTimeEdit;
        endTime->setDateTime(QDateTime::currentDateTime());
        endTime->setCalendarPopup(true);
        layout->addWidget(endTime, 0, 2);

        startTime->setMaximumDateTime(endTime->dateTime());
        endTime->setMinimumDateTime(startTime->dateTime());

        connect(startTime, &QDateTimeEdit::dateTimeChanged, endTime, &QDateTimeEdit::setMinimumDateTime);
        connect(endTime, &QDateTimeEdit::dateTimeChanged, startTime, &QDateTimeEdit::setMaximumDateTime);
        break;
    }
    case Condition::ConditionMatchRule: {
        auto includeExcludeCombobox = new QComboBox;
        QStringList selections = {tr("Contains"), tr("Not Contains")};
        includeExcludeCombobox->addItems(selections);
        layout->addWidget(includeExcludeCombobox, 0, 1);

        auto typeComboBox = new QComboBox;
        QStringList types = {tr("Name or Content"), tr("Name and Content")};
        typeComboBox->addItems(types);
        layout->addWidget(typeComboBox, 0, 2);
        break;
    }
    default:
        auto includeExcludeCombobox = new QComboBox;
        QStringList selections = {tr("Contains"), tr("Not Contains")};
        includeExcludeCombobox->addItems(selections);
        layout->addWidget(includeExcludeCombobox, 0, 1);

        auto nameEdit = new QLineEdit;
        nameEdit->setPlaceholderText(tr("Input file name, use \",\" to split"));
        layout->addWidget(nameEdit, 0, 2);
        break;
    }
}
