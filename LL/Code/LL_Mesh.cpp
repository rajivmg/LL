#include "LL_Mesh.h"
#include "LL_Scene.h"
#include "LL_Common.h"
#include "Camera.h"

namespace scene
{
    void meshgroup::CreateRenderResources(void *Vertices, u64 VerticesSizeInBytes, void *Indices, u64 IndicesSizeInBytes)
    {
        VertexBuffer.Create(L"meshgroup::VertexBuffer", VerticesSizeInBytes, 1, Vertices);
        IndexBuffer.Create(L"meshgroup::IndexBuffer", IndicesSizeInBytes, 1, Indices);
    }

    meshgroup_component::meshgroup_component(meshgroup_ref MeshGroup_, material_ref Material_) 
        : MeshGroup(MeshGroup_),
        Material(Material_)
    {
        
    }

    void meshgroup_component::Render(graphics_context &Gfx, camera &MainCamera)
    {
        Material->SetGfxState(Gfx);

        __declspec(align(16)) struct cb0
        {
            matrix4 ModelToProj;
        } CB0;
        CB0.ModelToProj = MainCamera.GetViewProjMatrix();
        Gfx.SetDynamicConstantBufferView(0, sizeof(cb0), &CB0);

        for(i32 I = 0; I < MeshGroup->Nodes.size(); ++I)
        {
            scene::meshnode &Node = MeshGroup->Nodes[I];

            // FIX THIS MESS 
            Gfx.SetVertexBuffer(0, MeshGroup->VertexBuffer.VertexBufferView());
            Gfx.SetIndexBuffer(MeshGroup->IndexBuffer.IndexBufferView(0, MeshGroup->IndexBuffer.GetBufferSize(), true));
            Gfx.DrawIndexed(Node.IndexCount, Node.StartIndexLocation, Node.BaseVertexLocation);
        }

    }

    static Vector3 CalculateVertexNormal(Vector3 const &P1, Vector3 const &P2, Vector3 const &P3)
    {
        Vector3 A = P2 - P1;
        Vector3 B = P3 - P1;
        return Normalize(Cross(A, B));
    }

