#include "LL_MeshExporter.h"

#include <cmath>

#include "imgui.h"

#include "FileUtility.h"
#include "LL_Common.h"
#include "LL_Utility.h"
#include "LL_Mesh.h"

namespace meshexporter
{
    using namespace Math;

    static bool IsGeomFileListLoaded;
    static utility::filelist GeomFileList;
    static i32 SelectedGeomFileIndex;

    void ResetValues()
    {
        IsGeomFileListLoaded = false;
        GeomFileList = nullptr;
        SelectedGeomFileIndex = 0;
    }

    void ShowWindow(bool *Show)
    {
        ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_Always);
        ImGui::Begin("MeshExporter##meshexporter", Show);
        if(!IsGeomFileListLoaded)
        {
            GeomFileList = utility::GetFileList(L"Content");
            IsGeomFileListLoaded = true;
        }
        // Left pane
        {
            ImGui::BeginChild("Left Pane##geom2cpp", ImVec2(150, 0), true);
            for(i32 I = 0; I < GeomFileList->size(); ++I)
            {
                if(ImGui::Selectable(utility::ToString(GeomFileList->at(I)).c_str(), SelectedGeomFileIndex == I))
                {
                    SelectedGeomFileIndex = I;
                }
            }
            ImGui::EndChild();
        }
        ImGui::SameLine();
        // Right pane
        {
            ImGui::BeginGroup();
            ImGui::BeginChild("Right Pane##meshexporter", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
            ImGui::Text("File: %s", utility::ToString(GeomFileList->at(SelectedGeomFileIndex)).c_str());
            ImGui::Separator();
            if(ImGui::BeginTabBar("MeshExporter##tabs", ImGuiTabBarFlags_None))
            {
                if(ImGui::BeginTabItem("Options"))
                {
                    ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
                    ImGui::EndTabItem();
                }
                if(ImGui::BeginTabItem("Log"))
                {
                    ImGui::Text("ID: 0123456789");
                    ImGui::EndTabItem();
                }
                ImGui::EndTabBar();
            }
            ImGui::EndChild();
            if(ImGui::Button("Save as Cpp"))
            { 
                //scene::meshgroup *MeshGroup = LoadObjAsMeshGroup(GeomFileList->at(SelectedGeomFileIndex));
            }
            //ImGui::SameLine();
            //if(ImGui::Button("Save")) {}
            ImGui::EndGroup();
        }
        ImGui::End();
    }
}