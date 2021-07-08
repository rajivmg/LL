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

#include "MotionBlur.h"
#include "PostEffects.h"
#include "TemporalEffects.h"
#include "FXAA.h"
#include "SSAO.h"

#include "Camera.h"

//
#include "LL_Common.h"
#include "ImGuiMiniEngineImpl.h"
#include "ImGuizmo.h"
#include "LL_Scene.h"
#include "LL_MeshExporter.h"
#include "LL_Utility.h"
//

using namespace GameCore;
using namespace Graphics;
using namespace Math;

class ll : public GameCore::IGameApp
{
public:
    ll() {}

    virtual void Startup() override;
    virtual void Cleanup() override;

    virtual void Update(float DeltaT) override;
    virtual void RenderScene() override;
    virtual void RenderUI(graphics_context& GfxCtx) override;

private:
    camera MainCamera;
    cameracontroller *MainCameraController;

    entt::entity SpotCowEntity;
    //
    bool ShowGeomToCppTool;
    //
    TextureRef SpotCowTexture;
    scene::material_ref LitMaterial;
    scene::meshgroup_ref SpotCowMeshGroup;
};

CREATE_APPLICATION(ll)

void ll::Startup()
{
    Display::Resize(1600, 900);
    SetWindowPos(GameCore::g_hWnd, 0, 0, 0, g_DisplayWidth, g_DisplayHeight, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);

    // Turn off post effects
    FXAA::Enable = false;
    SSAO::Enable = false;
    MotionBlur::Enable = false;
    PostEffects::EnableHDR = false;
    TemporalEffects::EnableTAA = false;
    PostEffects::EnableAdaptation = false;
    PostEffects::BloomEnable = false;

    imgui_miniengine::Init();

    // Initialize variables
    ShowGeomToCppTool = false;

    MainCamera.SetEyeAtUp(Vector3(0.0f, 0.0f, 15.0f), Vector3(0.0f, 0.0f, 0.0f), Vector3(0.0f, 1.0f, 0.0f));
    MainCamera.SetZRange(1.0f, 1000.0f);
    MainCamera.SetFOV(XM_PIDIV4);
    MainCameraController = new OrbitCamera(MainCamera, Math::BoundingSphere(0, 0, 0, 1), Vector3(kYUnitVector));

    SpotCowTexture = TextureManager::LoadDDSFromFile(L"Content/SpotCow_Albedo.dds");
    LitMaterial = CreateMaterialInstance(scene::lit_material, SpotCowTexture);

    SpotCowEntity = scene::CreateEntity();
    SpotCowMeshGroup = scene::LoadObjAsMeshGroup(L"SpotCow.obj");
    scene::meshgroup_component &MeshGroupComponent = scene::EntityEmplaceComponent<scene::meshgroup_component>(SpotCowEntity, SpotCowMeshGroup, LitMaterial);
}

void ll::Cleanup()
{
    imgui_miniengine::Shutdown();
}

void ll::Update(float DeltaT)
{
    ScopedTimer _prof(L"Update State");

    imgui_miniengine::NewFrame(DeltaT);
    ImGui::NewFrame();
    ImGuizmo::BeginFrame();
    /// ImGui Frame Begin - DO NOT call ImGui functions before this line
    MainCameraController->Update(DeltaT);

    // LL Menu
    ImGui::Begin("LL", NULL, ImGuiWindowFlags_MenuBar);
    if(ImGui::BeginMenuBar())
    {
        if(ImGui::BeginMenu("Tools"))
        {
            if(ImGui::MenuItem("MeshExporter", NULL, &ShowGeomToCppTool))
            {
                meshexporter::ResetValues();
            }
            ImGui::EndMenu();
        }
        ImGui::EndMenuBar();
    }
    ImGui::End();

    if(ShowGeomToCppTool)
    {
        meshexporter::ShowWindow(&ShowGeomToCppTool);
    }

    // Update something
    //colladautil::Update(true);
    MainCameraController->Update(DeltaT);
}

void ll::RenderScene()
{
    graphics_context &Gfx = GraphicsContext::Begin(L"Scene Render");

    Gfx.TransitionResource(g_SceneColorBuffer, D3D12_RESOURCE_STATE_RENDER_TARGET, true);
    Gfx.ClearColor(g_SceneColorBuffer);
    Gfx.TransitionResource(g_SceneDepthBuffer, D3D12_RESOURCE_STATE_DEPTH_WRITE, true);
    Gfx.ClearDepthAndStencil(g_SceneDepthBuffer);
    Gfx.SetRenderTarget(g_SceneColorBuffer.GetRTV(), g_SceneDepthBuffer.GetDSV());
    Gfx.SetViewportAndScissor(0, 0, g_SceneColorBuffer.GetWidth(), g_SceneColorBuffer.GetHeight());

    // Rendering something
    scene::DrawScene(Gfx, MainCamera);

    Gfx.Finish();
}

void ll::RenderUI(graphics_context& GfxCtx)
{
    ImGui::ShowDemoWindow();

    /// ImGui Frame End - DO NOT call ImGui functions after this line
    ImGui::EndFrame();
    imgui_miniengine::RenderFrame(GfxCtx);
}