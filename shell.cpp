#include <iostream>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <chrono>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "vec4.h"
#include "mat4.h"
#include "camera.h"
#include "mesh.h"
#include "model.h"
#include "proc.h"

#include "./assets/vertices.h"

#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 600

void loop();
void processInput(GLFWwindow* window, double deltaTime);
void processMouse(GLFWwindow* window, double xposIn, double yposIn);
unsigned int loadCubeMap(std::vector<std::string> faces);
void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods);


// Shader and Program Utils
GLuint loadShader(GLenum shaderType, const char* filePath)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader) 
    {
        std::ifstream shaderFile(filePath);
        if (shaderFile) {

            std::stringstream shaderStream;
            shaderStream << shaderFile.rdbuf();
            shaderFile.close();

            std::string shaderCode = shaderStream.str();
            const char* shaderSource = shaderCode.c_str();

            glShaderSource(shader, 1, &shaderSource, nullptr);
            glCompileShader(shader);

            GLint compiled = 0;
            glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

            if (!compiled)
            {
                GLint infolen = 0;
                glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infolen);

                if (infolen)
                {
                    char* errMessage = (char*)malloc(infolen);
                    glGetShaderInfoLog(shader, infolen, nullptr, errMessage);
                    std::cout << "Could not compile shader: " << shaderType << std::endl;
                    std::cout << "Error Message: " << errMessage << std::endl;
                    free(errMessage);
                }
                glDeleteShader(shader);
                shader = 0;
            }
        }
        else
        {
            std::cout << "Failed to open shader file: " << filePath << std::endl;
            glDeleteShader(shader);
            shader = 0;
        }
    }
    return shader;
}

GLuint createProgram(const char* vertexSourceFile, const char* fragmentSourceFile)
{
   GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSourceFile);
    if (!vertexShader)
    {
        return 0;
    }
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSourceFile);
    if (!fragmentShader)
    {
        return 0;
    }

	GLuint program = glCreateProgram();
		if (program)
		{
			glAttachShader(program , vertexShader);
			glAttachShader(program, fragmentShader);
			glLinkProgram(program);
			GLint linkStatus = GL_FALSE;
			glGetProgramiv(program , GL_LINK_STATUS, &linkStatus);
			if( linkStatus != GL_TRUE)
			{
				GLint infolen = 0;
				glGetProgramiv(program, GL_INFO_LOG_LENGTH, &infolen);
				if (infolen)
				{
					char* errorMessage = (char*) malloc(infolen);
					if (errorMessage)
					{
						glGetProgramInfoLog(program, infolen, nullptr, errorMessage);
                        std::cout << errorMessage << std::endl;
						free(errorMessage);
					}
				}
				glDeleteProgram(program);
				program = 0;
			}
		}
    return program;
}


const GLfloat vertices[] = {    
   //Positions      //Colors          //Texture    //Normal
  -0.5, 0.5, 0.0, 0.694, 0.784, 0.949, 0.0, 1.0, 0.0, 0.0, -1.0, // Top left
  -0.5, -0.5, 0.0, 0.694, 0.784, 0.949, 0.0, 0.0, 0.0, 0.0, -1.0, // Bottom left
   0.5, -0.5, 0.0, 0.694, 0.784, 0.949, 1.0, 0.0, 0.0, 0.0, -1.0,// Bottom right
   0.5, 0.5, 0.0, 0.694, 0.784, 0.949, 1.0, 1.0, 0.0, 0.0, -1.0 // Top right
};

const GLfloat b_vertices[] = { 
    -2.0, 2.0, -1.0,
    -2.0, -2.0, -1.0,
    2.0, -2.0, -1.0,
    2.0, 2.0, -1.0
};

const GLushort indices[] = {
	0, 1, 2, // first triangle
	0, 2, 3  // second triangle
};

const GLushort b_indices[] = {
	0, 1, 2, // first triangle
	0, 2, 3  // second triangle
};

