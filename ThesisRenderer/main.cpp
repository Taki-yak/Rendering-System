#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include "stb_image.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include "Shader.h"
#include "Mesh.h"
#include "Renderer.h"
#include "Scene.h"
#include "Texture.h"
#include "Model.h"
#include "Camera.h"
#include "Cubemap.h"
#include "Frustum.h"
#include "Component.h"
#include "RotatorComponent.h"
#include "OscillatorComponent.h"
#include "SceneSerializer.h"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"
#include "InputManager.h"
#include "GridRenderer.h"
#include "EditorUI.h"
#include "RotateComponent.h"
#include "AppMode.h"
#include "PlayerController.h"
#include "AssetDatabase.h"
#include "ThirdPersonController.h"
#include "AnimationLibrary.h"
#include "AnimatedModel.h"
#include <cstdlib>
#include <ctime>
#include <fstream>
#include <sstream>
#include <vector>
#include <cmath>
// ================= CAMERA VARIABLES =================
//glm::vec3 cameraPos = glm::vec3(0.0f, 0.0f, 3.0f);
//glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
//glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
//float yaw = -90.0f;
//float pitch = 0.0f;



glm::vec3 GetRayFromMouse(double mouseX, double mouseY, int width, int height,
    glm::mat4 projection, glm::mat4 view)
{
    float x = (2.0f * mouseX) / width - 1.0f;
    float y = 1.0f - (2.0f * mouseY) / height;
    float z = 1.0f;

    glm::vec3 ray_nds = glm::vec3(x, y, z);
    glm::vec4 ray_clip = glm::vec4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);

    glm::vec4 ray_eye = glm::inverse(projection) * ray_clip;
    ray_eye = glm::vec4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

    glm::vec3 ray_world = glm::vec3(glm::inverse(view) * ray_eye);
    ray_world = glm::normalize(ray_world);

    return ray_world;
}

bool isSelected;
bool useFrustumCulling = true;
double previousTime = glfwGetTime();
int frameCount = 0;
float lastX = 400;
float lastY = 300;
bool firstMouse = true;
void TestAssimp();
float deltaTime = 0.0f;
float lastFrame = 0.0f;
bool useCulling = true;
bool cKeyPressed = false;
bool nKeyPressed = false;
bool nKeyLastState = false;
bool mKeyPressed = false;
bool mouseClicked = false;
bool pPressed = false;
bool lPressed = false;
bool bPressed = false;
bool isDragging = false;
bool snapEnabled = false;
bool gPressed = false;
float gridSize = 1.0f;
bool useGridSnap = true;
bool blockEditorMouseLook =
false;
float playCameraLookOffset =0.0f;
bool rightMouseCameraActive =
false;

bool ignoreNextMouseDelta =
false;
bool IsEditorSavedObject(SceneObject* object)
{
    if (object == nullptr)
        return false;

    if (object->name == "Player")
        return false;

    if (object->name == "Ground")
        return false;

    if (object->name == "Forest Cabin Environment")
        return false;

    if (object->name == "Imported Tree")
        return false;

    if (object->name.find("Generated") != std::string::npos)
        return false;

    if (object->name.find("Border") != std::string::npos)
        return false;

    if (object->name.find("Grass Terrain Tile") != std::string::npos)
        return false;

    return true;
}

std::string GetEditorObjectType(SceneObject* object)
{
    if (object == nullptr)
        return "Unknown";

    if (object->name.find("Wooden House") != std::string::npos)
        return "WoodenHouse";

    if (object->name.find("Camp House") != std::string::npos)
        return "WoodenHouse";

    if (object->name.find("Pine Tree") != std::string::npos)
        return "PineTree";

    if (object->name.find("Camp Tree") != std::string::npos)
        return "PineTree";

    if (object->name.find("Common Tree") != std::string::npos)
        return "CommonTree";

    if (object->name.find("Rock") != std::string::npos)
        return "Rock";

    if (object->name.find("Bush") != std::string::npos)
        return "Bush";

    if (object->name.find("Wood Log") != std::string::npos)
        return "WoodLog";

    if (object->name.find("Camp Log") != std::string::npos)
        return "WoodLog";

    if (object->name.find("Tree Stump") != std::string::npos)
        return "TreeStump";

    if (object->name.find("Camp Stump") != std::string::npos)
        return "TreeStump";

    if (object->name.find("Grass") != std::string::npos)
        return "Grass";

    if (object->name.find("Wall") != std::string::npos)
        return "Cube";

    if (object->name.find("Fence") != std::string::npos)
        return "Cube";

    if (object->name.find("Path") != std::string::npos)
        return "Cube";

    if (object->name.find("Platform") != std::string::npos)
        return "Cube";

    return "Cube";
}

void SaveEditorObjects(
    Scene& scene,
    const std::string& filePath
)
{
    std::ofstream file(
        filePath
    );

    if (!file.is_open())
    {
        std::cout
            << "Failed to save editor objects."
            << std::endl;

        return;
    }

    for (SceneObject* object : scene.objects)
    {
        if (!IsEditorSavedObject(object))
            continue;

        std::string objectType =
            GetEditorObjectType(
                object
            );

        file
            << objectType << "|"
            << object->name << "|"

            << object->transform.position.x << " "
            << object->transform.position.y << " "
            << object->transform.position.z << "|"

            << object->transform.rotation.x << " "
            << object->transform.rotation.y << " "
            << object->transform.rotation.z << "|"

            << object->transform.scale.x << " "
            << object->transform.scale.y << " "
            << object->transform.scale.z << "|"

            << object->isCollider
            << "\n";
    }

    file.close();

    std::cout
        << "Editor objects saved with prefab types to "
        << filePath
        << std::endl;
}
Model* GetModelFromEditorType(
    const std::string& objectType,
    Model* woodenHouseModel,
    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel,
    Model* grassModel
)
{
    if (objectType == "WoodenHouse")
        return woodenHouseModel;

    if (objectType == "PineTree")
        return pineTreeModel;

    if (objectType == "CommonTree")
        return commonTreeModel;

    if (objectType == "Rock")
        return rockModel;

    if (objectType == "Bush")
        return bushModel;

    if (objectType == "WoodLog")
        return woodLogModel;

    if (objectType == "TreeStump")
        return treeStumpModel;

    if (objectType == "Grass")
        return grassModel;

    return nullptr;
}

void LoadEditorObjects(
    Scene& scene,
    const std::string& filePath,
    Mesh* cubeMesh,
    Shader* shader,
    Material* material,
    SceneObject*& selectedObject,
    Model* woodenHouseModel,
    Model* pineTreeModel,
    Model* commonTreeModel,
    Model* rockModel,
    Model* bushModel,
    Model* woodLogModel,
    Model* treeStumpModel,
    Model* grassModel
)
{
    std::ifstream file(
        filePath
    );

    if (!file.is_open())
    {
        std::cout
            << "Failed to load editor objects."
            << std::endl;

        return;
    }

    std::string line;

    while (std::getline(file, line))
    {
        if (line.empty())
            continue;

        std::stringstream lineStream(
            line
        );

        std::string typePart;
        std::string namePart;
        std::string positionPart;
        std::string rotationPart;
        std::string scalePart;
        std::string colliderPart;

        std::getline(lineStream, typePart, '|');
        std::getline(lineStream, namePart, '|');
        std::getline(lineStream, positionPart, '|');
        std::getline(lineStream, rotationPart, '|');
        std::getline(lineStream, scalePart, '|');
        std::getline(lineStream, colliderPart, '|');

        SceneObject* object =
            nullptr;

        Model* model =
            GetModelFromEditorType(
                typePart,
                woodenHouseModel,
                pineTreeModel,
                commonTreeModel,
                rockModel,
                bushModel,
                woodLogModel,
                treeStumpModel,
                grassModel
            );

        if (model != nullptr)
        {
            object =
                new SceneObject(
                    model,
                    shader
                );
        }
        else
        {
            object =
                new SceneObject(
                    cubeMesh,
                    shader,
                    material
                );
        }

        object->name =
            namePart;

        std::stringstream positionStream(
            positionPart
        );

        positionStream
            >> object->transform.position.x
            >> object->transform.position.y
            >> object->transform.position.z;

        std::stringstream rotationStream(
            rotationPart
        );

        rotationStream
            >> object->transform.rotation.x
            >> object->transform.rotation.y
            >> object->transform.rotation.z;

        std::stringstream scaleStream(
            scalePart
        );

        scaleStream
            >> object->transform.scale.x
            >> object->transform.scale.y
            >> object->transform.scale.z;

        object->isCollider =
            colliderPart == "1";

        object->boundingRadius =
            80.0f;

        object->colliderRadius =
            glm::max(
                object->transform.scale.x,
                object->transform.scale.z
            ) * 0.8f;

        scene.AddObject(
            object
        );

        selectedObject =
            object;
    }

    file.close();

    std::cout
        << "Editor objects loaded with prefab types from "
        << filePath
        << std::endl;
}
enum MoveAxis
{
    NONE,
    AXIS_X,
    AXIS_Y,
    AXIS_Z
};
enum GizmoMode
{
    TRANSLATE,
    ROTATE,
    SCALE
};
MoveAxis currentAxis = NONE;
GizmoMode currentGizmoMode = TRANSLATE;
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods)
{
    ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);

    if (ImGui::GetIO().WantCaptureMouse)
        return;

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        mouseClicked = true;
        isDragging = true;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        isDragging = false;
    }
}

SceneObject* selectedObject = nullptr;
Light* selectedLight = nullptr;
Camera camera;
bool editorCameraStartFixed =false;
Frustum frustum;
PlayerController playerController;
ThirdPersonController thirdPersonController;
SceneObject* playerObject = nullptr;
glm::vec3 playerSpawnPosition =
glm::vec3(
    0.0f,
    0.0f,
    -5.0f
);
// ================= MOUSE CALLBACK ==================
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
    ImGuiIO& io =
        ImGui::GetIO();

    if (
        !rightMouseCameraActive ||
        io.WantCaptureMouse
        )
    {
        lastX =
            static_cast<float>(xpos);

        lastY =
            static_cast<float>(ypos);

        firstMouse =
            true;

        return;
    }

    if (
        firstMouse ||
        ignoreNextMouseDelta
        )
    {
        lastX =
            static_cast<float>(xpos);

        lastY =
            static_cast<float>(ypos);

        firstMouse =
            false;

        ignoreNextMouseDelta =
            false;

        return;
    }

    float xoffset =
        static_cast<float>(xpos) -
        lastX;

    float yoffset =
        lastY -
        static_cast<float>(ypos);

    lastX =
        static_cast<float>(xpos);

    lastY =
        static_cast<float>(ypos);

    if (
        std::abs(xoffset) > 80.0f ||
        std::abs(yoffset) > 80.0f
        )
    {
        return;
    }

    if (blockEditorMouseLook)
    {
        playCameraLookOffset +=
            yoffset *
            0.02f;

        playCameraLookOffset =
            glm::clamp(
                playCameraLookOffset,
                -0.6f,
                5.5f
            );

        return;
    }

    camera.ProcessMouseMovement(
        xoffset,
        yoffset
    );
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (selectedObject != nullptr)
    {
        float scaleSpeed = 0.2f;

        selectedObject->transform.scale +=
            glm::vec3(yoffset * scaleSpeed);

        selectedObject->transform.scale =
            glm::max(
                selectedObject->transform.scale,
                glm::vec3(0.1f)
            );
    }
}
float gizmoVertices[] =
{
    // X axis (red)
    0.0f, 0.0f, 0.0f,
    2.0f, 0.0f, 0.0f,

    // Y axis (green)
    0.0f, 0.0f, 0.0f,
    0.0f, 2.0f, 0.0f,

    // Z axis (blue)
    0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 2.0f
};
// ================= SHADERS =================
const char* gridVertexShader = R"(

#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * vec4(aPos, 1.0);
}

)";
const char* gridFragmentShader = R"(

#version 330 core

out vec4 FragColor;

void main()
{
    FragColor = vec4(0.3, 0.3, 0.3, 1.0);
}

)";
const char* vertexShaderSource = R"(

#version 330 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

out vec3 FragPos;
out vec3 Normal;
out vec2 TexCoord;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    FragPos = vec3(model * vec4(aPos, 1.0));
    Normal = mat3(transpose(inverse(model))) * aNormal;
    TexCoord = aTexCoord;

    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

)";
const char* fragmentShaderSource = R"(

#version 330 core
#define MAX_LIGHTS 5

