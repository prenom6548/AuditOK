<?xml version="1.0" encoding="utf-8"?>
<!DOCTYPE TS>
<TS version="2.1" language="zh_CN">
<context>
    <name>BaseDialog</name>
    <message>
        <location filename="basedialog.cpp" line="18"/>
        <source>ID</source>
        <translation></translation>
    </message>
    <message>
        <location filename="basedialog.cpp" line="53"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
</context>
<context>
    <name>BaseInfoFrame</name>
    <message>
        <location filename="baseinfoframe.cpp" line="19"/>
        <source>Add depend</source>
        <translation>添加依赖</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="27"/>
        <source>The basic environment needed to build the package.Format: {channel}:{id}/{version}.  Example:  stable:top.openkylin.base/1.0.0</source>
        <oldsource>The basic environment needed to build the package.Format: {channel}:{id}/{version}.  Example:  base: stable:top.openkylin.base/1.0.0</oldsource>
        <translation>构建软件包所需基础环境。格式：{channel}:{id}/{version}。例：stable:top.openkylin.base/1.0.0</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="31"/>
        <source>The basic environment needed to build the package.
Format: {channel}:{id}/{version}
Example:
stable:top.openkylin.base/1.0.0</source>
        <oldsource>The basic environment needed to build the package.
Format: {channel}:{id}/{version}
Example:
base: stable:top.openkylin.base/1.0.0</oldsource>
        <translation>构建软件包所需基础环境
格式：{channel}:{id}/{version}
例:
stable:top.openkylin.base/1.0.0</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="44"/>
        <source>Specific environments that complement the base environment to build package,such as cmake,qmake, etc. Format: {channel}:{id}/{version}. Example: stable:top.openkylin.ukui/1.0.0</source>
        <oldsource>Specific environments that complement the base environment to build package,such as cmake,qmake, etc. Format: {channel}:{id}/{version}.  Example:runtime: stable:top.openkylin.ukui/1.0.0</oldsource>
        <translation>构建软件包所需特定环境。例如cmake、qmake等。格式：{channel}:{id}/{version}。例：stable:top.openkylin.ukui/1.0.0</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="49"/>
        <source>Specific environments that complement the base environment to build package,such as cmake,qmake, etc
Format: {channel}:{id}/{version}
Example:
stable:top.openkylin.ukui/1.0.0</source>
        <oldsource>Specific environments that complement the base environment to build package,such as cmake,qmake, etc
Format: {channel}:{id}/{version}
Example:
runtime: stable:top.openkylin.ukui/1.0.0</oldsource>
        <translation>构建软件包所需特定环境，例如cmake、qmake等。
格式：{channel}/{id}/{version}
例：
stable:top.openkylin.ukui/1.0.0</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="69"/>
        <source>Dependencies needed to build packages. Example:  top.openkylin.depend</source>
        <oldsource>Dependencies needed to build packages. Example:  top.openkylin.depend1/1.0.0\n top.openkylin.depend2.</oldsource>
        <translation>构建软件包所需依赖。例：top.openkylin.depend</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="75"/>
        <source>Dependencies needed to build packages.
Example:
 top.openkylin.depend</source>
        <translation>构建软件包所需依赖。
例：
top.openkylin.depend</translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="62"/>
        <source>base</source>
        <translation></translation>
    </message>
    <message>
        <location filename="baseinfoframe.cpp" line="63"/>
        <source>runtime</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>BuildOptionsDialog</name>
    <message>
        <location filename="buildoptionsdialog.cpp" line="19"/>
        <source>BuildOptions</source>
        <translation>构建选项配置</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="35"/>
        <source>Flags Options</source>
        <translation>编译标志选项</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="39"/>
        <source>cflags</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="40"/>
        <source>C/C++ compiler options environment variable, multiple options separated by spaces. Example: -O2 -g</source>
        <translation>C/C++编译器选项环境变量，多个选项用空格隔开。例：-O2 -g</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="41"/>
        <source>Environment variable for setting C/C++ compiler options
Can specify various compilation options like optimization level (e.g., -O2),
warning level (e.g., -Wall), include paths (e.g., -I/path/to/include)
Common options for both C and C++ compilers</source>
        <translation>用于设置C/C++编译器选项的环境变量
可以指定编译过程中的各种选项，如优化级别（如-O2）、
警告级别（如-Wall）、头文件包含路径（如-I/path/to/include）等
适用于C和C++编译器的通用选项</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="48"/>
        <source>cppflags</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="49"/>
        <source>Options needed by preprocessor, multiple options separated by spaces. Example: -I/usr/include -DDEBUG</source>
        <translation>预处理器需要的选项，多个选项用空格隔开。例如：-I/usr/include -DDEBUG</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="50"/>
        <source>Options needed by preprocessor
Mainly for specifying include paths (e.g., -I option)
Preprocessor will handle these options before compilation
to correctly find and include required header files</source>
        <translation>预处理器需要的选项
主要用于指定头文件路径（如-I选项）
在编译之前，预处理器会处理这些选项，
以便正确地找到和包含所需的头文件</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="56"/>
        <source>cxxflags</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="57"/>
        <source>Environment variable used in CMake, multiple options separated by spaces. Example: -std=c++11 -Wall</source>
        <translation>CMake中使用的环境变量，多个选项用空格隔开。例：-std=c++11 -Wall</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="58"/>
        <source>Environment variable used in CMake, specifically for C++ compiler options
When using CMake for project building, CXXFLAGS value will be used by CMake
to initialize CMAKE_CXX_FLAGS cache entry
Can add default compilation flags for C++ files</source>
        <translation>CMake中使用的环境变量，专门用于C++编译器的选项
当使用CMake进行项目构建时，CXXFLAGS环境变量的值会被CMake
用来初始化CMAKE_CXX_FLAGS缓存项
可以在编译C++文件时添加默认的编译标志</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="64"/>
        <source>ldflags</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="65"/>
        <source>Linker options environment variable, multiple options separated by spaces. Example: -L/path/to/lib -lmylib</source>
        <translation>链接器选项环境变量，多个选项用空格隔开。例：-L/path/to/lib -lmylib</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="66"/>
        <source>Environment variable for setting linker options
1. Specify library path: -L option for linker to search library files
2. Specify library files: -l option to link required libraries
3. Other linker options: can include other linker options as needed
Example: -L/path/to/lib -lmylib</source>
        <translation>用于设置链接器选项的环境变量
1. 指定库路径：通过-L选项，可以指定链接器搜索库文件的路径
2. 指定库文件：通过-l选项，可以指定需要链接的库文件
3. 其他链接选项：可以包含其他链接器选项以满足特定需求
例如：-L/path/to/lib -lmylib</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="73"/>
        <source>Path Options</source>
        <translation>路径选项</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="77"/>
        <source>append-path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="78"/>
        <source>Append to PATH, using leading colon. Example: :/opt/bin:/usr/local/bin</source>
        <translation>追加PATH，使用前导冒号。例如：:/opt/bin:/usr/local/bin</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="79"/>
        <source>This will be appended to PATH in build environment
Format: :path1:path2 (with leading colon)
Example: :/opt/bin:/usr/local/bin</source>
        <translation>这将被附加到构建环境中的PATH中
格式：:路径1:路径2（使用前导冒号）
示例：:/opt/bin:/usr/local/bin</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="84"/>
        <source>prepend-path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="85"/>
        <source>Prepend to PATH, using trailing colon. Example: /opt/bin:/usr/local/bin:</source>
        <translation>前置PATH，使用尾随冒号。例如：/opt/bin:/usr/local/bin:</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="86"/>
        <source>This will be prepended to PATH in build environment
Format: path1:path2: (with trailing colon)
Example: /opt/bin:/usr/local/bin:</source>
        <translation>这将在构建环境中预先设置为PATH
格式：路径1:路径2:（使用尾随冒号）
示例：/opt/bin:/usr/local/bin:</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="91"/>
        <source>append-ld-library-path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="92"/>
        <source>Append to LD_LIBRARY_PATH, using leading colon. Example: :/usr/local/lib:/opt/lib</source>
        <translation>追加LD_LIBRARY_PATH，使用前导冒号。例如：:/usr/local/lib:/opt/lib</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="93"/>
        <source>This will be appended to LD_LIBRARY_PATH in build environment
Format: :path1:path2 (with leading colon)
Example: :/usr/local/lib:/opt/lib</source>
        <translation>这将被附加到构建环境中的LD_LIBRARY_PATH中