// Set point at origin
GLfloat pointPosition[24] = { 0.0, 5.0, -9.0 };
// Set the point size
GLfloat pointSize = 150.0f;
// Set the point color
GLfloat pointColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };


GLuint quadProgram = 0;
GLuint planetProgram = 0;
GLuint fractalProgram = 0;
GLuint b_lightProgram = 0;
GLuint skyboxProgram = 0;
unsigned int skyboxVAO, skyboxVBO;
unsigned int quadVAO, quadVBO, quadEBO;
unsigned int fractVAO, fractVBO, fractEBO;
unsigned int b_lightVAO, b_lightVBO;
GLFWwindow* window;

// Camera Set up
Camera camera;
double lastX = CANVAS_WIDTH / 2.0;
double lastY = CANVAS_HEIGHT / 2.0;
double xpos = 0;
double ypos = 0;
bool firstMouse = true;
bool rightMouseButtonPressed = false;

// Process Time
double deltaTime = 0;
double lastTime = 0;

// Textures init
unsigned int tex;
unsigned int cubemapTexture;

Model model1;
Planet planet;

int main()
{
    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "WebGl window", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the created window the current context and set up MS, 
    // AA added on webGL context directly from generated JS Emscripten file
    glfwMakeContextCurrent(window);
    glfwWindowHint(GLFW_SAMPLES, 4);
    glfwSetMouseButtonCallback(window, mouseButtonCallback);

    // QUAD PROG, starting with textures
    quadProgram = createProgram("/shaders/shader.vert", "/shaders/shader.frag");


    glGenTextures(1, &tex);
    glBindTexture(GL_TEXTURE_2D, tex);
    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	
    int width, height, nrChannels;
    unsigned char *data = stbi_load("/assets/marble-texture.jpg", &width, &height, &nrChannels, 0);

    stbi_set_flip_vertically_on_load(true); 
    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture file" << std::endl;
    }

    stbi_image_free(data);

    // Create Buffers for Quad Program
    glGenVertexArrays(1, &quadVAO);
    glGenBuffers(1, &quadVBO);
    glGenBuffers(1, &quadEBO);

    glBindVertexArray(quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, quadEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    // Color Attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(3 * sizeof(float)));
	glEnableVertexAttribArray(1);
    // Texture data
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(6 * sizeof(float)));
	glEnableVertexAttribArray(2);
    // Normal Data
    glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, 11 * sizeof(float), (void*)(8 * sizeof(float)));
	glEnableVertexAttribArray(3);

    // Activate shaders before sending texture uniform
    glUseProgram(quadProgram);
    glUniform1i(glGetUniformLocation(quadProgram, "material.tex"), 0);

	char* path = (char*)"/assets/backpack/backpack.obj";
    model1 = Model(path);


    // Unbind VAO
    glBindVertexArray(0);
    glUseProgram(0);

    // Planet Program
	planetProgram = createProgram("/shaders/planet_shader.vert", "/shaders/planet_shader.frag");

    glUseProgram(planetProgram);
    std::vector<TerrainFace> terrainFaces;
    int resolution = 12;
    std::vector<float> color{ 0.7, 0.3, 0.4 };
    Mesh sharedMesh;
    // up, down then left, right then forward, back
    vec3 directions[6] = { vec3(0.0, 1.0, 0.0), vec3(0.0, -1.0, 0.0), 
        vec3(1.0, 0.0, 0.0), vec3(-1.0, 0.0, 0.0), 
        vec3(0.0, 0.0, 1.0), vec3(0.0, 0.0, -1.0) };
    for (int i = 0; i < 6; i++)
    {
        terrainFaces.push_back(TerrainFace(sharedMesh, resolution, directions[i], color));
    }
    planet = Planet(terrainFaces);
    planet.setBaseGUI(window);

    /* Debug
    std::cout << "Num of Meshes: " << terrainFaces.size() << std::endl;
    std::cout << "Planet mesh1 num of vertices: " << planet.terrainFaces[0].mesh.vertices.size() << std::endl;

    std::cout << "List of indices:" << std::endl;
    for (auto num : planet.terrainFaces[0].mesh.indices)
        std::cout << num << std::endl;
	std::cout << "List of vertices:" << std::endl;
    for (auto vertex : planet.terrainFaces[0].mesh.vertices)
        std::cout << "x: " << vertex.Pos[0] << "y: " << vertex.Pos[1] << "z: " << vertex.Pos[2] << std::endl;
    */

    // Fractal Program
    fractalProgram = createProgram("/shaders/shader_2.vert", "/shaders/shader_2.frag");
    // Create Buffers for Fractal Program
    glGenVertexArrays(1, &fractVAO);
    glGenBuffers(1, &fractVBO);
    glGenBuffers(1, &fractEBO);

    glBindVertexArray(fractVAO);

    glBindBuffer(GL_ARRAY_BUFFER, fractVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(b_vertices), b_vertices, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, fractEBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(b_indices), b_indices, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	// Unbind VAO
    glBindVertexArray(0);


	// BillBoard lights Progam
    b_lightProgram = createProgram("/shaders/b_light.vert", "/shaders/b_light.frag");
    // Create Buffers for Fractal Program
    glGenVertexArrays(1, &b_lightVAO);
    glGenBuffers(1, &b_lightVBO);

    glBindVertexArray(b_lightVAO);

	for (int i = 0; i < 7; i++)
    {
        double theta = degrees_to_radians(45);
        vec3 posVec = vec3(static_cast<double>(pointPosition[3 * i]), 
            static_cast<double>(pointPosition[1 + 3 * i]), 
            static_cast<double>(pointPosition[2 + 3 * i]));

        vec3 nextLight = vec3(cos(theta) * posVec.x() + sin(theta) * posVec.z(),
            posVec.y(),
            -(sin(theta) * posVec.x()) + cos(theta) * posVec.z());

        pointPosition[3 + 3 * i] = static_cast<GLfloat>(round(nextLight.x()));
        pointPosition[4 + 3 * i] = static_cast<GLfloat>(round(nextLight.y()));
        pointPosition[5 + 3 * i] = static_cast<GLfloat>(round(nextLight.z()));
    }

    glBindBuffer(GL_ARRAY_BUFFER, b_lightVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(pointPosition), pointPosition, GL_STATIC_DRAW);

    // position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	// Unbind VAO
    glBindVertexArray(0);


	// Skybox program
    skyboxProgram = createProgram("/shaders/skybox.vert", "/shaders/skybox.frag");
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);

    glBindVertexArray(skyboxVAO);

    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);

	std::vector<std::string> faces {
		"/assets/skybox/right.jpg",
		"/assets/skybox/left.jpg",
		"/assets/skybox/bottom.jpg",
		"/assets/skybox/top.jpg",
		"/assets/skybox/front.jpg",
		"/assets/skybox/back.jpg"
	};

    cubemapTexture = loadCubeMap(faces);

	// Unbind VAO
    glBindVertexArray(0);

    glUseProgram(skyboxProgram);
    glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);


    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);


    emscripten_set_main_loop(loop, 0, 1);
    glfwTerminate();
    return 0;
}