out vec4 FragColor;
uniform vec3 sunDirection;
uniform vec3 sunColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform bool isSelected;
uniform sampler2D texture1;
uniform sampler2D terrainGrassTex;
uniform sampler2D terrainCliffTex;
uniform vec3 materialTint;
uniform vec3 viewPos;
uniform bool isProceduralTerrain;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;
uniform bool useTexture;

void main()
{
    vec3 textureColor;
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

if (isProceduralTerrain)
{
    float slope =
        1.0 -
        clamp(norm.y, 0.0, 1.0);

    float heightValue =
        FragPos.y;

    vec3 valleyGrass =
        vec3(0.34, 0.44, 0.20);

    vec3 hillGrass =
        vec3(0.43, 0.53, 0.27);

    vec3 dryGrass =
        vec3(0.52, 0.49, 0.30);

    vec3 dirt =
        vec3(0.55, 0.42, 0.27);

    vec3 cliff =
        vec3(0.46, 0.44, 0.40);

    vec3 baseColor =
        mix(
            valleyGrass,
            hillGrass,
            smoothstep(-3.0, 10.0, heightValue)
        );

    baseColor =
        mix(
            baseColor,
            dryGrass,
            smoothstep(10.0, 22.0, heightValue) * 0.45
        );

    baseColor =
        mix(
            baseColor,
            dirt,
            smoothstep(0.16, 0.34, slope)
        );

    baseColor =
        mix(
            baseColor,
            cliff,
            smoothstep(0.36, 0.62, slope)
        );

    float broadVariation =
        sin(FragPos.x * 0.018) *
        cos(FragPos.z * 0.021);

    textureColor =
        baseColor +
        vec3(
            broadVariation * 0.025,
            broadVariation * 0.030,
            broadVariation * 0.012
        );
}
else if (useTexture)
{
    textureColor =
        texture(texture1, TexCoord).rgb *
        materialTint;
}
else
{
    textureColor =
        materialTint;
}
    vec3 result = vec3(0.0);
// Directional Light

vec3 dirLight =
normalize(-sunDirection);

float dirDiff =
max(dot(norm, dirLight), 0.12);

vec3 dirDiffuse =
dirDiff *
materialDiffuse *
textureColor *
sunColor;

vec3 dirReflect =
reflect(-dirLight,norm);

float dirSpec =
pow(
max(dot(viewDir,dirReflect),0.0),
materialShininess
);

vec3 dirSpecular =
materialSpecular *
dirSpec *
sunColor;

vec3 dirAmbient =
materialAmbient *
textureColor *
0.34;

result +=
    dirAmbient +
    dirDiffuse +
    dirSpecular;

float hemi =
    clamp(norm.y * 0.5 + 0.5, 0.0, 1.0);

vec3 hemiLight =
    mix(
        vec3(0.08, 0.07, 0.06),
        vec3(0.20, 0.24, 0.28),
        hemi
    );

result +=
    textureColor * hemiLight;

    for(int i = 0; i < MAX_LIGHTS; i++)
{
    if (length(lightColors[i]) < 0.001)
    {
        continue;
    }

    vec3 toLight =
        lightPositions[i] - FragPos;

    float distanceToLight =
        length(toLight);

    if (distanceToLight > 18.0)
    {
        continue;
    }

    vec3 lightDir =
        normalize(toLight);

    float attenuation =
        1.0 /
        (
            1.0 +
            0.18 * distanceToLight +
            0.08 * distanceToLight * distanceToLight
        );

    float diff =
        max(dot(norm, lightDir), 0.0);

    vec3 diffuse =
        diff *
        materialDiffuse *
        textureColor *
        lightColors[i] *
        attenuation;

    vec3 reflectDir =
        reflect(-lightDir, norm);

    float spec =
        pow(
            max(dot(viewDir, reflectDir), 0.0),
            materialShininess
        );

    vec3 specular =
        materialSpecular *
        spec *
        lightColors[i] *
        attenuation;

    vec3 ambient =
        materialAmbient *
        textureColor *
        lightColors[i] *
        0.02 *
        attenuation;

    result +=
        ambient +
        diffuse +
        specular;
}
if(isSelected)
{
    result *= 1.3;
}


result *= 0.90;
float fogDistance =
    distance(viewPos, FragPos);

float fogFactor =
    clamp(
        (fogDistance - 120.0) / 180.0,
        0.0,
        1.0
    );

vec3 fogColor =
    vec3(0.70, 0.77, 0.84);

result =
    mix(
        result,
        fogColor,
        fogFactor
    );

result = clamp(result, 0.0, 1.0);

FragColor = vec4(result, 1.0);

}

)";
const char* lightVertexSource = R"(
#version 330 core
layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}
)";

const char* lightFragmentSource = R"(

#version 330 core
out vec4 FragColor;

void main()
{
    FragColor = vec4(1.0);
}

)";
const char* skyboxVertex = R"(

#version 330 core
layout (location = 0) in vec3 aPos;

out vec3 TexCoords;

uniform mat4 view;
uniform mat4 projection;

void main()
{
    TexCoords = aPos;

    vec4 pos = projection * view * vec4(aPos, 1.0);
    gl_Position = pos.xyww;
}

)";
const char* skyboxFragment = R"(

#version 330 core

out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, TexCoords);
}

)";
const char* gizmoVertexShader = R"(

#version 330 core

layout (location = 0) in vec3 aPos;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main()
{
    gl_Position = projection * view * model * vec4(aPos, 1.0);
}

)";
const char* gizmoFragmentShader = R"(

#version 330 core

out vec4 FragColor;

uniform vec3 axisColor;

void main()
{
    FragColor = vec4(axisColor, 1.0);
}

)";
std::vector<std::string> faces =
{
    "textures/skybox/right.jpg",
    "textures/skybox/left.jpg",
    "textures/skybox/top.jpg",
    "textures/skybox/bottom.jpg",
    "textures/skybox/front.jpg",
    "textures/skybox/back.jpg"
};
float GetTerrainHeight(
    float x,
    float z
)
{
    float dist =
        std::sqrt(x * x + z * z);

    float base =
        std::sin(x * 0.010f) * 2.2f +
        std::cos(z * 0.012f) * 1.8f +
        std::sin((x + z) * 0.008f) * 1.5f;

    base +=
        std::sin(x * 0.035f) *
        std::cos(z * 0.030f) *
        2.2f;

    float peak1 =
        26.0f *
        std::exp(
            -(
                (x - 55.0f) * (x - 55.0f) +
                (z + 35.0f) * (z + 35.0f)
                ) / 2200.0f
        );

    float peak2 =
        18.0f *
        std::exp(
            -(
                (x + 70.0f) * (x + 70.0f) +
                (z - 45.0f) * (z - 45.0f)
                ) / 2600.0f
        );

    float peak3 =
        22.0f *
        std::exp(
            -(
                (x - 15.0f) * (x - 15.0f) +
                (z - 95.0f) * (z - 95.0f)
                ) / 1800.0f
        );

    float ring =
        0.0f;

    if (dist > 120.0f)
    {
        float t =
            (dist - 120.0f) / 90.0f;

        t =
            glm::clamp(
                t,
                0.0f,
                1.0f
            );

        ring =
            t * t * 38.0f;
    }

    return
        base +
        peak1 +
        peak2 +
        peak3 +
        ring -
        2.0f;
}
float GetPlayerTerrainY(
    float x,
    float z
)
{
    return
        GetTerrainHeight(
            x,
            z
        ) + 0.55f;
}

glm::vec3 GetTerrainNormal(
    float x,
    float z
)
{
    float heightLeft =
        GetTerrainHeight(
            x - 1.0f,
            z
        );

    float heightRight =
        GetTerrainHeight(
            x + 1.0f,
            z
        );

    float heightDown =
        GetTerrainHeight(
            x,
            z - 1.0f
        );

    float heightUp =
        GetTerrainHeight(
            x,
            z + 1.0f
        );

    glm::vec3 normal =
        glm::normalize(
            glm::vec3(
                heightLeft - heightRight,
                4.0f,
                heightDown - heightUp
            )
        );

    return normal;
}

float GetObjectTerrainY(
    float x,
    float z,
    float offset = 0.05f
)
{
    return
        GetTerrainHeight(
            x,
            z
        ) + offset;
}

glm::vec3 SnapObjectToTerrain(
    glm::vec3 position,
    float offset = 0.05f
)
{
    position.y =
        GetObjectTerrainY(
            position.x,
            position.z,
            offset
        );

    return position;
}

bool IsGoodTerrainSpawnPoint(
    float x,
    float z
)
{
    glm::vec3 normal =
        GetTerrainNormal(
            x,
            z
        );

    return normal.y > 0.72f;
}
void SetObjectMetadata(
    SceneObject* object,
    const std::string& assetId,
    AssetType assetType,
    SpawnSource spawnSource,
    bool persistent,
    bool showInHierarchy
)
{
    if (object == nullptr)
        return;

    object->assetId =
        assetId;

    object->assetType =
        assetType;

    object->spawnSource =
        spawnSource;

    object->persistent =
        persistent;

    object->showInHierarchy =
        showInHierarchy;
}
void AddTerrainVertex(
    std::vector<float>& vertices,
    float x,
    float z
)
{
    float y =
        GetTerrainHeight(
            x,
            z
        );

    glm::vec3 normal =
        GetTerrainNormal(
            x,
            z
        );
    float textureU =
        x * 0.035f;

    float textureV =
        z * 0.035f;

    // position
    vertices.push_back(x);
    vertices.push_back(y);
    vertices.push_back(z);

    // normal
    vertices.push_back(normal.x);
    vertices.push_back(normal.y);
    vertices.push_back(normal.z);

    // texcoord
    vertices.push_back(textureU);
    vertices.push_back(textureV);
}

void BuildProceduralTerrain(
    std::vector<float>& vertices,
    float terrainSize,
    int resolution
)
{
    vertices.clear();

    float halfSize =
        terrainSize * 0.5f;

    float step =
        terrainSize /
        static_cast<float>(resolution);

    for (int x = 0; x < resolution; x++)
    {
        for (int z = 0; z < resolution; z++)
        {
            float x0 =
                -halfSize +
                x * step;

            float z0 =
                -halfSize +
                z * step;

            float x1 =
                x0 + step;

            float z1 =
                z0 + step;

            // first triangle
            AddTerrainVertex(vertices, x0, z0);
            AddTerrainVertex(vertices, x1, z0);
            AddTerrainVertex(vertices, x1, z1);

            // second triangle
            AddTerrainVertex(vertices, x0, z0);
            AddTerrainVertex(vertices, x1, z1);
            AddTerrainVertex(vertices, x0, z1);
        }
    }
}
void UpdatePlayModeCameraFollow(
    Camera& camera,
    SceneObject* playerObject,
    float deltaTime
)
{
    if (playerObject == nullptr)
        return;

    static bool cameraFollowInitialized =
        false;

    static glm::vec3 smoothedFollowPosition =
        glm::vec3(
            0.0f
        );

    glm::vec3 playerPosition =
        playerObject->transform.position;

    glm::vec3 playerGroundPosition =
        glm::vec3(
            playerPosition.x,
            GetPlayerTerrainY(
                playerPosition.x,
                playerPosition.z
            ),
            playerPosition.z
        );

    if (
        !cameraFollowInitialized ||
        deltaTime > 0.2f
        )
    {
        smoothedFollowPosition =
            playerGroundPosition;

        cameraFollowInitialized =
            true;
    }

    float horizontalSmooth =
        glm::clamp(
            deltaTime * 12.0f,
            0.0f,
            1.0f
        );

    float verticalSmooth =
        glm::clamp(
            deltaTime * 3.5f,
            0.0f,
            1.0f
        );

    smoothedFollowPosition.x =
        glm::mix(
            smoothedFollowPosition.x,
            playerGroundPosition.x,
            horizontalSmooth
        );

    smoothedFollowPosition.z =
        glm::mix(
            smoothedFollowPosition.z,
            playerGroundPosition.z,
            horizontalSmooth
        );

    smoothedFollowPosition.y =
        glm::mix(
            smoothedFollowPosition.y,
            playerGroundPosition.y,
            verticalSmooth
        );

    glm::vec3 cameraForward =
        glm::vec3(
            camera.Front.x,
            0.0f,
            camera.Front.z
        );

    if (glm::length(cameraForward) < 0.001f)
    {
        cameraForward =
            glm::vec3(
                0.0f,
                0.0f,
                -1.0f
            );
    }

    cameraForward =
        glm::normalize(
            cameraForward
        );

    glm::vec3 lookTarget =
        smoothedFollowPosition +
        glm::vec3(
            0.0f,
            1.25f + playCameraLookOffset,
            0.0f
        );

    glm::vec3 desiredCameraPosition =
        smoothedFollowPosition
        - cameraForward * 6.5f
        + glm::vec3(
            0.0f,
            2.65f + playCameraLookOffset * 0.25f,
            0.0f
        );
    float cameraSmooth =
        glm::clamp(
            deltaTime * 10.0f,
            0.0f,
            1.0f
        );

    camera.Position =
        glm::mix(
            camera.Position,
            desiredCameraPosition,
            cameraSmooth
        );

    camera.Front =
        glm::normalize(
            lookTarget -
            camera.Position
        );
}
// ================= MAIN =================
glm::vec3 rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