格式：:路径1:路径2（使用前导冒号）
示例：:/usr/local/lib:/opt/lib</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="98"/>
        <source>prepend-ld-library-path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="99"/>
        <source>Prepend to LD_LIBRARY_PATH, using trailing colon. Example: /usr/local/lib:/opt/lib:</source>
        <translation>前置LD_LIBRARY_PATH，使用尾随冒号。例如：/usr/local/lib:/opt/lib:</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="100"/>
        <source>This will be prepended to LD_LIBRARY_PATH in build environment
Format: path1:path2: (with trailing colon)
Example: /usr/local/lib:/opt/lib:</source>
        <translation>这将在构建环境中被预处理为LD_LIBRARY_PATH
格式：路径1:路径2:（使用尾随冒号）
示例：/usr/local/lib:/opt/lib:</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="105"/>
        <source>append-pkg-config-path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="106"/>
        <source>Append to PKG_CONFIG_PATH, using leading colon. Example: :/new/pkg1:/new/pkg2</source>
        <translation>追加PKG_CONFIG_PATH，使用前导冒号。例如：:/new/pkg1:/new/pkg2</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="107"/>
        <source>This will be appended to PKG_CONFIG_PATH in build environment
Format: :path1:path2 (with leading colon)
Example: :/usr/local/lib/pkgconfig:/opt/lib/pkgconfig</source>
        <translation>这将被附加到构建环境中的PKG_CONFIG_PATH中
格式：:路径1:路径2（使用前导冒号）
示例：:/usr/local/lib/pkgconfig:/opt/lib/pkgconfig</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="112"/>
        <source>prepend-pkg-config-path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="113"/>
        <source>Prepend to PKG_CONFIG_PATH, using trailing colon. Example: /new/pkg1:/new/pkg2:</source>
        <translation>前置PKG_CONFIG_PATH，使用尾随冒号。例如：/new/pkg1:/new/pkg2:</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="114"/>
        <source>This will be prepended to PKG_CONFIG_PATH in build environment
Format: path1:path2: (with trailing colon)
Example: /usr/local/lib/pkgconfig:/opt/lib/pkgconfig:</source>
        <translation>这将在构建环境中被预处理为PKG_CONFIG_PATH
格式：路径1:路径2:（使用尾随冒号）
示例：/usr/local/lib/pkgconfig:/opt/lib/pkgconfig:</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="119"/>
        <source>Advanced Options</source>
        <translation>高级选项</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="145"/>
        <source>Add env</source>
        <translation>添加环境变量</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="148"/>
        <source>env</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="152"/>
        <source>Enter one make argument per line, example:
-j4
CFLAGS=&quot;-O2&quot;
DESTDIR=/tmp/pkg
VERBOSE=1</source>
        <translation>每行输入一个make参数，例如：
-j4
CFLAGS=\&quot;-O2\&quot;
DESTDIR=/tmp/pkg
VERBOSE=1</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="157"/>
        <source>make-args</source>
        <translation></translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="160"/>
        <source>Enter one make install argument per line, example:
INSTALL_ROOT=/mnt
strip
-i4</source>
        <translation>每行输入一个make install参数，例如：
INSTALL_ROOT=/mnt
strip
-i4</translation>
    </message>
    <message>
        <location filename="buildoptionsdialog.cpp" line="164"/>
        <source>make-install-args</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>BuildSettingDialogPrivate</name>
    <message>
        <location filename="buildsettingdialog.cpp" line="82"/>
        <source>Build Setting</source>
        <translation>构建命令设置</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="85"/>
        <source>Only execute the build and clean phases</source>
        <translation>只构建，不执行后续的完善信息、生成开明包操作</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="86"/>
        <source>Only execute the finish and export packaging stages</source>
        <translation>只执行完善信息操作（上次已构建成功的前提下）</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="87"/>
        <source>Only execute the export packaging phase</source>
        <translation>只执行生成开明包操作（上次构建、完善信息已完成的前提下）</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="88"/>
        <source>Clear the last build cache first, and then rebuild</source>
        <translation>先清除上次构建缓存，再重新构建</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="89"/>
        <source>Keep the source code for debugging after the construction is completed</source>
        <translation>构建完成后保留源码用于调试</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="91"/>
        <source>Build command:</source>
        <translation>构建命令</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="123"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="buildsettingdialog.cpp" line="124"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>BuildSettingWidgetPrivate</name>
    <message>
        <location filename="buildsettingwidget.cpp" line="83"/>
        <source>Build Setting</source>
        <translation>构建命令设置</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="86"/>
        <source>Only execute the build and clean phases</source>
        <translation>只构建，不执行后续的完善信息、生成开明包操作</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="87"/>
        <source>Only execute the finish and export packaging stages</source>
        <translation>只执行完善信息操作（上次已构建成功的前提下）</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="88"/>
        <source>Only execute the export packaging phase</source>
        <translation>只执行生成开明包操作（上次构建、完善信息已完成的前提下）</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="89"/>
        <source>Clear the last build cache first, and then rebuild</source>
        <translation>先清除上次构建缓存，再重新构建</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="90"/>
        <source>Keep the source code for debugging after the construction is completed</source>
        <translation>构建完成后保留源码用于调试</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="92"/>
        <source>Build command:</source>
        <translation>构建命令</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="124"/>
        <source>Next</source>
        <translation>下一步</translation>
    </message>
    <message>
        <location filename="buildsettingwidget.cpp" line="125"/>
        <source>Back</source>
        <translation>上一步</translation>
    </message>
</context>
<context>
    <name>ChooseLineEdit</name>
    <message>
        <location filename="component/chooselineedit.cpp" line="13"/>
        <source>Clear</source>
        <translation></translation>
    </message>
    <message>
        <location filename="component/chooselineedit.cpp" line="32"/>
        <source>Choose</source>
        <translation>选择</translation>
    </message>
</context>
<context>
    <name>ComponentWidgetPrivate</name>
    <message>
        <location filename="componentwidget.cpp" line="240"/>
        <source>name</source>
        <translation></translation>
    </message>
    <message>
        <source>The way of building component. 
The supported building modes are simple(“Customize the component building process in the build-script)、cmake、qmake、cmake-ninja、meson、autotools。</source>
        <oldsource>The way of building component. 
The supported building modes are simple、cmake、qmake、cmake-ninja、meson、autotools。</oldsource>
        <translation type="vanished">构建组件的方式，当前支持simple（在build-script中自定义组件构建过程）、cmake、qmake、cmake-ninja、meson、autotools</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="250"/>
        <source>buildsystem</source>
        <translation></translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="365"/>
        <source>Add config-opts</source>
        <translation>添加 config-opts</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="234"/>
        <source>The name of building component. Example: demo</source>
        <translation>构建组件名称。例：demo</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="237"/>
        <source>The name of building component.
Example:
demo</source>
        <translation>构建组件名称。
例：
demo</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="259"/>
        <location filename="componentwidget.cpp" line="267"/>
        <source>Custom build and installation behavior implemented through shell scripts.
Note: The software installation path should be in the /opt/apps/${APP-ID}/files directory, not in /usr. 
Eaxmple:
set -e
./configure --prefix==${APP_INSTALL_PREFIX}
make -j8
make install</source>
        <translation>以shell脚本实现的自定义构建、安装行为。注意：软件安装路径应为/opt/apps/${APP_ID}/files目录下，而非/usr下。
例： 
set -e
./configure --prefix==${APP_INSTALL_PREFIX}
make -j8
make install</translation>
    </message>
    <message>
        <source>A series of scripts that are executed after make and before make install. Example: mv *.cpp files</source>
        <translation type="vanished">在make之后，make install之前执行的一系列脚本命令。例：mv *.cpp files</translation>
    </message>
    <message>
        <source>A series of scripts that are executed after make and before make install
Example:
 mv *.cpp files
 install -D binary_file ...</source>
        <translation type="vanished">在make之后，make install之前执行的一系列脚本命令。
例：
mv *.cpp files</translation>
    </message>
    <message>
        <source>A series of scripts that are executed after the installation phase has completed execution. Example: rm -rf /data</source>
        <translation type="vanished">在安装阶段完成后执行的一系列脚本命令。例：rm -rf /data</translation>
    </message>
    <message>
        <source>A series of scripts that are executed after the installation phase has completed execution
Example:
 rm -rf /data
 cp demo target
</source>
        <translation type="vanished">在安装阶段完成后执行的一系列脚本命令。
例：
rm -rf /data</translation>
    </message>
    <message>
        <source>Files to be deleted after the software package is builded. Example: /path/a\n *.so</source>
        <translation type="vanished">软件构建完成后需要删除的文件列表。例：/path/a\n *.so</translation>
    </message>
    <message>
        <source>Files to be deleted after the software package is builded.
 Wildcard characters are supported. Separated by newlines.
