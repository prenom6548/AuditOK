将原最新的/usr/share/kylin-ai-runtime/knowledge-base/vector.db拷贝到/home/${USER}/vector.db

把faq数据，按照demo_faq.xlsx模板填写，并保存为xlsx格式，命名为faq.xlsx存放在readme文件同目录下

执行（下述脚本会重启向量数据库服务，请确保没有正在进行的向量数据库任务）：
bash start.sh

最终会生成包含用户手册和faq的向量数据库文件：
/home/${USER}/vector.db