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
 * FOR A PARTICULAR PURPOSE. See the GNU General Public License for more
 * details.
 *
 * You should have received a copy of the GNU General Public License along with
 * thi
 */

#include <pybind11/pybind11.h>

namespace py = pybind11;

class ParserExtension {
public:
    std::string parserOffice(const std::string& filepath);
};

PYBIND11_MODULE(libparser_extension, m) {
    m.doc() = "parser extension";
    py::class_<ParserExtension>(m, "ParserExtension")
        .def(py::init<>())
        .def("parserOffice", &ParserExtension::parserOffice, py::return_value_policy::copy);
}
