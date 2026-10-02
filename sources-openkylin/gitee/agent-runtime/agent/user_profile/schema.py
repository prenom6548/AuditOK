"""Profile taxonomy derived from user_profile.md."""

from dataclasses import dataclass
from typing import Literal

Lifecycle = Literal["static", "dynamic", "boundary"]


@dataclass(frozen=True)
class FieldDefinition:
    path: str
    description: str
    lifecycle: Lifecycle
    half_life_days: int | None = None


"""用户画像字段定义（修订版）。

设计原则：
1. 父级路径仅作为命名空间，不再同时保存父字段和子字段，避免重复与冲突。
2. FieldDefinition 的第三个参数继续兼容现有的 static / dynamic / boundary。
3. 信息的来源、置信度、作用域、状态和时间不编码进字段路径，统一由记录元数据承载。
4. 目标、需求、偏好、任务和权限尽量使用结构化条目，避免为每个类别无限增加字段。
5. 不默认建立性别、政治立场、宗教、收入、疾病诊断等敏感画像字段。
"""




# 对现有三类 FieldDefinition 的统一解释。
FIELD_TYPE_SEMANTICS = {
    "static": "相对稳定但可被新信息替换的事实、长期目标或长期需求；不是永久不变",
    "dynamic": "任务、近期兴趣或可能变化的偏好；应结合 TTL 和 last_seen_at 更新",
    "boundary": "用户授权、隐私或行动政策；默认持续到用户撤销，不使用普通 TTL 自动过期",
}