Example:
 /path/a 
 *.so 
 /path/b 
</source>
        <oldsource>Files to be deleted after the software package is constructed.
 Wildcard characters are supported. Separated by newlines.
Example:
 /path/a 
 *.so 
 /path/b 
</oldsource>
        <translation type="vanished">软件构建完成后需要删除的文件列表。支持通配符，以换行符分割。
例：
/path/a
*.so
/path/b</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="338"/>
        <source>config-opts</source>
        <translation></translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="339"/>
        <source>An array of options passed to the configuration session.
For example, the configuration part of the cmake build is cmake .
Example:
 -DBUILD_TESTING=OFF
 -DCMAKE_INSTALL_PREFIX:PATH=/opt/apps/myappid/files
 --disable-static</source>
        <translation>传递给配置阶段的选项参数。
例如：以cmake的配置阶段cmake .为例：
Example:
 -DBUILD_TESTING=OFF
 -DCMAKE_INSTALL_PREFIX:PATH=/opt/apps/myappid/files
 --disable-static</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="348"/>
        <source>An array of options passed to the configuration session(such as qmake xxx.pro | ./configure). Example: -DBUILD_TESTING=OFF</source>
        <translation>传递给配置阶段(如：qmake xxx.pro | ./configure)的选项参数。例：-DBUILD_TESTING=OFF</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="371"/>
        <source>Set build-options</source>
        <translation>设置 build-options</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="388"/>
        <source>sources</source>
        <translation></translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="389"/>
        <location filename="componentwidget.cpp" line="395"/>
        <source>A list of source codes to be downloaded and extracted for the component.
During the build process, the source code corresponding to all sources of the same component will be copied to the same directory within the project directory.</source>
        <translation>组件下载提取的源代码列表。
在构建过程中，相同组件需要的一系列源代码会被拷贝到属于项目目录的同一文件夹中。</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="398"/>
        <source>Add source</source>
        <translation>添加源码</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="183"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="190"/>
        <source>OK</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="244"/>
        <source>1. The source code root directory corresponding to the building component contains the &apos;CMakeLists. txt&apos; file, which is suitable for selecting &apos;cmake&apos; and &apos;cmake ninja&apos; to build the system. 
2. There is a &apos;meson. build&apos; file in the root directory of the source code corresponding to the building component, which is suitable for selecting &apos;meson&apos; to build the system.
3. There is a &apos;*. pro&apos; file in the root directory of the source code (there can only be one. pro file), which is suitable for choosing &apos;qmake&apos; to build the system.
4. There are &apos;configure&apos; and &apos;autogen&apos; in the root directory of the source code One of the files&apos; autogen. sh &apos;,&apos; bootstrap &apos;, or&apos; bootstrap. sh &apos;is suitable for choosing&apos; autotools&apos; to build a system.
5. In all other cases, you can choose &apos;implement&apos; to build the system and customize the software build and installation behavior in the &apos;build script&apos; property.
</source>
        <translation>1. 源码根目录下存在&apos;CMakeLists.txt&apos;文件，适合选择&apos;cmake&apos;和&apos;cmake-ninja&apos;构建系统。
2. 源码根目录下存在&apos;meson.build&apos;文件，适合选择&apos;meson&apos;构建系统。
3. 源码根目录下存在&apos;*.pro&apos;文件(只能有一个.pro文件)，适合选择&apos;qmake&apos;构建系统。
4. 源码根目录下存在&apos;configure&apos;和&apos;autogen&apos;、&apos;autogen.sh&apos;、&apos;bootstrap&apos;、&apos;bootstrap.sh&apos;文件之一，适合选择&apos;autotools&apos;构建系统。
5. 其余情况，都可以选择&apos;simple&apos;构建系统，在属性&apos;build-script&apos;中自定义软件构建、安装行为。</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="282"/>
        <location filename="componentwidget.cpp" line="287"/>
        <source>After make, execute a shell script before make install. Example: 
set -e
mv hello.sh hello</source>
        <translation>make或ninja之后，make install或ninja install之前执行的一段shell脚本。 例:
set -e
mv hello.sh hello</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="297"/>
        <location filename="componentwidget.cpp" line="304"/>
        <source>A shell script executed after make install or ninja install. 
Note: The software installation path should be in the /opt/apps/${APP-ID}/files directory, not in /usr. 
Example: 
set -e
install -D code.sh ${APP_INSTALL_PREFIX}/bin/code</source>
        <translation>make install或ninja install之后执行的一段shell脚本。
注意：软件安装路径应为/opt/apps/${APP_ID}/files目录下，而非/usr下。
例：
set -e
install -D code.sh ${APP_INSTALL_PREFIX}/bin/code</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="316"/>
        <source>Files to be deleted after the software package is builded. 
Example: 
/include 
 *.h</source>
        <translation>应该在构建结束删除的文件模式的数组。
例：
/include
 *.h</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="321"/>
        <source>Files to be deleted after the software package is builded.
 Wildcard characters are supported. Separated by newlines.
Example:
/include 
 *.h</source>
        <translation>应该在构建结束删除的文件模式的数组。支持通配符*，多个路径用换行符分割。
例：
/include
*.h</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="485"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="514"/>
        <source>×</source>
        <translation></translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="559"/>
        <source>--disable-static</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>DebianConvertDialog</name>
    <message>
        <location filename="debianconvert/debianconvertdialog.cpp" line="14"/>
        <source>Debian Source Conversion</source>
        <translation>Debian源码转换</translation>
    </message>
    <message>
        <location filename="debianconvert/debianconvertdialog.cpp" line="27"/>
        <source>Project Location</source>
        <translation>项目路径</translation>
    </message>
    <message>
        <location filename="debianconvert/debianconvertdialog.cpp" line="28"/>
        <source>Select Env</source>
        <translation>选择环境</translation>
    </message>
    <message>
        <location filename="debianconvert/debianconvertdialog.cpp" line="29"/>
        <source>Install Env</source>
        <translation>安装环境</translation>
    </message>
    <message>
        <location filename="debianconvert/debianconvertdialog.cpp" line="30"/>
        <source>Generate Project</source>
        <translation>生成项目</translation>
    </message>
</context>
<context>
    <name>GenerateProjectPage</name>
    <message>
        <location filename="debianconvert/generateprojectpage.cpp" line="16"/>
        <source>Kaiming Project Generation</source>
        <translation>开明项目生成</translation>
    </message>
    <message>
        <location filename="debianconvert/generateprojectpage.cpp" line="18"/>
        <source>Generating kaiming project...</source>
        <translation>生成开明项目...</translation>
    </message>
    <message>
        <location filename="debianconvert/generateprojectpage.cpp" line="33"/>
        <source>Conversion Log:</source>
        <translation>转换日志：</translation>
    </message>
    <message>
        <location filename="debianconvert/generateprojectpage.cpp" line="62"/>
        <source>next</source>
        <translation>下一步</translation>
    </message>
    <message>
        <location filename="debianconvert/generateprojectpage.cpp" line="103"/>
        <source>Conversion process failed.</source>
        <translation>转换失败</translation>
    </message>
</context>
<context>
    <name>InstallEnvPage</name>
    <message>
        <source>The basic environment was not installed successfully. Please use the command to manually install it: sudo kaiming install 1%</source>
        <translation type="vanished">base环境安装失败。请使用以下命令手动安装：sudo kaiming install %1</translation>
    </message>
    <message>
        <source>The runtime environment was not installed successfully. Please use the command to manually install it: sudo kaiming install 1%</source>
        <translation type="vanished">runtime环境安装失败。请使用以下命令手动安装：sudo kaiming install %1</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="69"/>
        <source>Install Success</source>
        <translation>安装成功</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="72"/>
        <source>Install Complete!</source>
        <translation>安装完成！</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="80"/>
        <source>The basic environment was not installed successfully. Please use the command to manually install it: sudo kaiming install %1

</source>
        <translation>base环境未成功安装。请使用以下命令手动安装：sudo kaiming install %1

</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="88"/>
        <source>The runtime environment was not installed successfully. Please use the command to manually install it: sudo kaiming install %1

</source>
        <translation>runtime环境未成功安装。请使用以下命令手动安装：sudo kaiming install %1

</translation>
    </message>
    <message>
        <source>Install Failed</source>
        <translation type="vanished">安装失败</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="171"/>
        <source>The basic environment was not installed successfully. Please use the command to manually install it: sudo kaiming install %1</source>
        <translation>基本环境未成功安装。请使用以下命令手动安装：sudo kaiming install %1</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="183"/>
        <source>The runtime environment was not installed successfully. Please use the command to manually install it: sudo kaiming install %1</source>
        <translation>运行环境未成功安装。请使用以下命令手动安装：sudo kaiming install %1</translation>
    </message>
