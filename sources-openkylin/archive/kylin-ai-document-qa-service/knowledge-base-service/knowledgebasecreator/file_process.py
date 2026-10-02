import json
from openpyxl import load_workbook
from datetime import datetime

def excel_to_json(file_path, output_json_path):
    """
    从Excel文件的第一个sheet中读取A到G列数据并转换为JSON

    参数:
        file_path (str): Excel文件路径
        output_json_path (str): 输出的JSON文件路径
    """
    # 自定义JSON序列化函数，处理datetime对象
    def default_serializer(obj):
        if isinstance(obj, datetime):
            return obj.strftime("%Y-%m-%d %H:%M:%S")
        raise TypeError(f"Object of type {type(obj)} is not JSON serializable")

    # 加载工作簿和第一个sheet
    wb = load_workbook(filename=file_path)
    sheet = wb.worksheets[0]  # 获取第一个sheet

    # 定义标题行
    headers = [
        "问题编号", "问题", "回答","分类","硬件平台","产品", "更新时间", "关键字",
        "问题_new", "回答_filter"
    ]

    data = []

    # 从第二行开始读取数据（假设第一行是标题）
    for row in sheet.iter_rows(min_row=2, min_col=1, max_col=10, values_only=True):
        # 检查行数据是否完整
        if len(row) < 10:
            # 如果不足7列，用空字符串填充缺失的列
            row = list(row) + [""] * (10 - len(row))

        # 创建每行数据的字典
        row_data = {}
        for i, header in enumerate(headers):
            # 处理可能为None的值和datetime对象
            value = row[i] if i < len(row) else ""
            if value is None:
                value = ""
            elif isinstance(value, datetime):
                value = value.strftime("%Y-%m-%d %H:%M:%S")
            row_data[header] = value
        data.append(row_data)

    # 写入JSON文件
    with open(output_json_path, 'w', encoding='utf-8') as json_file:
        json.dump(data, json_file, ensure_ascii=False, indent=4, default=default_serializer)

    print(f"数据已成功转换为JSON并保存到 {output_json_path}")

# 使用示例
if __name__ == "__main__":
    excel_file = "./faq.xlsx"  # 替换为你的Excel文件路径
    json_output = "output.json"  # 输出的JSON文件路径
    excel_to_json(excel_file, json_output)