#include "ShaderClass.hpp"
#include "stb_image.h"

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

#include <cstring>
#include <string>
#include <thread>
#include <utility>
#include <vector>

#pragma once

#ifndef MESH_CLASS_HPP
#  define MESH_CLASS_HPP

////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                        Smart openGL handler                            ////
////                        --------------------                            ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////

enum class HandleType
{
    VAO,
    VBO,
    EBO,
    Texture
};

template <HandleType type>
struct SmartGLHandle
{
    GLuint handle{};

    SmartGLHandle() noexcept;

    SmartGLHandle(const SmartGLHandle&) = delete;
    SmartGLHandle& operator = (const SmartGLHandle&) = delete;

    SmartGLHandle(SmartGLHandle&&) noexcept;
    SmartGLHandle& operator = (SmartGLHandle&&) noexcept;

    ~SmartGLHandle() noexcept;

    operator GLuint () const noexcept { return this->handle; }
};

template <HandleType type>
SmartGLHandle<type>::SmartGLHandle() noexcept
{
    if (type == HandleType::VAO)
        glGenVertexArrays(1, &(this->handle));
    else if ((type == HandleType::VBO) || (type == HandleType::EBO))
        glGenBuffers(1, &(this->handle));
    else if (type == HandleType::Texture)
        glGenTextures(1, &(this->handle));
}

template <HandleType type>
SmartGLHandle<type>::SmartGLHandle(SmartGLHandle&& other) noexcept
{
    this->handle = other.handle;
    other.handle = 0;
}

template <HandleType type>
typename SmartGLHandle<type>&
SmartGLHandle<type>::operator = (SmartGLHandle&& other) noexcept
{
    using std::swap;
    swap(this->handle, other.handle);

    return *this;
}

template <HandleType type>
SmartGLHandle<type>::~SmartGLHandle() noexcept
{
    if (this->handle != 0)
    {
        if constexpr (type == HandleType::VAO)
            glDeleteVertexArrays(1, &(this->handle));
        else if ((type == HandleType::VBO) || (type == HandleType::EBO))
            glDeleteBuffers(1, &(this->handle));
        else if (type == HandleType::Texture)
            glDeleteTextures(1, &(this->handle));
    }
}

////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                  The mesh class and its requirements                   ////
////                  -----------------------------------                   ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////

struct Vertex
{
    glm::vec3 position{};
    glm::vec3 normal{};
    glm::vec2 tex_coords{};
    glm::vec3 tangent{};
    glm::vec3 bitangent{};
};

struct Texture
{
    SmartGLHandle<HandleType::Texture> id{};
    std::string type{};
    std::string path{};
};

struct NonOwningTexture
{
    GLuint id{};
    std::string type{};

    explicit NonOwningTexture(GLuint id, const std::string& type) noexcept :
        id{ id }, type{ type }
    {}

    NonOwningTexture(NonOwningTexture&) = delete;
    NonOwningTexture& operator = (NonOwningTexture&) = delete;

    explicit NonOwningTexture(NonOwningTexture&& other) noexcept
    {
        id = other.id;
        this->type = std::move(other.type);
    }
};

class Mesh
{
    SmartGLHandle<HandleType::VAO> VAO{};
    SmartGLHandle<HandleType::VBO> VBO{};
    SmartGLHandle<HandleType::EBO> EBO{};
    void setup_mesh() noexcept;

public:
    std::vector<Vertex> vertices{};
    std::vector<GLuint> indices{};
    std::vector<NonOwningTexture> textures{};

    explicit Mesh(std::vector<Vertex>&&, std::vector<GLuint>&&,
        std::vector<NonOwningTexture>&&) noexcept;

    void draw(const ShaderProgram&) const noexcept;
};

Mesh::Mesh(std::vector<Vertex>&& vertices,
    std::vector<GLuint>&& indices,
    std::vector<NonOwningTexture>&& textures) noexcept
{
    this->vertices = std::move(vertices);
    this->indices = std::move(indices);
    this->textures = std::move(textures);

    this->setup_mesh();
}

void Mesh::setup_mesh() noexcept
{
    glBindVertexArray(this->VAO);

    glBindBuffer(GL_ARRAY_BUFFER, this->VBO);
    glBufferData(GL_ARRAY_BUFFER, (this->vertices.size() * sizeof(Vertex)),
        this->vertices.data(), GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, this->EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER,
        (this->indices.size() * sizeof(GLuint)), this->indices.data(),
        GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, normal));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
        (void*)offsetof(Vertex, tex_coords));
    glEnableVertexAttribArray(2);
}

