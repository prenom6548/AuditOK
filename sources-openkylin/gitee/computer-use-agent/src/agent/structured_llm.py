from __future__ import annotations
from typing import List, Optional, Dict, Any
from pydantic import BaseModel, Field, ConfigDict, field_validator, model_validator
from src.controller.views import *

# ---------------------------------------------------------------------------
# DISCRIMINATED UNION FOR A SINGLE ACTION ITEM
# ---------------------------------------------------------------------------

class ActionItem(BaseModel):
    """Exactly one of the fields must be populated to specify the concrete action."""
    model_config = ConfigDict(exclude_none=True, populate_by_name=True)
    done: Optional[NoParamsAction] = None
    input_text: Optional[InputTextAction] = None
    open_app: Optional[OpenAppAction] = None
    run_apple_script: Optional[AppleScriptAction] = None
    Hotkey: Optional[PressAction] = None
    multi_Hotkey: Optional[PressCombinedAction] = None
    RightSingle: Optional[RightClickPixel] = None
    Click: Optional[LeftClickPixel] = None
    DoubleClick: Optional[DoubleClickPixel] = None
    Drag: Optional[DragAction] = None
    move_mouse: Optional[MoveToAction] = None
    scroll_up: Optional[ScrollUpAction] = None
    scroll_down: Optional[ScrollDownAction] = None
    record_info: Optional[RecordAction] = None
    wait: Optional[WaitAction] = None

    def __repr__(self) -> str:
        non_none = self.model_dump(exclude_none=True)
        field_strs = ", ".join(f"{k}={v!r}" for k, v in non_none.items())
        return f"{self.__class__.__name__}({field_strs})"

    @field_validator("wait", mode="before")
    def fix_empty_string(cls, v):
        if v == "" or v is None:
            return {}
        if not isinstance(v, dict):
            return {}
        return v

    @model_validator(mode="after")
    def validate_single_action(self) -> "ActionItem":
        populated = [
            name
            for name, value in self.model_dump(exclude_none=True).items()
            if value is not None
        ]
        if len(populated) != 1:
            raise ValueError("Each action item must contain exactly one concrete action.")
        return self

# ---------------------------------------------------------------------------
# CURRENT-STATE SUB-MODEL
# ---------------------------------------------------------------------------

class Analysis(BaseModel):
    analysis: str = Field(..., description="用中文简要分析当前状态与预期状态是否一致；保留必要的英文协议/界面原文。")
    sop_check: str = Field(..., description="用中文说明当前适用的 Selected Skill 步骤；没有适用技能时写 'None'。")

class CurrentState(BaseModel):
    step_evaluate: str = Field(..., description="Success/Failed (based on step completion)")
    ask_human: str = Field(
        ...,
        description=(
            "如果需要用户确认，用中文说明需要用户做什么；如果不需要确认，写 No。"
        ),
    )
    next_goal: str = Field(
        ..., description="用中文生成基于当前状态和技能的可执行目标；字段名、动作名等协议文本保持英文。"
    )

class ReadFilesRequest(BaseModel):
    files: List[str] = Field(..., description="Recorded info filenames to read.")

# ---------------------------------------------------------------------------
# AGENT OUTPUT MODELS
# ---------------------------------------------------------------------------

class MemoryOutput(BaseModel):
    summary: str = Field(..., description="用于后续步骤记忆的中文简要总结。")
    file_name: str = Field(..., description="Filename to store the pre-summarization context.")

    def __repr__(self) -> str:
        non_none = self.model_dump(exclude_none=True)
        field_strs = ", ".join(f"{k}={v!r}" for k, v in non_none.items())
        return f"{self.__class__.__name__}({field_strs})"

    @property
    def content(self) -> str:
        return self.model_dump_json(exclude_none=True, exclude_unset=True)

    @property
    def parsed(self) -> Dict[str, Any]:
        return self.model_dump(exclude_none=True, exclude_unset=True)

