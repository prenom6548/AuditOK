import xml.etree.ElementTree as ET

def parse_lcov(lcov_file):
    files = {}
    current_file = None

    with open(lcov_file, 'r') as f:
        for line in f:
            if line.startswith("SF:"):
                current_file = line.strip().split(":", 1)[1]
                if current_file not in files.keys():
                    files[current_file] = {'lines': {}, 'branches': {}}

            elif line.startswith("DA:"):
                line_num, hit = map(int, line.strip().split(":")[1].split(","))
                if line_num in files[current_file]['lines']:
                    files[current_file]['lines'][line_num] += hit
                else:
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

def generate_xml(coverage_data):
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
    tree.write("sonarqube-coverage.xml", encoding="utf-8", xml_declaration=True)

if __name__ == "__main__":
    lcov_data = parse_lcov("./coverage.info")
    generate_xml(lcov_data)

