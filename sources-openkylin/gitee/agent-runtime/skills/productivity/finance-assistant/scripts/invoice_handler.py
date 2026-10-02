"""
财务智能助手 - 发票处理模块
支持发票识别、合规检查、报销分组
"""

def parse_invoice(text: str) -> dict:
    """
    解析发票文本，提取关键字段
    
    Args:
        text: 用户输入的发票信息
        
    Returns:
        dict: 解析后的发票数据
    """
    import re
    
    result = {
        "type": "unknown",
        "code": None,
        "number": None,
        "date": None,
        "amount": 0,
        "tax": 0,
        "category": "其他",
        "raw_text": text
    }
    
    # 识别发票类型
    if "专票" in text or "增值税专用" in text:
        result["type"] = "增值税专用发票"
    elif "电子" in text:
        result["type"] = "电子发票"
    else:
        result["type"] = "增值税普通发票"
    
    # 识别金额
    amount_pattern = r'[金额为]?(\d+[\.,]?\d*)元'
    amounts = re.findall(amount_pattern, text)
    if amounts:
        result["amount"] = float(amounts[0].replace(',', ''))
    
    # 识别税额
    tax_pattern = r'税额[为是]?(\d+[\.,]?\d*)元'
    taxes = re.findall(tax_pattern, text)
    if taxes:
        result["tax"] = float(taxes[0].replace(',', ''))
    
    # 识别日期
    date_pattern = r'(\d{4}[年\-\/]\d{1,2}[月\-\/]\d{1,2}[日]?)'
    dates = re.findall(date_pattern, text)
    if dates:
        result["date"] = dates[0]
    
    # 识别费用分类
    categories = {
        "差旅费": ["出差", "住宿", "酒店", "差旅", "机票", "火车票", "高铁"],
        "交通费": ["交通", "打车", "出租车", "滴滴", "公交", "地铁", "油费", "停车"],
        "招待费": ["招待", "宴请", "客户", "吃饭", "餐饮"],
        "办公费": ["办公", "文具", "打印", "耗材"],
        "通讯费": ["通讯", "电话", "手机", "网络"],
        "交通费": ["高铁", "火车", "机票", "车票"],
    }
    
    for cat, keywords in categories.items():
        for kw in keywords:
            if kw in text:
                result["category"] = cat
                break
    
    return result


def check_compliance(invoice: dict) -> dict:
    """
    发票合规性检查
    
    Args:
        invoice: 发票数据
        
    Returns:
        dict: 检查结果
    """
    checks = []
    
    # 金额检查
    if invoice["amount"] > 0:
        checks.append({"item": "金额", "status": "pass", "message": "金额正确"})
    else:
        checks.append({"item": "金额", "status": "fail", "message": "金额异常"})
    
    # 税额计算检查
    if invoice["type"] == "增值税专用发票":
        expected_tax = invoice["amount"] * 0.09  # 假设9%税率
        if abs(invoice["tax"] - expected_tax) < 1:
            checks.append({"item": "税额", "status": "pass", "message": f"税额计算正确（{invoice['tax']}元）"})
        else:
            checks.append({"item": "税额", "status": "warn", "message": f"税额可能异常，预期约{expected_tax:.2f}元"})
    else:
        checks.append({"item": "税额", "status": "info", "message": "普票不涉及进项抵扣"})
    
    # 业务招待费限额检查
    if invoice["category"] == "招待费" and invoice["amount"] > 1000:
        checks.append({"item": "招待费限额", "status": "warn", "message": "业务招待费注意限额（不超过营业收入5‰）"})
    
    return {"checks": checks, "overall": "pass" if all(c["status"] != "fail" for c in checks) else "fail"}


def group_by_category(invoices: list) -> dict:
    """
    按费用类型分组汇总
    
    Args:
        invoices: 发票列表
        
    Returns:
        dict: 分组汇总结果
    """
    grouped = {}
    
    for inv in invoices:
        cat = inv.get("category", "其他")
        if cat not in grouped:
            grouped[cat] = {"count": 0, "amount": 0, "invoices": []}
        grouped[cat]["count"] += 1
        grouped[cat]["amount"] += inv["amount"]
        grouped[cat]["invoices"].append(inv)
    
    return grouped


def generate_expense_report(invoices: list) -> str:
    """
    生成报销报告
    
    Args:
        invoices: 发票列表
        
    Returns:
        str: 格式化报告
    """
    report = ["## 发票识别结果\n"]
    report.append("| 序号 | 日期 | 发票类型 | 金额(元) | 税额(元) | 分类 |")
    report.append("|------|------|----------|----------|----------|------|")
    
    total = 0
    for i, inv in enumerate(invoices, 1):
        report.append(f"| {i} | {inv.get('date', '-')} | {inv['type']} | {inv['amount']:.2f} | {inv['tax']:.2f} | {inv['category']} |")
        total += inv["amount"]
    
    report.append("\n## 报销汇总\n")
    grouped = group_by_category(invoices)
    
    report.append("| 费用类型 | 金额(元) |")
    report.append("|----------|----------|")
    
    for cat, data in grouped.items():
        report.append(f"| {cat} | {data['amount']:.2f} |")
    
    report.append(f"| **合计** | **{total:.2f}** |")
    
    return "\n".join(report)


# 测试
if __name__ == "__main__":
    test_text = "2024年1月15日，北京出差住宿费，金额1580元，税额47.4元"
    inv = parse_invoice(test_text)
    print(inv)
    compliance = check_compliance(inv)
    print(compliance)