    scene::meshgroup_ref LoadObjAsMeshGroup(std::wstring const &Filename)
    {
        Utility::ByteArray FileData = Utility::ReadFileSync(L"Content\\" + Filename);
        bool HasTexCoords = false, HasNormals = false;
        int CharacterOffset = 0;
        int CharactersRead = 0;
        char DataType[4] = {};
        char *Ptr = (char *)(&FileData->front());

        scene::meshgroup_ref MeshGroup = std::make_shared<scene::meshgroup>();

        std::vector<f32> Positions;
        std::vector<f32> TexCoords;
        std::vector<f32> Normals;

        std::vector<scene::meshvertex> Vertices;
        std::vector<scene::meshindex> Indices;



        while(sscanf_s(Ptr + CharacterOffset, "%s%n", DataType, 4, &CharactersRead) != EOF)
        {
            CharacterOffset += CharactersRead;
            if(DataType[0] == 'v' && DataType[1] == '\0')
            {
                float V[3];
                ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %f %f %f%n", &V[0], &V[1], &V[2], &CharactersRead));
                Positions.push_back(V[0]);
                Positions.push_back(V[1]);
                Positions.push_back(V[2]);
                CharacterOffset += CharactersRead;
            }
            else if(DataType[0] == 'v' && DataType[1] == 't' && DataType[2] == '\0')
            {
                float T[2];
                ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %f %f%n", &T[0], &T[1], &CharactersRead));
                TexCoords.push_back(T[0]);
                TexCoords.push_back(T[1]);
                CharacterOffset += CharactersRead;
                HasTexCoords = true;
            }
            else if(DataType[0] == 'v' && DataType[1] == 'n' && DataType[2] == '\0')
            {
                float N[3];
                ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %f %f %f%n", &N[0], &N[1], &N[2], &CharactersRead));
                Normals.push_back(N[0]);
                Normals.push_back(N[1]);
                Normals.push_back(N[2]);
                CharacterOffset += CharactersRead;
                HasNormals = true;
            }
            else if(DataType[0] == 'f' && DataType[1] == '\0')
            {
                long int I[9];
                scene::meshvertex Vertex[3];

                size_t PosArraySize = Positions.size();
                size_t TCoordArraySize = TexCoords.size();
                size_t NormArraySize = Normals.size();

                if(HasTexCoords && HasNormals)
                {
                    ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %ld/%ld/%ld %ld/%ld/%ld %ld/%ld/%ld%n",
                        &I[0], &I[1], &I[2],
                        &I[3], &I[4], &I[5],
                        &I[6], &I[7], &I[8], &CharactersRead));
                    CharacterOffset += CharactersRead;

                    long PosIndex[3];
                    PosIndex[0] = (I[0] - 1);
                    PosIndex[1] = (I[3] - 1);
                    PosIndex[2] = (I[6] - 1);
                    long TCoordIndex[3];
                    TCoordIndex[0] = (I[1] - 1);
                    TCoordIndex[1] = (I[4] - 1);
                    TCoordIndex[2] = (I[7] - 1);
                    long NormIndex[3];
                    NormIndex[0] = (I[2] - 1);
                    NormIndex[1] = (I[5] - 1);
                    NormIndex[2] = (I[8] - 1);

                    long BasePositionIndex[3];
                    BasePositionIndex[0] = (PosIndex[0] > 0 ? PosIndex[0] * 3 : PosArraySize + (PosIndex[0] + 1) * 3);
                    BasePositionIndex[1] = (PosIndex[1] > 0 ? PosIndex[1] * 3 : PosArraySize + (PosIndex[1] + 1) * 3);
                    BasePositionIndex[2] = (PosIndex[2] > 0 ? PosIndex[2] * 3 : PosArraySize + (PosIndex[2] + 1) * 3);

                    long BaseTCoordIndex[3];
                    BaseTCoordIndex[0] = (TCoordIndex[0] > 0 ? TCoordIndex[0] * 2 : TCoordArraySize + (TCoordIndex[0] + 1) * 2);
                    BaseTCoordIndex[1] = (TCoordIndex[1] > 0 ? TCoordIndex[1] * 2 : TCoordArraySize + (TCoordIndex[1] + 1) * 2);
                    BaseTCoordIndex[2] = (TCoordIndex[2] > 0 ? TCoordIndex[2] * 2 : TCoordArraySize + (TCoordIndex[2] + 1) * 2);

                    long BaseNormIndex[3];
                    BaseNormIndex[0] = (NormIndex[0] > 0 ? NormIndex[0] * 3 : NormArraySize + (NormIndex[0] + 1) * 3);
                    BaseNormIndex[1] = (NormIndex[1] > 0 ? NormIndex[1] * 3 : NormArraySize + (NormIndex[1] + 1) * 3);
                    BaseNormIndex[2] = (NormIndex[2] > 0 ? NormIndex[2] * 3 : NormArraySize + (NormIndex[2] + 1) * 3);

                    // First vertex
                    Vertex[0].Position[0] = Positions[BasePositionIndex[0] + 0];
                    Vertex[0].Position[1] = Positions[BasePositionIndex[0] + 1];
                    Vertex[0].Position[2] = Positions[BasePositionIndex[0] + 2];

                    Vertex[0].TexCoord[0] = TexCoords[BaseTCoordIndex[0] + 0];
                    Vertex[0].TexCoord[1] = TexCoords[BaseTCoordIndex[0] + 1];

                    Vertex[0].Normal[0] = Normals[BaseNormIndex[0] + 0];
                    Vertex[0].Normal[1] = Normals[BaseNormIndex[0] + 1];
                    Vertex[0].Normal[2] = Normals[BaseNormIndex[0] + 2];

                    // Second vertex
                    Vertex[1].Position[0] = Positions[BasePositionIndex[1] + 0];
                    Vertex[1].Position[1] = Positions[BasePositionIndex[1] + 1];
                    Vertex[1].Position[2] = Positions[BasePositionIndex[1] + 2];

                    Vertex[1].TexCoord[0] = TexCoords[BaseTCoordIndex[1] + 0];
                    Vertex[1].TexCoord[1] = TexCoords[BaseTCoordIndex[1] + 1];

                    Vertex[1].Normal[0] = Normals[BaseNormIndex[1] + 0];
                    Vertex[1].Normal[1] = Normals[BaseNormIndex[1] + 1];
                    Vertex[1].Normal[2] = Normals[BaseNormIndex[1] + 2];

                    // Third vertex
                    Vertex[2].Position[0] = Positions[BasePositionIndex[2] + 0];
                    Vertex[2].Position[1] = Positions[BasePositionIndex[2] + 1];
                    Vertex[2].Position[2] = Positions[BasePositionIndex[2] + 2];

                    Vertex[2].TexCoord[0] = TexCoords[BaseTCoordIndex[2] + 0];
                    Vertex[2].TexCoord[1] = TexCoords[BaseTCoordIndex[2] + 1];

                    Vertex[2].Normal[0] = Normals[BaseNormIndex[2] + 0];
                    Vertex[2].Normal[1] = Normals[BaseNormIndex[2] + 1];
                    Vertex[2].Normal[2] = Normals[BaseNormIndex[2] + 2];
                }
                else if(HasTexCoords && !HasNormals)
                {
                    ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %ld/%ld %ld/%ld %ld/%ld%n",
                        &I[0], &I[1],
                        &I[2], &I[3],
                        &I[4], &I[5], &CharactersRead));
                    CharacterOffset += CharactersRead;