int main()
{

    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(1600, 900, "Orion", NULL, NULL);
    if (!window)
    {
        std::cout << "Failed to create window\n";
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);

    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        std::cout << "Failed to initialize GLAD\n";
        return -1;
    }

    Cubemap skybox(faces);
    glfwSetScrollCallback(window, scroll_callback);
    glfwSetCursorPosCallback(window, mouse_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);
    glEnable(GL_DEPTH_TEST);
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();

    ImGuiIO& io = ImGui::GetIO();
    (void)io;

    ImGui::StyleColorsDark();

    ImGui_ImplGlfw_InitForOpenGL(window, false);

    glfwSetCharCallback(
        window,
        ImGui_ImplGlfw_CharCallback
    );

    glfwSetKeyCallback(
        window,
        ImGui_ImplGlfw_KeyCallback
    );
    ImGui_ImplOpenGL3_Init("#version 330");
    glDisable(GL_CULL_FACE);
    TestAssimp();
    AnimationLibrary playerAnimations;
    InputManager::Init(window);
    playerAnimations.LoadAnimation(
        "Idle",
        "Assets/Models/Characters/Player/Idle.fbx"
    );

    playerAnimations.LoadAnimation(
        "Walk",
        "Assets/Models/Characters/Player/Walk.fbx"
    );

    playerAnimations.LoadAnimation(
        "Run",
        "Assets/Models/Characters/Player/Run.fbx"
    );

    playerAnimations.LoadAnimation(
        "Jump",
        "Assets/Models/Characters/Player/Jump.fbx"
    );
 
    AssetDatabase::Initialize();
    // ================= SHADER CLASS =================
    unsigned int gizmoVAO, gizmoVBO;

    glGenVertexArrays(1, &gizmoVAO);
    glGenBuffers(1, &gizmoVBO);

    glBindVertexArray(gizmoVAO);

    glBindBuffer(GL_ARRAY_BUFFER, gizmoVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(gizmoVertices), gizmoVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glBindVertexArray(0);
    Shader shader(vertexShaderSource, fragmentShaderSource);
    glm::vec3 lightPositions[] = {
    glm::vec3(2.0f, 2.0f, 2.0f),
    glm::vec3(-2.0f, 2.0f, 2.0f),
    glm::vec3(0.0f, 3.0f, -3.0f)
    };



    shader.setBool("isSelected", isSelected);
    glm::vec3 lightColors[] = {
        glm::vec3(1.0f),
        glm::vec3(1.0f, 0.8f, 0.6f),
        glm::vec3(0.6f, 0.6f, 1.0f)
    };
    Renderer renderer;
    Scene scene;
    PlayerController playerController;
    ThirdPersonController thirdPersonController;
  

    AppMode appMode = AppMode::Editor;
    static int lightCounter = 1;
   /* Light* testLight = new Light();

  
    testLight->name =
        "Light_" +
        std::to_string(lightCounter++);

    testLight->position =
        glm::vec3(
            0.0f,
            3.0f,
            0.0f
        );

    scene.AddLight(testLight);
    testLight = new Light();

    testLight->name = "Light 1";

    testLight->position =
        glm::vec3(
            0.0f,
            3.0f,
            0.0f
        );


    scene.AddLight(testLight);*/
   
    Light* sun = new Light();

    sun->name = "Sun";

    sun->type = LightType::Directional;

    sun->direction =
        glm::normalize(
            glm::vec3(-0.2f, -1.0f, -0.3f));

    scene.AddLight(sun);
    Model myModel("character-a.obj");
    AnimatedModel testAnimatedPlayer(
        "Assets/Models/Characters/Player/Idle.fbx"
    );
    Model treeModel("character-human.obj");
   Model importedTree(
        "Assets/Models/Nature/Tree.obj"
    );
   Model pineTree4Model(
       "Assets/Models/Environment/NaturePack/PineTree_4.obj"
   );

   Model pineTree5Model(
       "Assets/Models/Environment/NaturePack/PineTree_5.obj"
   );

   Model commonTree3Model(
       "Assets/Models/Environment/NaturePack/CommonTree_3.obj"
   );

   Model commonTree4Model(
       "Assets/Models/Environment/NaturePack/CommonTree_4.obj"
   );

   Model birchTree3Model(
       "Assets/Models/Environment/NaturePack/BirchTree_3.obj"
   );

   Model willowTreeModel(
       "Assets/Models/Environment/NaturePack/Willow_1.obj"
   );

   Model willowTree2Model(
       "Assets/Models/Environment/NaturePack/Willow_2.obj"
   );

   Model rock4Model(
       "Assets/Models/Environment/NaturePack/Rock_4.obj"
   );

   Model rock5Model(
       "Assets/Models/Environment/NaturePack/Rock_5.obj"
   );

   Model rockMoss2Model(
       "Assets/Models/Environment/NaturePack/Rock_Moss_2.obj"
   );

   Model rockMoss3Model(
       "Assets/Models/Environment/NaturePack/Rock_Moss_3.obj"
   );

   Model bushBerriesModel(
       "Assets/Models/Environment/NaturePack/BushBerries_1.obj"
   );

   Model grass2Model(
       "Assets/Models/Environment/NaturePack/Grass_2.obj"
   );

   Model wheatModel(
       "Assets/Models/Environment/NaturePack/Wheat.obj"
   );
    Model forestEnvironment(
        "Assets/Models/Environment/terrain.obj"
    );
    Model grassTerrainModel(
        "Assets/Models/Environment/ForestDemo/Mineways2Skfb.obj",
        "Assets/Models/Environment/ForestDemo/"
    );
    Model pineTreeModel(
        "Assets/Models/Environment/NaturePack/PineTree_1.obj"
    );

    Model commonTreeModel(
        "Assets/Models/Environment/NaturePack/CommonTree_1.obj"
    );

    Model rockModel(
        "Assets/Models/Environment/NaturePack/Rock_1.obj"
    );

    Model bushModel(
        "Assets/Models/Environment/NaturePack/Bush_1.obj"
    );

    Model grassModel(
        "Assets/Models/Environment/NaturePack/Grass.obj"
    );
    Model grassPatchModel(
        "Assets/Models/Environment/NaturePack/Grass_Short.obj"
    );

    Model woodLogModel(
        "Assets/Models/Environment/NaturePack/WoodLog.obj"
    );

    Model pineTree3Model(
        "Assets/Models/Environment/NaturePack/PineTree_3.obj"
    );

    Model birchTreeModel(
        "Assets/Models/Environment/NaturePack/BirchTree_1.obj"
    );

    Model birchTree2Model(
        "Assets/Models/Environment/NaturePack/BirchTree_2.obj"
    );

    Model commonTree2Model(
        "Assets/Models/Environment/NaturePack/CommonTree_2.obj"
    );

    Model rock2Model(
        "Assets/Models/Environment/NaturePack/Rock_2.obj"
    );

    Model rock3Model(
        "Assets/Models/Environment/NaturePack/Rock_3.obj"
    );

    Model rockMossModel(
        "Assets/Models/Environment/NaturePack/Rock_Moss_1.obj"
    );

    Model plantModel(
        "Assets/Models/Environment/NaturePack/Plant_1.obj"
    );

    Model plant2Model(
        "Assets/Models/Environment/NaturePack/Plant_2.obj"
    );

    Model flowersModel(
        "Assets/Models/Environment/NaturePack/Flowers.obj"
    );

    Model treeStumpModel(
        "Assets/Models/Environment/NaturePack/TreeStump.obj"
    );
    Model pineTree2Model(
        "Assets/Models/Environment/NaturePack/PineTree_2.obj"
    );

    Model bush2Model(
        "Assets/Models/Environment/NaturePack/Bush_2.obj"
    );
    Model torchModel(
        "Assets/Models/Environment/Torch/Torch.obj",
        "Assets/Models/Environment/Torch/"
    );
    Model woodenHouseModel(
        "Assets/Models/Environment/WoodenHouse/WoodenHouse.obj",
        "Assets/Models/Environment/WoodenHouse/"
    );
    Model grassClumpModel(
        "Assets/Models/Environment/GrassClump/grass1.obj",
        "Assets/Models/Environment/GrassClump/"
    );

    Model flowerClumpModel(
        "Assets/Models/Environment/GrassClump/Flower.obj",
        "Assets/Models/Environment/GrassClump/"
    );
    //Model mountain1Model(
    //    "Assets/Models/Environment/Mountains/Mountain01.obj",
    //    "Assets/Models/Environment/Mountains/"
    //);

    //Model mountain2Model(
    //    "Assets/Models/Environment/Mountains/Mountain02.obj",
    //    "Assets/Models/Environment/Mountains/"
    //);

    //Model mountain3Model(
    //    "Assets/Models/Environment/Mountains/Mountain03.obj",
    //    "Assets/Models/Environment/Mountains/"
    //);

    Shader skyboxShader(skyboxVertex, skyboxFragment);
    Shader gizmoShader(gizmoVertexShader, gizmoFragmentShader);
    Shader gridShader(gridVertexShader, gridFragmentShader);

    GridRenderer grid;
    // ================= CUBE DATA =================
    float vertices[] = {
        // positions          // normals           // texcoords

        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,
         0.5f,-0.5f,-0.5f, 0,0,-1, 1,0,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
         0.5f, 0.5f,-0.5f, 0,0,-1, 1,1,
        -0.5f, 0.5f,-0.5f, 0,0,-1, 0,1,
        -0.5f,-0.5f,-0.5f, 0,0,-1, 0,0,


        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,
         0.5f,-0.5f, 0.5f, 0,0,1, 1,0,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
         0.5f, 0.5f, 0.5f, 0,0,1, 1,1,
        -0.5f, 0.5f, 0.5f, 0,0,1, 0,1,
        -0.5f,-0.5f, 0.5f, 0,0,1, 0,0,

        -0.5f, 0.5f, 0.5f, -1,0,0, 1,0,
        -0.5f, 0.5f,-0.5f, -1,0,0, 1,1,
        -0.5f,-0.5f,-0.5f, -1,0,0, 0,1,
        -0.5f,-0.5f,-0.5f, -1,0,0, 0,1,
        -0.5f,-0.5f, 0.5f, -1,0,0, 0,0,
        -0.5f, 0.5f, 0.5f, -1,0,0, 1,0,

         0.5f, 0.5f, 0.5f, 1,0,0, 1,0,
         0.5f, 0.5f,-0.5f, 1,0,0, 1,1,
         0.5f,-0.5f,-0.5f, 1,0,0, 0,1,
         0.5f,-0.5f,-0.5f, 1,0,0, 0,1,
         0.5f,-0.5f, 0.5f, 1,0,0, 0,0,
         0.5f, 0.5f, 0.5f, 1,0,0, 1,0,

        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,1,
         0.5f,-0.5f,-0.5f, 0,-1,0, 1,1,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,0,
         0.5f,-0.5f, 0.5f, 0,-1,0, 1,0,
        -0.5f,-0.5f, 0.5f, 0,-1,0, 0,0,
        -0.5f,-0.5f,-0.5f, 0,-1,0, 0,1,

        -0.5f, 0.5f,-0.5f, 0,1,0, 0,1,
         0.5f, 0.5f,-0.5f, 0,1,0, 1,1,
         0.5f, 0.5f, 0.5f, 0,1,0, 1,0,
         0.5f, 0.5f, 0.5f, 0,1,0, 1,0,
        -0.5f, 0.5f, 0.5f, 0,1,0, 0,0,
        -0.5f, 0.5f,-0.5f, 0,1,0, 0,1
    };

    float skyboxVertices[] = {

-1.0f,  1.0f, -1.0f,
-1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f, -1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f, -1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,

-1.0f, -1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f, -1.0f,  1.0f,
-1.0f, -1.0f,  1.0f,

-1.0f,  1.0f, -1.0f,
 1.0f,  1.0f, -1.0f,
 1.0f,  1.0f,  1.0f,
 1.0f,  1.0f,  1.0f,
-1.0f,  1.0f,  1.0f,
-1.0f,  1.0f, -1.0f,

-1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
 1.0f, -1.0f, -1.0f,
 1.0f, -1.0f, -1.0f,
-1.0f, -1.0f,  1.0f,
 1.0f, -1.0f,  1.0f
    };
    unsigned int skyboxVAO, skyboxVBO;

    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), skyboxVertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);

    glBindVertexArray(0);
    Texture containerTexture("container.jpg");
    Texture terrainGrassTexture(
        "textures/terrain/grass.jpg"
    );

    Texture terrainCliffTexture(
        "textures/terrain/cliff.jpg"
    );

    Material cubeMaterial(&containerTexture);
    Material groundMaterial(
        &containerTexture
    );
    Mesh cube(vertices, sizeof(vertices));
    std::vector<float> proceduralTerrainVertices;

    BuildProceduralTerrain(
        proceduralTerrainVertices,
        520.0f,
        220
    );

    Mesh proceduralTerrainMesh(
        proceduralTerrainVertices.data(),
        proceduralTerrainVertices.size() * sizeof(float)
    );
    Material proceduralTerrainMaterial(
        nullptr
    );
    proceduralTerrainMaterial.tint =
        glm::vec3(
            1.0f,
            1.0f,
            1.0f
        );

    proceduralTerrainMaterial.ambient =
        glm::vec3(
            0.45f,
            0.45f,
            0.45f
        );

    proceduralTerrainMaterial.diffuse =
        glm::vec3(
            1.0f,
            1.0f,
            1.0f
        );

    proceduralTerrainMaterial.specular =
        glm::vec3(
            0.05f,
            0.05f,
            0.05f
        );

    proceduralTerrainMaterial.shininess =
        4.0f;
    SceneObject proceduralTerrainObject(
        &proceduralTerrainMesh,
        &shader,
        &proceduralTerrainMaterial
    );

    proceduralTerrainObject.name =
        "Procedural Terrain";
    SetObjectMetadata(
        &proceduralTerrainObject,
        "procedural_terrain",
        AssetType::Terrain,
        SpawnSource::Procedural,
        false,
        true
    );
    proceduralTerrainObject.transform.position =
        glm::vec3(
            0.0f
        );

    proceduralTerrainObject.transform.scale =
        glm::vec3(
            1.0f
        );

    proceduralTerrainObject.boundingRadius =
        800.0f;

    proceduralTerrainObject.isCollider =
        false;

    scene.AddObject(
        &proceduralTerrainObject
    );
    SceneObject cube1(&cube, &shader, &cubeMaterial);
    SceneObject cube2(&cube, &shader, &cubeMaterial);
    SceneObject cube3(&cube, &shader, &cubeMaterial);
    SceneObject ground(
        &cube,
        &shader,
        &groundMaterial
    );
    cube1.name = "Cube 1";
    cube2.name = "Cube 2";
    cube3.name = "Cube 3";
    ground.name = "Ground";
    cube1.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cube2.transform.position = glm::vec3(2.0f, 0.0f, 0.0f);
    cube3.transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    ground.transform.position =
        glm::vec3(
            0.0f,
            -0.55f,
            -10.0f
        );

    ground.transform.scale =
        glm::vec3(
            120.0f,
            0.1f,
            120.0f
        );
    ground.material->tint =
        glm::vec3(
            0.25f,
            0.45f,
            0.20f
        );
    cube2.transform.scale = glm::vec3(1.5f);
    cube3.transform.scale = glm::vec3(0.5f);
    ground.visible = false;
    ground.isCollider = false;
    scene.AddObject( &ground  );
   
    auto AddEnvironmentModel =
        [&](Model* model,
            const std::string& name,
            glm::vec3 position,
            glm::vec3 scale,
            bool collider)
        {
            SceneObject* obj =
                new SceneObject(
                    model,
                    &shader
                );

            obj->name = name;
            obj->assetId =
                name;

            obj->assetType =
                AssetType::Prop;

            obj->spawnSource =
                SpawnSource::Procedural;

            obj->persistent =
                false;

            obj->showInHierarchy =
                true;
            obj->transform.position =
                position;

            obj->transform.scale =
                scale;

            obj->boundingRadius =
                50.0f;

            obj->isCollider =
                collider;
            obj->colliderRadius =
                glm::max(
                    scale.x,
                    scale.z
                ) * 0.4f;
            scene.AddObject(obj);

            return obj;
        };

    float grassTileSpacing =
        8.0f;
    bool useOldGrassTiles =
        false;

    if (useOldGrassTiles)
    {
        for (int x = -10; x <= 10; x++)
        {
            for (int z = -10; z <= 10; z++)
            {
                AddEnvironmentModel(
                    &grassTerrainModel,
                    "Grass Terrain Tile",
                    glm::vec3(
                        x * grassTileSpacing,
                        -0.35f,
                        z * grassTileSpacing
                    ),
                    glm::vec3(
                        0.35f
                    ),
                    false
                );
            }
        }
    }
