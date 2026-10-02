/*
 * Copyright 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#include "nlpsession.h"


NlpResult nlp::NlpSession::init(const std::string & engineName,
    const std::string & apiKey, const std::string & secretKey,
    const std::string & appId)
{
    bool initResult = textProcessor_.initEngine(engineName, apiKey, secretKey, appId);
    if (initResult) {
        return NLP_SUCCESS;
    }
    return NLP_SESSION_ERROR;
}

void nlp::NlpSession::setResultCallback(NlpResultCallback callback, std::any userData)
{
    resultCallback_ = std::move(callback);
    userData_ = userData;

    textProcessor_.setChatCallback([this](const std::string &reply, void *) {
        resultCallback_(reply.c_str(), std::any_cast<void *>(userData_));
    });
}

void nlp::NlpSession::setActor(int actorId)
{
    textProcessor_.setActor(actorId);
}

NlpResult nlp::NlpSession::chat(const std::string &message)
{
    textProcessor_.chat(message);

    return NLP_SUCCESS;
}

void nlp::NlpSession::stopChat()
{
    textProcessor_.stopChat();
}

void nlp::NlpSession::setContext(int size)
{
    textProcessor_.setContext(size);
}

void nlp::NlpSession::clearContext()
{
    textProcessor_.clearContext();
}
