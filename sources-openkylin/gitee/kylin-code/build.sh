#!/bin/bash

# 脚本名称：build.sh
# 描述：快速构建deb脚本，用于解析参数、调用命令和显示帮助。
# 作者：wangpenglong
# 创建日期：2024年05月10日

# 开启调试模式，显示脚本执行过程 -x;
# 开启错误处理，脚本执行出错时退出 -e;
# set -ex
set -e

VERSION="1.0.0"
AUTHOR="wangpenglong"
DATE="2024年05月10日"

# 帮助信息
help_info() {
	echo "使用方法：$0 [选项]"
	echo "  -h, --help         						显示帮助信息"
	echo "  -v, --version      						显示脚本版本信息"
	echo "  -c, --clean        						清空构建的文件"
	echo "  -r, --replace_url 						替换url为内网url"
	echo "  -n, --npm_config_registry [arguments]   配置npm源,arguments可为kylin,taobao,tencent,aliyun,huawei,tsinghua.例如'./build.sh -n kylin'"
	echo "  -b, --build        						构建软件包"
}

# 版本信息
version_info() {
	echo "脚本版本：${VERSION}"
	echo "作者：${AUTHOR}"
	echo "创建日期：${DATE}"
}

#添加参数clean，清理编译出的所有内容
function clean() {
	git clean -fdx -e '*.deb' -e '*.rpm'   #清除新建or添加的文件
	rm -rf ../VSCode-linux-*
	# git reset --hard #清除修改的文件
}

#pre build
#获取arch类型
APT_CONFIG_DIR="/etc/apt"

arch=$(uname -m)
VSCODE_ARCH="undefined"
if [ "$arch" = "aarch64" ]; then
	VSCODE_ARCH="arm64"
	export USE_GNUPP2A=1
elif [ "$arch" = "x86_64" ]; then
	VSCODE_ARCH="x64"
else
	echo "Current arch not support,please check!"
fi

is_gnu_sed() {
	sed --version &> /dev/null
}

replace() {
	if is_gnu_sed; then
		sed -i -E "${1}" "${2}"
	else
		sed -i '' -E "${1}" "${2}"
	fi
}

#替换url为内网url
function replace_url() {
	echo "[start] replace local url."
	# css-parser
	css_parser_url="git\+ssh://git@github.com/ramya-rao-a/css-parser.git#370c480ac103bd17c7bcfb34bf5d577dc40d3660"
	css_parser_url_local="http://127.0.0.1:9099/node-depends/css-parser/css-parser-370c480ac103bd17c7bcfb34bf5d577dc40d3660.tar.gz"
	css_parser_lockfile="extensions/emmet/package-lock.json"

	css_parser_package="ramya-rao-a/css-parser#vscode"
	css_parser_packagefile="extensions/emmet/package.json"
	replace "s|${css_parser_url}|${css_parser_url_local}|g" ${css_parser_lockfile}
	echo "替换${css_parser_lockfile}中的地址"
	replace "s|${css_parser_package}|${css_parser_url_local}|g" ${css_parser_packagefile}
	echo "替换${css_parser_packagefile}中的地址"

	#替换calculate-deps.ts中地址
	calculate_deps_files=("build/linux/debian/calculate-deps.ts" "build/linux/debian/calculate-deps.js")
	dpkgShlibdepsUrl="https://raw.githubusercontent.com/chromium"
	dpkgShlibdepsUrl_local="http://127.0.0.1:9099"
	for calculate_deps_file in ${calculate_deps_files[@]}; do
		echo "替换${calculate_deps_file}中的地址"
		replace "s|${dpkgShlibdepsUrl}|${dpkgShlibdepsUrl_local}|g" ${calculate_deps_file}
		replace "s/\/third_party//" ${calculate_deps_file}
	done

	install_sysroot_files=("build/linux/debian/install-sysroot.ts" "build/linux/debian/install-sysroot.js")
	URL_PREFIX="https://msftelectronbuild.z5.web.core.windows.net"
	URL_PREFIX_local="http://127.0.0.1:9099"
	response_url="https://api.github.com/repos/Microsoft"
	response_url_local="http://127.0.0.1:9099"
	sysrootJSONUrl="https://raw.githubusercontent.com/electron"
	sysrootJSONUrl_local="http://127.0.0.1:9099/sysroots"
	asset_url="asset.url"
	asset_url_local="asset.browser_download_url"
	for install_sysroot_file in ${install_sysroot_files[@]}; do
		echo "替换${install_sysroot_file}中的地址"
		replace "s|${URL_PREFIX}|${URL_PREFIX_local}|g" ${install_sysroot_file}
		replace "s|${response_url}|${response_url_local}|g" ${install_sysroot_file}
		replace "s|${sysrootJSONUrl}|${sysrootJSONUrl_local}|g" ${install_sysroot_file}
		replace "s|${asset_url}|${asset_url_local}|g" ${install_sysroot_file}
		replace "s/https/http/g" ${install_sysroot_file}
	done

	# .npmrc
	ELECTRON_URL="https://electronjs.org"
	NODEJS_URL="https://nodejs.org/dist"
	NODEJS_URL_LOCAL="http://127.0.0.1:9099/headers"
	replace "s|${ELECTRON_URL}|${URL_PREFIX_local}|g" .npmrc
	echo "替换.npmrc中的地址"
	replace "s|${NODEJS_URL}|${NODEJS_URL_LOCAL}|g" remote/.npmrc
	echo "替换remote/.npmrc中的地址"
	replace "s|${NODEJS_URL}|${NODEJS_URL_LOCAL}|g" build/.npmrc
	echo "替换build/.npmrc中的地址"

	echo "" >> .npmrc
	echo "electron_mirror=http://127.0.0.1:9099/electron/" >> .npmrc
	echo "electron_use_remote_checksums=1" >> .npmrc
	echo "" >> .npmrc

	echo "[done] replace local url end."
}

