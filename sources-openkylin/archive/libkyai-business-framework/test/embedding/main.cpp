/*
 * Copyright (C) 2024 KylinSoft Co., Ltd.
 *
 * This program is free software: you can redistribute it and/or modify it under
 * the terms of the GNU General Public License as published by the Free Software
 * Foundation, either version 3 of the License, or (at your option) any later
 * version.
 *
 * This program is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * this program. If not, see <https://www.gnu.org/licenses/>.
 */

// #include <rpc/embedding/imageembeddingprocessor.h>
// #include <rpc/embedding/textembeddingprocessor.h>
// #include <unistd.h>

// #include <filesystem>
// #include <fstream>
// #include <iostream>

// #include "embeddingtaskqueue.h"
// using namespace scene;
// std::vector<uint8_t> readFile(const std::string& filePath) {
//     std::ifstream file(filePath, std::ios::binary | std::ios::ate);
//     if (!file.is_open()) {
//         throw std::runtime_error("Failed to open file");
//     }

//     std::streamsize size = file.tellg();
//     file.seekg(0, std::ios::beg);

//     std::vector<uint8_t> buffer(size);
//     if (file.read(reinterpret_cast<char*>(buffer.data()), size)) {
//         return buffer;
//     } else {
//         throw std::runtime_error("Failed to read file");
//     }
// }
// const std::string base64_chars =
//     "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
//     "abcdefghijklmnopqrstuvwxyz"
//     "0123456789+/";

// std::string base64Encode(const std::vector<uint8_t>& buffer) {
//     std::string encodedData;
//     int i = 0;
//     uint8_t char_array_3[3];
//     uint8_t char_array_4[4];

//     while (i < buffer.size()) {
//         char_array_3[0] = buffer[i++];
//         char_array_3[1] = (i < buffer.size()) ? buffer[i++] : 0;
//         char_array_3[2] = (i < buffer.size()) ? buffer[i++] : 0;

//         char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
//         char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
//         char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
//         char_array_4[3] = (char_array_3[2] & 0x3f);

//         for (int j = 0; (j < 4); ++j) {
//             encodedData += base64_chars[char_array_4[j]];
//         }
//     }

//     while ((encodedData.size() % 4) != 0) {
//         encodedData += '=';
//     }

//     return encodedData;
// }
// std::string encodeImageToBase64(std::string data) {
//     // todo
//     std::vector<uint8_t> imageData = readFile(data);
//     std::string base64Data = base64Encode(imageData);
//     return base64Data;
// }

// void printVector(const std::vector<float> vector) {
//     std::cout << "embedding(" << vector.size() << "): [ ";
//     for (const auto& value : vector) {
//         std::cout << value << " ";
//     }
//     std::cout << "]" << std::endl;
// }

// void testTextEmbedding() {
//     std::cout << "Test Text Embedding" << std::endl;

//     const std::vector<std::string> inputs{"这是一个测试用的文本段落。", "请输入这段文字进行向量化。",
//                                           "文本向量化是自然语言处理的重要技术之一，应用广泛。",
//                                           "通过向量化，文本可转为数据形式，便于计算机处理。"};

//     TextEmbeddingProcessor& te = TextEmbeddingProcessor::getInstance();
//     if (!te.init()) {
//         std::cerr << "TextEmbeddingProcessor init failed" << std::endl;
//         return;
//     }

//     std::vector<TaskID> ids;
//     for (const auto& input : inputs) {
//         TaskID id = te.addTask(input, EmbeddingDataType::Text);
//         ids.emplace_back(std::move(id));
//     }

//     for (const auto& id : ids) {
//         std::vector<float> result = te.getResult(id);
//         printVector(result);
//     }

// #if 0
//     // 与runtime重连的测试
//     sleep(5);
//     std::cout << std::endl;
//     std::cout << "Test Re Text Embedding" << std::endl;
//     const std::vector<std::string> inputsRe{"这是一个测试用的文本段落。", "请输入这段文字进行向量化。",
//                                             "文本向量化是自然语言处理的重要技术之一，应用广泛。",
//                                             "通过向量化，文本可转为数据形式，便于计算机处理。"};

