import asyncio
import hashlib
import logging
import os
import re
import uuid
from typing import Optional

import pyperclip

from src.agent.views import ActionModel, ActionResult
from src.controller.registry.service import Registry
from src.controller.views import (
	InputTextAction,
	OpenAppAction,
	PressAction,
	PressCombinedAction,
	DragAction,
	RightClickPixel,
	LeftClickPixel,
	DoubleClickPixel,
	ScrollDownAction,
	ScrollUpAction,
	MoveToAction,
	RecordAction,
	WaitAction,
)
from src.utils import time_execution_async, time_execution_sync
from src.utils.keyboard import normalize_key, split_hotkey
from src.platform_adapter import PlatformActions, open_application_by_name

logger = logging.getLogger(__name__)

class Controller:
	def __init__(
		self,
		exclude_actions: Optional[list[str]] = None,
		action_interval: float = 0.1,
		remember_clipboard_after_click: bool = False,
	):
		self.exclude_actions = list(exclude_actions or [])
		self.action_interval = max(0.0, float(action_interval))
		self.action_timeout = max(1.0, float(os.getenv("KYLIN_ACTION_TIMEOUT", "30")))
		self.remember_clipboard_after_click = remember_clipboard_after_click
		self.registry = Registry(self.exclude_actions)
		self.win = PlatformActions()
		self._important_clipboard_text: Optional[str] = None
		self._last_verified_clipboard_text: Optional[str] = None
		self._last_copy_error: Optional[str] = None
		self._clipboard_version = 0
		self._paste_target_epoch = 0
		self._completed_paste_attempts: dict[tuple[int, str, str], int] = {}
		self._register_default_actions()

	def _read_clipboard(self) -> Optional[str]:
		try:
			value = pyperclip.paste()
			return value if isinstance(value, str) else None
		except Exception as e:
			logger.debug("Failed to read clipboard: %s", e)
			return None

	def _write_clipboard(self, text: str) -> bool:
		try:
			pyperclip.copy(text)
			return True
		except Exception as e:
			logger.warning("Failed to write clipboard: %s", e)
			return False

	def _looks_like_important_clipboard_text(self, text: Optional[str]) -> bool:
		if not text:
			return False
		value = text.strip()
		if len(value) < 8:
			return False
		link_or_invite_terms = (
			"http://",
			"https://",
			"会议链接",
			"邀请链接",
			"入会链接",
			"腾讯会议",
			"会议号",
			"meeting",
			"invite",
			"join",
			"zoom.us",
			"voovmeeting",
			"wemeet",
		)
		if any(term.lower() in value.lower() for term in link_or_invite_terms):
			return True
		return bool(re.search(r"\b\d{3}[- ]?\d{3}[- ]?\d{3,4}\b", value))

	def _remember_clipboard_if_important(self, source: str = "") -> None:
		value = self._read_clipboard()
		if self._looks_like_important_clipboard_text(value):
			self._important_clipboard_text = value
			logger.info("Cached important clipboard text%s", f" from {source}" if source else "")

	def _restore_important_clipboard_if_needed(self) -> bool:
		if not self._important_clipboard_text:
			return False
		current = self._read_clipboard()
		if self._looks_like_important_clipboard_text(current):
			return False
		return self._write_clipboard(self._important_clipboard_text)

	async def _read_clipboard_async(self) -> Optional[str]:
		reader = getattr(self.win, "_read_clipboard_text", None)
		if reader is not None:
			try:
				return await reader()
			except Exception as e:
				logger.debug("Platform clipboard read failed: %s", e)
		return await asyncio.to_thread(self._read_clipboard)

	async def _write_clipboard_async(self, text: str) -> bool:
		writer = getattr(self.win, "_write_clipboard_text", None)
		if writer is not None:
			try:
				return bool(await writer(text))
			except Exception as e:
				logger.debug("Platform clipboard write failed: %s", e)
		return await asyncio.to_thread(self._write_clipboard, text)

	def _mark_paste_target_transition(self, source: str) -> None:
		"""Advance the fallback target identity after an explicit window switch."""
		self._paste_target_epoch += 1
		logger.debug(
			"Clipboard paste target epoch advanced to %d via %s",
			self._paste_target_epoch,
			source,
		)

	async def _paste_target_identity(self) -> str:
		"""Return the active window identity without depending on pasted content."""
		active_window = getattr(self.win, "_active_window_token", None)
		if active_window is not None:
			try:
				token = await active_window()
				if token:
					return f"{type(self.win).__name__}:window:{token}"
			except Exception as e:
				logger.debug("Failed to identify clipboard paste target window: %s", e)
		return f"{type(self.win).__name__}:epoch:{self._paste_target_epoch}"

	def _paste_key(self, clipboard_text: str, target_identity: str) -> tuple[int, str, str]:
		fingerprint = hashlib.sha256(clipboard_text.encode("utf-8")).hexdigest()
		return self._clipboard_version, fingerprint, target_identity

	async def _arm_clipboard_copy_probe(
		self,
	) -> tuple[Optional[str], Optional[str], Optional[ActionResult]]:
		previous = await self._read_clipboard_async()
		probe = f"__KYLIN_COPY_PROBE_{uuid.uuid4().hex}__"
		self._last_verified_clipboard_text = None
		self._last_copy_error = None

		if not await self._write_clipboard_async(probe):
			msg = "❌ Clipboard copy verification could not initialize the clipboard probe"
			self._last_copy_error = msg
			return previous, None, ActionResult(
				extracted_content=msg,
				error=msg,
				include_in_memory=True,
			)
		return previous, probe, None

	async def _finish_clipboard_copy_probe(
		self,
		previous: Optional[str],
		probe: str,
		source: str,
	) -> ActionResult:
		"""Verify a copy source replaced the unique clipboard probe."""
		timeout = max(0.1, float(os.getenv("KYLIN_COPY_VERIFY_TIMEOUT", "1.2")))
		poll_interval = max(0.02, float(os.getenv("KYLIN_COPY_VERIFY_INTERVAL", "0.1")))
		deadline = asyncio.get_running_loop().time() + timeout
		copied_text: Optional[str] = None
		while True:
			current = await self._read_clipboard_async()
			if current not in (None, "", probe):
				copied_text = current
				break
			if asyncio.get_running_loop().time() >= deadline:
				break
			await asyncio.sleep(poll_interval)

		if copied_text is None:
			await self._write_clipboard_async(previous if previous is not None else "")
			msg = (
				f"❌ Clipboard copy was not verified: {source} left the clipboard unchanged. "
				"Refocus/select the source or use its verified Copy command; do not paste stale content."
			)
			self._last_copy_error = msg
			logger.warning("Clipboard copy verification failed via %s", source)
			return ActionResult(extracted_content=msg, error=msg, include_in_memory=True)

		self._last_verified_clipboard_text = copied_text
		self._clipboard_version += 1
		# A newly verified copy is a new clipboard generation even when its text is
		# identical to the previous generation, so it may be pasted intentionally.
		self._completed_paste_attempts.clear()
		if self._looks_like_important_clipboard_text(copied_text):
			self._important_clipboard_text = copied_text
		self._last_copy_error = None
		logger.info(
			"Clipboard copy verified via %s (text_length=%d)",
			source,
			len(copied_text),
		)
		return ActionResult(
			extracted_content=(
				f"Clipboard copy verified for this action (text_length={len(copied_text)})."
			),
			include_in_memory=True,
		)

	async def _copy_selection_to_clipboard(self, source: str) -> ActionResult:
		"""Dispatch Ctrl+C and prove that this attempt produced text.

		A non-empty clipboard is not sufficient: it may contain stale text from an
		earlier task. A unique probe makes copying the same text that was already
		on the clipboard distinguishable from a no-op. On failure the previous
		clipboard value is restored so the probe never leaks into later pastes.
		"""
		previous, probe, arm_error = await self._arm_clipboard_copy_probe()
		if arm_error is not None or probe is None:
			return arm_error or ActionResult(error="Clipboard copy probe failed")

		press_successful = await self.win.press_hotkey("ctrl", "c")
		if not press_successful:
			await self._write_clipboard_async(previous if previous is not None else "")
			msg = "❌ Clipboard copy failed: Ctrl+C could not be dispatched"
			self._last_copy_error = msg
			return ActionResult(extracted_content=msg, error=msg, include_in_memory=True)
		return await self._finish_clipboard_copy_probe(previous, probe, source)

	async def _paste_from_clipboard(self, source: str) -> ActionResult:
		if self._last_copy_error:
			msg = (
				"❌ Clipboard paste blocked because the latest Ctrl+C copy was not verified. "
				"Use a different copy path before pasting."
			)
			return ActionResult(extracted_content=msg, error=msg, include_in_memory=True)

		expected_text = self._last_verified_clipboard_text or self._important_clipboard_text
		if expected_text:
			current = await self._read_clipboard_async()
			if current != expected_text:
				await self._write_clipboard_async(expected_text)
		clipboard_text = await self._read_clipboard_async()
		if clipboard_text is None or clipboard_text == "":
			msg = "❌ Clipboard paste failed: clipboard is empty or unavailable"
			return ActionResult(extracted_content=msg, error=msg)

		target_identity = await self._paste_target_identity()
		paste_key = self._paste_key(clipboard_text, target_identity)
		try:
			max_attempts = max(
				1,
				int(os.getenv("KYLIN_MAX_SAME_TARGET_PASTE_ATTEMPTS", "2")),
			)
		except ValueError:
			max_attempts = 2
		paste_attempts = self._completed_paste_attempts.get(paste_key, 0)
		if paste_attempts >= max_attempts:
			msg = (
				"❌ Duplicate clipboard paste blocked: this clipboard generation was already "
				"pasted into the active target window and one recovery retry was already "
				"allowed. Do not press Ctrl+V again while the visual result is uncertain; "
				"use another input strategy such as a context-menu paste, a different focus "
				"target, or a newly verified copy."
			)
			logger.warning(
				"Duplicate clipboard paste blocked (version=%d, fingerprint=%s, target=%s)",
				paste_key[0],
				paste_key[1][:12],
				target_identity,
			)
			return ActionResult(extracted_content=msg, error=msg, include_in_memory=True)

		press_successful = False
		type_text = getattr(self.win, "type_text", None)
		if type_text is not None:
			try:
				press_successful = await type_text(
					clipboard_text,
					press_enter=False,
					select_all=False,
				)
			except Exception as e:
				logger.debug("Clipboard paste via guarded text path failed: %s", e)
				press_successful = False
		if not press_successful:
			press_successful = await self.win.press_hotkey("ctrl", "v")
		if press_successful:
			self._completed_paste_attempts[paste_key] = paste_attempts + 1
			await asyncio.sleep(max(0.2, float(os.getenv("KYLIN_PASTE_SETTLE_DELAY", "0.6"))))
			logging.info(
				"✅ dispatched clipboard paste via %s (attempt %d/%d)",
				source,
				paste_attempts + 1,
				max_attempts,
			)
			retry_note = (
				" This was the final allowed recovery retry for this clipboard and target."
				if paste_attempts + 1 >= max_attempts
				else ""
			)
			return ActionResult(
				extracted_content=(
					"Clipboard paste was dispatched through the guarded text path; "
					"verify the target field contains the pasted text in the next screenshot."
					f"{retry_note}"
				),
				include_in_memory=True,
			)
		msg = "❌ Clipboard paste hotkey failed"
		return ActionResult(extracted_content=msg, error=msg)

	def _register_default_actions(self):
		"""Register all default desktop UI actions"""

		@self.registry.action(
				'Complete task',
				param_model=NoParamsAction)
		async def done():
			return ActionResult(extracted_content='done', is_done=True)
		@self.registry.action(
				'Type',
				param_model=InputTextAction,
				action_name='input_text')
		async def input_text(text: str, press_enter: bool = False, select_all: bool = True):
			try:
				original_clipboard = self._read_clipboard()
				if self._looks_like_important_clipboard_text(original_clipboard):
					self._important_clipboard_text = original_clipboard
				input_successful = await self.win.type_text(text, press_enter=press_enter, select_all=select_all)
				if input_successful:
					if self._looks_like_important_clipboard_text(original_clipboard):
						self._write_clipboard(original_clipboard)
					enter_note = " Enter was also pressed." if press_enter else ""
					mode_note = "" if select_all else " Existing content was not cleared (insert/append mode)."
					return ActionResult(
						extracted_content=(
							'Text paste dispatched successfully; verify the target '
							'field retains the intended value in the next screenshot.'
							f'{enter_note}{mode_note}'
						),
						include_in_memory=True,
					)
				else:
					detail = getattr(self.win, "last_input_error", None)
					msg = f'❌ Input failed{f": {detail}" if detail else ""}'
					return ActionResult(extracted_content=msg, error=msg)
			except Exception as e:
				msg = f'❌ An error occurred: {str(e)}'
				logging.error(msg)
				return ActionResult(extracted_content=msg, error=msg)


		@self.registry.action(
			"Open an installed desktop app by localized name or desktop ID. Unknown or ambiguous names return candidates without launching.",
			param_model=OpenAppAction,
			action_name="open_app",
		)
		async def open_app(app_name: str):
			"""
			Attempt to open an app by name.
			"""

			user_input = app_name
			if user_input.lower() == 'wechat':
				user_input = '微信'
			success, detail = await open_application_by_name(user_input)
			logger.info(f"\nLaunching app: {user_input}...")
			try:
				from src.utils.status_window import bring_status_window_to_front
				async def bring_status_window_later(delay: float):
					await asyncio.sleep(delay)
					bring_status_window_to_front()

				bring_status_window_to_front()
				await asyncio.sleep(max(0.0, float(os.getenv("KYLIN_OPEN_APP_STATUS_SETTLE", "0.2"))))
				bring_status_window_to_front()
				asyncio.create_task(bring_status_window_later(1.5))
				asyncio.create_task(bring_status_window_later(2.5))
				asyncio.create_task(bring_status_window_later(4.0))
			except Exception:
				logger.debug("Failed to bring status window to front after open_app", exc_info=True)
			if not success:
				msg = f"❌ Failed to launch '{user_input}': {detail}"
				logger.error(msg)
				return ActionResult(extracted_content=msg, error=msg)

			self._mark_paste_target_transition("open_app")
			pid = None

			success_msg = f"✅ Launched {user_input}. detail={detail}"
			logger.info(success_msg)
			return ActionResult(extracted_content=success_msg, current_app_pid=pid)
		
		@self.registry.action(
			'Single Hotkey',
			param_model=PressAction,
		)
		async def Hotkey(key: str = "enter"):
			keys = split_hotkey(key)
			if not keys:
				keys = ["enter"]

			# Route clipboard shortcuts through the verified copy/paste paths.
			if len(keys) == 2 and keys[0] == "ctrl" and keys[1] == "v":
				return await self._paste_from_clipboard("Hotkey")
			if len(keys) == 2 and keys[0] == "ctrl" and keys[1] == "c":
				return await self._copy_selection_to_clipboard("Hotkey")

			if len(keys) >= 3:
				press_successful = await self.win.press_hotkey(keys[0], keys[1], keys[2])
			elif len(keys) == 2:
				press_successful = await self.win.press_hotkey(keys[0], keys[1])
			else:
				press_successful = await self.win.press_key(keys[0])

			if press_successful:
				logging.info(f'✅ pressed key code: {key}')
				return ActionResult(extracted_content=f'Successfully press keyboard with key code {key}')
			msg = f'❌ Failed to press keyboard with key code {key}'
			return ActionResult(extracted_content=msg, error=msg)
			
		@self.registry.action(
			'Press Multiple Hotkey',
			param_model=PressCombinedAction,
		)
		async def multi_Hotkey(key1: str, key2: str, key3: Optional[str] = None):
			key1 = normalize_key(key1)
			key2 = normalize_key(key2)
			key3 = normalize_key(key3) if key3 else None

			if key3 is not None:
				press_successful = await self.win.press_hotkey(key1, key2, key3)
				if press_successful:
					logging.info(f'✅ pressed combination key: {key1}, {key2} and {key3}')
					return ActionResult(extracted_content=f'Successfully press keyboard with key code {key1}, {key2} and {key3}')
				msg = f'❌ Failed to press keyboard with key code {key1}, {key2} and {key3}'
				return ActionResult(extracted_content=msg, error=msg)

			if {key1, key2} == {"ctrl", "v"}:
				return await self._paste_from_clipboard("multi_Hotkey")
			if {key1, key2} == {"ctrl", "c"}:
				return await self._copy_selection_to_clipboard("multi_Hotkey")

			press_successful = await self.win.press_hotkey(key1, key2)
			if press_successful:
				if {key1, key2} == {"alt", "tab"}:
					self._mark_paste_target_transition("Alt+Tab")
				logging.info(f'✅ pressed combination key: {key1} and {key2}')
				return ActionResult(extracted_content=f'Successfully press keyboard with key code {key1} and {key2}')
			msg = f'❌ Failed to press keyboard with key code {key1} and {key2}'
			return ActionResult(extracted_content=msg, error=msg)

		@self.registry.action(
			'RightSingle click at specific pixel',
			param_model=RightClickPixel,
		)
		async def RightSingle(position: list):
			try:
				x, y = position
				logger.debug(f'Correct right clicking target position={position}')
				click_successful = await self.win.click(x, y, button='right')
				if click_successful:
					logging.info(f'✅ Finished right click at pixel: {position}')
					return ActionResult(extracted_content=f'Successfully clicked pixel {position}')
				else:
					msg = f'❌ Right click failed for pixel with position: {position}'
					return ActionResult(extracted_content=msg, error=msg)
			except Exception as e:
				msg = f'❌ An error occurred: {str(e)}'
				logging.error(msg)
				return ActionResult(extracted_content=msg, error=msg)
			
		@self.registry.action(
			'Left click at specific pixel',
			param_model=LeftClickPixel,
		)
		async def Click(position: list, expect_clipboard_change: bool = False):
			try:
				x, y = position
				logger.debug(f'Correct left clicking target position={position}')
				previous: Optional[str] = None
				probe: Optional[str] = None
				if expect_clipboard_change:
					previous, probe, arm_error = await self._arm_clipboard_copy_probe()
					if arm_error is not None or probe is None:
						return arm_error or ActionResult(error="Clipboard copy probe failed")
				click_successful = await self.win.click(x, y, button='left')
				if click_successful:
					if expect_clipboard_change and probe is not None:
						return await self._finish_clipboard_copy_probe(
							previous,
							probe,
							"Click(expect_clipboard_change=true)",
						)
					if self.remember_clipboard_after_click:
						await asyncio.sleep(0.2)
						self._remember_clipboard_if_important("click")
					logging.info(f'✅ Finished left click at pixel: {position}')
					return ActionResult(extracted_content=f'Successfully clicked pixel {position}')
				else:
					if expect_clipboard_change and probe is not None:
						await self._write_clipboard_async(
							previous if previous is not None else ""
						)
					msg = f'❌ Left click failed for pixel with position: {position}'
					if expect_clipboard_change:
						self._last_copy_error = msg
					return ActionResult(extracted_content=msg, error=msg)
			except Exception as e:
				msg = f'❌ An error occurred: {str(e)}'
				logging.error(msg)
				return ActionResult(extracted_content=msg, error=msg)

		@self.registry.action(
			'Double click to open desktop application icons or files. Use this action when you need to launch an app from desktop or open a file.',
			param_model=DoubleClickPixel,
		)
		async def DoubleClick(position: list):
			try:
				x, y = position
				logger.debug(f'Correct double clicking target position={position}')
				click_successful = await self.win.click(x, y, button='double')
				if click_successful:
					await asyncio.sleep(0.5)
					try:
						from src.utils.status_window import bring_status_window_to_front
						async def bring_status_window_later(delay: float):
							await asyncio.sleep(delay)
							bring_status_window_to_front()

						bring_status_window_to_front()
						asyncio.create_task(bring_status_window_later(1.0))
						asyncio.create_task(bring_status_window_later(2.0))
						asyncio.create_task(bring_status_window_later(3.0))
						asyncio.create_task(bring_status_window_later(4.0))
					except Exception:
						logger.debug("Failed to bring status window to front after double click", exc_info=True)
					logging.info(f'✅ Finished double click at pixel: {position}')
					return ActionResult(extracted_content=f'Successfully double clicked pixel {position}')
				else:
					msg = f'❌ Double click failed for pixel with position: {position}'
					return ActionResult(extracted_content=msg, error=msg)
			except Exception as e:
				msg = f'❌ An error occurred: {str(e)}'
				logging.error(msg)
				return ActionResult(extracted_content=msg, error=msg)
			
		@self.registry.action(
			'Drag an object from one pixel to another',
			param_model=DragAction,
		)
		async def Drag(position1: list = [0,0], position2: list = [0,0]):
			try:
				x1, y1 = position1
				x2, y2 = position2
				drag_successful = await self.win.drag(x1, y1, x2, y2)
				if drag_successful:
					logger.info(f'Correct draging pixel from position {position1} to {position2}')
					return ActionResult(extracted_content=f'Successfully drag pixel {position1} to {position2}')
				else:
					msg = f'❌ Drag failed for pixel with position: {position1}'
					return ActionResult(extracted_content=msg, error=msg)
			except Exception as e:
				msg = f'❌ An error occurred: {str(e)}'
				logging.error(msg)
				return ActionResult(extracted_content=msg, error=msg)
			
		@self.registry.action(
				'Move mouse to specific pixel',
				param_model=MoveToAction,
				action_name='move_mouse')
		async def move_mouse(position: list = [0,0]):
			logger.debug(f'Correct move mouse to position {position}')
			try:
				x, y = position
				move_successful = await self.win.move_mouse(x, y)
				if move_successful:
					logging.info(f'✅ Finished move mouse to pixel: {position}')
					return ActionResult(extracted_content=f'Successfully move mouse to {position}')
				else:
					msg = f'❌ Failed move mouse to pixel with position: {position}'
					return ActionResult(extracted_content=msg, error=msg)
			except Exception as e:
				msg = f'❌ An error occurred: {str(e)}'
				logging.error(msg)
				return ActionResult(extracted_content=msg, error=msg)
		
		@self.registry.action(
			'Scroll up',
			param_model=ScrollUpAction,
			action_name='scroll_up')
		async def scroll_up(position, dx: int = 0, dy: int = 2):
			x,y = position
			amount = dy
			scroll_successful = await self.win.scroll(x, y, amount)
			if scroll_successful:
				logging.info(f'✅ Scrolled up by {amount}')
				return ActionResult(extracted_content=f'Successfully scrolled up by {amount}')

		@self.registry.action(
			'Scroll down',
			param_model=ScrollDownAction,
			action_name='scroll_down')
		async def scroll_down(position, dx: int = 0, dy: int = 2):
			x,y = position
			amount = dy
			scroll_successful = await self.win.scroll(x, y, -amount)
			if scroll_successful:
				logging.info(f'✅ Scrolled down by {amount}')
				return ActionResult(extracted_content=f'Successfully scrolled down by {amount}')
			
		@self.registry.action(
			'Tell the short memory that you are recording information',
			param_model=RecordAction,
		)
		async def record_info(text: str, file_name: str):
			return ActionResult(extracted_content=f'{file_name}: {text}')
		
		@self.registry.action(
			'Wait for the UI to settle',
			param_model=WaitAction,
			action_name='wait',
		)
		async def wait(seconds: Optional[float] = None):
			duration = seconds
			if duration is None:
				duration = float(os.getenv("KYLIN_WAIT_SECONDS", "1.0"))
			duration = max(0.0, min(1.0, float(duration)))
			if duration > 0:
				await asyncio.sleep(duration)
			return ActionResult(extracted_content=f'Waited {duration:.1f}s for UI to settle')

	def action(self, description: str, **kwargs):
		"""Decorator for registering custom actions

		@param description: Describe the LLM what the function does (better description == better function calling)
		"""
		return self.registry.action(description, **kwargs)

	@time_execution_async('--multi-act')
	async def multi_act(
		self, actions: list[ActionModel], ui_tree_builder=None
	) -> list[ActionResult]:
		"""Execute multiple actions"""
		results = []
		for i, action in enumerate(actions):
			try:
				result = await asyncio.wait_for(self.act(action, ui_tree_builder), timeout=self.action_timeout)
			except asyncio.TimeoutError:
				action_text = action.model_dump(exclude_unset=True)
				msg = f"Action timed out after {self.action_timeout:.1f}s: {action_text}"
				logger.error(msg)
				result = ActionResult(extracted_content=msg, error=msg)
			results.append(result)
			logger.debug(f'Executed action {i + 1} / {len(actions)}')
			if results[-1].is_done or results[-1].error or i == len(actions) - 1:
				break
			if self.action_interval > 0:
				await asyncio.sleep(self.action_interval)

		return results

	@time_execution_sync('--act')
	async def act(self, action: ActionModel, ui_tree_builder=None) -> ActionResult:
		"""Execute an action"""
		try:
			for action_name, params in action.model_dump(exclude_unset=True).items():
				if params is not None:
					result = await self.registry.execute_action(action_name, params, ui_tree_builder=ui_tree_builder)
					if isinstance(result, str):
						return ActionResult(extracted_content=result)
					elif isinstance(result, ActionResult):
						return result
					elif result is None:
						return ActionResult()
					else:
						raise ValueError(f'Invalid action result type: {type(result)} of {result}')
			return ActionResult()
		except Exception as e:
			msg = f'Error executing action: {str(e)}'
			logger.error(msg)
			return ActionResult(extracted_content=msg, error=msg)

class NoParamsAction(ActionModel):
	"""
	Simple parameter model requiring no arguments.
	"""
	pass
