# 麒麟管家

## 参数说明

### 管家主体参数说明

| 参数 | 说明 |
| --- |-----------------------------------------------------------------------------------|
| --repair | 调用麒麟管家故障诊断功能来修复错误 |
| --errorCode | 错误码，需配合 --repair 参数使用 |
| --errorData | 错误数据，需配合 --repair 参数使用 |
| --module | 模块名，需配合 --repair 参数使用，***值列表：Upgrade、Software*** |
| --diagnosis | 调用麒麟管家故障诊断功能来检测指定问题 |
| --mode | 诊断的类型，需配合 --diagnosis 参数使用，***值列表：Network、PerformanceAndComponents、Application*** |
| --jumpTab | 跳转到指定的 Tab 页，***值列表：ServiceSupport、DriverManager*** |
| --pluginMessage | 传递给插件的信息，需配合 --jumpTab 参数使用，***值列表：ProblemFeedback*** |

### 全局反馈小窗参数说明

| 参数 | 说明 |
| --- |-----------------------------------------------------------------------------------|
| -mode | 反馈模式 1:用户主动触发 2:被动触发 3:无界面提交（结果不提示） |
| -exe | 触发反馈的二进制名称 |
| -path | 附件所在目录的绝对路径（非必填） |
| -restart |  重启命令（非必填，如此参数为空则不触发重启），一般为进程名，少数需要带参，为避免格式错乱作为参数传递时进行toHex处理 |
| -crashStackPath | 存储崩溃堆栈特征值的文件路径 |
| 命令示例 | /usr/lib/kylin-os-manager/bin/service-support-mini -mode=2 -exe=kylin-printer -path=/tmp/asfve -restart=2f7573722f62696e2f6b796c696e2d7072696e746572 -crashStackPath=/xxx/xx |

## 百宝箱插件配置文件格式

将文件存放在下述目录下：
    - /usr/share/kylin-os-manager/plugins/tool-box/plugins/
    - 磐石系统路径：/opt/system/resource/kylin/kylin-os-manager/plugins/tool-box/plugins/

```
文件名规则：00_0000_xxxxxx.conf
前 6 位为数字，用于指定在百宝箱中显示的顺序，后边的为正常的文件名

前两位数字代表分类
    00：特色工具
    01：运维调试工具
    02：故障修复工具

中间四位数字是在指定分类下的显示顺序

目前已使用的数字组合：
    00_0000_kylin-boot-guide.conf    启动助手
```

```
[ToolBox Entry]
Name=xxx          // 必填
Name[zh_CN]=xxx
Name[zh_HK]=xxx
Name[bo_CN]=xxx
Name[mn]=xxx
Comment=xxx       // 必填
Comment[zh_CN]=xxx
Comment[zh_HK]=xxx
Comment[bo_CN]=xxx
Comment[mn]=xxx
Icon=xxx          // 必填
Exec=xxx          // 必填
Categories=xxx    // 值列表：Feature(特色工具)、Debug(运维调试工具)、Troubleshooting(故障修复类工具)
Depends=xxx       // 依赖包，用分号隔开，例：A;B;C
UserManual=xxx    // 用户手册名
```

## 开发规范

### 安装目录

#### 主框架

麒麟管家主框架数据文件存放在 /usr/share/kylin-os-manager/ 目录下

麒麟管家用户级的数据文件存放在 ~/.kylin-os-manager/ 目录下

#### 插件

插件在 /usr/share/kylin-os-manager/plugins/ 目录下单独创建一个自己的目录，用于存放自身的数据文件

插件在 ~/.kylin-os-manager/plugins/ 目录下单独创建一个自己的目录，用于存放自身用户级的数据文件

#### 守护进程

守护进程在 /usr/share/kylin-os-manager/daemons/ 目录下单独创建一个自己的目录，用于存放自身的数据文件

守护进程在 ~/.kylin-os-manager/daemons/ 目录下单独创建一个自己的目录，用于存放自身用户级的数据文件

#### 百宝箱

百宝箱中的应用在 /usr/share/kylin-os-manager/treasure-chest/ 目录下单独创建一个自己的目录，用于存放自身的数据文件

百宝箱中的应用在 ~/.kylin-os-manager/treasure-chest/ 目录下单独创建一个自己的目录，用于存放自身用户级的数据文件

### 其他

为尽可能兼容各个版本的麒麟系统，功能方面要尽量减少对外部的依赖
