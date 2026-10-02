import asyncio
import logging
import os
import time
from turtle import pos
from typing import Optional, Dict, Any, Tuple
import pyautogui
import win32gui
import win32con
import win32api
import win32clipboard
import pyperclip

from src.utils.keyboard import normalize_key, pyautogui_key_name


logger = logging.getLogger(__name__)


class WindowsActions:
    """Windows-specific UI actions using Windows API and PyAutoGUI"""
    
    def __init__(self):
        self._last_click_window: Optional[int] = None
        self._last_click_time: Optional[float] = None
        self._last_click_position: Optional[Tuple[float, float]] = None
        self.last_input_error: Optional[str] = None

    def _record_click_focus(self) -> None:
        self._last_click_time = time.monotonic()
        try:
            self._last_click_window = int(win32gui.GetForegroundWindow())
        except Exception:
            self._last_click_window = None

    def _validate_text_focus(self) -> bool:
        try:
            max_age = max(
                1.0,
                float(os.getenv("KYLIN_TEXT_FOCUS_MAX_AGE", "180")),
            )
        except ValueError:
            max_age = 180.0
        if self._last_click_time is None:
            self.last_input_error = (
                "Text input blocked: no recent framework-recorded focus click."
            )
            return False
        if time.monotonic() - self._last_click_time > max_age:
            self.last_input_error = (
                "Text input blocked: the focus click is stale; click the field again."
            )
            return False
        try:
            active_window = int(win32gui.GetForegroundWindow())
        except Exception:
            active_window = None
        if (
            self._last_click_window is not None
            and active_window is not None
            and active_window != self._last_click_window
        ):
            self.last_input_error = (
                "Text input blocked: focus moved to a different window after "
                "the target field was clicked."
            )
            return False
        return True

    def _restore_text_focus(self) -> bool:
        """Return focus to the last framework-clicked field before pasting."""
        if self._validate_text_focus():
            return True

        previous_error = self.last_input_error
        if not self._last_click_position:
            return False

        try:
            if self._last_click_window:
                try:
                    win32gui.ShowWindow(self._last_click_window, win32con.SW_RESTORE)
                    win32gui.SetForegroundWindow(self._last_click_window)
                    time.sleep(0.05)
                except Exception as e:
                    logger.debug("Failed to reactivate text target window: %s", e)
            x, y = self._last_click_position
            pyautogui.click(x, y)
            time.sleep(0.05)
            self._record_click_focus()
            return self._validate_text_focus()
        except Exception as e:
            logger.debug("Failed to restore text focus: %s", e)
            self.last_input_error = previous_error
            return False

    def getscreen_size(self) -> Tuple[int, int]:
        """Get the size of the primary screen"""
        try:
            screen_width, screen_height = pyautogui.size()
            return screen_width, screen_height
        except Exception as e:
            logger.error(f"Error getting screen size: {e}")
            return 1920, 1080
        
    async def click(self, x: float, y: float, button: str = 'left') -> bool:
        """Click at specified coordinates"""
        if (0 <= x <= 1 and 0 <= y <= 1):
            # If the coordinates are normalized (0 to 1)
            pos_x = self.getscreen_size()[0] * x
            pos_y = self.getscreen_size()[1] * y
        else:
            # If the coordinates are normalized (0 to 1000)
            pos_x = self.getscreen_size()[0] * (x / 1000)
            pos_y = self.getscreen_size()[1] * (y / 1000)
        try:
            if button == 'left':
                pyautogui.click(pos_x, pos_y)
            elif button == 'right':
                pyautogui.rightClick(pos_x, pos_y)
            elif button == 'double':
                pyautogui.doubleClick(pos_x, pos_y)

            await asyncio.sleep(0.1)  # Small delay after click
            if button in {"left", "double"}:
                self._last_click_position = (pos_x, pos_y)
            self._record_click_focus()
            return True
        except Exception as e:
            logger.error(f"Error clicking at ({x}, {y}): {e}")
            return False

    async def drag(self, start_x: float, start_y: float, end_x: float, end_y: float, duration: float = 0.5) -> bool:
        """Drag from start to end position"""
        if (0<= start_x <= 1 and 0 <= start_y <= 1 and 0 <= end_x <= 1 and 0 <= end_y <= 1):
            # If the coordinates are normalized (0 to 1)
            pos_start_x = self.getscreen_size()[0] * start_x
            pos_start_y = self.getscreen_size()[1] * start_y
            pos_end_x = self.getscreen_size()[0] * end_x
            pos_end_y = self.getscreen_size()[1] * end_y
        else:
            # If the coordinates are normalized (0 to 1000)
            pos_start_x = self.getscreen_size()[0] * (start_x / 1000)
            pos_start_y = self.getscreen_size()[1] * (start_y / 1000)
            pos_end_x = self.getscreen_size()[0] * (end_x / 1000)
            pos_end_y = self.getscreen_size()[1] * (end_y / 1000)
        try:
            pyautogui.drag(pos_end_x - pos_start_x, pos_end_y - pos_start_y, duration=duration, button='left')
            return True
        except Exception as e:
            logger.error(f"Error dragging from ({start_x}, {start_y}) to ({end_x}, {end_y}): {e}")
            return False

    async def scroll(self, x: float, y: float, clicks: int) -> bool:
        """Scroll at specified position"""
        if (0 <= x <= 1 and 0 <= y <= 1):
            # If the coordinates are normalized (0 to 1)
            pos_x = self.getscreen_size()[0] * x
            pos_y = self.getscreen_size()[1] * y
        else:
            # If the coordinates are normalized (0 to 1000)
            pos_x = self.getscreen_size()[0] * (x / 1000)
            pos_y = self.getscreen_size()[1] * (y / 1000)
        try:
            pyautogui.scroll(clicks, x=pos_x, y=pos_y)
            return True
        except Exception as e:
            logger.error(f"Error scrolling at ({x}, {y}): {e}")
            return False
    
    async def type_text(self, text: str, press_enter: bool = False, select_all: bool = True) -> bool:
        """
        Type text using clipboard method to handle both English and Chinese characters
        This method bypasses input method editors (IMEs) and works reliably, with robust clipboard restoration to avoid interference.
        When select_all is true, it selects all existing text (Ctrl+A) before pasting to replace the content; otherwise it pastes at the current cursor position.
        """
        self.last_input_error = None
        try:
            if not text:
                return True
            if not self._restore_text_focus():
                logger.warning(self.last_input_error or "Text input focus validation failed.")
                return False

            # Save current clipboard content with retries
            original_clipboard = None
            for attempt in range(3):  # Increased retries
                try:
                    original_clipboard = pyperclip.paste()
                    break
                except Exception as e:
                    logger.warning(f"Attempt {attempt+1}: Failed to save original clipboard: {e}")
                    await asyncio.sleep(0.2 * (attempt + 1))  # Longer exponential backoff
            else:
                logger.error("Failed to save original clipboard after max retries - proceeding without save")

            # Set text to clipboard with retries and verification
            for attempt in range(3):
                try:
                    pyperclip.copy(text)
                    await asyncio.sleep(0.05)  # Brief pause for sync
                    if pyperclip.paste() != text:
                        raise ValueError("Clipboard copy verification failed")
                    break
                except Exception as e:
                    logger.warning(f"Attempt {attempt+1}: Failed to set clipboard: {e}")
                    await asyncio.sleep(0.2 * (attempt + 1))
            else:
                logger.error("Failed to set clipboard after max retries")
                return False

            # Optionally select all existing text first (Ctrl+A), then paste to replace
            if not self._restore_text_focus():
                logger.warning(self.last_input_error or "Text input focus validation failed.")
                return False
            await asyncio.sleep(0.1)
            if select_all:
                pyautogui.hotkey('ctrl', 'a')
                await asyncio.sleep(0.1)
            pyautogui.hotkey('ctrl', 'v')
            try:
                settle_delay = max(
                    0.2,
                    float(os.getenv("KYLIN_PASTE_SETTLE_DELAY", "0.6")),
                )
            except ValueError:
                settle_delay = 0.6
            await asyncio.sleep(settle_delay)
            if press_enter:
                pyautogui.press("enter")
                await asyncio.sleep(0.1)
            
            # Restore original clipboard content with retries and final verification
            if original_clipboard is not None:
                restored = False
                for attempt in range(5):
                    try:
                        pyperclip.copy(original_clipboard)
                        await asyncio.sleep(0.05)
                        if pyperclip.paste() == original_clipboard:
                            restored = True
                            break
                        else:
                            logger.warning(f"Attempt {attempt+1}: Restoration verification failed - retrying")
                    except Exception as e:
                        logger.warning(f"Attempt {attempt+1}: Failed to restore clipboard: {e}")
                        await asyncio.sleep(0.2 * (attempt + 1))
                if not restored:
                    logger.critical("Failed to restore original clipboard after max retries - interference may have occurred!")
                else:
                    logger.debug("Clipboard restored successfully")
            
            return True
        except Exception as e:
            logger.error(f"Error typing text '{text}': {e}")
            # Attempt emergency restore if possible
            if original_clipboard is not None:
                try:
                    pyperclip.copy(original_clipboard)
                except:
                    pass
            return False
    
    
    async def press_key(self, key: str) -> bool:
        """Press a single key"""
        try:
            pyautogui.press(pyautogui_key_name(normalize_key(key)))
            return True
        except Exception as e:
            logger.error(f"Error pressing key '{key}': {e}")
            return False

    async def press_hotkey(self, key1: str, key2: str, key3: Optional[str] = None) -> bool:
        """Press a combination of keys"""
        try:
            keys = [normalize_key(k) for k in (key1, key2, key3)]
            keys = [k for k in keys if k]
            if not keys:
                logger.error("Cannot press an empty hotkey.")
                return False
            pyautogui.hotkey(*(pyautogui_key_name(k) for k in keys))
            return True
        except Exception as e:
            logger.error(f"Error pressing hotkey {key1, key2, key3}: {e}")
            return False

    async def take_screenshot(self, save_path: Optional[str] = None) -> bool:
        """Take a screenshot"""
        try:
            screenshot = pyautogui.screenshot()
            if save_path:
                screenshot.save(save_path)
            return True
        except Exception as e:
            logger.error(f"Error taking screenshot: {e}")
            return False
    
    async def get_mouse_position(self) -> Tuple[int, int]:
        """Get current mouse position"""
        try:
            return pyautogui.position()
        except Exception as e:
            logger.error(f"Error getting mouse position: {e}")
            return (0, 0)

    async def move_mouse(self, x: float, y: float, duration: float = 0.0) -> bool:
        """Move mouse to specified position"""
        if (0 <= x <= 1 and 0 <= y <= 1):
            # If the coordinates are normalized (0 to 1)
            pos_x = self.getscreen_size()[0] * x
            pos_y = self.getscreen_size()[1] * y
        else:
            # If the coordinates are normalized (0 to 1000)
            pos_x = self.getscreen_size()[0] * (x / 1000)
            pos_y = self.getscreen_size()[1] * (y / 1000)
        try:
            pyautogui.moveTo(pos_x, pos_y, duration=duration)
            return True
        except Exception as e:
            logger.error(f"Error moving mouse to ({x}, {y}): {e}")
            return False
