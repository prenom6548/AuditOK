预制知识库工具 使用说明

为了保证数据的安全性
本工具所有接口都是操作拷贝出来的数据库文件

步骤1:首先停止系统自启动的向量数据库进程
systemctl --user stop kylin-ai-vector-engine.service

步骤2:为避免数据污染，每次更新预制知识库的时候，都需要拷贝一份干净的db文件
从/usr/share/kylin-ai-vector-engine/default.db 拷贝出一份
此时需要手动启动一个kylin-ai-vector-engine进程
命令如下：
/usr/bin/kylin-ai-vector-engine /path/default.db localhost:19530 INFO

注：
①：/path/default.db是你拷贝出来的向量数据库路径
②：路径不能有中文

步骤3:工具创建的默认向量数据库表结构如下：如果需要修改表结构，需要修改create和add接口内部与向量数据库相关的代码
CollectionSchema collection_schema(name);
collection_schema.AddField(FieldSchema(DATABASE_FILE_PATH, DataType::VARCHAR, "file path").WithMaxLength(100));
collection_schema.AddField(
    FieldSchema(DATABASE_DOCUMENT, DataType::VARCHAR, "document contents").WithMaxLength(1024));
collection_schema.AddField(FieldSchema(DATABASE_ID, DataType::VARCHAR, "ids", true, false).WithMaxLength(40));
collection_schema.AddField(
    FieldSchema(DATABASE_VECTOR, DataType::FLOAT_VECTOR, "vectors").WithDimension(DIMENSION));

IndexDesc index_desc(DATABASE_VECTOR, "vector-index", 0);


步骤4:使用工具
创建默认知识库 参数：name --create 
增加文件 参数： name --add filepath 
删除文件 参数：name --delete filepath 
更新文件 参数 name -- update filepath 
搜索 参数 name --search text threshold
删除默认知识库 参数：name --drop

example:
①： ./knowledgebase_tool kylin --create 
创建一个名为 kylin 的collection

②：  ./knowledgebase_tool kylin --add /home/kylin/xxx
在kylin 表中添加文件 文件路径为 /home/kylin/xxx（目录和路径均可,需要为本地真实路径文件）

③： ./knowledgebase_tool kylin --delete /home/kylin/xxx
 在kylin 表中删除文件 文件路径为 /home/kylin/xxx（仅支持传入路径）

④： ./knowledgebase_tool kylin --update /home/kylin/xxx
 在kylin 表中更新文件 文件路径为 /home/kylin/xxx（目录和路径均可，需要为本地真实路径文件）

⑤： ./knowledgebase_tool kylin --search 如何使用AI助手 0.7
 在kylin 表中进行相似搜索 搜索条件为 如何使用AI助手 会返回阈值大于0.7的内容

⑥： ./knowledgebase_tool kylin --drop 
删除一个名为 kylin 的collection







