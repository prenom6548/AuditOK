/*
 * Copyright (c) KylinSoft Co., Ltd. 2016-2024.All rights reserved.
 *
 * Authors:
 *  Alan Xie    xiehuijun@kylinos.cn
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; version 3.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "shader.h"
#include <syslog.h>

Shader::Shader(const char* vertexSrc, const char* fragmentSrc) 
	: mId(0)
{
	// vertex shader
	GLuint vertex = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertex, 1, &vertexSrc, NULL);
	glCompileShader(vertex);
	_checkCompileErrors(vertex, "VERTEX");

	// fragment Shader
	GLuint fragment = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragment, 1, &fragmentSrc, NULL);
	glCompileShader(fragment);
	_checkCompileErrors(fragment, "FRAGMENT"); 

	// shader Program
	mId = glCreateProgram();
	glAttachShader(mId, vertex);
	glAttachShader(mId, fragment);
	glLinkProgram(mId);
	_checkCompileErrors(mId, "PROGRAM");

	// delete the shaders as they're linked into our program now and no longer necessery
	glDeleteShader(vertex);
	glDeleteShader(fragment);

}

void Shader::_checkCompileErrors(GLuint shader, std::string type)
{
	GLint success;
	GLchar infoLog[1024] = {0};

	if (type != "PROGRAM") {
		glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(shader, 1024, NULL, infoLog);
			syslog(LOG_ERR, "[%s] Compile '%s' shader failed! Err msg: %s", __func__, type.c_str(), infoLog);
		}
	}
	else {
		glGetProgramiv(shader, GL_LINK_STATUS, &success);
		if (!success) {
			glGetProgramInfoLog(shader, 1024, NULL, infoLog);
			syslog(LOG_ERR, "[%s] Link program failed! Err msg: %s", __func__, infoLog);
		}
	}
}