//     std::vector<TaskID> idsRe;
//     for (const auto& input : inputsRe) {
//         TaskID id = te.addTask(input, EmbeddingDataType::Text);
//         idsRe.emplace_back(std::move(id));
//     }

//     for (const auto& id : idsRe) {
//         std::vector<float> result = te.getResult(id);
//         printVector(result);
//     }
// #endif
// }

// void testImageEmbedding() {
//     std::cout << "Test Image Embedding" << std::endl;

//     ImageEmbeddingProcessor& ie = ImageEmbeddingProcessor::getInstance();
//     if (!ie.init()) {
//         std::cerr << "ImageEmbeddingProcessor init failed" << std::endl;
//         return;
//     }

//     // test embedding text
//     const std::string text = "这是一个测试用的文本段落。";
//     std::cout << "test embedding image: " << text << std::endl;

//     TaskID id = ie.addTask(text, EmbeddingDataType::Text);
//     std::vector<float> result = ie.getResult(id);
//     printVector(result);

//     // test embedding image
//     namespace fs = std::filesystem;
//     fs::path dir = fs::path(__FILE__).parent_path();
//     fs::path path = dir / "20240709181353.jpg";

//     std::cout << "test embedding image: " << path.string() << std::endl;

//     id = ie.addTask(path.string(), EmbeddingDataType::ImageFlie);
//     result = ie.getResult(id);
//     printVector(result);

//     const std::string base64Image = encodeImageToBase64(path.string());

//     id = ie.addTask(base64Image, EmbeddingDataType::ImageBase64);
//     result = ie.getResult(id);
//     printVector(result);

// #if 0
//     // 与runtime重连的测试
//     std::cout << "========================================" << std::endl;
//     sleep(5);
//     std::cout << std::endl;
//     std::cout << "Test Re Image Embedding" << std::endl;

//     const std::string textRe = "这是一个测试用的文本段落。";

//     TaskID idRe = ie.addTask(textRe, EmbeddingDataType::Text);
//     std::vector<float> resultRe = ie.getResult(idRe);
//     printVector(resultRe);

//     idRe = ie.addTask(path.string(), EmbeddingDataType::ImageFlie);
//     resultRe = ie.getResult(idRe);
//     printVector(result);

//     const std::string base64ImageRe = encodeImageToBase64(path.string());

//     idRe = ie.addTask(base64ImageRe, EmbeddingDataType::ImageBase64);
//     resultRe = ie.getResult(idRe);
//     printVector(resultRe);
// #endif
// }

// void testTaskPriority() {
//     EmbeddingTaskQueue queue;

//     EmbeddingTask t1("a", EmbeddingDataType::Text);
//     queue.addTask(t1);
//     std::cout << "add task1: " << t1.id << std::endl;

//     EmbeddingTask t2("a", EmbeddingDataType::Text);
//     queue.addTask(t2);
//     std::cout << "add task2: " << t2.id << std::endl;

//     EmbeddingTask t3("c", EmbeddingDataType::Text, TaskPriority::HIGH);
//     queue.addTask(t3);
//     std::cout << "add task3: " << t3.id << std::endl;

//     EmbeddingTask task = queue.blockPopTask();
//     std::cout << "pop task: " << task.id << std::endl;
// }
// void testModelInfo() {
//     std::cout << "Test Get Text Embedding Model Info" << std::endl;

//     TextEmbeddingProcessor& te = TextEmbeddingProcessor::getInstance();
//     if (!te.init()) {
//         std::cerr << "TextEmbeddingProcessor init failed" << std::endl;
//         return;
//     }
//     te.init();
//     std::string model = te.getModelInfo();
//     std::cout << " text embedding model info: " << model << std::endl;

//     std::cout << "Test Get Image Embedding Model Info" << std::endl;

//     ImageEmbeddingProcessor& im = ImageEmbeddingProcessor::getInstance();
//     if (!te.init()) {
//         std::cerr << "ImageEmbeddingProcessor init failed" << std::endl;
//         return;
//     }
//     im.init();
//     model = im.getModelInfo();
//     std::cout << " image embedding model info: " << model << std::endl;
// }
// int main(int argc, char* argv[]) {
//     testTextEmbedding();
//     testImageEmbedding();
//     testTaskPriority();
//     testModelInfo();
//     return 0;
// }
