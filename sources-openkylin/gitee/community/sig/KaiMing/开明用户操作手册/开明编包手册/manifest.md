# 清单说明

``kaiming`` 使用``json``或``yaml``格式的manifest文件来描述如何从源代码构建应用程序及其依赖项。清单由``kaiming-builder``使用。

## 一、顶级属性

这些是可接受的属性：

`id`或`app-id`（字符串）

定义应用程序 ID 的字符串。

`branch`（字符串）

导出应用程序时要使用的分支。如果未设置，则默认值来自 default-branch 选项。

该键会覆盖 default-branch 键和 --default-branch 命令行选项。除非您需要非常具体的分支名称（例如运行时或扩展），否则建议使用 default-branch 键，因为您可以在构建测试版本时使用 --default-branch 覆盖默认值。

`default-branch`（字符串）

导出应用程序时使用的默认分支。默认为主。

该键可以通过 --default-branch 命令行选项覆盖。

`collection-id`（字符串）

存储库的集合 ID，默认未设置。设置全局唯一的集合 ID 允许存储库中的应用程序在对等系统上共享，而无需进一步配置。如果在现有存储库中构建，则集合 ID 必须与该存储库的现有配置集合 ID 匹配。

`extension-tag`（字符串）

如果构建扩展，则为要使用的扩展点的标签。从 kaiming 0.11.4 开始，运行时可以为同一扩展点定义多个位置，以便在每个位置安装扩展的不同分支。构建扩展时，有必要知道要将扩展安装到哪个扩展点。此选项解决了选择扩展点的任何歧义。如果未指定，则默认选择是安装到扩展点的唯一位置或安装到未标记的扩展点的位置。

`token-type`（整数）

安装此提交所需的令牌类型。将其设置为大于 0 的值意味着安装 kaiming 时需要进行身份验证。

`runtime`（字符串）

应用程序使用的运行时的名称。

`runtime-version`（字符串）

应用程序使用的运行时版本，默认为 master。

`sdk`（字符串）

构建应用程序所使用的开发运行时的名称。

`var`（字符串）

使用此运行时的副本初始化构建中的（否则为空）可写 /var。

`metadata`（字符串）

完成时使用此文件作为基础元数据文件。

`command`（字符串）

应用程序主二进制文件的文件名或路径。请注意，这实际上只是一个文件，而不是命令行。如果要传递参数，请安装 shell 脚本包装器并将其用作命令。

**另请注意**，当应用程序通过kaiming run 运行时使用该命令 ，并且不会影响应用程序以其他方式（例如通过桌面文件或 D-Bus 激活）运行时执行的内容。

`build-runtime`（布尔值）

构建新的运行时而不是应用程序。

`build-extension`（布尔值）

构建一个扩展。

`separate-locales`（布尔值）

将语言环境文件和翻译分离到扩展运行时。默认为 true。

`id-platform`（字符串）

构建运行时 sdk 时，还要使用此 id 创建基于它的平台。

`metadata-platform`（字符串）

用于我们创建的平台的元数据文件。

`writable-sdk`（布尔值）

如果为 true，则使用 /usr 的 sdk 的可写副本。如果指定了构建运行时，则默认为 true。

`appstream-compose`（布尔值）

在清理阶段运行 appstream-compose。默认为 true。

`sdk-extensions`（字符串数组）

在 /usr 中安装这些额外的 sdk 扩展。

`platform-extensions`（字符串数组）

创建平台时安装这些额外的 sdk 扩展。

`base`（字符串）

从指定应用程序中的文件开始。这可用于创建扩展另一个应用程序的应用程序。

`base-version`（字符串）

使用在 base 中指定的应用程序的特定版本。如果未指定，则使用分支中指定的值

`base-extensions`（字符串数组）

初始化应用程序目录时，从基本应用程序安装这些额外的扩展。

`inherit-extensions`（字符串数组）

完成构建后，从基础应用程序或 sdk 继承这些额外的扩展点。