FIELDS = (
    # ============================================================
    # 一、相对稳定的用户事实与长期信息
    # ============================================================

    # ------------------------------------------------------------
    # 1. 基本身份与区域设置
    # ------------------------------------------------------------
    FieldDefinition(
        "basic.preferred_name",
        "用户希望被如何称呼，包括姓名、昵称或称谓；仅记录用户明确提供的信息",
        "static",
    ),
    FieldDefinition(
        "basic.age",
        "用户明确提供的年龄、年龄段或出生年份；不要根据职业、外貌或其他信息推断",
        "static",
    ),
    FieldDefinition(
        "basic.languages",
        (
            "用户掌握、常用或偏好的语言及熟练程度；条目应将语言、熟练程度和使用场景"
            "绑定在一起，避免与技能字段重复"
        ),
        "static",
    ),
    FieldDefinition(
        "basic.education",
        "用户明确提供的教育背景，包括学历、专业、学校类型和当前学习阶段",
        "static",
    ),
    FieldDefinition(
        "locale.timezone",
        "用户明确设置或由产品设置提供的 IANA 时区；不要仅凭国家、语言或城市长期推断",
        "static",
    ),
    FieldDefinition(
        "locale.date_time_format",
        "用户偏好的日期格式、日期顺序、12/24 小时制和星期起始日",
        "static",
    ),
    FieldDefinition(
        "locale.measurement_system",
        "用户偏好的公制或英制等单位体系，以及必要的领域例外",
        "static",
    ),
    FieldDefinition(
        "locale.currency",
        "用户明确偏好的默认货币；仅用于展示和换算，不据此推断收入或资产",
        "static",
    ),
    FieldDefinition(
        "locale.number_format",
        "用户偏好的数字、小数点、千位分隔符和百分比格式",
        "static",
    ),

    # ------------------------------------------------------------
    # 2. 职业与学习状态
    # ------------------------------------------------------------
    FieldDefinition(
        "occupation.status",
        (
            "用户当前主要职业或学习状态，例如在校、就业、自雇、创业、待业、退休、休假；"
            "允许多个同时存在的状态，每个状态应带有效时间"
        ),
        "static",
    ),
    FieldDefinition(
        "occupation.category",
        "用户明确提供或可直接归类的职业大类；不得用职业大类替代具体岗位",
        "static",
    ),
    FieldDefinition(
        "occupation.title",
        "用户明确提供的职位、岗位、职业名称或业务身份，例如程序员、高校教师、店主",
        "static",
    ),
    FieldDefinition(
        "occupation.industry",
        "用户所在行业、业务领域或组织类型",
        "static",
    ),
    FieldDefinition(
        "occupation.organization",
        "用户明确提供的学校、公司或组织；只在任务需要时记录，并遵守敏感组织信息边界",
        "static",
    ),
    FieldDefinition(
        "occupation.engagement",
        "用户的任职或学习投入形式，例如全职、兼职、合同制、实习或非全日制学习",
        "static",
    ),
    FieldDefinition(
        "occupation.location_mode",
        "用户的工作或学习地点模式，例如远程、线下或混合；不与全职、兼职混在一起",
        "static",
    ),
    FieldDefinition(
        "occupation.schedule",
        "用户相对稳定的工作、上课或经营时间安排；临时排班应进入任务上下文",
        "static",
    ),

    # ------------------------------------------------------------
    # 3. 设备、软件与实体工具
    # ------------------------------------------------------------
    FieldDefinition(
        "ecosystem.devices",
        "用户常用设备及其用途，例如电脑、手机、平板、手表和智能家居设备",
        "static",
    ),
    FieldDefinition(
        "ecosystem.operating_systems",
        "用户常用操作系统、版本和设备对应关系，例如 macOS、Windows、Linux、iOS、Android",
        "static",
    ),
    FieldDefinition(
        "ecosystem.smart_home",
        "用户使用的智能家居平台、协议或主要设备生态",
        "static",
    ),
    FieldDefinition(
        "ecosystem.physical_tools",
        "用户长期使用且与任务相关的实体工具，例如厨具、维修工具、乐器或摄影器材",
        "static",
    ),
    FieldDefinition(
        "ecosystem.software.communication",
        "用户常用的邮件、即时通信、会议或团队协作软件",
        "static",
    ),
    FieldDefinition(
        "ecosystem.software.productivity",
        "用户常用的笔记、任务、日历、文档、知识管理、文献管理和办公软件",
        "static",
    ),
    FieldDefinition(
        "ecosystem.software.development",
        "用户长期使用的编程语言、IDE、终端、版本管理、云平台和开发工具",
        "static",
    ),
    FieldDefinition(
        "ecosystem.software.creative",
        "用户长期使用的设计、音视频、摄影后期、建模或其他创作软件",
        "static",
    ),

    # ------------------------------------------------------------
    # 4. 关系、家庭与照护责任
    # 父路径 life.relations 仅作为命名空间，不单独存储。
    # ------------------------------------------------------------
    FieldDefinition(
        "life.relations.family",
        (
            "用户明确提供且与需求相关的家庭成员或伴侣关系；只记录必要事实，"
            "不得根据同住、子女或称谓推断婚姻状态"
        ),
        "static",
    ),
    FieldDefinition(
        "life.relations.friends",
        "用户明确提供且与任务相关的朋友关系；避免保存无关的第三方个人信息",
        "static",
    ),
    FieldDefinition(
        "life.relations.work",
        "用户明确提供且与任务相关的同事、上级、下属、客户或合作伙伴关系",
        "static",
    ),
    FieldDefinition(
        "life.relations.community",
        "用户明确提供且与任务相关的邻居、室友、社群成员或其他社区关系",
        "static",
    ),
    FieldDefinition(
        "life.relations.pets",
        "用户饲养的宠物，包括种类、名字、年龄和相关照护需求",
        "static",
    ),
    FieldDefinition(
        "life.care_responsibilities",
        "用户长期承担的育儿、赡养、宠物照护或其他照护责任",
        "static",
    ),

    # ------------------------------------------------------------
    # 5. 居住环境
    # ------------------------------------------------------------
    FieldDefinition(
        "life.residence.country_region",
        "用户当前长期居住或主要生活的国家、地区；与家乡、国籍和临时旅行地点区分",
        "static",
    ),
    FieldDefinition(
        "life.residence.city",
        "用户当前长期居住或主要生活的城市；临时所在城市进入任务上下文",
        "static",
    ),
    FieldDefinition(
        "life.residence.area",
        (
            "用户居住位置的必要概况，例如城区或生活圈；默认不记录精确门牌、坐标、"
            "实时位置等高敏感信息"
        ),
        "static",
    ),
    FieldDefinition(
        "life.residence.housing",
        "用户住房情况，例如租住、自有、宿舍、独居或合住，以及与需求相关的房屋类型",
        "static",
    ),
    FieldDefinition(
        "life.residence.household",
        "用户明确提供的同住构成和家庭规模；不要由此反推婚姻或亲属关系",
        "static",
    ),
    FieldDefinition(
        "life.residence.facilities",
        "用户居住地周边经常使用且与需求有关的设施，例如超市、医院、公园和公共交通",
        "static",
    ),
    FieldDefinition(
        "life.residence.environment",
        "与用户需求有关的居住环境特征，例如噪音、气候、通勤便利性或无障碍条件",
        "static",
    ),

    # ------------------------------------------------------------
    # 6. 生活方式
    # ------------------------------------------------------------
    FieldDefinition(
        "life.lifestyle.diet",
        (
            "用户相对稳定的口味、饮食结构、忌口、过敏、素食或宗教饮食要求；"
            "健康和宗教相关信息应按敏感信息处理"
        ),
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.sleep",
        "用户相对稳定的作息、睡眠时间和晨型或夜型倾向；不据此推断疾病",
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.free_time",
        "用户通常可自由安排的时间段，以及工作日和周末的空闲规律",
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.transport",
        "用户一般出行时常用或偏好的交通方式",
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.commute",
        "用户固定通勤的方式、时长、路线类型和约束；与一般出行偏好区分",
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.spending",
        "用户明确表达的预算倾向、价格敏感度和消费决策偏好；不得推断收入、资产或负债",
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.exercise",
        "用户长期运动习惯、频率和常见运动类型；不据此推断健康诊断",
        "static",
    ),
    FieldDefinition(
        "life.lifestyle.routines",
        "用户相对稳定的日常习惯、固定流程或生活仪式",
        "static",
    ),

    # ------------------------------------------------------------
    # 7. 知识、技能与经验
    # ------------------------------------------------------------
    FieldDefinition(
        "background.domains",
        "用户熟悉、从事或系统学习过的专业领域和知识领域，表示用户了解什么",
        "static",
    ),
    FieldDefinition(
        "background.skills",
        (
            "用户明确掌握的可执行技能，表示用户能做什么；每个技能条目应同时保存名称、"
            "熟练程度、使用场景和证据"
        ),
        "static",
    ),
    FieldDefinition(
        "background.experience",
        "用户相关工作、学习、项目、创业或生活经历，表示用户做过什么",
        "static",
    ),
    FieldDefinition(
        "background.certifications",
        "用户明确提供的证书、资质、执照或职业认证及有效期",
        "static",
    ),
    FieldDefinition(
        "background.learning_style",
        "用户明确表达或多次稳定表现出的学习方式，例如实践型、阅读型或课程型",
        "static",
    ),

    # ------------------------------------------------------------
    # 8. 人格与决策倾向
    # ------------------------------------------------------------
    FieldDefinition(
        "personality.traits",
        (
            "用户明确自述或在长期、多次互动中稳定表现出的性格特征；推断内容必须标记"
            "source=inferred 或 observed_repeated，且不得依据单次对话作心理判断"
        ),
        "static",
    ),
    FieldDefinition(
        "personality.risk_preference",
        (
            "用户在一般决策、尝试新方案或职业选择中的风险偏好；必须保留适用领域，"
            "不得自动泛化至投资、医疗或其他高风险决策"
        ),
        "static",
    ),
    FieldDefinition(
        "personality.decision_style",
        "用户相对稳定的决策方式，例如数据驱动、直觉驱动、谨慎比较或快速试错",
        "static",
    ),
    FieldDefinition(
        "personality.social_preference",
        "用户明确表达的独处、社交、协作或沟通倾向，并记录适用场景",
        "static",
    ),

    # ------------------------------------------------------------
    # 9. 长期目标与长期需求
    # 使用结构化条目，不同时保存总字段和领域子字段。
    # ------------------------------------------------------------
    FieldDefinition(
        "goals.items",
        (
            "用户明确提出的目标列表；"
            "目标表示用户希望达到的结果"
        ),
        "static",
    ),
    FieldDefinition(
        "needs.items",
        (
            "用户长期、反复需要的支持列表；"
            "可覆盖信息、娱乐、学习、效率、节省开支或无障碍；需求表示助手反复提供什么支持"
        ),
        "static",
    ),

    # ============================================================
    # 二、动态信息：偏好、兴趣与近期任务
    # ============================================================

    # ------------------------------------------------------------
    # 10. 个性化偏好
    # 正向和负向偏好采用同一结构，避免 format/formats 等重复字段。
    # ------------------------------------------------------------
    FieldDefinition(
        "preferences.items",
        (
            "用户的个性化偏好条目；每项包含场景和偏好；"
            "场景偏好必须设置范围，不能无条件覆盖全局偏好；用户可能存在多条偏好，及时更新"
        ),
        "dynamic",
        180,
    ),

    # ------------------------------------------------------------
    # 11. 兴趣
    # 父路径仅作为命名空间。
    # ------------------------------------------------------------
    FieldDefinition(
        "interests.long_term.topics",
        "用户长期、稳定或反复表现出的兴趣主题，例如 AI、足球、摄影、历史或音乐",
        "dynamic",
        365,
    ),
    FieldDefinition(
        "interests.long_term.activities",
        "用户长期喜欢参与的活动、爱好或社群",
        "dynamic",
        365,
    ),
    FieldDefinition(
        "interests.long_term.content",
        "用户长期偏好的内容类型、题材、创作者、媒体或信息来源",
        "dynamic",
        365,
    ),
    FieldDefinition(
        "interests.short_term.topics",
        "用户近期明确关注、研究或准备参与的主题",
        "dynamic",
        30,
    ),
    FieldDefinition(
        "interests.short_term.travel",
        "用户近期关注、计划前往或正在比较的旅行目的地和旅行主题",
        "dynamic",
        30,
    ),
    FieldDefinition(
        "interests.short_term.products",
        "用户近期正在研究、比较或考虑购买的产品与服务类别",
        "dynamic",
        30,
    ),
    FieldDefinition(
        "interests.short_term.events",
        "用户近期准备参加、关注或考虑参加的活动、比赛、展览或会议",
        "dynamic",
        30,
    ),

    # ------------------------------------------------------------
    # 12. 项目、任务与决策上下文
    # 所有条目应关联 project_id 或 task_id，避免多个任务之间串线。
    # ------------------------------------------------------------
    FieldDefinition(
        "context.projects",
        "用户近期活跃项目的名称、目的、状态和关联任务；项目结束后可归档而非立即丢弃",
        "dynamic",
        180,
    ),
    FieldDefinition(
        "context.tasks",
        (
            "用户当前或近期任务列表；"
        ),
        "dynamic",
        30,
    ),
    FieldDefinition(
        "context.completed_actions",
        "用户已经完成的行动和结果；每项必须阐明具体任务。",
        "dynamic",
        90,
    ),
    FieldDefinition(
        "context.attempts",
        (
            "用户为解决问题尝试过的方法、结果和反馈；成功、失败或部分有效由 outcome 标记，"
        ),
        "dynamic",
        90,
    ),
    FieldDefinition(
        "context.decisions",
        (
            "用户在项目或任务中明确选择、确认、批准或拒绝的方案及原因；"
        ),
        "dynamic",
        90,
    ),
    FieldDefinition(
        "context.constraints",
        "当前或近期任务的预算、时间、设备、权限、地点和人员约束；",
        "dynamic",
        90,
    ),
    FieldDefinition(
        "context.available_resources",
        "用户在当前或近期任务中拥有或可使用的材料、数据、工具和人员资源；",
        "dynamic",
        90,
    ),
    FieldDefinition(
        "context.pending_questions",
        "任务尚未解决、需要补充信息或等待确认的问题；",
        "dynamic",
        30,
    ),
    FieldDefinition(
        "context.pending_commitments",
        "对话中已经承诺但尚未完成的用户行动或助手事项；每项包含负责人、截止时间和具体任务",
        "dynamic",
        30,
    ),

    # ============================================================
    # 三、持续生效的个性化、隐私与行动边界
    # ============================================================

    FieldDefinition(
        "boundaries.retention_policies",
        (
            "用户对不同信息类别的保存政策；每项包含类型和策略，策略为拒绝、允许"
        ),
        "boundary",
    ),
    FieldDefinition(
        "boundaries.inference_policies",
        (
            "用户对不同信息类别的推断政策；每项包含类型和策略，策略为拒绝、允许"
        ),
        "boundary",
    ),
    FieldDefinition(
        "boundaries.proactive_policies",
        (
            "用户对主动建议、提醒、推荐、跟踪或自动推进的领域级政策；"
            "每项包含类型和策略，策略为拒绝、允许"
        ),
        "boundary",
    ),
    FieldDefinition(
        "boundaries.content_policies",
        (
            "用户对话题、表达方式或内容类型的限制；"
            "用于表达绝对禁忌、避免主动提及或需要谨慎处理的内容"
        ),
        "boundary",
    ),
    FieldDefinition(
        "boundaries.location_privacy",
        "用户对精确位置、家庭地址、行程和实时位置的记录、推断、披露及使用限制",
        "boundary",
    ),
    FieldDefinition(
        "boundaries.third_party_privacy",
        "用户对亲友、同事等第三方信息的记录、推断、披露和使用限制",
        "boundary",
    ),
    FieldDefinition(
        "boundaries.sensitive_topics",
        "用户对健康、财务、政治、宗教、法律等敏感领域的个性化处理要求",
        "boundary",
    ),
)


