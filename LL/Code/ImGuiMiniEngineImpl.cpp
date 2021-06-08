#include "imgui.h"

#include "Basics.h"
#include "ImGuiMiniEngineImpl.h"

#include "GraphicsCore.h"
#include "GraphicsCommon.h"
#include "BufferManager.h"
#include "CommandContext.h"
#include "RootSignature.h"
#include "PipelineState.h"
#include "UploadBuffer.h"
#include "Display.h"
#include "GameInput.h"

#include "CompiledShaders/ImGuiVS.h"
#include "CompiledShaders/ImGuiPS.h"

namespace imgui_miniengine
{
    struct frame_resources
    {
        UploadBuffer Vb, Ib;
    };

    __declspec(align(16)) struct imgui_constant_buffer
    {
        float MVP[4][4];
    };

    static int SwapChainBufferCount = 0;
    static frame_resources *FrameResources = nullptr;
    static unsigned int CurrentFrameResource = 0;

    static Texture FontTexture;
    static GraphicsPSO PSO;
    static RootSignature RootSig;

    //static DescriptorHeap TextureHeap;
    //static DescriptorHandle TextureDescriptorHandle;

    static void CreateIndexBuffers(unsigned long Size)
    {
        for(int I = 0; I < SwapChainBufferCount; ++I)
        {
            FrameResources[I].Ib.Create(std::wstring(L"ImGui_VertexBuffer") + std::to_wstring(I), Size);
        }
    }

    static void DestroyIndexBuffers()
    {
        for(int I = 0; I < SwapChainBufferCount; ++I)
        {
            FrameResources[I].Ib.Destroy();
        }
    }

    static void CreateVertexBuffers(unsigned long Size)
    {
        for(int I = 0; I < SwapChainBufferCount; ++I)
        {
            FrameResources[I].Vb.Create(std::wstring(L"ImGui_VertexBuffer") + std::to_wstring(I), Size);
        }
    }

    static void DestroyVertexBuffers()
    {
        for(int I = 0; I < SwapChainBufferCount; ++I)
        {
            FrameResources[I].Vb.Destroy();
        }
    }

    static size_t GetVertexBufferCapacity()
    {
        return FrameResources[CurrentFrameResource].Vb.GetBufferSize();
    }

    static size_t GetIndexBufferCapacity()
    {
        return FrameResources[CurrentFrameResource].Ib.GetBufferSize();
    }

    static void SetupRenderState(GraphicsContext &GfxCtx, frame_resources &Fr, ImDrawData &DrawData)
    {
        imgui_constant_buffer VertexCb;
        {
            float L = DrawData.DisplayPos.x;
            float R = DrawData.DisplayPos.x + DrawData.DisplaySize.x;
            float T = DrawData.DisplayPos.y;
            float B = DrawData.DisplayPos.y + DrawData.DisplaySize.y;
            float MVP[4][4] =
            {
                { 2.0f / (R - L),   0.0f,           0.0f,   0.0f },
                { 0.0f,         2.0f / (T - B),     0.0f,   0.0f },
                { 0.0f,         0.0f,           0.5f,   0.0f },
                { (R + L) / (L - R),  (T + B) / (B - T),    0.5f,   1.0f }
            };
            memcpy(VertexCb.MVP, MVP, sizeof(MVP));
        }

        D3D12_VIEWPORT Vp = {};
        Vp.Width = DrawData.DisplaySize.x;
        Vp.Height = DrawData.DisplaySize.y;
        Vp.MinDepth = 0.0f;
        Vp.MaxDepth = 1.0f;
        Vp.TopLeftX = 0.0f;
        Vp.TopLeftY = 0.0f;
        GfxCtx.SetViewport(Vp);

        D3D12_VERTEX_BUFFER_VIEW VtxBuffView = {};
        VtxBuffView.BufferLocation = Fr.Vb.GetGpuVirtualAddress();
        VtxBuffView.SizeInBytes = (UINT)Fr.Vb.GetBufferSize();
        VtxBuffView.StrideInBytes = sizeof(ImDrawVert);
        GfxCtx.SetVertexBuffer(0, VtxBuffView);

        D3D12_INDEX_BUFFER_VIEW IdxBuffView = {};
        IdxBuffView.BufferLocation = Fr.Ib.GetGpuVirtualAddress();
        IdxBuffView.SizeInBytes = (UINT)Fr.Ib.GetBufferSize();
        IdxBuffView.Format = sizeof(ImDrawIdx) == 2 ? DXGI_FORMAT_R16_UINT : DXGI_FORMAT_R32_UINT;
        GfxCtx.SetIndexBuffer(IdxBuffView);

        GfxCtx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        GfxCtx.SetPipelineState(PSO);
        GfxCtx.SetRootSignature(RootSig);
        GfxCtx.SetConstantArray(0, 16, &VertexCb);
        GfxCtx.SetBlendFactor(Color(0.0f, 0.0f, 0.0f, 0.0f));
    }

