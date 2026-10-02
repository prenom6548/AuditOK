# Qwen3-Embedding-0.6B 模型

这是一个 0.6B 参数的中文 embedding 模型，用于 skill 语义检索。

## 文件说明

### 小文件（已提交到 Git）

- `config.json` - 模型配置
- `config_sentence_transformers.json` - sentence-transformers 配置
- `configuration.json` - 基础配置
- `generation_config.json` - 生成配置
- `modules.json` - 模块定义
- `tokenizer_config.json` - 分词器配置
- `tokenizer.json` (11MB) - **分词器** ✓ 已提交
- `vocab.json` (2.7MB) - 词汇表
- `merges.txt` (1.6MB) - BPE merges
- `1_Pooling/config.json` - Pooling 层配置
- `README.md` - 原始模型说明

### 大文件（部署时下载）

- `model.safetensors` (1.2GB) - **模型权重** ⚠️ 未提交到 Git

## 部署说明

在新环境部署时，运行以下命令下载大文件：

```bash
cd models/Qwen3-Embedding-0.6B
./download_model.sh
```

该脚本会自动从 ModelScope 下载：
- `model.safetensors` (1.2GB) - 模型权重
- `tokenizer.json` (11MB) - 分词器

### 手动下载（备选）

如果脚本下载失败，可以手动下载：

```bash
cd models/Qwen3-Embedding-0.6B

# 下载模型权重
wget -c "https://modelscope.cn/models/Qwen/Qwen3-Embedding-0.6B/resolve/master/model.safetensors" \
  -O model.safetensors
```

### 使用 ModelScope CLI（推荐）

也可以使用 ModelScope CLI 下载完整模型：

```bash
pip install modelscope

# 下载到指定目录
python3 -c "
from modelscope import snapshot_download
snapshot_download('Qwen/Qwen3-Embedding-0.6B', cache_dir='./models')
"
```

## 验证安装

运行以下命令验证模型是否完整：

```bash
ls -lh model.safetensors
```

应该看到：
- `model.safetensors` - 约 1.2GB

## 模型信息

- **模型名称**: Qwen3-Embedding-0.6B
- **参数量**: 0.6B (6亿)
- **向量维度**: 1024
- **支持语言**: 中文、英文
- **适用场景**: 文本检索、语义相似度计算
- **来源**: https://modelscope.cn/models/Qwen/Qwen3-Embedding-0.6B

## 使用示例

```python
from sentence_transformers import SentenceTransformer

# 加载模型
model = SentenceTransformer('./models/Qwen3-Embedding-0.6B')

# 编码文本
embeddings = model.encode(['这是一个测试', '另一个测试'])
print(embeddings.shape)  # (2, 1024)
```

## 注意事项

1. **Git 仓库大小**: 通过排除大文件，Git 仓库从 ~2.4GB 减少到 ~30MB
2. **首次部署**: 需要运行 `download_model.sh` 下载模型权重 (1.2GB)
3. **网络要求**: 下载需要约 1.2GB 流量，建议在稳定网络环境下进行
4. **断点续传**: 使用 `wget -c` 支持断点续传，下载中断可继续
5. **磁盘空间**: 确保有至少 2GB 可用磁盘空间
