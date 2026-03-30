#pragma once
#include <glm/glm.hpp>

class Frustum
{
public:

    glm::vec4 planes[6];

    void Update(const glm::mat4& viewProjection);

    bool CheckPoint(const glm::vec3& point);
    bool IsSphereVisible(const glm::vec3& center, float radius)
    {
        for (int i = 0; i < 6; i++)
        {
            glm::vec3 normal = glm::vec3(planes[i]);
            float distance = planes[i].w;

            if (glm::dot(normal, center) + distance < -radius)
                return false;
        }
        return true;
    }
};