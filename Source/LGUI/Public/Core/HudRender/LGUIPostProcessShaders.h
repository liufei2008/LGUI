// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "MaterialShader.h"
#include "Engine/Texture2D.h"

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
	void SetParameters(FRHICommandListImmediate& RHICmdList)
	{

	}
private:
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
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef SceneTexture, FRHISamplerState* SceneTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureParameter, MainTextureSamplerParameter, SceneTextureSampler, SceneTexture);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
};
class FLGUISimpleCopyTargetWithMaskPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUISimpleCopyTargetWithMaskPS, Global);
public:
	FLGUISimpleCopyTargetWithMaskPS() {}
	FLGUISimpleCopyTargetWithMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
		OriginTextureParameter.Bind(Initializer.ParameterMap, TEXT("_OriginTex"));
		OriginTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_OriginTexSampler"));
		MaskTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MaskTex"));
		MaskTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MaskTexSampler"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList
		, FTexture2DRHIRef MainTexture
		, FTexture2DRHIRef OriginTexture
		, FTexture2DRHIRef MaskTexture
		, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
		, FRHISamplerState* OriginTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
		, FRHISamplerState* MaskTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), OriginTextureParameter, OriginTextureSamplerParameter, OriginTextureSampler, OriginTexture);
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MaskTextureParameter, MaskTextureSamplerParameter, MaskTextureSampler, MaskTexture);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
	LAYOUT_FIELD(FShaderResourceParameter, OriginTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, OriginTextureSamplerParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureSamplerParameter);
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
	void SetMainTexture(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef MainTexture, FRHISamplerState* MainTextureSampler)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("USE_MASK"), 0);
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetInverseTextureSize(FRHICommandListImmediate& RHICmdList, const FVector2D& InvSize)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), InvSizeParameter, InvSize);
	}
	void SetBlurStrength(FRHICommandListImmediate& RHICmdList, float BlurStrength)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), BlurStrengthParameter, BlurStrength);
	}
	void SetHorizontalOrVertical(FRHICommandListImmediate& RHICmdList, bool HorizontalOrVertical)
	{
		SetShaderValue(RHICmdList, RHICmdList.GetBoundPixelShader(), HorizontalOrVerticalFilterParameter, HorizontalOrVertical ? FVector2D(1.0f, 0.0f) : FVector2D(0.0f, 1.0f));
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, HorizontalOrVerticalFilterParameter);
	LAYOUT_FIELD(FShaderParameter, InvSizeParameter);
	LAYOUT_FIELD(FShaderParameter, BlurStrengthParameter);
};
class FLGUIPostProcessGaussianBlurWithStrengthTexturePS :public FLGUIPostProcessGaussianBlurPS
{
	DECLARE_SHADER_TYPE(FLGUIPostProcessGaussianBlurWithStrengthTexturePS, Global);
public:
	FLGUIPostProcessGaussianBlurWithStrengthTexturePS() {}
	FLGUIPostProcessGaussianBlurWithStrengthTexturePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessGaussianBlurPS(Initializer)
	{
		StrengthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_StrengthTex"));
		StrengthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_StrengthTexSampler"));
	}
	void SetStrengthTexture(FRHICommandListImmediate& RHICmdList, FTexture2DRHIRef StrengthTexture, FRHISamplerState* StrengthTextureSampler)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), StrengthTextureParameter, StrengthTextureSamplerParameter, StrengthTextureSampler, StrengthTexture);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("USE_STRENGTH_TEXTURE"), 1);
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, StrengthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, StrengthTextureSamplerParameter);
};
