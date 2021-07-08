#pragma once

#include "LL_Common.h"

#include "cereal/types/string.hpp"
#include "cereal/types/vector.hpp"
#include "cereal/archives/json.hpp"

#include "entt/entt.hpp"

#include "LL_Mesh.h"
#include "LL_Material.h"

namespace scene
{
    using namespace Math;

    struct name_component
    {
        std::string Name = "";

        template <class archive>
        void Serialize(archive &Archive)
        {
            Archive(Name);
        }
    };

    struct transform_component
    {
        vector3 Translation = vector3(kIdentity);
        quat Rotation = quat(kIdentity);
        vector3 Scale = vector3(kIdentity);

        template <class archive>
        void Serialize(archive& Archive)
        {
            Archive(Translation.m_vec, Rotation.m_vec, Scale.m_vec);
        }
    };
    
    void DrawScene(graphics_context &Gfx, camera &MainCamera);

    extern entt::registry EntityRegistry;

    entt::entity CreateEntity();

    template <typename t, typename ...args>
    inline t& EntityEmplaceComponent(entt::entity Entity, args &&...Args)
    {
        return EntityRegistry.emplace<t>(Entity, std::forward<args>(Args)...);
    }

    template <typename t, typename ...args>
    inline t& EntityEmplaceOrReplaceComponent(entt::entity Entity, args &&...Args)
    {
        return EntityRegistry.emplace_or_replace<t>(Entity, std::forward<args>(Args)...);
    }

    template <typename t, typename ...args>
    inline t& EntityGetOrEmplaceComponent(entt::entity Entity, args &&...Args)
    {
        return EntityRegistry.get_or_emplace<t>(Entity, std::forward<args>(Args)...);
    }
}
