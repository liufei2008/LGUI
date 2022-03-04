// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundVertexShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(RHICmdList, RHICmdList.GetBoundVertexShader(), View, View.ViewUniformBuffer);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundVertexShader(), MaterialRenderProxy, *Material, View);
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
	SetUniformBufferParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), *Mesh.Elements[0].PrimitiveUniformBufferResource);
	SetViewParameters(RHICmdList, RHICmdList.GetBoundPixelShader(), View, View.ViewUniformBuffer);
	FMaterialShader::SetParameters(RHICmdList, RHICmdList.GetBoundPixelShader(), MaterialRenderProxy, *Material, View);
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
	SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthBlendParameter, DepthBlend);
	SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
}
