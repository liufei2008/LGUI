// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
	virtual bool Serialize(FArchive& Ar)override;
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

	virtual bool Serialize(FArchive& Ar) override;
};

class FLGUIWorldRenderPS : public FLGUIHudRenderPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIWorldRenderPS, Material);

	FLGUIWorldRenderPS() {}
	FLGUIWorldRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);
	static void ModifyCompilationEnvironment(const FMaterialShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment);

	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI());
	virtual bool Serialize(FArchive& Ar) override;
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
};
