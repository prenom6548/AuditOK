# openKylin 社区仓库与权限申请指南

本文档介绍如何通过 `openkylin/community` 仓库下各 SIG 的 `sig.yaml` 文件来申请新的仓库或申请仓库权限。

---

## 一、前置条件

### 1.1 签署 CLA

在参与社区贡献前，必须先签署 openKylin 贡献者许可协议（CLA）：

- **签署地址**: https://cla.openkylin.top/cla
- 个人开发者签署个人 CLA
- 企业需要签署组织 CLA，企业开发者签署员工 CLA

### 1.2 了解 SIG 组

openKylin 社区通过 SIG（Special Interest Group，兴趣小组）来组织各项社区开发事务。每个 SIG 组管理特定领域的软件包仓库。

**查看现有 SIG 组**: https://gitee.com/openkylin/community/tree/master/sig

常见 SIG 组包括：
- `packaging` - 打包组
- `kernel` - 内核组
- `framework` - 框架组
- `qa` - 质量保障组
- `compatibility` - 兼容性组
- `ukui` - UKUI 桌面环境组

---

## 二、openkylin/community 仓库结构

```
openkylin/community/
├── sig/
│   ├── packaging/
│   │   ├── README.md      # SIG 组章程
│   │   └── sig.yaml       # SIG 组配置文件
│   ├── kernel/
│   │   ├── README.md
│   │   └── sig.yaml
│   ├── ukui/
│   │   ├── README.md
│   │   └── sig.yaml
│   └── ...
└── README.md
```

每个 SIG 目录下包含：
- `README.md` - SIG 组章程，介绍组的目标、沟通方式等
- `sig.yaml` - SIG 组配置文件，包含负责人、成员、维护的包列表

---

## 三、sig.yaml 文件格式

### 3.1 整体结构

`sig.yaml` 文件采用 YAML 格式，包含以下基本元素：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | 字符串 | SIG组名称 |
| description | 字符串 | SIG组描述 |
| owner | 列表 | SIG组负责人 |
| maintainers | 列表 | SIG组成员 |
| packages | 列表 | SIG组维护包列表 |

### 3.2 maintainers 列表格式

maintainers 列表中每一条记录代表一位 maintainer 的个人信息：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| name | 字符串 | 成员姓名（**必填**，对应 gitee_id） |
| displayname | 字符串 | 成员姓名或昵称（选填） |
| openkylinid | 字符串 | 成员的 openkylinid 用户名（选填） |
| email | 字符串 | 联系邮箱（选填） |

> **重要**：`name` 字段对应的是 **gitee_id**（Gitee 用户名），请正确填写！

**查看 gitee_id 方式**：登录 Gitee 后，进入个人主页，URL 中的用户名即为 gitee_id。

### 3.3 packages 列表格式

packages 列表按照所属权限不同划分为**公共包**和**单独包**。

#### 公共包

公共包为 packages 列表下的每一条直接记录：

```yaml
packages:
- ukui-session-manager
- kylin-miraclecast
```

公共包由 SIG 的所有 maintainer 共同维护。

#### 单独包（权限分级管理）

当 SIG 管理的包仓库较多时，可以通过单独包将仓库权限分级管理。单独包通过 packages 的二级列表配置：

| 字段 | 类型 | 说明 |
| --- | --- | --- |
| package | 列表 | 单独包列表 |
| maintainer | 列表 | 维护单独包的成员 |

```yaml
packages:
- ukui-session-manager      # 公共包
- kylin-miraclecast         # 公共包
- package:
  - peony                   # 单独包
  - ukui-default-settings   # 单独包
  maintainer: 
  - name: larue             # 负责维护单独包的成员（package maintainer）
```

### 3.4 权限说明

| 角色 | 权限范围 |
| --- | --- |
| SIG Maintainer | 对公共包仓库和单独包仓库都有管理员权限 |
| Package Maintainer (PM) | 只对所维护的包仓库有管理员权限 |

### 3.5 完整样例

```yaml
name: UKUI
description: UKUI team
owner:
- wx-kylin
maintainers:
- name: handsome_feng
- name: zhaikangning
- name: MouseZhang
packages:
- ukui-session-manager
- kylin-miraclecast
- package:
  - peony
  - ukui-default-settings
  maintainer: 
  - name: larue
- package:
  - ukui-menu
  maintainer:
  - name: lixueman0110
```