</context>
<context>
    <name>InstallEnvPagePrivate</name>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="206"/>
        <source>💡 If the selected base<byte value="xd"/>untime environment is not installed, it will be automatically installed.</source>
        <translation>💡 如果所选base\runtime环境未安装，则会自动进行安装。</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="221"/>
        <source>Environment Installation</source>
        <translation>环境安装</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="223"/>
        <source>Installing required dependencies...</source>
        <translation>安装需要的依赖环境...</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="240"/>
        <source>Installation Log:</source>
        <translation>安装日志：</translation>
    </message>
</context>
<context>
    <name>MainWindow</name>
    <message>
        <location filename="mainwindow.cpp" line="344"/>
        <source>Normal</source>
        <translation>还原</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="349"/>
        <source>Maximize</source>
        <translation>最大化</translation>
    </message>
</context>
<context>
    <name>MainWindowPrivate</name>
    <message>
        <location filename="mainwindow.cpp" line="431"/>
        <source>kaiming-builder-gui</source>
        <translation>开明构建工具</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="438"/>
        <source>Options</source>
        <translation>选项</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="439"/>
        <source>Minimize</source>
        <translation>最小化</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="440"/>
        <source>Maximize</source>
        <translation>最大化</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="441"/>
        <source>Close</source>
        <translation>关闭</translation>
    </message>
    <message>
        <location filename="mainwindow.cpp" line="472"/>
        <source>Help</source>
        <translation>帮助</translation>
    </message>
    <message>
        <source>Build set</source>
        <translation type="vanished">构建设置</translation>
    </message>
    <message>
        <source>Set</source>
        <translation type="vanished">设置</translation>
    </message>
</context>
<context>
    <name>ManifestWidgetPrivate</name>
    <message>
        <location filename="manifestwidget.cpp" line="182"/>
        <source>Project Path</source>
        <translation>项目路径</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="183"/>
        <source>Create a project under this path.Click the search button on the right to select from the local</source>
        <oldsource>Project Path，click the search button on the right to select from the local</oldsource>
        <translation>在此路径下创建项目，点击右边搜索按钮可选择本地路径。</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="292"/>
        <source>Save</source>
        <translation>保存</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="295"/>
        <source>Refresh</source>
        <translation>刷新</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="299"/>
        <source>Next</source>
        <translation>下一步</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="444"/>
        <source>Back</source>
        <translation>上一步</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="204"/>
        <source>Package Info</source>
        <translation>软件包信息</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="211"/>
        <source>Base Select</source>
        <translation>基础环境配置</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="218"/>
        <source>Setup Info</source>
        <translation>构建信息配置</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="233"/>
        <source>components</source>
        <translation></translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="234"/>
        <location filename="manifestwidget.cpp" line="235"/>
        <source>The software package itself and the components it depends on.</source>
        <oldsource>Dependent components to build the package and itself</oldsource>
        <translation>软件包本身及其所依赖的组件。</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="238"/>
        <source>Add component</source>
        <translation>添加组件</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="255"/>
        <source>Component List</source>
        <translation>组件列表</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="263"/>
        <source>Annotations</source>
        <translation>其他</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="266"/>
        <source>The application requires sudo to run</source>
        <translation>应用需要提权运行</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="269"/>
        <source>Install data to path /usr</source>
        <translation>安装数据到/usr目录</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="273"/>
        <source>Programs that depend on other applications</source>
        <translation>依赖的其他应用的程序</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="276"/>
        <source>Enter dependent executables (absolute paths or names), separated by commas
Example 1 (absolute paths): /usr/bin/b, /opt/app/a
Example 2 (program names): b, a</source>
        <translation>输入依赖的可执行程序（绝对路径或程序名），多个程序用逗号分隔
示例1（绝对路径）: /usr/bin/b, /opt/app/a
示例2（程序名）: b, a</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="279"/>
        <source>Configure dependent executable programs

Can be absolute paths (e.g., /usr/bin/b) or program names (e.g., b)
Multiple entries should be comma-separated

Recommendation: Match the calling style used in this application:
- If called with absolute paths, specify absolute paths
- If called with program names, specify program names</source>
        <translation>配置依赖的其他应用的可执行程序

可以是绝对路径（如 /usr/bin/b）或程序名（如 b）
多个程序用逗号分隔

建议和本应用中的调用方式保持一致：
- 如果使用绝对路径调用，则填写绝对路径
- 如果使用程序名调用，则填写程序名</translation>
    </message>
    <message>
        <source>Set Permission</source>
        <translation type="vanished">设置权限</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="520"/>
        <source>Edit</source>
        <translation>编辑</translation>
    </message>
</context>
<context>
    <name>OutputWidget</name>
    <message>
        <location filename="outputwidget.cpp" line="79"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="80"/>
        <source>No supported file manager found (peony/nautilus)</source>
        <translation>未找到支持的文件管理器（peony/nautilus）</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="110"/>
        <source>Build Complete</source>
        <translation>构建完成</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="111"/>
        <source>The build process has completed successfully!</source>
        <translation>构建成功完成！</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="115"/>
        <source>Build Failed</source>
        <translation>构建失败</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="116"/>
        <source>The build process has failed!</source>
        <translation>构建失败！</translation>
    </message>
</context>
<context>
    <name>OutputWidgetPrivate</name>
    <message>
        <location filename="outputwidget.cpp" line="179"/>
        <source>Back</source>
        <translation>上一步</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="183"/>
        <source>Stop</source>
        <translation>停止构建</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="184"/>
        <source>Open project directory</source>
        <translation>打开项目所在目录</translation>
    </message>
    <message>
        <location filename="outputwidget.cpp" line="186"/>
        <source>Home Page</source>
        <translation>返回首页</translation>
    </message>
</context>
<context>
    <name>PackageFrame</name>
    <message>
        <location filename="packageframe.cpp" line="43"/>
        <source>id</source>
        <oldsource>ID</oldsource>
        <translation></translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="48"/>
        <source>Unique identifier of the software. It consists of a triplet.Example: top.openkylin.demo</source>
        <oldsource>Unique identifier of the software. It consists of a triplet.Example: id: top.openkylin.demo</oldsource>
        <translation>软件的唯一标识符。由三元组组成。例：top.openkylin.demo</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="51"/>
        <source>Unique identifier of the software. It consists of a triplet.
Example:
top.openkylin.demo</source>
        <oldsource>Unique identifier of the software. It consists of a triplet.
Example:
id: top.openkylin.demo</oldsource>
        <translation>软件唯一标识符，由三元组组成。
例：
top.openkylin.demo</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="56"/>
        <source>name</source>
        <translation></translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="57"/>
        <source>Package name, which is usually the last value of the id.Example:  demo</source>
        <oldsource>Package name, which is usually the last value of the id.Example:  name: demo</oldsource>
        <translation>软件包名称，通常是id的最后一部分。例：demo</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="60"/>
        <source>Package name, which is usually the last value of the id.
Example:
demo</source>
        <oldsource>Package name, which is usually the last value of the id.
Example:
name: demo</oldsource>
        <translation>软件包名称，通常是id的最后一部分。
例：
demo</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="65"/>
        <source>version</source>
        <translation></translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="66"/>
        <source>Software version number.Example:  1.0.1</source>
        <oldsource>Software version number.Example:  version: 1.0.1</oldsource>
        <translation>软件版本号。例：version: 1.0.1</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="69"/>
        <source>Software version number.
Example:
1.0.1</source>
        <oldsource>Software version number.
Example:
version: 1.0.1</oldsource>
        <translation>软件包版本号
例：
1.0.0</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="74"/>
        <source>kind</source>
        <translation></translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="75"/>
        <source>Software package type, including app, base, runtime, and depend.</source>
        <translation>软件包类型，包括app、base、runtime、depend</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="78"/>
        <source>description</source>
        <translation></translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="79"/>
        <source>A brief description of the package. Example:  Demo for test</source>
        <oldsource>A brief description of the package. Example:  description: Demo for test</oldsource>
        <translation>对软件包的简单介绍。例：用于测试的软件包</translation>
    </message>
    <message>
        <location filename="packageframe.cpp" line="82"/>
        <source>A brief description of the package.
Example:
Demo for test</source>
        <oldsource>A brief description of the package.
Example:
description: Demo for test</oldsource>
        <translation>对软件包的简单介绍。
