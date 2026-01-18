// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIRender/LexUIShaders.h"
#include "LGUI.h"
#include "PipelineStateCache.h"
#include "Materials/Material.h"
#include "ShaderParameterUtils.h"
#include "PrimitiveUniformShaderParameters.h"
#include "MeshBatch.h"
#include "MaterialDomain.h"

IMPLEMENT_MATERIAL_SHADER_TYPE(, FLexUIScreenRenderVS, TEXT("/Plugin/LGUI/Private/LexUIShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLexUIScreenRenderPS, TEXT("/Plugin/LGUI/Private/LexUIShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLexUIWorldRenderPS, TEXT("/Plugin/LGUI/Private/LexUIShader.usf"), TEXT("MainPS"), SF_Pixel);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLexUIWorldRenderDepthFadePS, TEXT("/Plugin/LGUI/Private/LexUIShader.usf"), TEXT("MainPS"), SF_Pixel);

IMPLEMENT_GLOBAL_SHADER_PARAMETER_STRUCT(FLexUIWorldRenderDepthTexUB, "LexUIWorldRenderDepthTexUB");

FLexUIScreenRenderVS::FLexUIScreenRenderVS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	: FMaterialShader(Initializer)
{
	
}
bool FLexUIScreenRenderVS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	return
		(Parameters.MaterialParameters.MaterialDomain == MD_Surface && (Parameters.MaterialParameters.ShadingModels.CountShadingModels() == 1 && Parameters.MaterialParameters.ShadingModels.GetFirstShadingModel() == EMaterialShadingModel::MSM_Unlit))
		|| Parameters.MaterialParameters.MaterialDomain == MD_UI
		;
}
void FLexUIScreenRenderVS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	//OutEnvironment.SetDefine(TEXT("NUM_CUSTOMIZED_UVS"), Material->GetNumCustomizedUVs());
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("NEEDS_WORLD_POSITION_EXCLUDING_SHADER_OFFSETS"), true);
}
void FLexUIScreenRenderVS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(BatchedParameters, View, View.ViewUniformBuffer);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundVertexShader(), BatchedParameters);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundVertexShader(), MaterialRenderProxy, *Material, View);
}



FLexUIScreenRenderPS::FLexUIScreenRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FMaterialShader(Initializer)
{
	LexUIGammaValuesParameter.Bind(Initializer.ParameterMap, TEXT("_LexUIGammaValues"));
}
bool FLexUIScreenRenderPS::ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters)
{
	return
		(Parameters.MaterialParameters.MaterialDomain == MD_Surface && (Parameters.MaterialParameters.ShadingModels.CountShadingModels() == 1 && Parameters.MaterialParameters.ShadingModels.GetFirstShadingModel() == EMaterialShadingModel::MSM_Unlit))
		|| Parameters.MaterialParameters.MaterialDomain == MD_UI
		;
}
void FLexUIScreenRenderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	FMaterialShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	//OutEnvironment.SetDefine(TEXT("NUM_CUSTOMIZED_UVS"), Material->GetNumCustomizedUVs());
	OutEnvironment.SetDefine(TEXT("HAS_PRIMITIVE_UNIFORM_BUFFER"), true);
	OutEnvironment.SetDefine(TEXT("VF_SUPPORTS_PRIMITIVE_SCENE_DATA"), false);
	OutEnvironment.SetDefine(TEXT("NEEDS_WORLD_POSITION_EXCLUDING_SHADER_OFFSETS"), true);
}
void FLexUIScreenRenderPS::SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(BatchedParameters, View, View.ViewUniformBuffer);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundPixelShader(), MaterialRenderProxy, *Material, View);
}
void FLexUIScreenRenderPS::SetGammaValue(FRHICommandList& RHICmdList, float value)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	FVector4f GammaValues(2.2f / value, 1.0f / value, 0.0f, 0.0f);
	SetShaderValue(BatchedParameters, LexUIGammaValuesParameter, GammaValues);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
}


FLexUIWorldRenderPS::FLexUIWorldRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FLexUIScreenRenderPS(Initializer)
{
	SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
	SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
}
void FLexUIWorldRenderPS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("LEXUI_BLEND_DEPTH"), true);
	FLexUIScreenRenderPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}
void FLexUIWorldRenderPS::SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler)
{
	FLexUIWorldRenderDepthTexUB UB;
	UB._SceneDepthTex = DepthTexture;
	UB._SceneDepthTexSampler = DepthTextureSampler;
	TUniformBufferRef<FLexUIWorldRenderDepthTexUB> UniformBuffer = TUniformBufferRef<FLexUIWorldRenderDepthTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIWorldRenderDepthTexUB>(), UniformBuffer);
	SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
	SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
}


FLexUIWorldRenderDepthFadePS::FLexUIWorldRenderDepthFadePS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FLexUIWorldRenderPS(Initializer)
{
	SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
}
void FLexUIWorldRenderDepthFadePS::ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
{
	OutEnvironment.SetDefine(TEXT("LEXUI_DEPTH_FADE"), true);
	FLexUIWorldRenderPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
}
void FLexUIWorldRenderDepthFadePS::SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade)
{
	FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
	SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
	RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
}
