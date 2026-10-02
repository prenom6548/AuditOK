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

#pragma once

#include <GLES2/gl2.h>
#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

class Shader
{
public:
	Shader(const char* vertex, const char* fragment);
	virtual ~Shader() 
    {
        if (mId) glDeleteProgram(mId);
    }

    void use() { 
        glUseProgram(mId); 
    }
    
    void setBool(const std::string &name, bool value) const {         
        glUniform1i(glGetUniformLocation(mId, name.c_str()), (int)value); 
    }

    void setInt(const std::string &name, int value) const { 
        glUniform1i(glGetUniformLocation(mId, name.c_str()), value); 
    }

    void setFloat(const std::string &name, float value) const { 
        glUniform1f(glGetUniformLocation(mId, name.c_str()), value); 
    }

	GLint getAttribLoc(const std::string &name) const {
		return glGetAttribLocation(mId, name.c_str());
	}
    
	GLint getUniformLoc(const std::string &name) const {
		return glGetUniformLocation(mId, name.c_str());
	}
	

private:
    GLuint mId;
	void _checkCompileErrors(GLuint shader, std::string type);
   
};