void loop()
{

    if (rightMouseButtonPressed)
    {
        glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
    }

    double now = glfwGetTime();

    deltaTime = now - lastTime;
    lastTime = now;

    // Process Inputs
    glfwGetCursorPos(window, &xpos, &ypos);
    
    processInput(window, deltaTime);
    processMouse(window, xpos, ypos);

    mat4 view = camera.GetViewMatrix();
    mat4 proj = projection_mat(60, CANVAS_WIDTH, CANVAS_HEIGHT, 0.1, 100);
	mat4 vp = proj * view;

    glClearColor(0.1, 0.1, 0.2, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


    // Start Quad program
    glUseProgram(quadProgram);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, tex);

    glBindVertexArray(quadVAO);

    unsigned int shineLoc = glGetUniformLocation(quadProgram, "material.shininess");
    glUniform1f(shineLoc, 32.0f);

    mat4 vp1 = vp;
    unsigned int vpLoc = glGetUniformLocation(quadProgram, "vp");
    std::vector<float> formattedVP1 = vp1.toFloatVector();
	glUniformMatrix4fv(vpLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedVP1.data()));

    mat4 model;
	unsigned int modelLoc = glGetUniformLocation(quadProgram, "model");

    // Light loop
    for (GLuint i = 0; i < 8; i++)
    {
        std::string num = std::to_string(i);

        glUniform3f(glGetUniformLocation(quadProgram, ("pointLights[" + num + "].position").c_str()),
            pointPosition[0 + i * 3], pointPosition[1 + i * 3], pointPosition[2 + i * 3]);
        glUniform3f(glGetUniformLocation(quadProgram,("pointLights[" + num + "].ambient").c_str()), 0.1f, 0.1f, 0.1f);
        glUniform3f(glGetUniformLocation(quadProgram, ("pointLights[" + num + "].diffuse").c_str()), 0.3f, 0.3f, 0.3f);
        glUniform3f(glGetUniformLocation(quadProgram, ("pointLights[" + num + "].specular").c_str()), 0.5f, 0.5f, 0.5f);
        glUniform1f(glGetUniformLocation(quadProgram, ("pointLights[" + num + "].constant").c_str()), 1.0f);
        glUniform1f(glGetUniformLocation(quadProgram, ("pointLights[" + num + "].linear").c_str()), 0.09f);
        glUniform1f(glGetUniformLocation(quadProgram, ("pointLights[" + num + "].quadratic").c_str()), 0.0032f);
    }

    mat4 model0 = model;
    for (int i = 0; i < 8; i++)
    {
        model = translate(model0, vec3(-12 + 4 * i, -2, 0));
        model = pitch(model, -90);
        model = scale(model, 4);

        std::vector<float> formattedModel = model.toFloatVector();
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedModel.data()));


        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        for (int j = 0; j < 8; j++)
        {
            model = translate(model0, vec3(-12 + 4 * i, -2, -12 + 4 * j));
            model = pitch(model, -90);
            model = scale(model, 4);

			std::vector<float> formattedModel = model.toFloatVector();
			glUniformMatrix4fv(modelLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedModel.data()));

            if (j != 3)
                glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
        }
    }
    
    GLfloat camPos[] = {static_cast<GLfloat>(camera.Position.x()),
        static_cast<GLfloat>(camera.Position.y()),
        static_cast<GLfloat>(camera.Position.z())};

	unsigned int viewPosLoc = glGetUniformLocation(quadProgram, "viewPos");
    glUniform3fv(viewPosLoc, 1, camPos);

    mat4 reset;
    model = reset;
    model = translate(model, vec3(4.0, 0.0, -2.0));
    model = yaw(model, 180);
	model = scale(model, 0.4);
    std::vector<float> formattedModel = model.toFloatVector();
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedModel.data()));

	model1.Draw(quadProgram);

    glBindVertexArray(0);


    // Start Planet program
    glUseProgram(planetProgram);

    mat4 vp3 = vp;
    unsigned int vp3Loc = glGetUniformLocation(planetProgram, "vp");
    std::vector<float> formattedVP3 = vp3.toFloatVector();
	glUniformMatrix4fv(vp3Loc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedVP3.data()));

    mat4 model3;
    model3 = translate(model3, vec3(-6.0, 0.0, 0.0));
	unsigned int modelLoc3 = glGetUniformLocation(planetProgram, "model");
	std::vector<float> formattedModel3 = model3.toFloatVector();
    glUniformMatrix4fv(modelLoc3, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedModel3.data()));

	unsigned int viewPosLoc2 = glGetUniformLocation(planetProgram, "viewPos");
    glUniform3fv(viewPosLoc2, 1, camPos);

	if (planet.applyGradient)
	{
		unsigned int gradLoc = glGetUniformLocation(planetProgram, "applyGradient");
		GLint start = 1;
		glUniform1i(gradLoc, start);
	}

    planet.Draw(planetProgram);

    // Begin Fractal program
    glUseProgram(fractalProgram);

    mat4 model2;
    mat4 mvp2 = proj * view * model2;

    unsigned int mvpLoc2 = glGetUniformLocation(fractalProgram, "mvp");
    unsigned int timeLoc = glGetUniformLocation(fractalProgram, "time");

    std::vector<float> formattedMVP2 = mvp2.toFloatVector();
    glUniformMatrix4fv(mvpLoc2, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedMVP2.data()));

    glUniform1f(timeLoc, static_cast<GLfloat>(now));

    glBindVertexArray(fractVAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, 0);
    glBindVertexArray(0);


	// Begin skybox program
	mat4 vp2 = proj * view;
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxProgram);

    unsigned int vp2Loc = glGetUniformLocation(skyboxProgram, "vp");
    std::vector<float> formattedVP2 = vp2.toFloatVector();
	glUniformMatrix4fv(vp2Loc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedVP2.data()));
 
    // skybox cube
    glBindVertexArray(skyboxVAO);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
 
    glDepthFunc(GL_LESS); // set depth function back to default

    //Begin billboard Light program
    glUseProgram(b_lightProgram);
    glBindVertexArray(b_lightVAO);

    mat4 pointView = view;
    mat4 pointProj = proj;

    unsigned int viewLoc = glGetUniformLocation(b_lightProgram, "view");
    unsigned int projLoc = glGetUniformLocation(b_lightProgram, "proj");

    std::vector<float> formattedView = pointView.toFloatVector();
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedView.data()));
    std::vector<float> formattedProj = pointProj.toFloatVector();
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedProj.data()));

    unsigned int pointSizeLoc = glGetUniformLocation(b_lightProgram, "pointSize");
    unsigned int colorLoc = glGetUniformLocation(b_lightProgram, "pointColor");

    glUniform1f(pointSizeLoc, pointSize);
    glUniform4fv(colorLoc, 1, pointColor);

    glDrawArrays(GL_POINTS, 0, 8);
    glBindVertexArray(0);

     // Set a square around the sphere to pop IMGUI, press G to enbale mouse
    if (camera.Position[0] < -3.0 && camera.Position[0] > -9.0 &&
        camera.Position[2] < 6.0 && camera.Position[2] > -6.0)
    {
        planet.RenderUI(window);
        planet.update();
    }

    glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

