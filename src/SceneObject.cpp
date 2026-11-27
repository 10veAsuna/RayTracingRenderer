#include "SceneObject.h"
#define TINYOBJLOADER_IMPLEMENTATION
#include "tiny_obj_loader.h"
#include "Triangle.h"
#include <stdexcept>

bool SceneObject::Intersect(Ray ray, Intersection& isect) const
{
    bool bHit = false;
    for (const auto& primitive : mPrimitives)
    {
        if (primitive->Intersect(ray, isect))
        {
            ray.maxt = isect.t;
            bHit = true;
        }
    }
    return bHit;
}

SceneObject::~SceneObject()
{
    /*for (const auto& primitive : mPrimitives)
    {
        delete primitive;
    }*/
}

void SceneObject::Sample(Vector3f& p, Vector3f& normal, float& pdf)
{
    //随机取一个primitive
    int index = RandomInt(0, mPrimitives.size() - 1);
    mPrimitives[index]->Sample(p, normal, pdf);
    pdf /= mPrimitives.size();
}

void SceneObject::CreatePrimitivesFromMesh(const std::string& meshFilePath)
{
    tinyobj::attrib_t attrib;
    std::vector<tinyobj::shape_t> shapes;
    std::vector<tinyobj::material_t> materials;

    std::string warn;
    std::string err;

    bool ret = tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, meshFilePath.c_str());

    if (!warn.empty()) {
        std::cout << warn << std::endl;
    }

    if (!err.empty()) {
        std::cerr << err << std::endl;
    }

    if (!ret) {
        throw std::runtime_error("cannot load OBJ '" + meshFilePath + "': " + err);
    }

    const size_t previousCount = mPrimitives.size();
    // Loop over shapes
    for (size_t s = 0; s < shapes.size(); s++)
    {
        // Loop over faces(polygon)
        size_t index_offset = 0;

        // 遍历所有面（通常是三角形面）：
        for (size_t f = 0; f < shapes[s].mesh.num_face_vertices.size(); f++)
        {
            // 该面包含的顶点数（通常是3，表示三角形面）
            size_t fv = size_t(shapes[s].mesh.num_face_vertices[f]);
            assert(fv == 3);

            // 用来存储当前三角形面的三个顶点
            Vector3f positions[3] = {};
            Vector3f normals[3] = {};
            Vector2f texcoords[3] = {};

            // 遍历该三角形面的所有顶点：
            for (size_t v = 0; v < 3; v++) {

                // access to vertex
                tinyobj::index_t idx = shapes[s].mesh.indices[index_offset + v];

                positions[v].x = attrib.vertices[3 * size_t(idx.vertex_index) + 0];
                positions[v].y = attrib.vertices[3 * size_t(idx.vertex_index) + 1];
                positions[v].z = attrib.vertices[3 * size_t(idx.vertex_index) + 2];

                // Check if `normal_index` is zero or positive. negative = no normal data
                if (idx.normal_index >= 0) {
                    normals[v].x = attrib.normals[3 * size_t(idx.normal_index) + 0];
                    normals[v].y = attrib.normals[3 * size_t(idx.normal_index) + 1];
                    normals[v].z = attrib.normals[3 * size_t(idx.normal_index) + 2];
                }

                // Check if `texcoord_index` is zero or positive. negative = no texcoord data
                if (idx.texcoord_index >= 0) {
                    texcoords[v].x = attrib.texcoords[2 * size_t(idx.texcoord_index) + 0];
                    texcoords[v].y = attrib.texcoords[2 * size_t(idx.texcoord_index) + 1];
                }
            }
            index_offset += fv;

            CreatePrimitive<Triangle>(positions, texcoords, normals);
        }
    }
    if (mPrimitives.size() == previousCount)
        throw std::runtime_error("OBJ contains no triangles: " + meshFilePath);
}
