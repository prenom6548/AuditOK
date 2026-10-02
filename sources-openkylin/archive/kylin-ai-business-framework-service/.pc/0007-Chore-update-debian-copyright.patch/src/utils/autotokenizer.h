#ifndef AUTOTOKENIZER_H
#define AUTOTOKENIZER_H

#include <Python.h>

#include <string>
#include <map>
#include <vector>

/*
 * map {tokens_name: tokens} 例如
 * {
 *   'input_ids': [[101, 3345, 2225, 7991, 102]],
 *   'token_type_ids': [[0, 0, 0, 0, 0]],
 *   'attention_mask': [[1, 1, 1, 1, 1]]
 * }
 * input_ids 是 token，如果你的模型只需要 token 你可以只读取这个字段并传递给模型
 */
typedef std::map<std::string, std::vector<int64_t>> Token;

class AutoTokenizer {
public:
    explicit AutoTokenizer(const std::string& modelFolderPath);
    ~AutoTokenizer();
    Token tokenize(const std::string& text);

private:
    PyObject* tokenizer_ = nullptr;
};

#endif // AUTOTOKENIZER_H
