// SPDX-FileCopyrightText: 2026 KylinSoft Co., Ltd.
//
// SPDX-License-Identifier: GPL-1.0-or-later

#ifndef DYNAMICUPDATE_H
#define DYNAMICUPDATE_H

#include <QObject>
#include <QVariant>

class DynamicDataUpdate : public QObject
{
    Q_OBJECT
    Q_PROPERTY(QVariantMap list READ list WRITE setList NOTIFY listChanged)

  public:
    explicit DynamicDataUpdate(QObject *parent = nullptr);

    void updateNode(const QJsonObject &json);

    QVariantMap list() const;
    void setList(const QVariantMap &info);

  signals:
    void listChanged();

  private:
    QVariantMap m_listMap;
    void updateList(const QJsonObject &json);
};

#endif // DYNAMICUPDATE_H
