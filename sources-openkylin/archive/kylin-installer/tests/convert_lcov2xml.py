"""
# coding:  utf8
@Time:     2025/02/2025/2/25
@Author:   80402
@File:     convert_lcov2xml.py
@Desc:     convert_lcov2xml.py
@email:    weijiang1994_1@qq.com
@Software: Pycharm
@License:  MIT
"""
import xml.etree.ElementTree as ET
import argparse


def parse_lcov(lcov_file):
    files = {}
    current_file = None

    with open(lcov_file, 'r') as f:
        for line in f:
            if line.startswith("SF:"):
                current_file = line.strip().split(":", 1)[1]
                files[current_file] = {'lines': {}, 'branches': {}}

            elif line.startswith("DA:"):
                line_num, hit = map(int, line.strip().split(":")[1].split(","))
                files[current_file]['lines'][line_num] = hit

            elif line.startswith("BRDA:"):
                parts = line.strip().split(":")[1].split(",")
                line_num, branch_num, branch_hit = int(parts[0]), int(parts[2]), int(parts[3])
                if line_num not in files[current_file]['branches']:
                    files[current_file]['branches'][line_num] = {'total': 0, 'covered': 0}
                files[current_file]['branches'][line_num]['total'] += 1
                if branch_hit > 0:
                    files[current_file]['branches'][line_num]['covered'] += 1

    return files


def generate_xml(coverage_data, output_file='sonarqube-coverage.xml'):
    root = ET.Element("coverage", version="1")
    for file_path, data in coverage_data.items():
        file_elem = ET.SubElement(root, "file", path=file_path)

        for line_num, hit in data['lines'].items():
            attribs = {
                "lineNumber": str(line_num),
                "covered": "true" if hit > 0 else "false"
            }
            if line_num in data['branches']:
                attribs["branchesToCover"] = str(data['branches'][line_num]['total'])
                attribs["coveredBranches"] = str(data['branches'][line_num]['covered'])

            ET.SubElement(file_elem, "lineToCover", attrib=attribs)

    tree = ET.ElementTree(root)
    tree.write(output_file, encoding="utf-8", xml_declaration=True)


if __name__ == "__main__":
    parser = argparse.ArgumentParser(description='lcov info文件转换为sonarqube支持的xml覆盖率文件')
    parser.add_argument('-l', '--lcov', type=str, help='指定lcov命令生成coverage.info文件路径，建议绝对路径',
                        default='coverage.info')
    parser.add_argument('-o', '--output', type=str, default='sonarqube-coverage.xml',
                        help='输出xml覆盖率文件路径，建议绝对路径，使用 `包名-coverage.xml` 的形式定义，存放debian目录下')
    args = parser.parse_args()

    lcov_data = parse_lcov(args.lcov)
    generate_xml(lcov_data, args.output)
