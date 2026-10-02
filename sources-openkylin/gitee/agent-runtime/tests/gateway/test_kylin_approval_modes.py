"""Desktop approval selection is scoped to one active chat session."""

import json
from unittest.mock import Mock, patch

from gateway.platforms.kylin_chat_approval import ChatApprovals, require_tool_approval
from tools import approval


def _chat(mode, session_id):
    return ChatApprovals(Mock(), Mock(interrupt=Mock()), session_id, mode=mode)


def test_full_access_is_removed_at_end_of_turn():
    chat = _chat('full', 'approval-mode-full-test')
    chat.start()
    try:
        assert approval.is_session_yolo_enabled(chat.key)
    finally:
        chat.cleanup()
    assert not approval.is_session_yolo_enabled(chat.key)


def test_desktop_mode_overrides_global_config_for_one_turn():
    chat = _chat('assist', 'approval-mode-assist-test')
    with patch.object(approval, '_get_approval_config', return_value={'mode': 'off'}):
        assert approval._get_approval_mode() == 'off'
        chat.start()
        try:
            assert approval._get_approval_mode() == 'manual'
        finally:
            chat.cleanup()
        assert approval._get_approval_mode() == 'off'


def test_request_mode_prompts_for_active_tool_only():
    chat = _chat('request', 'approval-mode-request-test')
    seen = []

    def approve(data):
        seen.append(data)
        with approval._lock:
            entry = approval._gateway_queues[chat.key][-1]
        entry.result = 'once'
        entry.event.set()

    chat.notify = approve
    chat.start()
    try:
        assert require_tool_approval(chat.session_id, 'read_file', {'path': '/tmp/a'}) is None
        assert seen == []
        assert require_tool_approval(chat.session_id, 'write_file', {'path': '/tmp/a'}) is None
        assert len(seen) == 1
        assert seen[0]['approval_scope'] == 'each'
        assert require_tool_approval('other-session', 'write_file', {'path': '/tmp/a'}) is None
    finally:
        chat.cleanup()


def test_request_mode_blocks_a_denied_tool():
    chat = _chat('request', 'approval-mode-deny-test')

    def deny(data):
        with approval._lock:
            entry = approval._gateway_queues[chat.key][-1]
        entry.result = 'deny'
        entry.event.set()

    chat.notify = deny
    chat.start()
    try:
        assert require_tool_approval(chat.session_id, 'web_search', {'query': 'x'}).startswith('BLOCKED:')
    finally:
        chat.cleanup()


def test_dispatch_does_not_run_tool_after_denied_approval():
    from model_tools import handle_function_call

    with patch('gateway.platforms.kylin_chat_approval.require_tool_approval',
               return_value='BLOCKED: denied'):
        result = handle_function_call(
            'write_file', {'path': '/tmp/approval-mode-test', 'content': 'secret'},
            session_id='approval-mode-dispatch-test',
        )
    assert json.loads(result)['error'] == 'BLOCKED: denied'
