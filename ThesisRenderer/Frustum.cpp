#include "Frustum.h"

void Frustum::Update(const glm::mat4& vp)
{
    planes[0] = glm::vec4(vp[0][3] + vp[0][0],
        vp[1][3] + vp[1][0],
        vp[2][3] + vp[2][0],
        vp[3][3] + vp[3][0]);

    planes[1] = glm::vec4(vp[0][3] - vp[0][0],
        vp[1][3] - vp[1][0],
        vp[2][3] - vp[2][0],
        vp[3][3] - vp[3][0]);

    planes[2] = glm::vec4(vp[0][3] + vp[0][1],
        vp[1][3] + vp[1][1],
        vp[2][3] + vp[2][1],
        vp[3][3] + vp[3][1]);

    planes[3] = glm::vec4(vp[0][3] - vp[0][1],
        vp[1][3] - vp[1][1],
        vp[2][3] - vp[2][1],
        vp[3][3] - vp[3][1]);

    planes[4] = glm::vec4(vp[0][3] + vp[0][2],
        vp[1][3] + vp[1][2],
        vp[2][3] + vp[2][2],
        vp[3][3] + vp[3][2]);

    planes[5] = glm::vec4(vp[0][3] - vp[0][2],
        vp[1][3] - vp[1][2],
        vp[2][3] - vp[2][2],
        vp[3][3] - vp[3][2]);
}

bool Frustum::CheckPoint(const glm::vec3& point)
{
    for (int i = 0; i < 6; i++)
    {
        if (planes[i].x * point.x +
            planes[i].y * point.y +
            planes[i].z * point.z +
            planes[i].w <= 0)
        {
            return false;
        }
    }

    return true;

}