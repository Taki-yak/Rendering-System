#include "SceneSerializer.h"
#include <iostream>

void SceneSerializer::Save(Scene& scene, const std::string& filename)
{
    std::ofstream file(filename);

    if (!file.is_open())
    {
        std::cout << "Failed to save scene\n";
        return;
    }

    for (SceneObject* obj : scene.objects)
    {
        glm::vec3 p = obj->transform.position;
        glm::vec3 r = obj->transform.rotation;
        glm::vec3 s = obj->transform.scale;

        file << p.x << " " << p.y << " " << p.z << " ";
        file << r.x << " " << r.y << " " << r.z << " ";
        file << s.x << " " << s.y << " " << s.z << "\n";
    }

    file.close();
    std::cout << "Scene saved!\n";
}
void SceneSerializer::Load(Scene& scene, const std::string& filename,
    Mesh* mesh, Shader* shader, Material* material)
{
    std::ifstream file(filename);

    if (!file.is_open())
    {
        std::cout << "Failed to load scene\n";
        return;
    }

    // Clear old objects
    for (SceneObject* obj : scene.objects)
        delete obj;

    scene.objects.clear();

    while (!file.eof())
    {
        SceneObject* obj = new SceneObject(mesh, shader, material);

        glm::vec3 p, r, s;

        file >> p.x >> p.y >> p.z;
        file >> r.x >> r.y >> r.z;
        file >> s.x >> s.y >> s.z;

        if (file.fail()) break;

        obj->transform.position = p;
        obj->transform.rotation = r;
        obj->transform.scale = s;

        scene.AddObject(obj);
    }

    file.close();
    std::cout << "Scene loaded!\n";
}