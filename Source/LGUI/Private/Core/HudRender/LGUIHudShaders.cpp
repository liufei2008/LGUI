// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	SetUniformBufferParameter(RHICmdList, GetVertexShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	FMaterialShader::SetParameters(RHICmdList, GetVertexShader(), MaterialRenderProxy, *Material, View, View.ViewUniformBuffer, ESceneTextureSetupMode::None);
}
bool FLGUIHudRenderVS::Serialize(FArchive& Ar)
{
	bool bShaderHasOutdatedParameters = FMaterialShader::Serialize(Ar);
	return bShaderHasOutdatedParameters;
}



FLGUIHudRenderPS::FLGUIHudRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FMaterialShader(Initializer)
{
	
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
	SetUniformBufferParameter(RHICmdList, GetPixelShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	const ESceneTextureSetupMode SceneTextures = ESceneTextureSetupMode::SceneDepth | ESceneTextureSetupMode::SSAO | ESceneTextureSetupMode::CustomDepth;
	FMaterialShader::SetParameters(RHICmdList, GetPixelShader(), MaterialRenderProxy, *Material, View, View.ViewUniformBuffer, SceneTextures);
}
bool FLGUIHudRenderPS::Serialize(FArchive& Ar)
{
	bool bShaderHasOutdatedParameters = FMaterialShader::Serialize(Ar);
	return bShaderHasOutdatedParameters;
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
void FLGUIWorldRenderPS::SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler)
{
	SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
	SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
	SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
}
bool FLGUIWorldRenderPS::Serialize(FArchive& Ar)
{
	bool bShaderHasOutdatedParameters = FLGUIHudRenderPS::Serialize(Ar);
	Ar << SceneDepthTextureParameter;
	Ar << SceneDepthTextureSamplerParameter;
	Ar << SceneDepthBlendParameter;
	return bShaderHasOutdatedParameters;
}
