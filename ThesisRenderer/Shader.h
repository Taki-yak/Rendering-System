#ifndef SHADER_H
#define SHADER_H

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <string>

class Shader
{
public:
    unsigned int ID;   

    Shader(const char* vertexSource, const char* fragmentSource);

    void use();

    void setMat4(const std::string& name, const float* value) const;
    void setBool(const std::string& name, bool value) const
    {
        glUniform1i(glGetUniformLocation(ID, name.c_str()), (int)value);
    }
    void setVec3(const std::string& name, const glm::vec3& value) const;
    void setInt(const std::string& name, int value) const;
    void setFloat(const std::string& name, float value) const
    {
        glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
    }
};

#endif