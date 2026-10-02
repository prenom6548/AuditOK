from agent.delegation_router import decide_delegation_hint


TOOLS = {"delegate_task", "terminal", "read_file"}


def test_proactive_router_is_disabled_by_default():
    decision = decide_delegation_hint(
        "修改会话路由逻辑，并且添加回归测试",
        valid_tool_names=TOOLS,
        config={},
    )

    assert decision.should_hint is False
    assert decision.must_delegate is False


def test_explicit_mainline_branch_request_gets_hint():
    decision = decide_delegation_hint(
        "请使用主干-分支方式，分别从 system prompt、工具 schema、会话历史三个方向分析变慢原因",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True
    assert decision.score >= 3
    assert "todo" in decision.hint
    assert "delegate_task" in decision.hint


def test_simple_single_action_does_not_get_hint():
    decision = decide_delegation_hint(
        "打开微信，搜索联系人江杰",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False


def test_router_requires_delegate_tool():
    decision = decide_delegation_hint(
        "请并行分析三个方向的日志",
        valid_tool_names={"terminal"},
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "delegate_tool_unavailable"


def test_router_respects_global_delegation_disabled():
    decision = decide_delegation_hint(
        "请使用主干-分支方式并行分析三个方向的日志",
        valid_tool_names=TOOLS,
        config={"enabled": False, "router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "disabled"


def test_router_respects_mainline_branch_disabled():
    decision = decide_delegation_hint(
        "请使用主干-分支方式并行分析三个方向的日志",
        valid_tool_names=TOOLS,
        config={"enabled": True, "mainline_branch_enabled": False, "router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "mainline_branch_disabled"


def test_router_skips_lean_child_profile():
    decision = decide_delegation_hint(
        "请并行分析三个方向的日志",
        valid_tool_names=TOOLS,
        prompt_profile="lean",
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "lean_profile"


def test_complex_implementation_task_is_enforced_by_default():
    decision = decide_delegation_hint(
        "修改会话路由逻辑，并且添加回归测试",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True
    assert "MUST call `todo`" in decision.hint


def test_enforcement_can_be_disabled_without_disabling_hint():
    decision = decide_delegation_hint(
        "请并行分析三个方向的日志",
        valid_tool_names=TOOLS,
        config={"router_enabled": True, "enforce_complex_tasks": False},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is False
    assert "Delegation router hint" in decision.hint


def test_simple_action_is_never_forced():
    decision = decide_delegation_hint(
        "读取 README.md",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.must_delegate is False


def test_opening_desktop_app_is_direct_single_action():
    decision = decide_delegation_hint(
        "打开 WPS",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "direct_single_action"


def test_explicit_wechat_message_is_direct_single_action():
    decision = decide_delegation_hint(
        "使用微信发送消息给张三：“会议改到三点”",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "direct_single_action"


def test_open_then_analyze_remains_orchestrated():
    decision = decide_delegation_hint(
        "打开日志，然后分析认证失败的根因",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True


def test_single_but_substantive_implementation_task_is_enforced():
    decision = decide_delegation_hint(
        "实现用户登录功能",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True


def test_trivial_typo_change_stays_on_mainline():
    decision = decide_delegation_hint(
        "修改 README 里的一个拼写错误",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "simple_task"


def test_uncertain_task_defaults_to_orchestration():
    decision = decide_delegation_hint(
        "把登录偶发失效的问题彻底处理掉",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True
    assert "uncertain_defaults_to_orchestration" in decision.reason


def test_short_factual_question_stays_direct():
    decision = decide_delegation_hint(
        "什么是微服务架构？",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "simple_qa"


def test_short_question_without_fixed_prefix_stays_direct():
    decision = decide_delegation_hint(
        "Python list 和 tuple 有什么区别？",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is False
    assert decision.reason == "simple_qa"


def test_uncertain_default_can_be_disabled():
    decision = decide_delegation_hint(
        "把登录偶发失效的问题彻底处理掉",
        valid_tool_names=TOOLS,
        config={
            "router_enabled": True,
            "orchestrate_uncertain_tasks": False,
        },
    )

    assert decision.should_hint is False
    assert decision.reason == "score_below_threshold"


def test_simple_action_with_complex_followup_is_orchestrated():
    decision = decide_delegation_hint(
        "打开日志，然后分析认证失败的根因",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True


def test_complex_title_parser_fix_is_not_mistaken_for_text_edit():
    decision = decide_delegation_hint(
        "修复会导致生产故障的标题解析逻辑",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True


def test_explicit_parallel_work_is_still_enforced():
    decision = decide_delegation_hint(
        "请并行处理代码审查和性能基准这两个独立任务",
        valid_tool_names=TOOLS,
        config={"router_enabled": True},
    )

    assert decision.should_hint is True
    assert decision.must_delegate is True
    assert "parallel_language" in decision.reason
