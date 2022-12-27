#include "vendors/GLFW/glfw3.h"
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
#include <alloca.h>
#include <OpenGL/gl3.h>
#include <fstream>
#include <sstream>
#include <csignal>

#define GLCall(x) GLClearError();\
    x;\
    ASSERT(GLLogCall(#x, __FILE__, __LINE__))
#define ASSERT(x) if (!(x)) raise(SIGTRAP);


static void GLClearError(){
    while (glGetError() != GL_NO_ERROR);

}

static bool GLLogCall(const char* function, const char*file, int line){
    while(GLenum error = glGetError()){
        std::cout<<"[OpenGL Error]: ("<<error<<"): "<<function<<" "<<file<<" "<<line<<std::endl;
        return false;
    };
    return true;
}

struct ShaderProgramSource{
    std::string VertexSource;
    std::string FragmentSource;
};

static ShaderProgramSource ParseShader(const std::string& filepath){

    std::cout<<std::filesystem::current_path()<<std::endl;
    std::ifstream stream(filepath);

    enum class ShaderType{
        NONE = -1, VERTEX = 0, FRAGMENT =1
    };
    std::string line;
    std::stringstream ss[2];
    ShaderType type= ShaderType::NONE;
    while (getline(stream, line)){
        std::cout<<line<<std::endl;
        if (line.find("#shader") != std::string::npos){
            if (line.find("vertex") != std::string::npos){
                type = ShaderType::VERTEX;
            } else if (line.find("fragment") != std::string::npos){
                type = ShaderType::FRAGMENT;
            }
        } else {
            ss[(int) type ] << line<<'\n';
        }
    }
    return {ss[0].str(), ss[1].str()};
};


static unsigned int ComplileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    // if null is passed in as length, each string is assumed to be null terminated (see docs) 
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;

    // returns a parameter from shader object, in this case we return the status of compilation 
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE){
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        // alloca is a dynamic stack allocation function
        char* message = (char*)alloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message);
        std::cout<<"FAILED TO COMPILE SHADER OF TYPE "<< (type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT")<<std::endl;
        std::cout<<message<<std::endl;
        glDeleteShader(id);
        return 0;
    }

    return id;
};

// this will link our frag and vert shaders, and give back the id of the linked shader
static unsigned int CreateShader(const std::string& vertexShader, const std::string& fragmentShader){
    unsigned int program = glCreateProgram();
    unsigned int vs = ComplileShader(GL_VERTEX_SHADER, vertexShader); 
    unsigned int fs = ComplileShader(GL_FRAGMENT_SHADER, fragmentShader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program); 
    
    // we do this because the shaders have already been compiled and linked/are no longer needed
    glDeleteShader(fs);
    glDeleteShader(vs);

    return program;

}




int main(void)
{
    GLFWwindow *window;

    /* Initialize the library */
    if (!glfwInit())
        return -1;


    // Mac stuff
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);

    /* Create a windowed mode window and its OpenGL context */
    window = glfwCreateWindow(640, 480, "Yaya", NULL, NULL);
    if (!window)
    {
        glfwTerminate();
        return -1;
    }

    /* Make the window's context current */
    GLCall(glfwMakeContextCurrent(window));


    unsigned int buffer_id;
    unsigned int vao;
    GLCall(glGenBuffers(1, &buffer_id));
    GLCall(glGenVertexArrays(1, &vao));
    GLCall(glBindVertexArray(vao));

    // Select/bind the buffer
    GLCall(glBindBuffer(GL_ARRAY_BUFFER, buffer_id));

    //specify buffer size
    std::cout << glGetString(GL_VERSION) << std::endl;

    float positions[] = {
        -0.5f, -0.5f,
        0.5f, -0.5f,
        0.5f, 0.5f,

        // 0.5f, 0.5f,
        -0.5f, 0.5f,
        // -0.5f, -0.5f,
    };
    
    // this data tells open GL which vertices to use
    unsigned int indices[] = {
        0,1,2,
        2,3,0
    };

    GLCall(glBufferData(GL_ARRAY_BUFFER, 6 * 2 * sizeof(float), positions, GL_STATIC_DRAW));
    GLCall(glEnableVertexAttribArray(0));

    // stride is the width of the whole vertex, offset is the offset into each vertex
    GLCall(glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), 0));

    unsigned int ibo;
    GLCall(glGenBuffers(1, &ibo));
    GLCall(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo));
    GLCall(glBufferData(GL_ELEMENT_ARRAY_BUFFER, 6*sizeof(unsigned int), indices, GL_STATIC_DRAW));


    ShaderProgramSource source = ParseShader("res/shaders/shader.shader");
    unsigned int shader = CreateShader(source.VertexSource, source.FragmentSource);
    GLCall(glUseProgram(shader));





    while (!glfwWindowShouldClose(window))
    {
        /* Render here */
        GLCall(glClear(GL_COLOR_BUFFER_BIT));


        // draws currently bound buffer
        GLCall(glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr));
        /* Swap front and back buffers */
        GLCall(glfwSwapBuffers(window));

        /* Poll for and process events */
        GLCall(glfwPollEvents());
    }
    glDeleteProgram(shader);

    glfwTerminate();
    return 0;
}