例：
用于测试的软件包。</translation>
    </message>
</context>
<context>
    <name>PermissionDialog</name>
    <message>
        <source>💡 Please enable the following permissions based on your app&apos;s needs. Disabled permissions will restrict corresponding features.</source>
        <translation type="vanished">💡 请根据应用实际需求开启以下权限，未授权将禁止应用使用对应功能</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="22"/>
        <source>💡 Please enable the following permissions according to the actual needs of the application. Unauthorized access will prohibit the application from using the corresponding functions.</source>
        <translation>设置应用运行所需权限。💡 请根据应用实际需求开启以下权限，未授权将禁止应用使用对应功能</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="43"/>
        <source>filesystem</source>
        <translation>文件系统</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="46"/>
        <source>xdg-user-dirs</source>
        <translation>挂载XDG用户目录</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="50"/>
        <source>Only mount standard user directories (desktop, documents, images, music, videos, downloads, etc.)</source>
        <translation>仅挂载标准用户目录(桌面、文档、图片、音乐、视频、下载等)</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="55"/>
        <source>Mount home directory</source>
        <translation>挂载家目录</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="59"/>
        <source>If not checked, the host home directory cannot be used</source>
        <translation>如果不勾选，则不能使用主机家目录</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="76"/>
        <source>filesystem-dirs</source>
        <translation>额外挂载路径</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="79"/>
        <source>User-specified directory paths. Example: /path/a, /path/b</source>
        <translation>用户自定需访问的目录路径.例如: /path/a, /path/b</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="82"/>
        <source>User-specified directory paths, Separated with commas.
Example:
/path/a, /path/b</source>
        <translation>用户自定需访问的目录路径，用逗号分隔.
例如:
/path/a, /path/b</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="90"/>
        <source>net</source>
        <translation>网络</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="92"/>
        <source>network</source>
        <translation>网络访问</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="95"/>
        <source>scanning</source>
        <translation>网络扫描</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="101"/>
        <source>device</source>
        <translation>设备</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="103"/>
        <source>bluetooth</source>
        <translation>蓝牙</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="106"/>
        <source>microphone</source>
        <translation>扬声器</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="109"/>
        <source>camera</source>
        <translation>摄像头</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="112"/>
        <source>usb</source>
        <translation>USB设备</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="115"/>
        <source>serial</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="121"/>
        <source>processManager</source>
        <translation>进程管理</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="124"/>
        <source>autostart</source>
        <translation>允许应用设置自启动</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="125"/>
        <source>If not checked, the application settings will not take effect upon self startup</source>
        <translation>如果不勾选，则应用内设置开机自启不生效</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="137"/>
        <source>others</source>
        <translation>其他</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="139"/>
        <source>system-notice</source>
        <translation>系统通知</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="142"/>
        <source>system-time</source>
        <translation>系统时间设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="145"/>
        <source>system-server</source>
        <translation>systemd服务设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="148"/>
        <source>prevent-sleep</source>
        <translation>允许阻塞关机、重启、睡眠</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="151"/>
        <source>system-display-resolution</source>
        <translation>显示分辨率设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="154"/>
        <source>system-display-brightness</source>
        <translation>显示亮度设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="157"/>
        <source>system-audio-setting</source>
        <translation>声音设置</translation>
    </message>
    <message>
        <source>system-audio-size</source>
        <translation type="vanished">音量设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="160"/>
        <source>system-crontab</source>
        <translation>定时任务设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="163"/>
        <source>system-power-mode</source>
        <translation>电源使用模式设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="166"/>
        <source>system-power-policy</source>
        <translation>睡眠、休眠策略设置</translation>
    </message>
    <message>
        <location filename="permissiondialog.cpp" line="177"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
</context>
<context>
    <name>PermissionWidget</name>
    <message>
        <location filename="permissionwidget.cpp" line="20"/>
        <source>💡 Please enable the following permissions according to the actual needs of the application. Unauthorized access will prohibit the application from using the corresponding functions.</source>
        <translation>设置应用运行所需权限。💡 请根据应用实际需求开启以下权限，未授权将禁止应用使用对应功能</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="41"/>
        <source>filesystem</source>
        <translation>文件系统</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="44"/>
        <source>xdg-user-dirs</source>
        <translation>挂载XDG用户目录</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="48"/>
        <source>Only mount standard user directories (desktop, documents, images, music, videos, downloads, etc.)</source>
        <translation>仅挂载标准用户目录(桌面、文档、图片、音乐、视频、下载等)</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="53"/>
        <source>Mount home directory</source>
        <translation>挂载家目录</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="57"/>
        <source>If not checked, the host home directory cannot be used</source>
        <translation>如果不勾选，则不能使用主机家目录</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="74"/>
        <source>filesystem-dirs</source>
        <translation>额外挂载路径</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="77"/>
        <source>User-specified directory paths. Example: /path/a, /path/b</source>
        <translation>用户自定需访问的目录路径.例如: /path/a, /path/b</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="80"/>
        <source>User-specified directory paths, Separated with commas.
Example:
/path/a, /path/b</source>
        <translation>用户自定需访问的目录路径，用逗号分隔.
例如:
/path/a, /path/b</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="88"/>
        <source>net</source>
        <translation>网络</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="90"/>
        <source>network</source>
        <translation>网络访问</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="93"/>
        <source>scanning</source>
        <translation>网络扫描</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="99"/>
        <source>device</source>
        <translation>设备</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="101"/>
        <source>bluetooth</source>
        <translation>蓝牙</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="104"/>
        <source>microphone</source>
        <translation>扬声器</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="107"/>
        <source>camera</source>
        <translation>摄像头</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="110"/>
        <source>usb</source>
        <translation>USB设备</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="113"/>
        <source>serial</source>
        <translation>串口</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="119"/>
        <source>processManager</source>
        <translation>进程管理</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="122"/>
        <source>autostart</source>
        <translation>允许应用设置自启动</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="123"/>
        <source>If not checked, the application settings will not take effect upon self startup</source>
        <translation>如果不勾选，则应用内设置开机自启不生效</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="135"/>
        <source>others</source>
        <translation>其他</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="137"/>
        <source>system-notice</source>
        <translation>系统通知</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="140"/>
        <source>system-time</source>
        <translation>系统时间设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="143"/>
        <source>system-server</source>
        <translation>systemd服务设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="146"/>
        <source>prevent-sleep</source>
        <translation>允许阻塞关机、重启、睡眠</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="149"/>
        <source>system-display-resolution</source>
        <translation>显示分辨率设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="152"/>
        <source>system-display-brightness</source>
        <translation>显示亮度设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="155"/>
        <source>system-audio-setting</source>
        <translation>声音设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="158"/>
        <source>system-crontab</source>
        <translation>定时任务设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="161"/>
        <source>system-power-mode</source>
        <translation>电源使用模式设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="164"/>
        <source>system-power-policy</source>
        <translation>睡眠、休眠策略设置</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="181"/>
        <source>Back</source>
        <translation>上一步</translation>
    </message>
    <message>
        <location filename="permissionwidget.cpp" line="185"/>
        <source>Next</source>
        <translation>下一步</translation>
    </message>
</context>
<context>
    <name>ProjectPathPage</name>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="17"/>
        <source>Project Location</source>
        <translation>项目路径</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="19"/>
        <source>This wizard is used to convert Debian source code to Kaiming project.</source>
        <translation>此向导用于将Debian源代码转换为Kaiming项目</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="25"/>
        <source>Project ID</source>
        <translation>项目ID</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="27"/>
        <source>Such as : top.openkylin.demo</source>
        <translation>形如：top.openkylin.demo</translation>
    </message>
    <message>
        <source>Create Path</source>
        <translation type="vanished">创建路径</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="38"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="35"/>
        <source>Debian Source</source>
        <translation>Debian源码</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="57"/>
        <source>Please select debian source path</source>
        <translation>请选择Debian源码路径</translation>
    </message>
    <message>
        <source>Please select project path</source>
        <translation type="vanished">请选择项目路径</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="95"/>
        <source>Id can&apos;t be longer than 255 characters</source>
        <translation>Id长度不能超过255个字符</translation>
    </message>
    <message>
        <source>Id is wrong! Elements may not begin with a digit.</source>
        <translation type="vanished">Id错误！每一段组成都不能以数字开头。</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="132"/>
        <source>Id is wrong! Notes:
    a）Ids are composed of 3 or more elements separated by a period(&apos;.&apos;) character.
    b) Each element must only contain the ASCII characters &apos;[A-Z][a-z][0-9]_-&apos;.
