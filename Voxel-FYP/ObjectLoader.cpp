#include "ObjectLoader.h"
#include <unordered_map>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include <optional>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

namespace std
{
    template<> struct hash<Vertex>
    {
        size_t operator()(const Vertex& vertex) const
        {
            return (hash<glm::vec3>()(vertex.pos) ^ (hash<glm::vec3>()(vertex.normal) << 1));
        }
    };
}

namespace ObjectLoader
{
    Mesh LoadMesh(const std::string path)
    {
        tinyobj::attrib_t attrib;
        std::vector<tinyobj::shape_t> shapes;
        std::vector<tinyobj::material_t> materials;
        std::string warn, err;

        if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
            throw std::runtime_error(warn + err);

        Mesh mesh{};
        mesh.min = glm::vec3(std::numeric_limits<float>::max());
        mesh.max = glm::vec3(std::numeric_limits<float>::min());
        std::unordered_map<Vertex, uint32_t> uniqueVertices{};

        if (shapes.size() > 1)
            throw std::runtime_error("Require model to have triangles only");

        for (const auto& shape : shapes) 
        {
            for (size_t i = 0; i < shape.mesh.indices.size(); i+=3)
            {
                bool calculateNormal = true;
                std::vector<Vertex> triangleVertices;
                triangleVertices.reserve(3);

                // get vertices from the attrib
                for (size_t j = 0; j < 3; ++j)
                {
                    const auto& index = shape.mesh.indices[i+j];
                    Vertex vertex{};

                    vertex.pos = {
                        attrib.vertices[3 * index.vertex_index + 0],
                        attrib.vertices[3 * index.vertex_index + 1],
                        attrib.vertices[3 * index.vertex_index + 2]
                    };

                    if (index.normal_index >= 0) {
                        vertex.normal = {
                            attrib.normals[3 * index.normal_index + 0],
                            attrib.normals[3 * index.normal_index + 1],
                            attrib.normals[3 * index.normal_index + 2]
                        };
                        calculateNormal = false;
                    }
                    triangleVertices.push_back(vertex);
                }


                // calculate the normal of the vertices if needed
                if (calculateNormal)
                {
                    glm::vec3 edge0 = triangleVertices[0].pos - triangleVertices[2].pos;
                    glm::vec3 edge1 = triangleVertices[1].pos - triangleVertices[2].pos;
                    glm::vec3 normal = glm::normalize(glm::cross(edge0, edge1));
                    for (auto& vertex : triangleVertices)
                    {
                        vertex.normal = normal;
                    }
                }

                // add the vertices and indices to the mesh
                for (auto& vertex : triangleVertices)
                {
                    if (uniqueVertices.count(vertex) == 0)
                    {
                        uniqueVertices[vertex] = static_cast<uint32_t>(mesh.vertices.size());
                        for (int j = 0; j < 3; ++j)
                        {
                            mesh.max[j] = std::max(mesh.max[j], vertex.pos[j]);
                            mesh.min[j] = std::min(mesh.min[j], vertex.pos[j]);
                        }

                        mesh.vertices.push_back(vertex);
                    }

                    mesh.indices.push_back(uniqueVertices[vertex]);
                }
            }            
        }
        return mesh;
    }
}