// Input Utils
void processInput(GLFWwindow* window, double deltaTime)
{
	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.reset();

    if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        camera.processKeyboard(FORWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        camera.processKeyboard(BACKWARD, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        camera.processKeyboard(LEFT, deltaTime);
    if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        camera.processKeyboard(RIGHT, deltaTime);
}

void processMouse(GLFWwindow* window, double xposIn, double yposIn)
{
    if (rightMouseButtonPressed)
    {
        double xpos = xposIn;
        double ypos = yposIn;

        if (firstMouse)
        {
            lastX = xpos;
            lastY = ypos;
            firstMouse = false;
        }

        double xoffset = xpos - lastX;
        double yoffset = lastY - ypos;

        lastX = xpos;
        lastY = ypos;

        camera.processMouseMovement(xoffset, yoffset);
    }
    else
    {
        // Store the cursor position when the right mouse button is released
        lastX = xpos;
        lastY = ypos;
        firstMouse = true;
    }
}

unsigned int loadCubeMap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (int i = 0; i < faces.size(); i++)
    {
        unsigned char* cubemap_data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (cubemap_data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                0, GL_RGB, GL_UNSIGNED_BYTE, cubemap_data);
            stbi_image_free(cubemap_data);
        }
        else
        {
            std::cout << "Failed to load cubemap at: " << faces[i] << std::endl;
            stbi_image_free(cubemap_data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}

void mouseButtonCallback(GLFWwindow* window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_RIGHT)
    {
        if (action == GLFW_PRESS)
        {
            rightMouseButtonPressed = true;

        }
        else if (action == GLFW_RELEASE)
        {
            rightMouseButtonPressed = false;
        }
    }
}