# Schema v1 profiles may still contain these paths.  Keep them readable so a
# field-list revision cannot make the complete encrypted document unloadable.
# They are deliberately excluded from ``FIELDS`` / ``schema_for_prompt()`` so
# the extractor only writes the current schema.
LEGACY_FIELDS = (
    FieldDefinition("basic.city", "所在城市（旧版字段）", "static"),
    FieldDefinition("basic.country_region", "国家或地区（旧版字段）", "static"),
    FieldDefinition("basic.marital_status", "婚姻状态（旧版字段）", "static"),
    FieldDefinition("boundaries.confirm_each_time", "每次都需要确认的内容（旧版字段）", "boundary"),
    FieldDefinition("boundaries.destructive_or_payment_confirmation", "付款或删除数据等操作的确认要求（旧版字段）", "boundary"),
    FieldDefinition("boundaries.do_not_infer", "不希望被推断的信息（旧版字段）", "boundary"),
    FieldDefinition("boundaries.do_not_record", "不希望记录的信息（旧版字段）", "boundary"),
    FieldDefinition("boundaries.forbidden_actions", "绝不能自动执行的操作（旧版字段）", "boundary"),
    FieldDefinition("boundaries.long_term_allowed", "允许长期保存的信息（旧版字段）", "boundary"),
    FieldDefinition("boundaries.no_proactive_recommendations", "不接受主动推荐的领域（旧版字段）", "boundary"),
    FieldDefinition("boundaries.session_only", "只能在本次会话使用的信息（旧版字段）", "boundary"),
    FieldDefinition("context.current_task", "当前任务（旧版字段）", "dynamic", 7),
    FieldDefinition("context.last_decision", "最近决策（旧版字段）", "dynamic", 30),
    FieldDefinition("context.rejected_options", "已排除方案（旧版字段）", "dynamic", 30),
    FieldDefinition("context.tried_methods", "已尝试方法（旧版字段）", "dynamic", 30),
    FieldDefinition("ecosystem.tools", "常用工具链（旧版字段）", "static"),
    FieldDefinition("goals.long_term", "长期目标及其子目标（旧版字段）", "static"),
    FieldDefinition("interests.long_term", "长期兴趣（旧版字段）", "dynamic", 365),
    FieldDefinition("interests.short_term", "近期关注或计划（旧版字段）", "dynamic", 30),
    FieldDefinition("life.relations", "亲友、同事、邻居或宠物关系（旧版字段）", "static"),
    FieldDefinition("life.residence.location", "居住位置（旧版字段）", "static"),
    FieldDefinition("needs.long_term", "长期需求（旧版字段）", "static"),
    FieldDefinition("preferences.negative.dislikes", "不喜欢的内容（旧版字段）", "dynamic", 180),
    FieldDefinition("preferences.negative.formats", "不喜欢的回答格式（旧版字段）", "dynamic", 180),
    FieldDefinition("preferences.negative.taboos", "绝对禁忌（旧版字段）", "boundary"),
    FieldDefinition("preferences.positive.format", "回答格式偏好（旧版字段）", "dynamic", 180),
    FieldDefinition("preferences.positive.proactivity", "主动性偏好（旧版字段）", "dynamic", 180),
    FieldDefinition("preferences.positive.tone", "回答语气偏好（旧版字段）", "dynamic", 180),
    FieldDefinition("preferences.positive.verbosity", "回答详略偏好（旧版字段）", "dynamic", 180),
)

CURRENT_FIELD_BY_PATH = {field.path: field for field in FIELDS}
LEGACY_FIELD_BY_PATH = {field.path: field for field in LEGACY_FIELDS}
FIELD_BY_PATH = {**LEGACY_FIELD_BY_PATH, **CURRENT_FIELD_BY_PATH}
ALLOWED_PATHS = frozenset(CURRENT_FIELD_BY_PATH)
STORED_PATHS = frozenset(FIELD_BY_PATH)


def schema_for_prompt() -> str:
    return "\n".join(
        f"- {field.path} [{field.lifecycle}]: {field.description}" for field in FIELDS
    )
