# 系统启动 / Grub 验证

打开页面：`ukui-control-center -m boot`

接口：system DBus `com.control.center.interface`，对象路径 `/`，服务 `com.control.center.qt.systemdbus`。开启/关闭需管理员认证。

## 开启 / 关闭 Grub 验证

流程：开启且未提供密码则询问 → 调用 DBus → 返回结果。

```bash
# 开启（需设置 Grub 编辑密码）
gdbus call --system --dest com.control.center.qt.systemdbus --object-path / \
  --method com.control.center.interface.setGrupPasswd "root" "grub123" "$(echo $LANG)" true

# 关闭（可传空密码）
gdbus call --system --dest com.control.center.qt.systemdbus --object-path / \
  --method com.control.center.interface.setGrupPasswd "root" "" "$(echo $LANG)" false

# 状态：true 已开启，false 已关闭
gdbus call --system --dest com.control.center.qt.systemdbus --object-path / \
  --method com.control.center.interface.getGrupPasswdStatus
```

注意：会更新 grub 配置，可能耗时较长。
