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

#ifndef CONDITIONEDITOR_H
#define CONDITIONEDITOR_H

#include <QMainWindow>

namespace UKUI {
namespace IDM {
class FileInfo;
}
}

class Condition;

class ConditionEditor : public QMainWindow
{
    Q_OBJECT
public:
    enum Mode {
        Create,
        Edit,
        Search
    };

    explicit ConditionEditor(int spaceId, const QString &spaceName, Mode mode = Edit, QWidget *parent = nullptr);
    ~ConditionEditor();

    static QVariant pharseConditions(const QList<QSharedPointer<Condition> > &conditions);
    static QList<QSharedPointer<Condition> > generateContitionsFromDescription(const QVariant &description);

signals:
    void conditionChanged(int spaceId, const QString &spaceName, const QVariant &conditionDescription);
    void conditionCreatedRequest(const QString &spaceName, const QVariant &conditionDescription, bool isCreatedFromSearch, const QList<QSharedPointer<UKUI::IDM::FileInfo>> &infos = QList<QSharedPointer<UKUI::IDM::FileInfo>>());

private:
    int m_spaceId = -1;
    QString m_spaceName;
    Mode m_mode = Edit;
};

#endif // CONDITIONEDITOR_H
