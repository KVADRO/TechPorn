#define GL_GLEXT_PROTOTYPES

#include <SDL.h>
#include <SDL_opengl.h>

#include <glm/gtc/matrix_transform.hpp>

#include <PerlinNoise.hpp>

#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <array>

#include "Program.h"
#include "Isosurface.h"

/*template<typename T>
struct Value2D
{
    T x;
    T y;
};*/

GLenum glCheckError_(const char *file, int line)
{
    GLenum errorCode;
    while ((errorCode = glGetError()) != GL_NO_ERROR)
    {
        std::string error;
        switch (errorCode)
        {
            case GL_INVALID_ENUM:                  error = "INVALID_ENUM"; break;
            case GL_INVALID_VALUE:                 error = "INVALID_VALUE"; break;
            case GL_INVALID_OPERATION:             error = "INVALID_OPERATION"; break;
            case GL_STACK_OVERFLOW:                error = "STACK_OVERFLOW"; break;
            case GL_STACK_UNDERFLOW:               error = "STACK_UNDERFLOW"; break;
            case GL_OUT_OF_MEMORY:                 error = "OUT_OF_MEMORY"; break;
            case GL_INVALID_FRAMEBUFFER_OPERATION: error = "INVALID_FRAMEBUFFER_OPERATION"; break;
        }
        std::cout << error << " | " << file << " (" << line << ")" << std::endl;
    }
    return errorCode;
}

#define glCheckError() glCheckError_(__FILE__, __LINE__)

class Camera
{
public:
    void Init(int w, int h)
    {
        const float aspect = static_cast<float>(w) / static_cast<float>(h);
        m_ProjectionMTX = glm::perspective(glm::radians(60.0f), aspect, 0.1f, 1000.0f);
    }
    
    void LookAt(const glm::vec3& pos, const glm::vec3& target)
    {
        m_ViewMTX = glm::lookAt(pos, target, glm::vec3{0.0f, 1.0f, 0.0f});
    }

    const glm::mat4& GetViewMTX() const { return m_ViewMTX; }
    const glm::mat4& GetProjectionMTX() const { return m_ProjectionMTX; }

private:
    glm::mat4 m_ViewMTX;
    glm::mat4 m_ProjectionMTX;
};

class Input
{
public:
    enum  MouseAxis
    {
        MouseX = 0,
        MouseY
    };
    
    enum Action
    {
        FORWARD,
        BACKWARD,
        LEFT,
        RIGHT
    };
    
public:
    void ProcessKeyEvent(SDL_Keycode code, bool pressed)
    {
        std::map<SDL_Keycode, Action> actions
        {
            {SDLK_w, FORWARD},
            {SDLK_s, BACKWARD},
            {SDLK_a, LEFT},
            {SDLK_d, RIGHT}
        };
        
        auto it = actions.find(code);
        if(it != actions.end())
        {
            m_Actions[it->second] = pressed;
        }
    }
    
    float GetMouseDelta(MouseAxis axis) const
    {
        return m_MouseDelta[axis];
    }
    
    bool IsActionActive(Action action)
    {
        //#TODO:
        return false;
    }
    
private:
    std::map<Action, bool> m_Actions;
    float m_MouseDelta[2] = {0.0f};
};

class Scene
{
public:
    bool Create(int w, int h)
    {
        m_Camera.Init(w, h);
        m_Camera.LookAt(glm::vec3{ 10.0f, 10.0f, 10.0f }, glm::vec3{ 0.0f, 0.0f, 0.0f });
        
#if 1
        ConstantNoiseProvider provider;
#else
        PerlinNoiseProvider provider;
#endif

        if(!m_Isosurface.Create(provider)
           || !m_IsoProgram.Create("iso_shader.vs", "iso_shader.fs")
           || !m_GridProgram.Create("grid_shader.vs", "grid_shader.fs"))
        {
            return false;
        }

        return true;
    }
    
