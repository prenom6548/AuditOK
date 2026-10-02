class OutputSchemas:
    """
    A container for all model output schemas and their associated response formats.
    """

    ACTION_SCHEMA = {
        "type": "object",
        "properties": {
            "action": {
                "type": "array",
                "minItems": 0,
                "default": [{"wait": {}}],
                "items": {
                    "type": "object",
                    "properties": {
                        # ----- task finished -----
                        "done": {
                            "type": "object",
                            "properties": {"text": {"type": "string"}},
                        },

                        # ----- typing -----
                        "input_text": {
                            "type": "object",
                            "properties": {
                                "text": {"type": "string"},
                                "press_enter": {
                                    "type": "boolean",
                                    "default": False,
                                    "description": "Press Enter after pasting text only when the current goal explicitly requires send/search/submit/confirm/modeify.",
                                },
                                "select_all": {
                                    "type": "boolean",
                                    "default": True,
                                    "description": "Select all existing text before pasting to replace the field. Set false to insert/append at the cursor without clearing existing content (e.g., Excel cells, document editing).",
                                },
                            },
                            "required": ["text"],
                        },

                        # ----- open app -----
                        "open_app": {
                            "type": "object",
                            "properties": {"app_name": {"type": "string"}},
                            "required": ["app_name"],
                        },

                        # ----- AppleScript -----
                        "run_apple_script": {
                            "type": "object",
                            "properties": {"script": {"type": "string"}},
                            "required": ["script"],
                        },

                        # ----- hotkeys -----
                        "Hotkey": {
                            "type": "object",
                            "properties": {"key": {"type": "string"}},
                            "required": ["key"],
                        },
                        "multi_Hotkey": {
                            "type": "object",
                            "properties": {
                                "key1": {"type": "string"},
                                "key2": {"type": "string"},
                                "key3": {"anyOf": [{"type": "string"}, {"type": "null"}]},
                            },
                            "required": ["key1", "key2"],
                        },

                        # ----- clicks -----
                        "RightSingle": {
                            "type": "object",
                            "properties": {
                                "position": {
                                    "type": "array",
                                    "items": {"type": "number"},
                                    "minItems": 2,
                                    "maxItems": 2,
                                },
                            },
                            "required": ["position"],
                        },
                        "Click": {
                            "type": "object",
                            "properties": {
                                "position": {
                                    "type": "array",
                                    "items": {"type": "number"},
                                    "minItems": 2,
                                    "maxItems": 2,
                                },
                                "expect_clipboard_change": {
                                    "type": "boolean",
                                    "default": False,
                                    "description": "Set true only when clicking a visible Copy/复制 command.",
                                },
                            },
                            "required": ["position"],
                        },
                        "DoubleClick": {
                            "type": "object",
                            "properties": {
                                "position": {
                                    "type": "array",
                                    "items": {"type": "number"},
                                    "minItems": 2,
                                    "maxItems": 2,
                                },
                            },
                            "required": ["position"],
                        },

                        # ----- drag -----
                        "Drag": {
                            "type": "object",
                            "properties": {
                                "position1": {"type": "array", "items": {"type": "number"}},
                                "position2": {"type": "array", "items": {"type": "number"}},
                            },
                            "required": ["position1", "position2"],
                        },

                        # ----- move mouse -----
                        "move_mouse": {
                            "type": "object",
                            "properties": {
                                "position": {"type": "array", "items": {"type": "number"}}
                            },
                            "required": ["position"],
                        },

                        # ----- scrolling -----
                        "scroll_up": {
                            "type": "object",
                            "properties": {
                                "position": {
                                    "type": "array",
                                    "items": {"type": "number"},
                                    "description": "Coordinates inside the target scrollable area. The framework will ensure move_mouse targets this same area before scrolling.",
                                },
                                "dx": {"type": "number"},
                                "dy": {
                                    "type": "number",
                                    "description": "Use 1-2 for compact selectors/dropdowns, 3-6 for full pages or long lists.",
                                },
                            },
                            "required": ["position"],
                        },
                        "scroll_down": {
                            "type": "object",
                            "properties": {
                                "position": {
                                    "type": "array",
                                    "items": {"type": "number"},
                                    "description": "Coordinates inside the target scrollable area. The framework will ensure move_mouse targets this same area before scrolling.",
                                },
                                "dx": {"type": "number"},
                                "dy": {
                                    "type": "number",
                                    "description": "Use 1-2 for compact selectors/dropdowns, 3-6 for full pages or long lists.",
                                },
                            },
                            "required": ["position"],
                        },

                        # ----- memory + wait -----
                        "record_info": {
                            "type": "object",
                            "properties": {
                                "text": {"type": "string"},
                                "file_name": {"type": "string"},
                            },
                            "required": ["text", "file_name"],
                        },
                        "wait": {
                            "type": "object",
                            "properties": {
                                "seconds": {
                                    "anyOf": [
                                        {"type": "number", "minimum": 0, "maximum": 1},
                                        {"type": "null"},
                                    ]
                                }
                            },
                        },
                    },
                },
            },
        },
        "required": ["action"],
    }

    ACTION_RESPONSE_FORMAT = {
        "type": "json_schema",
        "json_schema": {
            "name": "agent_action_output",
            "strict": True,
            "schema": ACTION_SCHEMA,
        },
    }

    BRAIN_SCHEMA = {
        "oneOf": [
            {
                "type": "object",
                "properties": {
                    "analysis": {
                        "type": "object",
                        "properties": {
                            "analysis": {"type": "string", "maxLength": 500},
                            "sop_check": {"type": "string", "maxLength": 300}
                        },
                        "required": ["analysis", "sop_check"]
                    },
                    "current_state": {
                        "type": "object",
                        "properties": {
                            "step_evaluate": {"type": "string", "maxLength": 20},
                            "ask_human": {"type": "string", "maxLength": 200},
                            "next_goal": {"type": "string", "maxLength": 600},
                        },
                        "required": ["step_evaluate", "ask_human", "next_goal"],
                    },
                },
                "required": ["analysis", "current_state"],
                "additionalProperties": False,
            },
            {
                "type": "object",
                "properties": {
                    "read_files": {
                        "type": "object",
                        "properties": {
                            "files": {
                                "type": "array",
                                "items": {"type": "string"},
                                "minItems": 1,
                            }
                        },
                        "required": ["files"],
                    }
                },
                "required": ["read_files"],
                "additionalProperties": False,
            },
        ]
    }

    BRAIN_RESPONSE_FORMAT = {
        "type": "json_schema",
        "json_schema": {
            "name": "agent_state_output",
            "strict": True,
            "schema": BRAIN_SCHEMA,
        },
    }

    COMBINED_SCHEMA = {
        "oneOf": [
            {
                "type": "object",
                "properties": {
                    "analysis": BRAIN_SCHEMA["oneOf"][0]["properties"]["analysis"],
                    "current_state": BRAIN_SCHEMA["oneOf"][0]["properties"]["current_state"],
                    "action": ACTION_SCHEMA["properties"]["action"],
                },
                "required": ["analysis", "current_state", "action"],
                "additionalProperties": False,
            },
            BRAIN_SCHEMA["oneOf"][1],
        ]
    }

    COMBINED_RESPONSE_FORMAT = {
        "type": "json_schema",
        "json_schema": {
            "name": "agent_combined_step_output",
            "strict": True,
            "schema": COMBINED_SCHEMA,
        },
    }

    MEMORY_SCHEMA = {
        "type": "object",
        "properties": {
            "summary": {"type": "string"},
            "file_name": {"type": "string"},
        },
        "required": ["summary", "file_name"],
    }

    MEMORY_RESPONSE_FORMAT = {
        "type": "json_schema",
        "json_schema": {
            "name": "memory_output",
            "strict": True,
            "schema": MEMORY_SCHEMA,
        },
    }

    PLANNER_SCHEMA = {
        "type": "object",
        "properties": {
            "iteration_info": {
                "type": "object",
                "properties": {
                    "current_iteration": {"type": "integer", "minimum": 1},
                    "total_iterations": {"type": "integer", "minimum": 1}
                },
                "required": ["current_iteration", "total_iterations"],
                "additionalProperties": False
            },
            "search_summary": {"type": "string"},
            "selected_skills": {
                "type": "array",
                "items": {"type": "string"},
                "default": []
            },
            "natural_language_plan": {
                "type": "string"
            },
            "step_by_step_plan": {
                "type": "array",
                "items": {
                    "type": "object",
                    "properties": {
                        "step_id": {"type": "string", "pattern": "^Step \\d+$"},
                        "description": {"type": "string"},
                        "important_search_info": {"type": "string"},
                    },
                    "required": ["step_id", "description", "important_search_info"],
                    "additionalProperties": False,
                },
            }
        },
        "required": ["iteration_info", "search_summary", "selected_skills", "step_by_step_plan"],
        "additionalProperties": False,
    }

    PLANNER_RESPONSE_FORMAT = {
        "type": "json_schema",
        "json_schema": {
            "name": "planner_output",
            "strict": True,
            "schema": PLANNER_SCHEMA,
        },
    }