void Mesh::draw(const ShaderProgram& shader) const noexcept
{
    std::uint32_t diffuse_number{ 0 };
    std::uint32_t specular_number{ 0 };
    std::uint32_t normal_number{ 0 };
    std::uint32_t height_number{ 0 };

    for (std::uint32_t i{ 0 }; i < this->textures.size(); i++)
    {
        glActiveTexture(GL_TEXTURE0 + i);

        const std::string& name{ this->textures[i].type };
        const std::string number{ [&]() {
            if (name == "diffuse")
                return std::to_string(diffuse_number++);
            else if (name == "specular")
                return std::to_string(specular_number++);
            else if (name == "normal")
                return std::to_string(normal_number++);
            else if (name == "height")
                return std::to_string(height_number++);
            else
                return std::string{"unknown"};
        }() };

        shader.set_int(("material." + name + '[' + number + ']').c_str(), i);
        glBindTexture(GL_TEXTURE_2D, this->textures[i].id);
    }

    shader.set_int("material.num_diffuse", diffuse_number);
    shader.set_int("material.num_specular", specular_number);
    shader.set_int("material.num_normal", normal_number);
    shader.set_int("material.num_height", height_number);

    glBindVertexArray(this->VAO);
    glDrawElements(GL_TRIANGLES, this->indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);

    glActiveTexture(GL_TEXTURE0);
}

////////////////////////////////////////////////////////////////////////////////
////                                                                        ////
////                 The model class and its requirements                   ////
////                 ------------------------------------                   ////
////                                                                        ////
////////////////////////////////////////////////////////////////////////////////

typename SmartGLHandle<HandleType::Texture>
texture_from_file(const char*, const std::string&, bool = false,
    GLint = GL_REPEAT, GLint = GL_REPEAT, GLint = GL_REPEAT, GLint = GL_LINEAR,
    GLint = GL_LINEAR);

class Model
{
    std::vector<Texture> loaded_textures{};
    std::vector<Mesh> meshes{};
    std::string directory{};

    void load_model(const std::string&);
    void process_node(aiNode*, const aiScene*);
    Mesh process_mesh(aiMesh*, const aiScene*);

    std::vector<NonOwningTexture> load_material_textures(aiMaterial*,
        aiTextureType, std::string);

public:
    explicit Model(const char* path) { load_model(path); }

    void draw(const ShaderProgram&) const;
};

void Model::draw(const ShaderProgram& shader) const
{
    for (const auto& mesh : this->meshes)
        mesh.draw(shader);
}

void Model::load_model(const std::string& path)
{
    Assimp::Importer import{};
    const aiScene* scene{ import.ReadFile(path,
        aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals)
    };

    if (!scene || (scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE) ||
        !scene->mRootNode)
    {
        std::cerr << "Error: Assimp: " << import.GetErrorString() << '\n';
        return;
    }

    this->directory = path.substr(0, path.find_last_of('/'));
    this->process_node(scene->mRootNode, scene);
}

void Model::process_node(aiNode* node, const aiScene* scene)
{
    for (std::size_t i{ 0 }; i < node->mNumMeshes; i++)
    {
        aiMesh* mesh{ scene->mMeshes[node->mMeshes[i]] };
        this->meshes.emplace_back(std::move(process_mesh(mesh, scene)));
    }

    for (std::size_t i{ 0 }; i < node->mNumChildren; i++)
        this->process_node(node->mChildren[i], scene);
}