`inherit-sdk-extensions`（字符串数组）

完成构建后，从基础应用程序或 sdk 继承这些额外的扩展点，但不要将它们继承到平台中。

`tags`（字符串数组）

将这些标签添加到元数据文件中。

`build-options`（对象）

指定构建环境的对象。详情请参阅下文。

`modules`（对象或字符串数​​组）

指定要按顺序构建的模块的对象数组。数组中的字符串成员被解释为包含模块的单独 json 或 yaml 文件的名称。详情请参阅下文。

`add-extensions`（对象）

这是扩展对象的字典。关键是扩展名。详情请参阅下文。

`add-build-extensions`（对象）

这是一个扩展对象的字典，类似于添加扩展。主要区别在于扩展是提前添加的并且可以在构建期间使用。

`cleanup`（字符串数组）

应在最后删除的文件模式数组。以 / 开头的模式被视为完整路径名（不带 /app 前缀），否则它们仅与基本名称匹配。

`cleanup-commands`（字符串数组）

在清理阶段运行的一组命令行。

`cleanup-platform`（字符串数组）

平台中需要清理的额外文件。

`cleanup-platform-commands`（字符串数组）

在平台清理阶段运行的一组命令行。

`prepare-platform-commands`（字符串数组）

在导入基础平台之后、应用 sdk 中的新文件之前运行的一组命令行。这是一个好地方，例如从基础中删除可能与 sdk 中添加的文件冲突的内容。

`finish-args`（字符串数组）

**传递给kaiming build-finish**命令的参数数组。

`rename-desktop-file`（字符串）

任何具有此名称的桌面文件都将在清理阶段重命名为基于 id 的名称。

`rename-appdata-file`（字符串）

任何具有此名称的 appdata 文件都将在清理阶段重命名为基于 id 的名称。

`rename-icon`（字符串）

任何具有此名称的图标都将在清理阶段重命名为基于 id 的名称。请注意，这是图标名称，而不是完整的文件名，因此它不应包含文件扩展名。

`appdata-license`（字符串）

将 appdata project\_license 字段替换为该字符串。这很有用，因为上游许可证通常仅与应用程序本身有关，而捆绑的应用程序也可以包含其他许可证。

`copy-icon`（布尔值）

如果设置了 rename-icon，请保留旧图标文件的副本。

`desktop-file-name-prefix`（字符串）

该字符串将作为主应用程序桌面文件中 Name 键的前缀。

`desktop-file-name-suffix`（字符串）

该字符串将作为主应用程序桌面文件中 Name 键的后缀。


## 二、构建选项:build-options

build-options指定模块的构建环境，可以全局指定，也可以按模块指定。也可以使用arch特性在每个体系结构的基础上指定选项。

这些是可接受的属性：

`cflags`（字符串）

这是在构建期间在环境变量 CFLAGS 中设置的。多个规范（例如在每个拱形区域中）被连接起来，并用空格分隔。

`cflags-override`（布尔值）

如果是这样，请先清除以前的构建选项中的 cflags，然后再从这些选项中添加它。

`cppflags`（字符串）

这是在构建期间在环境变量 CPPFLAGS 中设置的。多个规范（例如在每个拱形区域中）被连接起来，并用空格分隔。

`cppflags-override`（布尔值）

如果是这样，请先从以前的构建选项中清除 cppflags，然后再从这些选项中添加它。

`cxxflags`（字符串）

这是在构建期间在环境变量 CXXFLAGS 中设置的。多个规范（例如在每个拱形区域中）被连接起来，并用空格分隔。

`cxxflags-override`（布尔值）

如果是这样，请先从以前的构建选项中清除 cxxflags，然后再从这些选项中添加它。

`ldflags`（字符串）

这是在构建期间在环境变量 LDFLAGS 中设置的。多个规范（例如在每个拱形区域中）被连接起来，并用空格分隔。

`ldflags-override`（布尔值）

