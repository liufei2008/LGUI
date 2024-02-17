// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIRender/LGUIShaders.h"
#include "LGUI.h"
#include "PipelineStateCache.h"
#include "Materials/Material.h"
#include "ShaderParameterUtils.h"
#include "PrimitiveUniformShaderParameters.h"
#include "MeshBatch.h"

IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIScreenRenderVS, TEXT("/Plugin/LGUI/Private/LGUIShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIScreenRenderPS, TEXT("/Plugin/LGUI/Private/LGUIShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIWorldRenderPS, TEXT("/Plugin/LGUI/Private/LGUIShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIWorldRenderDepthFadePS, TEXT("/Plugin/LGUI/Private/LGUIShader.usf"), TEXT("MainPS"), SF_Pixel);


FLGUIScreenRenderVS::FLGUIScreenRenderVS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	: FMaterialShader(Initializer)
{
	
}
bool FLGUIScreenRenderVS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	return
		Parameters.MaterialParameters.MaterialDomain == MD_Surface
		|| Parameters.MaterialParameters.MaterialDomain == MD_UI
		;
}
void FLGUIScreenRenderVS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	//OutEnvironment.SetDefine(TEXT("NUM_CUSTOMIZED_UVS"), Material->GetNumCustomizedUVs());
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("NEEDS_WORLD_POSITION_EXCLUDING_SHADER_OFFSETS"), true);
}
void FLGUIScreenRenderVS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundVertexShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(RHICmdList, RHICmdList.GetBoundVertexShader(), View, View.ViewUniformBuffer);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundVertexShader(), MaterialRenderProxy, *Material, View);
}



FLGUIScreenRenderPS::FLGUIScreenRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FMaterialShader(Initializer)
{
	LGUIGammaValuesParameter.Bind(Initializer.ParameterMap, TEXT("_LGUIGammaValues"));
}
bool FLGUIScreenRenderPS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	return
		Parameters.MaterialParameters.MaterialDomain == MD_Surface
		|| Parameters.MaterialParameters.MaterialDomain == MD_UI
		;
}
void FLGUIScreenRenderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	//OutEnvironment.SetDefine(TEXT("NUM_CUSTOMIZED_UVS"), Material->GetNumCustomizedUVs());
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("NEEDS_WORLD_POSITION_EXCLUDING_SHADER_OFFSETS"), true);
}
void FLGUIScreenRenderPS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(RHICmdList, RHICmdList.GetBoundPixelShader(), View, View.ViewUniformBuffer);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundPixelShader(), MaterialRenderProxy, *Material, View);
}
void FLGUIScreenRenderPS::SetGammaValue(FRHICommandList& RHICmdList, float value)
{
	FVector4f GammaValues(2.2f / value, 1.0f / value, 0.0f, 0.0f);
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), LGUIGammaValuesParameter, GammaValues);
}


FLGUIWorldRenderPS::FLGUIWorldRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FLGUIScreenRenderPS(Initializer)
{
	SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
	SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
	SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
	SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
}
void FLGUIWorldRenderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
	FLGUIScreenRenderPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}
void FLGUIWorldRenderPS::SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler)
{
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthBlendParameter, DepthBlend);
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
}


FLGUIWorldRenderDepthFadePS::FLGUIWorldRenderDepthFadePS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FLGUIWorldRenderPS(Initializer)
{
	SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
}
void FLGUIWorldRenderDepthFadePS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
	FLGUIWorldRenderPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}
void FLGUIWorldRenderDepthFadePS::SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade)
{
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthFadeParameter, DepthFade);
}
