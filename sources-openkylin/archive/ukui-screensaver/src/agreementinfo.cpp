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
#include "agreementinfo.h"

AgreementInfo::AgreementInfo(QObject *parent)
    : QObject(parent)
    , m_showLoginPrompt(false)
    , m_hideTitle(false)
    , m_promptTitle("")
    , m_promptText("")
    , m_promptTextFilePath("")
{
}

AgreementInfo::AgreementInfo(const AgreementInfo &agreementInfo)
    : QObject(agreementInfo.parent())
    , m_showLoginPrompt(agreementInfo.m_showLoginPrompt)
    , m_hideTitle(agreementInfo.m_hideTitle)
    , m_promptTitle(agreementInfo.m_promptTitle)
    , m_promptText(agreementInfo.m_promptText)
    , m_promptTextFilePath(agreementInfo.m_promptTextFilePath)
{
}

AgreementInfo::~AgreementInfo() {}

bool AgreementInfo::operator==(const AgreementInfo &agreementInfo) const
{
    return (
        agreementInfo.showLoginPrompt() == m_showLoginPrompt && agreementInfo.hideTitle() == m_hideTitle
        && agreementInfo.promptTitle() == m_promptTitle && agreementInfo.promptText() == m_promptText
        && agreementInfo.promptTextFilePath() == m_promptTextFilePath);
}

void AgreementInfo::updateShowLoginPrompt(const bool &showLoginPrompt)
{
    m_showLoginPrompt = showLoginPrompt;
}

void AgreementInfo::updateHideTitle(const bool &hideTitle)
{
    m_hideTitle = hideTitle;
}

void AgreementInfo::updatePromptTitle(const QString &promptTitle)
{
    m_promptTitle = promptTitle;
}

void AgreementInfo::updatePromptText(const QString &promptText)
{
    m_promptText = promptText;
}

void AgreementInfo::updatePromptTextFilePath(const QString &promptTextFilePath)
{
    m_promptTextFilePath = promptTextFilePath;
}

QDebug operator<<(QDebug stream, const AgreementInfo &agreementInfo)
{
    stream << "[" << agreementInfo.showLoginPrompt() << agreementInfo.hideTitle() << agreementInfo.promptTitle()
           << agreementInfo.promptText() << agreementInfo.promptTextFilePath() << "]";
    return stream;
}
