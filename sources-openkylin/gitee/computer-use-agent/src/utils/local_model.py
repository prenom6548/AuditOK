"""
Local model wrapper using Hugging Face transformers.
Provides a ChatOpenAI-compatible interface for local models.
"""
from __future__ import annotations

import base64
import io
import json
import logging
import os
import re
from typing import Any, Dict, List, Optional, Sequence

from PIL import Image
from langchain_core.messages import (
    AIMessage,
    BaseMessage,
    HumanMessage,
    SystemMessage,
)
from langchain_core.outputs import ChatGeneration, ChatResult

logger = logging.getLogger(__name__)


class LocalTransformersModel:
    """
    A wrapper around Hugging Face transformers models that mimics ChatOpenAI interface.
    Supports vision-language models like Qwen3VL.
    """

    def __init__(
        self,
        model_path: str,
        temperature: float = 0.1,
        max_tokens: int = 2048,
        device_map: str = "auto",
        torch_dtype: str = "bfloat16",
        trust_remote_code: bool = True,
        load_in_8bit: bool = False,
        load_in_4bit: bool = False,
        cpu_offload: bool = True,
        **kwargs,
    ):
        """
        Initialize local model using transformers.

        Args:
            model_path: Path to the local model directory
            temperature: Sampling temperature
            max_tokens: Maximum tokens to generate
            device_map: Device mapping strategy ("auto", "cuda", "cpu")
            torch_dtype: Precision ("float16", "bfloat16", "float32")
            trust_remote_code: Whether to trust remote code
            load_in_8bit: Load model in 8-bit quantization (saves memory)
            load_in_4bit: Load model in 4-bit quantization (saves more memory)
            cpu_offload: Enable CPU offloading for memory-constrained GPUs
        """
        self.model_path = model_path
        self.temperature = temperature
        self.max_tokens = max_tokens
        self._model_name = f"local:{os.path.basename(model_path)}"

        # Store extra kwargs for generation
        self._extra_kwargs = kwargs
        self._cpu_offload = cpu_offload

        # Lazy import torch and transformers to avoid early torchvision issues
        import torch

        # Determine torch dtype
        dtype_map = {
            "float16": torch.float16,
            "bfloat16": torch.bfloat16,
            "float32": torch.float32,
        }
        torch_dtype_resolved = dtype_map.get(torch_dtype, torch.bfloat16)

        # Check if CUDA is available
        cuda_available = torch.cuda.is_available()
        if cuda_available:
            gpu_memory = torch.cuda.get_device_properties(0).total_memory / (1024**3)  # GB
            logger.info(f"CUDA available: GPU with {gpu_memory:.2f} GB memory")
            
            # Auto-adjust settings based on GPU memory (only if not explicitly set)
            # Note: 8-bit quantization has compatibility issues, use with caution
            if gpu_memory < 8 and not load_in_8bit and not load_in_4bit:
                logger.info("GPU memory < 8GB, enabling 4-bit quantization")
                load_in_4bit = True
        else:
            device_map = "cpu"
            torch_dtype_resolved = torch.float16  # CPU fallback
            logger.warning("CUDA not available, using CPU with float16")

        logger.info(f"Loading local model from: {model_path}")
        logger.info(f"Device map: {device_map}, dtype: {torch_dtype_resolved}, 8bit: {load_in_8bit}, 4bit: {load_in_4bit}")

        # Load processor and model - import inside to avoid early torchvision trigger
        try:
            # Try loading as Qwen3VL first (based on config.json architectures)
            from transformers import Qwen3VLForConditionalGeneration, Qwen3VLProcessor

            self.processor = Qwen3VLProcessor.from_pretrained(model_path, trust_remote_code=trust_remote_code)
            
            # Prepare model loading kwargs
            model_kwargs = {
                "trust_remote_code": trust_remote_code,
                "device_map": device_map,
            }
            
            if load_in_4bit:
                from transformers import BitsAndBytesConfig
                model_kwargs["quantization_config"] = BitsAndBytesConfig(
                    load_in_4bit=True,
                    bnb_4bit_compute_dtype=torch_dtype_resolved,
                    bnb_4bit_use_double_quant=True,
                    bnb_4bit_quant_type="nf4",
                )
            elif load_in_8bit:
                from transformers import BitsAndBytesConfig
                model_kwargs["quantization_config"] = BitsAndBytesConfig(
                    load_in_8bit=True,
                    llm_int8_threshold=6.0,
                    llm_int8_enable_fp32_cpu_offload=True,  # Enable CPU offload for 8bit
                )
            else:
                model_kwargs["torch_dtype"] = torch_dtype_resolved
            
            self.model = Qwen3VLForConditionalGeneration.from_pretrained(
                self.model_path,
                **model_kwargs,
            )
            logger.info("Loaded as Qwen3VLForConditionalGeneration")
        except Exception as e1:
            # Fallback to auto classes
            logger.warning(f"Qwen3VL loading failed ({e1}), trying auto classes")
            from transformers import AutoModel, AutoProcessor

            self.processor = AutoProcessor.from_pretrained(model_path, trust_remote_code=trust_remote_code)
            
            # Prepare fallback model kwargs
            fallback_kwargs = {
                "device_map": device_map,
                "trust_remote_code": trust_remote_code,
            }
            
            if load_in_4bit:
                from transformers import BitsAndBytesConfig
                fallback_kwargs["quantization_config"] = BitsAndBytesConfig(
                    load_in_4bit=True,
                    bnb_4bit_compute_dtype=torch_dtype_resolved,
                    bnb_4bit_use_double_quant=True,
                    bnb_4bit_quant_type="nf4",
                )
            elif load_in_8bit:
                from transformers import BitsAndBytesConfig
                fallback_kwargs["quantization_config"] = BitsAndBytesConfig(
                    load_in_8bit=True,
                    llm_int8_threshold=6.0,
                    llm_int8_enable_fp32_cpu_offload=True,
                )
            else:
                fallback_kwargs["torch_dtype"] = torch_dtype_resolved
            
            self.model = AutoModel.from_pretrained(
                model_path,
                **fallback_kwargs,
            )
            logger.info("Loaded as AutoModel")

        self.model.eval()
        logger.info(f"Local model loaded successfully: {model_path}")

    @property
    def model_name(self) -> str:
        return self._model_name

    def _parse_messages(self, messages: Sequence[BaseMessage]) -> tuple[Optional[str], List[Dict[str, Any]]]:
        """
        Parse LangChain messages into system prompt and conversation turns.

        Returns:
            tuple: (system_prompt, conversation_turns)
            where conversation_turns is a list of {"role": str, "content": list} dicts
        """
        system_prompt = None
        turns = []  # List of {"role": str, "content": [...]}
        current_turn = None

        for msg in messages:
            if isinstance(msg, SystemMessage):
                system_prompt = msg.content if isinstance(msg.content, str) else str(msg.content)
                continue

            if isinstance(msg, HumanMessage):
                # Start a new user turn
                if current_turn is not None and current_turn["role"] == "user":
                    # Shouldn't happen normally, but handle it
                    turns.append(current_turn)
                current_turn = {"role": "user", "content": []}
                self._append_content_to_turn(current_turn, msg.content)

            elif isinstance(msg, AIMessage):
                # Start/complete an assistant turn
                if current_turn is not None:
                    turns.append(current_turn)
                current_turn = {"role": "assistant", "content": []}
                self._append_content_to_turn(current_turn, msg.content)

        # Don't forget the last turn
        if current_turn is not None:
            turns.append(current_turn)

        return system_prompt, turns

    def _append_content_to_turn(self, turn: Dict[str, Any], content) -> None:
        """Append content to a turn's content list."""
        if isinstance(content, str):
            if content.startswith("data:image"):
                # Base64 image
                match = re.match(r"data:image/(\w+);base64,(.+)", content)
                if match:
                    img_data = match.group(2)
                    image_bytes = base64.b64decode(img_data)
                    image = Image.open(io.BytesIO(image_bytes))
                    turn["content"].append({"type": "image", "source": image})
            else:
                turn["content"].append({"type": "text", "text": content})
        elif isinstance(content, list):
            for item in content:
                if isinstance(item, dict):
                    item_type = item.get("type", "text")
                    if item_type == "text":
                        text_val = item.get("text") or item.get("content", "")
                        turn["content"].append({"type": "text", "text": text_val})
                    elif item_type == "image_url":
                        image_url = item.get("image_url", "")
                        if isinstance(image_url, dict):
                            image_url = image_url.get("url", "")
                        if image_url.startswith("data:image"):
                            match = re.match(r"data:image/(\w+);base64,(.+)", image_url)
                            if match:
                                img_data = match.group(2)
                                image_bytes = base64.b64decode(img_data)
                                image = Image.open(io.BytesIO(image_bytes))
                                turn["content"].append({"type": "image", "source": image})
                    elif item_type == "image" and "source" in item:
                        turn["content"].append({"type": "image", "source": item["source"]})

    def _build_prompt(
        self, system_prompt: Optional[str], turns: List[Dict[str, Any]], response_format: Optional[Dict] = None
    ) -> tuple[List[Dict[str, Any]], Dict[str, Any]]:
        """
        Build the final prompt for the model.

        Returns:
            tuple: (messages_for_model, generation_config)
        """
        messages = []

        # Add system prompt if present
        if system_prompt:
            # If response_format is specified, prepend JSON format instruction
            if response_format:
                system_prompt = """IMPORTANT: You MUST output ONLY valid JSON. No explanations, no text before or after the JSON.
Your response must be parseable as a JSON object. Do not include markdown code blocks.""" + "\n\n" + system_prompt
            messages.append({"role": "system", "content": system_prompt})

        # Add each turn
        for turn in turns:
            role = turn["role"]
            content_items = turn.get("content", [])

            # Convert internal format to Qwen3VL format
            qwen_content = []
            for item in content_items:
                if item["type"] == "text":
                    qwen_content.append({"type": "text", "text": item["text"]})
                elif item["type"] == "image":
                    qwen_content.append({"type": "image", "image": item["source"]})

            messages.append({"role": role, "content": qwen_content})

        # Build generation config
        generation_config = {
            "max_new_tokens": self.max_tokens,
            "temperature": self.temperature if self.temperature > 0 else None,
            "do_sample": self.temperature > 0,
            "pad_token_id": self.processor.tokenizer.pad_token_id,
            "eos_token_id": self.processor.tokenizer.eos_token_id,
        }

        # Handle response format (JSON schema mode)
        if response_format:
            generation_config["temperature"] = 0.1  # Lower temp for structured output

        return messages, generation_config

    def _generate(self, messages: Sequence[BaseMessage], **kwargs: Any) -> ChatResult:
        """Generate response using transformers"""
        import torch

        response_format = kwargs.get("response_format")
        if response_format is None:
            response_format = getattr(self, "_response_format", None)
        
        # Debug logging
        if response_format:
            logger.info("[LocalTransformersModel] response_format present: %s", type(response_format).__name__)
            if isinstance(response_format, dict):
                logger.info("[LocalTransformersModel] response_format keys: %s", list(response_format.keys()))
                if "json_schema" in response_format:
                    js = response_format["json_schema"]
                    if isinstance(js, dict):
                        logger.info("[LocalTransformersModel] json_schema name: %s", js.get("name", "unknown"))
        else:
            logger.warning("[LocalTransformersModel] No response_format found in kwargs or instance!")

        # Parse messages into turns
        system_prompt, turns = self._parse_messages(messages)

        # Build prompt
        model_messages, gen_config = self._build_prompt(system_prompt, turns, response_format)

        # Apply chat template
        text = self.processor.apply_chat_template(
            model_messages,
            tokenize=False,
            add_generation_prompt=True,
        )

        # Extract all images from all turns
        all_images = []
        for turn in turns:
            for item in turn.get("content", []):
                if item.get("type") == "image" and "source" in item:
                    all_images.append(item["source"])

        # Limit number of images to prevent OOM
        max_images = 4
        if len(all_images) > max_images:
            logger.warning(f"Limiting images from {len(all_images)} to {max_images}")
            all_images = all_images[:max_images]

        # Process inputs
        inputs = self.processor(
            text=[text],
            images=all_images if all_images else None,
            videos=None,
            return_tensors="pt",
            padding=True,
        )

        # Move to device
        if hasattr(self.model, "device") and hasattr(torch, "cuda"):
            inputs = {k: v.to(self.model.device) for k, v in inputs.items() if isinstance(v, torch.Tensor)}

        # Generate with memory-efficient settings
        with torch.no_grad():
            outputs = self.model.generate(
                **inputs,
                max_new_tokens=min(gen_config["max_new_tokens"], 1024),  # Limit output length
                temperature=gen_config["temperature"],
                do_sample=gen_config["do_sample"],
                pad_token_id=gen_config["pad_token_id"],
                eos_token_id=gen_config["eos_token_id"],
                use_cache=True,  # Enable KV cache for efficiency
            )

        # Clear CUDA cache after generation
        if hasattr(torch, "cuda") and torch.cuda.is_available():
            torch.cuda.empty_cache()

        # Decode output
        generated_ids = outputs[0][inputs["input_ids"].shape[1] :]
        response_text = self.processor.decode(generated_ids, skip_special_tokens=True)

        # Handle JSON format if requested - extract JSON from response
        if response_format:
            response_text = self._extract_json_from_response(response_text)
            logger.debug(f"Extracted JSON: {response_text[:200]}...")

        # Create ChatGeneration
        generation = ChatGeneration(message=AIMessage(content=response_text), text=response_text)

        return ChatResult(generations=[generation])

    def _extract_json_from_response(self, text: str) -> str:
        """Extract JSON from model response text"""
        if not text or not text.strip():
            logger.warning("Empty model response")
            return "{}"
        
        text = text.strip()
        original_text = text
        
        # Remove common non-JSON prefixes
        # Look for patterns like "action=..." or "Response: ..."
        json_start_patterns = [r'\{', r'\[']
        json_start = -1
        for pattern in json_start_patterns:
            idx = text.find(pattern)
            if idx >= 0:
                if json_start == -1 or idx < json_start:
                    json_start = idx
        
        if json_start > 0:
            text = text[json_start:]
        
        # Try to find JSON block with markdown
        json_match = re.search(r"```(?:json)?\s*({.*?})\s*```", text, re.DOTALL)
        if json_match:
            return json_match.group(1)
        
        # Try to find JSON array block
        array_match = re.search(r"```(?:json)?\s*(\[.*?\])\s*```", text, re.DOTALL)
        if array_match:
            return array_match.group(1)

        # Try to find raw JSON object (balance braces)
        start = text.find("{")
        if start >= 0:
            brace_count = 0
            in_string = False
            escape_next = False
            for i in range(start, len(text)):
                char = text[i]
                if escape_next:
                    escape_next = False
                    continue
                if char == "\\":
                    escape_next = True
                    continue
                if char == '"' and not escape_next:
                    in_string = not in_string
                    continue
                if not in_string:
                    if char == "{":
                        brace_count += 1
                    elif char == "}":
                        brace_count -= 1
                        if brace_count == 0:
                            candidate = text[start:i+1]
                            try:
                                json.loads(candidate)
                                return candidate
                            except json.JSONDecodeError:
                                pass
                            break
        
        # Try to find raw JSON array (balance brackets)
        start = text.find("[")
        if start >= 0:
            bracket_count = 0
            in_string = False
            escape_next = False
            for i in range(start, len(text)):
                char = text[i]
                if escape_next:
                    escape_next = False
                    continue
                if char == "\\":
                    escape_next = True
                    continue
                if char == '"' and not escape_next:
                    in_string = not in_string
                    continue
                if not in_string:
                    if char == "[":
                        bracket_count += 1
                    elif char == "]":
                        bracket_count -= 1
                        if bracket_count == 0:
                            candidate = text[start:i+1]
                            try:
                                json.loads(candidate)
                                return candidate
                            except json.JSONDecodeError:
                                pass
                            break

        # Last resort: try simple find { and }
        try:
            end = text.rfind("}") + 1
            if end > 0:
                candidate = text[:end]
                json.loads(candidate)
                return candidate
        except json.JSONDecodeError:
            pass

        # If all else fails, log and return empty JSON
        logger.warning(f"Could not extract valid JSON from response: {original_text[:200]}...")
        return "{}"

    async def ainvoke(self, messages: Sequence[BaseMessage], **kwargs: Any) -> Any:
        """Async invoke - runs sync in thread pool"""
        import asyncio

        loop = asyncio.get_event_loop()
        return await loop.run_in_executor(None, lambda: self.invoke(messages, **kwargs))

    def invoke(self, messages: Sequence[BaseMessage], **kwargs: Any) -> Any:
        """Synchronous invoke matching ChatOpenAI interface"""
        import copy
        
        chat_result = self._generate(messages, **kwargs)
        response = chat_result.generations[0]
        response_text = response.text

        # Create a response object similar to ChatOpenAI
        class ModelResponse:
            def __init__(self, content, response_metadata=None, parsed=None):
                self.content = content
                self.response_metadata = response_metadata or {}
                if parsed is not None:
                    self.parsed = parsed

        # Try to parse as JSON if response_format was specified
        parsed = None
        response_format = getattr(self, "_response_format", None)
        if response_format:
            try:
                # Clean the response text
                cleaned_text = response_text.strip()
                logger.debug("[LocalModel] Raw response text: %s", cleaned_text[:500])
                
                # Remove markdown code blocks if present
                if cleaned_text.startswith("```"):
                    lines = cleaned_text.split("\n")
                    if lines[0].startswith("```"):
                        cleaned_text = "\n".join(lines[1:])
                    if lines[-1].strip() == "```":
                        cleaned_text = "\n".join(lines[:-1])
                    cleaned_text = cleaned_text.strip()

                # Try to parse as JSON
                parsed_dict = json.loads(cleaned_text)
                logger.debug("[LocalModel] Parsed dict: %s", parsed_dict)

                # Post-process the parsed dict to convert action format
                # Model may output: {"action": {"action": "Click", "position": [...]}}
                # We need: {"action": [{"Click": {"position": [...]}}]}
                parsed_dict = self._normalize_action_format(parsed_dict)
                logger.debug("[LocalModel] Normalized dict: %s", parsed_dict)

                parsed = parsed_dict
            except Exception as e:
                logger.warning(f"Failed to parse response as JSON: {e}")
                logger.debug(f"Raw response text: {response_text[:500]}...")
                parsed = None
        else:
            logger.warning("[LocalModel] No response_format set, parsed will be None")

        response_obj = ModelResponse(
            content=response_text,
            response_metadata={
                "model": self._model_name,
                "usage": {
                    "prompt_tokens": -1,  # Not tracked for local models
                    "completion_tokens": -1,
                    "total_tokens": -1,
                },
            },
            parsed=parsed,
        )

        return response_obj
    
    def _normalize_action_format(self, data: dict) -> dict:
        """
        Normalize action format from model output to expected format.
        
        Model may output: {"action": {"action": "Click", "position": [...]}}
        We need: {"action": [{"Click": {"position": [...]}}]}
        """
        if not isinstance(data, dict):
            return data
        
        # Check if this is an action output
        if "action" in data:
            action_data = data["action"]
            
            # Case 1: action is a dict with "action" key (wrong format)
            if isinstance(action_data, dict) and "action" in action_data:
                action_name = action_data["action"]
                # Copy all other fields as parameters
                params = {k: v for k, v in action_data.items() if k != "action"}
                # Convert to correct format: {action_name: params}
                data["action"] = [{action_name: params}]
            
            # Case 2: action is already a list
            elif isinstance(action_data, list):
                normalized_actions = []
                for item in action_data:
                    if isinstance(item, dict) and "action" in item:
                        action_name = item["action"]
                        params = {k: v for k, v in item.items() if k != "action"}
                        normalized_actions.append({action_name: params})
                    else:
                        normalized_actions.append(item)
                data["action"] = normalized_actions
        
        return data

    def bind(self, **kwargs: Any) -> "LocalTransformersModel":
        """Bind parameters for future calls"""
        # Create a copy with updated kwargs
        import copy

        new_instance = copy.copy(self)
        new_instance._extra_kwargs = {**self._extra_kwargs, **kwargs}

        # Store response_format if provided
        if "response_format" in kwargs:
            new_instance._response_format = kwargs["response_format"]

        return new_instance

    def with_structured_output(self, schema: Dict | type) -> "LocalTransformersModel":
        """Support structured output - wraps bind"""
        return self.bind(response_format={"type": "json_schema", "json_schema": schema})


# Compatibility alias
LocalModel = LocalTransformersModel
