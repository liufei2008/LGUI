// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/HudRender/LGUIPostProcessShaders.h"
#include "LGUI.h"
#include "Materials/Material.h"

IMPLEMENT_SHADER_TYPE(, FLGUISimplePostProcessVS, TEXT("/Plugin/LGUI/Private/LGUIPostProcessVertexShader.usf"), TEXT("SimplePostProcessVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLGUIMeshPostProcessVS, TEXT("/Plugin/LGUI/Private/LGUIPostProcessVertexShader.usf"), TEXT("MeshPostProcessVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLGUIPostProcessGaussianBlurPS, TEXT("/Plugin/LGUI/Private/LGUIPostProcessGaussianBlur.usf"), TEXT("GaussianBlurPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUISimpleCopyTargetPS, TEXT("/Plugin/LGUI/Private/LGUICopyTargetShader.usf"), TEXT("SimpleCopyTargetPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIMeshCopyTargetPS, TEXT("/Plugin/LGUI/Private/LGUICopyTargetShader.usf"), TEXT("MeshCopyTargetPS"), SF_Pixel)
