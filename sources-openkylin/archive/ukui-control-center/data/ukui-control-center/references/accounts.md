# 账户信息设置

打开页面：`ukui-control-center -m userinfo`

## 添加用户

账户类型：普通用户 `0`，管理员 `1`。

```bash
gdbus call --system --dest org.ukui.groupmanager --object-path /org/ukui/groupmanager \
  --method org.ukui.groupmanager.interface.createUser \
  用户名 用户昵称 账户类型 "/usr/share/ukui/faces/01-default-commercial.png" 密码

# 示例
gdbus call --system --dest org.ukui.groupmanager --object-path /org/ukui/groupmanager \
  --method org.ukui.groupmanager.interface.createUser \
  test test 0 "/usr/share/ukui/faces/01-default-commercial.png" default123
```

## 删除用户

流程：未提供用户名则询问 → `whoami` 与 `cat /etc/group | grep sudo` 判断是否当前用户或最后一个管理员 → 是则拒绝 → 否则询问是否删除用户数据 → 再删除。

```bash
whoami
cat /etc/group | grep sudo
id 用户名

# 删除（true=删除用户数据，false=保留）
gdbus call --system --dest org.freedesktop.Accounts --object-path /org/freedesktop/Accounts \
  --method org.freedesktop.Accounts.DeleteUser 用户id true

gdbus call --system --dest org.freedesktop.Accounts --object-path /org/freedesktop/Accounts \
  --method org.freedesktop.Accounts.DeleteUser 用户id false
```

## 修改昵称

```bash
id 用户名
gdbus call --system --dest org.freedesktop.Accounts \
  --object-path /org/freedesktop/Accounts/User用户id \
  --method org.freedesktop.Accounts.User.SetRealName 昵称
```

## 更改密码

```bash
passwd 用户名
```

## 修改头像

要求：图片 < 1M，格式 jpg/jpeg/png/svg/gif。

```bash
id 用户名
# 若路径不在 /var/lib/AccountsService/icons/ 或 /usr/share/ukui/faces/
# 则 reFilename = 原名-用户名-yyyyMMddhhmmss.扩展名（不改原文件）

gdbus call --system --dest org.freedesktop.Accounts \
  --object-path /org/freedesktop/Accounts/User用户id \
  --method org.freedesktop.Accounts.User.SetIconFileRename filename reFilename
```

## 免密登录

```bash
gdbus call --system --dest org.ukui.groupmanager --object-path /org/ukui/groupmanager \
  --method org.ukui.groupmanager.interface.setNoPwdLoginStatus true 用户名
gdbus call --system --dest org.ukui.groupmanager --object-path /org/ukui/groupmanager \
  --method org.ukui.groupmanager.interface.setNoPwdLoginStatus false 用户名
```

## 自动登录

```bash
id 用户名
gdbus call --system --dest org.freedesktop.Accounts \
  --object-path /org/freedesktop/Accounts/User用户id \
  --method org.freedesktop.Accounts.User.SetAutomaticLogin true
gdbus call --system --dest org.freedesktop.Accounts \
  --object-path /org/freedesktop/Accounts/User用户id \
  --method org.freedesktop.Accounts.User.SetAutomaticLogin false
```

## 添加用户组

```bash
gdbus call --system --dest org.ukui.groupmanager --object-path /org/ukui/groupmanager \
  --method org.ukui.groupmanager.interface.add 组名 组ID

# 示例
gdbus call --system --dest org.ukui.groupmanager --object-path /org/ukui/groupmanager \
  --method org.ukui.groupmanager.interface.add testgroup 2222
```
