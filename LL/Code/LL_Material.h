#pragma once

#include "LL_Common.h"

namespace scene
{
    struct material_base
    {
        rootsignature RootSig;
        graphics_pso PSO;

        virtual ~material_base() {}

        virtual void Initialize() {};
        virtual void SetGfxState(graphics_context &Gfx) = 0;
    };

    typedef std::shared_ptr<material_base> material_ref;

#define CreateMaterialInstance(type, ...) std::make_shared<type>(__VA_ARGS__) 

    struct lit_material : material_base
    {
        TextureRef AlbedoTexture;

        lit_material(TextureRef AlbedoTexture_);
        void SetGfxState(graphics_context &Gfx) override;
    };
}
