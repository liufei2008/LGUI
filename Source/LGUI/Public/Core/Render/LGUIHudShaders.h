// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "MaterialShader.h"
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

	void SetBlendState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material);
	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);

	virtual bool Serialize(FArchive& Ar) override;
};


class FLGUIPostProcessShader :public FGlobalShader
{
public:
	FLGUIPostProcessShader() {}
	FLGUIPostProcessShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		:FGlobalShader(Initializer)
	{

	}
	static bool ShouldCompilePermutation(const FGlobalShaderPermutationParameters& Parameters)
	{
		return true;
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	static bool ShouldCache(EShaderPlatform Platform)
	{
		return true;
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		return FGlobalShader::Serialize(Ar);
	}
};
class FLGUISimplePostProcessVS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUISimplePostProcessVS, Global);
public:
	FLGUISimplePostProcessVS() {}
	FLGUISimplePostProcessVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		PositionScaleAndOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_PositionScaleAndOffset"));
		UVScaleAndOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_UVScaleAndOffset"));
		FlipUVYParameter.Bind(Initializer.ParameterMap, TEXT("_FlipUV_Y"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, bool FlipUVY = false, const FVector4& PositionScaleAndOffset = FVector4(1, 1, 0, 0), const FVector4& UVScaleAndOffset = FVector4(1, 1, 0, 0))
	{
		SetShaderValue(RHICmdList, GetVertexShader(), PositionScaleAndOffsetParameter, PositionScaleAndOffset);
		SetShaderValue(RHICmdList, GetVertexShader(), UVScaleAndOffsetParameter, UVScaleAndOffset);
		SetShaderValue(RHICmdList, GetVertexShader(), FlipUVYParameter, FlipUVY);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << PositionScaleAndOffsetParameter;
		Ar << UVScaleAndOffsetParameter;
		Ar << FlipUVYParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter PositionScaleAndOffsetParameter;
	FShaderParameter UVScaleAndOffsetParameter;
	FShaderParameter FlipUVYParameter;
};
class FLGUIMeshPostProcessVS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIMeshPostProcessVS, Global);
public:
	FLGUIMeshPostProcessVS() {}
	FLGUIMeshPostProcessVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		ViewProjectionParameter.Bind(Initializer.ParameterMap, TEXT("_ViewProjectionMatrix"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix& ViewProjectionMatrix)
	{
		SetShaderValue(RHICmdList, GetVertexShader(), ViewProjectionParameter, ViewProjectionMatrix);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << ViewProjectionParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter ViewProjectionParameter;
};
class FLGUISimpleCopyTargetPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUISimpleCopyTargetPS, Global);
public:
	FLGUISimpleCopyTargetPS() {}
	FLGUISimpleCopyTargetPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SceneTexture)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), SceneTexture);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
};
class FLGUIMeshCopyTargetPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIMeshCopyTargetPS, Global);
public:
	FLGUIMeshCopyTargetPS() {}
	FLGUIMeshCopyTargetPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SceneTexture)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), SceneTexture);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
};
class FLGUIPostProcessGaussianBlurPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIPostProcessGaussianBlurPS, Global);
public:
	FLGUIPostProcessGaussianBlurPS() {}
	FLGUIPostProcessGaussianBlurPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
		HorizontalOrVerticalFilterParameter.Bind(Initializer.ParameterMap, TEXT("_HorizontalOrVerticalFilter"));
		InvSizeParameter.Bind(Initializer.ParameterMap, TEXT("_InvSize"));
		BlurStrengthParameter.Bind(Initializer.ParameterMap, TEXT("_BlurStrength"));
	}
	void SetMainTexture(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef MainTexture, const FSamplerStateRHIParamRef& MainTextureSampler)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
	}
	void SetInverseTextureSize(FRHICommandListImmediate& RHICmdList, const FVector2D& InvSize)
	{
		SetShaderValue(RHICmdList, GetPixelShader(), InvSizeParameter, InvSize);
	}
	void SetBlurStrength(FRHICommandListImmediate& RHICmdList, float BlurStrength)
	{
		SetShaderValue(RHICmdList, GetPixelShader(), BlurStrengthParameter, BlurStrength);
	}
	void SetHorizontalOrVertical(FRHICommandListImmediate& RHICmdList, bool HorizontalOrVertical)
	{
		SetShaderValue(RHICmdList, GetPixelShader(), HorizontalOrVerticalFilterParameter, HorizontalOrVertical ? FVector2D(1.0f, 0.0f) : FVector2D(0.0f, 1.0f));
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		Ar << HorizontalOrVerticalFilterParameter;
		Ar << InvSizeParameter;
		Ar << BlurStrengthParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
	FShaderParameter HorizontalOrVerticalFilterParameter;
	FShaderParameter InvSizeParameter;
	FShaderParameter BlurStrengthParameter;
};
