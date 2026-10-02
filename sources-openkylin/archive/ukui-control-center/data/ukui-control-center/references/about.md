# 关于 / 诊断数据

打开页面：`ukui-control-center -m about`

接口：**system** DBus（不是 session）  
`dest=com.kylin.daq`，路径 `/com/kylin/daq`，接口 `com.kylin.daq.interface`。  
控制面板「关于」页即通过该 system 接口调用 `SetUploadState` / `GetUploadState`。

禁止：
- `gdbus call --session …`（会报 ServiceUnknown：session 上没有 com.kylin.daq）
- 方法写成 `com.kylin.daq.SetUpLoadState`（大小写/接口名都错；正确为 `com.kylin.daq.interface.SetUploadState`）

## 发送可选诊断数据

```bash
# 开启
gdbus call --system --dest com.kylin.daq --object-path /com/kylin/daq \
  --method com.kylin.daq.interface.SetUploadState 1

# 关闭
gdbus call --system --dest com.kylin.daq --object-path /com/kylin/daq \
  --method com.kylin.daq.interface.SetUploadState 0

# 查询：0 已关闭，1 已开启
gdbus call --system --dest com.kylin.daq --object-path /com/kylin/daq \
  --method com.kylin.daq.interface.GetUploadState
```

若正确命令仍报 `dbus limit control, operation not permitted`：属系统对命令行调用的限制，控制面板进程可调用故 UI 能开关；此时应告知用户在「关于」页手动切换，或说明 shell 侧受策略限制，不要改成 --session。