如果是这样，请先从以前的构建选项中清除 ldflags，然后再从这些选项中添加它。

`prefix`（字符串）

模块的构建前缀（默认为`/app`应用程序和`/usr`运行时）。

`libdir`（字符串）

模块的构建 libdir（默认用于`/app/lib`应用程序和`/usr/lib`运行时）。

`append-path`（字符串）

这将被附加到构建环境中的 PATH（如果需要，带有前导冒号）。

`prepend-path`（字符串）

这将被添加到构建环境中的 PATH 前面（如果需要，可以使用尾随冒号）。

`append-ld-library-path`（字符串）

这将被附加到构建环境中的 LD\_LIBRARY\_PATH（如果需要，带有前导冒号）。

`prepend-ld-library-path`（字符串）

这将被添加到构建环境中的 LD\_LIBRARY\_PATH 之前（如果需要，带有尾随冒号）。

`append-pkg-config-path`（字符串）

这将被附加到构建环境中的 PKG\_CONFIG\_PATH（如果需要，带有前导冒号）。

`prepend-pkg-config-path`（字符串）

这将被添加到构建环境中的 PKG\_CONFIG\_PATH 之前（如果需要，带有尾随冒号）。

`env`（对象）

这是定义在构建期间设置的环境变量的字典。其中的元素会覆盖设置环境的属性，例如 cflags 和 ldflags。具有空值的键会取消设置相应的变量。

`secret-env`（字符串数组）

这是一个数组，定义将哪些主机环境变量传输到构建命令或安装后环境。

`build-args`（字符串数组）

这是一个包含传递给 kaiming 构建的额外选项的数组。

`test-args`（字符串数组）

与 build-args 类似，但影响测试，而不是正常构建。

`config-opts`（字符串数组）

这是一个包含传递给配置的额外选项的数组。

`secret-opts`（字符串数组）

这是将传递给配置的选项数组，旨在用于通过主机环境变量传递机密。将选项放在环境变量中并会预先解决。'-DSECRET\_ID=$CI\_SECRET'

`make-args`（字符串数组）

将传递给 make 的额外参数数组

`make-install-args`（字符串数组）

将传递给 make install 的额外参数数组

`strip`（布尔值）

如果这是 true（默认为 false），那么所有 ELF 文件将在安装后被删除。

`no-debuginfo`（布尔值）

默认情况下（如果 strip 不为 true），kaiming-builder 将 ELF 文件中的所有调试信息提取到单独的文件中，并将其放入扩展名中。如果要禁用此功能，请将 no-debuginfo 设置为 true。

`no-debuginfo-compression`（布尔值）

默认情况下，当提取调试信息时，我们会压缩调试部分。如果要禁用此功能，请将 no-debuginfo-compression 设置为 true。

`arch`（对象）

这是一个字典，为每个架构定义了一个单独的构建选项对象，该对象会覆盖主构建选项对象。

## 三、扩展

扩展定义了应用程序/运行时中可以通过扩展实现的扩展点，提供运行时可用的额外文件。

这些是可接受的属性：

`directory`（字符串）

扩展安装的目录。如果扩展点用于应用程序，则此路径相对于 /app，否则相对于 /usr。

`bundle`（布尔值）

如果这是真的，那么在扩展目录中创建的数据将从结果中省略，而是打包在单独的扩展中。

`remove-after-build`（布尔值）

如果这是真的，则扩展将在完成时被删除。这仅对 add-build-extensions 属性中的扩展有意义。

此外，还支持标准的 kaiming 扩展属性，并直接放入元数据文件中：autodelete、no-autodownload、subdirectories、add-ld-path、download-if、enable-if、merge-dirs、subdirectory-suffix、locale-subset ，版本，版本。有关这些的更多信息，请参阅 kaiming 元数据文档。

## 四、模块：modules

每个模块指定一个必须单独构建和安装的源。它包含构建选项以及构建之前要下载和提取的源列表。

模块可以嵌套，以便用一个键打开和关闭相关模块。

