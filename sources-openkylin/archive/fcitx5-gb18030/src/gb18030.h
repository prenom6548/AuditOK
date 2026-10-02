/*
 * Copyright 2023 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

#ifndef _FCITX5_GBIM_GBIM_H_
#define _FCITX5_GBIM_GBIM_H_

#include <iconv.h>

#include <fcitx-utils/inputbuffer.h>
#include <fcitx/addonfactory.h>
#include <fcitx/addonmanager.h>
#include <fcitx/inputcontext.h>
#include <fcitx/inputcontextproperty.h>
#include <fcitx/inputmethodengine.h>
#include <fcitx/instance.h>

class GBIMEngine;

class GBIMState : public fcitx::InputContextProperty {
public:
    GBIMState(GBIMEngine *engine, fcitx::InputContext *ic)
        : engine_(engine), ic_(ic) {}

    void keyEvent(fcitx::KeyEvent &keyEvent);
    void updateUI();
    void reset() {
        buffer_.clear();
        updateUI();
    }

private:
    static bool isKeyboardDigitKey(const fcitx::Key &key);
    static bool isHexLetterKey(const fcitx::Key &key);
    static bool shouldProcessCharKey(const fcitx::Key &key);

    size_t getMaxUserInputBufferSize() const;
    bool shouldUpdateCandidateWords() const;

private:
    GBIMEngine *engine_;
    fcitx::InputContext *ic_;
    fcitx::InputBuffer buffer_{{fcitx::InputBufferOption::AsciiOnly,
                                fcitx::InputBufferOption::FixedCursor}};
};

class GBIMEngine : public fcitx::InputMethodEngineV2 {
public:
    GBIMEngine(fcitx::Instance *instance);

    void activate(const fcitx::InputMethodEntry &entry,
                  fcitx::InputContextEvent &event) override;
    void keyEvent(const fcitx::InputMethodEntry &entry,
                  fcitx::KeyEvent &keyEvent) override;

    void reset(const fcitx::InputMethodEntry &,
               fcitx::InputContextEvent &event) override;

    auto factory() const { return &factory_; }
    auto conv() const { return conv_; }
    auto instance() const { return instance_; }

private:
    fcitx::Instance *instance_;
    fcitx::FactoryFor<GBIMState> factory_;
    iconv_t conv_;
};

class GBIMEngineFactory : public fcitx::AddonFactory {
    fcitx::AddonInstance *create(fcitx::AddonManager *manager) override {
        return new GBIMEngine(manager->instance());
    }
};

#endif // _FCITX5_GBIM_GBIM_H_
