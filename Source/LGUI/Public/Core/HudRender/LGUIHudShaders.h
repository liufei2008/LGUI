// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "Engine/Texture2D.h"
#include "MeshMaterialShader.h"

class FLGUIHudRenderVS :public FMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FLGUIHudRenderVS, Material);

	FLGUIHudRenderVS() {}
	FLGUIHudRenderVS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);

	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);
	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);
	
	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);
};
class FLGUIHudRenderPS : public FMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FLGUIHudRenderPS, Material);

	FLGUIHudRenderPS() {}
	FLGUIHudRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);
	static bool ShouldCompilePermutation(const FMaterialShaderPermutationParameters& Parameters);
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);
	void SetGammaValue(FRHICommandList& RHICmdList, float value);
private:
	LAYOUT_FIELD(FShaderParameter, LGUIGammaValuesParameter);
};

class FLGUIWorldRenderPS : public FLGUIHudRenderPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIWorldRenderPS, Material);

	FLGUIWorldRenderPS() {}
	FLGUIWorldRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};

class FLGUIWorldRenderDepthFadePS : public FLGUIWorldRenderPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIWorldRenderDepthFadePS, Material);

	FLGUIWorldRenderDepthFadePS() {}
	FLGUIWorldRenderDepthFadePS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	void SetDepthFadeParameter(FRHICommandList& RHICmdList, float DepthFade);
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
};
