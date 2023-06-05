#include <iostream>
#include <GLES3/gl3.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 600
#define FLOAT32_BYTE_SIZE 4
#define STRIDE FLOAT32_BYTE_SIZE*4

void loop();

// Vertex shader source
const char* vertexShader = R"(
    //TODO add MVP mat
    attribute vec4 a_vertex; // vertex position 
    attribute vec3 a_colors; // vertex colors
    attribute vec2 a_texcoord; // texture coordinates

    varying vec2 v_texcoords;
    varying vec3 v_colors;

    void main()
    {
        gl_Position = a_vertex;
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
        gl_FragColor = texture(texture, v_textcoord) * vec4(v_colors, 1.0);
    }
)";

GLuint loadShader(GLenum shaderType, const char* shaderSource)
{
    GLuint shader = glCreateShader(shaderType);
    if (shader) 
    {
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
    return shader;
}

GLuint createProgram(const char* vertexSource, const char* fragmentSource)
{
   GLuint vertexShader = loadShader(GL_VERTEX_SHADER, vertexSource);
    if (!vertexShader)
    {
        return 0;
    }
    GLuint fragmentShader = loadShader(GL_FRAGMENT_SHADER, fragmentSource);
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

float vertices[] = {
    // positions          // colors           // texture coords
     0.5f,  0.5f, 0.0f,   1.0f, 0.0f, 0.0f,   1.0f, 1.0f,   // top right
     0.5f, -0.5f, 0.0f,   0.0f, 1.0f, 0.0f,   1.0f, 0.0f,   // bottom right
    -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, 1.0f,   0.0f, 0.0f,   // bottom left
    -0.5f,  0.5f, 0.0f,   1.0f, 1.0f, 0.0f,   0.0f, 1.0f    // top left 
};

GLuint quadProgram = 0;
GLint vPosition = 0;
GLint colors = 0;
GLint texCoord = 0;
GLFWwindow* window;


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

    int width, height, nrChannels;
    unsigned char *data = stbi_load("/assets/container.jpg", &width, &height, &nrChannels, 0);

    unsigned int texture;
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // set the texture wrapping/filtering options (on the currently bound texture object)
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (data) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);
    }
    else {
        std::cout << "Failed to load texture file" << std::endl;
    }

    stbi_image_free(data);

    quadProgram = createProgram(vertexShader, fragmentShader);
    vPosition = glGetAttribLocation(quadProgram, "vPosition");

    glUseProgram(quadProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    emscripten_set_main_loop(loop, 0, 0);
    return 0;
}

void loop()
{
    glUseProgram(quadProgram);

    glClearColor( 0.4, 0.3, 0.2, 1 );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(GLfloat), (void*)(6 * sizeof(GLfloat)));
    glEnableVertexAttribArray(2);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}