#include "Camera.h"

void Camera::Initialize(const Vector3f& p, const Vector3f& target, const Vector3f& up,
    float fov, float n, float f,
    int w, int h)
{
    mPosition = p;
    
    //求观察矩阵
    /*Vector3f l = glm::normalize(target - p); //观察方向（从相机位置指向目标位置的向量）
    Vector3f r = glm::normalize(glm::cross(up, l));
    Vector3f u = glm::normalize(glm::cross(l, r));
    
    Matrix4x4 viewMatrix = glm::transpose(Matrix4x4(
        r.x, r.y, r.z, 0.0f,
        u.x, u.y, u.z, 0.0f,
        -u.x, -u.y, -u.z, 0.0f,
        0.0f, 0.0f, 0.0f, 1.0f
        )) *
        MakeTranslation(-p);*/
    
    //left-hand 左手系 
    Matrix4x4 viewMatrix = glm::lookAtLH(p, target, up);
    
    //求投影矩阵
    Matrix4x4 projectionMatrix = glm::perspectiveFovLH_ZO(fov, (float)w,  (float)h, n, f);
    
    //求视口矩阵
    Matrix4x4 viewportMatrix = Matrix4x4(
        w / 2.0f, 0.0f, 0.0f, 0.0f,
        0.0f, -h / 2.0f, 0.0f, 0.0f,
        0.0f, 0.0f, 1.0f, 0.0f,
        w / 2.0f, h / 2.0f, 0.0f, 1.0f
    );
    
    Matrix4x4 combinedMatrix = viewportMatrix * projectionMatrix * viewMatrix;
    Matrix4x4 invCombinedMatrix = glm::inverse(combinedMatrix);
    
    mCombinedMatrix = combinedMatrix;
    mInvCombinedMatrix = invCombinedMatrix;
}

Ray Camera::getRay(float x, float y) const
{
    Ray ray;
    ray.origin = mPosition;
    
    Vector4f p(x, y, 0.0f, 1.0f);
    Vector4f worldPos = mInvCombinedMatrix * p;
    
    worldPos /= worldPos.w; //齐次坐标除以w分量得到世界坐标
    ray.direction = glm::normalize(Vector3f(worldPos) - mPosition); //从相机位置指向世界坐标的方向向量
    return ray;
}