function prepare_env(){

	export NODE_OPTIONS="--max-old-space-size=8192"

	#设置环境变量，构建软件包时，可不下载 electron二进制和 playwright browser包
	export ELECTRON_SKIP_BINARY_DOWNLOAD=1
	export PLAYWRIGHT_SKIP_BROWSER_DOWNLOAD=1
	export VSCODE_SKIP_NODE_VERSION_CHECK=1
	export VSCODE_SYSROOT_PREFIX='-glibc-2.28-gcc-10.5.0'

	if [[ -n "${USE_GNUPP2A}" ]]; then
		INCLUDES=$(cat <<EOF
{
  "target_defaults": {
    "conditions": [
      ["OS=='linux'", {
        'cflags_cc!': [ '-std=gnu++20' ],
        'cflags_cc': [ '-std=gnu++2a' ],
      }]
    ]
  }
}
EOF
		)

		if [ ! -d "$HOME/.gyp" ]; then
			mkdir -p "$HOME/.gyp"
		fi

		echo "${INCLUDES}" > "$HOME/.gyp/include.gypi"
	fi
}

#配置npm源
function npm_config_registry(){
	#当无参数时，默认设置kylin的npm源
	if [ -z "$1" ]; then
		echo "config default: tabao"
		npm config set registry https://registry.npmmirror.com/
		return
	fi
	key="$1"
	echo "config npm registry: $key"
	if [ $key == "kylin" ]; then
		npm config set registry https://maven.kylinos.cn/repository/npmmirror/
	elif [ $key == "taobao" ]; then
		npm config set registry https://registry.npmmirror.com/
	elif [ $key == "tencent" ]; then
		npm config set registry https://mirrors.cloud.tencent.com/npm/
	elif [ $key == "aliyun" ]; then
		npm config set registry https://npm.aliyun.com/
	elif [ $key == "huawei" ]; then
		npm config set registry https://mirrors.huaweicloud.com/repository/npm/
	elif [ $key == "tsinghua" ]; then
		npm config set registry https://mirrors.tuna.tsinghua.edu.cn/npm/
	else
		echo "config_npm_registry配置了未知类型:$1"
	fi
}

function build(){
	#start build
	echo "[start] build start."
	set -ex

	prepare_env

	for i in {1..5}; do # try 5 times
		npm ci && break
		if [[ $i == 3 ]]; then
			echo "Npm install failed too many times" >&2
			exit 1
		fi
		echo "Npm install failed $i, trying again..."

		sleep $(( 15 * (i + 1)))
	done

	#关闭telemetry，node_modules也会有地址
	./close_telemetry.sh
	#检查
	npm run monaco-compile-check
	npm run valid-layers-check
	#编译
	npm run gulp compile-build-without-mangling
	npm run gulp compile-extension-media
	npm run gulp compile-extensions-build
	#压缩代码
	npm run gulp minify-vscode
	npm run gulp "vscode-linux-${VSCODE_ARCH}-min-ci" #this will download electron,need change code.
	#打包
	if [ -d $APT_CONFIG_DIR ]; then
		npm run gulp "vscode-linux-${VSCODE_ARCH}-prepare-deb"
		npm run gulp "vscode-linux-${VSCODE_ARCH}-build-deb"
		cp .build/linux/deb/*/deb/*.deb .
	else
		npm run gulp "vscode-linux-${VSCODE_ARCH}-prepare-rpm"
		npm run gulp "vscode-linux-${VSCODE_ARCH}-build-rpm"
		cp .build/linux/rpm/*/*.rpm .
	fi

	echo "[done] build end."
}

# 解析参数
parse_args() {
	while [[ $# -gt 0 ]]; do
		key="$1"

		case $key in
			-h|--help)
			help_info
			exit 0
			;;
			-v|--version)
			version_info
			exit 0
			;;
			-c|--clean)
			clean
			exit 0
			;;
			-r|--replace_url)
			replace_url
			exit 0
			;;
			-n|--npm_config_registry)
			npm_config_registry $2
			exit 0
			;;
			-b|--build)
			build
			exit 0
			;;
			*)
			echo "未知选项：$1"
			help_info
			exit 1
			;;
		esac
		shift
	done
	# 没有选项时，显示帮助信息
	if [ $# -eq 0 ]; then
		help_info
		exit 1
	fi
}

# 主函数
main() {
	parse_args "$@"
}

# 调用主函数
main "$@"
