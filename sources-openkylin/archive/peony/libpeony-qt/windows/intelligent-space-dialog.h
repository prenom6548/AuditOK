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

#ifndef INTELLIGENT_SPACE_DIALOG_H
#define INTELLIGENT_SPACE_DIALOG_H

#include <QDialog>
#include <QLineEdit>
#include <QCheckBox>
#include <QLabel>
#include <QPushButton>
#include "peony-core_global.h"

class PEONYCORESHARED_EXPORT IntelligentSpaceRenameDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IntelligentSpaceRenameDialog(const QString &currentName, QWidget *parent = nullptr);

    QString getNewName() const;
    bool isDynamicUpdate() const;

private:
    void setupUi();
    QString m_currentName;
    QLineEdit *m_nameEdit = nullptr;
    QCheckBox *m_dynamicCheck = nullptr;
};

class PEONYCORESHARED_EXPORT IntelligentSpaceCreateDialog : public QDialog
{
    Q_OBJECT

public:
    explicit IntelligentSpaceCreateDialog(QWidget *parent = nullptr);
    ~IntelligentSpaceCreateDialog();
Q_SIGNALS:
    void spaceCreateRequested(const QString &name, const QString &path, bool dynamicUpdate);

public:
    QString getNewName() const;
    bool isDynamicUpdate() const;

private:
    void setupUi();
    QLineEdit *m_nameEdit = nullptr;
    QCheckBox *m_dynamicCheck = nullptr;
};

// Global function for byte limit truncation
QString truncateToByteLimitSpaceName(const QString &text, int maxBytes);

#endif // INTELLIGENT_SPACE_DIALOG_H
