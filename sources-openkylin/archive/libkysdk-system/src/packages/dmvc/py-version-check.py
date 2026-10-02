#!/usr/bin/python3

import ast
import os
import re
import shutil
import subprocess
import sys
from pathlib import Path
from collections import defaultdict
from typing import Dict, Tuple


class PythonFeatureDetector:
    """Python特性版本检测器"""

    # Python各版本引入的主要特性
    VERSION_FEATURES = {
        (3, 12): {
            'features': [
                '类型参数语法 (type X[int])',
                'f-string改进（多行、嵌套）',
                '缓冲协议支持优化'
            ],
            'keywords': ['type'],
            'syntax': []
        },
        (3, 11): {
            'features': [
                '异常组和except*',
                '变量注解延迟求值',
                'Self类型',
                '零开销异常'
            ],
            'keywords': [],
            'syntax': [r'except\s*\*']
        },
        (3, 10): {
            'features': [
                'match-case语句',
                '带括号的上下文管理器',
                '更清晰的错误信息',
                '联合类型操作符|'
            ],
            'keywords': ['match', 'case'],
            'syntax': [r'match\b', r'case\b']
        },
        (3, 9): {
            'features': [
                '字典合并更新运算符 (|, |=)',
                'str.removeprefix(), str.removesuffix()',
                '类型提示泛型语法',
                'zoneinfo模块',
                'graphlib模块'
            ],
            'keywords': [],
            'syntax': [r'\|=', r'\.removeprefix\(', r'\.removesuffix\(', r'list\[', r'dict\[']
        },
        (3, 8): {
            'features': [
                '海象运算符 (:=)',
                '仅位置参数 (/)',
                'f-string调试语法 (=)',
                '赋值表达式'
            ],
            'keywords': [],
            'syntax': [r':=', r'def\s+\w+\s*\([^)]*/\s*,', r'f".*\{.*=.*\}"']
        },
        (3, 7): {
            'features': [
                '数据类装饰器 (@dataclass)',
                '延迟类型求值',
                '内置breakpoint()',
                '异步生成器',
                '异步推导式'
            ],
            'keywords': ['breakpoint'],
            'syntax': [r'@dataclass', r'from\s+__future__\s+import\s+annotations']
        },
        (3, 6): {
            'features': [
                'f字符串',
                '变量类型注解',
                '数字字面量下划线',
                '异步生成器',
                'secrets模块'
            ],
            'keywords': ['async', 'await'],
            'syntax': [r'f["\']', r'\w+:\s*\w+', r'\d+_\d+']
        },
        (3, 5): {
            'features': [
                'async/await语法',
                '矩阵乘法运算符 (@)',
                '解包泛化 (*, **)',
                '类型提示'
            ],
            'keywords': ['async', 'await'],
            'syntax': [r'async\s+def', r'await\s+', r'@\s*(?![\(])', r'\*\s*\*']
        },
        (3, 4): {
            'features': [
                'enum模块',
                'pathlib模块',
                'statistics模块',
                'asyncio模块',
                '单调度泛函数'
            ],
            'keywords': [],
            'syntax': []
        },
        (3, 3): {
            'features': [
                'yield from表达式',
                'u前缀字符串',
                '虚拟环境支持'
            ],
            'keywords': [],
            'syntax': [r'yield\s+from', r'u["\']']
        },
        (3, 0): {
            'features': [
                'print函数',
                '统一字符串类型',
                '整数除法返回浮点数',
                '新式类'
            ],
            'keywords': [],
            'syntax': [r'print\(']
        },
        (2, 7): {
            'features': [
                '字典推导式',
                '集合推导式',
                'print语句',
                '旧式类',
                'unicode字符串前缀'
            ],
            'keywords': ['print'],
            'syntax': [r'print\s+[^(]', r'ur["\']']
        }
    }

    def __init__(self, deb_path):
        self.features_found = defaultdict(list)
        self.min_version = (3, 0)
        self.shebang_version = None
        self.file_path = ""
        self.deb_path = deb_path

    def analyze_file(self, filepath: str) -> Dict:
        """分析文件并返回结果"""
        path = Path(filepath)
        if not path.exists():
            return {"error": "文件不存在"}

        try:
            content = path.read_text(encoding='utf-8', errors='ignore')

            self.features_found.clear()
            self.min_version = (3, 0)

            self._check_shebang(content)
            # self._check_text_features(content)
            self._check_ast_features(content)

            return self._format_results()

        except Exception as e:
            return {"error": f"分析失败: {str(e)}"}

    def _check_shebang(self, content: str):
        """检查shebang行"""
        lines = content.split('\n')
        if lines and lines[0].startswith('#!'):
            shebang = lines[0].lower()
            if 'python2' in shebang:
                self.min_version = (2, 7)
                self.shebang_version = '2.7'
            elif 'python3' in shebang:
                self.min_version = (3, 0)
                self.shebang_version = '3.0'

    def _check_text_features(self, content: str):
        """通过正则表达式检查文本特征"""
        for version, info in self.VERSION_FEATURES.items():
            for pattern in info.get('syntax', []):
                matches = re.findall(pattern, content, re.MULTILINE | re.DOTALL)
                if matches:
                    self.features_found[version].extend(matches[:3])  # 只记录前3个匹配
                    self.min_version = max(self.min_version, version)

            for keyword in info.get('keywords', []):
                if re.search(rf'\b{keyword}\b', content):
                    self.features_found[version].append(f"关键字: {keyword}")
                    self.min_version = max(self.min_version, version)

    def _check_ast_features(self, content: str):
        """通过AST检查语法特征"""
        try:
            tree = ast.parse(content)

            # 遍历AST节点
            for node in ast.walk(tree):
                # Python 3.12: 类型参数语法
                if hasattr(ast, 'TypeAlias') and isinstance(node, ast.TypeAlias):
                    self._add_feature((3, 12), '类型别名语法')

                # Python 3.11: 异常组
                if isinstance(node, ast.ExceptHandler):
                    if hasattr(node, 'type') and isinstance(node.type, ast.Tuple):
                        for item in node.type.elts:
                            if isinstance(item, ast.Starred):
                                self._add_feature((3, 11), '异常组 (except*)')

                # Python 3.10: match-case
                if isinstance(node, ast.Match):
                    self._add_feature((3, 10), 'match语句')

                # Python 3.8: 海象运算符
                if isinstance(node, ast.NamedExpr):
                    self._add_feature((3, 8), '海象运算符 (:=)')

                # Python 3.8: 仅位置参数
                if isinstance(node, ast.arguments) and node.posonlyargs:
                    self._add_feature((3, 8), '仅位置参数 (/)')

                # Python 3.7: 数据类
                if isinstance(node, ast.ClassDef):
                    for decorator in node.decorator_list:
                        if isinstance(decorator, ast.Name) and decorator.id == 'dataclass':
                            self._add_feature((3, 7), '数据类装饰器')
                        elif isinstance(decorator, ast.Attribute) and decorator.attr == 'dataclass':
                            self._add_feature((3, 7), '数据类装饰器')

                # Python 3.6: 变量注解
                if isinstance(node, ast.AnnAssign):
                    self._add_feature((3, 6), '变量类型注解')

                # Python 3.6: f-string
                if isinstance(node, ast.JoinedStr):
                    self._add_feature((3, 6), 'f-string')

                # Python 3.5: async/await
                if isinstance(node, (ast.AsyncFunctionDef, ast.AsyncFor, ast.AsyncWith)):
                    self._add_feature((3, 5), 'async语法')
                if isinstance(node, ast.Await):
                    self._add_feature((3, 5), 'await表达式')

                # Python 3.5: 矩阵乘法
                if isinstance(node, ast.MatMult):
                    self._add_feature((3, 5), '矩阵乘法运算符 (@)')

                # Python 3.4: yield from
                if isinstance(node, ast.YieldFrom):
                    self._add_feature((3, 3), 'yield from表达式')

                # Python 3.0: print函数
                if isinstance(node, ast.Call) and isinstance(node.func, ast.Name):
                    if node.func.id == 'print':
                        self._add_feature((3, 0), 'print函数')

        except SyntaxError as e:
            # 解析错误可能意味着使用了新版本的语法
            self.features_found[self.min_version].append(f"语法解析错误（可能使用了新特性）: {e}")

    def _add_feature(self, version: Tuple[int, int], feature: str):
        """添加检测到的特性"""
        self.features_found[version].append(feature)
        self.min_version = max(self.min_version, version)

    def _format_results(self) -> Dict:
        """格式化输出结果"""
        # 按版本排序
        sorted_versions = sorted(self.features_found.keys(), reverse=True)

        result = {
            "min_version": f"{self.min_version[0]}.{self.min_version[1]}",
            "shebang_version": self.shebang_version,
            "features_by_version": {},
            "summary": {}
        }

        # 按版本组织特性
        for version in sorted_versions:
            version_str = f"{version[0]}.{version[1]}"
            result["features_by_version"][version_str] = {
                "features": list(set(self.features_found[version])),  # 去重
                "count": len(set(self.features_found[version]))
            }

        result["summary"] = {
            "versions_with_features": len(self.features_found),
            "min_required_version": f"{self.min_version[0]}.{self.min_version[1]}"
        }

        return result

    def print_detailed_report(self, filepath: str):
        """打印详细报告"""
        self.file_path = filepath
        result = self.analyze_file(filepath)

        if "error" in result:
            print(f"{result['error']}")
            return

        print(f"{self.deb_path}: {result['min_version']}")
        # if result['shebang_version']:
        #     print(f"   Shebang指定版本: Python {result['shebang_version']}")

        # if not result['features_by_version']:
        #     print("   未检测到特定版本特性（可能使用基础语法）")
        #     return

        # for version_str in sorted(result['features_by_version'].keys(), key=lambda x: tuple(map(int, x.split('.'))),
        #                           reverse=True):
        #     info = result['features_by_version'][version_str]
        #     if info['features']:
        #         print(f"Python {version_str} 特性 ({info['count']}个):")

# 使用示例
if __name__ == "__main__":
    if len(sys.argv) > 1:
        filename = sys.argv[1]
        if os.path.exists(filename):
            pys = Path(filename).rglob('*.py')
            for py in pys:
                detector = PythonFeatureDetector(filename)
                detector.print_detailed_report(str(py))
        else:
            print('软件包不存在，请重试!')



