/*
 * Copyright (C) 2025, KylinSoft Co., Ltd.
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
 * Authors: JinQin He <hejinqin@kylinos.cn>
 *
 */

#include "intelligent-space-dialog.h"
#include "search-bar-container.h"
#include <QMessageBox>
#include <QApplication>
IntelligentSpaceRenameDialog::IntelligentSpaceRenameDialog(const QString &currentName, QWidget *parent)
    : QDialog(parent), m_currentName(currentName)
{
    setWindowFlags(Qt::Dialog);
    setWindowIcon(QIcon::fromTheme("folder-space"));
    setWindowTitle(tr("Rename Intelligent Space"));
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);
    setupUi();
    setFixedSize(424,224);
}

void IntelligentSpaceRenameDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 10, 24, 20);
    mainLayout->setSpacing(16);

    // Space Name Label
    QLabel *nameLabel = new QLabel(tr("Space Name And Description"), this);
    QFont font = nameLabel->font();
    font.setBold(true);
    nameLabel->setFont(font);
    mainLayout->addWidget(nameLabel);

    // Name Input
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(0);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setFixedHeight(36);
    m_nameEdit->setPlaceholderText(tr("Describe the files you want to aggregate, e.g. 'Contracts PDF from 2026'"));
    m_nameEdit->setText(m_currentName);
    m_nameEdit->setFocus();
    m_nameEdit->selectAll();
    Peony::GradientLineEditStyle *lineStyle = new Peony::GradientLineEditStyle();
    m_nameEdit->setStyle(lineStyle);

    // Connect text change for byte limit check
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (text.toUtf8().size() > 255) {
            QString truncated = truncateToByteLimitSpaceName(text, 255);
            m_nameEdit->blockSignals(true);
            m_nameEdit->setText(truncated);
            m_nameEdit->blockSignals(false);
        }
    });

    inputLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(inputLayout);

    // Dynamic Update Checkbox Row
    mainLayout->addSpacing(6);
    QHBoxLayout *checkLayout = new QHBoxLayout();
    checkLayout->setSpacing(20);

    m_dynamicCheck = new QCheckBox(tr("Rename content synchronization is updated as space aggregation condition"), this);
    m_dynamicCheck->setChecked(true);
    checkLayout->addWidget(m_dynamicCheck);
    checkLayout->addStretch();

    mainLayout->addLayout(checkLayout);

    // Button Row
    mainLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setFixedSize(96, 36);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *okBtn = new QPushButton(tr("OK"), this);
    okBtn->setFixedSize(96, 36);
    okBtn->setDefault(true);
    okBtn->setAutoDefault(true);
    okBtn->setCheckable(true);
    okBtn->setChecked(true);
    connect(okBtn, &QPushButton::clicked, this, [this]() {
        accept();
    });

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);

    mainLayout->addLayout(buttonLayout);
}

QString IntelligentSpaceRenameDialog::getNewName() const
{
    return m_nameEdit->text();
}

bool IntelligentSpaceRenameDialog::isDynamicUpdate() const
{
    return m_dynamicCheck->isChecked();
}

// IntelligentSpaceCreateDialog implementation
IntelligentSpaceCreateDialog::IntelligentSpaceCreateDialog(QWidget *parent)
    : QDialog(parent)
{
    setWindowFlags(Qt::Dialog);
    setWindowIcon(QIcon::fromTheme("folder-space"));
    setWindowTitle(tr("Create Intelligent Space"));
    QPalette palette = this->palette();
    palette.setColor(QPalette::Window, Qt::white);
    setPalette(palette);
    setupUi();
    setFixedSize(424,224);
}

IntelligentSpaceCreateDialog::~IntelligentSpaceCreateDialog()
{

}


