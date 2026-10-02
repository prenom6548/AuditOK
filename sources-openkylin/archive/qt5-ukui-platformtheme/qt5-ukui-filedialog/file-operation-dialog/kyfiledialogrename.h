/*
 * KWin Style UKUI
 *
 * Copyright (C) 2023, KylinSoft Co., Ltd.
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
 * Authors: Jing Tan <tanjing@kylinos.cn>
 *
 */

#ifndef KYFILEDIALOGRENAME_H
#define KYFILEDIALOGRENAME_H

#include <QTextEdit>
#include <QLabel>
#include "file-operation-error-handler.h"
#include "kyfileoperationdialog.h"
namespace UKUIFileDialog {

enum TurnCateType {
    Post = 0,
    Front,
};

class MyTextEdit : public QTextEdit
{
    Q_OBJECT
public:
    explicit MyTextEdit(QWidget *parent = nullptr);
    ~MyTextEdit();

};


class KyFileDialogRename : public KyFileOperationDialog, public Peony::FileOperationErrorHandler
{
    Q_OBJECT
public:
    explicit KyFileDialogRename(QWidget *parent = nullptr);
    ~KyFileDialogRename();

    void handle(Peony::FileOperationError &error) override;
    const QString truncateDestFileName(const QString &uri, const int cateType, QString fsType);
    QWidget *m_currentWidget = nullptr;

    bool dialogResult();

    void setCurrentUri(QString uri);
    QString currentUri();
    void setMaxLength(int maxLength);
    protected:
    void showEvent(QShowEvent *event) override;


Q_SIGNALS:
    void changeFileName(QString name);
    void muchLongNameReject();

private:
    bool m_dialogAccept = true;
    QString m_truncateFileName;
    QString m_currentUri;
    int m_maxLength = 255;
    QLabel *m_content = nullptr;
    QLabel *m_iconTopLabel = nullptr;
    QString m_labelText;
};
}
#endif // KYFILEDIALOGRENAME_H