这些是可接受的属性：

`name`（字符串）

模块的名称，例如构建日志中使用的。该名称还用于构造文件名和命令行参数，因此在此字符串中使用空格或“/”是一个坏主意。

`disabled`（布尔值）

如果为真，则跳过此模块

`sources`（对象或字符串数​​组）

定义将按顺序下载和提取的源的对象数组。数组中的字符串成员被解释为包含源的单独 json 或 yaml 文件的名称。详情请参阅下文。

`secret-env`（字符串数组）

定义将哪些主机环境变量传输到构建命令或安装后环境的数组。

`config-opts`（字符串数组）

将传递给配置的选项数组

`secret-opts`（字符串数组）

将传递给配置的选项数组，用于通过主机环境变量传递机密。将选项放在环境变量中并会预先解决。'-DSECRET\_ID=$CI\_SECRET'

`make-args`（字符串数组）

将传递给 make 的参数数组

`make-install-args`（字符串数组）

将传递给 make install 的参数数组

`rm-configure`（布尔值）

如果为 true，则在开始构建之前删除配置脚本

`no-autogen`（布尔值）

忽略 autogen 脚本的存在

`no-parallel-make`（布尔值）

不要使用参数调用 make 来并行构建

`install-rule`（字符串）

传递给安装阶段 make 的规则名称，默认为 install

`no-make-install`（布尔值）

不要运行 make install （或等效的）阶段

`no-python-timestamp-fix`（布尔值）

不要修复 \*.py\[oc\] 标头时间戳以供 ostree 使用。

`cmake`（布尔值）

使用 cmake 而不是 configure（已弃用：使用 buildsystem 代替）

`buildsystem`（字符串）

使用的构建系统：autotools、cmake、cmake-ninja、meson、simple、qmake

`builddir`（布尔值）

使用与源目录分开的构建目录

`subdir`（字符串）

在提取的源的子目录中构建

`build-options`（对象）

可以覆盖全局选项的构建选项对象

`build-commands`（字符串数组）

在构建期间运行的一组命令（如果使用了这些命令，则在 make 和 make install 之间）。这在使用“简单”构建系统时主要有用。每个命令都在 中运行`/bin/sh -c`，因此它可以使用标准 POSIX shell 语法，例如管道输出。

`post-install`（字符串数组）

在安装阶段之后运行的一系列 shell 命令。例如，可以清理安装目录，或安装额外的文件。

`cleanup`（字符串数组）

应在最后删除的文件模式数组。以 / 开头的模式被视为完整路径名（不带 /app 前缀），否则它们仅与基本名称匹配。请注意，任何模式都只会匹配此模块安装的文件。

`ensure-writable`（字符串数组）

构建器的工作方式是安装目录中的文件是到缓存文件的硬链接，因此不允许您就地修改它们。如果您在此列出文件，则硬链接将被破坏，您可以对其进行修改。这是一种解决方法，理想情况下安装文件应该替换文件，而不是修改现有文件。

`only-arches`（字符串数组）

如果非空，则仅在列出的拱门上构建模块。

`skip-arches`（字符串数组）

不要在列出的任何拱门上进行构建。

`cleanup-platform`（字符串数组）

平台中需要清理的额外文件。

`run-tests`（布尔值）

如果 true 这将在安装后运行测试。

`test-rule`（字符串）

运行测试时要构建的目标。对于 make 默认为“检查”，对于 ninja 默认为“测试”。设置为空以禁用。

`test-commands`（字符串数组）

在测试期间运行的命令数组。

`modules`（对象或字符串数​​组）

一组对象，指定要在此模块之前构建的嵌套模块。数组中的字符串成员被解释为包含模块的单独 json 或 yaml 文件的名称。

## 五、源：modules-->sources

它们包含一个指向源的指针，该指针将在构建开始之前提取到源目录中。它们可以有多种类型，通过类型属性来区分。