    static void RenderDrawData(GraphicsContext &Ctx, ImDrawData &DrawData)
    {
        if(DrawData.DisplaySize.x <= 0.0f || DrawData.DisplaySize.y <= 0.0f)
        {
            return;
        }

        frame_resources &CurrentFr = FrameResources[CurrentFrameResource];

        if(GetVertexBufferCapacity() < DrawData.TotalVtxCount)
        {
            DestroyVertexBuffers();
            CreateVertexBuffers(Math::AlignPowerOfTwo(DrawData.TotalVtxCount + MEGABYTE(2)));
        }

        if(GetIndexBufferCapacity() < DrawData.TotalIdxCount)
        {
            DestroyIndexBuffers();
            CreateIndexBuffers(Math::AlignPowerOfTwo(DrawData.TotalIdxCount + MEGABYTE(2)));
        }

        ImDrawVert *VtxDst = (ImDrawVert *)CurrentFr.Vb.Map();
        ImDrawIdx *IdxDst = (ImDrawIdx *)CurrentFr.Ib.Map();
        for(int I = 0; I < DrawData.CmdListsCount; ++I)
        {
            const ImDrawList *CmdList = DrawData.CmdLists[I];
            memcpy(VtxDst, CmdList->VtxBuffer.Data, CmdList->VtxBuffer.Size * sizeof(ImDrawVert));
            memcpy(IdxDst, CmdList->IdxBuffer.Data, CmdList->IdxBuffer.Size * sizeof(ImDrawIdx));
            VtxDst += CmdList->VtxBuffer.Size;
            IdxDst += CmdList->IdxBuffer.Size;
        }
        CurrentFr.Vb.Unmap();
        CurrentFr.Ib.Unmap();

        SetupRenderState(Ctx, CurrentFr, DrawData);

        int VtxOffset = 0;
        int IdxOffset = 0;

        ImVec2 ClipOffset = DrawData.DisplayPos;
        for(int N = 0; N < DrawData.CmdListsCount; ++N)
        {
            const ImDrawList *CmdList = DrawData.CmdLists[N];
            for(int I = 0; I < CmdList->CmdBuffer.Size; ++I)
            {
                const ImDrawCmd *Cmd = &CmdList->CmdBuffer[I];
                if(Cmd->UserCallback != NULL)
                {
                    if(Cmd->UserCallback == ImDrawCallback_ResetRenderState)
                    {
                        SetupRenderState(Ctx, CurrentFr, DrawData);
                    }
                    else
                    {
                        Cmd->UserCallback(CmdList, Cmd);
                    }
                }
                else
                {
                    const D3D12_RECT Rectangle =
                    {
                        (LONG)(Cmd->ClipRect.x - ClipOffset.x),
                        (LONG)(Cmd->ClipRect.y - ClipOffset.y),
                        (LONG)(Cmd->ClipRect.z - ClipOffset.x),
                        (LONG)(Cmd->ClipRect.w - ClipOffset.y)
                    };

                    if(Rectangle.right > Rectangle.left && Rectangle.bottom > Rectangle.top)
                    {
                        //Ctx.SetDescriptorTable(1, *(DescriptorHandle *)Cmd->TextureId);
                        Ctx.SetDynamicDescriptor(1, 0, *(D3D12_CPU_DESCRIPTOR_HANDLE *)Cmd->TextureId);
                        Ctx.SetScissor(Rectangle);
                        Ctx.DrawIndexedInstanced(Cmd->ElemCount, 1, Cmd->IdxOffset + IdxOffset, Cmd->VtxOffset + VtxOffset, 0);
                    }
                }
            }
            IdxOffset += CmdList->IdxBuffer.Size;
            VtxOffset += CmdList->VtxBuffer.Size;
        }
    }