void IntelligentSpaceCreateDialog::setupUi()
{
    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(24, 10, 24, 20);
    mainLayout->setSpacing(16);

    // Space Name Label
    QLabel *nameLabel = new QLabel(tr("Space Name And Description"), this);
    QFont font = nameLabel->font();
    font.setBold(true);
    nameLabel->setFont(font);
    mainLayout->addWidget(nameLabel);

    // Name Input
    QHBoxLayout *inputLayout = new QHBoxLayout();
    inputLayout->setSpacing(0);

    m_nameEdit = new QLineEdit(this);
    m_nameEdit->setFixedHeight(36);
    m_nameEdit->setPlaceholderText(tr("Please input space name"));
    m_nameEdit->setFocus();
    Peony::GradientLineEditStyle *lineStyle = new Peony::GradientLineEditStyle();
    m_nameEdit->setStyle(lineStyle);

    // Connect text change for byte limit check
    connect(m_nameEdit, &QLineEdit::textChanged, this, [this](const QString &text) {
        if (text.toUtf8().size() > 255) {
            QString truncated = truncateToByteLimitSpaceName(text, 255);
            m_nameEdit->blockSignals(true);
            m_nameEdit->setText(truncated);
            m_nameEdit->blockSignals(false);
        }
    });

    inputLayout->addWidget(m_nameEdit);
    mainLayout->addLayout(inputLayout);

    // Dynamic Update Checkbox Row
    mainLayout->addSpacing(6);
    QHBoxLayout *checkLayout = new QHBoxLayout();
    checkLayout->setSpacing(10);

    m_dynamicCheck = new QCheckBox(tr("realtime intelligent updates"), this);
    m_dynamicCheck->setChecked(true);
    checkLayout->addWidget(m_dynamicCheck);

    QLabel *helpIcon = new QLabel(this);
    helpIcon->setPixmap(QIcon::fromTheme("help-about-symbolic").pixmap(16, 16));
    helpIcon->setToolTip(tr("When enabled, the system continuously monitors for new files matching the above criteria and automatically adds them to this space."));
    checkLayout->addWidget(helpIcon);
    checkLayout->addStretch();

    mainLayout->addLayout(checkLayout);

    // Button Row
    mainLayout->addSpacing(10);
    QHBoxLayout *buttonLayout = new QHBoxLayout();
    buttonLayout->setSpacing(8);

    QPushButton *cancelBtn = new QPushButton(tr("Cancel"), this);
    cancelBtn->setFixedSize(96, 36);
    connect(cancelBtn, &QPushButton::clicked, this, &QDialog::reject);

    QPushButton *okBtn = new QPushButton(tr("OK"), this);
    okBtn->setFixedSize(96, 36);
    okBtn->setDefault(true);
    okBtn->setAutoDefault(true);
    okBtn->setCheckable(true);
    okBtn->setChecked(true);
    QWidget *widget = QApplication::topLevelAt(QCursor::pos());
    connect(okBtn, &QPushButton::clicked, this, [=]() {
        bool isDymaic =m_dynamicCheck->isChecked();
        bool success = Peony::IDMDBusManager::getInstance()->onSpaceCreateRequestWithFiles(m_nameEdit->text(), "", false, QStringList());
        close();
        if (success) {
            Peony::IDMDBusManager::getInstance()->updateSpacestatus(m_nameEdit->text(), isDymaic);
        }else{
            QMessageBox::warning(widget,"", tr("Failed to create space %1,%1 is existed").arg(m_nameEdit->text()), QMessageBox::Ok);
        }
    });

    buttonLayout->addStretch();
    buttonLayout->addWidget(cancelBtn);
    buttonLayout->addWidget(okBtn);

    mainLayout->addLayout(buttonLayout);
}

QString IntelligentSpaceCreateDialog::getNewName() const
{
    return m_nameEdit->text();
}

bool IntelligentSpaceCreateDialog::isDynamicUpdate() const
{
    return m_dynamicCheck->isChecked();
}

// Global function
QString truncateToByteLimitSpaceName(const QString &text, int maxBytes)
{
    QByteArray utf8Data = text.toUtf8();
    if (utf8Data.size() <= maxBytes) {
        return text;
    }
    int truncatePos = maxBytes;
    while (truncatePos > 0 && (utf8Data.at(truncatePos) & 0xC0) == 0x80) {
        truncatePos--;
    }
    if (truncatePos == 0) {
        return "";
    }
    QByteArray safeData = utf8Data.left(truncatePos);
    return QString::fromUtf8(safeData);
}