Mesh Model::process_mesh(aiMesh* mesh, const aiScene* scene)
{
    std::vector<Vertex> vertices{};
    std::vector<GLuint> indices{};
    std::vector<NonOwningTexture> textures{};

    vertices.reserve(mesh->mNumVertices);

    for (std::size_t i{ 0 }; i < mesh->mNumVertices; i++)
    {
        Vertex vertex{};

        vertex.position = {
            mesh->mVertices[i].x,
            mesh->mVertices[i].y,
            mesh->mVertices[i].z
        };

        vertex.normal = {
           mesh->mNormals[i].x,
           mesh->mNormals[i].y,
           mesh->mNormals[i].z
        };

        if (mesh->mTextureCoords[0])
        {
            vertex.tex_coords = {
                mesh->mTextureCoords[0][i].x,
                mesh->mTextureCoords[0][i].y
            };

            if (vertex.tex_coords == glm::vec2{0, 0})
                std::cout << "Tex coords are 0";
        }
        else
            vertex.tex_coords = { 0.0f, 0.0f };

        if (mesh->mTangents)
        {
            vertex.tangent = {
                mesh->mTangents[i].x,
                mesh->mTangents[i].y,
                mesh->mTangents[i].z
            };
        }
        else
            vertex.tangent = { 0.0f, 0.0f, 0.0f };

        if (mesh->mBitangents)
        {
            vertex.bitangent = {
                mesh->mBitangents[i].x,
                mesh->mBitangents[i].y,
                mesh->mBitangents[i].z
            };
        }
        else
            vertex.bitangent = { 0.0f, 0.0f, 0.0f };

        vertices.emplace_back(std::move(vertex));
    }

    for (std::size_t i{ 0 }; i < mesh->mNumFaces; i++)
    {
        aiFace face{ mesh->mFaces[i] };
        for (std::size_t j{ 0 }; j < face.mNumIndices; j++)
            indices.push_back(face.mIndices[j]);
    }

    aiMaterial* material{ scene->mMaterials[mesh->mMaterialIndex] };

    std::vector<NonOwningTexture> diffuse_maps{
        this->load_material_textures(material, aiTextureType_DIFFUSE,
                                     "diffuse")
    };

    std::vector<NonOwningTexture> specular_maps{
        this->load_material_textures(material, aiTextureType_SPECULAR,
                                     "specular")
    };

    std::vector<NonOwningTexture> normal_maps{
        this->load_material_textures(material, aiTextureType_HEIGHT,
                                     "normal")
    };

    std::vector<NonOwningTexture> height_maps{
        this->load_material_textures(material, aiTextureType_AMBIENT,
                                     "height")
    };

    textures.reserve(diffuse_maps.size() + specular_maps.size() +
        normal_maps.size() + height_maps.size());

    for (auto& texture : diffuse_maps)
        textures.emplace_back(std::move(texture));
    for (auto& texture : specular_maps)
        textures.emplace_back(std::move(texture));
    for (auto& texture : normal_maps)
        textures.emplace_back(std::move(texture));
    for (auto& texture : height_maps)
        textures.emplace_back(std::move(texture));

    return Mesh(std::move(vertices), std::move(indices), std::move(textures));
}

std::vector<NonOwningTexture>
Model::load_material_textures(aiMaterial* material, aiTextureType type,
    std::string type_name)
{
    std::vector<NonOwningTexture> textures{};

    for (std::size_t i{ 0 }; i < material->GetTextureCount(type); i++)
    {
        aiString str{};
        material->GetTexture(type, i, &str);

        bool skip{ false };
        for (auto& texture : this->loaded_textures)
        {
            if (!std::strcmp(texture.path.data(), str.C_Str()))
            {
                textures.emplace_back(texture.id.handle, texture.type);
                skip = true;
                break;
            }
        }

        if (!skip)
        {
            Texture texture{
                std::move(texture_from_file(str.C_Str(), this->directory)),
                type_name,
                str.C_Str()
            };

            textures.emplace_back(texture.id.handle, texture.type);
            this->loaded_textures.emplace_back(std::move(texture));
        }
    }

    return textures;
}

typename SmartGLHandle<HandleType::Texture>
texture_from_file(const char* path, const std::string& directory,
    bool gamma, GLint wrap_s, GLint wrap_t, GLint wrap_r, GLint min_filter,
    GLint mag_filter)
{
    std::string filename{ path };
    filename = directory + '/' + filename;

    SmartGLHandle<HandleType::Texture> texture_id{};
    std::int32_t width{}, height{}, number_channels{};
    
    unsigned char* data{
        stbi_load(filename.c_str(), &width, &height, &number_channels, 0)
    };

    if (data)
    {
        GLenum format = [&]() {
            if (number_channels == 1)
                return GL_RED;
            else if (number_channels == 3)
                return GL_RGB;
            else if (number_channels == 4)
                return GL_RGBA;
            else
                return GL_RED;
        }();

        glBindTexture(GL_TEXTURE_2D, texture_id);
        glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format,
            GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrap_s);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrap_t);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, wrap_r);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, min_filter);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, mag_filter);
    }
    else
        std::cerr << "Error: Unable to load texture at path: " << path << '\n';

    stbi_image_free(data);

    return texture_id;
}

#endif