//// Trees
//    AddEnvironmentModel(
//        &pineTreeModel,
//        "Pine Tree 1",
//        glm::vec3(-8.0f, 0.05f, -12.0f),
//        glm::vec3(0.35f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &pineTreeModel,
//        "Pine Tree 2",
//        glm::vec3(7.0f, 0.0f, -14.0f),
//        glm::vec3(1.0f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &commonTreeModel,
//        "Common Tree 1",
//        glm::vec3(-12.0f, 0.0f, -4.0f),
//        glm::vec3(0.9f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &commonTreeModel,
//        "Common Tree 2",
//        glm::vec3(10.0f, 0.0f, -6.0f),
//        glm::vec3(1.1f),
//        false
//    );
//
//    // Rocks
//    AddEnvironmentModel(
//        &rockModel,
//        "Rock 1",
//        glm::vec3(-4.0f, 0.0f, -8.0f),
//        glm::vec3(1.0f),
//        true
//    );
//
//    AddEnvironmentModel(
//        &rockModel,
//        "Rock 2",
//        glm::vec3(5.0f, 0.0f, -10.0f),
//        glm::vec3(0.7f),
//        true
//    );
//
//    // Bushes
//    AddEnvironmentModel(
//        &bushModel,
//        "Bush 1",
//        glm::vec3(-2.0f, 0.0f, -5.0f),
//        glm::vec3(0.8f),
//        false
//    );
//
//    AddEnvironmentModel(
//        &bushModel,
//        "Bush 2",
//        glm::vec3(3.0f, 0.0f, -7.0f),
//        glm::vec3(0.9f),
//        true
//    );
//
//    // Grass patches
//    AddEnvironmentModel(
//        &grassModel,
//        "Grass Patch 1",
//        glm::vec3(0.0f, 0.0f, -6.0f),
//        glm::vec3(1.0f),
//        false
//    );
//
//    AddEnvironmentModel(
//        &grassModel,
//        "Grass Patch 2",
//        glm::vec3(6.0f, 0.0f, -3.0f),
//        glm::vec3(1.2f),
//        true
//    );
//
//    // Wood log
//    AddEnvironmentModel(
//        &woodLogModel,
//        "Wood Log",
//        glm::vec3(-6.0f, 0.0f, -6.0f),
//        glm::vec3(1.0f),
//        true
//    );
    // ================= EXTRA FOREST DECORATION =================

