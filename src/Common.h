#pragma once
#include <iostream>
#include <random>
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
const float INV_PI = 1.0f / PI;

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

inline float Random01()
{
    thread_local std::mt19937 gen(std::random_device{}());
    thread_local std::uniform_real_distribution<float> dist(0.f, 1.f);
    return dist(gen);
}

inline float Random(float min, float max)
{
    return Random01() * (max - min) + min;
}

inline int RandomInt(int min, int max)
{
    return static_cast<int>(Random01() * (max - min + 1) + min);
}

inline Vector3f GetSphericalCoordinate(float theta, float phi)
{
    return Vector3f(
        sinf(theta) * cosf(phi),
        sinf(theta) * sinf(phi),
        cosf(theta)
        );
}

inline bool ComputeRefractVector(const Vector3f& wi, float etaI, float etaT, Vector3f& wt)
{
    float cosThetaI = wi.z;
    float sinThetaI = sqrtf(1.0f - cosThetaI * cosThetaI);
    float sinThetaT = etaI / etaT * sinThetaI;
    //全反射
    if (sinThetaT >= 1.0f)
        return false;
    float cosThetaT = sqrtf(1.0f - sinThetaT * sinThetaT);
    wt.z = wi.z > 0 ? -cosThetaT : cosThetaT;
    wt.x = -wi.x * sinThetaT / glm::max(sinThetaI, 1e-4f);
    wt.y = -wi.y * sinThetaT / glm::max(sinThetaI, 1e-4f);
    wt = glm::normalize(wt);
    return true;
}

//在圆形内均匀采样
inline Vector2f UniformRandomDisk(float R)
{
    float e1 = Random01();
    float e2 = Random01();
    
    float r = R * sqrtf(e1);
    float theta = 2.0f * PI * e2;
    
    float x = r * cosf(theta);
    float y = r * sinf(theta);
    
    return Vector2f(x, y);
}

//在单位半球面均匀采样
inline Vector3f UniformRandomHemisphere()
{
    float e1 = Random01();
    float e2 = Random01();
    float cosTheta = e1;
    float sinTheta = sqrtf(1.0f - cosTheta * cosTheta);
    float phi = 2.0f * PI * e2;
    float x = sinTheta * glm::cos(phi);
    float y = sinTheta * glm::sin(phi);
    float z = cosTheta;
    return Vector3f(x, y, z);
}

inline Vector3f UniformRandomSphere()
{
    float e1 = Random01(); // [0,1]
    float e2 = Random01(); // [0,1]

    float theta = acosf(1.0f - 2.0f * e1); // [0, PI]
    float phi   = 2.0f * PI * e2;          // [0, 2PI]

    float x = sinf(theta) * cosf(phi);
    float y = sinf(theta) * sinf(phi);
    float z = cosf(theta);

    return Vector3f(x, y, z);
}

//三角形内均匀采样
inline Vector3f UniformRandomTriangle(const Vector3f& a, const Vector3f& b, const Vector3f& c)
{
    float e1 = Random01();
    float e2 = Random01();
    float sqtE1 = sqrtf(e1);
    float u = 1.0f - sqtE1;
    float v = e2 * sqtE1;
    Vector3f p = (1 - u - v) * a + u * b + v * c;
    return p;
}
