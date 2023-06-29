#include <iostream>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>
#include <chrono>
#include <fstream>
#include <sstream>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"
#include "vec4.h"
#include "mat4.h"

#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 600
#define FLOAT32_BYTE_SIZE 4
#define STRIDE FLOAT32_BYTE_SIZE*4

void loop();

// Vertex shader source
const char* vertexShader = R"(

    attribute vec4 a_vertex; // vertex position 
    attribute vec3 a_colors; // vertex colors
    attribute vec2 a_texcoord; // texture coordinates

    uniform mat4 mvp;

    varying vec2 v_texcoord;
    varying vec3 v_colors;

    void main()
    {
        gl_Position = a_vertex * mvp;
        v_colors = a_colors;
        v_texcoord = a_texcoord;
    }
)";

// Fragment shader source
const char* fragmentShader = R"(
    precision mediump float;
    
    varying vec3 v_colors;
    varying vec2 v_texcoord;

    uniform sampler2D texture;

    void main()
    {
        gl_FragColor = texture2D(texture, v_texcoord) * vec4(v_colors, 1.0);
    }
)";

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
  -0.5, 0.5, 0.0,   // Top left
  -0.5, -0.5, 0.0,  // Bottom left
  0.5, -0.5, 0.0,   // Bottom right
  0.5, 0.5, 0.0     // Top right
};

const GLfloat colors[] = {
  1.0, 0.0, 0.0, 1.0,   // Red (Top left)
  0.0, 1.0, 0.0, 1.0,   // Green (Bottom left)
  0.0, 0.0, 1.0, 1.0,   // Blue (Bottom right)
  1.0, 1.0, 0.0, 1.0    // Yellow (Top right)
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

GLint a_vertex = 0;
GLint a_colors = 0;
GLint a_texcoord = 0;
GLuint quadProgram = 0;
GLFWwindow* window;
unsigned int texture;

int main()
{

    if (!glfwInit())
    {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    GLFWwindow* window = glfwCreateWindow(CANVAS_WIDTH, CANVAS_HEIGHT, "OpenGL Triangle", nullptr, nullptr);
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
    unsigned char *data = stbi_load("/assets/container.jpg", &width, &height, &nrChannels, 0);

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

    a_vertex = glGetAttribLocation(quadProgram, "a_vertex");
    a_colors = glGetAttribLocation(quadProgram, "a_colors");
    a_texcoord = glGetAttribLocation(quadProgram, "a_texcoord");

    glUseProgram(quadProgram);
    glUniform1i(glGetUniformLocation(quadProgram, "texture"), 0);

    glEnable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);

    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    emscripten_set_main_loop(loop, 0, 0);

    return 0;
}

void loop()
{
    glUseProgram(quadProgram);

    glClearColor( 0.1, 0.1, 0.2, 1 );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);

	// position attribute
    glVertexAttribPointer(a_vertex, 3, GL_FLOAT, GL_FALSE, 0, vertices);
    glEnableVertexAttribArray(a_vertex);
    
    // color attribute
    glVertexAttribPointer(a_colors, 3, GL_FLOAT, GL_FALSE, 0, colors);
    glEnableVertexAttribArray(a_colors);

    // Texture attribute
    glVertexAttribPointer(a_texcoord, 2, GL_FLOAT, GL_FALSE, 0, texturecoord);
    glEnableVertexAttribArray(a_texcoord);

	double deltaTime = 0;
	double lastTime = 0;

	double now = glfwGetTime();

	deltaTime = now - lastTime;
	lastTime = now;

    double angle = 0;

	angle += 30 * deltaTime;
    if (angle > 360)
        angle = angle - 360;

	mat4 model;
	model = scale(model, 1.2);
    model = roll(model, angle);
    model = rotate(model, -55, vec3(1, 0, 0));

    mat4 view = view_mat(vec3(0, 0, -3), vec3(0, 0, 0), vec3(0, 1, 0));
    mat4 proj = projection_mat(60, CANVAS_WIDTH, CANVAS_HEIGHT, 0.1, 100);

    mat4 mvp = proj * view * model;

    unsigned int mvpLoc = glGetUniformLocation(quadProgram, "mvp");

    std::vector<float> formattedMVP = mvp.toFloatVector();

	glUniformMatrix4fv(mvpLoc, 1, GL_FALSE, reinterpret_cast<GLfloat*>(formattedMVP.data()));

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}