// Trees
    //AddEnvironmentModel(
    //    &pineTree2Model,
    //    "Pine Tree 3",
    //    glm::vec3(-18.0f, 0.05f, -18.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &pineTree3Model,
    //    "Pine Tree 4",
    //    glm::vec3(18.0f, 0.05f, -22.0f),
    //    glm::vec3(0.38f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &birchTreeModel,
    //    "Birch Tree 1",
    //    glm::vec3(-22.0f, 0.05f, 8.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &birchTree2Model,
    //    "Birch Tree 2",
    //    glm::vec3(22.0f, 0.05f, 10.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &commonTree2Model,
    //    "Common Tree 3",
    //    glm::vec3(0.0f, 0.05f, -25.0f),
    //    glm::vec3(0.40f),
    //    false
    //);

    //// Rocks
    //AddEnvironmentModel(
    //    &rock2Model,
    //    "Rock 3",
    //    glm::vec3(-10.0f, 0.05f, 6.0f),
    //    glm::vec3(0.35f),
    //    true
    //);

    //AddEnvironmentModel(
    //    &rock3Model,
    //    "Rock 4",
    //    glm::vec3(12.0f, 0.05f, -4.0f),
    //    glm::vec3(0.35f),
    //    true
    //);

    //AddEnvironmentModel(
    //    &rockMossModel,
    //    "Moss Rock",
    //    glm::vec3(4.0f, 0.05f, 14.0f),
    //    glm::vec3(0.40f),
    //    true
    //);

    //// Plants / flowers
    //AddEnvironmentModel(
    //    &plantModel,
    //    "Plant 1",
    //    glm::vec3(-5.0f, 0.05f, 10.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &plant2Model,
    //    "Plant 2",
    //    glm::vec3(8.0f, 0.05f, 12.0f),
    //    glm::vec3(0.35f),
    //    false
    //);

    //AddEnvironmentModel(
    //    &flowersModel,
    //    "Flowers",
    //    glm::vec3(2.0f, 0.05f, 8.0f),
    //    glm::vec3(0.45f),
    //    false
    //);

    //// Stump
    //AddEnvironmentModel(
    //    &treeStumpModel,
    //    "Tree Stump",
    //    glm::vec3(-14.0f, 0.05f, -2.0f),
    //    glm::vec3(0.45f),
    //    true
    //);
    // ================= GRASS FIELD =================

   /* for (int x = -8; x <= 8; x++)
    {
        for (int z = -8; z <= 8; z++)
        {
            float worldX =
                x * 4.0f;

            float worldZ =
                z * 4.0f;

            float worldY =
                TerrainGenerator::GetHeight(
                    worldX,
                    worldZ
                );

            AddEnvironmentModel(
                &grassPatchModel,
                "Grass Patch",
                glm::vec3(
                    worldX,
                    worldY + 0.05f,
                    worldZ
                ),
                glm::vec3(
                    0.7f
                ),
                false
            );
        }
    }*/
    // ================= REAL MOUNTAIN BORDER =================

    //std::vector<Model*> mountainModels =
    //{
    //    &mountain1Model,
    //    &mountain2Model,
    //    &mountain3Model
    //};

    //auto AddBorderMountain =
    //    [&](float x, float z, float scale)
    //    {
    //        Model* selectedMountain =
    //            mountainModels[
    //                rand() % mountainModels.size()
    //            ];

    //        SceneObject* mountain =
    //            AddEnvironmentModel(
    //                selectedMountain,
    //                "Border Mountain",
    //                glm::vec3(
    //                    x,
    //                    -2.0f,
    //                    z
    //                ),
    //                glm::vec3(
    //                    scale
    //                ),
    //                false
    //            );

    //        mountain->boundingRadius =
    //            500.0f;

    //        return mountain;
    //    };
    //// Back border mountains
    //for (int i = -4; i <= 4; i++)
    //{
    //    AddBorderMountain(
    //        i * 35.0f,
    //        -120.0f,
    //        8.0f
    //    );
    //}

    //// Left border mountains
    //for (int i = -3; i <= 3; i++)
    //{
    //    AddBorderMountain(
    //        -120.0f,
    //        i * 35.0f,
    //        8.0f
    //    );
    //}

    //// Right border mountains
    //for (int i = -3; i <= 3; i++)
    //{
    //    AddBorderMountain(
    //        120.0f,
    //        i * 35.0f,
    //        8.0f
    //    );
    //}
    // ================= TEST ONE MOUNTAIN =================

   /* SceneObject* testMountain =
        new SceneObject(
            &mountain2Model,
            &shader
        );

    testMountain->name =
        "Test Mountain";

    testMountain->transform.position =
        glm::vec3(
            0.0f,
            -2.0f,
            -80.0f
        );

    testMountain->transform.scale =
        glm::vec3(
            0.02f
        );

    testMountain->boundingRadius =
        500.0f;

    testMountain->isCollider =
        false;*/

  /*  scene.AddObject(
        testMountain
    );*/

    // ================= FOREST GENERATOR V1 =================
    // ================= WORLD BORDER MOUNTAINS =================

    srand(7);

    auto RandomRange =
        [](float minValue, float maxValue)
        {
            float t =
                (float)(rand() % 1000) / 1000.0f;

            return minValue +
                t * (maxValue - minValue);
        };

    std::vector<Model*> treeModels =
    {
        &pineTreeModel,
        &pineTree2Model,
        &pineTree3Model,
        &pineTree4Model,
        &pineTree5Model,

        &commonTreeModel,
        &commonTree2Model,
        &commonTree3Model,
        &commonTree4Model,

        &birchTreeModel,
        &birchTree2Model,
        &birchTree3Model,

        &willowTreeModel,
        &willowTree2Model
    };

    std::vector<Model*> rockModels =
    {
        &rockModel,
        &rock2Model,
        &rock3Model,
        &rock4Model,
        &rock5Model,

        &rockMossModel,
        &rockMoss2Model,
        &rockMoss3Model
    };

    std::vector<Model*> plantModels =
    {
        &bushModel,
        &bush2Model,
        &bushBerriesModel,

        &plantModel,
        &plant2Model,

        &flowersModel,
        &grassModel,
        &grass2Model,
        &wheatModel
    };
    // ================= SIMPLE LOW-POLY GRASS FIELD =================

    for (int i = 0; i < 160; i++)
    {
        float x =
            RandomRange(
                -240.0f,
                240.0f
            );

        float z =
            RandomRange(
                -240.0f,
                240.0f
            );

        if (!IsGoodTerrainSpawnPoint(x, z))
            continue;

        float y =
            GetObjectTerrainY(
                x,
                z,
                0.02f
            );

        SceneObject* grassObject =
            AddEnvironmentModel(
                &grassClumpModel,
                "Grass Clump",
                glm::vec3(
                    x,
                    y,
                    z
                ),
                glm::vec3(
                    RandomRange(
                        0.8f,
                        1.3f
                    )
                ),
                false
            );

        grassObject->transform.rotation =
            glm::vec3(
                0.0f,
                RandomRange(
                    0.0f,
                    360.0f
                ),
                0.0f
            );

        grassObject->boundingRadius =
            8.0f;
        SetObjectMetadata(
            grassObject,
            "grass_clump",
            AssetType::Grass,
            SpawnSource::Procedural,
            false,
            false
        );
    }

    // ================= SIMPLE FLOWER PATCHES =================

    for (int i = 0; i < 70; i++)
    {
        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
            RandomRange(
                -220.0f,
                220.0f
            );

        if (!IsGoodTerrainSpawnPoint(x, z))
            continue;

        float y =
            GetObjectTerrainY(
                x,
                z,
                0.03f
            );

        SceneObject* flowerObject =
            AddEnvironmentModel(
                &flowerClumpModel,
                "Flower Clump",
                glm::vec3(
                    x,
                    y,
                    z
                ),
                glm::vec3(
                    RandomRange(
                        1.0f,
                        1.6f
                    )
                ),
                false
            );

        flowerObject->transform.rotation =
            glm::vec3(
                0.0f,
                RandomRange(
                    0.0f,
                    360.0f
                ),
                0.0f
            );

        flowerObject->boundingRadius =
            8.0f;
        SetObjectMetadata(
            flowerObject,
            "flower_clump",
            AssetType::Flower,
            SpawnSource::Procedural,
            false,
            false
        );
    }
    // ================= PROCEDURAL GRASS AND FLOWERS =================

    // Trees
    for (int i = 0; i < 60; i++)
    {
        Model* chosenTree =
            treeModels[
                rand() % treeModels.size()
            ];

        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
            RandomRange(
                -220.0f,
                220.0f
            );

        float scale =
            RandomRange(
                1.35f,
                2.10f
            );
        SceneObject* treeObject =
            AddEnvironmentModel(
                chosenTree,
                "Generated Tree",
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                false
            );

        SetObjectMetadata(
            treeObject,
            "generated_tree",
            AssetType::Tree,
            SpawnSource::Procedural,
            false,
            true
        );
    }

    // Rocks
    for (int i = 0; i < 40; i++)
    {
        Model* chosenRock =
            rockModels[
                rand() % rockModels.size()
            ];

        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
            RandomRange(
                -220.0f,
                220.0f
            );

        float scale =
            RandomRange(
                1.20f,
                2.00f
            );

        SceneObject* rockObject =
            AddEnvironmentModel(
                chosenRock,
                "Generated Rock",
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                true
            );

        SetObjectMetadata(
            rockObject,
            "generated_rock",
            AssetType::Rock,
            SpawnSource::Procedural,
            false,
            true
        );
    }

    // Bushes / plants / flowers
    for (int i = 0; i < 100; i++)
    {
        Model* chosenPlant =
            plantModels[
                rand() % plantModels.size()
            ];

        float x =
            RandomRange(
                -220.0f,
                220.0f
            );

        float z =
          RandomRange(
    -220.0f,
    220.0f
);

        float scale =
            RandomRange(
                0.50f,
                0.85f
            );

        SceneObject* plantObject =
            AddEnvironmentModel(
                chosenPlant,
                "Generated Plant",
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                false
            );

        SetObjectMetadata(
            plantObject,
            "generated_plant",
            AssetType::Bush,
            SpawnSource::Procedural,
            false,
            false
        );
    }

    // Logs / stumps
    for (int i = 0; i < 20; i++)
    {
        Model* chosenObject =
            (rand() % 2 == 0)
            ? &woodLogModel
            : &treeStumpModel;

        float x =
            RandomRange(
                -200.0f,
                200.0f
            );

        float z =
            RandomRange(
                -200.0f,
                200.0f
            );

        float scale =
            RandomRange(
                0.80f,
                1.25f
            );

        AddEnvironmentModel(
            chosenObject,
            "Generated Log/Stump",
            glm::vec3(
                x,
                GetTerrainHeight(x, z) + 0.05f,
                z
            ),
            glm::vec3(
                scale
            ),
            true
        );
    }
    // ================= FOREST BORDER WALL =================

    auto AddBorderTree =
        [&](float x, float z)
        {
            Model* chosenTree =
                treeModels[
                    rand() % treeModels.size()
                ];

            float scale =
                RandomRange(
                    1.1f,
                    1.7f
                );

            AddEnvironmentModel(
                chosenTree,
                "Border Tree",
                glm::vec3(
                    x,
                    GetTerrainHeight(x, z) + 0.05f,
                    z
                ),
                glm::vec3(
                    scale
                ),
                false
            );
        };

    // Back forest border
    for (int i = -14; i <= 14; i++)
    {
        AddBorderTree(
            i * 7.0f,
            -85.0f + RandomRange(-3.0f, 3.0f)
        );
    }

    // Left forest border
    for (int i = -12; i <= 12; i++)
    {
        AddBorderTree(
            -85.0f + RandomRange(-3.0f, 3.0f),
            i * 7.0f
        );
    }

    // Right forest border
    for (int i = -12; i <= 12; i++)
    {
        AddBorderTree(
            85.0f + RandomRange(-3.0f, 3.0f),
            i * 7.0f
        );
    }
    // ================= EXTRA ROCK CLUSTERS =================

    for (int cluster = 0; cluster < 12; cluster++)
    {
        float centerX =
            RandomRange(
                -70.0f,
                70.0f
            );

        float centerZ =
            RandomRange(
                -70.0f,
                70.0f
            );

        int rocksInCluster =
            3 + rand() % 4;

        for (int i = 0; i < rocksInCluster; i++)
        {
            Model* chosenRock =
                rockModels[
                    rand() % rockModels.size()
                ];

            float rockX =
                centerX + RandomRange(-3.0f, 3.0f);

            float rockZ =
                centerZ + RandomRange(-3.0f, 3.0f);

            AddEnvironmentModel(
                chosenRock,
                "Rock Cluster",
                glm::vec3(
                    rockX,
                    GetTerrainHeight(rockX, rockZ) + 0.05f,
                    rockZ
                ),
                glm::vec3(
                    RandomRange(0.6f, 1.2f)
                ),
                true
            );
        }
    }
    playerObject =
        new SceneObject(
            &myModel,
            &shader
        );
    playerObject->name = "Player";
    SetObjectMetadata(
        playerObject,
        "player",
        AssetType::Player,
        SpawnSource::Manual,
        true,
        true
    );
    playerObject->transform.position =
        playerSpawnPosition;
    playerObject->transform.position.y =
        GetPlayerTerrainY(
            playerObject->transform.position.x,
            playerObject->transform.position.z
        );
    playerSpawnPosition =
        playerObject->transform.position;
    playerObject->transform.scale =
        glm::vec3(
            0.6f
        );

    playerObject->boundingRadius = 3.0f;

    scene.AddObject(playerObject);
    /*SceneObject* environmentObject =
        new SceneObject(
            &forestEnvironment,
            &shader
        );

    environmentObject->name =
        "Forest Cabin Environment";

    environmentObject->transform.position =
        glm::vec3(
            0.0f,
            0.0f,
            -10.0f
        );

    environmentObject->transform.scale =
        glm::vec3(
            1.0f
        );

    environmentObject->boundingRadius =
        300.0f;
    environmentObject->isCollider = false;
    scene.AddObject(
        environmentObject
    );*/
    
    /*ground.boundingRadius = 100.0f;
    SceneObject* treeObject =
        new SceneObject(
            &importedTree,
            &shader
        );

    treeObject->name = "Imported Tree";
    treeObject->boundingRadius = 20.0f;

    treeObject->transform.position =
        glm::vec3(
            0.0f,
            0.0f,
            -6.0f
        );

    treeObject->transform.scale =
        glm::vec3(
            0.1f
        );

    scene.AddObject(treeObject);*/
   /* std::vector<SceneObject*> manyCubes;

    for (int i = 0; i < 100; i++)
    {
        SceneObject* obj = new SceneObject(&cube, &shader, &cubeMaterial);
        obj->name = "Cube_" + std::to_string(i);
        float x = (rand() % 50 - 25);
        float y = (rand() % 10 - 5);
        float z = -(rand() % 50);

        obj->transform.position = glm::vec3(x, y, z);

        scene.AddObject(obj);
        manyCubes.push_back(obj);
    }*/

    /*glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);*/

    // ================= TEXTURE =================

    unsigned int texture;
    //glGenTextures(1, &texture);
   //glBindTexture(GL_TEXTURE_2D, texture);
   //Texture containerTexture("D:\\taki\\POLAND\\POLAND\\ThesisRenderer\\ThesisRenderer\\container.jpg");
      /*  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

    int width, height, nrChannels;
    std::cout << "==== CONTROLS ====\n";
    std::cout << "WASD - Move camera\n";
    std::cout << "Mouse - Look around\n";
    std::cout << "Left Click - Select object\n";
    std::cout << "N - Add cube\n";
    std::cout << "M - Delete object\n";
    std::cout << "Arrows/PageUp/PageDown - Move object\n";
    std::cout << "IJKLUO - Rotate object\n";
    std::cout << "+/- - Scale object\n";
    std::cout << "C - Toggle frustum culling\n";
    std::cout << "===================\n";
    // unsigned char* data = stbi_load("D:\\taki\\POLAND\\POLAND\\ThesisRenderer\\ThesisRenderer\\container.jpg", &width, &height, &nrChannels, 0);
    Shader lightShader(lightVertexSource, lightFragmentSource);
    shader.setVec3("lightPos", glm::vec3(1.2f, 1.0f, 2.0f));
    shader.setVec3("viewPos", camera.Position);
    shader.setVec3("lightColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setVec3("objectColor", glm::vec3(1.0f, 1.0f, 1.0f));
    shader.setBool("useTexture", true);
    glm::vec3 cubePositions[] = {
    glm::vec3(0.0f,  0.0f,  0.0f),
    glm::vec3(2.0f,  5.0f, -15.0f),
    glm::vec3(-1.5f, -2.2f, -2.5f),
    glm::vec3(-3.8f, -2.0f, -12.3f),
    glm::vec3(2.4f, -0.4f, -3.5f),
    glm::vec3(-1.7f,  3.0f, -7.5f),
    glm::vec3(1.3f, -2.0f, -2.5f),
    glm::vec3(1.5f,  2.0f, -2.5f),
    glm::vec3(1.5f,  0.2f, -1.5f),
    glm::vec3(-1.3f,  1.0f, -1.5f)
    };


    // ================= RENDER LOOP =================

    cube1.AddChild(&cube2);
    cube2.AddChild(&cube3);
    scene.objects[0]->AddChild(scene.objects[1]);
    // ===== ATTACH COMPONENTS =====
    cube1.AddComponent(new RotatorComponent(glm::vec3(0.0f, 1.0f, 0.0f), 50.0f));
    cube2.AddComponent(new RotatorComponent(glm::vec3(1.0f, 0.0f, 0.0f), 30.0f));
    cube3.AddComponent(new OscillatorComponent(0.5f, 2.0f));

    AppMode previousAppMode = appMode;
    while (!glfwWindowShouldClose(window))
    {
        for (SceneObject* obj : scene.objects)
        {
            obj->UpdateComponents(deltaTime);
        }
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
       ImGuiIO& debugIO = ImGui::GetIO();
       // resseting plaaayer to the start .............................
   /*     if (
            previousAppMode == AppMode::Editor &&
            appMode == AppMode::Play
            )
        {
            if (playerObject != nullptr)
            {
                playerObject->transform.position =
                    playerSpawnPosition;

                playerObject->transform.rotation =
                    glm::vec3(0.0f);

                thirdPersonController.verticalVelocity =
                    0.0f;

                thirdPersonController.isGrounded =
                    true;
            }
        }*/

        previousAppMode =
            appMode;
       
        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;
        if (!editorCameraStartFixed)
        {
            camera.Position =
                glm::vec3(
                    0.0f,
                    25.0f,
                    45.0f
                );

            editorCameraStartFixed =
                true;
        }
        float cameraSpeed =
    5.0f *
    deltaTime;

