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

#include "gb18030.h"

#include <utility>

#include <fcitx-utils/utf8.h>
#include <fcitx/candidatelist.h>
#include <fcitx/inputpanel.h>
#include <fcitx/userinterfacemanager.h>

namespace {

static const std::array<fcitx::Key, 16> selectionKeys = {
    fcitx::Key{FcitxKey_0}, fcitx::Key{FcitxKey_1}, fcitx::Key{FcitxKey_2},
    fcitx::Key{FcitxKey_3}, fcitx::Key{FcitxKey_4}, fcitx::Key{FcitxKey_5},
    fcitx::Key{FcitxKey_6}, fcitx::Key{FcitxKey_7}, fcitx::Key{FcitxKey_8},
    fcitx::Key{FcitxKey_9}, fcitx::Key{FcitxKey_a}, fcitx::Key{FcitxKey_b},
    fcitx::Key{FcitxKey_c}, fcitx::Key{FcitxKey_d}, fcitx::Key{FcitxKey_e},
    fcitx::Key{FcitxKey_f},

};

// 四字节部分采用GB/T11383——1989未采用的0x30～0x39作为对双字节编码扩充的后缀，
// 编码范围为0x81308130～0xFE39FE39
static int is4ByteCode(const std::string &input) {
    return input.length() >= 3 && input[2] == FcitxKey_3;
}

static const size_t MAX_CANDIDATE_COUNT_FOR_TWO_BYTE_CODE = 16;
static const size_t MAX_CANDIDATE_COUNT_FOR_FOUR_BYTE_CODE = 10;
static int getMaxCandidateCount(const std::string &input) {
    return is4ByteCode(input) ? MAX_CANDIDATE_COUNT_FOR_FOUR_BYTE_CODE
                              : MAX_CANDIDATE_COUNT_FOR_TWO_BYTE_CODE;
}

static const size_t TWO_BYTE_CODE_COUNT = 2;
static const size_t FOUR_BYTE_CODE_COUNT = 4;
static size_t getMaxByteCodeCount(const std::string &input) {
    return is4ByteCode(input) ? FOUR_BYTE_CODE_COUNT : TWO_BYTE_CODE_COUNT;
}

static unsigned char charToValue(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';
    }

    if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;
    }

    if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;
    }

    return 0;
}

class GBIMCandidateWord : public fcitx::CandidateWord {
public:
    GBIMCandidateWord(GBIMEngine *engine, std::string text) : engine_(engine) {
        setText(fcitx::Text(std::move(text)));
    }

    void select(fcitx::InputContext *inputContext) const override {
        inputContext->commitString(text().toString());
        auto state = inputContext->propertyFor(engine_->factory());
        state->reset();
    }

private:
    GBIMEngine *engine_;
};

class GBIMCandidateList : public fcitx::CandidateList {
public:
    GBIMCandidateList(GBIMEngine *engine, fcitx::InputContext *ic,
                      const std::string &userInput)
        : engine_(engine), ic_(ic), userInput_(userInput) {
        const int maxCandidateCount = getMaxCandidateCount(userInput_);
        std::vector<std::string> laybelList = {"0", "1", "2", "3", "4", "5",
                                               "6", "7", "8", "9", "a", "b",
                                               "c", "d", "e", "f"};
        for (int i = 0; i < maxCandidateCount; i++) {
            labels_[i].append(laybelList[i]);
            labels_[i].append(". ");
        }
        generate();
    }

    const fcitx::Text &label(int idx) const override { return labels_[idx]; }

    const fcitx::CandidateWord &candidate(int idx) const override {
        return *candidates_[idx];
    }
    int size() const override { return getMaxCandidateCount(userInput_); }
    fcitx::CandidateLayoutHint layoutHint() const override {
        return fcitx::CandidateLayoutHint::NotSet;
    }

