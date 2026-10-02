/*
 * Copyright 2026 KylinSoft Co., Ltd.
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

#include "slotextractor.h"

#include <Python.h>
#include <pybind11/embed.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include <filesystem>
#include <memory>
#include <mutex>

#include "assistant/intentdialoguemanager/dialoguemessage.h"
#include "intentslotrules.h"
#include "utils/logger.h"

namespace py = pybind11;

// GIL 管理的 RAII 辅助类
class GilLock {
public:
    GilLock() : gstate_(PyGILState_Ensure()) {}
    ~GilLock() { PyGILState_Release(gstate_); }
    // 禁止拷贝和移动
    GilLock(const GilLock&) = delete;
    GilLock& operator=(const GilLock&) = delete;

private:
    PyGILState_STATE gstate_;
};

// 静态变量：确保 Python 解释器只初始化一次
static std::unique_ptr<py::scoped_interpreter> g_python_interpreter = nullptr;
static std::once_flag g_python_init_flag;

class __attribute__((visibility("hidden"))) SlotExtractor::SlotExtractorImpl {
public:
    SlotExtractorImpl() {
        initializePythonInterpreterOnce();
        initializePythonExtractor();
    }

    ~SlotExtractorImpl() {
        if (Py_IsInitialized()) {
            GilLock lock;
            pythonSlotExtractor_ = py::object();
        }
    }

    void initializePythonInterpreterOnce() {
        std::call_once(g_python_init_flag, []() {
            if (!Py_IsInitialized()) {
                // 在初始化前设置不安装信号处理程序
                Py_InitializeEx(0);  // 参数0表示不安装信号处理
// 仅在 Python 3.6 及更早版本中需要显式初始化线程
#if PY_VERSION_HEX < 0x03070000
                PyEval_InitThreads();
#endif
                // 释放主线程的 GIL，允许其他线程获取 GIL
                PyEval_SaveThread();
            }
        });
    }

    bool initializePythonExtractor() {
        if (!Py_IsInitialized()) {
            LOGE("错误: Python解释器未初始化");
            return false;
        }

        GilLock lock;
        try {
            py::module sys = py::module::import("sys");
            py::list sys_path = sys.attr("path");
            sys_path.append(DialogueConfig::slotRegexPythonScript);

            py::module slot_extractor_module = py::module::import("slot_extractor");
            pythonSlotExtractor_ = slot_extractor_module.attr("SlotExtractor")();

            if (pythonSlotExtractor_.is_none()) {
                LOGE("错误: 创建 Python SlotExtractor 实例失败");
                pythonSlotExtractor_ = py::object();
                return false;
            }
            return true;
        } catch (const py::error_already_set& e) {
            LOGE("错误: 初始化 Python SlotExtractor 失败 - {}", e.what());
            PyErr_Print();
            pythonSlotExtractor_ = py::object();
            return false;
        } catch (const std::exception& e) {
            LOGE("错误: 初始化时发生异常 - {}", e.what());
            pythonSlotExtractor_ = py::object();
            return false;
        }
    }

    std::map<std::string, std::string> extractSlotsImpl(const std::string& user_input,
                                                        const std::string& predict_intent_name,
                                                        const std::vector<std::string>& slot_name_list) {
        if (pythonSlotExtractor_.is_none()) {
            LOGE("错误: Python extractor未初始化，无法提取槽位");
            return {};
        }

        if (user_input.empty() || predict_intent_name.empty()) {
            LOGE("错误: 输入参数为空");
            return {};
        }

        GilLock lock;
        try {
            py::object result =
                pythonSlotExtractor_.attr("extract_slots")(user_input, predict_intent_name, slot_name_list);
            return result.cast<std::map<std::string, std::string>>();
        } catch (const py::error_already_set& e) {
            LOGE("错误: Python异常 - {}", e.what());
            PyErr_Print();
            return {};
        } catch (const py::cast_error& e) {
            LOGE("错误: 类型转换失败 - {}", e.what());
            return {};
        } catch (const std::exception& e) {
            LOGE("错误: 提取槽位时发生异常 - {}", e.what());
            return {};
        }
    }

private:
    py::object pythonSlotExtractor_;
};

SlotExtractor::SlotExtractor(std::shared_ptr<IntentSlotRules> rules)
    : intentSlotRules_(rules), pImpl_(std::make_unique<SlotExtractorImpl>()) {}

SlotExtractor::~SlotExtractor() = default;

std::vector<ExtractedSlot> SlotExtractor::extractSlots(const std::string& user_input,
                                                       const std::string& predict_intent_name) {
    const auto& intent_slot_data = intentSlotRules_->getIntentSlotDataMap();
    auto it = intent_slot_data.find(predict_intent_name);
    if (it == intent_slot_data.end()) {
        LOGW("警告: 未找到意图 '{}' 的定义", predict_intent_name);
        return {};
    }

    const IntentDefinition& intent_def = it->second;
    std::vector<std::string> slot_name_list;
    std::map<std::string, bool> slot_name_to_required;
    slot_name_list.reserve(intent_def.slots.size());
    for (const auto& slot : intent_def.slots) {
        slot_name_list.push_back(slot.name);
        slot_name_to_required[slot.name] = slot.required;
    }

    auto extracted_slots = pImpl_->extractSlotsImpl(user_input, predict_intent_name, slot_name_list);
    std::vector<ExtractedSlot> result;
    result.reserve(extracted_slots.size());
    for (const auto& [name, value] : extracted_slots) {
        auto req_it = slot_name_to_required.find(name);
        bool is_required = req_it != slot_name_to_required.end() && req_it->second;
        result.emplace_back(ExtractedSlot{name, value, is_required});
    }
    return result;
}