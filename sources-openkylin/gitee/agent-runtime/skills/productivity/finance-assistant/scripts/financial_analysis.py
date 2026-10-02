"""
财务智能助手 - 财务报表分析模块
支持杜邦分析、盈利能力分析、偿债能力分析
"""

def dupont_analysis(net_income: float, revenue: float, total_assets: float, equity: float) -> dict:
    """
    杜邦分析法：ROE = 净利润率 × 总资产周转率 × 权益乘数
    
    Args:
        net_income: 净利润
        revenue: 营业收入
        total_assets: 总资产
        equity: 所有者权益
        
    Returns:
        dict: 杜邦分析结果
    """
    # 计算各指标
    net_margin = net_income / revenue if revenue > 0 else 0  # 净利润率
    asset_turnover = revenue / total_assets if total_assets > 0 else 0  # 总资产周转率
    equity_multiplier = total_assets / equity if equity > 0 else 0  # 权益乘数
    roe = net_margin * asset_turnover * equity_multiplier  # ROE
    
    return {
        "roe": roe,
        "net_margin": net_margin,
        "asset_turnover": asset_turnover,
        "equity_multiplier": equity_multiplier,
        "decomposition": f"ROE = {net_margin:.2%} × {asset_turnover:.2f} × {equity_multiplier:.2f} = {roe:.2%}"
    }


def profitability_analysis(net_income: float, revenue: float, gross_profit: float = None) -> dict:
    """
    盈利能力分析
    
    Args:
        net_income: 净利润
        revenue: 营业收入
        gross_profit: 毛利润（可选）
        
    Returns:
        dict: 盈利能力指标
    """
    result = {
        "net_margin": net_income / revenue if revenue > 0 else 0,
        "analysis": []
    }
    
    if result["net_margin"] > 0.15:
        result["analysis"].append("净利润率优秀（>15%）")
    elif result["net_margin"] > 0.08:
        result["analysis"].append("净利润率良好（8%-15%）")
    else:
        result["analysis"].append("净利润率偏低，需关注")
    
    if gross_profit:
        result["gross_margin"] = gross_profit / revenue if revenue > 0 else 0
        if result["gross_margin"] > 0.30:
            result["analysis"].append("毛利率表现优秀")
    
    return result


def solvency_analysis(current_assets: float, current_liabilities: float, 
                      total_assets: float, total_liabilities: float,
                      ebit: float = None, interest_expense: float = None) -> dict:
    """
    偿债能力分析
    
    Args:
        current_assets: 流动资产
        current_liabilities: 流动负债
        total_assets: 总资产
        total_liabilities: 总负债
        ebit: 息税前利润（可选）
        interest_expense: 利息支出（可选）
        
    Returns:
        dict: 偿债能力指标
    """
    result = {
        "current_ratio": current_assets / current_liabilities if current_liabilities > 0 else 0,
        "quick_ratio": (current_assets - 0) / current_liabilities if current_liabilities > 0 else 0,  # 简化计算
        "debt_ratio": total_liabilities / total_assets if total_assets > 0 else 0,
        "analysis": []
    }
    
    # 流动比率
    if result["current_ratio"] > 2:
        result["analysis"].append("流动比率优秀（>2），短期偿债能力强")
    elif result["current_ratio"] > 1:
        result["analysis"].append("流动比率正常（1-2）")
    else:
        result["analysis"].append("⚠️ 流动比率偏低，短期偿债风险")
    
    # 资产负债率
    if result["debt_ratio"] < 0.4:
        result["analysis"].append("资产负债率低，财务稳健")
    elif result["debt_ratio"] < 0.6:
        result["analysis"].append("资产负债率适中")
    else:
        result["analysis"].append("⚠️ 资产负债率偏高，注意债务风险")
    
    # 利息保障倍数
    if ebit and interest_expense and interest_expense > 0:
        result["interest_coverage"] = ebit / interest_expense
        if result["interest_coverage"] > 3:
            result["analysis"].append("利息保障倍数充足")
        else:
            result["analysis"].append("⚠️ 利息保障倍数不足")
    
    return result


