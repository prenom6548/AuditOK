"""
财务智能助手 - 税务计算模块
支持个税计算、社保计算、年终奖计税、离职补偿金计算
"""

# 2024年个人所得税税率表（7级超额累进）
TAX_RATE_TABLE = [
    (36000, 0.03, 0),
    (144000, 0.10, 2520),
    (300000, 0.20, 16920),
    (420000, 0.25, 31920),
    (660000, 0.30, 52920),
    (960000, 0.35, 85920),
    (float('inf'), 0.45, 181920),
]

# 社保公积金缴费比例（示例，实际情况因城市而异）
SOCIAL_INSURANCE_RATES = {
    "北京": {"pension": 0.08, "medical": 0.02, "unemployment": 0.005, "housing": 0.12},
    "上海": {"pension": 0.08, "medical": 0.02, "unemployment": 0.005, "housing": 0.07},
    "深圳": {"pension": 0.08, "medical": 0.02, "unemployment": 0.005, "housing": 0.12},
    "广州": {"pension": 0.08, "medical": 0.02, "unemployment": 0.005, "housing": 0.10},
    "杭州": {"pension": 0.08, "medical": 0.02, "unemployment": 0.005, "housing": 0.12},
}

# 社保缴费基数上下限（2024年示例）
SOCIAL_INSURANCE_LIMITS = {
    "北京": {"min": 5644, "max": 28221},
    "上海": {"min": 7310, "max": 36549},
    "深圳": {"min": 2360, "max": 23601},
    "广州": {"min": 2300, "max": 33786},
    "杭州": {"min": 4462, "max": 22311},
}


def calculate_individual_tax(monthly_salary: float, 
                              social_insurance: float = 0,
                              special_deductions: float = 0,
                              special_additional: float = 0) -> dict:
    """
    计算月度个人所得税
    
    Args:
        monthly_salary: 月薪
        social_insurance: 社保公积金个人部分
        special_deductions: 专项扣除（如5000元起征点）
        special_additional: 专项附加扣除
        
    Returns:
        dict: 税务计算结果
    """
    # 计算应纳税所得额
    taxable_income = monthly_salary - social_insurance - special_deductions - special_additional
    
    if taxable_income <= 0:
        return {
            "monthly_salary": monthly_salary,
            "social_insurance": social_insurance,
            "special_deductions": special_deductions,
            "special_additional": special_additional,
            "taxable_income": 0,
            "monthly_tax": 0,
            "effective_rate": 0,
        }
    
    # 查找适用税率
    tax = 0
    for threshold, rate, deduction in TAX_RATE_TABLE:
        if taxable_income <= threshold:
            tax = taxable_income * rate - deduction
            break
    
    return {
        "monthly_salary": monthly_salary,
        "social_insurance": social_insurance,
        "special_deductions": special_deductions,
        "special_additional": special_additional,
        "taxable_income": taxable_income,
        "monthly_tax": max(0, tax),
        "effective_rate": tax / monthly_salary if monthly_salary > 0 else 0,
    }


def calculate_social_insurance(salary: float, city: str = "北京") -> dict:
    """
    计算社保公积金
    
    Args:
        salary: 缴费基数
        city: 城市名称
        
    Returns:
        dict: 社保计算结果
    """
    city = city if city in SOCIAL_INSURANCE_RATES else "北京"
    rates = SOCIAL_INSURANCE_RATES[city]
    limits = SOCIAL_INSURANCE_LIMITS.get(city, {"min": 2000, "max": 20000})
    
    # 确保在范围内
    base = max(limits["min"], min(salary, limits["max"]))
    
    pension = base * rates["pension"]
    medical = base * rates["medical"]
    unemployment = base * rates["unemployment"]
    housing = base * rates["housing"]
    
    total = pension + medical + unemployment + housing
    
    return {
        "city": city,
        "base": base,
        "pension": pension,
        "medical": medical,
        "unemployment": unemployment,
        "housing": housing,
        "total": total,
        "rates": rates
    }