    int cursorIndex() const override { return 0; }

private:
    typedef struct {
        char buffer[5];
        size_t length;
    } InputCodeBuffer;

private:
    void initInputCodeBuffer() {
        memset(&inputCodeBuffer_, 0, sizeof(inputCodeBuffer_));

        inputCodeBuffer_.length = getMaxByteCodeCount(userInput_);

        for (size_t i = 0; i < inputCodeBuffer_.length; i++) {
            size_t userInputIndex = i * 2;

            size_t least4ByteValue = 0;
            if (userInputIndex + 1 < userInput_.length()) {
                least4ByteValue = charToValue(userInput_[userInputIndex + 1]);
            }

            inputCodeBuffer_.buffer[i] =
                charToValue(userInput_[userInputIndex]) * 16 + least4ByteValue;
        }
    }

    bool isInBlacklist(char code[2]) {
        static char blacklist[][2] = {
            {(char)0xfd, (char)0x9c}, {(char)0xfd, (char)0x9d},
            {(char)0xfd, (char)0x9e}, {(char)0xfd, (char)0x9f},
            {(char)0xfd, (char)0xa0}, {(char)0xfe, (char)0x40},
            {(char)0xfe, (char)0x41}, {(char)0xfe, (char)0x47},
            {(char)0xfe, (char)0x49}};

        size_t blacklistLength = sizeof(blacklist) / sizeof(blacklist[0]);
        for (size_t i = 0; i < blacklistLength; i++) {
            if (blacklist[i][0] == code[0] && blacklist[i][1] == code[1]) {
                return true;
            }
        }

        return false;
    }

    void convert() {
        memset(utf8WordBuffer_, 0, sizeof(utf8WordBuffer_));
        if (isInBlacklist(inputCodeBuffer_.buffer)) {
            return;
        }

        char *inputBuffer = inputCodeBuffer_.buffer;
        size_t inputBufferBytesLeft = inputCodeBuffer_.length;
        char *outputBuffer = utf8WordBuffer_;
        size_t outputBufferBytesLeft = sizeof(utf8WordBuffer_);

        iconv(engine_->conv(), &inputBuffer, &inputBufferBytesLeft,
              &outputBuffer, &outputBufferBytesLeft);
    }

    void generate() {
        const int maxCandidateCount = getMaxCandidateCount(userInput_);

        initInputCodeBuffer();

        for (int i = 0; i < maxCandidateCount; i++) {
            convert();

            candidates_[i] =
                std::make_unique<GBIMCandidateWord>(engine_, utf8WordBuffer_);

            inputCodeBuffer_.buffer[inputCodeBuffer_.length - 1]++;
        }
    }

    GBIMEngine *engine_;
    fcitx::InputContext *ic_;
    fcitx::Text labels_[16];
    std::unique_ptr<GBIMCandidateWord> candidates_[16];
    std::string userInput_;

    InputCodeBuffer inputCodeBuffer_;

    static const int UTF8_MAX_LENGTH = 6;

    char utf8WordBuffer_[UTF8_MAX_LENGTH + 1];
}; // namespace

} // namespace

// static
bool GBIMState::isKeyboardDigitKey(const fcitx::Key &key) {
    return (key.sym() >= FcitxKey_0 && key.sym() <= FcitxKey_9) ||
           (key.sym() >= FcitxKey_KP_0 && key.sym() <= FcitxKey_KP_9);
}

// static
bool GBIMState::isHexLetterKey(const fcitx::Key &key) {
    if (!key.isLAZ() && !key.isUAZ()) {
        return false;
    }

    return (key.sym() >= FcitxKey_a && key.sym() <= FcitxKey_f) ||
           (key.sym() >= FcitxKey_A && key.sym() <= FcitxKey_F);
}

// static
bool GBIMState::shouldProcessCharKey(const fcitx::Key &key) {
    return isKeyboardDigitKey(key) || isHexLetterKey(key);
}

