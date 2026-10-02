"""
财务智能助手 - 智能记账模块
支持自然语言记账、自动分类、预算管理、月度分析
"""

import re
from datetime import datetime
from typing import List, Dict, Optional

# 分类规则
CATEGORY_RULES = {
    "餐饮": ["吃饭", "午餐", "晚餐", "早餐", "外卖", "咖啡", "奶茶", "食堂", "餐厅", "餐饮"],
    "交通": ["打车", "滴滴", "公交", "地铁", "高铁", "火车", "飞机", "机票", "油费", "停车", "过路费", "出租车"],
    "购物": ["购物", "超市", "网购", "淘宝", "京东", "天猫", "衣服", "鞋子", "包"],
    "住房": ["房租", "房贷", "物业", "水电", "燃气", "住房"],
    "通讯": ["电话", "手机", "话费", "网络", "宽带"],
    "医疗": ["医院", "药店", "医疗", "看病", "买药", "医保"],
    "教育": ["学费", "培训", "课程", "书籍", "教育", "文具"],
    "娱乐": ["电影", "游戏", "旅游", "健身", "娱乐", "KTV", "酒吧"],
    "社交": ["请客", "送礼", "红包", "人情", "招待"],
    "办公": ["办公", "文具", "打印", "耗材", "设备"],
    "收入": ["工资", "奖金", "报销", "退款", "返现", "投资收益", "兼职"],
    "转账": ["转账", "提现", "还款"],
}

# 支出关键词
EXPENSE_KEYWORDS = ["花", "买", "消费", "支出", "付", "用", "扣", "支"]

# 收入关键词
INCOME_KEYWORDS = ["收入", "到账", "收到", "入账", "奖励", "返现", "退款"]


def parse_natural_language(text: str) -> dict:
    """
    解析自然语言记账输入
    
    Args:
        text: 用户输入的记账描述
        
    Returns:
        dict: 解析结果
    """
    result = {
        "type": "expense",  # expense or income
        "amount": 0,
        "category": "其他",
        "description": text,
        "date": datetime.now().strftime("%Y-%m-%d")
    }
    
    # 判断收支类型
    for keyword in INCOME_KEYWORDS:
        if keyword in text:
            result["type"] = "income"
            break
    
    # 提取金额
    amount_patterns = [
        r'([\d,]+)元',  # 1580元
        r'花了?([\d,]+)',  # 花了1580
        r'([\d,]+)\s*元',  # 1580 元
        r'¥?\s*([\d,]+)',  # ¥1580 或 1580
        r'收到?\s*([\d,]+)',  # 收到500
    ]
    
    for pattern in amount_patterns:
        match = re.search(pattern, text)
        if match:
            amount_str = match.group(1).replace(',', '')
            result["amount"] = float(amount_str)
            break
    
    # 智能分类
    result["category"] = classify_text(text)
    
    # 提取日期（如果有）
    date_pattern = r'(\d{1,2})月(\d{1,2})日'
    date_match = re.search(date_pattern, text)
    if date_match:
        month, day = date_match.groups()
        result["date"] = f"{datetime.now().year}-{int(month):02d}-{int(day):02d}"
    
    return result


def classify_text(text: str) -> str:
    """
    根据文本内容自动分类
    
    Args:
        text: 文本描述
        
    Returns:
        str: 分类名称
    """
    for category, keywords in CATEGORY_RULES.items():
        for keyword in keywords:
            if keyword in text:
                return category
    return "其他"


class BudgetManager:
    """
    预算管理器
    """
    
    def __init__(self):
        self.budgets = {}
        self.spending = {}
    
    def set_budget(self, category: str, amount: float):
        """设置预算"""
        self.budgets[category] = amount
        if category not in self.spending:
            self.spending[category] = 0
    
    def add_spending(self, category: str, amount: float):
        """添加支出"""
        if category not in self.spending:
            self.spending[category] = 0
        self.spending[category] += amount
    
    def get_budget_status(self, category: str) -> dict:
        """获取预算状态"""
        budget = self.budgets.get(category, 0)
        spent = self.spending.get(category, 0)
        remaining = budget - spent
        percentage = (spent / budget * 100) if budget > 0 else 0
        
        status = "normal"
        if percentage >= 100:
            status = "over"
        elif percentage >= 80:
            status = "warning"
        
        return {
            "category": category,
            "budget": budget,
            "spent": spent,
            "remaining": remaining,
            "percentage": percentage,
            "status": status
        }
    
    def get_all_status(self) -> List[dict]:
        """获取所有预算状态"""
        statuses = []
        for category in self.budgets:
            statuses.append(self.get_budget_status(category))
        return statuses


