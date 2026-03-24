#pragma once
#include <glm/glm.hpp>

class Frustum
{
public:

    glm::vec4 planes[6];

    void Update(const glm::mat4& viewProjection);

    bool CheckPoint(const glm::vec3& point);
};