if (
    appMode == AppMode::Editor &&
    glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS
)
{
    cameraSpeed =
        15.0f *
        deltaTime;
}
blockEditorMouseLook =
appMode == AppMode::Play;
        glfwPollEvents();



        ImGuiIO& io = ImGui::GetIO();

      
        bool rightMouseDown =
            glfwGetMouseButton(
                window,
                GLFW_MOUSE_BUTTON_RIGHT
            ) == GLFW_PRESS;

        bool canControlCamera =
            rightMouseDown &&
            !io.WantCaptureMouse;

        if (
            canControlCamera &&
            !rightMouseCameraActive
            )
        {
            double mouseX;
            double mouseY;

            glfwGetCursorPos(
                window,
                &mouseX,
                &mouseY
            );

            lastX =
                static_cast<float>(mouseX);

            lastY =
                static_cast<float>(mouseY);

            firstMouse =
                true;

            ignoreNextMouseDelta =
                true;

            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_DISABLED
            );

            rightMouseCameraActive =
                true;
        }
        else if (
            !canControlCamera &&
            rightMouseCameraActive
            )
        {
            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_NORMAL
            );

            rightMouseCameraActive =
                false;

            firstMouse =
                true;

            ignoreNextMouseDelta =
                true;
        }
        bool nKeyCurrent =
            glfwGetKey(
                window,
                GLFW_KEY_N
            ) == GLFW_PRESS;

        nKeyLastState = nKeyCurrent;

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_PRESS && !mKeyPressed)
        {
            if (selectedObject != nullptr)
            {
                for (auto it = scene.objects.begin(); it != scene.objects.end(); ++it)
                {
                    if (*it == selectedObject)
                    {
                        SceneObject* toDelete = *it;
                        scene.objects.erase(it);
                        delete toDelete;
                        selectedObject = nullptr;
                        break;
                    }
                }

                selectedObject = nullptr;

                std::cout << "Selected object deleted!\n";
            }

            mKeyPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        {
            mKeyPressed = false;
        }
        if (
            appMode == AppMode::Editor &&
            selectedObject != nullptr
            )
        {
            float speed = 5.0f * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)
                selectedObject->transform.position.z -= speed;

            if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)
                selectedObject->transform.position.z += speed;

            if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS)
                selectedObject->transform.position.x -= speed;

            if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS)
                selectedObject->transform.position.x += speed;


            if (glfwGetKey(window, GLFW_KEY_PAGE_UP) == GLFW_PRESS)
                selectedObject->transform.position.y += speed;

            if (glfwGetKey(window, GLFW_KEY_PAGE_DOWN) == GLFW_PRESS)
                selectedObject->transform.position.y -= speed;
        }
        
        if (glfwGetKey(window, GLFW_KEY_M) == GLFW_RELEASE)
        {
            mKeyPressed = false;
        }
        // ImGuiIO& io = ImGui::GetIO();

        if (appMode == AppMode::Editor)
        {
            if (!io.WantCaptureKeyboard)
            {
                if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
                    camera.Position += cameraSpeed * camera.Front;

                if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                    camera.Position -= cameraSpeed * camera.Front;

                if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                    camera.Position -=
                    glm::normalize(
                        glm::cross(
                            camera.Front,
                            camera.Up
                        )
                    ) * cameraSpeed;

                if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                    camera.Position +=
                    glm::normalize(
                        glm::cross(
                            camera.Front,
                            camera.Up
                        )
                    ) * cameraSpeed;
            }
        }
        else
        {
            thirdPersonController.Update(
                window,
                playerObject,
                camera,
                scene,
                deltaTime
            );
        }
       
        // ================= TERRAIN PLAYER COLLISION =================

        if (appMode == AppMode::Play && playerObject != nullptr)
        {
            float targetY =
                GetPlayerTerrainY(
                    playerObject->transform.position.x,
                    playerObject->transform.position.z
                );

            bool jumpHeld =
                glfwGetKey(
                    window,
                    GLFW_KEY_SPACE
                ) == GLFW_PRESS;

            bool movingUp =
                thirdPersonController.verticalVelocity > 0.01f;

            bool nearGround =
                playerObject->transform.position.y <=
                targetY + 0.65f;

            if (movingUp)
            {
                thirdPersonController.isGrounded =
                    false;
            }

            if (
                !jumpHeld &&
                thirdPersonController.isGrounded
                )
            {
                playerObject->transform.position.y =
                    targetY;

                thirdPersonController.verticalVelocity =
                    0.0f;

                thirdPersonController.isGrounded =
                    true;
            }
            else if (
                !movingUp &&
                nearGround
                )
            {
                playerObject->transform.position.y =
                    targetY;

                thirdPersonController.verticalVelocity =
                    0.0f;

                thirdPersonController.isGrounded =
                    true;
            }
        }
        if (appMode == AppMode::Play && playerObject != nullptr)
        {
            UpdatePlayModeCameraFollow(
                camera,
                playerObject,
                deltaTime
            );
        }
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            rotationAxis = glm::vec3(1.0f, 0.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            rotationAxis = glm::vec3(0.0f, 1.0f, 0.0f);

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            rotationAxis = glm::vec3(0.0f, 0.0f, 1.0f);
        if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
            currentAxis = AXIS_X;


        if (glfwGetKey(window, GLFW_KEY_T) == GLFW_PRESS)
        {
            currentGizmoMode = TRANSLATE;
            std::cout << "Translate Mode\n";
        }

        if (glfwGetKey(window, GLFW_KEY_R) == GLFW_PRESS)
        {
            currentGizmoMode = ROTATE;
            std::cout << "Rotate Mode\n";
        }

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        {
            currentGizmoMode = SCALE;
            std::cout << "Scale Mode\n";
        }

        if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
            currentAxis = AXIS_Y;

        if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
            currentAxis = AXIS_Z;

        if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
            currentAxis = NONE;
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_PRESS && !cKeyPressed)
        {
            useCulling = !useCulling;
            cKeyPressed = true;



            if (useCulling)
                std::cout << "Culling ON\n";
            else
                std::cout << "Culling OFF\n";
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS && !pPressed)
        {
            SceneSerializer::Save(scene, "D:\\taki\\POLAND\\POLAND\\ThesisRenderer\\ThesisRenderer\\scene.txt");
            pPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_P) == GLFW_RELEASE)
        {
            pPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS && !lPressed)
        {
            SceneSerializer::Load(scene, "D:\\taki\\POLAND\\POLAND\\ThesisRenderer\\ThesisRenderer\\scene.txt", &cube, &shader, &cubeMaterial);
            lPressed = true;
        }
        if (glfwGetKey(window, GLFW_KEY_L) == GLFW_RELEASE)
        {
            lPressed = false;
        }
        static bool gPressed = false;

        /* if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed)
         {
             useGridSnap = !useGridSnap;
             gPressed = true;

             std::cout << "Grid Snap: " << (useGridSnap ? "ON\n" : "OFF\n");
         }

         if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
         {
             gPressed = false;
         }*/

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_PRESS && !bPressed)
        {
            if (selectedObject != nullptr)
            {
                SceneObject* copy = new SceneObject(
                    selectedObject->mesh,
                    selectedObject->shader,
                    selectedObject->material
                );


                copy->transform = selectedObject->transform;


                copy->transform.position += glm::vec3(1.0f, 0.0f, 0.0f);

                scene.AddObject(copy);


                selectedObject->isSelected = false;
                selectedObject = copy;
                selectedObject->isSelected = true;

                std::cout << "Object duplicated!\n";
            }

            bPressed = true;
        }

        if (glfwGetKey(window, GLFW_KEY_B) == GLFW_RELEASE)
        {
            bPressed = false;
        }
        // ===== ROTate
        if (
            appMode == AppMode::Editor &&
            selectedObject != nullptr
            )
        {
            float rotSpeed = 50.0f * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_I) == GLFW_PRESS)
                selectedObject->transform.rotation.x += rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_K) == GLFW_PRESS)
                selectedObject->transform.rotation.x -= rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)
                selectedObject->transform.rotation.y += rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_L) == GLFW_PRESS)
                selectedObject->transform.rotation.y -= rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)
                selectedObject->transform.rotation.z += rotSpeed;

            if (glfwGetKey(window, GLFW_KEY_O) == GLFW_PRESS)
                selectedObject->transform.rotation.z -= rotSpeed;
            if (snapEnabled)
            {
                glm::vec3& pos = selectedObject->transform.position;

                pos.x = round(pos.x / gridSize) * gridSize;
                pos.y = round(pos.y / gridSize) * gridSize;
                pos.z = round(pos.z / gridSize) * gridSize;
            }
        }
        //if (glfwGetKey(window, GLFW_KEY_X) == GLFW_PRESS)
        //    currentAxis = X_AXIS;

        //if (glfwGetKey(window, GLFW_KEY_Y) == GLFW_PRESS)
        //    currentAxis = Y_AXIS;

        //if (glfwGetKey(window, GLFW_KEY_Z) == GLFW_PRESS)
        //    currentAxis = Z_AXIS;

        //if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
        //    currentAxis = FREE;// reset
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_PRESS && !gPressed)
        {
            snapEnabled = !snapEnabled;
            gPressed = true;

            if (snapEnabled)
                std::cout << "Grid Snapping ON\n";
            else
                std::cout << "Grid Snapping OFF\n";
        }
       
        if (glfwGetKey(window, GLFW_KEY_G) == GLFW_RELEASE)
        {
            gPressed = false;
        }
        if (glfwGetKey(window, GLFW_KEY_1) == GLFW_PRESS)
        {
            currentAxis = AXIS_X;
            std::cout << "Move X axis\n";
        }

        if (glfwGetKey(window, GLFW_KEY_2) == GLFW_PRESS)
        {
            currentAxis = AXIS_Z;
            std::cout << "Move Z axis\n";
        }
        if (glfwGetKey(window, GLFW_KEY_0) == GLFW_PRESS)
        {
            currentAxis = NONE;
            std::cout << "Free move\n";
        }

        // ===== SCALE
        if (
            appMode == AppMode::Editor &&
            selectedObject != nullptr
            )
        {
            float scaleSpeed = 2.0f * deltaTime;

            if (glfwGetKey(window, GLFW_KEY_EQUAL) == GLFW_PRESS) // +
                selectedObject->transform.scale += glm::vec3(scaleSpeed);

            if (glfwGetKey(window, GLFW_KEY_MINUS) == GLFW_PRESS) // -
                selectedObject->transform.scale -= glm::vec3(scaleSpeed);
            selectedObject->transform.scale =
                glm::max(
                    selectedObject->transform.scale,
                    glm::vec3(0.1f)
                );
        }

        //selectedObject->transform.scale = glm::max(selectedObject->transform.scale, glm::vec3(0.1f));
        if (glfwGetKey(window, GLFW_KEY_C) == GLFW_RELEASE)
        {
            cKeyPressed = false;
        }
        int width, height;

        glfwGetFramebufferSize(
            window,
            &width,
            &height
        );

        glViewport(
            0,
            0,
            width,
            height
        );
        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f),
            (float)glfwGetTime(),
            glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 view = camera.GetViewMatrix();
        glm::mat4 projection = glm::perspective(
            glm::radians(45.0f),
            (float)width / (float)height,
            0.1f,
            900.0f
        );

        glDepthFunc(GL_LEQUAL);
        glDepthMask(GL_FALSE);
        gridShader.use();

        gridShader.setMat4("view", glm::value_ptr(view));
        gridShader.setMat4("projection", glm::value_ptr(projection));

        grid.Draw(gridShader);
        skyboxShader.use();
        skyboxShader.setInt("skybox", 0);

        glm::mat4 skyboxView = glm::mat4(glm::mat3(camera.GetViewMatrix()));
        skyboxShader.setMat4("view", glm::value_ptr(skyboxView));
        skyboxShader.setMat4("projection", glm::value_ptr(projection));
        glm::mat4 vp = projection * view;
        frustum.Update(vp);
        glBindVertexArray(skyboxVAO);
        glActiveTexture(GL_TEXTURE0);
        skybox.Bind();
        glDrawArrays(GL_TRIANGLES, 0, 36);

        glDepthMask(GL_TRUE);
        glDepthFunc(GL_LESS);
        if (
            appMode == AppMode::Editor &&
            mouseClicked &&
            !ImGui::GetIO().WantCaptureMouse
            )
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            glm::vec3 rayDir = GetRayFromMouse(
                mouseX,
                mouseY,
                width,
                height,
                projection,
                view
            );
            glm::vec3 rayOrigin = camera.Position;

            float closestDist = 1000.0f;

            SceneObject* hitObject = nullptr;

            for (SceneObject* obj : scene.objects)
            {
                float radius = obj->boundingRadius;
                glm::vec3 oc = rayOrigin - obj->transform.position;

                float a = glm::dot(rayDir, rayDir);
                float b = 2.0f * glm::dot(oc, rayDir);
                float c = glm::dot(oc, oc) - radius * radius;

                float discriminant = b * b - 4 * a * c;

                if (discriminant > 0)
                {
                    float dist = (-b - sqrt(discriminant)) / (2.0f * a);

                    if (dist < closestDist && dist > 0)
                    {
                        closestDist = dist;
                        hitObject = obj;
                    }
                }
            }

            if (hitObject != nullptr)
            {
                if (selectedObject != nullptr)
                    selectedObject->isSelected = false;

                selectedObject = hitObject;
                if (selectedObject != nullptr)
                    selectedObject->isSelected = true;
                std::cout << "Object selected!\n";
            }

            mouseClicked = false;
        }
        if (
            appMode == AppMode::Editor &&
            isDragging &&
            selectedObject != nullptr
            )
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // ================= TRANSLATE MODE =================
            if (currentGizmoMode == TRANSLATE)
            {
                glm::vec3 rayDir = GetRayFromMouse(
                    mouseX,
                    mouseY,
                    width,
                    height,
                    projection,
                    view
                );
                glm::vec3 rayOrigin = camera.Position;

                float planeY = selectedObject->transform.position.y;

                float t = (planeY - rayOrigin.y) / rayDir.y;

                if (t > 0.0f)
                {
                    glm::vec3 hitPoint = rayOrigin + rayDir * t;

                    if (currentAxis == AXIS_X)
                        selectedObject->transform.position.x = hitPoint.x;

                    else if (currentAxis == AXIS_Z)
                        selectedObject->transform.position.z = hitPoint.z;

                    else if (currentAxis == AXIS_Y)
                        selectedObject->transform.position.y = hitPoint.y;

                    else
                    {
                        selectedObject->transform.position.x = hitPoint.x;
                        selectedObject->transform.position.z = hitPoint.z;
                    }
                }
            }


            else if (currentGizmoMode == ROTATE)
            {
                float rotationSpeed = 0.2f;

                selectedObject->transform.rotation.y +=
                    (float)(mouseX - lastX) * rotationSpeed;
            }


            else if (currentGizmoMode == SCALE)
            {
                float scaleSpeed = 0.01f;

                float scaleAmount =
                    (float)(mouseX - lastX) * scaleSpeed;

                selectedObject->transform.scale +=
                    glm::vec3(scaleAmount);

                selectedObject->transform.scale =
                    glm::max(
                        selectedObject->transform.scale,
                        glm::vec3(0.1f)
                    );
            }
        }

        shader.use();
        shader.setInt(
    "texture1",
    0
);

