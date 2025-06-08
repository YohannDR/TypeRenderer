#include <cstdlib>
#include <iostream>

#include "glad.h"
#include "macros.hpp"
#include "type_renderer.hpp"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "imgui/backends/imgui_impl_glfw.h"
#include "imgui/backends/imgui_impl_opengl3.h"

#include "examples/base_types.hpp"
#include "examples/callbacks.hpp"
#include "examples/containers.hpp"
#include "examples/custom.hpp"
#include "examples/enums.hpp"
#include "examples/nesting.hpp"
#include "examples/range.hpp"
#include "examples/tooltip.hpp"

namespace
{
    GLFWwindow* window;

    bool_t Setup()
    {
        if (!glfwInit())
        {
            std::cout << "Failed to initialize glfw" << '\n';
            return false;
        }

        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        window = glfwCreateWindow(1000, 800, "Type renderer test", nullptr, nullptr);

        glfwMakeContextCurrent(window);
        glfwShowWindow(window);

        gladLoadGL();

        glfwSwapInterval(1); // Enable vsync

        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        ImGuiIO& io = ImGui::GetIO();
        io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard; // Enable Keyboard Controls
        io.Fonts->AddFontDefault();

        ImGui::StyleColorsDark();
        ImGui_ImplGlfw_InitForOpenGL(window, true);
        ImGui_ImplOpenGL3_Init("#version 460");

        return true;
    }

    void PreLoop()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
    }

    void PostLoop()
    {
        ImGui::Render();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        GLFWwindow* ctxBackup = glfwGetCurrentContext();
        glfwMakeContextCurrent(ctxBackup);

        glfwPollEvents();
        glfwSwapBuffers(window);
    }

    void Shutdown()
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplGlfw_Shutdown();
        ImGui::DestroyContext();
        
        glfwDestroyWindow(window);
        glfwTerminate();
    }
}

int main(int, char*[])
{
    if (!Setup())
        return EXIT_FAILURE;

    MAYBE_UNUSED BaseTypesExample baseTypesExample;
    MAYBE_UNUSED RangeExample rangeExample;
    MAYBE_UNUSED EnumExample enumExample;
    MAYBE_UNUSED CallbacksExample callbacksExample;
    MAYBE_UNUSED ContainersExample containersExample;
    MAYBE_UNUSED TooltipExample tooltipExample;
    MAYBE_UNUSED CustomExample customExample;
    MAYBE_UNUSED NestingExample nestingExample;

    while (!glfwWindowShouldClose(window))
    {
        PreLoop();

        TypeRenderer::RenderType(&baseTypesExample, true);
        TypeRenderer::RenderType(&rangeExample, true);
        TypeRenderer::RenderType(&enumExample, true);
        TypeRenderer::RenderType(&callbacksExample, true);
        TypeRenderer::RenderType(&containersExample, true);
        TypeRenderer::RenderType(&tooltipExample, true);
        TypeRenderer::RenderType(&customExample, true);
        TypeRenderer::RenderType(&nestingExample, true);

        PostLoop();
    }

    Shutdown();
    
    return 0;
}
