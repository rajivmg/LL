#include "LL_Scene.h"

#include "Camera.h"

namespace scene
{
    entt::registry EntityRegistry;

    entt::entity CreateEntity()
    {
        return EntityRegistry.create();
    }

    void UpdateScene()
    {

    }

    void DrawScene(graphics_context &Gfx, camera &MainCamera)
    {
        auto MeshGroupRenderView = EntityRegistry.view<meshgroup_component>();

        for(auto Iter = MeshGroupRenderView.begin(), EndIter = MeshGroupRenderView.end();
            Iter != EndIter; ++Iter)
        {
            meshgroup_component &Comp = MeshGroupRenderView.get<meshgroup_component>(*Iter);
            
            Comp.Material->SetGfxState(Gfx);

            //__declspec(align(16)) struct cb0
            //{
            //    matrix4 ModelToProj;
            //} CB0;
            //CB0.ModelToProj = MainCamera.GetViewProjMatrix();
            //Gfx.SetDynamicConstantBufferView(0, sizeof(cb0), &CB0);

            Comp.Render(Gfx, MainCamera);
        }
    }
}