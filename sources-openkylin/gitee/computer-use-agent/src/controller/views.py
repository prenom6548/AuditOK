from typing import Literal, Optional

from typing import List
from pydantic import BaseModel, Field, field_validator, model_validator
from src.controller.registry.views import ActionModel
# mlx-use actions 


class DoneAction(BaseModel):
	text: str

class InputTextAction(BaseModel):
	text: str
	press_enter: bool = Field(
		default=False,
		description="Press Enter after the text is pasted. Use only for explicit send/search/submit/confirm goals.",
	)
	select_all: bool = Field(
		default=True,
		description=(
			"Select all existing text (Ctrl+A) before pasting so the field is "
			"replaced. Set false to insert/append at the current cursor position "
			"without clearing existing content (e.g., Excel cells, document editing)."
		),
	)

class RecordAction(BaseModel):
	text: str = Field(..., description="information you need to record")
	file_name: str = Field(..., description="filename summary for the recorded info")

class OpenAppAction(BaseModel):
	app_name: str

class AppleScriptAction(BaseModel):
	script: str

class PressAction(BaseModel):
	key: str

class PressCombinedAction(BaseModel):
	key1: str
	key2: str
	key3: Optional[str] = None

class LeftClickPositionAction(BaseModel):
	index: int

class RightClickPositionAction(BaseModel):
	index: int

class MoveToAction(BaseModel):
	position: List[float] = Field(..., description="Coordinates (normalised) [x,y]")

class ClickTarget(BaseModel):
	position: List[float] = Field(
		description="Coordinates [x,y], each normalized to 0..1000 relative to the current screenshot (not pixels or 0..1).",
	)

	@model_validator(mode="after")
	def require_position(self):
		if len(self.position) != 2:
			raise ValueError("position must contain exactly two numbers: [x,y]")
		if any(not 0 <= value <= 1000 for value in self.position):
			raise ValueError("position coordinates must be finite numbers in 0..1000")
		return self


class LeftClickPixel(ClickTarget):
	expect_clipboard_change: bool = Field(
		default=False,
		description=(
			"Set true only when clicking a visible Copy/复制 command. "
			"The controller will verify that this click replaced the clipboard."
		),
	)

class RightClickPixel(ClickTarget):
	pass

class DoubleClickPixel(ClickTarget):
	pass

class ScrollUpAction(BaseModel):
	position: List[float] = Field(..., description="Coordinates (normalised) [x,y] inside the target scrollable area; move_mouse must target this same area before scrolling")
	dx: Optional[int] = Field(default=0, description="Amount to scroll horizontally. Default=0")
	dy: Optional[int] = Field(default=3, description="Amount to scroll up. Use 1-2 for dropdowns/compact selectors, 3-6 for full pages or long lists")

class ScrollDownAction(BaseModel):
	position: List[float] = Field(..., description="Coordinates (normalised) [x,y] inside the target scrollable area; move_mouse must target this same area before scrolling")
	dx: Optional[int] = Field(default=0, description="Amount to scroll horizontally. Default=0")
	dy: Optional[int] = Field(default=3, description="Amount to scroll down. Use 1-2 for dropdowns/compact selectors, 3-6 for full pages or long lists")

class ExtractAction(BaseModel):
	position1: List[float] = Field(..., description="Coordinates (normalised) [x,y]")

class DragAction(BaseModel):
	position1: List[float] = Field(..., description="Starting Coordinates (normalised) [x,y]")
	position2: List[float] = Field(..., description="Ending Coordinates (normalised) [x,y]")

class NoParamsAction(ActionModel):
	"""
	Simple parameter model requiring no arguments.
	"""
	pass

class WaitAction(ActionModel):
	"""
	Wait for the UI to settle. Keep waits short so the agent can re-inspect quickly.
	"""
	seconds: Optional[float] = Field(default=None, ge=0, le=1)

	@field_validator("seconds", mode="before")
	@classmethod
	def clamp_seconds(cls, value):
		if value is None:
			return None
		try:
			return min(1.0, float(value))
		except (TypeError, ValueError):
			return None