def calculate_year_end_bonus_tax(bonus: float, monthly_salary: float = 10000) -> dict:
    """
    年终奖计税（单独计税 vs 合并计税）
    
    Args:
        bonus: 年终奖金额
        monthly_salary: 月薪（用于计算单独计税的适用税率）
        
    Returns:
        dict: 年终奖计税对比
    """
    # 单独计税：年终奖/12找税率
    monthly_bonus = bonus / 12
    
    # 查找税率
    separate_rate = 0.03
    for threshold, rate, _ in TAX_RATE_TABLE:
        if monthly_bonus <= threshold:
            separate_rate = rate
            break
    
    separate_tax = bonus * separate_rate
    
    # 合并计税：将年终奖与12月工资合并
    total_annual_income = monthly_salary * 12 + bonus
    
    # 减除60000免征额和社保
    taxable = max(0, total_annual_income - 60000)
    
    # 计算年度个税
    combined_tax = 0
    remaining = taxable
    prev_threshold = 0
    for threshold, rate, deduction in TAX_RATE_TABLE:
        if remaining <= threshold:
            combined_tax = remaining * rate - deduction
            break
        prev_threshold = threshold
    
    # 月均税
    combined_monthly_tax = combined_tax / 12
    
    return {
        "bonus": bonus,
        "separate_tax": separate_tax,
        "combined_tax": combined_tax,
        "separate_monthly": separate_tax / 12,
        "combined_monthly": combined_monthly_tax,
        "savings": abs(combined_tax - separate_tax),
        "better_option": "单独计税" if separate_tax < combined_tax else "合并计税"
    }


def calculate_severance_pay(work_years: int, monthly_salary: float, reason: str = "negotiated") -> dict:
    """
    计算离职补偿金
    
    Args:
        work_years: 工作年限
        monthly_salary: 月工资（上年度月平均工资）
        reason: 离职原因（negotiated/dismissal/layoff）
        
    Returns:
        dict: 补偿金计算结果
    """
    # 补偿标准
    years = min(work_years, 12)  # 最高12年
    
    if reason == "layoff":
        # 经济性裁员，额外加6个月
        months = years + 6
    else:
        months = years
    
    # 月工资超过3倍平均工资的，按3倍计算，且年限不超过12年
    max_months = 12
    avg_salary = monthly_salary  # 上年度月平均工资
    
    if monthly_salary > 3 * avg_salary:
        compensation = 3 * avg_salary * min(work_years, 12)
    else:
        compensation = monthly_salary * min(work_years, max_months)
    
    return {
        "work_years": work_years,
        "monthly_salary": monthly_salary,
        "reason": reason,
        "months": min(work_years, max_months),
        "compensation": compensation,
        "tax_free_portion": 3 * avg_salary * min(work_years, 12),  # 部分免税
        "note": "超过3倍工资部分需缴纳个税" if monthly_salary > 3 * avg_salary else ""
    }


def generate_tax_report(tax_result: dict, bonus_result: dict = None) -> str:
    """
    生成税务报告
    
    Args:
        tax_result: 个税计算结果
        bonus_result: 年终奖计算结果（可选）
        
    Returns:
        str: 格式化报告
    """
    report = ["## 个税计算结果\n"]
    
    report.append("### 月度个税计算")
    report.append(f"| 项目 | 金额 |")
    report.append(f"|------|------|")
    report.append(f"| 月薪 | {tax_result['monthly_salary']:,.2f} |")
    report.append(f"| 社保公积金 | {tax_result['social_insurance']:,.2f} |")
    report.append(f"| 专项附加扣除 | {tax_result['special_additional']:,.2f} |")
    report.append(f"| 应纳税所得额 | {tax_result['taxable_income']:,.2f} |")
    report.append(f"| 月度个税 | {tax_result['monthly_tax']:,.2f} |")
    report.append(f"| 实际税率 | {tax_result['effective_rate']:.2%} |")
    
    if bonus_result:
        report.append("\n### 年终奖计税对比")
        report.append(f"| 计税方式 | 应纳税额 | 税后收入 |")
        report.append(f"|----------|----------|----------|")
        report.append(f"| 单独计税 | {bonus_result['separate_tax']:,.2f} | {bonus_result['bonus'] - bonus_result['separate_tax']:,.2f} |")
        report.append(f"| 合并计税 | {bonus_result['combined_tax']:,.2f} | {bonus_result['bonus'] - bonus_result['combined_tax']:,.2f} |")
        report.append(f"| **差额** | **{abs(bonus_result['savings']):,.2f}** |")
        report.append(f"\n### 建议")
        report.append(f"✅ 建议选择**{bonus_result['better_option']}**，可节省{bonus_result['savings']:.2f}元")
    
    return "\n".join(report)


# 测试
if __name__ == "__main__":
    # 测试个税计算
    tax = calculate_individual_tax(15000, 1575, 5000, 5000)
    print("个税:", tax)
    
    # 测试社保
    insurance = calculate_social_insurance(15000, "北京")
    print("社保:", insurance)
    
    # 测试年终奖
    bonus = calculate_year_end_bonus_tax(24000, 15000)
    print("年终奖:", bonus)