def operating_analysis(revenue: float, total_assets: float,
                       inventory: float = None, accounts_receivable: float = None,
                       cost_of_goods_sold: float = None, accounts_payable: float = None) -> dict:
    """
    营运能力分析
    
    Args:
        revenue: 营业收入
        total_assets: 总资产
        inventory: 存货（可选）
        accounts_receivable: 应收账款（可选）
        cost_of_goods_sold: 营业成本（可选）
        accounts_payable: 应付账款（可选）
        
    Returns:
        dict: 营运能力指标
    """
    result = {
        "asset_turnover": revenue / total_assets if total_assets > 0 else 0,
        "analysis": []
    }
    
    # 总资产周转率
    if result["asset_turnover"] > 1.5:
        result["analysis"].append("总资产周转率高，资产利用效率好")
    elif result["asset_turnover"] > 1:
        result["analysis"].append("总资产周转率正常")
    else:
        result["analysis"].append("⚠️ 总资产周转率偏低，资产利用效率有待提升")
    
    # 存货周转率
    if inventory and cost_of_goods_sold and inventory > 0:
        result["inventory_turnover"] = cost_of_goods_sold / inventory
        if result["inventory_turnover"] > 6:
            result["analysis"].append("存货周转快，变现能力强")
        elif result["inventory_turnover"] > 3:
            result["analysis"].append("存货周转正常")
        else:
            result["analysis"].append("⚠️ 存货周转偏慢，注意积压风险")
    
    # 应收账款周转率
    if accounts_receivable and revenue and accounts_receivable > 0:
        result["receivable_turnover"] = revenue / accounts_receivable
        result["receivable_days"] = 365 / result["receivable_turnover"] if result["receivable_turnover"] > 0 else 0
        if result["receivable_days"] < 60:
            result["analysis"].append("应收账款回收快，现金流好")
        elif result["receivable_days"] < 90:
            result["analysis"].append("应收账款回收正常")
        else:
            result["analysis"].append("⚠️ 应收账款回收偏慢，关注坏账风险")
    
    return result


def generate_boss_report(dupont: dict, profitability: dict, solvency: dict, operating: dict) -> str:
    """
    生成老板版简报
    
    Args:
        dupont: 杜邦分析结果
        profitability: 盈利能力分析
        solvency: 偿债能力分析
        operating: 营运能力分析
        
    Returns:
        str: 老板版简报
    """
    report = ["## 📊 财务简报（老板版）\n"]
    
    # ROE总结
    roe = dupont["roe"]
    if roe > 0.2:
        report.append(f"✅ 公司ROE达{roe:.1%}，盈利能力优秀！")
    elif roe > 0.1:
        report.append(f"📈 公司ROE为{roe:.1%}，盈利表现良好")
    else:
        report.append(f"⚠️ 公司ROE为{roe:.1%}，盈利有待提升")
    
    # 关键洞察
    report.append("\n### 💡 关键发现")
    for a in profitability.get("analysis", [])[:1]:
        report.append(f"- {a}")
    for a in solvency.get("analysis", [])[:1]:
        report.append(f"- {a}")
    for a in operating.get("analysis", [])[:1]:
        report.append(f"- {a}")
    
    # 风险提示
    report.append("\n### ⚠️ 风险提示")
    all_warnings = [a for a in profitability.get("analysis", []) + solvency.get("analysis", []) + operating.get("analysis", []) if "⚠️" in a]
    if all_warnings:
        for w in all_warnings:
            report.append(f"- {w}")
    else:
        report.append("- 暂无明显风险")
    
    return "\n".join(report)


def generate_professional_report(dupont: dict, profitability: dict, solvency: dict, operating: dict) -> str:
    """
    生成专业版报告
    
    Args:
        dupont: 杜邦分析结果
        profitability: 盈利能力分析
        solvency: 偿债能力分析
        operating: 营运能力分析
        
    Returns:
        str: 专业版报告
    """
    report = ["## 📈 财务报表分析报告（专业版）\n"]
    
    # 杜邦分析
    report.append("### 杜邦分析\n")
    report.append(f"**{dupont['decomposition']}**\n")
    report.append("| 指标 | 数值 | 说明 |")
    report.append("|------|------|------|")
    report.append(f"| ROE | {dupont['roe']:.2%} | 净资产收益率 |")
    report.append(f"| 净利润率 | {dupont['net_margin']:.2%} | 反映盈利能力 |")
    report.append(f"| 总资产周转率 | {dupont['asset_turnover']:.2f}次 | 反映运营效率 |")
    report.append(f"| 权益乘数 | {dupont['equity_multiplier']:.2f} | 反映财务杠杆 |")
    
    # 盈利能力
    report.append("\n### 盈利能力\n")
    for a in profitability.get("analysis", []):
        report.append(f"- {a}")
    
    # 偿债能力
    report.append("\n### 偿债能力\n")
    report.append(f"- 流动比率：{solvency['current_ratio']:.2f}")
    report.append(f"- 资产负债率：{solvency['debt_ratio']:.2%}")
    for a in solvency.get("analysis", []):
        report.append(f"- {a}")
    
    # 营运能力
    report.append("\n### 营运能力\n")
    report.append(f"- 总资产周转率：{operating['asset_turnover']:.2f}次")
    for a in operating.get("analysis", []):
        report.append(f"- {a}")
    
    return "\n".join(report)


# 测试
if __name__ == "__main__":
    dupont = dupont_analysis(500, 5000, 4000, 2500)
    print("杜邦分析:", dupont)
    
    profitability = profitability_analysis(500, 5000)
    print("盈利能力:", profitability)
    
    solvency = solvency_analysis(2000, 1500, 4000, 1500)
    print("偿债能力:", solvency)
    
    operating = operating_analysis(5000, 4000)
    print("营运能力:", operating)
