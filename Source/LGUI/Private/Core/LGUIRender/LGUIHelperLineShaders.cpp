// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRender/LGUIHelperLineShaders.h"
#include "LGUI.h"
#include "Materials/Material.h"

IMPLEMENT_SHADER_TYPE(, FLGUIHelperLineShaderVS, TEXT("/Plugin/LGUI/Private/LGUIHelperLineShader.usf"), TEXT("MainVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLGUIHelperLineShaderPS, TEXT("/Plugin/LGUI/Private/LGUIHelperLineShader.usf"), TEXT("MainPS"), SF_Pixel)