</source>
        <translation>Id错误！注意：
    a） ID由3个或更多由句点（&apos;.&apos;）字符分隔的元素组成。
    b） 每个组成部分只能包含ASCII字符: [A-Z][A-Z][0-9]_-.。</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="154"/>
        <source>Project ID and Path cannot be empty.</source>
        <translation>项目ID和项目路径不能为空</translation>
    </message>
    <message>
        <location filename="debianconvert/projectpathpage.cpp" line="169"/>
        <source>You do not have write permission in this directory!</source>
        <translation>您在此目录中没有写入权限！</translation>
    </message>
</context>
<context>
    <name>QObject</name>
    <message>
        <location filename="mainwindow.cpp" line="93"/>
        <source>kaiming-builder-gui</source>
        <translation>开明构建工具</translation>
    </message>
    <message>
        <source>Please check network</source>
        <translation type="vanished">请检查网络</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="195"/>
        <location filename="componentwidget.cpp" line="203"/>
        <location filename="componentwidget.cpp" line="210"/>
        <location filename="componentwidget.cpp" line="538"/>
        <location filename="debianconvert/generateprojectpage.cpp" line="103"/>
        <location filename="debianconvert/installenvpage.cpp" line="94"/>
        <location filename="debianconvert/installenvpage.cpp" line="170"/>
        <location filename="debianconvert/installenvpage.cpp" line="182"/>
        <location filename="debianconvert/projectpathpage.cpp" line="154"/>
        <location filename="debianconvert/projectpathpage.cpp" line="160"/>
        <location filename="debianconvert/projectpathpage.cpp" line="169"/>
        <location filename="debianconvert/selectenvpage.cpp" line="92"/>
        <location filename="manifestwidget.cpp" line="303"/>
        <location filename="manifestwidget.cpp" line="314"/>
        <location filename="manifestwidget.cpp" line="326"/>
        <location filename="manifestwidget.cpp" line="337"/>
        <location filename="manifestwidget.cpp" line="348"/>
        <location filename="manifestwidget.cpp" line="359"/>
        <location filename="manifestwidget.cpp" line="370"/>
        <location filename="manifestwidget.cpp" line="433"/>
        <location filename="selectdependsdialog.cpp" line="627"/>
        <location filename="sourcedialog.cpp" line="119"/>
        <location filename="sourcedialog.cpp" line="132"/>
        <location filename="sourcedialog.cpp" line="141"/>
        <location filename="sourcedialog.cpp" line="154"/>
        <location filename="sourcedialog.cpp" line="167"/>
        <location filename="startwidget.cpp" line="237"/>
        <location filename="startwidget.cpp" line="243"/>
        <location filename="startwidget.cpp" line="253"/>
        <location filename="startwidget.cpp" line="261"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="196"/>
        <source>Please set component name</source>
        <oldsource>Please set name</oldsource>
        <translation>请设置component name</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="204"/>
        <source>When buildsystem is simple, must set build-script</source>
        <translation>当buildsystem=simple时，必须设置build-script</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="211"/>
        <source>Please set sources</source>
        <translation>请设置sources</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="539"/>
        <source>Please set component name first</source>
        <translation>请优先设置component name。</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="304"/>
        <source>Please set project path</source>
        <translation>请设置项目路径</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="327"/>
        <source>Please set project name</source>
        <translation>请设置项目名称</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="338"/>
        <source>Please set package version</source>
        <oldsource>Please set project version</oldsource>
        <translation>请设置version</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="349"/>
        <source>Please set base</source>
        <translation>请设置base</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="360"/>
        <source>Please set command</source>
        <translation>请设置command</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="371"/>
        <source>Please set components</source>
        <translation>请设置components</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="749"/>
        <source>Success</source>
        <translation>成功</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="750"/>
        <source>Manifest saved successfully to:
%1</source>
        <translation>清单已成功保存到：
%1</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="753"/>
        <location filename="manifestwidget.cpp" line="777"/>
        <source>Error</source>
        <translation>错误</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="754"/>
        <source>Failed to save manifest to:
%1</source>
        <translation>未能将清单保存到：
%1</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="778"/>
        <source>Failed to load manifest from:
%1
%2</source>
        <translation>未能从以下位置加载清单：
%1
%2</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="783"/>
        <source>File Not Found</source>
        <translation>找不到文件</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="784"/>
        <source>Please save the manifest first to:
%1</source>
        <translation>请先将清单保存到:
%1</translation>
    </message>
    <message>
        <location filename="componentwidget.cpp" line="197"/>
        <location filename="componentwidget.cpp" line="205"/>
        <location filename="componentwidget.cpp" line="212"/>
        <location filename="componentwidget.cpp" line="540"/>
        <location filename="debianconvert/generateprojectpage.cpp" line="103"/>
        <location filename="debianconvert/installenvpage.cpp" line="73"/>
        <location filename="debianconvert/installenvpage.cpp" line="96"/>
        <location filename="debianconvert/installenvpage.cpp" line="174"/>
        <location filename="debianconvert/installenvpage.cpp" line="186"/>
        <location filename="debianconvert/projectpathpage.cpp" line="154"/>
        <location filename="debianconvert/projectpathpage.cpp" line="160"/>
        <location filename="debianconvert/projectpathpage.cpp" line="169"/>
        <location filename="debianconvert/selectenvpage.cpp" line="92"/>
        <location filename="manifestwidget.cpp" line="305"/>
        <location filename="manifestwidget.cpp" line="316"/>
        <location filename="manifestwidget.cpp" line="328"/>
        <location filename="manifestwidget.cpp" line="339"/>
        <location filename="manifestwidget.cpp" line="350"/>
        <location filename="manifestwidget.cpp" line="361"/>
        <location filename="manifestwidget.cpp" line="372"/>
        <location filename="manifestwidget.cpp" line="435"/>
        <location filename="manifestwidget.cpp" line="751"/>
        <location filename="manifestwidget.cpp" line="755"/>
        <location filename="manifestwidget.cpp" line="780"/>
        <location filename="manifestwidget.cpp" line="786"/>
        <location filename="sourcedialog.cpp" line="121"/>
        <location filename="sourcedialog.cpp" line="134"/>
        <location filename="sourcedialog.cpp" line="143"/>
        <location filename="sourcedialog.cpp" line="156"/>
        <location filename="sourcedialog.cpp" line="169"/>
        <location filename="startwidget.cpp" line="237"/>
        <location filename="startwidget.cpp" line="243"/>
        <location filename="startwidget.cpp" line="253"/>
        <location filename="startwidget.cpp" line="261"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="315"/>
        <source>Please set project id</source>
        <translation>请设置项目id</translation>
    </message>
    <message>
        <location filename="manifestwidget.cpp" line="434"/>
        <source>Not found project file : </source>
        <translation>未找到工程文件</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="120"/>
        <location filename="sourcedialog.cpp" line="133"/>
        <source>Please set url</source>
        <translation>请设置url</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="142"/>
        <source>Please set sha256</source>
        <translation>请设置sha256</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="155"/>
        <location filename="sourcedialog.cpp" line="168"/>
        <source>Please set path</source>
        <translation>请设置path</translation>
    </message>
    <message>
        <location filename="debianconvert/installenvpage.cpp" line="71"/>
        <source>Install Complete</source>
        <translation>安装完成</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="629"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="630"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
</context>
<context>
    <name>SelectDependsDialogPrivate</name>
    <message>
        <location filename="selectdependsdialog.cpp" line="226"/>
        <source>Support multiple dependencies separated by commas.</source>
        <oldsource>It can support multiple dependencies separated by commas.</oldsource>
        <translation>支持&quot;,&quot;隔开的多个depend</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="245"/>
        <source>Please Select</source>
        <translation>请选择现有depend</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="265"/>
        <source>Selected</source>
        <translation>已选择depend</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="545"/>
        <source>Base or runtime includes</source>
        <translation>base或runtime包含</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="313"/>
        <location filename="selectdependsdialog.cpp" line="321"/>
        <source>No relevant content was found in the search</source>
        <translation>未搜索到相关内容</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="577"/>
        <source>There exists in the deb source</source>
        <translation>deb源中存在</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="324"/>
        <location filename="selectdependsdialog.cpp" line="593"/>
        <source>Source code construction required</source>
        <translation>需源码捆绑构建</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="308"/>
        <source>Base or runtime already exists</source>
        <translation>base或runtime中已包含</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="316"/>
        <source>Can be converted from deb source</source>
        <translation>可从deb源中转制</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="337"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="338"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="386"/>
        <source>Please fill in the base or runtime first</source>
        <translation>请先填写base或runtime</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="402"/>
        <source>runtime and base are not installed, please install first</source>
        <translation>runtime和base都没有安装，请先安装</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="408"/>
        <source>runtime not installed, please install first</source>
        <translation>runtime没有安装，请先安装</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="417"/>
        <source>base not installed, please install first</source>
        <oldsource>base not installed, please install first</oldsource>
        <translation>base没有安装，请先安装</translation>
    </message>
    <message>
        <location filename="selectdependsdialog.cpp" line="624"/>
        <source>The depend %1 of version %2 has already been added. Do you wish to continue adding version %3.</source>
        <oldsource>The depend %1 of version %2 has already been selected. Do you wish to continue adding version %3.</oldsource>
        <translation>依赖“%1”的版本“%2”已经被添加，是否继续添加“%3”</translation>
    </message>
