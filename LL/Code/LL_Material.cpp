#include "LL_Material.h"

#include "LL_Common.h"
#include "BufferManager.h"
#include "CommandContext.h"

#include "CompiledShaders/LitVS.h"
#include "CompiledShaders/LitPS.h"

namespace scene
{
    lit_material::lit_material(TextureRef AlbedoTexture_) 
        : AlbedoTexture(AlbedoTexture_)
    {
        RootSig.Reset(2, 1);
        RootSig[0].InitAsConstantBuffer(0, D3D12_SHADER_VISIBILITY_ALL);
        RootSig[1].InitAsDescriptorRange(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 0, 1, D3D12_SHADER_VISIBILITY_PIXEL);
        RootSig.InitStaticSampler(0, Graphics::SamplerLinearClampDesc, D3D12_SHADER_VISIBILITY_PIXEL);
        RootSig.Finalize(L"LitMaterialRootSig", D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

        D3D12_INPUT_ELEMENT_DESC InputElementDesc[] =
        {
            {"VS_POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"VS_TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
            {"VS_NORMAL", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
        };

        PSO.SetRootSignature(RootSig);
        PSO.SetRasterizerState(Graphics::RasterizerDefault);
        PSO.SetBlendState(Graphics::BlendTraditional);
        PSO.SetDepthStencilState(Graphics::DepthStateReadWrite);
        PSO.SetPrimitiveTopologyType(D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE);
        PSO.SetRenderTargetFormats(1, &Graphics::g_SceneColorBuffer.GetFormat(), Graphics::g_SceneDepthBuffer.GetFormat());
        PSO.SetInputLayout(ARRAY_COUNT(InputElementDesc), InputElementDesc);
        PSO.SetVertexShader(g_pLitVS, sizeof(g_pLitVS));
        PSO.SetPixelShader(g_pLitPS, sizeof(g_pLitPS));
        PSO.Finalize();
    }

    void lit_material::SetGfxState(graphics_context &Gfx)
    {
        Gfx.SetRootSignature(RootSig); // Reduntant??
        Gfx.SetPipelineState(PSO);
        Gfx.SetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        Gfx.SetDynamicDescriptor(1, 0, AlbedoTexture->GetSRV());
        // Update lit shader related constbuffers here
    }
}