    void SysUpdate(const SDL_Event& event)
    {
        switch(event.type)
        {
            case SDL_KEYDOWN:
            case SDL_KEYUP:
            {
                m_Input.ProcessKeyEvent(event.key.keysym.sym, event.type == SDL_KEYDOWN);
            }
                break;
                
            case SDL_MOUSEMOTION:
            {
                //#TODO:
            }
                break;
            
            default:
                break;
        }
    }
    
    void Update(float delta)
    {
        UpdateCamera(delta);
        
        static float yRot = 0.0f;
        yRot += 1.0f * delta;
        
        m_IsoProgram.Bind();
        m_IsoProgram.Set(glm::rotate(glm::mat4{1}, yRot, glm::vec3{0.0f, 1.0f, 0.0f}), "uMMtx");
        m_IsoProgram.Set(m_Camera.GetViewMTX(), "uVMtx");
        m_IsoProgram.Set(m_Camera.GetProjectionMTX(), "uPMtx");
        m_Isosurface.RenderIso();
        
        m_GridProgram.Bind();
        m_GridProgram.Set(glm::rotate(glm::mat4{1}, yRot, glm::vec3{0.0f, 1.0f, 0.0f}), "uMMtx");
        m_GridProgram.Set(m_Camera.GetViewMTX(), "uVMtx");
        m_GridProgram.Set(m_Camera.GetProjectionMTX(), "uPMtx");
        m_Isosurface.RenderGrid();
        
        glCheckError();
    }
    
    void UpdateCamera(float delta)
    {
        /*glm::vec3 forward = m_Camera.GetForward();
        glm::vec3 up = m_Camera.GetUp();
        glm::vec3 side = glm::normalize(glm::cross(forward, up));
        
        // #TODO: Rotate
        
        const float MoveSpeed = 1.0f;
        glm::vec3 displacement;
        
        if(m_Input.IsActionActive(Input::FORWARD))
        {
            displacement -= forward * MoveSpeed * delta;
        }
        else if(m_Input.IsActionActive(Input::BACKWARD))
        {
            displacement += forward * MoveSpeed * delta;
        }
        
        if(m_Input.IsActionActive(Input::LEFT))
        {
            displacement += side * MoveSpeed * delta;
        }
        else if(m_Input.IsActionActive(Input::RIGHT))
        {
            displacement -= side * MoveSpeed * delta;
        }
        
        displacement = glm::normalize(displacement);*/
    }
    
private:
    Input m_Input;
    Camera m_Camera;
    Isosurface m_Isosurface;
    
    Program m_IsoProgram;
    Program m_GridProgram;
};



int main(int argc, char * argv[])
{
    SDL_Init(SDL_INIT_VIDEO);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_RED_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_GREEN_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_BLUE_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_ALPHA_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);
    SDL_GL_SetAttribute(SDL_GL_STENCIL_SIZE, 8);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

    static const int width = 800;
    static const int height = 600;

    SDL_Window * window = SDL_CreateWindow(""
                                           , SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED
                                           , width, height
                                           , SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
    
    SDL_GLContext context = SDL_GL_CreateContext(window);
    std::cout << "GL_SHADING_LANGUAGE_VERSION: " << glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;
    
    const float FrameTime = 1.0f / 60.0f;
    float delta = 0.0f;
    
    Scene scene;
    if(scene.Create(width, height))
    {
        glClearColor(255, 255, 255, 255);
        
        //glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
        glDisable(GL_CULL_FACE);
        
        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        
        bool shouldQuit = false;
        while(!shouldQuit)
        {
            Uint64 start = SDL_GetTicks64();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            SDL_Event event;
            while(SDL_PollEvent(&event))
            {
                scene.SysUpdate(event);
                switch(event.type)
                {
                    case SDL_QUIT:
                        shouldQuit = true;
                        break;
                }
            }
            
            scene.Update(delta);
            
            SDL_GL_SwapWindow(window);
            SDL_Delay(1);
            
            float epleased = static_cast<float>(SDL_GetTicks64() - start) / 1000.0f;
            if (epleased < FrameTime)
            {
                SDL_Delay((FrameTime - epleased) * 1000.0f);
            }

            delta = static_cast<float>(SDL_GetTicks64() - start) / 1000.0f;
        }
    }
    
    SDL_GL_DeleteContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}
