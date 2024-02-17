// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRender/LGUIResolveShaders.h"
#include "LGUI.h"
#include "Materials/Material.h"

IMPLEMENT_SHADER_TYPE(, FLGUIResolveShaderVS, TEXT("/Plugin/LGUI/Private/LGUIResolveShader.usf"), TEXT("LGUIResolveVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLGUIResolveShader2xPS, TEXT("/Plugin/LGUI/Private/LGUIResolveShader.usf"), TEXT("LGUIResolve2xPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIResolveShader4xPS, TEXT("/Plugin/LGUI/Private/LGUIResolveShader.usf"), TEXT("LGUIResolve4xPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIResolveShader8xPS, TEXT("/Plugin/LGUI/Private/LGUIResolveShader.usf"), TEXT("LGUIResolve8xPS"), SF_Pixel)
