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

    if (glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_RIGHT) == GLFW_PRESS
        && !io.WantCaptureMouse)
    {
        camera.ProcessMouseMovement(xoffset, yoffset);
    }
}
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
    if (selectedObject != nullptr)
    {
        float scaleSpeed = 0.2f;

        selectedObject->transform.scale += glm::vec3(yoffset * scaleSpeed);

    
        selectedObject->transform.scale = glm::max(
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

in vec3 FragPos;
in vec3 Normal;
in vec2 TexCoord;
uniform bool isSelected;
uniform sampler2D texture1;

uniform vec3 viewPos;

uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightColors[MAX_LIGHTS];

uniform vec3 materialAmbient;
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

void main()
{
    vec3 textureColor = texture(texture1, TexCoord).rgb; 

    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);

    vec3 result = vec3(0.0);

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
    result *= 1.5; // brighter
}

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

    GLFWwindow* window = glfwCreateWindow(800, 600, "ThesisRenderer", NULL, NULL);
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
    ImGui_ImplOpenGL3_Init("#version 330");
    glDisable(GL_CULL_FACE);
    TestAssimp();
    InputManager::Init(window);
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
    Model myModel("character-human.obj");
    Model treeModel("character-a.obj");
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

    Mesh cube(vertices, sizeof(vertices));
    SceneObject cube1(&cube, &shader, &cubeMaterial);
    SceneObject cube2(&cube, &shader, &cubeMaterial);
    SceneObject cube3(&cube, &shader, &cubeMaterial);

    cube1.name = "Cube 1";
    cube2.name = "Cube 2";
    cube3.name = "Cube 3";

    cube1.transform.position = glm::vec3(0.0f, 0.0f, 0.0f);
    cube2.transform.position = glm::vec3(2.0f, 0.0f, 0.0f);
    cube3.transform.position = glm::vec3(-2.0f, 0.0f, 0.0f);
    cube2.transform.scale = glm::vec3(1.5f);
    cube3.transform.scale = glm::vec3(0.5f);

    std::vector<SceneObject*> manyCubes;

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
    }

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

    // ===== ATTACH COMPONENTS =====
    cube1.AddComponent(new RotatorComponent(glm::vec3(0.0f, 1.0f, 0.0f), 50.0f));
    cube2.AddComponent(new RotatorComponent(glm::vec3(1.0f, 0.0f, 0.0f), 30.0f));
    cube3.AddComponent(new OscillatorComponent(0.5f, 2.0f));

   
    while (!glfwWindowShouldClose(window))
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        float currentFrame = glfwGetTime();
        deltaTime = currentFrame - lastFrame;
        lastFrame = currentFrame;

        float cameraSpeed = 2.5f * deltaTime;

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
       
        if (selectedObject != nullptr)
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

        if (!io.WantCaptureKeyboard)
        {
            if (InputManager::IsKeyPressed(GLFW_KEY_W))
                camera.Position += cameraSpeed * camera.Front;

            if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
                camera.Position -= cameraSpeed * camera.Front;

            if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
                camera.Position -=
                glm::normalize(glm::cross(camera.Front, camera.Up))
                * cameraSpeed;

            if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
                camera.Position +=
                glm::normalize(glm::cross(camera.Front, camera.Up))
                * cameraSpeed;
        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
            camera.Position -= cameraSpeed * camera.Front;

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
            camera.Position -= glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
            camera.Position += glm::normalize(glm::cross(camera.Front, camera.Up)) * cameraSpeed;
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
        if (selectedObject != nullptr)
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
        if (selectedObject != nullptr)
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

        glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glm::mat4 model = glm::rotate(glm::mat4(1.0f),
            (float)glfwGetTime(),
            glm::vec3(0.5f, 1.0f, 0.0f));
        glm::mat4 view = camera.GetViewMatrix();

        glm::mat4 projection = glm::perspective(glm::radians(45.0f),
            800.0f / 600.0f,
            0.1f,
            100.0f);

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
        if (mouseClicked && !ImGui::GetIO().WantCaptureMouse)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            glm::vec3 rayDir = GetRayFromMouse(mouseX, mouseY, 800, 600, projection, view);
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
        if (isDragging && selectedObject != nullptr)
        {
            double mouseX, mouseY;
            glfwGetCursorPos(window, &mouseX, &mouseY);

            // ================= TRANSLATE MODE =================
            if (currentGizmoMode == TRANSLATE)
            {
                glm::vec3 rayDir = GetRayFromMouse(mouseX, mouseY, 800, 600, projection, view);
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
        for (int i = 0; i < 3; i++)
        {
            shader.setVec3("lightPositions[" + std::to_string(i) + "]", lightPositions[i]);
            shader.setVec3("lightColors[" + std::to_string(i) + "]", lightColors[i]);
        }
        shader.setMat4("view", glm::value_ptr(view));
        shader.setMat4("projection", glm::value_ptr(projection));
        shader.setVec3("viewPos", camera.Position);
        shader.setVec3("materialAmbient", glm::vec3(0.3f));
        shader.setVec3("materialDiffuse", glm::vec3(1.0f));
        shader.setVec3("materialSpecular", glm::vec3(0.5f));
        shader.setFloat("materialShininess", 32.0f);
        shader.setInt("texture1", 0);

        // ===== DRAW CUBES (container.jpg) =====
        glActiveTexture(GL_TEXTURE0);
        containerTexture.Bind();

        // Update all components (rotation, oscillation, etc.)
        cube1.transform.position = glm::vec3(0.0f, 0.0f, -3.0f);
        cube1.UpdateComponents(deltaTime);
        cube1.Draw(renderer, glm::mat4(1.0f));
        scene.objects[0]->AddChild(scene.objects[1]);
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
            if (!useCulling || frustum.IsSphereVisible(obj->transform.position, obj->boundingRadius))
            {
                visibleObjects++;

                obj->Draw(renderer, glm::mat4(1.0f));
            }
            else
            {
                culledObjects++;
            }
        }
       
        // ===== DRAW MODELS (each binds its own texture) =====
        glm::mat4 model1 = glm::mat4(1.0f);
        model1 = glm::translate(model1, glm::vec3(-3.0f, 2.0f, -3.0f));
        shader.setMat4("model", glm::value_ptr(model1));
        myModel.Draw();

        glm::mat4 model2 = glm::mat4(1.0f);
        model2 = glm::translate(model2, glm::vec3(3.0f, 0.0f, -5.0f));
        shader.setMat4("model", glm::value_ptr(model2));
        treeModel.Draw();

        // ===== DRAW LIGHT CUBE =====
        lightShader.use();
        glm::mat4 lightModel = glm::mat4(1.0f);
        lightModel = glm::translate(lightModel, glm::vec3(2.0f, 2.0f, 2.0f));
        lightModel = glm::scale(lightModel, glm::vec3(0.1f));
        lightShader.setMat4("model", glm::value_ptr(lightModel));
        lightShader.setMat4("view", glm::value_ptr(view));
        lightShader.setMat4("projection", glm::value_ptr(projection));
        renderer.DrawMesh(cube, lightShader, lightModel);

        // ===== FPS COUNTER =====
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
            EditorUI::DrawToolbar(
                scene,
                selectedObject,
                &cube,
                &shader,
                &cubeMaterial,
                lightCounter
            );

            EditorUI::DrawToolbar(
                scene,
                selectedObject,
                &cube,
                &shader,
                &cubeMaterial,
                lightCounter
            );

            EditorUI::DrawHierarchy(
                scene,
                selectedObject,
                selectedLight
            );
            EditorUI::DrawLightInspector(
                selectedLight
            );
            EditorUI::DrawInspector(
                selectedObject
            );
            if (!scene.lights.empty())
            {
                EditorUI::DrawLightInspector(
                    scene.lights[0]
                );
            }
            EditorUI::DrawDebug(
                deltaTime,
                totalObjects,
                visibleObjects,
                culledObjects,
                selectedObject
            );

            EditorUI::DrawStatistics(
                scene,
                camera,
                selectedObject,
                deltaTime
            );
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