                    long PosIndex[3];
                    PosIndex[0] = (I[0] - 1);
                    PosIndex[1] = (I[2] - 1);
                    PosIndex[2] = (I[4] - 1);
                    long TCoordIndex[3];
                    TCoordIndex[0] = (I[1] - 1);
                    TCoordIndex[1] = (I[3] - 1);
                    TCoordIndex[2] = (I[5] - 1);

                    long BasePositionIndex[3];
                    BasePositionIndex[0] = (PosIndex[0] > -1 ? PosIndex[0] * 3 : PosArraySize + (PosIndex[0] + 1) * 3);
                    BasePositionIndex[1] = (PosIndex[1] > -1 ? PosIndex[1] * 3 : PosArraySize + (PosIndex[1] + 1) * 3);
                    BasePositionIndex[2] = (PosIndex[2] > -1 ? PosIndex[2] * 3 : PosArraySize + (PosIndex[2] + 1) * 3);

                    long BaseTCoordIndex[3];
                    BaseTCoordIndex[0] = (TCoordIndex[0] > -1 ? TCoordIndex[0] * 2 : TCoordArraySize + (TCoordIndex[0] + 1) * 2);
                    BaseTCoordIndex[1] = (TCoordIndex[1] > -1 ? TCoordIndex[1] * 2 : TCoordArraySize + (TCoordIndex[1] + 1) * 2);
                    BaseTCoordIndex[2] = (TCoordIndex[2] > -1 ? TCoordIndex[2] * 2 : TCoordArraySize + (TCoordIndex[2] + 1) * 2);

                    // First vertex
                    Vertex[0].Position[0] = Positions[BasePositionIndex[0] + 0];
                    Vertex[0].Position[1] = Positions[BasePositionIndex[0] + 1];
                    Vertex[0].Position[2] = Positions[BasePositionIndex[0] + 2];

                    Vertex[0].TexCoord[0] = TexCoords[BaseTCoordIndex[0] + 0];
                    Vertex[0].TexCoord[1] = TexCoords[BaseTCoordIndex[0] + 1];

                    // Second vertex
                    Vertex[1].Position[0] = Positions[BasePositionIndex[1] + 0];
                    Vertex[1].Position[1] = Positions[BasePositionIndex[1] + 1];
                    Vertex[1].Position[2] = Positions[BasePositionIndex[1] + 2];

                    Vertex[1].TexCoord[0] = TexCoords[BaseTCoordIndex[1] + 0];
                    Vertex[1].TexCoord[1] = TexCoords[BaseTCoordIndex[1] + 1];

                    // Third vertex
                    Vertex[2].Position[0] = Positions[BasePositionIndex[2] + 0];
                    Vertex[2].Position[1] = Positions[BasePositionIndex[2] + 1];
                    Vertex[2].Position[2] = Positions[BasePositionIndex[2] + 2];

                    Vertex[2].TexCoord[0] = TexCoords[BaseTCoordIndex[2] + 0];
                    Vertex[2].TexCoord[1] = TexCoords[BaseTCoordIndex[2] + 1];

                    // Normal of the triangle face
                    Vector3 N = CalculateVertexNormal(
                        Vector3(Vertex[0].Position[0], Vertex[0].Position[1], Vertex[0].Position[2]),
                        Vector3(Vertex[1].Position[0], Vertex[1].Position[1], Vertex[1].Position[2]),
                        Vector3(Vertex[2].Position[0], Vertex[2].Position[1], Vertex[2].Position[2]));

                    f32 NArray[3] = { XMVectorGetX(N), XMVectorGetY(N), XMVectorGetZ(N) };

