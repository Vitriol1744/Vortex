//
// Created by Vitriol1744 on 05.09.2022.
//
#include <GLFW/glfw3.h>
#include "Vortex/vtpch.hpp"
#include "Engine.hpp"

#include "Vortex/Core/Events/EventSystem.hpp"
#include "Vortex/Core/Math/Math.hpp"

#include "Vortex/Graphics/API/IRendererAPI.hpp"
#include "Vortex/Graphics/API/IGraphicsPipeline.hpp"
#include "Vortex/Graphics/API/IFramebuffer.hpp"

#include "Vortex/Graphics/API/Vulkan/VulkanRendererAPI.hpp"
#include "Vortex/Graphics/API/ImGui_Backend.hpp"

#include "imgui.h"

using namespace Vortex;
using namespace Vortex::Graphics;

namespace Vortex
{
    using namespace Vortex::Graphics;

    Engine* Engine::instance = nullptr;

    Engine::Engine(std::vector<char*>& args)
    {
        VT_CORE_ASSERT_MSG(instance == nullptr, "'Engine' can only have one instance!");
        instance = this;
        this->args = args;
    }

    bool Engine::Initialize()
    {
        //NOTE: LoggingManager should always be initialized first, because every other component/subsystem is using it
        LoggingManager::GetInstance(); // Initialize Logging Manager
        VTCoreLogInfo("C++ Standard: {}", __cplusplus);
        VTCoreLogTrace("Initializing Vortex Engine...");
        app = CreateApplication(args);

        VTCoreLogTrace("Initializing RendererAPI...");
        IRendererAPI::Initialize(GraphicsAPI::eVulkan, app->GetName(), app->GetVersion());
        VTCoreLogTrace("RendererAPI Initialized!");
        VTCoreLogInfo("Graphics API: {}", ToString(IRendererAPI::GetGraphicsAPI()));

        app->GetWindow() = IWindow::CreateWindow(width, height, std::string(app->GetName()));
        //app->GetWindow2() = IWindow::CreateWindow(100, 100, std::string(app->GetName()));
        VTCoreLogInfo("Arguments: ");
        for (uint32 i = 0; i < args.size(); i++)
            VTCoreLogInfo("Arg[{}]: {}", i, args[i]);
        VTCoreLogInfo("");
        VTCoreLogTrace("Vortex Engine Initialized!");
        VTCoreLogInfo("Vortex Version: {}.{}.{}", vortexVersion_Major, vortexVersion_Minor, vortexVersion_Patch);

        return true;
    }
    void Engine::Shutdown()
    {
        delete app;
        VTCoreLogTrace("Shutting Down Vortex Engine...");
        IRendererAPI::Shutdown();
    }

    void Engine::Run()
    {
        running = true;

        VTCoreLogTrace("Initializing Application!");
        VTCoreLogInfo("Application Name: {}", app->GetName());
        uint32 app_Major = static_cast<uint32>(app->GetVersion().x);
        uint32 app_Minor = static_cast<uint32>(app->GetVersion().y);
        uint32 app_Patch = static_cast<uint32>(app->GetVersion().z);
        VTCoreLogInfo("Application Version: {}.{}.{}", app_Major, app_Minor, app_Patch);
        app->Initialize();

        Ref<IShader> shader = IShader::Create("assets/shaders/basic.vulkan.vert.spv", "assets/shaders/default.vulkan.frag.spv");
        GraphicsPipelineCreateInfo createInfo;
        VertexBufferLayout layout;
        layout.AddElement(ShaderDataType::eFloat2);
        layout.AddElement(ShaderDataType::eFloat3);
        createInfo.shader = shader;
        createInfo.layout = layout;
        Ref<IGraphicsPipeline> pipeline = IGraphicsPipeline::Create(createInfo, app->GetWindow());
        Ref<IFramebuffer> framebuffer = IFramebuffer::Create(pipeline);
        //TODO: Engine: Shutdown ImGui
        ImGui::Initialize(app->GetWindow());
        //Ref<IGraphicsPipeline> pipeline2 = IGraphicsPipeline::Create(createInfo, app->GetWindow2());
        //Ref<IFramebuffer> framebuffer2 = IFramebuffer::Create(pipeline2);

        struct Vertex
        {
            glm::vec2 pos;
            glm::vec3 color;
        };

        Vertex vertices[] =
        {
            {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}},
            {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}},
            {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
            {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}}
        };

        uint16 indices[] { 0, 1, 2, 2, 3, 0 };

        vertexBuffer = IVertexBuffer::Create(vertices, std::size(vertices) * sizeof(Vertex));
        indexBuffer = IIndexBuffer::Create(indices, std::size(indices) * sizeof(uint16));

        while (app->GetWindow()->IsOpen())
        {
            app->GetWindow()->PollEvents();
            Update();

            if (!app->GetWindow()->IsMinimized())
            {
                pipeline->Bind();
                framebuffer->Bind();
                Render();
                app->GetWindow()->Present();
            }

            EventSystem::GetInstance()->ProcessEvents();
        }
        vkDeviceWaitIdle(VulkanRendererAPI::GetDevice().GetLogicalDevice());
    }

    void Engine::Update()
    {
        app->Update();
    }
    void Engine::Render()
    {
        IRendererAPI::Clear();
        static Color color = { 0.0f, 0.0f, 1.0f, 1.0f };
        IRendererAPI::SetClearColor(color);

        IRendererAPI::BeginRenderPass();
        IRendererAPI::DrawIndexed(vertexBuffer, indexBuffer, 6);
        ImGui::Begin();
        ::ImGui::SliderFloat4("Clear Color", reinterpret_cast<float*>(&color), 0.0f, 1.0f);
        bool demoWindow = true;
        ::ImGui::ShowDemoWindow(&demoWindow);
        ImGui::End();
        IRendererAPI::EndRenderPass();

        ImGuiIO& io = ::ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_ViewportsEnable)
        {
            ::ImGui::UpdatePlatformWindows();
            ::ImGui::RenderPlatformWindowsDefault();
        }

        app->Render();
    }
}