此外，源列表可以包含一个纯字符串，该字符串被解释为此时读取和插入的单独 json 或 yaml 文件的名称。该文件可以包含单个源或一组源。

### 5.1 All sources

`only-arches`（字符串数组）

如果非空，则仅在列出的拱门上构建模块。

`skip-arches`（字符串数组）

不要在列出的任何拱门上进行构建。

`dest`（字符串）

将在其中提取该源的源目录内的目录。

### 5.2 Archive sources (tar, zip)

`type`

“archive”

`path`（字符串）

存档的路径

`url`（字符串）

将下载的远程存档的 URL。如果两者都指定了，这将覆盖路径。

`mirror-urls`（字符串数组）

主 url 失败时使用的备用 url 列表。

`git-init`（布尔值）

是否将存储库初始化为 git 存储库。

`archive-type`（字符串）

如果无法从路径猜测存档的类型。可能的值为“rpm”、“tar”、“tar-gzip”、“tar-compress”、“tar-bzip2”、“tar-lzip”、“tar-lzma”、“tar-lzop”、“tar- xz”、“tar-zst”、“zip”和“7z”。

`md5`（字符串）

文件的md5校验和，下载后验证

请注意，md5 不再被视为安全校验和，我们建议您至少使用 sha256。

`sha1`（字符串）

文件的sha1校验和，下载后验证

请注意，sha1 不再被视为安全校验和，我们建议您至少使用 sha256。

`sha256`（字符串）

文件的sha256校验和，下载后验证

`sha512`（字符串）

文件的sha512校验和，下载后验证

`strip-components`（整数）

提取期间要剥离的初始路径名组件的数量。默认为 1。

`dest-filename`（字符串）

下载文件的文件名，默认为 url 的基本名称。

### 5.3 Git sources

`type`

“git”

`path`（字符串）

git 存储库本地签出的路径。由于 git-clone 的工作原理，这比指定 file:///... 的 URL 快得多。

`url`（字符串）

git 存储库的 URL。如果两者都指定了，这将覆盖路径。通过 SSH 使用 git 时，正确的语法是 ssh://user@domain/path/to/repo.git。

`branch`（字符串）

要使用 git 存储库中的分支

`tag`（字符串）

要使用 git 存储库中的标签

`commit`（字符串）

从 git 存储库使用的提交。如果还指定了分支，则验证分支/标签是否位于此特定提交处。这是一种可读的方式来记录您正在使用特定标签，但要验证它不会更改。

`disable-fsckobjects`（布尔值）

不要使用transfer.fsckObjects=1来镜像git存储库。对于某些（损坏的）存储库可能需要这样做。

`disable-shallow-clone`（布尔值）

下载 git 存储库时不要通过进行浅克隆来进行优化。

`disable-submodules`（布尔值）

克隆存储库时不要签出 git 子模块。

### 5.4 Bzr sources

Bazaar（bzr）是另一个开源的 DVCS（Distributed Version Control System，即分布式版本控制系统），它试图给 SCM（Source Code Management，即源码管理） 的世界里带来一些新的东西。bzr是由ubuntu linux发行公司，由python编写。简单理解为和git功能类似。

`type`

“bzr”

`url`（字符串）

bzr 存储库的 URL

`revision`（字符串）

在分支中使用的特定修订版

### 5.5 Svn sources

`type`

“svn”

`url`（字符串）

svn 存储库的 URL，包括分支/标签部分

`revision`（字符串）

要使用的特定修订号

### 5.6 Directory sources

`type`

“dir”

`path`（字符串）

本地目录的路径，其内容将被复制到源目录中。请注意，目录源当前不支持缓存，因此每次都会重新构建它们。

`skip`（字符串数组）

目录中要忽略的源文件。

### 5.7 File sources

`type`

“file”

`path`（字符串）

将复制到源目录的本地文件的路径

`url`（字符串）

将下载并复制到源目录中的远程文件的 URL。如果两者都指定了，这将覆盖路径。

