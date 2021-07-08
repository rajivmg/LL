#pragma once

#include "LL_Common.h"
#include "LL_Material.h"

namespace scene
{
    struct meshvertex
    {
        f32 Position[3];
        f32 TexCoord[2];
        f32 Normal[3];
    };

    typedef u32 meshindex;

    struct meshnode
    {
        std::wstring Name;
        matrix4 Transform;
        u32 IndexCount;
        u32 StartIndexLocation;
        i32 BaseVertexLocation;
    };

    struct meshgroup
    {
        std::wstring Name;
        std::vector<meshnode> Nodes;
        structured_buffer VertexBuffer, IndexBuffer;

        void CreateRenderResources(void *Vertices, u64 VerticesSizeInBytes, void *Indices, u64 IndicesSizeInBytes);
    };

    typedef std::shared_ptr<meshgroup> meshgroup_ref;

    struct meshgroup_component
    {
        meshgroup_ref MeshGroup;
        material_ref Material;

        meshgroup_component(meshgroup_ref MeshGroup_, material_ref Material_);
        
        void Render(graphics_context &Gfx, camera &MainCamera);
    };

    scene::meshgroup_ref LoadObjAsMeshGroup(std::wstring const &Filename);
}