def generate_daily_report(transactions: List[dict]) -> str:
    """
    生成日报
    
    Args:
        transactions: 交易列表
        
    Returns:
        str: 日报内容
    """
    if not transactions:
        return "今日暂无记账记录"
    
    report = ["## 今日记账汇总\n"]
    
    # 列出所有交易
    report.append("### 交易明细")
    report.append("| 时间 | 摘要 | 分类 | 金额 | 类型 |")
    report.append("|------|------|------|------|------|")
    
    total_income = 0
    total_expense = 0
    
    for t in transactions:
        amount_str = f"{t['amount']:.2f}"
        sign = "+" if t["type"] == "income" else "-"
        if t["type"] == "income":
            total_income += t["amount"]
        else:
            total_expense += t["amount"]
        
        report.append(f"| {t.get('date', '-')} | {t.get('description', '-')} | {t['category']} | {sign}{amount_str} | {'收入' if t['type'] == 'income' else '支出'} |")
    
    # 汇总
    report.append("\n### 收支汇总")
    report.append(f"| 类型 | 金额 |")
    report.append(f"|------|------|")
    report.append(f"| 收入 | +{total_income:.2f} |")
    report.append(f"| 支出 | -{total_expense:.2f} |")
    report.append(f"| **净收入** | **{total_income - total_expense:.2f}** |")
    
    return "\n".join(report)


def generate_monthly_analysis(transactions: List[dict], month: str = None) -> str:
    """
    生成月度分析报告
    
    Args:
        transactions: 交易列表
        month: 月份（格式：YYYY-MM）
        
    Returns:
        str: 月度分析报告
    """
    if not transactions:
        return "本月暂无记账记录"
    
    if month is None:
        month = datetime.now().strftime("%Y-%m")
    
    # 筛选当月数据
    month_transactions = [t for t in transactions if t.get("date", "").startswith(month)]
    
    report = [f"## {month} 月度分析报告\n"]
    
    # 收支概览
    income = sum(t["amount"] for t in month_transactions if t["type"] == "income")
    expense = sum(t["amount"] for t in month_transactions if t["type"] == "expense")
    
    report.append("### 收支概览")
    report.append(f"- 总收入：{income:,.2f} 元")
    report.append(f"- 总支出：{expense:,.2f} 元")
    report.append(f"- 净结余：{income - expense:,.2f} 元")
    report.append(f"- 储蓄率：{(income - expense) / income * 100:.1f}%" if income > 0 else "- 储蓄率：0%")
    
    # 支出分析
    report.append("\n### 支出分类分析")
    
    expense_by_category = {}
    for t in month_transactions:
        if t["type"] == "expense":
            cat = t["category"]
            expense_by_category[cat] = expense_by_category.get(cat, 0) + t["amount"]
    
    if expense_by_category:
        report.append("| 分类 | 金额 | 占比 |")
        report.append("|------|------|------|")
        
        sorted_categories = sorted(expense_by_category.items(), key=lambda x: x[1], reverse=True)
        for cat, amount in sorted_categories:
            percentage = amount / expense * 100
            report.append(f"| {cat} | {amount:,.2f} | {percentage:.1f}% |")
    
    # 趋势分析
    report.append("\n### 💡 建议")
    
    if expense_by_category:
        top_category = max(expense_by_category.items(), key=lambda x: x[1])
        report.append(f"- {top_category[0]}支出最高，达{top_category[1]:,.2f}元，可适当控制")
    
    if income > 0 and expense / income > 0.8:
        report.append("- ⚠️ 支出占收入比例偏高，建议增加储蓄")
    
    return "\n".join(report)


# 测试
if __name__ == "__main__":
    # 测试自然语言解析
    test_texts = [
        "今天买书花了58元",
        "中午请客户吃饭用了580元",
        "报销得来差旅费1200元已到账"
    ]
    
    for text in test_texts:
        result = parse_natural_language(text)
        print(f"输入: {text}")
        print(f"解析: {result}\n")
