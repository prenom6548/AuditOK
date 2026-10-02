# kylin-ai-abstract-models

内含模型必须的文件，主要是配置文件。tokenization 模型还包含 python 模型本身。ensemble 模型还包含了一个空的版本目录`1`，因为 ensemble 模型没有模型文件，所以在这里为其保留一个必须的版本目录，这样就不需要其他组件再单独为其安装一个空目录。

## 模型列表

### bge-m3-onnx

#### bge_m3 实现的文本向量化

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| tokenizer_bge_m3_text | 文本转 token 模型。用于将文本转为 token。 | bge_m3 的词汇表 + transformers 提供的 tokenizer |
| bge_m3_text | bge_m3 文本向量化模型，用于将 token 转为向量。对于输入 token 的长度不限制，文本越长，token 数量越多。 | bge_m3 模型 |
| ensemble_bge_m3_text | 上面两个模型的 ensemble 模型 | ensemble 模型，集合 tokenizer_bge_m3_text 和 bge_m3_text 两个模型，完成 文本 到 向量 的转换 |

### cn-clip-onnx

####  cn_clip_text 实现的文本向量化

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| tokenizer_cn_clip_text | 文本转 token 模型。用于将文本转为 token。 | cn_clip 的词汇表 + transformers 提供的 tokenizer |
| cn_clip_text | cn_clip 文本向量化模型，用于将 token 转为向量。对于输入 token 的长度有限制，token 不能超过 52 个，所以在 tokenizer_cn_clip_text 中做了扩充和截断。 | cn_clip 模型 |
| ensemble_cn_clip_text | 上面两个模型的 ensemble 模型 | ensemble 模型，集合 tokenizer_cn_clip_text 和 cn_clip_text 两个模型，完成 文本 到 向量 的转换 |

#### cn_clip_image 实现的图像向量化

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| cn_clip_image | cn_clip 图像向量化模型，用于将 图像 转为向量。 | cn_clip 模型 |

### gte-base-uint8-onnx

#### gte_base_uint8 实现的文本向量化

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| tokenizer_gte_base_uint8_text | 文本转 token 模型。用于将文本转为 token。 | gte 的词汇表 + transformers 提供的 tokenizer |
| gte_base_uint8_text | gte_base 文本向量化模型，用于将 token 转为向量。对于输入 token 的长度不限制，文本越长，token 数量越多。 | gte_base 模型 |
| ensemble_gte_base_uint8_text | 上面两个模型的 ensemble 模型 | ensemble 模型，集合 tokenizer_gte_base_uint8_text 和 gte_base_uint8_text 两个模型，完成 文本 到 向量 的转换 |

### cn-clip-uint8-onnx

####  cn_clip_512_uint8_text 实现的文本向量化

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| tokenizer_cn_clip_512_uint8_text | 文本转 token 模型。用于将文本转为 token。 | cn_clip 的词汇表 + transformers 提供的 tokenizer |
| cn_clip_512_uint8_text | cn_clip 文本向量化模型，用于将 token 转为向量。对于输入 token 的长度有限制，token 不能超过 512 个，所以在 tokenizer_cn_clip_512_uint8_text 中做了扩充和截断。 | cn_clip 模型 |
| ensemble_cn_clip_512_uint8_text | 上面两个模型的 ensemble 模型 | ensemble 模型，集合 tokenizer_cn_clip_512_uint8_text 和 cn_clip_512_uint8_text 两个模型，完成 文本 到 向量 的转换 |

#### cn_clip_512_uint8_image 实现的图像向量化

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| cn_clip_512_uint8_image | cn_clip 图像向量化模型，用于将 图像 转为向量。 | cn_clip 模型 |

### ppocr

#### ocr 从图像中提取文本的Triton cpp backend

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| ocr | 用于提取图像中的文本的Triton cpp backend共享库 | ppocr |

### nlp

#### qwen_fastllm flm大语言模型

| 模型名称 | 模型简介 | 模型来源 |
| :--- | :--- | :--- |
| qwen_fastllm | 用于AI助手对话等语言功能。 | qwen 模型 |