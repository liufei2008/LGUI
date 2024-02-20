﻿// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "MaterialShader.h"
#include "Engine/Texture2D.h"
#include "RHIStaticStates.h"

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
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTextureRHIRef SceneTexture, FRHISamplerState* SceneTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureParameter, MainTextureSamplerParameter, SceneTextureSampler, SceneTexture);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
};
class FLGUISimpleCopyTargetPS_ColorCorrect : public FLGUISimpleCopyTargetPS
{
	DECLARE_SHADER_TYPE(FLGUISimpleCopyTargetPS_ColorCorrect, Global);
public:
	FLGUISimpleCopyTargetPS_ColorCorrect() {}
	FLGUISimpleCopyTargetPS_ColorCorrect(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUISimpleCopyTargetPS(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_COLORCORRECT"), true);
		FLGUISimpleCopyTargetPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
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
		BlurStrengthParameter.Bind(Initializer.ParameterMap, TEXT("_BlurStrength"));
	}
	void SetMainTexture(FRHICommandListImmediate& RHICmdList, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetBlurStrength(FRHICommandListImmediate& RHICmdList, const FVector2f& BlurStrength)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, BlurStrengthParameter, BlurStrength);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
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
	void SetStrengthTexture(FRHICommandListImmediate& RHICmdList, FTextureRHIRef StrengthTexture, FRHISamplerState* StrengthTextureSampler)
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), StrengthTextureParameter, StrengthTextureSamplerParameter, StrengthTextureSampler, StrengthTexture);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("USE_STRENGTH_TEXTURE"), 1);
		FLGUIPostProcessGaussianBlurPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, StrengthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, StrengthTextureSamplerParameter);
};





//render mesh region 
class FLGUICopyMeshRegionVS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUICopyMeshRegionVS, Global);
public:
	FLGUICopyMeshRegionVS() {}
	FLGUICopyMeshRegionVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		
	}
};

//render mesh pixel shader
class FLGUICopyMeshRegionPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUICopyMeshRegionPS, Global);
public:
	FLGUICopyMeshRegionPS() {}
	FLGUICopyMeshRegionPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
		MainTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_MainTextureScaleOffset"));
		MVPParameter.Bind(Initializer.ParameterMap, TEXT("_MVP"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix44f& MVP, const FVector4f& MainTextureScaleOffset, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
		SetShaderValue(BatchedParameters, MVPParameter, MVP);
		SetShaderValue(BatchedParameters, MainTextureScaleOffsetParameter, MainTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, MainTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, MVPParameter);
};
class FLGUICopyMeshRegionPS_ColorCorrect : public FLGUICopyMeshRegionPS
{
	DECLARE_SHADER_TYPE(FLGUICopyMeshRegionPS_ColorCorrect, Global);
public:
	FLGUICopyMeshRegionPS_ColorCorrect() {}
	FLGUICopyMeshRegionPS_ColorCorrect(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUICopyMeshRegionPS(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_COLORCORRECT"), true);
		FLGUISimpleCopyTargetPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
};




//common render mesh vertex shader
class FLGUIRenderMeshVS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshVS, Global);
public:
	FLGUIRenderMeshVS() {}
	FLGUIRenderMeshVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MVPParameter.Bind(Initializer.ParameterMap, TEXT("_MVP"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix44f& MVP)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, MVPParameter, MVP);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundVertexShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, MVPParameter);
};
class FLGUIRenderMeshWorldVS : public FLGUIRenderMeshVS
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldVS, Global);

	FLGUIRenderMeshWorldVS() {}
	FLGUIRenderMeshWorldVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshVS(Initializer)
	{

	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshVS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
};