void GBIMState::keyEvent(fcitx::KeyEvent &event) {
    if (auto candidateList = ic_->inputPanel().candidateList()) {
        if (candidateList->size() <= 0) {
            return;
        }

        int idx = event.key().keyListIndex(selectionKeys);
        if (idx >= 0 && idx < candidateList->size()) {
            candidateList->candidate(idx).select(ic_);
            return event.filterAndAccept();
        }

        if (event.key().check(FcitxKey_space) ||
            event.key().check(FcitxKey_KP_Space)) {
            int idx = candidateList->cursorIndex();
            if (idx < 0) {
                idx = 0;
            }
            candidateList->candidate(idx).select(ic_);
            return event.filterAndAccept();;
        }
    }

    if (buffer_.empty()) {
        if (!shouldProcessCharKey(event.key())) {
            if (event.key().isUAZ() || event.key().isLAZ()) {
                return event.filterAndAccept();
            } else {
                return event.filter();
            }
        }
    } else {
        if (event.key().check(FcitxKey_BackSpace)) {
            buffer_.backspace();
            updateUI();
            return event.filterAndAccept();
        }
        if (event.key().check(FcitxKey_Return)) {
            ic_->commitString(buffer_.userInput());
            reset();
            return event.filterAndAccept();
        }
        if (event.key().check(FcitxKey_Escape)) {
            reset();
            return event.filterAndAccept();
        }
        if (!shouldProcessCharKey(event.key())) {
            return event.filterAndAccept();
        }
    }

    if (event.key().sym() >= FcitxKey_KP_0 && event.key().sym() <= FcitxKey_KP_9) {
        buffer_.type(event.key().sym() - 65408);
    } else {
        buffer_.type(event.key().sym());
    }

    updateUI();
    return event.filterAndAccept();
}

// GB18030使用双字节和四字节对汉字进行编码并且使用16进制数字
// 来表示相应对内码，2个16进制符号编码的数字可以用一个字节表示
// 对应对数值
size_t GBIMState::getMaxUserInputBufferSize() const {
    return getMaxByteCodeCount(buffer_.userInput()) * 2;
}

bool GBIMState::shouldUpdateCandidateWords() const {
    return buffer_.userInput().size() == getMaxUserInputBufferSize() - 1;
}

void GBIMState::updateUI() {
    auto &inputPanel = ic_->inputPanel();
    inputPanel.reset();
    if (shouldUpdateCandidateWords()) {
        inputPanel.setCandidateList(std::make_unique<GBIMCandidateList>(
            engine_, ic_, buffer_.userInput()));
    }
    if (ic_->capabilityFlags().test(fcitx::CapabilityFlag::Preedit)) {
        fcitx::Text preedit(buffer_.userInput(),
                            fcitx::TextFormatFlag::HighLight);
        inputPanel.setClientPreedit(preedit);
    } else {
        fcitx::Text preedit(buffer_.userInput());
        inputPanel.setPreedit(preedit);
    }
    ic_->updateUserInterface(fcitx::UserInterfaceComponent::InputPanel);
    ic_->updatePreedit();
}

GBIMEngine::GBIMEngine(fcitx::Instance *instance)
    : instance_(instance), factory_([this](fcitx::InputContext &ic) {
          return new GBIMState(this, &ic);
      }) {
    conv_ = iconv_open("UTF-8", "GB18030");
    if (conv_ == reinterpret_cast<iconv_t>(-1)) {
        throw std::runtime_error("Failed to create converter");
    }

    instance->inputContextManager().registerProperty("GBIMState", &factory_);
}

void GBIMEngine::activate(const fcitx::InputMethodEntry &entry,
                          fcitx::InputContextEvent &event) {
    FCITX_UNUSED(entry);
    FCITX_UNUSED(event);
}

void GBIMEngine::keyEvent(const fcitx::InputMethodEntry &entry,
                          fcitx::KeyEvent &keyEvent) {
    FCITX_UNUSED(entry);

    if (keyEvent.isRelease() || keyEvent.key().states()) {
        return;
    }

    auto ic = keyEvent.inputContext();
    auto *state = ic->propertyFor(&factory_);
    state->keyEvent(keyEvent);
}

void GBIMEngine::reset(const fcitx::InputMethodEntry &,
                       fcitx::InputContextEvent &event) {
    auto *state = event.inputContext()->propertyFor(&factory_);
    state->reset();
}

FCITX_ADDON_FACTORY(GBIMEngineFactory);