---

## 四、申请新仓库

### 4.1 操作步骤

#### 步骤 1: Fork community 仓库

1. 访问 https://gitee.com/openkylin/community
2. 点击右上角「Fork」按钮
3. 选择个人目标空间，完成 Fork

#### 步骤 2: 克隆到本地

```bash
# 克隆个人 fork 的仓库
git clone git@gitee.com:<username>/community.git

# 进入目录
cd community
```

#### 步骤 3: 确定所属 SIG 组

根据软件包的性质，确定应该归属于哪个 SIG 组。例如：
- 内核相关软件包 → `kernel` SIG
- 基础打包软件包 → `packaging` SIG
- IDE 开发工具 → `framework` SIG
- UKUI 桌面组件 → `ukui` SIG

#### 步骤 4: 修改 sig.yaml 文件

编辑对应 SIG 的 `sig.yaml` 文件，在 `packages` 列表中添加新的软件包名称：

```yaml
name: UKUI
description: UKUI team
owner:
- wx-kylin
maintainers:
- name: handsome_feng
packages:
- ukui-session-manager
- kylin-miraclecast
- new-package-name    # 新增的软件包
```

> **注意**: 软件包名称使用源码包名称（source package name），不需要 `openkylin/` 前缀

#### 步骤 5: 提交修改并推送

```bash
# 添加修改
git add sig/ukui/sig.yaml

# 提交修改
git commit -m "新增 xxx 软件包仓库"

# 推送到个人仓库
git push origin master
```

#### 步骤 6: 创建 Pull Request

1. 访问你 fork 的仓库页面
2. 点击「+ Pull Request」
3. 填写 PR 标题和描述：
   - 标题格式：`新增 xxx 软件包仓库`
   - 描述内容：说明软件包用途、来源等
4. 提交 PR

#### 步骤 7: 等待审核

- **审核流程**: sig.yaml 的更新由 SIG 主负责人提交 PR，经过技术委员会审核后合入
- 审核通过后，CI 平台会自动创建对应的 Gitee 仓库
- 仓库地址格式：`https://gitee.com/openkylin/<package-name>`

### 4.2 注意事项

1. **仓库名称规范**：
   - 只能包含小写字母、数字、中划线
   - 不能以数字开头
   - 需与软件包源码名称一致

2. **选择正确的 SIG 组**：
   - 仓库归属的 SIG 组决定了后续谁有权管理该仓库
   - 如不确定，可以先在社区论坛或邮件列表咨询

3. **PR 描述要求**：
   - 说明软件包的用途
   - 提供上游项目地址
   - 说明为什么需要引入该软件包

---

## 五、申请仓库权限

### 5.1 权限类型

| 权限级别 | 说明 |
|----------|------|
| SIG Maintainer | 对 SIG 下所有包仓库有管理员权限 |
| Package Maintainer (PM) | 只对指定的包仓库有管理员权限 |
| Contributor | 可提交 Issue 和 PR |

### 5.2 申请成为 SIG Maintainer

#### 方式一：联系现有 Maintainer

1. 查看对应 SIG 目录下的 `sig.yaml` 文件，找到现有 maintainer 列表
2. 通过 Gitee 私信、邮件或 Issue 联系 maintainer 或 owner
3. 说明你的贡献经历和申请理由

#### 方式二：参加 SIG 例会

1. 关注 openKylin 社区日程（小程序或官网）
2. 参加目标 SIG 的例会
3. 在例会上提出加入申请
4. 获得支持后，由 SIG 主负责人提交 PR 更新 sig.yaml

### 5.3 申请成为 Package Maintainer

如果只需要某个特定仓库的权限，可以申请成为 Package Maintainer：

1. **联系 SIG Maintainer**：通过 Gitee 私信或邮件联系该仓库所属 SIG 的 maintainer
2. **参与贡献**：先通过 PR 贡献代码，证明能力后更容易获得权限
3. **更新 sig.yaml**：由 SIG 主负责人在 sig.yaml 中将该包设为单独包，并添加你的 gitee_id 为 package maintainer

配置示例：

```yaml
packages:
- existing-public-package
- package:
  - target-package    # 你要维护的包
  maintainer:
  - name: your-gitee-id
```

