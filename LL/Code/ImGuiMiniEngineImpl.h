#pragma once
#include "imgui.h"

class GraphicsContext;

namespace imgui_miniengine
{
    IMGUI_IMPL_API void Init();
    IMGUI_IMPL_API void Shutdown();
    IMGUI_IMPL_API void NewFrame(float DeltaT);
    IMGUI_IMPL_API void RenderFrame(GraphicsContext &Ctx);
}