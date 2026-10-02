/*
 * Copyright 2024 KylinSoft Co., Ltd.
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
