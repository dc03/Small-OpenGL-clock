#include <glad/glad.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#pragma once

#ifndef SHADER_CLASS_HPP
#  define SHADER_CLASS_HPP

class Shader
{
    GLuint m_shader_id{};

public:
    Shader(const std::string & = "", GLenum = GL_VERTEX_SHADER) noexcept;

    GLuint get_shader_id() const noexcept;
};

Shader::Shader(const std::string& shader_path,
    GLenum type) noexcept
{
    std::ifstream shader_source_file{ shader_path, std::ios::in };
    std::stringstream shader_source_str{};
    shader_source_str << shader_source_file.rdbuf();
    std::string shader_souce_file_rdbuf{ shader_source_str.str() };
    const char* shader_source_ptr{ shader_souce_file_rdbuf.c_str() };

    m_shader_id = glCreateShader(type);
    glShaderSource(this->m_shader_id, 1, &shader_source_ptr, nullptr);
    glCompileShader(this->m_shader_id);

    GLint status;
    char buffer[512];
    glGetShaderiv(this->m_shader_id, GL_COMPILE_STATUS, &status);
    if (!status)
    {
        glGetShaderInfoLog(this->m_shader_id, 512, nullptr, buffer);
        std::cerr << "Error: Shader compilation failed:\n" << buffer << '\n';
    }
}

GLuint Shader::get_shader_id() const noexcept
{
    return this->m_shader_id;
}

class ShaderProgram
{
    GLuint m_program_id{};
    Shader m_vertex_shader{};
    Shader m_fragment_shader{};

public:
    ShaderProgram(const std::string&, const std::string&) noexcept;

    void activate_program() const noexcept;

    void set_bool(const std::string&, GLboolean) const noexcept;
    void set_int(const std::string&, GLint) const noexcept;
    void set_float(const std::string&, GLfloat) const noexcept;

    void set_vec1(const std::string&, const glm::vec1&) const noexcept;
    void set_vec2(const std::string&, const glm::vec2&) const noexcept;
    void set_vec3(const std::string&, const glm::vec3&) const noexcept;

    void set_mat2(const std::string&, const glm::mat2&) const noexcept;
    void set_mat3(const std::string&, const glm::mat3&) const noexcept;
    void set_mat4(const std::string&, const glm::mat4&) const noexcept;

    GLuint get_program_id() const noexcept;
};

ShaderProgram::ShaderProgram(const std::string& vertex_shader_path,
    const std::string& fragment_shader_path) noexcept :
    m_vertex_shader{ vertex_shader_path, GL_VERTEX_SHADER },
    m_fragment_shader{ fragment_shader_path, GL_FRAGMENT_SHADER }
{
    this->m_program_id = glCreateProgram();
    glAttachShader(this->m_program_id, this->m_vertex_shader.get_shader_id());
    glAttachShader(this->m_program_id, this->m_fragment_shader.get_shader_id());

    glLinkProgram(this->m_program_id);

    GLint status;
    char buffer[512];
    glGetProgramiv(this->m_program_id, GL_LINK_STATUS, &status);
    if (!status)
    {
        glGetProgramInfoLog(this->m_program_id, 512, nullptr, buffer);
        std::cerr << "Error: Program linking failed:\n" << buffer << '\n';
    }

    glDeleteShader(this->m_vertex_shader.get_shader_id());
    glDeleteShader(this->m_fragment_shader.get_shader_id());
}

void ShaderProgram::activate_program() const noexcept
{
    glUseProgram(this->m_program_id);
}

void ShaderProgram::set_bool(const std::string& uniform_name, GLboolean value)
const noexcept
{
    glUniform1i(glGetUniformLocation(this->m_program_id, uniform_name.c_str()),
        static_cast<int>(value));
}

void ShaderProgram::set_int(const std::string& uniform_name, GLint value)
const noexcept
{
    glUniform1i(glGetUniformLocation(this->m_program_id, uniform_name.c_str()),
        value);
}

void ShaderProgram::set_float(const std::string& uniform_name, GLfloat value)
const noexcept
{
    glUniform1f(glGetUniformLocation(this->m_program_id, uniform_name.c_str()),
        value);
}

void ShaderProgram::set_vec1(const std::string& uniform_name,
    const glm::vec1& value) const noexcept
{
    glUniform1fv(glGetUniformLocation(this->m_program_id, uniform_name.c_str()),
        1, &value[0]);
}

void ShaderProgram::set_vec2(const std::string& uniform_name,
    const glm::vec2& value) const noexcept
{
    glUniform2fv(glGetUniformLocation(this->m_program_id, uniform_name.c_str()),
        1, &value[0]);
}

void ShaderProgram::set_vec3(const std::string& uniform_name,
    const glm::vec3& value) const noexcept
{
    glUniform3fv(glGetUniformLocation(this->m_program_id, uniform_name.c_str()),
        1, &value[0]);
}

void ShaderProgram::set_mat2(const std::string& uniform_name,
    const glm::mat2& value) const noexcept
{
    glUniformMatrix2fv(glGetUniformLocation(this->m_program_id,
        uniform_name.c_str()),
        1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::set_mat3(const std::string& uniform_name,
    const glm::mat3& value) const noexcept
{
    glUniformMatrix3fv(glGetUniformLocation(this->m_program_id,
        uniform_name.c_str()),
        1, GL_FALSE, &value[0][0]);
}

void ShaderProgram::set_mat4(const std::string& uniform_name,
    const glm::mat4& value) const noexcept
{
    glUniformMatrix4fv(glGetUniformLocation(this->m_program_id,
        uniform_name.c_str()),
        1, GL_FALSE, &value[0][0]);
}

GLuint ShaderProgram::get_program_id() const noexcept
{
    return this->m_program_id;
}

#endif