# 故障诊断

## 说明

* 检测和修复都是以单个诊断项为基础
* 错误码在程序内部使用 qint64 类型，在与 D-Bus 交互时使用 QString 类型

## 接口说明

```
Bus Name:    com.kylin.kydoctor
Object Path: /com/kylin/kydoctor
interface:   com.kylin.kydoctor
```

1. 获取所有检测项

    ```
    目前采用二级列表，情况如下：
    - 分类1
      - 检测项1    // 1. 检测项的名称 2. 检测项的描述
      - 检测项2
    - 分类2
      - 检测项1
      - 检测项2
   
    GetCheckList() -> String
    & Json 格式的数据，具体格式如下：
      Category：分类
      Types：该分类下所有的检测项，其中每一个对象代表一个检测项
          Type：检测项的标识，用于唯一标识一个检测项
          Name：检测项在界面上显示的名字
          Desc：检测项在界面上显示的描述
    [
        {
            "Category": "Security",
            "Types": [
                {
                    "Type": "0x1234",
                    "Name": "cve 检测",
                    "Describe": "检测 cve 漏洞"
                },
                {
                    "Type": "0x9876",
                    "Name": "kve 检测",
                    "Describe": "检测 kve 漏洞"
                }
            ]
        },
        {
            "Category": "Hardware",
            "Types": [
                {
                    "Type": "0x1111",
                    "Name": "硬盘检测",
                    "Describe": "检测硬盘是否正常"
                },
                {
                    "Type": "0x2222",
                    "Name": "蓝牙检测",
                    "Describe": "检测蓝牙是否正常"
                }
            ]
        }
    ]
    ```
   
2. 获取外部错误对应的检测项

    ```
    GetCheckListNonUniversal(int64 external_error_code) -> String
    @external_error_code 外部应用传递给麒麟管家的错误码，
                         比如软件商店出错，调用麒麟管家修复时，传递给麒麟管家的错误码
    & 与 GetCheckList 返回值格式一致
   ```

3. 检测检测项，得到检测结果

    ```
    CheckItem(int64 type) -> String
    # 该接口为异步接口，通过绑定 CheckItem 和 CheckItemOver 信号获取结果
    @type 检测项标识，GetCheckList 接口中返回的 Type 字段的值
    & 异步接口，该返回值没有意义
    ```
    ```
    信号：
    1. CheckItem(String result)
       # 每检测完一项，发送一次该信号
       @result 检测项的检测结果，为 Json 格式的数据，格式如下：
       {
           "Overview": "检测了 10 项，7 项正确，3 项错误",
           "Details": [
               {
                   "Name": "错误名",
                   "ErrorCode": "123456",
                   "Describe": "错误原因描述"
               },
               {
                   "Name": "错误名",
                   "ErrorCode": "9876",
                   "Describe": "错误原因描述"
               }
           ]
       }

    2. CheckItemOver(bool ok)
       # 当所有检测完毕时发送该信号
       @ok true 检测结束并成功，false 检测结束但失败
    ```
   
4. 检测非通用检测项，得到检测结果(软件商店，系统升级)

    ```
    CheckItemNonUniversal(int64 type, String data) -> String
    # 该接口为异步接口，通过绑定 CheckItemNonU 和 CheckItemNonUOver 信号来获取结果
    @type 检测项标识，GetCheckList 接口中返回的 Type 字段的值
    @data 携带的数据
    & 异步接口，该返回值没有意义(为处理dbus的25秒超时，默认收到信息后立刻回传)
    ```
    ```
    信号：
    1. CheckItemNonU(String result)
       @result 检测结果，与 CheckItem 信号中参数的格式一致
    2. CheckItemNonUOver(bool ok)
       @ok true 检测结束并成功，false 检测结束但失败
    ```

5. 修复错误，得到修复结果

    ```
    RepairItem(String repair_items) -> bool
    # 以检测项为基准，修复该检测项下所有的错误
    # 异步接口，通过绑定 SigFixed 和 RepairOver 信号获取结果
    @repair_items 需要修复的错误，为 Json 格式，每个元素代表一个修复项
    [
        {
            "ErrorCode": "0x11111"
        },
        {
            "ErrorCode": "0x22222"
        }
    ]
    & 异步接口，返回值没有意义
    ```
    ```
    信号：
    1. SigFixed(int64 error_code, bool ok, String reason, int64 type, String context)
       # 每修复完一个错误发送一次该信号
       @error_code 修复的错误码
       @ok 是否修复成功，true 成功，false 失败
       @reason 失败原因，成功时为空
       @type 失败时需要手动修复的类型
       @context 手动修复的方案内容
   
    2. RepairOver(bool ok)
       # 所有错误修复完成
       @ok true 修复结束并成功，false 修复结束但失败
   
    3. SignalProgress(int64 type, int32 progress)
       # 修复进度
       @type 检测项标记，GetCheckList 返回值中 type 的值
       @progress 进度值 0 - 100
    ```
   
## 实现逻辑

### 修复外部应用错误

1. 外部应用通过命令行调起麒麟管家，传递错误码和所需的参数
2. 麒麟管家根据传递进来的错误码获取对应的检测项信息，构建界面
3. 自动开始检测
4. 如果可以修复，则用户可以点击一键修复进行修复，如果不可以修复可以显示手动修复或其他展示形式
5. 如果修复成功，则显示修复成功，如果修复失败，则可以显示手动修复或其他展示形式

命令调用形式如下：

```
kylin-os-manager --repair --errorCode xxx --errorData xxx
```