</context>
<context>
    <name>SelectEnvPage</name>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="16"/>
        <source>Environment Selection</source>
        <translation>环境选择</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="18"/>
        <source>Please select the build environment for your project.</source>
        <translation>请为您的项目选择构建环境</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="31"/>
        <source>Format: {channel}:{id}/{version}.Example:  stable:top.openkylin.base/1.0.0</source>
        <translation>格式: {channel}:{id}/{version}.示例: stable:top.openkylin.base/1.0.0</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="34"/>
        <source>The basic environment needed to build the package.
Format: {channel}:{id}/{version}
Example:
stable:top.openkylin.base/1.0.0</source>
        <translation>构建软件包所需基础环境
格式：{channel}:{id}/{version}
例:
stable:top.openkylin.base/1.0.0</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="48"/>
        <source>Format: {channel}:{id}/{version}.Example: stable:top.openkylin.ukui/1.0.0</source>
        <translation>格式: {channel}:{id}/{version}.示例: stable:top.openkylin.ukui/1.0.0</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="51"/>
        <source>Specific environments that complement the base environment to build package,such as cmake,qmake, etc
Format: {channel}:{id}/{version}
Example:
stable:top.openkylin.ukui/1.0.0</source>
        <translation>构建软件包所需特定环境，例如cmake、qmake等。
格式：{channel}/{id}/{version}
例：
stable:top.openkylin.ukui/1.0.0</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="64"/>
        <source>base</source>
        <translation>base</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="65"/>
        <source>runtime</source>
        <translation>runtime</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="92"/>
        <source>base cannot be empty.</source>
        <translation>base不能为空</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="102"/>
        <location filename="debianconvert/selectenvpage.cpp" line="113"/>
        <source>Invalid Format</source>
        <translation>格式错误</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="103"/>
        <source>Base format must be: {channel}:{id}/{version}
Example: stable:top.openkylin.base/1.0.0</source>
        <translation>base格式为: {channel}:{id}/{version}
例: stable:top.openkylin.base/1.0.0</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="105"/>
        <location filename="debianconvert/selectenvpage.cpp" line="116"/>
        <source>OK</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="debianconvert/selectenvpage.cpp" line="114"/>
        <source>Runtime format must be: {channel}:{id}/{version}
Example: stable:top.openkylin.ukui/1.0.0</source>
        <translation>runtime格式为: {channel}:{id}/{version}
例: stable:top.openkylin.ukui/1.0.0</translation>
    </message>
</context>
<context>
    <name>SetupInfoFrame</name>
    <message>
        <location filename="setupinfoframe.cpp" line="92"/>
        <source>channel</source>
        <translation></translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="93"/>
        <source>Each developer can create their own channels.
All the software packages installed will belong to this channel.
Example:
stable</source>
        <oldsource>Each developer can create their own channels.
All the software packages created will belong to this channel.
Example:
stable</oldsource>
        <translation>每个开发者可以创建自己的通道,安装时软件包部署在所属通道下。
例：
stable</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="99"/>
        <source>command</source>
        <oldsource>command &quot;c&quot;</oldsource>
        <translation></translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="100"/>
        <source>An executable file called by the software package. with parameters added later. If a parameter contains spaces, enclose it in quotation marks. Eexample: cmd arg1 arg2 &quot;arg3.1 arg3.2&quot; arg4</source>
        <oldsource>An executable file called by the software package. with parameters added later. If a parameter contains spaces, enclose it in quotation marks. Eexample: command: cmd arg1 arg2 &quot;arg3.1 arg3.2&quot; arg4</oldsource>
        <translation>软件包的可执行命令，参数添加在其后面，如果参数包含空格使用引号引起来。例：cmd arg1 arg2 &quot;arg3.1 arg3.2&quot; arg4</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="106"/>
        <source>An executable file called by the software package
 while it is running, with parameters added later. 
When kind is set to app, the command must be configured.If a parameter contains spaces, enclose it in quotation marks.
Eexample:
cmd arg1 arg2 &quot;arg3.1 arg3.2&quot; arg4</source>
        <oldsource>An executable file called by the software package
 while it is running, with parameters added later. 
When kind is set to app, the command must be configured.If a parameter contains spaces, enclose it in quotation marks.
Eexample:
command: cmd arg1 arg2 &quot;arg3.1 arg3.2&quot; arg4</oldsource>
        <translation>软件包的可执行命令，参数添加在其后面，如果参数包含空格使用引号引起来。
例：
cmd arg1 arg2 &quot;arg3.1 arg3.2&quot; arg4</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="119"/>
        <source>cleanup</source>
        <translation></translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="120"/>
        <source>Files to be deleted after the software package is builded.
 Wildcard characters are supported. Separated by newlines.
Example:
 /path/a 
 *.so 
 /path/b </source>
        <translation>软件包构建完成后需要删除的文件，支持通配符，由换行符隔开。
例：
 /path/a 
 *.so 
 /path/b </translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="132"/>
        <source>modules</source>
        <translation></translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="133"/>
        <source>After the completion of the construction of the need to export the module, there are two types of binary,devel 
binary for running, devel for development.depend only has devel module, app only has binary module. 
Example :
 binary:&quot;.*&quot;</source>
        <translation>软件包构建完成后需要导出到指定模块。当前支持的模块有binary和devel两种。binary用于运行，devel用于开发。
depend只有devel模块。app只有binary模块。
例：
 binary:&quot;.*&quot;</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="137"/>
        <source>There are two types of modules,including binary and devel. It is binary by default.Binary for running, devel for development, depend only has devel module, app only has binary module.  Example: binary:&quot;.*&quot;.</source>
        <oldsource>There are two types of modules,including binary,devel. It is binary by default.Binary for running, devel for development, depend only has devel module, app only has binary module.  Example: binary:&quot;.*&quot;.</oldsource>
        <translation>软件包构建完成后需要导出到指定模块。当前支持的模块有binary和devel两种。binary用于运行，devel用于开发。depend只有devel模块。app只有binary模块。例：binary:&quot;.*&quot;</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="113"/>
        <source>Files to be deleted after the software package is builded. Wildcard characters are supported. Separated by newlines. Example:  /path/a \n *.so \n /path/b </source>
        <oldsource>Files to be deleted after the software package is builded. Wildcard characters are supported. Separated by newlines. Example: cleanup:  /path/a \n *.so \n /path/b </oldsource>
        <translation>软件包构建完成后需要删除的文件，支持通配符，以换行符隔开，例：/path/a \n *.so \n /path/b</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="155"/>
        <source>Add module</source>
        <translation>添加module</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="161"/>
        <source>env</source>
        <translation></translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="163"/>
        <source>Add environment variables.
Key is the name of the environment variable, and value is the value of environment variable.
Example: 
 PATH: /usr/local/bin</source>
        <translation>添加环境变量，Key是变量名，Value是值。
例：
 PATH: /usr/local/bin</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="167"/>
        <source>Add environment variables. Key is the name of the environment variable, and value is the value of environment variable. Example:  PATH: /usr/local/bin.</source>
        <oldsource>Add environment variables.
Key is the name of the environment variable, and value is the value of environment variable.
Example: 
env:
 PATH: /usr/local/bin</oldsource>
        <translation>添加环境变量，key是变量名，value是值。例：PATH: /usr/local/bin</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="196"/>
        <source>Add env</source>
        <translation>添加环境变量</translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="215"/>
        <source>devel</source>
        <translation></translation>
    </message>
    <message>
        <location filename="setupinfoframe.cpp" line="215"/>
        <source>binary</source>
        <translation></translation>
    </message>
</context>
<context>
    <name>SourceDialog</name>
    <message>
        <source>A series of script commands that are executed after the source code is extracted 
Commands are separated by newlines 
Example:
 mv *.cpp files 
 rm -rf data 
 install -D demo ... 
