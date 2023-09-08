// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/HudRender/LGUIHudShaders.h"
#include "LGUI.h"
#include "PipelineStateCache.h"
#include "Materials/Material.h"
#include "ShaderParameterUtils.h"
#include "PrimitiveUniformShaderParameters.h"
#include "MeshBatch.h"

IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIHudRenderVS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIHudRenderPS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIWorldRenderPS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIWorldRenderDepthFadePS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainPS"), SF_Pixel);


FLGUIHudRenderVS::FLGUIHudRenderVS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	: FMaterialShader(Initializer)
{
	
}
bool FLGUIHudRenderVS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	return true;
}
void FLGUIHudRenderVS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	//OutEnvironment.SetDefine(TEXT("NUM_CUSTOMIZED_UVS"), Material->GetNumCustomizedUVs());
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("NEEDS_WORLD_POSITION_EXCLUDING_SHADER_OFFSETS"), true);
}
void FLGUIHudRenderVS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(BatchedParameters, View, View.ViewUniformBuffer);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundVertexShader(), BatchedParameters);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundVertexShader(), MaterialRenderProxy, *Material, View);
}



FLGUIHudRenderPS::FLGUIHudRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FMaterialShader(Initializer)
{
	ColorCorrectionValueForHDRParameter.Bind(Initializer.ParameterMap, TEXT("_ColorCorrectionValueForHDR"));
}
bool FLGUIHudRenderPS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	return true;
}
void FLGUIHudRenderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	//OutEnvironment.SetDefine(TEXT("NUM_CUSTOMIZED_UVS"), Material->GetNumCustomizedUVs());
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("NEEDS_WORLD_POSITION_EXCLUDING_SHADER_OFFSETS"), true);
}
void FLGUIHudRenderPS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(BatchedParameters, View, View.ViewUniformBuffer);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundPixelShader(), MaterialRenderProxy, *Material, View);
}
void FLGUIHudRenderPS::SetColorCorrectionValue(FRHICommandList& RHICmdList, float value)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetShaderValue(BatchedParameters, ColorCorrectionValueForHDRParameter, value);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
}


FLGUIWorldRenderPS::FLGUIWorldRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FLGUIHudRenderPS(Initializer)
{
	SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
	SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
	SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
	SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
}
void FLGUIWorldRenderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
	FLGUIHudRenderPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}
void FLGUIWorldRenderPS::SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
	SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
	SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
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
void FLGUIWorldRenderDepthFadePS::SetDepthFadeParameter(FRHICommandList& RHICmdList, float DepthFade)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
}
