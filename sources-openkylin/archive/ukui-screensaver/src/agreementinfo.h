/*
 * Copyright (C) 2023 KylinSoft Co., Ltd.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 3, or (at your option)
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, see <http://www.gnu.org/licenses/>.
 *
 **/
#ifndef AGREEMENTINFO_H
#define AGREEMENTINFO_H

#include <QObject>
#include <memory>
#include <QDebug>

class AgreementInfo : public QObject
{
    Q_OBJECT
public:
    explicit AgreementInfo(QObject *parent = nullptr);
    explicit AgreementInfo(const AgreementInfo &agreementInfo);
    virtual ~AgreementInfo();

    bool operator==(const AgreementInfo &agreementInfo) const;

public:
    inline bool showLoginPrompt() const
    {
        return m_showLoginPrompt;
    }
    inline bool hideTitle() const
    {
        return m_hideTitle;
    }
    inline QString promptTitle() const
    {
        return m_promptTitle;
    }
    inline QString promptText() const
    {
        return m_promptText;
    }
    inline QString promptTextFilePath() const
    {
        return m_promptTextFilePath;
    }

    void updateShowLoginPrompt(const bool &showLoginPrompt);
    void updateHideTitle(const bool &hideTitle);
    void updatePromptTitle(const QString &promptTitle);
    void updatePromptText(const QString &promptText);
    void updatePromptTextFilePath(const QString &promptTextFilePath);

protected:
    bool m_showLoginPrompt;       //是否显示特别提示界面
    bool m_hideTitle;             //是否隐藏标题
    QString m_promptTitle;        //特别提示标题
    QString m_promptText;         //特别提示文本
    QString m_promptTextFilePath; //特别提示文本路径
};

typedef std::shared_ptr<AgreementInfo> AgreementInfoPtr;

QDebug operator<<(QDebug stream, const AgreementInfo &agreementInfo);

#endif // USERINFO_H
