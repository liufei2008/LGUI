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

	}
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
class FLGUICopyTargetSimplePS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUICopyTargetSimplePS, Global);
public:
	FLGUICopyTargetSimplePS() {}
	FLGUICopyTargetSimplePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
		FlipYParameter.Bind(Initializer.ParameterMap, TEXT("_FlipY"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SceneTexture, bool FlipY)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), SceneTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), FlipYParameter, FlipY);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		Ar << FlipYParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
	FShaderParameter FlipYParameter;
};
class FLGUICopyTargetMeshPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUICopyTargetMeshPS, Global);
public:
	FLGUICopyTargetMeshPS() {}
	FLGUICopyTargetMeshPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
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
class FLGUIBlurShaderPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIBlurShaderPS, Global);
public:
	FLGUIBlurShaderPS() {}
	FLGUIBlurShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
		UVStartParameter.Bind(Initializer.ParameterMap, TEXT("_UVStart"));
		UVIntervalParameter.Bind(Initializer.ParameterMap, TEXT("_UVInterval"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SceneTexture, float BlurStrength, bool HorizontalOrVertical)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI(), SceneTexture);
		BlurStrength = FMath::Min(BlurStrength, 1.0f);
		if (HorizontalOrVertical)
		{
			FVector2D _UVInterval = FVector2D(BlurStrength / SceneTexture->GetSizeX(), 0);
			FVector2D _UVStart = FVector2D(-_UVInterval.X * 3.0f, 0.0f);
			SetShaderValue(RHICmdList, GetPixelShader(), UVStartParameter, _UVStart);
			SetShaderValue(RHICmdList, GetPixelShader(), UVIntervalParameter, _UVInterval);
		}
		else
		{
			FVector2D _UVInterval = FVector2D(0.0f, BlurStrength / SceneTexture->GetSizeY());
			FVector2D _UVStart = FVector2D(0.0f, -_UVInterval.Y * 3.0f);
			SetShaderValue(RHICmdList, GetPixelShader(), UVStartParameter, _UVStart);
			SetShaderValue(RHICmdList, GetPixelShader(), UVIntervalParameter, _UVInterval);
		}
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		Ar << UVStartParameter;
		Ar << UVIntervalParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
	FShaderParameter UVStartParameter;
	FShaderParameter UVIntervalParameter;
};