                    Vertex[0].Normal[0] = NArray[0];
                    Vertex[0].Normal[1] = NArray[1];
                    Vertex[0].Normal[2] = NArray[2];

                    Vertex[1].Normal[0] = NArray[0];
                    Vertex[1].Normal[1] = NArray[1];
                    Vertex[1].Normal[2] = NArray[2];

                    Vertex[2].Normal[0] = NArray[0];
                    Vertex[2].Normal[1] = NArray[1];
                    Vertex[2].Normal[2] = NArray[2];

                }
                else if(!HasTexCoords && HasNormals)
                {
                    ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %ld//%ld %ld//%ld %ld//%ld%n",
                        &I[0], &I[1],
                        &I[2], &I[3],
                        &I[4], &I[5], &CharactersRead));
                    CharacterOffset += CharactersRead;

                    long PosIndex[3];
                    PosIndex[0] = (I[0] - 1);
                    PosIndex[1] = (I[2] - 1);
                    PosIndex[2] = (I[4] - 1);
                    long NormIndex[3];
                    NormIndex[0] = (I[1] - 1);
                    NormIndex[1] = (I[3] - 1);
                    NormIndex[2] = (I[5] - 1);

                    long BasePositionIndex[3];
                    BasePositionIndex[0] = (PosIndex[0] > 0 ? PosIndex[0] * 3 : PosArraySize + (PosIndex[0] + 1) * 3);
                    BasePositionIndex[1] = (PosIndex[1] > 0 ? PosIndex[1] * 3 : PosArraySize + (PosIndex[1] + 1) * 3);
                    BasePositionIndex[2] = (PosIndex[2] > 0 ? PosIndex[2] * 3 : PosArraySize + (PosIndex[2] + 1) * 3);

                    long BaseNormIndex[3];
                    BaseNormIndex[0] = (NormIndex[0] > 0 ? NormIndex[0] * 3 : NormArraySize + (NormIndex[0] + 1) * 3);
                    BaseNormIndex[1] = (NormIndex[1] > 0 ? NormIndex[1] * 3 : NormArraySize + (NormIndex[1] + 1) * 3);
                    BaseNormIndex[2] = (NormIndex[2] > 0 ? NormIndex[2] * 3 : NormArraySize + (NormIndex[2] + 1) * 3);

                    // First vertex
                    Vertex[0].Position[0] = Positions[BasePositionIndex[0] + 0];
                    Vertex[0].Position[1] = Positions[BasePositionIndex[0] + 1];
                    Vertex[0].Position[2] = Positions[BasePositionIndex[0] + 2];

                    Vertex[0].TexCoord[0] = 0.0f;
                    Vertex[0].TexCoord[1] = 0.0f;

                    Vertex[0].Normal[0] = Normals[BaseNormIndex[0] + 0];
                    Vertex[0].Normal[1] = Normals[BaseNormIndex[0] + 1];
                    Vertex[0].Normal[2] = Normals[BaseNormIndex[0] + 2];

                    // Second vertex
                    Vertex[1].Position[0] = Positions[BasePositionIndex[1] + 0];
                    Vertex[1].Position[1] = Positions[BasePositionIndex[1] + 1];
                    Vertex[1].Position[2] = Positions[BasePositionIndex[1] + 2];

                    Vertex[1].TexCoord[0] = 0.0f;
                    Vertex[1].TexCoord[1] = 0.0f;

                    Vertex[1].Normal[0] = Normals[BaseNormIndex[1] + 0];
                    Vertex[1].Normal[1] = Normals[BaseNormIndex[1] + 1];
                    Vertex[1].Normal[2] = Normals[BaseNormIndex[1] + 2];

                    // Third vertex
                    Vertex[2].Position[0] = Positions[BasePositionIndex[2] + 0];
                    Vertex[2].Position[1] = Positions[BasePositionIndex[2] + 1];
                    Vertex[2].Position[2] = Positions[BasePositionIndex[2] + 2];

                    Vertex[2].TexCoord[0] = 0.0f;
                    Vertex[2].TexCoord[1] = 0.0f;

                    Vertex[2].Normal[0] = Normals[BaseNormIndex[2] + 0];
                    Vertex[2].Normal[1] = Normals[BaseNormIndex[2] + 1];
                    Vertex[2].Normal[2] = Normals[BaseNormIndex[2] + 2];
                }
                else /* !HasTexCoords && !HasNormals */
                {
                    ASSERT(EOF != sscanf(Ptr + CharacterOffset, " %ld %ld %ld%n",
                        &I[0], &I[1], &I[2], &CharactersRead));
                    CharacterOffset += CharactersRead;

                    long PosIndex[3];
                    PosIndex[0] = (I[0] - 1);
                    PosIndex[1] = (I[1] - 1);
                    PosIndex[2] = (I[2] - 1);

                    long BasePositionIndex[3];
                    BasePositionIndex[0] = (PosIndex[0] > 0 ? PosIndex[0] * 3 : PosArraySize + (PosIndex[0] + 1) * 3);
                    BasePositionIndex[1] = (PosIndex[1] > 0 ? PosIndex[1] * 3 : PosArraySize + (PosIndex[1] + 1) * 3);
                    BasePositionIndex[2] = (PosIndex[2] > 0 ? PosIndex[2] * 3 : PosArraySize + (PosIndex[2] + 1) * 3);

                    // First vertex
                    Vertex[0].Position[0] = Positions[BasePositionIndex[0] + 0];
                    Vertex[0].Position[1] = Positions[BasePositionIndex[0] + 1];
                    Vertex[0].Position[2] = Positions[BasePositionIndex[0] + 2];

                    Vertex[0].TexCoord[0] = 0.0f;
                    Vertex[0].TexCoord[1] = 0.0f;

                    // Second vertex
                    Vertex[1].Position[0] = Positions[BasePositionIndex[1] + 0];
                    Vertex[1].Position[1] = Positions[BasePositionIndex[1] + 1];
                    Vertex[1].Position[2] = Positions[BasePositionIndex[1] + 2];

                    Vertex[1].TexCoord[0] = 0.0f;
                    Vertex[1].TexCoord[1] = 0.0f;

                    // Third vertex
                    Vertex[2].Position[0] = Positions[BasePositionIndex[2] + 0];
                    Vertex[2].Position[1] = Positions[BasePositionIndex[2] + 1];
                    Vertex[2].Position[2] = Positions[BasePositionIndex[2] + 2];

                    Vertex[2].TexCoord[0] = 0.0f;
                    Vertex[2].TexCoord[1] = 0.0f;

                    // Normal of the triangle face
                    Vector3 N = CalculateVertexNormal(
                        Vector3(Vertex[0].Position[0], Vertex[0].Position[1], Vertex[0].Position[2]),
                        Vector3(Vertex[1].Position[0], Vertex[1].Position[1], Vertex[1].Position[2]),
                        Vector3(Vertex[2].Position[0], Vertex[2].Position[1], Vertex[2].Position[2]));

                    f32 NArray[3] = { XMVectorGetX(N), XMVectorGetY(N), XMVectorGetZ(N) };

                    Vertex[0].Normal[0] = NArray[0];
                    Vertex[0].Normal[1] = NArray[1];
                    Vertex[0].Normal[2] = NArray[2];

                    Vertex[1].Normal[0] = NArray[0];
                    Vertex[1].Normal[1] = NArray[1];
                    Vertex[1].Normal[2] = NArray[2];

                    Vertex[2].Normal[0] = NArray[0];
                    Vertex[2].Normal[1] = NArray[1];
                    Vertex[2].Normal[2] = NArray[2];
                }

                u32 BaseIndex = Vertices.size();
                Indices.push_back(BaseIndex + 0);
                Indices.push_back(BaseIndex + 1);
                Indices.push_back(BaseIndex + 2);
                Vertices.push_back(Vertex[0]);
                Vertices.push_back(Vertex[1]);
                Vertices.push_back(Vertex[2]);
            }

            // Handle CRLF and LF
            if(Ptr[CharacterOffset] == '\r')
            {
                ++CharacterOffset;
            }
            if(Ptr[CharacterOffset] == '\n')
            {
                ++CharacterOffset;
            }
        }
        MeshGroup->Nodes.push_back(scene::meshnode{ L"RootNode", Matrix4(XMMatrixIdentity()), (u32)Indices.size(), 0, 0 });

        // Create vertex and index buffers
        MeshGroup->VertexBuffer.Create(L"meshgroup_vertexbuf", Vertices.size(), sizeof(scene::meshvertex), &Vertices.front());
        MeshGroup->IndexBuffer.Create(L"meshgroup_indexbuf", Indices.size(), sizeof(scene::meshindex), &Indices.front());

        return MeshGroup;
    }
}