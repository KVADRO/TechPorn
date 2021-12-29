#pragma once

#include <algorithm>
#include <vector>
#include <string>
#include <functional>

#include <glm/glm.hpp>
#include <tiny_obj_loader.h>

class Mesh
{
public:
    using IndexType_t = unsigned int;
    using HashType_t = unsigned long long;

    struct Vertex
    {
        glm::vec3 pos;
        glm::vec3 normal;
        glm::vec2 texCoord;
        glm::vec4 color{1.0f, 0.0f, 0.0f, 1.0f};
    };

public:
    ~Mesh()
    {
        glDeleteVertexArrays(1, &m_VAO);
        
        glDeleteBuffers(1, &m_VBO);
        glDeleteBuffers(1, &m_EBO);
    }

    bool Create(const std::vector<Vertex>& vertices, const std::vector<IndexType_t>& indices)
    {
        glGenVertexArrays(1, &m_VAO);
        glBindVertexArray(m_VAO);
        
        glGenBuffers(1, &m_VBO);
        glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
        
        glGenBuffers(1, &m_EBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(IndexType_t), indices.data(), GL_STATIC_DRAW);
        
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
        
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
        
        glEnableVertexAttribArray(2);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
        
        glEnableVertexAttribArray(3);
        glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, color));
        
        glBindVertexArray(0);
        
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        
        m_IndexCount = indices.size();
        return true;
    }

    bool Create(const std::string& path)
    {
        tinyobj::attrib_t attrib;
        
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;

        std::string warn;
        std::string err;
        
        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
        {
            std::cout << "tinyobj ERROR: " << err << std::endl;
            return false;
        }

        if (!warn.empty())
        {
            std::cout << "tinyobj WARNING: " << warn << std::endl;
        }

        struct HashedVertex
        {
            HashType_t hash;
            Vertex vtx;
        };

        std::vector<HashedVertex> vertices;
        std::vector<IndexType_t> indices;

        for (const tinyobj::shape_t& shape : shapes)
        {
            for (const tinyobj::index_t& index : shape.mesh.indices)
            {
                int vIdx = index.vertex_index;
                int nIdx = index.normal_index;

                // Hashing: https://stackoverflow.com/questions/682438/hash-function-providing-unique-uint-from-an-integer-coordinate-pair
                // Hashing: http://szudzik.com/ElegantPairing.pdf

                const HashType_t hash = Hash(vIdx, nIdx);

                auto it = std::find_if(vertices.begin(), vertices.end(), [hash](const HashedVertex& vtx) { return vtx.hash == hash; });
                if (it == vertices.end())
                {
                    glm::vec3 vertex
                    {
                        attrib.vertices[3 * vIdx],
                        attrib.vertices[3 * vIdx + 1],
                        attrib.vertices[3 * vIdx + 2]
                    };

                    glm::vec3 normal
                    {
                        attrib.normals[3 * nIdx],
                        attrib.normals[3 * nIdx + 1],
                        attrib.normals[3 * nIdx + 2]
                    };

                    HashedVertex vtx;
                    vtx.hash = hash;
                    vtx.vtx.pos = vertex;
                    vtx.vtx.normal = normal;

                    vertices.push_back(vtx);
                    it = std::prev(vertices.end());
                }

                indices.push_back(std::distance(vertices.begin(), it));
            }
        }

        std::vector<Vertex> finalizedVertices;
        finalizedVertices.reserve(vertices.size());

        for (const HashedVertex& vtx : vertices)
        {
            finalizedVertices.push_back(vtx.vtx);
        }

        return Create(finalizedVertices, indices);
    }

    void Render()
    {
        glBindVertexArray(m_VAO);
        glDrawArrays(GL_TRIANGLES, 0, m_IndexCount);
    }

    IndexType_t GetIndexCount() const { return m_IndexCount; }

private:
    unsigned long Hash(int a, int b)
    {
        return a >= b ? a * a + a + b : a + b * b;
    }

private:
    GLuint m_VAO{0};
    GLuint m_VBO{0};
    GLuint m_EBO{0};
    
    IndexType_t m_IndexCount{ 0 };
};
