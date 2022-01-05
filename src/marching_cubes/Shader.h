#pragma once

#include <string>
#include <sstream>
#include <fstream>

#include <SDL_opengl.h>

class Program;
class Shader
{
    friend class Program;
    
public:
    ~Shader()
    {
        glDeleteShader(m_Handle);
    }
    
    bool Create(const std::string& path, GLenum type)
    {
        std::ifstream file(path);
        if(!file.is_open())
        {
            return false;
        }
        
        std::stringstream stream;
        stream << file.rdbuf();
        
        const std::string content = stream.str();
        const GLchar* contentBuff = content.c_str();
        
        m_Handle = glCreateShader(type);
        glShaderSource(m_Handle, 1, &contentBuff, nullptr);
        glCompileShader(m_Handle);
        
        GLint compiled = 0;
        glGetShaderiv(m_Handle, GL_COMPILE_STATUS, &compiled);
        
        if(!compiled)
        {
            char log[512] = {0};
            glGetShaderInfoLog(m_Handle, 512, nullptr, log);
            
            std::cout << "Shader error: " << log << std::endl;
            return false;
        }
        
        return true;
    }
    
private:
    GLuint m_Handle{0};
};
