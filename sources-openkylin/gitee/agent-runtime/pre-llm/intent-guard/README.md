# intent-guard 插件

每次用户输入后、LLM 调用前，执行外部脚本做意图识别。命中直接返回结果（零 LLM 调用），未命中穿透正常流程。

## 安装

```bash
# 1. 复制插件到 hermes 插件目录
cp -r tempplugin/intent-guard ~/.kylin-agent-runtime/plugins/

# 2. 复制示例脚本（或用自己的）
mkdir -p ~/.kylin-agent-runtime/scripts
cp tempplugin/intent-guard/intent_guard.py.example ~/.kylin-agent-runtime/scripts/intent_guard.py
chmod +x ~/.kylin-agent-runtime/scripts/intent_guard.py

# 3. 修改 conversation_loop.py 支持 final_response 短路
#    （见下方「框架改动」）

# 4. 重启 gateway
kylin-agent-runtime gateway restart

# 5. 验证（发送 "现在几点了？" 应该秒回当前时间）
```

## 环境变量

| 变量 | 默认值 | 说明 |
|------|-------|------|
| `HERMES_INTENT_GUARD_SCRIPT` | `~/.kylin-agent-runtime/scripts/intent_guard.py` | 外部脚本路径，不存在则插件空转 |
| `HERMES_INTENT_GUARD_TIMEOUT` | `5` | 脚本超时秒数，超时视为穿透 |

## 外部脚本协议

**输入**（stdin JSON）：
```json
{"user_message": "现在几点了？", "session_id": "...", "platform": "telegram", "is_first_turn": false}
```

**输出**（stdout）：
- `None` / 空 / 退出码非 0 → 穿透，正常 Agent 流程
- 其他字符串 → 直接作为 final_response 返回给用户

## 框架改动

需修改 `~/.kylin-agent-runtime/agent-runtime/agent/conversation_loop.py` 第 515-522 行：

```python
        _ctx_parts: list[str] = []
        _plugin_final_response = None
        for r in _pre_results:
            if isinstance(r, dict) and r.get("final_response"):
                _plugin_final_response = str(r["final_response"])
                break
            if isinstance(r, dict) and r.get("context"):
                _ctx_parts.append(str(r["context"]))
            elif isinstance(r, str) and r.strip():
                _ctx_parts.append(r)
        if _ctx_parts:
            _plugin_user_context = "\n\n".join(_ctx_parts)
    except Exception as exc:
        logger.warning("pre_llm_call hook failed: %s", exc)

    # Main conversation loop
    api_call_count = 0
    final_response = None
    if _plugin_final_response is not None:
        final_response = _plugin_final_response
        messages.append({"role": "assistant",
                         "content": final_response})
        _turn_exit_reason = "plugin_short_circuit"
```

这样 while 循环第一行检测到 `_plugin_final_response` 非 None 时立刻 break，进入正常的收尾流程（persist、memory sync、hooks 等）。
