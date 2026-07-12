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
    if (firstMouse)
    {
        lastX = (float)xpos;
        lastY = (float)ypos;
        firstMouse = false;
    }

    float xoffset = (float)xpos - lastX;
    float yoffset = lastY - (float)ypos;

    lastX = (float)xpos;
    lastY = (float)ypos;

    //camera.ProcessMouseMovement(xoffset, yoffset);
    ImGuiIO& io = ImGui::GetIO();

    if (
        glfwGetMouseButton(
            window,
            GLFW_MOUSE_BUTTON_RIGHT
        ) == GLFW_PRESS
        &&
        !io.WantCaptureMouse
        )
    {
        camera.ProcessMouseMovement(
            xoffset,
            yoffset
        );
    }
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
#define MAX_LIGHTS 3

out vec4 FragColor;
uniform vec3 sunDirection;
uniform vec3 sunColor;
in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;

uniform bool isSelected;
uniform sampler2D texture1;
uniform vec3 materialTint;
uniform vec3 viewPos;

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

if (useTexture)
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

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

// Directional Light

vec3 dirLight =
normalize(-sunDirection);

float dirDiff =
max(dot(norm,dirLight),0.0);

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
0.15;

result +=
dirAmbient +
dirDiffuse +
dirSpecular;

    for(int i = 0; i < MAX_LIGHTS; i++)
    {
        vec3 lightDir = normalize(lightPositions[i] - FragPos);

        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * materialDiffuse * textureColor * lightColors[i];

        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), materialShininess);
        vec3 specular = materialSpecular * spec * lightColors[i];

        vec3 ambient = materialAmbient * textureColor;

        result += ambient + diffuse + specular;
    }

if(isSelected)
{
    result *= 1.3;
}


result *= 0.45;


result = clamp(result, 0.0, 1.0);

FragColor = vec4(result, 1.0);

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
    Light* testLight = new Light();

    static int lightCounter = 1;

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

    scene.AddLight(testLight);
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


    Material cubeMaterial(&containerTexture);
    Material groundMaterial(
        &containerTexture
    );
    Mesh cube(vertices, sizeof(vertices));
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

    // Trees
    for (int i = 0; i < 60; i++)
    {
        Model* chosenTree =
            treeModels[
                rand() % treeModels.size()
            ];

        float x =
            RandomRange(
                -55.0f,
                55.0f
            );

        float z =
            RandomRange(
                -55.0f,
                55.0f
            );

        float scale =
            RandomRange(
                0.70f,
                1.10f
            );
        AddEnvironmentModel(
            chosenTree,
            "Generated Tree",
            glm::vec3(
                x,
                0.05f,
                z
            ),
            glm::vec3(
                scale
            ),
            false
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
                -50.0f,
                50.0f
            );

        float z =
            RandomRange(
                -50.0f,
                50.0f
            );

        float scale =
            RandomRange(
                0.60f,
                1.00f
            );

        AddEnvironmentModel(
            chosenRock,
            "Generated Rock",
            glm::vec3(
                x,
                0.05f,
                z
            ),
            glm::vec3(
                scale
            ),
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
                -55.0f,
                55.0f
            );

        float z =
            RandomRange(
                -55.0f,
                55.0f
            );

        float scale =
            RandomRange(
                0.50f,
                0.85f
            );

        AddEnvironmentModel(
            chosenPlant,
            "Generated Plant",
            glm::vec3(
                x,
                0.05f,
                z
            ),
            glm::vec3(
                scale
            ),
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
                -45.0f,
                45.0f
            );

        float z =
            RandomRange(
                -45.0f,
                45.0f
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
                0.05f,
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
                    0.05f,
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

            AddEnvironmentModel(
                chosenRock,
                "Rock Cluster",
                glm::vec3(
                    centerX + RandomRange(-3.0f, 3.0f),
                    0.05f,
                    centerZ + RandomRange(-3.0f, 3.0f)
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
    playerObject->transform.position =
        playerSpawnPosition;

    playerObject->transform.scale =
        glm::vec3(
            0.6f
        );

    playerObject->boundingRadius = 3.0f;

    scene.AddObject(playerObject);
    SceneObject* environmentObject =
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
    );
    
    ground.boundingRadius = 100.0f;
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

    scene.AddObject(treeObject);
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

        glfwPollEvents();



        ImGuiIO& io = ImGui::GetIO();

        if (
            glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT)
            == GLFW_PRESS
            &&
            !io.WantCaptureMouse
            )
        {
            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_DISABLED
            );
        }
        else
        {
            glfwSetInputMode(
                window,
                GLFW_CURSOR,
                GLFW_CURSOR_NORMAL
            );
        }
        bool nKeyCurrent = glfwGetKey(window, GLFW_KEY_N) == GLFW_PRESS;

        if (nKeyCurrent && !nKeyLastState)
        {
            SceneObject* newCube = new SceneObject(&cube, &shader, &cubeMaterial);
            newCube->name = "New Cube";
            newCube->AddComponent(
                new RotateComponent()
            );

            newCube->transform.position = camera.Position + camera.Front * 3.0f;

            scene.AddObject(newCube);
            selectedObject = newCube;
            std::cout << "New cube added and selected\n";


        }

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
            100.0f
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
        shader.setVec3(
            "sunDirection",
            glm::vec3(-0.2f, -1.0f, -0.3f));

        shader.setVec3(
            "sunColor",
            glm::vec3(1.0f));
        for (int i = 0; i < 3; i++)
        {
            shader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            shader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
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
                if (obj->material != nullptr)
                {
                    shader.setBool("useTexture", true);

                    glActiveTexture(GL_TEXTURE0);

                    if (obj->material->texture != nullptr)
                    {
                        obj->material->texture->Bind();
                    }
                    shader.setBool("useTexture", true);
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
                    glPolygonMode(
                        GL_FRONT_AND_BACK,
                        GL_FILL
                    );

                    shader.setVec3(
                        "materialAmbient",
                        glm::vec3(0.4f)
                    );

                    shader.setVec3(
                        "materialDiffuse",
                        glm::vec3(0.8f)
                    );

                    shader.setVec3(
                        "materialSpecular",
                        glm::vec3(0.1f)
                    );

                    shader.setVec3(
                        "materialTint",
                        glm::vec3(1.0f)
                    );

                    shader.setFloat(
                        "materialShininess",
                        8.0f
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
            &cube,
            &shader,
            &cubeMaterial,
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

                &pineTreeModel,
                &commonTreeModel,
                &rockModel,
                &bushModel,
                &woodLogModel,
                &treeStumpModel,
                &grassModel
            );

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


