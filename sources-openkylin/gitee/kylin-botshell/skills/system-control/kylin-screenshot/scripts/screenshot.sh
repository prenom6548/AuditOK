
## 配套脚本 screenshot.sh（存放路径：~/.kylinbot/workspace/skills/screenshot/scripts/）
```bash
#!/bin/bash
# kylin-screenshot 截图技能封装脚本
set -e

# 帮助文档
show_help() {
cat << EOF
用法: $0 run [模式] [--delay N] [--path 路径] [--Newname name]
模式：full / screen / gui
--delay  延迟截图秒数，数字
--path   指定图片保存路径
--NewName 保存文件名字
示例：
$0 run full --delay 3 --path ~/Desktop --Newname snap.png
EOF
}

# 参数初始化
mode=""
delay=0
save_path=""
name=""

# 解析入参
while [[ $# -gt 0 ]]; do
    case "$1" in
        run)
            mode="$2"
            shift 2
            ;;
        --delay)
            delay="$2"
            shift 2
            ;;
        --path)
            save_path="$2"
            shift 2
            ;;
        --Newname)
            name="$2"
            shift 2
            ;;
        --help)
            show_help
            exit 0
            ;;
        *)
            echo "错误：未知参数 $1"
            show_help
            exit 1
            ;;
    esac
done

# 校验截图模式
if [[ ! " full window area " =~ " $mode " ]]; then
    echo '{"success":false,"msg":"截图模式仅支持 full/screen/gui"}'
    exit 1
fi

# 安全过滤路径，防shell注入
black_chars=(";" "|" "&" "`" "$" ">" "<")
for c in "${black_chars[@]}"; do
    if [[ "$save_path" == *"$c"* ]]; then
        echo '{"success":false,"msg":"保存路径包含非法特殊字符，禁止执行"}'
        exit 1
    fi
done

# 组装命令
cmd=("kylin-screenshot" "$mode")
if [[ "$delay" -gt 0 ]]; then
    cmd+=("--delay" "$delay")
fi
if [[ -n "$save_path" ]]; then
    cmd+=("--path" "$save_path")
fi
if [[ -n "$name" ]]; then
    cmd+=("--Newname" "$name")
fi
cmd+=("--json")

# 适配图形界面
export DISPLAY=:0

# 执行截图
result=$("${cmd[@]}" 2>&1)
code=$?

# 输出JSON结果
cat << JSON
{
  "success": $([ $code -eq 0 ] && echo true || echo false),
  "exit_code": $code,
  "output": $result
}
JSON