    void Init()
    {
        ImGui::CreateContext();

        ImGuiIO &IO = ImGui::GetIO();

        IO.BackendPlatformName = "imgui_miniengine";
        IO.BackendRendererName = "imgui_miniengine_dx12";
        IO.BackendFlags |= ImGuiBackendFlags_HasMouseCursors;
        IO.BackendFlags |= ImGuiBackendFlags_HasSetMousePos;
        IO.BackendFlags |= ImGuiBackendFlags_RendererHasVtxOffset;
        IO.ImeWindowHandle = GameCore::g_hWnd;
        IO.MouseDrawCursor = true;

        // Sampler for imgui pixel shader texture
        SamplerDesc Sampler = {};
        {
            Sampler.Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
            Sampler.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            Sampler.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            Sampler.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
            Sampler.MipLODBias = 0.0f;
            Sampler.MaxAnisotropy = 0;
            Sampler.ComparisonFunc = D3D12_COMPARISON_FUNC_ALWAYS;
            Sampler.MinLOD = 0.0f;
            Sampler.MaxLOD = 0.0f;
        }

        // Create RootSignature
        RootSig.Reset(2, 1);
        RootSig[0].InitAsConstants(0, 16, D3D12_SHADER_VISIBILITY_VERTEX);
        RootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        RootSig.InitStaticSampler(0, Sampler, D3D12_SHADER_VISIBILITY_PIXEL);
        RootSig.Finalize(L"ImGui_RootSig",
            D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_HULL_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_DOMAIN_SHADER_ROOT_ACCESS |
            D3D12_ROOT_SIGNATURE_FLAG_DENY_GEOMETRY_SHADER_ROOT_ACCESS);

        // Create Pipeline State Object
        PSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        PSO.SetRootSignature(RootSig);
        PSO.SetSampleMask(UINT_MAX);
        PSO.SetRenderTargetFormat(Graphics::g_OverlayBuffer.GetFormat(), DXGI_FORMAT_UNKNOWN);;

        D3D12_INPUT_ELEMENT_DESC VertexLayout[] =
        {
            { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, pos), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT,   0, (UINT)IM_OFFSETOF(ImDrawVert, uv),  D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
            { "COLOR",    0, DXGI_FORMAT_R8G8B8A8_UNORM, 0, (UINT)IM_OFFSETOF(ImDrawVert, col), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
        };
        PSO.SetInputLayout(_countof(VertexLayout), VertexLayout);

        PSO.SetVertexShader(g_pImGuiVS, sizeof(g_pImGuiVS));
        PSO.SetPixelShader(g_pImGuiPS, sizeof(g_pImGuiPS));

        D3D12_BLEND_DESC BlendDesc = {};
        BlendDesc.AlphaToCoverageEnable = false;
        BlendDesc.RenderTarget[0].BlendEnable = true;
        BlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
        BlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
        BlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
        BlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
        BlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_INV_SRC_ALPHA;
        BlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
        BlendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;
        PSO.SetBlendState(BlendDesc);

        D3D12_RASTERIZER_DESC RasterizerDesc = {};
        RasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
        RasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
        RasterizerDesc.FrontCounterClockwise = FALSE;
        RasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
        RasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
        RasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
        RasterizerDesc.DepthClipEnable = true;
        RasterizerDesc.MultisampleEnable = FALSE;
        RasterizerDesc.AntialiasedLineEnable = FALSE;
        RasterizerDesc.ForcedSampleCount = 0;
        RasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;
        PSO.SetRasterizerState(RasterizerDesc);

        D3D12_DEPTH_STENCIL_DESC DepthStencilDesc;
        DepthStencilDesc.DepthEnable = false;
        DepthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
        DepthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        DepthStencilDesc.StencilEnable = false;
        DepthStencilDesc.FrontFace.StencilFailOp = D3D12_STENCIL_OP_KEEP;
        DepthStencilDesc.FrontFace.StencilDepthFailOp = D3D12_STENCIL_OP_KEEP;
        DepthStencilDesc.FrontFace.StencilPassOp = D3D12_STENCIL_OP_KEEP;
        DepthStencilDesc.FrontFace.StencilFunc = D3D12_COMPARISON_FUNC_ALWAYS;
        DepthStencilDesc.BackFace = DepthStencilDesc.FrontFace;
        PSO.SetDepthStencilState(DepthStencilDesc);

        PSO.Finalize();

        // Query SwapChain buffer count
        DXGI_SWAP_CHAIN_DESC1 SwapChainDesc;
        ASSERT_SUCCEEDED(Graphics::s_SwapChain1->GetDesc1(&SwapChainDesc));
        SwapChainBufferCount = SwapChainDesc.BufferCount;
        ASSERT(SwapChainBufferCount > 0);

        // Create frame_resources for each swapchain buffer
        FrameResources = new frame_resources[SwapChainBufferCount];
        CreateVertexBuffers(MEGABYTE(8));
        CreateIndexBuffers(MEGABYTE(8));

        // Create texture heap
        //TextureHeap.Create(L"ImGui_TextureHeap", D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, 8);
        //TextureDescriptorHandle = TextureHeap.Alloc(1);

        // Get font texture data from ImGui
        unsigned char *Pixels;
        int Width, Height;
        IO.Fonts->GetTexDataAsRGBA32(&Pixels, &Width, &Height);

        // Calculate texture row pitch
        size_t RowPitch;
        RowPitch = (Width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
        ASSERT(RowPitch == Width * 4); // Assumption

        FontTexture.Create2D(RowPitch, Width, Height, DXGI_FORMAT_R8G8B8A8_UNORM, Pixels);
        IO.Fonts->SetTexID((ImTextureID) &(FontTexture.GetSRV()));
    }

    void Shutdown()
    {
        RootSig.DestroyAll();
        PSO.DestroyAll();
        DestroyVertexBuffers();
        DestroyIndexBuffers();
        FontTexture.Destroy();

        ImGui::DestroyContext();
    }

    void NewFrame(float DeltaT)
    {
        ImGuiIO &IO = ImGui::GetIO();
        ASSERT(IO.Fonts->IsBuilt() && "Font atlas not built");

        //RECT Rectangle = {};
        //::GetClientRect(GameCore::g_hWnd, &Rectangle);
        IO.DisplaySize = ImVec2((float)Graphics::g_DisplayWidth, (float)Graphics::g_DisplayHeight); //ImVec2((float)(Rectangle.right - Rectangle.left), (float)(Rectangle.bottom - Rectangle.top));

        IO.DeltaTime = DeltaT;

        // Update mouse button state
        IO.MouseDown[0] = GameInput::IsPressed(GameInput::kMouse0);
        IO.MouseDown[1] = GameInput::IsPressed(GameInput::kMouse1);
        IO.MouseDown[2] = GameInput::IsPressed(GameInput::kMouse2);

        // Update mouse scroll state
        IO.MouseWheel = GameInput::GetTimeCorrectedAnalogInput(GameInput::kAnalogMouseScroll) * 30.0f;

        // Set mouse position if ImGui wants to
        if(IO.WantSetMousePos)
        {
            POINT Pos = { (int)IO.MousePos.x, (int)IO.MousePos.y };
            if(::ClientToScreen(GameCore::g_hWnd, &Pos))
            {
                ::SetCursorPos(Pos.x, Pos.y);
            }
        }

        // Query mouse position from OS and set ImGui mouse position
        //IO.MousePos = ImVec2(GameInput::GetAnalogInput(GameInput::kAnalogMouseX), GameInput::GetAnalogInput(GameInput::kAnalogMouseY));
        IO.MousePos = ImVec2(-FLT_MAX, -FLT_MAX);
        POINT Pos;
        if(HWND ActiveWindow = ::GetForegroundWindow())
        {
            if(ActiveWindow == GameCore::g_hWnd || ::IsChild(ActiveWindow, GameCore::g_hWnd))
            {
                if(::GetCursorPos(&Pos) && ::ScreenToClient(GameCore::g_hWnd, &Pos))
                {
                    IO.MousePos = ImVec2((float)Pos.x, (float)Pos.y);
                }
            }
        }

        // Update mouse cursor type
        if(!(IO.ConfigFlags & ImGuiConfigFlags_NoMouseCursorChange))
        {
            ImGuiMouseCursor ImGuiCursor = ImGui::GetMouseCursor();
            if(ImGuiCursor == ImGuiMouseCursor_None || IO.MouseDrawCursor)
            {
                ::SetCursor(NULL);
            }
            else
            {
                LPTSTR Win32Cursor = IDC_ARROW;
                switch(ImGuiCursor)
                {
                case ImGuiMouseCursor_Arrow:        Win32Cursor = IDC_ARROW; break;
                case ImGuiMouseCursor_TextInput:    Win32Cursor = IDC_IBEAM; break;
                case ImGuiMouseCursor_ResizeAll:    Win32Cursor = IDC_SIZEALL; break;
                case ImGuiMouseCursor_ResizeEW:     Win32Cursor = IDC_SIZEWE; break;
                case ImGuiMouseCursor_ResizeNS:     Win32Cursor = IDC_SIZENS; break;
                case ImGuiMouseCursor_ResizeNESW:   Win32Cursor = IDC_SIZENESW; break;
                case ImGuiMouseCursor_ResizeNWSE:   Win32Cursor = IDC_SIZENWSE; break;
                case ImGuiMouseCursor_Hand:         Win32Cursor = IDC_HAND; break;
                case ImGuiMouseCursor_NotAllowed:   Win32Cursor = IDC_NO; break;
                }
                ::SetCursor(::LoadCursor(NULL, Win32Cursor));
            }
        }
    }

    void RenderFrame(GraphicsContext &Ctx)
    {
        //ImGui::NewFrame();
        //ImGui::ShowDemoWindow();
        //ImGui::EndFrame();
        ImGui::Render();
        RenderDrawData(Ctx, *ImGui::GetDrawData());
        CurrentFrameResource = (CurrentFrameResource + 1) % SwapChainBufferCount;
    }
}