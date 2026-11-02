#pragma once
#include <iostream>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>
#include <glm/ext.hpp>

using Vector2f = glm::vec2;
using Vector3f = glm::vec3;
using Vector4f = glm::vec4;
using Vector2i = glm::ivec2;
using Vector3i = glm::ivec3;
using Vector4i = glm::ivec4;
using Matrix3x3 = glm::mat3;
using Matrix4x4 = glm::mat4;
using Color = glm::vec3;

const float PI = glm::pi<float>();

inline void DumpVector(const Vector3f& v)
{
    std::cout << "Vector3f(" << v.x << ", " << v.y << ", " << v.z << ")" << std::endl;
}

// 构造一个平移矩阵 (列主序，平移量在第四列)
inline Matrix4x4 MakeTranslation(const Vector3f& t)
{
    return Matrix4x4(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        t.x,  t.y,  t.z,  1.0f
    );
}

// 构造一个均匀缩放矩阵
inline Matrix4x4 MakeScale(float s)
{
    return Matrix4x4(
        s,    0.0f, 0.0f, 0.0f,
        0.0f, s,    0.0f, 0.0f,
        0.0f, 0.0f, s,    0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );
}

// 构造一个旋转矩阵 (传入一个欧拉角)
// 注释：先绕x轴旋转，再绕y轴旋转，最后绕z轴旋转 (外旋 ZYX 顺序)
inline Matrix4x4 MakeRotation(const Vector3f& euler)
{
    float cx = cosf(euler.x);
    float sx = sinf(euler.x);
    float cy = cosf(euler.y);
    float sy = sinf(euler.y);
    float cz = cosf(euler.z);
    float sz = sinf(euler.z);

    // 绕 X 轴旋转
    Matrix4x4 rx(
        1.0f, 0.0f, 0.0f, 0.0f,
        0.0f, cx,   sx,  0.0f,
        0.0f, -sx,  cx,  0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 绕 Y 轴旋转
    Matrix4x4 ry(
        cy,   0.0f, -sy,  0.0f,
        0.0f, 1.0f, 0.0f, 0.0f,
        sy,   0.0f, cy,   0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 绕 Z 轴旋转
    Matrix4x4 rz(
        cz,   sz,   0.0f, 0.0f,
        -sz,  cz,   0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
    );

    // 组合旋转：先X，再Y，最后Z (矩阵乘法顺序为 rz * ry * rx)
    return rz * ry * rx;
}

// 构造世界变换矩阵 (平移 * 旋转 * 缩放)
inline Matrix4x4 MakeWorldTransform(const Vector3f& position, const Vector3f& rotation, float s)
{
    Matrix4x4 T = MakeTranslation(position);
    Matrix4x4 R = MakeRotation(rotation);
    Matrix4x4 S = MakeScale(s);
    
    // 注意矩阵乘法的顺序：先缩放(S)，再旋转(R)，最后平移(T)
    return T * R * S; 
}

// 构造的新坐标系到世界坐标系的变换矩阵 (正交基)
inline Matrix3x3 MakeCoordinateSystem(const Vector3f& w)
{
    // u, v
    Vector3f u(1.0f, 0.0f, 0.0f);

    // 如果 w 与 X 轴过于接近，则换一个初始 u 轴 (Y轴) 避免叉积为零
    if (glm::dot(w, u) > 0.99f)
    {
        u = Vector3f(0.0f, 1.0f, 0.0f);
    }

    Vector3f v = glm::cross(w, u);
    u = glm::cross(v, w);

    u = glm::normalize(u);
    v = glm::normalize(v);

    // 返回列主序矩阵，三列分别为 u, v, w
    return Matrix3x3(u, v, w); 
}