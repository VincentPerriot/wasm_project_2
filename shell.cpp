#include <iostream>
#include <GLES2/gl2.h>
#include <GLFW/glfw3.h>
#include <emscripten.h>


#define CANVAS_WIDTH 800
#define CANVAS_HEIGHT 600
#define FLOAT32_BYTE_SIZE 4
#define STRIDE FLOAT32_BYTE_SIZE*4

void loop();

// Vertex shader source
const char* vertexShader = R"(
    attribute vec4 vPosition;

    void main()
    {
        gl_Position = vPosition;
    }
)";

// Fragment shader source
const char* fragmentShader = R"(
    precision mediump float;

    void main()
    {
        gl_FragColor = vec4(0.8, 0.6, 0.4, 1.0);
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

// Vertex data
const GLfloat vertices[] = {
        0.0, 0.5,
        -0.5, -0.5,
        0.5, -0.5
};

GLuint triangleProgram = 0;
GLint vPosition = 0;
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

    triangleProgram = createProgram(vertexShader, fragmentShader);
    vPosition = glGetAttribLocation(triangleProgram, "vPosition");

    glUseProgram(triangleProgram);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    emscripten_set_main_loop(loop, 0, 0);
    return 0;
}

void loop()
{
    glUseProgram(triangleProgram);

    glClearColor( 0.4, 0.3, 0.6, 1 );
    glClear( GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT );

    glVertexAttribPointer(vPosition, 2, GL_FLOAT, GL_FALSE, 0 , vertices);
    glEnableVertexAttribArray(vPosition);

    glDrawArrays(GL_TRIANGLES, 0, 3);
}