#include "pch.h"
#include "GameCore.h"
#include "GraphicsCore.h"
#include "SystemTime.h"
#include "TextRenderer.h"
#include "GameInput.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "BufferManager.h"

//
#include "Basics.h"
#include "ImGuiMiniEngineImpl.h"
//

using namespace GameCore;
using namespace Graphics;

class LL : public GameCore::IGameApp
{
public:

    LL()
    {
    }

    virtual void Startup() override;
    virtual void Cleanup() override;

    virtual void Update(float DeltaT) override;
    virtual void RenderScene() override;
    virtual void RenderUI(GraphicsContext& GfxCtx) override;

private:
};

CREATE_APPLICATION( LL )

void LL::Startup()
{
    Display::Resize(1600, 900);
    SetWindowPos(GameCore::g_hWnd, 0, 0, 0, g_DisplayWidth, g_DisplayHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    imgui_miniengine::Init();
}

void LL::Cleanup()
{
    imgui_miniengine::Shutdown();
}

void LL::Update(float DeltaT)
{
    ScopedTimer _prof(L"Update State");

    imgui_miniengine::NewFrame(DeltaT);
    ImGui::NewFrame();
    /// ImGui Frame Begin - DO NOT call ImGui functions before this line

    // Update something

}

void LL::RenderScene()
{
    GraphicsContext& gfxContext = GraphicsContext::Begin(L"Scene Render");

    gfxContext.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    gfxContext.ClearColor(g_SceneColorBuffer);
    gfxContext.SetRenderTarget(g_SceneColorBuffer.GetRTV());
    gfxContext.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    // Rendering something

    gfxContext.Finish();
}

void LL::RenderUI(GraphicsContext& GfxCtx)
{
    ImGui::ShowDemoWindow();

    /// ImGui Frame End - DO NOT call ImGui functions after this line
    ImGui::EndFrame();
    imgui_miniengine::RenderFrame(GfxCtx);
}