class BrainOutput(BaseModel):
    analysis: Optional[Analysis] = None
    current_state: Optional[CurrentState] = None
    read_files: Optional[ReadFilesRequest] = None

    @model_validator(mode="after")
    def validate_output(self) -> "BrainOutput":
        if self.read_files:
            if self.analysis or self.current_state:
                raise ValueError("Read-files output must not include analysis or current_state.")
        else:
            if not (self.analysis and self.current_state):
                raise ValueError("analysis and current_state are required when read_files is not requested.")
        return self

    def __repr__(self) -> str:
        non_none = self.model_dump(exclude_none=True)
        field_strs = ", ".join(f"{k}={v!r}" for k, v in non_none.items())
        return f"{self.__class__.__name__}({field_strs})"

    @property
    def content(self) -> str:
        return self.model_dump_json(exclude_none=True, exclude_unset=True)

    @property
    def parsed(self) -> Dict[str, Any]:
        return self.model_dump(exclude_none=True, exclude_unset=True)

class ActorOutput(BaseModel):
    action: List[ActionItem] = Field(
        ...,
        min_items=0,
        max_items=10,
        description="Ordered list of 0-10 actions for this step.",
    )

    def __repr__(self) -> str:
        non_none = self.model_dump(exclude_none=True)
        field_strs = ", ".join(f"{k}={v!r}" for k, v in non_none.items())
        return f"{self.__class__.__name__}({field_strs})"

    @property
    def content(self) -> str:
        return self.model_dump_json(exclude_none=True, exclude_unset=True)

    @property
    def parsed(self) -> Dict[str, Any]:
        return self.model_dump(exclude_none=True, exclude_unset=True)

class CombinedOutput(BaseModel):
    analysis: Optional[Analysis] = None
    current_state: Optional[CurrentState] = None
    action: Optional[List[ActionItem]] = None
    read_files: Optional[ReadFilesRequest] = None

    @model_validator(mode="after")
    def validate_output(self) -> "CombinedOutput":
        if self.read_files:
            if self.analysis or self.current_state or self.action:
                raise ValueError("Read-files output must not include analysis/current_state/action.")
        else:
            if not (self.analysis and self.current_state and self.action is not None):
                raise ValueError("analysis, current_state, and action are required when read_files is not requested.")
        return self

    @property
    def content(self) -> str:
        return self.model_dump_json(exclude_none=True, exclude_unset=True)

    @property
    def parsed(self) -> Dict[str, Any]:
        return self.model_dump(exclude_none=True, exclude_unset=True)

class Step(BaseModel):
    step_id: str = Field(..., pattern=r"^Step \d+$")
    description: Optional[str] = Field(default=None, description="中文高层步骤目标；不要写低层点击/按键细节。")
    important_search_info: str = Field(..., description="该步骤相关搜索信息的中文摘要；没有则为空字符串。")

class IterationInfo(BaseModel):
    current_iteration: int = Field(..., ge=1)
    total_iterations: int = Field(..., ge=1)

class PlannerOutput(BaseModel):
    iteration_info: IterationInfo
    search_summary: str = Field(..., description="最相关搜索发现的中文简要总结。")
    selected_skills: List[str] = Field(
        default_factory=list,
        description="Planner-selected skills by name. Empty list if none apply."
    )
    natural_language_plan: Optional[str] = Field(
        default=None,
        description="中文高层计划，不包含 step ID。"
    )
    step_by_step_plan: List[Step] = Field(...,
        min_items=1,
        description="Ordered high-level plan objects, each must start with 'Step N'.",
    )

    @property
    def content(self) -> str:
        lines = []
        if self.search_summary:
            lines.append(f"Search summary: {self.search_summary}")
        if self.selected_skills:
            lines.append(f"Selected skills: {', '.join(self.selected_skills)}")
        if self.natural_language_plan:
            lines.append("Plan:")
            lines.append(self.natural_language_plan)
        for step in self.step_by_step_plan:
            info = step.important_search_info or ""
            lines.append(f"{step.step_id}: {step.description} (search: {info})")
        return "\n".join(lines)

__all__ = [
    "BrainOutput",
    "ActorOutput",
    "CombinedOutput",
    "PlannerOutput",
    "MemoryOutput",
]