//render mesh pixel shader
class FLGUIRenderMeshPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshPS, Global);
public:
	FLGUIRenderMeshPS() {}
	FLGUIRenderMeshPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_MASK"), 0);
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, RHICmdList.GetBoundPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
};
class FLGUIRenderMeshWorldPS : public FLGUIRenderMeshPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldPS, Global);

	FLGUIRenderMeshWorldPS() {}
	FLGUIRenderMeshWorldPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshPS(Initializer)
	{
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
		SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLGUIRenderMeshWorldDepthFadePS : public FLGUIRenderMeshWorldPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldDepthFadePS, Global);

	FLGUIRenderMeshWorldDepthFadePS() {}
	FLGUIRenderMeshWorldDepthFadePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWorldPS(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
		FLGUIRenderMeshWorldPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade, const FVector2f& ViewSizeInv)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
		SetShaderValue(BatchedParameters, ViewSizeInvParameter, ViewSizeInv);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
	LAYOUT_FIELD(FShaderParameter, ViewSizeInvParameter);
};
//render mesh pixel shader, use a mask texture
class FLGUIRenderMeshWithMaskPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskPS, Global);
public:
	FLGUIRenderMeshWithMaskPS() {}
	FLGUIRenderMeshWithMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		MainTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MainTex"));
		MainTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MainTexSampler"));
		MaskTextureParameter.Bind(Initializer.ParameterMap, TEXT("_MaskTex"));
		MaskTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_MaskTexSampler"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_MASK"), 1);
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList
		, FTextureRHIRef MainTexture
		, FTextureRHIRef MaskTexture
		, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
		, FRHISamplerState* MaskTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
		SetTextureParameter(BatchedParameters, MaskTextureParameter, MaskTextureSamplerParameter, MaskTextureSampler, MaskTexture);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MainTextureSamplerParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, MaskTextureSamplerParameter);
};
class FLGUIRenderMeshWithMaskWorldPS : public FLGUIRenderMeshWithMaskPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskWorldPS, Global);

	FLGUIRenderMeshWithMaskWorldPS() {}
	FLGUIRenderMeshWithMaskWorldPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskPS(Initializer)
	{
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
		SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshWithMaskPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLGUIRenderMeshWithMaskWorldDepthFadePS : public FLGUIRenderMeshWithMaskWorldPS
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskWorldDepthFadePS, Global);

	FLGUIRenderMeshWithMaskWorldDepthFadePS() {}
	FLGUIRenderMeshWithMaskWorldDepthFadePS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskWorldPS(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
		FLGUIRenderMeshWithMaskWorldPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade, const FVector2f& ViewSizeInv)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
		SetShaderValue(BatchedParameters, ViewSizeInvParameter, ViewSizeInv);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
	LAYOUT_FIELD(FShaderParameter, ViewSizeInvParameter);
};

