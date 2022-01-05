#pragma once

#include "Shader.h"

class Program
{
public:
    ~Program()
    {
        glDeleteProgram(m_Program);
    }
    
    bool Create(const std::string& vsPath, const std::string& fsPath)
    {
        Shader vs;
        if(!vs.Create(vsPath, GL_VERTEX_SHADER))
        {
            return false;
        }
        
        Shader fs;
        if(!fs.Create(fsPath, GL_FRAGMENT_SHADER))
        {
            return false;
        }
        
        m_Program = glCreateProgram();
        glAttachShader(m_Program, vs.m_Handle);
        glAttachShader(m_Program, fs.m_Handle);
        glLinkProgram(m_Program);
        
        int linked = 0;
        glGetProgramiv(m_Program, GL_LINK_STATUS, &linked);
        
        if(!linked)
        {
            char log[512] = {0};
            glGetProgramInfoLog(m_Program, 512, nullptr, log);
            
            std::cout << "Program link error: " << log << std::endl;
            return false;
        }
        
        glDetachShader(m_Program, vs.m_Handle);
        glDetachShader(m_Program, fs.m_Handle);
        
        return true;
    }
    
    void Bind()
    {
        glUseProgram(m_Program);
    }
    
    template<typename T>
    void Set(const T& object, const std::string& name);
    
    template<>
    void Set(const glm::mat4& object, const std::string& name)
    {
        glUniformMatrix4fv(glGetUniformLocation(m_Program, name.c_str()), 1, GL_FALSE, &object[0][0]);
    }
    
    template<>
    void Set(const GLint& object, const std::string& name)
    {
        glUniform1i(glGetUniformLocation(m_Program, name.c_str()), object);
    }
    
private:
    GLuint m_Program{0};
};
