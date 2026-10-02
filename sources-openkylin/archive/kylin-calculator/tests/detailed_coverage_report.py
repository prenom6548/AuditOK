#!/usr/bin/env python3
# -*- coding: utf-8 -*-

#  使用方法，放到根目录，输入xml文件和输出文件，python3 detailed_coverage_report.py kylin-calculator.xml -o detailed_coverage_report.txt

import re
import sys
import argparse
from collections import defaultdict

def parse_detailed_coverage(xml_file):
    """
    解析覆盖率XML文件并按文件计算详细覆盖率信息
    """
    # 读取文件内容
    with open(xml_file, 'r', encoding='utf-8') as f:
        content = f.read()
    
    # 存储每个文件的覆盖率信息
    file_coverage = defaultdict(lambda: {
        'total_lines': 0,
        'covered_lines': 0,
        'total_branches': 0,
        'covered_branches': 0
    })
    
    # 提取文件路径和对应的行信息
    # 首先找到所有文件块
    file_pattern = r'<file path="([^"]+)">([^<]*(?:<(?!/file>)[^<]*)*)</file>'
    file_matches = re.findall(file_pattern, content, re.DOTALL)
    
    # 处理每个文件
    for file_path, file_content in file_matches:
        # 在文件内容中查找所有行信息
        line_pattern = r'<lineToCover lineNumber="(\d+)" covered="(true|false)"(?: branchesToCover="(\d+)")?(?: coveredBranches="(\d+)")?'
        line_matches = re.findall(line_pattern, file_content)
        
        # 统计当前文件的覆盖率信息
        total_lines = 0
        covered_lines = 0
        total_branches = 0
        covered_branches = 0
        
        for match in line_matches:
            line_number = match[0]
            covered = match[1]
            branches_to_cover = match[2] if match[2] else None
            covered_branches_count = match[3] if match[3] else None
            
            # 统计行覆盖率
            total_lines += 1
            if covered == 'true':
                covered_lines += 1
                
            # 统计分支覆盖率
            if branches_to_cover is not None and covered_branches_count is not None:
                total_branches += int(branches_to_cover)
                covered_branches += int(covered_branches_count)
        
        # 存储文件的覆盖率信息
        file_coverage[file_path] = {
            'total_lines': total_lines,
            'covered_lines': covered_lines,
            'total_branches': total_branches,
            'covered_branches': covered_branches
        }
    
    return file_coverage

def calculate_percentages(total_lines, covered_lines, total_branches, covered_branches):
    """
    计算覆盖率百分比
    """
    line_coverage = (covered_lines / total_lines * 100) if total_lines > 0 else 0
    branch_coverage = (covered_branches / total_branches * 100) if total_branches > 0 else 0
    
    return line_coverage, branch_coverage

def generate_report_content(file_coverage):
    """
    生成报告内容字符串
    """
    # 计算总计
    total_lines_all = sum(info['total_lines'] for info in file_coverage.values())
    covered_lines_all = sum(info['covered_lines'] for info in file_coverage.values())
    total_branches_all = sum(info['total_branches'] for info in file_coverage.values())
    covered_branches_all = sum(info['covered_branches'] for info in file_coverage.values())
    
    # 计算总体百分比
    overall_line_coverage, overall_branch_coverage = calculate_percentages(
        total_lines_all, covered_lines_all, total_branches_all, covered_branches_all)
    
    # 构建报告内容
    report_lines = []
    report_lines.append("=" * 80)
    report_lines.append("详细代码覆盖率报告")
    report_lines.append("=" * 80)
    report_lines.append(f"总体统计:")
    report_lines.append(f"  总行数: {total_lines_all}")
    report_lines.append(f"  已覆盖行数: {covered_lines_all}")
    report_lines.append(f"  行覆盖率: {overall_line_coverage:.2f}%")
    report_lines.append("")
    report_lines.append(f"  总分支数: {total_branches_all}")
    report_lines.append(f"  已覆盖分支数: {covered_branches_all}")
    report_lines.append(f"  分支覆盖率: {overall_branch_coverage:.2f}%")
    report_lines.append("=" * 80)
    
    # 按行覆盖率排序文件
    sorted_files = sorted(file_coverage.items(), 
                         key=lambda x: (x[1]['covered_lines'] / x[1]['total_lines']) if x[1]['total_lines'] > 0 else 0, 
                         reverse=True)
    
    # 输出每个文件的详细信息
    report_lines.append("各文件详细覆盖率信息 (按行覆盖率排序):")
    report_lines.append("-" * 80)
    report_lines.append(f"{'文件路径':<40} {'行覆盖率':<10} {'分支覆盖率':<10}")
    report_lines.append("-" * 80)
    
    for file_path, info in sorted_files:
        line_cov_pct, branch_cov_pct = calculate_percentages(
            info['total_lines'], info['covered_lines'], 
            info['total_branches'], info['covered_branches'])
        
        # 只显示文件名，不显示完整路径
        file_name = file_path.split('/')[-1]
        if len(file_name) > 35:
            file_name = file_name[:32] + "..."
            
        report_lines.append(f"{file_name:<40} {line_cov_pct:>6.2f}%   {branch_cov_pct:>6.2f}%")
    
    report_lines.append("=" * 80)
    
    # 输出覆盖率最低的10个文件
    report_lines.append("\n行覆盖率最低的10个文件:")
    report_lines.append("-" * 80)
    report_lines.append(f"{'文件路径':<40} {'行覆盖率':<10} {'分支覆盖率':<10}")
    report_lines.append("-" * 80)
    
    # 筛选出有代码行的文件（至少有1行代码）
    files_with_code = [(path, info) for path, info in file_coverage.items() if info['total_lines'] > 0]
    # 按行覆盖率升序排序
    lowest_coverage_files = sorted(files_with_code, 
                                  key=lambda x: (x[1]['covered_lines'] / x[1]['total_lines']) if x[1]['total_lines'] > 0 else 0)
    
    # 显示最低的10个文件
    for file_path, info in lowest_coverage_files[:10]:
        line_cov_pct, branch_cov_pct = calculate_percentages(
            info['total_lines'], info['covered_lines'], 
            info['total_branches'], info['covered_branches'])
        
        # 只显示文件名，不显示完整路径
        file_name = file_path.split('/')[-1]
        if len(file_name) > 35:
            file_name = file_name[:32] + "..."
            
        report_lines.append(f"{file_name:<40} {line_cov_pct:>6.2f}%   {branch_cov_pct:>6.2f}%")
    
    report_lines.append("=" * 80)
    
    return "\n".join(report_lines)

def main():
    parser = argparse.ArgumentParser(description='Generate detailed coverage report from XML file')
    parser.add_argument('xml_file', help='Coverage XML file to process')
    parser.add_argument('-o', '--output', help='Output file (default: stdout)')
    
    args = parser.parse_args()
    
    try:
        # 解析覆盖率数据
        file_coverage = parse_detailed_coverage(args.xml_file)
        
        # 生成报告内容
        report_content = generate_report_content(file_coverage)
        
        # 输出报告
        if args.output:
            with open(args.output, 'w', encoding='utf-8') as f:
                f.write(report_content)
            print(f"Report saved to {args.output}")
        else:
            print(report_content)
        
    except Exception as e:
        print(f"Error processing coverage file: {e}")
        import traceback
        traceback.print_exc()
        sys.exit(1)

if __name__ == "__main__":
    main()