#pragma region RectClip
//render mesh pixel shader
class FLGUIRenderMeshPS_RectClip :public FLGUIRenderMeshPS
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshPS_RectClip, Global);
public:
	FLGUIRenderMeshPS_RectClip() {}
	FLGUIRenderMeshPS_RectClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshPS(Initializer)
	{
		OffsetAndSizeParameter.Bind(Initializer.ParameterMap, TEXT("_RectClipOffsetAndSize"));
		FeatherParameter.Bind(Initializer.ParameterMap, TEXT("_RectClipFeather"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_RECT_CLIP"), 1);
		FLGUIRenderMeshPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetClipParameters(FRHICommandListImmediate& RHICmdList, const FVector4f& OffsetAndSize, const FVector4f& Feather)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, OffsetAndSizeParameter, OffsetAndSize);
		SetShaderValue(BatchedParameters, FeatherParameter, Feather);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, OffsetAndSizeParameter);
	LAYOUT_FIELD(FShaderParameter, FeatherParameter);
};
class FLGUIRenderMeshWorldPS_RectClip : public FLGUIRenderMeshPS_RectClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldPS_RectClip, Global);

	FLGUIRenderMeshWorldPS_RectClip() {}
	FLGUIRenderMeshWorldPS_RectClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshPS_RectClip(Initializer)
	{
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
		SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshPS_RectClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLGUIRenderMeshWorldDepthFadePS_RectClip : public FLGUIRenderMeshWorldPS_RectClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldDepthFadePS_RectClip, Global);

	FLGUIRenderMeshWorldDepthFadePS_RectClip() {}
	FLGUIRenderMeshWorldDepthFadePS_RectClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWorldPS_RectClip(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
		FLGUIRenderMeshWorldPS_RectClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade, const FVector2f& ViewSizeInv)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
		SetShaderValue(BatchedParameters, ViewSizeInvParameter, ViewSizeInv);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
	LAYOUT_FIELD(FShaderParameter, ViewSizeInvParameter);
};
//render mesh pixel shader, use a mask texture
class FLGUIRenderMeshWithMaskPS_RectClip :public FLGUIRenderMeshWithMaskPS
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskPS_RectClip, Global);
public:
	FLGUIRenderMeshWithMaskPS_RectClip() {}
	FLGUIRenderMeshWithMaskPS_RectClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskPS(Initializer)
	{
		OffsetAndSizeParameter.Bind(Initializer.ParameterMap, TEXT("_RectClipOffsetAndSize"));
		FeatherParameter.Bind(Initializer.ParameterMap, TEXT("_RectClipFeather"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_RECT_CLIP"), 1);
		FLGUIRenderMeshWithMaskPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetClipParameters(FRHICommandListImmediate& RHICmdList, const FVector4f& OffsetAndSize, const FVector4f& Feather)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, OffsetAndSizeParameter, OffsetAndSize);
		SetShaderValue(BatchedParameters, FeatherParameter, Feather);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, OffsetAndSizeParameter);
	LAYOUT_FIELD(FShaderParameter, FeatherParameter);
};
class FLGUIRenderMeshWithMaskWorldPS_RectClip : public FLGUIRenderMeshWithMaskPS_RectClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskWorldPS_RectClip, Global);

	FLGUIRenderMeshWithMaskWorldPS_RectClip() {}
	FLGUIRenderMeshWithMaskWorldPS_RectClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskPS_RectClip(Initializer)
	{
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
		SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshWithMaskPS_RectClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLGUIRenderMeshWithMaskWorldDepthFadePS_RectClip : public FLGUIRenderMeshWithMaskWorldPS_RectClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskWorldDepthFadePS_RectClip, Global);

	FLGUIRenderMeshWithMaskWorldDepthFadePS_RectClip() {}
	FLGUIRenderMeshWithMaskWorldDepthFadePS_RectClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskWorldPS_RectClip(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
		FLGUIRenderMeshWithMaskWorldPS_RectClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade, const FVector2f& ViewSizeInv)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
		SetShaderValue(BatchedParameters, ViewSizeInvParameter, ViewSizeInv);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
	LAYOUT_FIELD(FShaderParameter, ViewSizeInvParameter);
};
#pragma endregion

