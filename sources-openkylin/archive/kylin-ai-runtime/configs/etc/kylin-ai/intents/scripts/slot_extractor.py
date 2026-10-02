from slot_pattern_rules import SlotPatternRules
from typing import Dict, Optional, Any
import copy
import re

class SlotExtractor:
    """
    槽位提取器，用于从文本中根据预定义的正则规则提取不同类型的槽位
    """
    def __init__(self):
        """初始化槽位提取器"""
        # 疑问句识别规则：包含以下关键词的视为疑问句，槽位提取为空
        self.question_patterns = [
            r'怎么', r'如何', r'为啥', r'为什么', r'吗', r'呢', r'？', r'怎样', r'能否', r'是否'
        ]

        # 初始化槽位提取模式
        self.slot_pattern_rules = SlotPatternRules()
        # 初始化槽位处理函数与正则表达式的映射
        self._init_slot_func_and_patterns_dict()

    def extract_slots(self, user_input: str, predict_intent_name: str, slots_name_list: list):
        """根据意图名称动态调用对应的槽位提取函数"""
        if predict_intent_name not in self.slot_func_and_patterns:
            raise ValueError(f"不支持的意图: {predict_intent_name}")

        # 获取对应的处理函数
        handler_info = self.slot_func_and_patterns[predict_intent_name]
        handler = handler_info["func"]
        patterns = handler_info["patterns"]
        print(
            f"处理 {predict_intent_name} 使用的函数为: {handler.__name__ if hasattr(handler, '__name__') else str(handler)}")

        try:
            # 根据模式类型处理参数
            if isinstance(patterns, list):
                combined_args = copy.deepcopy(patterns)
            elif isinstance(patterns, dict):
                combined_args = list(patterns.values())
            else:
                raise TypeError(f"不支持的patterns类型: {type(patterns)}，仅支持list或dict")

            # 对于需要意图名称的提取函数，添加意图参数
            if handler.__name__ == "extract_pattern_operation_value":
                combined_args.append(slots_name_list)

            args_to_pass = [user_input] + list(combined_args)

            print(f"传递给handler的参数列表: {args_to_pass}")
            print(f"参数数量: {len(args_to_pass)}")

            return handler(*args_to_pass)

        except Exception as e:
            raise RuntimeError(f"处理意图 {predict_intent_name} 时发生错误: {str(e)}")

    def is_question(self, text: str) -> bool:
        """判断文本是否为疑问句"""
        for pattern in self.question_patterns:
            if re.search(pattern, text, re.IGNORECASE):
                return True
        return False

    def extract_pattern_operation_value(self, text, patterns, slots_name_list):
        """从文本中提取槽位值"""
        # 初始化结果字典
        result = {}

        if not slots_name_list:
            return result

        for slot_name in slots_name_list:
            result[slot_name] = ""

        for current_slot in slots_name_list:
            # 遍历所有模式，匹配到第一个符合的就设置值
            for (pattern, operation) in patterns:
                match = re.search(pattern, text, re.IGNORECASE)  # 忽略大小写匹配
                if match:
                    # 处理值（支持lambda或固定值）
                    if callable(operation):
                        value = operation(match)
                    else:
                        value = operation

                    if value is not None:
                        if isinstance(value, tuple):
                            result[current_slot] = str(value[1]) + value[0][0]
                        else:
                            result[current_slot] = str(value) if value is not None else ""
                        print(f"匹配到槽位 {current_slot}，值为: {value}")
                        break  # 匹配到一个后停止，避免重复覆盖

        return result

    def extract_multimodal_input_info(self, text: str, attachment_rules, api_rules) -> Dict[str, str]:
        """核心提取逻辑：先确定附件类型，再匹配对应API接口"""
        result = {
            "附件类型": "",
            "API接口名称": ""
        }

        # 识别附件类型（图片/文档）
        for pattern, att_type in attachment_rules:
            if re.search(pattern, text, re.IGNORECASE):  # 忽略大小写匹配
                result["附件类型"] = att_type
                break

        # 根据附件类型匹配对应API接口
        if result["附件类型"] == "图片":
            # 先匹配明确的操作指令
            for pattern, api in api_rules:
                if re.search(pattern, text, re.IGNORECASE):
                    result["API接口名称"] = api
                    break

        elif result["附件类型"] == "文档":
            # 文档类需求统一对应"文本总结"接口
            result["API接口名称"] = "文本总结"

        # 特殊情况补充（无明确附件标识但可推断）
        if result["附件类型"] == "":
            # 无"图片/报告"但含图片生成关键词→推断为图片类型
            if re.search(r'生成|制作|绘制|背景图', text, re.IGNORECASE):
                result["附件类型"] = "图片"
                result["API接口名称"] = "文生图"

        return result

    def extract_application_info(self, text: str, operation_patterns, common_applications) -> Dict[str, str]:
        """从文本中提取应用控制信息"""
        result = {
            "桌面应用名称": "",
            "附件名称": "",
            "操作类型": ""
        }

        # 提取操作类型
        for pattern, operation in operation_patterns:
            if re.search(pattern, text, re.IGNORECASE):
                result["操作类型"] = operation
                break

        # 提取桌面应用名称
        # 先从常见应用列表中匹配
        for app in common_applications:
            if app in text:
                result["桌面应用名称"] = app
                break

        # 如果未匹配到常见应用，尝试从文本中提取
        if result["桌面应用名称"] == "" and result["操作类型"] != "":
            # 处理"操作+应用"的模式
            pattern = re.escape(result["操作类型"])
            match = re.search(
                f'{pattern}(?:我的|的|下|了|一下|)*(.+?)(?:，|,| |$)', text, re.IGNORECASE)
            if match and match.group(1):
                result["桌面应用名称"] = match.group(1).strip()

        # 提取附件名称（如果有）
        # 处理"用XX打开YY"的模式
        match = re.search(
            r'用(.+?)打开(.+?)(?:文档|图片|表格|pdf文件|PPT|txt文档|截图|)$', text, re.IGNORECASE)
        if match:
            # 从"用XX打开"中提取应用名称（如果尚未识别）
            if result["桌面应用名称"] == "" and match.group(1):
                result["桌面应用名称"] = match.group(1).strip()
            # 提取附件名称
            if match.group(2):
                result["附件名称"] = match.group(2).strip()

        # 处理"打开应用，操作附件"的模式
        if result["操作类型"] == "打开" and result["桌面应用名称"] != "":
            # 移除已知的操作类型和应用名称，剩余部分作为附件名称
            temp_text = text
            for pattern, _ in operation_patterns:
                temp_text = re.sub(pattern, '', temp_text, flags=re.IGNORECASE)
            temp_text = re.sub(re.escape(result["桌面应用名称"]), '', temp_text)
            # 清理标点符号和空格
            temp_text = re.sub(r'[，,。.；;：:、]', '', temp_text).strip()
            if temp_text:
                result["附件名称"] = temp_text

        # 处理直接打开附件的模式（未指定应用）
        if result["操作类型"] == "打开" and result["桌面应用名称"] == "":
            match = re.search(
                r'打开(.+?)(?:文档|图片|表格|pdf文件|PPT|txt文档|截图|)$', text, re.IGNORECASE)
            if match and match.group(1):
                result["附件名称"] = match.group(1).strip()

        return result

    def extract_player_controls(self, text, media_type_patterns, player_operations):
        """从文本中提取多媒体播放器控制的槽位信息"""
        result = {
            "类型": "",
            "操作": ""
        }

        # 提取媒体类型
        for pattern, media_type in media_type_patterns:
            if re.search(pattern, text, re.IGNORECASE):
                result["类型"] = str(media_type) if media_type is not None else ""
                break

        for pattern, op in player_operations:
            if re.search(pattern, text, re.IGNORECASE):
                result["操作"] = str(op) if op is not None else ""
                break

        return result

    def extract_zoom_level(self, text: str, zoom_patterns) -> Dict[str, str]:
        """从文本中提取屏幕缩放比例"""
        result = ""
        for pattern, operation in zoom_patterns:
            match = re.search(pattern, text, re.IGNORECASE)
            if match:
                # 处理匹配结果
                if callable(operation):
                    result = operation(match)
                else:
                    result = operation

        return {"缩放比例": result if result else ""}

    def extract_alarm_info(self, text: str, operation_patterns, date_patterns, event_keywords) -> Dict[str, str]:
        """从文本中提取闹钟设置信息"""
        result = {
            "日期": "",
            "事件": "",
            "操作": ""
        }

        # 提取操作类型
        for pattern, operation in operation_patterns:
            if re.search(pattern, text, re.IGNORECASE):
                result["操作"] = operation
                break

        # 提取日期信息
        for pattern in date_patterns:
            match = re.search(pattern, text, re.IGNORECASE)
            if match:
                result["日期"] = match.group(0)
                break

        # 提取事件信息
        for keyword in event_keywords:
            if keyword in text:
                result["事件"] = keyword
                break

        if result["日期"] == "" or result["事件"] == "":
            result = self.extract_time_and_subject(text)

        return result

    def extract_schedule_info(self, text: str, schedule_operation_patterns, date_patterns, event_keywords) -> Dict[str, str]:
        """从文本中提取日程信息"""
        result = {
            "日期": "",
            "事件": "",
            "操作": ""
        }

        # 提取操作类型
        for pattern, operation in schedule_operation_patterns:
            if re.search(pattern, text, re.IGNORECASE):
                result["操作"] = operation
                break

        # 提取日期信息
        for pattern in date_patterns:
            if pattern in text:
                result["日期"] = pattern
                break

        return result

    def extract_timezone_info(self, text: str, operation_rules, timezone_keywords) -> Dict[str, str]:
        """从文本中提取操作类型和时区名称"""
        result = {
            "操作": "",
            "时区名称": ""
        }

        # 1. 提取操作类型
        for pattern, operation in operation_rules:
            if re.search(pattern, text, re.IGNORECASE):
                result["操作"] = operation
                break

        for pattern, replacement in timezone_keywords:  # 使用类属性中的正则规则
            # 忽略大小写匹配文本
            match = re.search(pattern, text, re.IGNORECASE)
            if match:
                # 用替换规则提取时区名称（处理分组引用，如\2）
                timezone_name = match.expand(replacement).strip()
                # 确保提取到有效内容（非空）
                if timezone_name:
                    result["时区名称"] = timezone_name
                    break  # 匹配到第一个则停止

        return result

    def extract_music_info(self, text: str, known_singers) -> Dict[str, str]:
        """从文本中提取歌手和歌名信息"""
        result = {
            "歌手": "",  # 非必填，默认空字符串
            "歌名": ""   # 非必填，默认空字符串
        }

        # 1. 提取歌名（优先匹配带书名号的歌曲）
        song_match = re.search(r'《(.*?)》', text, re.IGNORECASE)
        if song_match:
            result["歌名"] = song_match.group(1).strip()

        # 2. 提取歌手（匹配已知歌手列表）
        for singer in known_singers:
            # 构建精确匹配正则（避免部分匹配，如"周杰伦"不会匹配"周杰伦123"）
            pattern = re.compile(rf'\b{singer}\b', re.IGNORECASE)
            if pattern.search(text):
                result["歌手"] = singer
                break

        return result

    def extract_video_name(self, text: str, video_paly_rules) -> Dict[str, str]:
        """从文本中提取要播放的视频名称"""
        result = {
            "名称": ""  # 非必填，默认空字符串
        }

        # 1. 优先匹配带书名号的视频名称（最常见格式）
        name_match = re.search(r'《(.*?)》', text, re.IGNORECASE)
        if name_match:
            result["名称"] = name_match.group(1).strip()
            return result

        return result

    def extract_touchpad_pointer_speed(self, text: str, operation_rules) -> Dict[str, str]:
        """从文本中提取触控板指针速度的操作类型（加快/减慢/具体数值）"""
        result = {
            "操作": ""
        }

        # 遍历规则，按优先级匹配（文字指令 > 倍数 > 纯数值）
        for pattern, handler in operation_rules:
            # 对当前规则的pattern进行匹配
            match = re.search(pattern, text, re.IGNORECASE)
            if match:
                try:
                    operation_value = handler(match)
                except TypeError:
                    # 容错：若 handler 意外为字符串（兼容旧格式）
                    operation_value = handler

                # 根据返回值类型格式化结果
                if isinstance(operation_value, str):
                    # 文字指令（如"加快"、"减慢"）
                    result["操作"] = operation_value
                elif isinstance(operation_value, tuple) and operation_value[0] == "倍数":
                    # 倍数（如('倍数', 2.0) → "倍数:2.0"）
                    result["操作"] = f"倍数:{operation_value[1]}"
                elif isinstance(operation_value, float):
                    # 纯数值（如2.5 → "2.5"，-1 → "-1.0"）
                    # 若为整数小数（如2.0），可简化为整数（如"2"）
                    result["操作"] = f"{operation_value:.0f}" if operation_value.is_integer(
                    ) else f"{operation_value}"

                # 匹配到后按优先级返回，不再继续匹配
                return result

        # 若未匹配到任何规则，返回空操作
        return result

    def extract_taskbar_auto_hide_patterns(self, text: str, operation_rules) -> Optional[Dict[str, str]]:
        """自动隐藏任务栏"""
        if not text or len(text.strip()) == 0:
            return {"操作": ""}

        # 第一步：过滤疑问句，直接返回空值
        if self.is_question(text):
            return {"操作":""}

        # 第二步：匹配非疑问句式的槽位规则
        for pattern, operation in operation_rules:
            if re.search(pattern, text, re.IGNORECASE):
                return {"操作": operation}

        # 无匹配句式，返回空值
        return {"操作": ""}

    # # 新增6个意图场景
    def extract_time_expressions(self, text):
        """从文本中提取时间表达式，返回原始文本中的时间字符串"""
        time_expressions = []

        # 1. 相对时间模式: "X分钟后/小时后/天后"
        relative_patterns = [
            r'(\d+)\s*(?:分钟|分|小时|时|天|日)(?:后|之后)',
            r'(?:明天|后天|今天|昨日|昨日|明天|大后天|上周|下周|本月|上月)\s*(?:的)?\s*(?:上午|下午|早上|晚上|凌晨)?\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?',
            r'(?:上午|下午|早上|晚上|凌晨)\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?'
        ]

        # 2. 绝对时间模式: "HH点MM分", "HH:MM", "HH时MM分"
        absolute_patterns = [
            r'(\d{1,2})\s*[点时:\.]\s*(\d{1,2})\s*分?',
            r'(\d{1,2})\s*[点时:\.]\s*(\d{1,2})\s*[:：]\s*(\d{1,2})',
            r'(?:上午|下午|早上|晚上|凌晨)?\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?',
            r'(\d{1,2})[:：](\d{1,2})(?:[:：](\d{1,2}))?',
            r'(\d{1,2})\s*点\s*(\d{1,2})?\s*分',
            r'(\d{1,2})\s*时\s*(\d{1,2})?\s*分'
        ]

        # 3. 日期+时间模式
        datetime_patterns = [
            r'(?:\d{4}年)?\s*\d{1,2}月\s*\d{1,2}日\s*(?:上午|下午|早上|晚上|凌晨)?\s*\d{1,2}\s*[点时:\.]\s*\d{1,2}',
            r'(?:周一|周二|周三|周四|周五|周六|周日|星期一|星期二|星期三|星期四|星期五|星期六|星期日)\s*(?:上午|下午|早上|晚上|凌晨)?\s*\d{1,2}\s*[点时:\.]\s*\d{1,2}'
        ]

        # 4. 其他常见时间表达
        other_patterns = [
            r'午夜(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'凌晨(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'早上(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'上午(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'中午(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'下午(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'晚上(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?',
            r'深夜(?:\s*(\d{1,2})\s*[点时]\s*(\d{1,2})?\s*分?)?'
        ]

        all_patterns = relative_patterns + absolute_patterns + \
            datetime_patterns + other_patterns

        for pattern in all_patterns:
            matches = re.finditer(pattern, text)
            for match in matches:
                # 获取完整的匹配文本
                matched_text = match.group(0).strip()
                # 避免重复添加
                if matched_text and matched_text not in time_expressions:
                    time_expressions.append(matched_text)

        return time_expressions

    def clean_subject(self, text):
        """
        清理主题文本，移除开头和结尾的提醒关键词及标点符号
        """
        # 移除开头的提醒关键词
        text = re.sub(
            r'^[\s，。、,、；;]*(?:提醒(?:我)?|记得|别忘了|在|于)[\s，。、,、；;]*', '', text)
        # 移除结尾的提醒关键词
        text = re.sub(
            r'[\s，。、,、；;]*(?:提醒(?:我)?|记得|别忘了|在|于)[\s，。、,、；;]*$', '', text)
        # 移除首尾标点符号和空格
        text = text.strip(' ，。、,;；:（）()')
        return text

    def extract_time_and_subject(self, text):
        """
        闹钟设置 （设置闹铃和提醒）
        从文本中提取时间表达式和提醒主题
        返回格式: (时间表达式, 主题)
        """
        result = {
            "日期": "",
            "事件": "",
            "操作": ""
        }

        # 先提取时间表达式
        time_expressions = self.extract_time_expressions(text)

        if time_expressions:
            # 取第一个匹配到的时间表达式
            time_expr = time_expressions[0]
            result['日期'] = time_expr

            # 从文本中移除第一个出现的时间表达式
            text_without_time = re.sub(re.escape(time_expr), '', text, count=1)
            # 清理主题文本
            subject = self.clean_subject(text_without_time)
        else:
            # 没有时间表达式，直接清理整个文本
            subject = self.clean_subject(text)

        if subject:
            result['事件'] = subject
            result['操作'] = "添加"

        return result

    def extract_shutdown_commands(self, text, shutdown_patterns, restart_patterns):
        """
        电源控制：从文本中提取重启、关机等系统命令
        返回格式: {'操作': "重启" / "关机"}
        """
        result = {'操作': ""}

        normalized_text = re.sub(r'\s+', ' ', text).strip().lower()
        all_patterns = {
            '关机': shutdown_patterns,
            '重启': restart_patterns,
        }

        # 查找匹配的命令类型
        for cmd_type, patterns in all_patterns.items():
            for pattern in patterns:
                if re.search(pattern, normalized_text):
                    result['操作'] = cmd_type
                    break
            if result['操作']:
                break
        return result

    def _strip_action_prefix_from_location(self, location, action_prefixes):
        """去除地点前误匹配的动词前缀，如'查询北京'->'北京'。前缀按长度降序排列以优先匹配最长。"""
        if not location:
            return location
        for prefix in action_prefixes:
            if prefix and location.startswith(prefix):
                stripped = location[len(prefix):]
                if stripped and len(stripped) >= 2:  # 至少保留2个字符作为地名
                    return stripped
        return location

    def extract_cities_name(self, text):
        """从天气 query 中提取单个地点名称，不依赖第三方分词工具和城市列表。"""
        result = {"地点": ""}
        if not text or not text.strip():
            return result

        _HAN = r'[\u4e00-\u9fa5]'
        _VERB = r'(?:查询|查看|了解|知道|告诉我|帮我查)(?:一下|下)?'
        _WEATHER = r'(?:天气|气温|温度|预报)'
        _WEATHER_OR_HOW = r'(?:天气|气温|温度|预报|怎么样|如何)'
        _SUFFIX = r'(?:市|县|区|省|自治区|特别行政区|州|镇|村|乡|街道|街道办)'

        non_location = {
            '天气', '气温', '温度', '预报', '气象', '查询', '查看', '了解', '知道',
            '的', '是', '有', '在', '到', '去', '来', '看', '查', '问', '要', '想',
            '能', '会', '可以', '今天', '明天', '后天', '昨天', '现在', '当前',
        }
        action_prefixes = [
            '帮我查一下', '我查一下', '帮我查', '告诉我', '查一下', '查询', '查看',
            '了解', '知道', '帮一下', '帮查', '我查', '告诉', '帮',
        ]
        weather_exclude = {
            '天气', '气温', '温度', '预报', '气象', '下雨', '下雪', '降雨', '降雪',
            '晴天', '阴天', '多云', '雾', '霾', '风', '雨', '雪',
        }
        multi_sep = ('和', '与', '、')

        # 所有抽取规则：(正则, 捕获组下标, 可选：命中后需排除的集合)
        city_patterns = [
            (rf'({_HAN}{{2,6}})(?:和|与|、){_HAN}{{2,4}}(?:的)?{_WEATHER}', 1, None),  # A和B的天气 -> A
            (rf'{_VERB}({_HAN}{{2,6}})的{_WEATHER}', 1, None),
            (rf'{_VERB}({_HAN}{{2,6}}){_WEATHER}(?!的)', 1, None),
            (rf'({_HAN}{{1,6}}){_SUFFIX}(?:的)?{_WEATHER_OR_HOW}', 1, None),
            (rf'{_VERB}({_HAN}{{1,6}}){_SUFFIX}(?:的)?{_WEATHER}?', 1, None),
            (rf'({_HAN}{{1,6}}){_SUFFIX}', 1, None),
            (rf'({_HAN}{{2,6}})的{_WEATHER}', 1, None),
            (rf'({_HAN}{{2,6}}){_WEATHER}', 1, weather_exclude),  # XX天气，排除天气词本身
            (rf'{_VERB}({_HAN}{{2,6}})(?:现在|今天|明天|后天|昨天)?(?:的)?(?:天气|气温|温度)(?:怎么样|如何|多少)', 1, None),
            (rf'({_HAN}{{2,6}})(?:现在|今天|明天|后天|昨天)?(?:的)?(?:天气|气温|温度)(?:怎么样|如何|多少)', 1, None),
            (rf'({_HAN}{{2,6}})(?:热不热|冷不冷|下雨吗|下雪吗)', 1, None),
        ]

        text = re.sub(r'\s+', '', text.strip())
        cities = []

        def normalize_city(loc):
            if not loc:
                return None
            if loc.endswith('的'):
                loc = loc[:-1]
            loc = self._strip_action_prefix_from_location(loc, action_prefixes)
            if not loc or len(loc) < 2 or len(loc) > 6:
                return None
            if loc in non_location or any(s in loc for s in multi_sep):
                return None
            return loc

        for pattern, group_idx, exclude in city_patterns:
            for m in re.finditer(pattern, text):
                raw = m.group(group_idx)
                if exclude and raw in exclude:
                    continue
                city = normalize_city(raw)
                if city and city not in cities:
                    cities.append(city)

        if cities:
            result["地点"] = cities[0]
        return result

    def extract_battery_display(self, text: str, action_keywords, target_keywords, location_keywords) -> Dict[str, any]:
        """
        显示充放电时间
        从文本中提取系统控制-显示充放电时间类的意图和操作
        """
        # 初始化结果
        result = {
            "操作": "",
        }

        text = text.strip()

        # 构建正则表达式模式
        show_pattern = "|".join(action_keywords["show"])
        hide_pattern = "|".join(action_keywords["hide"])

        target_pattern = "|".join(target_keywords)
        location_pattern = "|".join(location_keywords)

        # 特殊模式：处理"关闭...显示"这类表达
        special_hide_pattern = re.compile(
            r"关闭.*?显示|隐藏.*?显示|取消.*?显示|禁用.*?显示",
            re.IGNORECASE
        )

        # 特殊模式：处理"显示..."这类表达
        special_show_pattern = re.compile(
            r"显示.*?时间|查看.*?时间|开启.*?时间|打开.*?时间",
            re.IGNORECASE
        )

        # 检查特殊模式
        if special_hide_pattern.search(text):
            result["操作"] = "关闭"  # hide
        elif special_show_pattern.search(text):
            result["操作"] = "打开"  # show

        # 构建完整匹配模式
        patterns = [
            # 模式1: 操作 + 目标 + 位置
            re.compile(
                rf"({show_pattern}|{hide_pattern}).*?({target_pattern})(?:.*?({location_pattern}))?", re.IGNORECASE),
            # 模式2: 目标 + 操作
            re.compile(
                rf"({target_pattern}).*?({show_pattern}|{hide_pattern})", re.IGNORECASE),
            # 模式3: 仅操作 + 目标
            re.compile(
                rf"({show_pattern}|{hide_pattern}).*?({target_pattern})", re.IGNORECASE),
            # 模式4: 仅目标
            re.compile(rf"({target_pattern})", re.IGNORECASE)
        ]

        # 尝试匹配各模式
        for pattern in patterns:
            match = pattern.search(text)
            if match:
                groups = match.groups()

                # 处理模式1和模式3: 操作+目标
                if len(groups) >= 2 and groups[0] and groups[1]:
                    action_text = groups[0]
                    # target_text = groups[1]

                    # 确定操作类型
                    if any(word in action_text for word in action_keywords["hide"]):
                        result["操作"] = "关闭"
                    elif any(word in action_text for word in action_keywords["show"]):
                        result["操作"] = "打开"

                # 处理模式2: 目标+操作
                elif len(groups) >= 2 and groups[0] and groups[1]:
                    # target_text = groups[0]
                    action_text = groups[1]

                    if any(word in action_text for word in action_keywords["hide"]):
                        result["操作"] = "关闭"
                    elif any(word in action_text for word in action_keywords["show"]):
                        result["操作"] = "打开"

                break  # 找到匹配就停止

        # 如果文本中有"关闭"/"隐藏"但没有明确匹配到
        if any(word in text for word in action_keywords["hide"]) and result["操作"] == "打开":
            if re.search(r"关闭|隐藏|取消|禁用", text):
                # 再确认是否与充放电相关
                if re.search(r"(电池|充电|放电|电量)", text):
                    result["操作"] = "关闭"

        # 特殊处理: "关闭充放电时间显示"
        if "关闭" in text and "显示" in text and any(t in text for t in target_keywords):
            result["操作"] = "关闭"

        return result

    def _init_slot_func_and_patterns_dict(self):
        self.slot_func_and_patterns = {
            "设置鼠标双击间隔时长": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.mouse_double_click_patterns]
            },
            "设置显示器屏幕缩放": {
                "func": self.extract_zoom_level,
                "patterns": [self.slot_pattern_rules.screen_zoom_patterns]
            },
            "设置显示器亮度": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.screen_brightness_patterns]
            },
            "音量控制": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.volume_patterns]
            },
            "切换电源模式": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.power_mode_patterns]
            },
            "设置鼠标指针大小": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.mouse_pointer_size_patterns]
            },
            "激活屏保时锁定屏幕设置": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.screensaver_lock_patterns]
            },
            "合并任务栏上的图标": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.taskbar_icon_merge_patterns]
            },
            "设置任务栏在屏幕上的位置": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.taskbar_position_patterns]
            },
            "设置任务栏大小": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.taskbar_size_patterns]
            },
            "自动隐藏任务栏": {
                "func": self.extract_taskbar_auto_hide_patterns,
                "patterns": [self.slot_pattern_rules.taskbar_auto_hide_patterns]
            },
            "锁定任务栏": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.taskbar_lock_patterns]
            },
            "设置系统字体": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.system_font_size_patterns]
            },
            "音量增强启用": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.volume_boost_patterns]
            },
            "设置唤醒屏幕时需要密码": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.wakeup_password_patterns]
            },
            "设置鼠标指针速度": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.mouse_pointer_speed_patterns]
            },
            "设置鼠标滚轮速度": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.mouse_scroll_speed_patterns]
            },
            "鼠标加速启用": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.mouse_acceleration_patterns]
            },
            "VPN启用": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.vpn_enable_patterns]
            },
            "设置窗口外观": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.window_appearance_patterns]
            },
            "任务栏多任务视图启用": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.taskbar_multitask_patterns]
            },
            "设置显示器色温与护眼": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.display_color_temperature_patterns]
            },
            "设置鼠标主按钮": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.mouse_primary_button_patterns]
            },
            "多媒体-播放器控制": {
                "func": self.extract_player_controls,
                "patterns": {
                    "media_type": self.slot_pattern_rules.media_type_patterns,
                    "operations": self.slot_pattern_rules.player_operations
                }
            },
            "应用控制": {
                "func": self.extract_application_info,
                "patterns": {
                    "operations": self.slot_pattern_rules.app_operation_patterns,
                    "applications": self.slot_pattern_rules.common_applications
                }
            },
            "多模态输入识别": {
                "func": self.extract_multimodal_input_info,
                "patterns": {
                    "attachments": self.slot_pattern_rules.multimodal_input_attachment_rules,
                    "apis": self.slot_pattern_rules.multimodal_input_api_rules
                }
            },
            "闹钟设置": {
                "func": self.extract_alarm_info,
                "patterns": {
                    "operations": self.slot_pattern_rules.alarm_setting_operation_patterns,
                    "dates": self.slot_pattern_rules.date_patterns,
                    "events": self.slot_pattern_rules.event_keywords
                }
            },
            "日程查看&安排": {
                "func": self.extract_schedule_info,
                "patterns": {
                    "operations": self.slot_pattern_rules.schedule_operation_patterns,
                    "dates": self.slot_pattern_rules.date_patterns,
                    "events": self.slot_pattern_rules.event_keywords
                }
            },
            # 以下为低频意图 ********************************************************
            "自定义时区时间": {
                "func": self.extract_timezone_info,
                "patterns": {
                    "operations": self.slot_pattern_rules.timezone_operation_rules,
                    "time_zone_name": self.slot_pattern_rules.timezone_name_keywords,
                }
            },
            "切换显示器投屏方式": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.display_casting_mode_casting_mode_rules]
            },
            "设置免密登录": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.password_free_login_operation_rules]
            },
            "设置开机自动登录": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.auto_login_setting_operation_rules]
            },
            "多媒体-音乐播放": {
                "func": self.extract_music_info,
                "patterns": {
                    "singer_name": self.slot_pattern_rules.music_play_known_singers
                }
            },
            "多媒体-视频播放": {
                "func": self.extract_video_name,
                "patterns": [self.slot_pattern_rules.video_play_rules]
            },
            # 设备不支持意图场景 ********************************************************
            "蓝牙操作": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.bluetooth_operation_rules]
            },
            "设置侦听此设备": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.listening_device_operation_rules]
            },
            "设置鼠标光标闪烁频率": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.cursor_blink_rate_operation_rules]
            },
            "触控板点击功能启用": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.touchpad_click_operation_rules]
            },
            "滚动条跟随手指滚动设置": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.scrollbar_follow_finger_operation_rules]
            },
            "设置在文本区域闪烁光标": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.text_area_cursor_blink_operation_rules]
            },
            "插入鼠标时禁用触控板设置": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.touchpad_disable_on_mouse_operation_rules]
            },
            "设置触控板指针速度": {
                "func": self.extract_touchpad_pointer_speed,
                "patterns": [self.slot_pattern_rules.touchpad_pointer_speed_operation_rules]
            },
            "触控板启用": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.touchpad_enable_operation_rules]
            },
            "打字时禁用触控板设置": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.disable_touchpad_while_typing_operation_rules]
            },
            "侧边栏显示平板模式": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.sidebar_tablet_mode_operation_rules]
            },
            # # 新增6个意图场景
            "电源控制": {
                "func": self.extract_shutdown_commands,
                "patterns": [self.slot_pattern_rules.shutdown_patterns_rules,
                             self.slot_pattern_rules.restart_patterns_rules]
            },
            "自动调节显示器亮度": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.brightness_control_patterns_rules]
            },
            "低电量节能模式": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.low_power_battery_patterns_rules]
            },
            "显示充放电时间": {
                "func": self.extract_battery_display,
                "patterns": [self.slot_pattern_rules.battery_action_keywords,
                             self.slot_pattern_rules.battery_target_keywords,
                             self.slot_pattern_rules.battery_location_keywords]
            },
            "显示电池百分比": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.battery_percent_patterns_rules]
            },
            "录屏": {
                "func": self.extract_pattern_operation_value,
                "patterns": [self.slot_pattern_rules.screen_record_patterns_rules]
            },
            "查询天气": {
                "func": self.extract_cities_name,
                "patterns": []
            }
        }
