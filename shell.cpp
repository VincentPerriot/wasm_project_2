#include <iostream>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <emscripten/html5.h>
#include <chrono>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vec4.h"
#include "mat4.h"
#include "camera.h"
#include "./assets/vertices.h"

#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 600

void loop();
void processInput(GLFWwindow* window, double deltaTime);
void processMouse(GLFWwindow* window, double xposIn, double yposIn);
unsigned int loadCubeMap(std::vector<std::string> faces);


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
  -0.5, 0.5, -2.0,   // Top left
  -0.5, -0.5, -2.0,  // Bottom left
  0.5, -0.5, -2.0,   // Bottom right
  0.5, 0.5, -2.0     // Top right
};

const GLfloat b_vertices[] = { 
    -2.0, 2.0, -1.0,
    -2.0, -2.0, -1.0,
    2.0, -2.0, -1.0,
    2.0, 2.0, -1.0
};

const GLfloat colors[] = {
  0.694, 0.784, 0.949,   // Blue (Top left)
  0.694, 0.784, 0.949,    // Blue (Bottom left)
  0.694, 0.784, 0.949,   // Blue (Bottom right)
  0.694, 0.784, 0.949,    // Blue (Top right)
};

const GLfloat texturecoord[] = {
	0.0, 1.0,   // Top left
	0.0, 0.0,   // Bottom left
	1.0, 0.0,   // Bottom Right
	1.0, 1.0    // Top Right
};

const GLushort indices[] = {
	0, 1, 2, // first triangle
	0, 2, 3  // second triangle
};

const GLushort b_indices[] = {
	0, 1, 2, // first triangle
	0, 2, 3  // second triangle
};

GLuint quadProgram = 0;
GLuint fractalProgram = 0;
GLuint skyboxProgram = 0;
GLuint skyboxVAO, skyboxVBO;
GLFWwindow* window;

// Camera Set up
Camera camera;
double lastX = CANVAS_WIDTH / 2.0;
double lastY = CANVAS_HEIGHT / 2.0;
double xpos = 0;
double ypos = 0;
bool firstMouse = true;

// Process Time
double deltaTime = 0;
double lastTime = 0;

// Textures init
unsigned int texture;
unsigned int cubemapTexture;

int main()
{

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    window = glfwCreateWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "WebGl wndow", nullptr, nullptr);
    if (!window)
    {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    // Make the created window the current context
    glfwMakeContextCurrent(window);

    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);
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

    quadProgram = createProgram("/shaders/shader.vert", "/shaders/shader.frag");

    glUseProgram(quadProgram);

    fractalProgram = createProgram("/shaders/shader_2.vert", "/shaders/shader_2.frag");
    glUseProgram(fractalProgram);
    
    /*
	// Skybox program
    skyboxProgram = createProgram("/shaders/skybox.vert", "/shaders/skybox.frag");
    // As this is more than just 2 trianlges we create VAO / VBO
    glGenVertexArrays(1, &skyboxVAO);
    glGenBuffers(1, &skyboxVBO);
    glBindVertexArray(skyboxVAO);
    glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    
    unsigned int VAOLoc = glGetAttribLocation(skyboxProgram, "c_vertex");
    glEnableVertexAttribArray(VAOLoc);
    glVertexAttribPointer(VAOLoc, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), skyboxVertices);

	std::vector<std::string> faces {
		"/assets/skybox/right.jpg",
		"/assets/skybox/left.jpg",
		"/assets/skybox/top.jpg",
		"/assets/skybox/bottom.jpg",
		"/assets/skybox/front.jpg",
		"/assets/skybox/back.jpg"
	};

    cubemapTexture = loadCubeMap(faces);

    glUseProgram(skyboxProgram);
    //glUniform1i(glGetUniformLocation(skyboxProgram, "skybox"), 0);
    */

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    emscripten_set_main_loop(loop, 0, 1);

    glDeleteVertexArrays(1, &skyboxVAO);
    glDeleteBuffers(1, &skyboxVBO);

    glfwTerminate();
    return 0;
}

void loop()
{
	double now = glfwGetTime();

	deltaTime = now - lastTime;
	lastTime = now;

    // Process Inputs
    glfwGetCursorPos(window, &xpos, &ypos);
    processInput(window, deltaTime);
    processMouse(window, xpos, ypos);

    // Change angle with time
    double angle = 0;

	angle += 30 * deltaTime;
    if (angle > 360)
        angle = angle - 360;

    // Build mvp
	mat4 model;
    model = translate(model, vec3(0, 37, 0));
    model = pitch(model, -90);
	model = scale(model, 20);

    mat4 view = camera.GetViewMatrix();
    mat4 proj = projection_mat(60, CANVAS_WIDTH, CANVAS_HEIGHT, 0.1, 100);

    mat4 mvp = proj * view * model;

    // Begin Quad Render program
    glUseProgram(quadProgram);

    glClearColor( 0.1, 0.1, 0.2, 1 );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

	// position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(0);

    // color attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(1);

    // Texture attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, texturecoord);
    glEnableVertexAttribArray(2);

    unsigned int mvpLoc = glGetUniformLocation(quadProgram, "mvp");
    std::vector<float> formattedMVP = mvp.toFloatVector();
	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedMVP.data()));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);


    // Begin Fractal program
    glUseProgram(fractalProgram);
	// Shaders2 position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, b_vertices);
    glEnableVertexAttribArray(0);

    mat4 model2;
    mat4 mvp2 = proj * view * model2;

    unsigned int mvpLoc2 = glGetUniformLocation(fractalProgram, "mvp");
    unsigned int timeLoc = glGetUniformLocation(fractalProgram, "time");

    std::vector<float> formattedMVP2 = mvp2.toFloatVector();
	glUniformMatrix4fv(mvpLoc2, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedMVP2.data()));

    glUniform1f(timeLoc, static_cast<GLfloat>(now));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, b_indices);
    
    /*
    // Begin skybox program
    glDepthFunc(GL_LEQUAL);
    glUseProgram(skyboxProgram);

    mat4 model3;
    mat4 mvp3 = proj * view * model3;
    
    unsigned int mvpLoc3 = glGetUniformLocation(skyboxProgram, "mvp");
    std::vector<float> formattedMVP3 = mvp3.toFloatVector();
	glUniformMatrix4fv(mvpLoc3, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedMVP3.data()));
    

   // skybox cube
   glBindVertexArray(skyboxVAO);
   //glActiveTexture(GL_TEXTURE0);
   //glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
   glDrawArrays(GL_TRIANGLES, 0, 36);
   glBindVertexArray(0);
 
   glDepthFunc(GL_LESS); // set depth function back to default
    */
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

unsigned int loadCubeMap(std::vector<std::string> faces)
{
    unsigned int textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    int width, height, nrChannels;
    for (int i = 0; i < faces.size(); i++)
    {
        unsigned char* skybox_data = stbi_load(faces[i].c_str(), &width, &height, &nrChannels, 0);
        if (skybox_data)
        {
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, GL_RGB, width, height,
                0, GL_RGB, GL_UNSIGNED_BYTE, skybox_data);
            stbi_image_free(skybox_data);
        }
        else
        {
            std::cout << "Failed to load cubemap at: " << faces[i] << std::endl;
            stbi_image_free(skybox_data);
        }
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return textureID;
}
