﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRender/LGUIPostProcessShaders.h"
#include "LGUI.h"
#include "Materials/Material.h"

IMPLEMENT_SHADER_TYPE(, FLGUISimplePostProcessVS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessVertexShader.usf"), TEXT("SimplePostProcessVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLGUIPostProcessGaussianBlurPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessGaussianBlur.usf"), TEXT("GaussianBlurPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIPostProcessGaussianBlurWithStrengthTexturePS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessGaussianBlur.usf"), TEXT("GaussianBlurPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUISimpleCopyTargetPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessPixelShader.usf"), TEXT("SimpleCopyTargetPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUISimpleCopyTargetPS_ColorCorrect, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessPixelShader.usf"), TEXT("SimpleCopyTargetPS"), SF_Pixel)



IMPLEMENT_SHADER_TYPE(, FLGUICopyMeshRegionVS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessVertexShader.usf"), TEXT("CopyMeshRegionVS"), SF_Vertex)
IMPLEMENT_SHADER_TYPE(, FLGUICopyMeshRegionPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessPixelShader.usf"), TEXT("CopyMeshRegionPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUICopyMeshRegionPS_ColorCorrect, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIPostProcessPixelShader.usf"), TEXT("CopyMeshRegionPS"), SF_Pixel)



IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshVS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshVertexShader.usf"), TEXT("RenderMeshVS"), SF_Vertex)

IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)

IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshPS_RectClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskPS_RectClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)

IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshPS_TextureClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskPS_TextureClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)



IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldVS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshVertexShader.usf"), TEXT("RenderMeshVS"), SF_Vertex)

IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldDepthFadePS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskWorldPS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskWorldDepthFadePS, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)

IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldPS_RectClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldDepthFadePS_RectClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskWorldPS_RectClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskWorldDepthFadePS_RectClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)

IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldPS_TextureClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWorldDepthFadePS_TextureClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskWorldPS_TextureClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
IMPLEMENT_SHADER_TYPE(, FLGUIRenderMeshWithMaskWorldDepthFadePS_TextureClip, TEXT("/Plugin/LGUI/Private/PostProcess/LGUIRenderMeshPixelShader.usf"), TEXT("RenderMeshPS"), SF_Pixel)