</source>
        <translation type="vanished">代码提取完成后执行的一系列脚本命令，以换行符分割
例：
 mv *.cpp files 
 rm -rf data 
 install -D demo ... </translation>
    </message>
    <message>
        <source>Build the component only on the selected arches</source>
        <translation type="vanished">只在列出的架构上进行构建。</translation>
    </message>
    <message>
        <source>Ignore the selected architectures, don&apos;t build the component on them</source>
        <translation type="vanished">不在列出的架构上进行构建。</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="87"/>
        <source>type</source>
        <oldsource>type:</oldsource>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="88"/>
        <source>The kind of code source</source>
        <translation>源码类别</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="109"/>
        <source>OK</source>
        <translation>确定</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="328"/>
        <source>Git project URL. Example: https://github.com/user/repo.git</source>
        <translation>git项目远程路径。例：https://github.com/user/repo.git</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="331"/>
        <source>Git repository URL, 
Example: 
https://github.com/user/repo.git</source>
        <translation>git项目远程路径。
例：
https://github.com/user/repo.git</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="334"/>
        <location filename="sourcedialog.cpp" line="396"/>
        <source>url</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="338"/>
        <source>The branch to which a Git project belongs in the repository. Example:  master</source>
        <translation>git项目所在分支branch。例：master</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="341"/>
        <source>The branch to which a Git project belongs in the repository.
Example:
master</source>
        <translation>git项目所在分支branch。
例：
master</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="344"/>
        <source>branch</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="348"/>
        <source>The tag of a git project in the repository. Example: tag_test/v1.0.0</source>
        <translation>git项目的一次标记tag。例：tag_test/v1.0.0</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="351"/>
        <source>The tag of a git project in the repository.
Example:
tag_test/v1.0.0</source>
        <translation>git项目的一次标记tag。
例：
tag_test/v1.0.0</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="354"/>
        <source>tag</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="358"/>
        <source>The commit to use in the git project. Example: d3b07384d113edec49eaa6238ad5ff0</source>
        <translation>git项目中一次提交的commit哈希值。例：d3b07384d113edec49eaa6238ad5ff0</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="361"/>
        <source>If a branch, a tag, and a commit are set simultaneously, the priority order is commit &gt; tag &gt; branch.
Example:
d3b07384d113edec49eaa6238ad5ff0</source>
        <oldsource>The commit to use in the git project. 
If a branch is also specified, it verifies that the branch/tag is in that particular commit.
Example:
d3b07384d113edec49eaa6238ad5ff0</oldsource>
        <translation>git项目中一次提交的哈希值。如果branch,tag,commit同时被设置，优先顺序:commit &gt; tag &gt; branch。
例：
d3b07384d113edec49eaa6238ad5ff0</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="364"/>
        <source>commit</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="390"/>
        <source>Remote archive address. Example: https://....../dir/file.zip</source>
        <translation>archive文件远程地址，例：https://....../dir/file.zip</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="393"/>
        <source>Remote archive address (if path is also set, url is preferred),The remote address can be http, https, ftp, or ftps
Example:
https://....../dir/file.zip</source>
        <translation>archive文件的远程地址（如果path与url同时设置,url优先）。远程地址支持http，https，ftp，ftps协议。
例：
https://....../dir/file.zip</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="401"/>
        <source>sha256 checksum of the file.
Example:
962eb48e42ae130da99c7909a4c11a93c9799d62932c8531cb78f646a35f653c</source>
        <translation>文件的sha256校验和。
例：
962eb48e42ae130da99c7909a4c11a93c9799d62932c8531cb78f646a35f653c</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="404"/>
        <source>sha256</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="442"/>
        <location filename="sourcedialog.cpp" line="512"/>
        <source>Original Path</source>
        <translation>源码路径</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="451"/>
        <source>The path of the local directory to be copied
Example:
./src</source>
        <translation>待拷贝目录的本地路径。
例：
./src</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="458"/>
        <source>Ignored files/directories when copying. Example: *.so, src</source>
        <translation>拷贝目录时无需拷贝的文件或文件夹。例：*.so, src</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="461"/>
        <source>Ignored files/directories, Wildcards are supported, Separated with commas.
Example:
*.so, src</source>
        <translation>拷贝目录时无需拷贝的文件或文件夹。支持通配符，以逗号隔开。
例：
*.so, src</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="464"/>
        <source>skip</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="472"/>
        <location filename="sourcedialog.cpp" line="563"/>
        <source>Please select source path</source>
        <translation>请选择源码路径。</translation>
    </message>
    <message>
        <source>A series of script commands that are executed after the source code is extracted. Example: mv *.cpp files \n rm -rf data </source>
        <oldsource>A series of script commands that are executed after the source code is extracted. Example: command: mv *.cpp files \n rm -rf data </oldsource>
        <translation type="vanished">源码提取完成后执行的一系列脚本命令。例：mv *.cpp files \n rm -rf data </translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="400"/>
        <source>sha256 checksum of the file </source>
        <oldsource>sha256 checksum of the file (optional with sha512)</oldsource>
        <translation>文件的sha256校验和</translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="454"/>
        <location filename="sourcedialog.cpp" line="522"/>
        <source>path</source>
        <translation></translation>
    </message>
    <message>
        <location filename="sourcedialog.cpp" line="521"/>
        <source>The path of the local file to be copied, read-only</source>
        <oldsource>The path of the local file to be copied
Example:
path: ./demo.sh</oldsource>
        <translation>本地待拷贝文件路径。
例：
path: ./demo.sh</translation>
    </message>
</context>
<context>
    <name>StartWidget</name>
    <message>
        <location filename="startwidget.cpp" line="22"/>
        <source>Projects</source>
        <translation>项目</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="32"/>
        <source>New</source>
        <translation>新建项目</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="33"/>
        <source>Open</source>
        <translation>打开项目</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="34"/>
        <source>Debian Source Convert</source>
        <translation>Debian源码转换</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="51"/>
        <source>kaiming-builder-gui</source>
        <translation>开明构建工具</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="67"/>
        <source>kaiming-builder-gui is a graphical version of kaiming-builder, providing developers with a more convenient way to write build manifest files and build source code into Kaiming packages</source>
        <translation>开明可视化构建工具是命令行构建工具kaiming-builder的图形化版本，为开发者提供更便捷的方式编写构建清单文件，将源码构建为开明包</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="118"/>
        <source>Id can&apos;t be longer than 255 characters</source>
        <translation>Id长度不能超过255个字符</translation>
    </message>
    <message>
        <source>Id is wrong! Elements may not begin with a digit.</source>
        <translation type="vanished">Id错误！每一段组成都不能以数字开头。</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="155"/>
        <source>Id is wrong! Notes:
    a）Ids are composed of 3 or more elements separated by a period(&apos;.&apos;) character.
    b) Each element must only contain the ASCII characters &apos;[A-Z][a-z][0-9]_-&apos;.
</source>
        <translation>Id错误！注意：
    a） ID由3个或更多由句点（&apos;.&apos;）字符分隔的元素组成。
    b） 每个组成部分只能包含ASCII字符: [A-Z][A-Z][0-9]_-.。</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="172"/>
        <location filename="startwidget.cpp" line="211"/>
        <source>Please select project path</source>
        <translation>请选择一个项目路径</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="181"/>
        <source>Project ID</source>
        <translation>应用id</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="183"/>
        <source>Such as : top.openkylin.demo</source>
        <translation>形如：top.openkylin.demo，3个或更多由.分割的部分组成</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="187"/>
        <source>Create Path</source>
        <translation>创建路径</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="189"/>
        <source>Browse</source>
        <translation>浏览</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="199"/>
        <source>Confirm</source>
        <translation>确认</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="200"/>
        <source>Cancel</source>
        <translation>取消</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="237"/>
        <source>Project ID and Path cannot be empty.</source>
        <translation>项目ID和路径不能为空</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="253"/>
        <source>The directory already exists. Please clean it up manually.</source>
        <translation>目录已存在，请手动清理。</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="261"/>
        <source>Failed to create the directory. Please check the path.</source>
        <translation>创建目录失败，请检查路径。</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="280"/>
        <source>Please select YAML file</source>
        <translation>请选择YAML文件</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="282"/>
        <source>YAML Files (*.km.yaml *.km.yml)</source>
        <translation></translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="379"/>
        <source>Warning</source>
        <translation>警告</translation>
    </message>
    <message>
        <location filename="startwidget.cpp" line="380"/>
        <source>The project file does not exist.</source>
        <translation>该项目文件不存在</translation>
    </message>
</context>
</TS>
