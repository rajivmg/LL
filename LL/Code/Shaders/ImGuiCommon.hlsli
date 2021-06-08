#define ImGui_RootSig \
    "RootFlags(ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT | DENY_HULL_SHADER_ROOT_ACCESS |"     \
    "DENY_DOMAIN_SHADER_ROOT_ACCESS | DENY_GEOMETRY_SHADER_ROOT_ACCESS),"               \
    "CBV(b0, visibility = SHADER_VISIBILITY_VERTEX),"   \
    "DescriptorTable(SRV(t0), visibility = SHADER_VISIBILITY_PIXEL),"   \
    "StaticSampler(s0, visibility = SHADER_VISIBILITY_PIXEL,"   \
    "   addressU = TEXTURE_ADDRESS_WRAP,"               \
    "   addressV = TEXTURE_ADDRESS_WRAP,"               \
    "   addressW = TEXTURE_ADDRESS_WRAP,"               \
    "   mipLODBias = 0.0f,"                             \
    "   maxAnisotropy = 0,"                             \
    "   comparisonFunc = COMPARISON_ALWAYS,"            \
    "   minLOD = 0.0f, maxLOD = 0.0f,"                  \
    "   filter = FILTER_MIN_MAG_MIP_LINEAR),"           