`mirror-urls`（字符串数组）

主 url 失败时使用的备用 url 列表。

`md5`（字符串）

文件的 md5 校验和，下载后进行验证。对于本地文件来说，这是可选的。

请注意，md5 不再被视为安全校验和，我们建议您至少使用 sha256。

`sha1`（字符串）

文件的 sha1 校验和，下载后验证。对于本地文件来说，这是可选的。

请注意，sha1 不再被视为安全校验和，我们建议您至少使用 sha256。

`sha256`（字符串）

文件的 sha256 校验和，下载后进行验证。对于本地文件来说，这是可选的。

`sha512`（字符串）

文件的 sha512 校验和，下载后进行验证。对于本地文件来说，这是可选的。

`dest-filename`（字符串）

在源目录中使用的文件名，默认为路径的基本名称。

### 5.8 Script sources

这是一种从内联命令集创建shell（/bin/sh）脚本的方法。

`type`

“script”

`commands`（字符串数组）

将放入 shellscript 文件中的 shell 命令数组

`dest-filename`（字符串）

在源目录中使用的文件名，默认为 autogen.sh。

### 5.9 Inline data sources

这是一种创建具有给定内容的文件的方法。

`type`

“inline”

`dest-filename`（字符串）

在源目录中使用的文件名。

`contents`（字符串）

将放入文件中的文本数据。

`base64`（布尔值）

内容是否采用 Base64 编码。


### 5.10 Shell sources

这是一种通过运行 shell 命令来创建/修改源的方法。

`type`

“shell”

`commands`（字符串数组）

将在源提取期间运行的一系列 shell 命令

### 5.11 Patch sources

`type`

“patch”

`path`（字符串）

将在源目录中应用的补丁文件的路径

`paths`（字符串数组）

将在源目录中应用的补丁文件的路径列表，按顺序排列

`strip-components`（整数）

patch 的 -p 参数的值默认为 1。

`use-git`（布尔值）

是否使用“git apply”而不是“patch”来应用补丁，当补丁文件包含二进制差异时需要。

`use-git-am`（布尔值）

是否使用“git am”而不是“patch”来应用补丁，当补丁文件包含二进制差异时需要。您不能与 一起使用它`use-git`。

`options`（字符串数组）

传递给 patch 命令的额外选项。

### 5.12 Extra data sources

`type`

“extra-data”

`filename`（字符串）

用于下载的额外数据的名称

`url`（字符串）

额外数据的 URL。

`sha256`（字符串）

额外数据的 sha256。

`size`（数字）

额外数据的大小（以字节为单位）。

`installed-size`（字符串）

这会增加应用程序的额外安装大小（可选）。

## 六、Shared Modules

共享模块（或共享模块）是一个包含各种清单的存储库，用于构建公共库。它打算用作git子模块。

常用命令：

```bash
$ git clone <repository> --recursive //递归的方式克隆整个项目 
$ git submodule add <repository> <path> //添加子模块 
$ git submodule init //初始化子模块 
$ git submodule update //更新子模块 
$ git submodule foreach git pull //拉取所有子模块
```

如果git clone 忘了带–recursive，或者是由于网络原因clone失败，可以输入：

```shell
$ git submodule update --init
```

相当于为所有的submodule进行git submodule init，然后执行git submodule update 。

举例：

要将其添加到存储库中，请运行以下命令：

```shell
$ git submodule add https://gitee.com/zhao_min_yong/shared-modules.git
```

.gitmodules内容参考如下：

然后，添加您想要的任何模块。在本例中，我们将使用gtk2：

```yaml
modules:  
- shared-modules/gtk2/gtk2.json
```

要更新子模块，请运行以下命令：

```shell
$ git submodule update --remote --merge
```

要删除子模块，请运行以下命令：

```shell
$ git submodule deinit -f -- shared-modules
$ rm -rf .git/modules/shared-modules
$ git rm -f shared-modules
$ rm -f .gitmodules
```