#pragma region TextureClip
//render mesh pixel shader
class FLGUIRenderMeshPS_TextureClip :public FLGUIRenderMeshPS
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshPS_TextureClip, Global);
public:
	FLGUIRenderMeshPS_TextureClip() {}
	FLGUIRenderMeshPS_TextureClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshPS(Initializer)
	{
		OffsetAndSizeParameter.Bind(Initializer.ParameterMap, TEXT("_TextureClipOffsetAndSize"));
		ClipTextureParameter.Bind(Initializer.ParameterMap, TEXT("_ClipTex"));
		ClipTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_ClipTexSampler"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_TEXTURE_CLIP"), 1);
		FLGUIRenderMeshPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetClipParameters(FRHICommandListImmediate& RHICmdList, const FVector4f& OffsetAndSize
		, FTextureRHIRef ClipTexture
		, FRHISamplerState* ClipTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, OffsetAndSizeParameter, OffsetAndSize);
		SetTextureParameter(BatchedParameters, ClipTextureParameter, ClipTextureSamplerParameter, ClipTextureSampler, ClipTexture);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, OffsetAndSizeParameter);
	LAYOUT_FIELD(FShaderResourceParameter, ClipTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, ClipTextureSamplerParameter);
};
class FLGUIRenderMeshWorldPS_TextureClip : public FLGUIRenderMeshPS_TextureClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldPS_TextureClip, Global);

	FLGUIRenderMeshWorldPS_TextureClip() {}
	FLGUIRenderMeshWorldPS_TextureClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshPS_TextureClip(Initializer)
	{
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
		SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshPS_TextureClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLGUIRenderMeshWorldDepthFadePS_TextureClip : public FLGUIRenderMeshWorldPS_TextureClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWorldDepthFadePS_TextureClip, Global);

	FLGUIRenderMeshWorldDepthFadePS_TextureClip() {}
	FLGUIRenderMeshWorldDepthFadePS_TextureClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWorldPS_TextureClip(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
		FLGUIRenderMeshWorldPS_TextureClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade, const FVector2f& ViewSizeInv)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
		SetShaderValue(BatchedParameters, ViewSizeInvParameter, ViewSizeInv);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
	LAYOUT_FIELD(FShaderParameter, ViewSizeInvParameter);
};
//render mesh pixel shader, use a mask texture
class FLGUIRenderMeshWithMaskPS_TextureClip :public FLGUIRenderMeshWithMaskPS
{
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskPS_TextureClip, Global);
public:
	FLGUIRenderMeshWithMaskPS_TextureClip() {}
	FLGUIRenderMeshWithMaskPS_TextureClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskPS(Initializer)
	{
		OffsetAndSizeParameter.Bind(Initializer.ParameterMap, TEXT("_TextureClipOffsetAndSize"));
		ClipTextureParameter.Bind(Initializer.ParameterMap, TEXT("_ClipTex"));
		ClipTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_ClipTexSampler"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters & Parameters, FShaderCompilerEnvironment & OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_TEXTURE_CLIP"), 1);
		FLGUIRenderMeshWithMaskPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetClipParameters(FRHICommandListImmediate & RHICmdList, const FVector4f & OffsetAndSize
		, FTextureRHIRef ClipTexture
		, FRHISamplerState * ClipTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, OffsetAndSizeParameter, OffsetAndSize);
		SetTextureParameter(BatchedParameters, ClipTextureParameter, ClipTextureSamplerParameter, ClipTextureSampler, ClipTexture);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, OffsetAndSizeParameter);
	LAYOUT_FIELD(FShaderResourceParameter, ClipTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, ClipTextureSamplerParameter);
};
class FLGUIRenderMeshWithMaskWorldPS_TextureClip : public FLGUIRenderMeshWithMaskPS_TextureClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskWorldPS_TextureClip, Global);

	FLGUIRenderMeshWithMaskWorldPS_TextureClip() {}
	FLGUIRenderMeshWithMaskWorldPS_TextureClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskPS_TextureClip(Initializer)
	{
		SceneDepthTextureParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTex"));
		SceneDepthTextureSamplerParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTexSampler"));
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_BLEND_DEPTH"), true);
		FLGUIRenderMeshWithMaskPS_TextureClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetTextureParameter(BatchedParameters, SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureParameter);
	LAYOUT_FIELD(FShaderResourceParameter, SceneDepthTextureSamplerParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLGUIRenderMeshWithMaskWorldDepthFadePS_TextureClip : public FLGUIRenderMeshWithMaskWorldPS_TextureClip
{
public:
	DECLARE_SHADER_TYPE(FLGUIRenderMeshWithMaskWorldDepthFadePS_TextureClip, Global);

	FLGUIRenderMeshWithMaskWorldDepthFadePS_TextureClip() {}
	FLGUIRenderMeshWithMaskWorldDepthFadePS_TextureClip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIRenderMeshWithMaskWorldPS_TextureClip(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LGUI_DEPTH_FADE"), true);
		FLGUIRenderMeshWithMaskWorldPS_TextureClip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthFadeParameter(FRHICommandList& RHICmdList, int DepthFade, const FVector2f& ViewSizeInv)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, SceneDepthFadeParameter, DepthFade);
		SetShaderValue(BatchedParameters, ViewSizeInvParameter, ViewSizeInv);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthFadeParameter);
	LAYOUT_FIELD(FShaderParameter, ViewSizeInvParameter);
};
#pragma endregion