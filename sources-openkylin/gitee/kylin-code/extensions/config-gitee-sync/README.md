# 配置扩展同步 config-gitee-sync

一个配置扩展同步的gitee版.

1、多设备间同步配置和扩展（由于gitee gist改为了只能私有，所以该功能无法使用了）

2、方便团队统一开发环境。将Gist设置为`public`即可，具体参考[这里](#快速开始)

## 功能

在`本地和云端之间同步您的所有 IDE 配置`，包括：

#### 1. 上传 IDE 配置:

  * 包括您的`用户设置`，`快捷键`，`扩展`，`语言设置`以及所有`代码片段（Snippets）`；
  * 因为 `Mac` 和`非 Mac` 设备的配置通常会有一些差异，所以`快捷键`将会按照操作系统的不同分别上传；
  * 自动帮您创建新的 `Gist` 来保存 IDE 配置，例如当您第一次使用这个扩展上传配置时；
  * 为了加快同步速度，整个同步过程都是`增量`的；
  * 您可以`排除某些 IDE 配置项和扩展`，以防它们被上传，具体请参考[这里](#ide-配置项)。

#### 2. 下载 IDE 配置:
  * 请注意，下载配置时会**覆盖**您的本地配置（以云端为准，精确同步）；
  * 自动帮您`安装`、`升级`和`删除`扩展；
  * 您可以从一个`公开的 Gist` 中下载配置。团队共享配置和扩展，只要一个公开的 `Gist ID` 就行了，具体请参考[这里](#快速开始)；
  * 您可以`排除某些 IDE 配置项和扩展`，以防它们被下载，具体请参考[这里](#ide-配置项)。

## 命令

在 `IDE 的命令面板`中，您可以输入 `upload`、`download`（或者 `gitee`）来快速搜索和执行 `config-gitee-sync` 的所有命令。

1. ***`config-gitee-sync: Upload Settings`***

    > 上传配置到 Gitee Gist。

1. ***`config-gitee-sync: Download Settings`***

    > 从 Gitee Gist 下载配置。

1. ***`config-gitee-sync: Open Syncing Settings`***

    > 设置 `Gitee Personal Access Token`、`Gist ID`。

## 快捷键

**默认情况下所有的快捷键都是关闭的**，但是您可以很轻松的在 `IDE 键盘快捷方式`中指定您想要的快捷键。截图是`syncing`的，搜索`config-gitee-sync`：

  ![keyboard shortcuts](https://gitee.com/openkylin/kylin-code/raw/master/extensions/config-gitee-sync/docs/png/Keyboard-Shortcuts.png)

## IDE 配置项

您可以在 `IDE 用户设置`中找到以下 `config-gitee-sync` 配置项。

1. ***`giteeSync.excludedExtensions`***

    通过这个配置项，您可以`排除特定的 IDE 扩展`，以防它们被同步。那些不在此列表中的 IDE 扩展将不受影响，依然会正常同步。

    > 配置规则可以参考 [Glob Patterns](https://github.com/isaacs/minimatch)。

    举个栗子：

    ```json
    "giteeSync.excludedExtensions" : [
        "somepublisher.*",
        "kylinideteam.config-gitee-sync"
    ]
    ```

    要注意的是，这里被排除的`扩展名称`其实是`扩展的 ID`（可在 IDE 的`扩展`页中找到），如下图所示：

    ![exclude extensions](https://gitee.com/openkylin/kylin-code/raw/master/extensions/config-gitee-sync/docs/png/Exclude-Extensions.png)

    这样一来 `nonoroazoro.syncing`（也就是本扩展）以及所有属于 `somepublisher` 这个作者的扩展就不会再被同步啦。

1. ***`giteeSync.excludedSettings`***

    通过这个配置项，您可以`排除特定的 IDE 配置项`，以防它们被同步。那些不在此列表中的 IDE 配置项将不受影响，依然会正常同步。

    > 配置规则可以参考 [Glob Patterns](https://github.com/isaacs/minimatch)。

    举个栗子：

    ```json
    "giteeSync.excludedSettings" : [
        "editor.*",
        "workbench.colorTheme"
    ]
    ```

    这样一来您的 IDE 主题（`workbench.colorTheme`）以及所有与编辑器（`editor`）相关的配置项就不会再被同步啦。

1. ***`giteeSync.extensions.autoUpdate`***

    通过这个配置项，可以让 `config-gitee-sync` 在同步时自动升级您的 IDE 扩展。

    这个功能是`默认开启`的，当然您也可以在 `IDE 用户设置` 中关掉它。

1. ***`giteeSync.pokaYokeThreshold`***

    通过配置这个阈值，每次同步配置时，`config-gitee-sync` 都会检查您的本地和云端配置间的差异量。如果超出这个阈值，就会显示一个确认对话框，以防错误的覆盖您的配置。

    这个配置项的默认值是 `10`。您可以将其设置为 `0` 来关闭这个功能（不再检查和显示确认对话框）。

    举个栗子：

    ```json
    "giteeSync.pokaYokeThreshold" : 10
    ```

1. ***`giteeSync.separateKeybindings`***

    通过这个配置项，可以决定是否让 `config-gitee-sync` 按照设备`操作系统`的不同来分开同步您的`快捷键`配置。

    鉴于 `IDE` 从 `1.27` 版本开始提供了 [Platform Specific Keybindings](https://code.visualstudio.com/updates/v1_27#_platform-specific-keybindings) 功能，您现在可以关闭这个功能。注意：在关闭该功能之前，请务必确保您已经手动合并了现有的`快捷键`配置。

    这个功能是`默认开启`的。您可以在 `IDE 用户设置` 中关掉它。

## 快速开始

#### 1. 创建你自己的`Gitee Personal Access Token`

[码云](https://gitee.com) > 设置 > 私人令牌 > +生成新令牌

默认会有全部权限，这里只勾选gists即可，user_info权限是必选。

> 请务必备份你的令牌

#### 2. 同步你的IDE配置
在第一次使用时，会向您询问一些必要的信息并保存下来以供后续使用，主要就是前面申请的令牌 `Access Token` 。

**上传配置**
   * 在IDE的命令面板中输入 `upload`
      ![upload settings](https://gitee.com/openkylin/kylin-code/raw/master/extensions/config-gitee-sync/docs/png/Upload-Settings.png)
   * 填写申请的令牌（Access Token）
   * 在下拉矿中选择或者手动输入一个`Gist ID`
      > 留空会自动创建一个新的Gist。
   * 完成
   * 上传完成后，Gitee的代码片段中可以找到对应Gist。可以设置为`public`公开给团队。

**下载配置**
   * 在IDE的命令面板中输入 `download`
      ![download settings](https://gitee.com/openkylin/kylin-code/raw/master/extensions/config-gitee-sync/docs/png/Download-Settings.png)
   * 填写你的令牌（Access Token）
      > 留空，即可在下一步填写公开的Gist
   * 在下拉框中选择或者手动输入一个 `Gist ID`（也可以输入一个`公开的 Gist ID`）。
   * 完成


