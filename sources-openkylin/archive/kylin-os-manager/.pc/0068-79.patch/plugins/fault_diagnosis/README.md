# 故障诊断

## 说明

* 检测和修复都是以单个诊断项为基础
* 错误码在程序内部使用 qint64 类型，在与 D-Bus 交互时使用 QString 类型

## 检测流程

1. 获取检测项列表

    ```
    目前采用二级列表，情况如下：
    - 分类1
      - 检测项1 // 1. 检测项的名称 2. 检测项的描述
      - 检测项2
    - 分类2
      - 检测项1
      - 检测项2
   
    DBus 接口：DiagnosticsList() -> (String list)
    返回值为 Json 格式：
    Category：分类
    Types：检测项数组，其中每一个对象代表一个检测项
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

2. 执行检测项，得到检测结果

    ```
    只提供一个检测接口：
    Check(int64 type) -> (String result)
    @type：为  DiagnosticsList 接口中返回的 Type 字段的值
    @result：返回值为 Json 格式：
    
    {
        "Overview": "检测了 10 项，7 项正确，3 项错误",
        "Details": [
            {
                "Name": "类型名",
                "ErrorCode": "123456",
                "Describe": "错误原因描述"
            },
            {
                "Name": "类型名",
                "ErrorCode": "9876",
                "Describe": "错误原因描述"
            }
        ]
    }
    ```

3. 根据检测结果从数据库中查询对应的解决方案

    ```
    Solution(int64 error_code) -> (solution result)
    type struct _solution {
        string type;           // 解决方案类型，从数据库中获取
        string solution;    // 解决方案，从数据库中获取
        string crc;         // 校验字符串，从数据库中获取
    } solution;
    ```
   
4. 执行解决方案，得到执行结果

    ```
    Repair(string repair_items) -> (bool ok)
    @repair_items: 需要修复的错误项，为 json 数组格式的字符串，每个元素代表一个修复项
    [
        {
            "ErrorCode": "0x123456",
            "SolutionType": "0x987",
            "Solution": "xxx",
            "Crc": "0x111"
        },
        {
            "ErrorCode": "0x3241",
            "SolutionType": "0x326",
            "Solution": "xxx",
            "Crc": "0x222"
        }
    ]
   
    @ok：是否执行成功
    ```
   
5. 单项检测修复进度获取信号 progress 

    ```
    progress(int64 type, int progress)
    @type: DiagnosticsList 函数返回值中的 type 值
    @progress: 进度值 0 - 100
    ```
   
6. 修复时，每修复完一项发送信号

    ```
    Fixed(int64 errorcode, bool ok, string reason)
    @errorcode: 本次修复完成的错误码
    @ok: 是否修复成功
    @reason: 错误原因，如果成功则为空
    ```
