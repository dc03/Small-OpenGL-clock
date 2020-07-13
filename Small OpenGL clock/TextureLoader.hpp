#include <glad/glad.h>
#include "stb_image.h"

#include <cstddef>
#include <iostream>
#include <string>

#pragma once

#ifndef TEXTURE_LOADER_HPP
#  define TEXTURE_LOADER_HPP

class Texture
{
    GLuint m_texture_id{};

public:
    Texture(const std::string&);
    ~Texture();

    GLuint get_texture_id();
};

Texture::Texture(const std::string& texture_path)
{
    glGenTextures(1, &this->m_texture_id);
    glBindTexture(GL_TEXTURE_2D, this->m_texture_id);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    std::int32_t width{}, height{}, number_channels{};
    unsigned char* data{ stbi_load(texture_path.c_str(), &width, &height,
                                  &number_channels, 0) };
    if (data == nullptr)
        std::cerr << "Error: Unable to load texture: '" << texture_path << "'";
    else
    {
        auto format{ [&]() {
            if (number_channels == 1)
                return GL_RED;
            else if (number_channels == 3)
                return GL_RGB;
            else if (number_channels == 4)
                return GL_RGBA;
            else
                return GL_RED;
        }() };

        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    stbi_image_free(data);
}

Texture::~Texture()
{
    glDeleteTextures(1, &this->m_texture_id);
}

GLuint Texture::get_texture_id()
{
    return this->m_texture_id;
}

#endif