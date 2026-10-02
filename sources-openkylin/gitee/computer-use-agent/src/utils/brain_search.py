from __future__ import annotations

import json
import logging
import re
from typing import Callable, Optional, Tuple

from langchain_core.language_models.chat_models import BaseChatModel
from langchain_core.messages import BaseMessage

from src.agent.message_manager.service import MessageManager
from src.utils.record_store import RecordStore

logger = logging.getLogger(__name__)


class BrainSearchFlow:
    def __init__(self, record_store: RecordStore) -> None:
        self.record_store = record_store

    def extract_read_files(self, parsed: dict) -> Optional[list[str]]:
        read_value = parsed.get("read_files")
        if not read_value:
            return None
        if isinstance(read_value, dict):
            files = read_value.get("files", [])
            if isinstance(files, list):
                return [str(f).strip() for f in files if str(f).strip()]
            return None
        if isinstance(read_value, list):
            return [str(f).strip() for f in read_value if str(f).strip()]
        if isinstance(read_value, str):
            return [f.strip() for f in read_value.split(",") if f.strip()]
        return None

    def parse_response(self, text: str, label: str = "Brain") -> dict:
        cleaned = re.sub(r"^```(json)?", "", text.strip())
        cleaned = re.sub(r"```$", "", cleaned).strip()
        logger.debug("[%s] Raw text: %s", label, cleaned)
        try:
            return json.loads(cleaned)
        except json.JSONDecodeError:
            extracted = self._extract_first_json_object(cleaned)
            if extracted:
                logger.warning("[%s] Parsed JSON object extracted from non-JSON response text.", label)
                return json.loads(extracted)
            raise

    def _extract_first_json_object(self, text: str) -> Optional[str]:
        start = text.find("{")
        if start < 0:
            return None

        depth = 0
        in_string = False
        escape = False
        for idx in range(start, len(text)):
            char = text[idx]
            if escape:
                escape = False
                continue
            if char == "\\":
                escape = True
                continue
            if char == '"':
                in_string = not in_string
                continue
            if in_string:
                continue
            if char == "{":
                depth += 1
            elif char == "}":
                depth -= 1
                if depth == 0:
                    return text[start:idx + 1]
        return None

    async def maybe_reinvoke(
        self,
        parsed: dict,
        build_state_content: Callable[..., list[dict]],
        message_manager: MessageManager,
        llm: BaseChatModel,
    ) -> Tuple[dict, list[BaseMessage]]:
        # Handle case where parsed is not a dict (e.g., model returned array or invalid JSON)
        if not isinstance(parsed, dict):
            logger.warning(
                "[BrainSearchFlow] Expected parsed response to be a dict, got %s. Raw value: %s",
                type(parsed).__name__,
                str(parsed)[:200],
            )
            # Try to extract a dict from the first element if it's a list
            if isinstance(parsed, list) and len(parsed) > 0:
                if isinstance(parsed[0], dict):
                    logger.info("[BrainSearchFlow] Using first element of list as parsed response")
                    parsed = parsed[0]
                else:
                    logger.warning("[BrainSearchFlow] Cannot recover from list response")
                    # Return as-is, will cause error handling upstream
                    return parsed, message_manager.get_messages()
            else:
                logger.warning("[BrainSearchFlow] Cannot recover from non-dict response")
                return parsed, message_manager.get_messages()
        
        read_files = self.extract_read_files(parsed)
        if read_files:
            file_contents = self.record_store.read_files(read_files)
            state_content = build_state_content(
                read_files_content=file_contents,
                read_files_list=read_files,
            )
            message_manager._remove_last_state_message()
            message_manager._remove_last_AIntool_message()
            message_manager.add_state_message(state_content)
            brain_messages = message_manager.get_messages()
            response = await llm.ainvoke(brain_messages)
            parsed = self.parse_response(str(response.content), label="Brain post-read")
            return parsed, brain_messages
        return parsed, message_manager.get_messages()