### 5.4 信息更新流程

**原则上 sig.yaml 的更新由 SIG 主负责人来提交 PR，经过技术委员会审核后合入。**

1. 联系 SIG owner 或 maintainer
2. 说明申请理由和贡献经历
3. 由 SIG 主负责人提交 PR 更新 sig.yaml
4. 技术委员会审核通过后生效

---

## 六、常见问题

### Q1: PR 提交后多久能创建仓库？

通常 PR 审核通过后，CI 平台会在几分钟内自动创建仓库。如长时间未创建，可以在 PR 中留言询问。

### Q2: 如何查看某个仓库属于哪个 SIG？

查看 `openkylin/community` 仓库下各 SIG 目录的 `sig.yaml` 文件，在 packages 列表中搜索软件包名称即可确定归属。或者可以查看 https://gitee.com/kiber/community/blob/master/sig/sig.yaml 这个总的sig.yaml文件（注意：请不要在这个总的sig.yaml上修改，而是查到是哪个SIG组后，去SIG组对应的目录里修改其自己的sig.yaml文件）

### Q3: 可以同时向多个 SIG 申请吗？

可以，但每个软件包仓库只能归属于一个 SIG。如果软件包涉及多个领域，需要与相关 SIG 协商确定归属。

### Q4: 申请被拒绝怎么办？

- 查看 Maintainer 给出的拒绝原因
- 根据反馈修改后重新提交
- 如有疑问，可以通过邮件列表或例会沟通

### Q5: 如何创建新的 SIG 组？

如果现有 SIG 都不适合你的项目，可以申请新建 SIG 组：

1. 在 `openkylin/community` 仓库提交 Issue
2. 说明新 SIG 的名称、目标、初始成员
3. 等待技术委员会审批
4. 审批通过后创建 SIG 目录、README.md 和 sig.yaml 文件

### Q6: gitee_id 填写错误会怎样？

`name` 字段必须填写正确的 gitee_id（Gitee 用户名），否则权限无法正确分配。填写前请确认你的 Gitee 用户名。

---

## 七、相关链接

- **community 仓库**: https://gitee.com/openkylin/community
- **SIG README 规范**: https://gitee.com/openkylin/community/blob/master/sig/README.md
- **CLA 签署**: https://cla.openkylin.top/cla
- **社区论坛**: https://forum.openkylin.top/
- **文档中心**: https://docs.openkylin.top/
- **SIG 申请页面**: https://www.openkylin.top/sig/application-cn.html
- **源码自主选型构建流程**: https://docs.openkylin.top/zh/04_社区贡献/开发指南/openKylin源码自主选型构建流程

---

## 八、流程图

```
┌─────────────────────────────────────────────────────────────┐
│                    申请新仓库流程                            │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  1. 签署 CLA                                                 │
│       ↓                                                     │
│  2. Fork community 仓库                                      │
│       ↓                                                     │
│  3. 克隆到本地                                               │
│       ↓                                                     │
│  4. 修改 sig.yaml（在 packages 列表添加仓库名）              │
│       ↓                                                     │
│  5. 提交 PR                                                  │
│       ↓                                                     │
│  6. 技术委员会审核                                           │
│       ↓                                                     │
│  7. CI 自动创建仓库                                          │
│                                                             │
└─────────────────────────────────────────────────────────────┘

┌─────────────────────────────────────────────────────────────┐
│                  申请权限流程                                │
├─────────────────────────────────────────────────────────────┤
│                                                             │
│  方式一：联系 SIG Maintainer                                 │
│       ↓                                                     │
│  SIG 主负责人提交 PR 更新 sig.yaml                          │
│       ↓                                                     │
│  技术委员会审核                                              │
│       ↓                                                     │
│  权限生效                                                    │
│                                                             │
│  方式二：参加 SIG 例会申请                                   │
│       ↓                                                     │
│  获得支持后，由 SIG 主负责人更新 sig.yaml                    │
│       ↓                                                     │
│  技术委员会审核                                              │
│       ↓                                                     │
│  权限生效                                                    │
│                                                             │
└─────────────────────────────────────────────────────────────┘
```

---

*文档更新时间：2026年4月19日*
*参考来源：https://gitee.com/openkylin/community/blob/master/sig/README.md*