shader.setInt(
    "terrainGrassTex",
    1
);

shader.setInt(
    "terrainCliffTex",
    2
);
        shader.setBool(
            "isProceduralTerrain",
            false
        );
        shader.setVec3(
            "sunDirection",
            glm::normalize(
                glm::vec3(
                    -0.55f,
                    -1.0f,
                    -0.35f
                )
            )
        );
        shader.setVec3(
            "sunColor",
            glm::vec3(
                1.10f,
                1.05f,
                0.95f
            )
        );
        int pointLightIndex =
            0;

        for (Light* light : scene.lights)
        {
            if (light == nullptr)
                continue;

            if (light->type == LightType::Directional)
                continue;

            if (pointLightIndex >= 5)
                break;

            shader.setVec3(
                "lightPositions[" + std::to_string(pointLightIndex) + "]",
                light->position
            );

            shader.setVec3(
                "lightColors[" + std::to_string(pointLightIndex) + "]",
                glm::vec3(
                    1.4f,
                    1.1f,
                    0.65f
                )
            );

            pointLightIndex++;
        }

        for (int i = pointLightIndex; i < 5; i++)
        {
            shader.setVec3(
                "lightPositions[" + std::to_string(i) + "]",
                glm::vec3(
                    0.0f,
                    -1000.0f,
                    0.0f
                )
            );

            shader.setVec3(
                "lightColors[" + std::to_string(i) + "]",
                glm::vec3(
                    0.0f
                )
            );
        }
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setVec3("viewPos", camera.Position);
        shader.setInt("texture1", 0);

        // ===== DRAW CUBES (container.jpg) =====
        glActiveTexture(GL_TEXTURE0);
        containerTexture.Bind();

        // Update all components (rotation, oscillation, etc.)
        cube1.transform.position = glm::vec3(0.0f, 0.0f, -3.0f);
        cube1.UpdateComponents(deltaTime);
        cube1.Draw(renderer, glm::mat4(1.0f));
       
        int totalObjects = scene.objects.size();
        int visibleObjects = 0;
        int culledObjects = 0;

        for (SceneObject* obj : scene.objects)
        {
            obj->UpdateComponents(deltaTime);

            // Highlight selected object
            if (obj == selectedObject)
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
                glLineWidth(3.0f);
            }
            else
            {
                glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
            }
            if (
                obj->useModel ||
                !useCulling ||
                frustum.IsSphereVisible(
                    obj->transform.position,
                    obj->boundingRadius
                )
                )
            {
                visibleObjects++;

                bool objectIsProceduralTerrain =
                    obj->name == "Procedural Terrain";

                shader.setBool(
                    "isProceduralTerrain",
                    objectIsProceduralTerrain
                );

                if (objectIsProceduralTerrain)
                {
                    glActiveTexture(GL_TEXTURE1);
                    terrainGrassTexture.Bind();

                    glActiveTexture(GL_TEXTURE2);
                    terrainCliffTexture.Bind();

                    shader.setBool(
                        "useTexture",
                        false
                    );

                    shader.setVec3(
                        "materialAmbient",
                        obj->material->ambient
                    );

                    shader.setVec3(
                        "materialDiffuse",
                        obj->material->diffuse
                    );

                    shader.setVec3(
                        "materialSpecular",
                        obj->material->specular
                    );

                    shader.setVec3(
                        "materialTint",
                        obj->material->tint
                    );

                    shader.setFloat(
                        "materialShininess",
                        obj->material->shininess
                    );

                    glPolygonMode(
                        GL_FRONT_AND_BACK,
                        GL_FILL
                    );
                }
                else if (obj->material != nullptr)
                {
                    bool objectHasTexture =
                        obj->material->texture != nullptr;

                    shader.setBool(
                        "useTexture",
                        objectHasTexture
                    );

                    if (objectHasTexture)
                    {
                        glActiveTexture(GL_TEXTURE0);

                        obj->material->texture->Bind();
                    }

                    shader.setVec3("materialAmbient", obj->material->ambient);
                    shader.setVec3("materialDiffuse", obj->material->diffuse);
                    shader.setVec3("materialSpecular", obj->material->specular);
                    shader.setVec3("materialTint", obj->material->tint);
                    shader.setFloat("materialShininess", obj->material->shininess);
                    if (obj->material->wireframe)
                    {
                        glPolygonMode(
                            GL_FRONT_AND_BACK,
                            GL_LINE
                        );
                    }
                    else
                    {
                        glPolygonMode(
                            GL_FRONT_AND_BACK,
                            GL_FILL
                        );
                    }
                }
                else
                {
                    shader.setBool(
                        "isProceduralTerrain",
                        false
                    );

                    shader.setBool(
                        "useTexture",
                        obj->useModel
                    );

                    glPolygonMode(
                        GL_FRONT_AND_BACK,
                        GL_FILL
                    );

                    shader.setVec3(
                        "materialAmbient",
                        glm::vec3(
                            0.65f,
                            0.65f,
                            0.65f
                        )
                    );

                    shader.setVec3(
                        "materialDiffuse",
                        glm::vec3(
                            1.0f,
                            1.0f,
                            1.0f
                        )
                    );

                    shader.setVec3(
                        "materialSpecular",
                        glm::vec3(
                            0.03f,
                            0.03f,
                            0.03f
                        )
                    );

                    shader.setVec3(
                        "materialTint",
                        glm::vec3(
                            1.0f,
                            1.0f,
                            1.0f
                        )
                    );

                    shader.setFloat(
                        "materialShininess",
                        4.0f
                    );
                }
                obj->Draw(renderer, glm::mat4(1.0f));
       
            }
            else
            {
                culledObjects++;
            }
        }

        // ===== DRAW MODELS (each binds its own texture) =====
  /*      glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-3.0f, 2.0f, -3.0f));
        shader.setMat4("model", glm::value_ptr(model1));
        myModel.Draw(shader);*/

        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(3.0f, 0.0f, -5.0f));
        shader.setMat4("model", glm::value_ptr(model2));
        treeModel.Draw(shader);

        // ===== DRAW LIGHT CUBE =====
        lightShader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, glm::vec3(2.0f, 2.0f, 2.0f));
        lightModel = glm::scale(lightModel, glm::vec3(0.1f));
        lightShader.setMat4("model", glm::value_ptr(lightModel));
        lightShader.setMat4("view", glm::value_ptr(view));
        lightShader.setMat4("projection", glm::value_ptr(projection));
        renderer.DrawMesh(cube, lightShader, lightModel);

        double currentTime = glfwGetTime();
        frameCount++;
        if (currentTime - previousTime >= 1.0)
        {
            double fps = frameCount / (currentTime - previousTime);
            std::string title = "FPS: " + std::to_string((int)fps) +
                " | Visible: " + std::to_string(visibleObjects) +
                " | Culled: " + std::to_string(culledObjects) +
                " | Total: " + std::to_string(totalObjects);
            if (selectedObject != nullptr)
            {
                title += " | Selected Pos: (" +
                    std::to_string(selectedObject->transform.position.x) + "," +
                    std::to_string(selectedObject->transform.position.y) + "," +
                    std::to_string(selectedObject->transform.position.z) + ")";
            }
            else
            {
                title += " | No Object Selected";
            }
            if (selectedObject != nullptr)
            {
                glm::vec3 p = selectedObject->transform.position;

                title += " | Selected Pos: (" +
                    std::to_string((int)p.x) + "," +
                    std::to_string((int)p.y) + "," +
                    std::to_string((int)p.z) + ")";
            }
            if (selectedObject != nullptr)
            {
                glm::vec3 p = selectedObject->transform.position;
                glm::vec3 r = selectedObject->transform.rotation;
                glm::vec3 s = selectedObject->transform.scale;

                title += " | Pos(" +
                    std::to_string((int)p.x) + "," +
                    std::to_string((int)p.y) + "," +
                    std::to_string((int)p.z) + ")";

                title += " Rot(" +
                    std::to_string((int)r.x) + "," +
                    std::to_string((int)r.y) + "," +
                    std::to_string((int)r.z) + ")";

                title += " Scale(" +
                    std::to_string((int)s.x) + ")";
            }
            std::string modeText = "Translate";

            if (currentGizmoMode == ROTATE)
                modeText = "Rotate";

            if (currentGizmoMode == SCALE)
                modeText = "Scale";

            title += " | Mode: " + modeText;
            glfwSetWindowTitle(window, title.c_str());

            frameCount = 0;
            previousTime = currentTime;

        }
        EditorUI::DrawToolbar(
            scene,
            selectedObject,
            selectedLight,
            &cube,
            &shader,
            &cubeMaterial,
            camera,
            &torchModel,
            lightCounter,
            appMode
        );

        if (appMode == AppMode::Editor)
        {
            EditorUI::DrawHierarchy(scene, selectedObject, selectedLight);
            EditorUI::DrawLightInspector(selectedLight);
            EditorUI::DrawInspector(selectedObject);
            EditorUI::DrawDebug(deltaTime, totalObjects, visibleObjects, culledObjects, selectedObject);
            EditorUI::DrawStatistics(scene, camera, selectedObject, deltaTime);

            EditorUI::DrawAssetBrowser(
                scene,
                selectedObject,
                &cube,
                &shader,
                &cubeMaterial,
                camera,

                &woodenHouseModel,
                &pineTreeModel,
                &commonTreeModel,
                &rockModel,
                &bushModel,
                &woodLogModel,
                &treeStumpModel,
                &grassModel
            );
            // ================= PLAYER SPAWN TOOLS =================
        

            ImGui::SetNextWindowPos(
                ImVec2(
                    1240.0f,
                    360.0f
                ),
                ImGuiCond_Once
            );

            ImGui::SetNextWindowSize(
                ImVec2(
                    340.0f,
                    260.0f
                ),
                ImGuiCond_Once
            );

            ImGui::Begin("Player Tools");

            if (playerObject != nullptr)
            {
                ImGui::Text("Player Position");
                ImGui::Text(
                    "X: %.2f  Y: %.2f  Z: %.2f",
                    playerObject->transform.position.x,
                    playerObject->transform.position.y,
                    playerObject->transform.position.z
                );

                ImGui::Separator();

                ImGui::Text("Saved Spawn Position");
                ImGui::Text(
                    "X: %.2f  Y: %.2f  Z: %.2f",
                    playerSpawnPosition.x,
                    playerSpawnPosition.y,
                    playerSpawnPosition.z
                );

                ImGui::Separator();

                if (ImGui::Button("Set Spawn Here"))
                {
                    playerSpawnPosition =
                        playerObject->transform.position;

                    playerSpawnPosition.y =
                        GetPlayerTerrainY(
                            playerSpawnPosition.x,
                            playerSpawnPosition.z
                        );
                    playerObject->transform.position =
                        playerSpawnPosition;

                    std::cout
                        << "Player spawn set on terrain: "
                        << playerSpawnPosition.x << ", "
                        << playerSpawnPosition.y << ", "
                        << playerSpawnPosition.z
                        << std::endl;
                }

                if (ImGui::Button("Respawn Player"))
                {
                    playerObject->transform.position =
                        playerSpawnPosition;

                    playerObject->transform.position.y =
                        GetPlayerTerrainY(
                            playerObject->transform.position.x,
                            playerObject->transform.position.z
                        );

                    playerObject->transform.rotation =
                        glm::vec3(
                            0.0f
                        );

                    thirdPersonController.verticalVelocity =
                        0.0f;

                    thirdPersonController.isGrounded =
                        true;

                    std::cout
                        << "Player respawned on terrain."
                        << std::endl;
                }

                if (ImGui::Button("Place Player In Front Of Camera"))
                {
                    glm::vec3 forward =
                        glm::vec3(
                            camera.Front.x,
                            0.0f,
                            camera.Front.z
                        );

                    if (glm::length(forward) < 0.001f)
                    {
                        forward =
                            glm::vec3(
                                0.0f,
                                0.0f,
                                -1.0f
                            );
                    }

                    forward =
                        glm::normalize(
                            forward
                        );

                    glm::vec3 playerNewPosition =
                        camera.Position +
                        forward * 5.0f;

                    playerNewPosition.y =
                        GetPlayerTerrainY(
                            playerNewPosition.x,
                            playerNewPosition.z
                        );

                    playerObject->transform.position =
                        playerNewPosition;

                    thirdPersonController.verticalVelocity =
                        0.0f;

                    thirdPersonController.isGrounded =
                        true;

                    selectedObject =
                        playerObject;

                    std::cout
                        << "Player placed on terrain in front of camera."
                        << std::endl;
                }

                if (ImGui::Button("Select Player"))
                {
                    selectedObject =
                        playerObject;

                    selectedLight =
                        nullptr;

                    std::cout
                        << "Player selected."
                        << std::endl;
                }
            }
            else
            {
                ImGui::Text("Player not found.");
            }

            ImGui::End();
            // ================= SELECTED OBJECT PLACEMENT TOOLS =================
            ImGui::SetNextWindowPos(
                ImVec2(
                    1240.0f,
                    600.0f
                ),
                ImGuiCond_Once
            );

            ImGui::SetNextWindowSize(
                ImVec2(
                    340.0f,
                    280.0f
                ),
                ImGuiCond_Once
            );
            ImGui::Begin("Selected Object Tools");

            if (selectedObject != nullptr)
            {
                ImGui::Text("Selected:");
                ImGui::Text("%s", selectedObject->name.c_str());

                ImGui::Separator();

                ImGui::Text(
                    "Position: %.2f, %.2f, %.2f",
                    selectedObject->transform.position.x,
                    selectedObject->transform.position.y,
                    selectedObject->transform.position.z
                );

                ImGui::Text(
                    "Scale: %.2f, %.2f, %.2f",
                    selectedObject->transform.scale.x,
                    selectedObject->transform.scale.y,
                    selectedObject->transform.scale.z
                );

                ImGui::Separator();

                if (ImGui::Button("Move Selected In Front Of Camera"))
                {
                    glm::vec3 forward =
                        glm::vec3(
                            camera.Front.x,
                            0.0f,
                            camera.Front.z
                        );

                    if (glm::length(forward) < 0.001f)
                    {
                        forward =
                            glm::vec3(
                                0.0f,
                                0.0f,
                                -1.0f
                            );
                    }

                    forward =
                        glm::normalize(
                            forward
                        );

                    glm::vec3 newPosition =
                        camera.Position +
                        forward * 6.0f;

                    newPosition.y =
                        selectedObject->transform.position.y;
                    float oldTerrainY =
                        GetTerrainHeight(
                            selectedObject->transform.position.x,
                            selectedObject->transform.position.z
                        );

                    float heightOffset =
                        selectedObject->transform.position.y -
                        oldTerrainY;

                    newPosition.y =
                        GetTerrainHeight(
                            newPosition.x,
                            newPosition.z
                        ) +
                        heightOffset;
                    selectedObject->transform.position =
                        newPosition;

                    std::cout
                        << "Moved selected object in front of camera."
                        << std::endl;
                }

                if (ImGui::Button("Snap Selected To Ground"))
                {
                    selectedObject->transform.position.y =
                        GetObjectTerrainY(
                            selectedObject->transform.position.x,
                            selectedObject->transform.position.z,
                            0.05f
                        );

                    std::cout
                        << "Selected object snapped to terrain."
                        << std::endl;
                }

                ImGui::Separator();

                if (ImGui::Button("Rotate Y +15"))
                {
                    selectedObject->transform.rotation.y +=
                        15.0f;
                }

                ImGui::SameLine();

                if (ImGui::Button("Rotate Y -15"))
                {
                    selectedObject->transform.rotation.y -=
                        15.0f;
                }

                if (ImGui::Button("Rotate Y +90"))
                {
                    selectedObject->transform.rotation.y +=
                        90.0f;
                }

                ImGui::SameLine();

                if (ImGui::Button("Random Y Rotation"))
                {
                    selectedObject->transform.rotation.y =
                        static_cast<float>(
                            rand() % 360
                            );
                }

                ImGui::Separator();

                if (ImGui::Button("Scale Up"))
                {
                    selectedObject->transform.scale *=
                        1.1f;
                }

                ImGui::SameLine();

                if (ImGui::Button("Scale Down"))
                {
                    selectedObject->transform.scale *=
                        0.9f;
                }

                if (ImGui::Button("Random Small Scale"))
                {
                    float randomScale =
                        0.8f +
                        static_cast<float>(rand() % 41) / 100.0f;

                    selectedObject->transform.scale =
                        glm::vec3(
                            randomScale
                        );
                }

                ImGui::Separator();

                if (ImGui::Button("Mark As Collider"))
                {
                    selectedObject->isCollider =
                        true;

                    selectedObject->colliderRadius =
                        glm::max(
                            selectedObject->transform.scale.x,
                            selectedObject->transform.scale.z
                        ) * 0.8f;

                    std::cout
                        << "Selected object marked as collider."
                        << std::endl;
                }

                ImGui::SameLine();

                if (ImGui::Button("Remove Collider"))
                {
                    selectedObject->isCollider =
                        false;

                    std::cout
                        << "Selected object collider removed."
                        << std::endl;
                }

                ImGui::Text(
                    "Collider: %s",
                    selectedObject->isCollider ? "ON" : "OFF"
                );
            }
            else
            {
                ImGui::Text("No object selected.");
            }

            ImGui::End();
            // ================= CAMP BUILDER TOOLS =================

            ImGui::SetNextWindowPos(
                ImVec2(
                    920.0f,
                    360.0f
                ),
                ImGuiCond_Once
            );

            ImGui::SetNextWindowSize(
                ImVec2(
                    300.0f,
                    220.0f
                ),
                ImGuiCond_Once
            );

            // ================= CAMP BUILDER TOOLS =================

            ImGui::SetNextWindowPos(
                ImVec2(
                    920.0f,
                    360.0f
                ),
                ImGuiCond_Once
            );

            ImGui::SetNextWindowSize(
                ImVec2(
                    300.0f,
                    160.0f
                ),
                ImGuiCond_Once
            );

            ImGui::Begin("Camp Builder");

            ImGui::TextWrapped(
                "Use Asset Browser -> Structures -> Build Real Camp."
            );

            ImGui::Separator();

            ImGui::TextWrapped(
                "The old cube camp builder was removed because it created placeholder objects."
            );

            ImGui::End();
            if (selectedObject != nullptr)
            {
                glLineWidth(4.0f);
                gizmoShader.use();

                glm::mat4 gizmoModel =
                    glm::translate(
                        glm::mat4(1.0f),
                        selectedObject->transform.position
                    );

                gizmoShader.setMat4("model", glm::value_ptr(gizmoModel));
                gizmoShader.setMat4("view", glm::value_ptr(view));
                gizmoShader.setMat4("projection", glm::value_ptr(projection));

                glBindVertexArray(gizmoVAO);

                gizmoShader.setVec3("axisColor", glm::vec3(1, 0, 0));
                glDrawArrays(GL_LINES, 0, 2);

                gizmoShader.setVec3("axisColor", glm::vec3(0, 1, 0));
                glDrawArrays(GL_LINES, 2, 2);

                gizmoShader.setVec3("axisColor", glm::vec3(0, 0, 1));
                glDrawArrays(GL_LINES, 4, 2);

                glBindVertexArray(0);
            }
        }
        else
        {
            EditorUI::DrawCrosshair();

            ImGui::Begin("Play Mode Info");

            ImGui::Text("PLAY MODE ACTIVE");
            ImGui::Separator();

            ImGui::Text("Controls:");
            ImGui::Text("WASD - Move");
            ImGui::Text("SHIFT - Run");
            ImGui::Text("SPACE - Jump");
            ImGui::Text("Right Mouse - Look Around");

            ImGui::Separator();

            ImGui::Text(
                "Animation State: %s",
                thirdPersonController.GetAnimStateName()
            );

            ImGui::Text(
                "Grounded: %s",
                thirdPersonController.isGrounded ? "Yes" : "No"
            );

            ImGui::Text(
                "Speed: %.2f",
                thirdPersonController.currentMoveSpeed
            );

            ImGui::Separator();
            ImGui::Text("Press Stop to return to editor.");
            ImGui::Separator();

            if (ImGui::Button("Save Editor Objects"))
            {
                SaveEditorObjects(
                    scene,
                    "SavedEditorObjects.txt"
                );
            }

            if (ImGui::Button("Load Editor Objects"))
            {
                LoadEditorObjects(
                    scene,
                    "SavedEditorObjects.txt",
                    &cube,
                    &shader,
                    &cubeMaterial,
                    selectedObject,
                    &woodenHouseModel,
                    &pineTreeModel,
                    &commonTreeModel,
                    &rockModel,
                    &bushModel,
                    &woodLogModel,
                    &treeStumpModel,
                    &grassModel
                );
            }
            ImGui::End();
        }

        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        glfwSwapBuffers(window);
}
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    glfwTerminate();

    return 0;

}


