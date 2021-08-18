// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, SceneTextureSampler, SceneTexture);
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
	void SetMainTexture(FRHICommandListImmediate& RHICmdList, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
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
		SetTextureParameter(RHICmdList, GetPixelShader(), StrengthTextureParameter, StrengthTextureSamplerParameter, StrengthTextureSampler, StrengthTexture);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("USE_STRENGTH_TEXTURE"), 1);
		FLGUIPostProcessGaussianBlurPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessGaussianBlurPS::Serialize(Ar);
		Ar << StrengthTextureParameter;
		Ar << StrengthTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter StrengthTextureParameter;
	FShaderResourceParameter StrengthTextureSamplerParameter;
};

class FLGUIPostProcessCustomDepthMaskPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIPostProcessCustomDepthMaskPS, Global);
public:
	FLGUIPostProcessCustomDepthMaskPS() {}
	FLGUIPostProcessCustomDepthMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		_ScreenTex.Bind(Initializer.ParameterMap, TEXT("_ScreenTex"));
		_ScreenTexSampler.Bind(Initializer.ParameterMap, TEXT("_ScreenTexSampler"));
		_OriginScreenTex.Bind(Initializer.ParameterMap, TEXT("_OriginScreenTex"));
		_OriginScreenTexSampler.Bind(Initializer.ParameterMap, TEXT("_OriginScreenTexSampler"));
		_CustomDepthTex.Bind(Initializer.ParameterMap, TEXT("_CustomDepthTex"));
		_CustomDepthTexSampler.Bind(Initializer.ParameterMap, TEXT("_CustomDepthTexSampler"));
		_MaskStrength.Bind(Initializer.ParameterMap, TEXT("_MaskStrength"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList
		, FTextureRHIRef ScreenTexture, FRHISamplerState* ScreenTextureSampler
		, FTextureRHIRef OriginScreenTexture, FRHISamplerState* OriginScreenTextureSampler
		, FTextureRHIRef CustomDepthTexture, FRHISamplerState* CustomDepthTextureSampler
		, float MaskStrength
	)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), _ScreenTex, _ScreenTexSampler, ScreenTextureSampler, ScreenTexture);
		SetTextureParameter(RHICmdList, GetPixelShader(), _OriginScreenTex, _OriginScreenTexSampler, OriginScreenTextureSampler, OriginScreenTexture);
		SetTextureParameter(RHICmdList, GetPixelShader(), _CustomDepthTex, _CustomDepthTexSampler, CustomDepthTextureSampler, CustomDepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), _MaskStrength, MaskStrength);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << _ScreenTex;
		Ar << _ScreenTexSampler;
		Ar << _OriginScreenTex;
		Ar << _OriginScreenTexSampler;
		Ar << _CustomDepthTex;
		Ar << _CustomDepthTexSampler;
		Ar << _MaskStrength;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter _ScreenTex;
	FShaderResourceParameter _ScreenTexSampler;
	FShaderResourceParameter _OriginScreenTex;
	FShaderResourceParameter _OriginScreenTexSampler;
	FShaderResourceParameter _CustomDepthTex;
	FShaderResourceParameter _CustomDepthTexSampler;
	FShaderParameter _MaskStrength;
};
class FLGUIPostProcessCustomDepthStencilMaskPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIPostProcessCustomDepthStencilMaskPS, Global);
public:
	FLGUIPostProcessCustomDepthStencilMaskPS() {}
	FLGUIPostProcessCustomDepthStencilMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		_ScreenTex.Bind(Initializer.ParameterMap, TEXT("_ScreenTex"));
		_ScreenTexSampler.Bind(Initializer.ParameterMap, TEXT("_ScreenTexSampler"));
		_OriginScreenTex.Bind(Initializer.ParameterMap, TEXT("_OriginScreenTex"));
		_OriginScreenTexSampler.Bind(Initializer.ParameterMap, TEXT("_OriginScreenTexSampler"));
		_CustomStencilTex.Bind(Initializer.ParameterMap, TEXT("_CustomStencilTex"));
		_StencilValue.Bind(Initializer.ParameterMap, TEXT("_StencilValue"));
		_TextureSize.Bind(Initializer.ParameterMap, TEXT("_TextureSize"));
		_MaskStrength.Bind(Initializer.ParameterMap, TEXT("_MaskStrength"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList
		, FTextureRHIRef ScreenTexture, FRHISamplerState* ScreenTextureSampler
		, FTextureRHIRef OriginScreenTexture, FRHISamplerState* OriginScreenTextureSampler
		, FRHIShaderResourceView* StencilResourceView
		, int StencilValue
		, int screenWidth, int screenHeight
		, float MaskStrength
	)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), _ScreenTex, _ScreenTexSampler, ScreenTextureSampler, ScreenTexture);
		SetTextureParameter(RHICmdList, GetPixelShader(), _OriginScreenTex, _OriginScreenTexSampler, OriginScreenTextureSampler, OriginScreenTexture);
		SetSRVParameter(RHICmdList, GetPixelShader(), _CustomStencilTex, StencilResourceView);
		SetShaderValue(RHICmdList, GetPixelShader(), _StencilValue, StencilValue);
		SetShaderValue(RHICmdList, GetPixelShader(), _TextureSize, FIntVector(screenWidth, screenHeight, 0));
		SetShaderValue(RHICmdList, GetPixelShader(), _MaskStrength, MaskStrength);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << _ScreenTex;
		Ar << _ScreenTexSampler;
		Ar << _OriginScreenTex;
		Ar << _OriginScreenTexSampler;
		Ar << _CustomStencilTex;
		Ar << _StencilValue;
		Ar << _TextureSize;
		Ar << _MaskStrength;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter _ScreenTex;
	FShaderResourceParameter _ScreenTexSampler;
	FShaderResourceParameter _OriginScreenTex;
	FShaderResourceParameter _OriginScreenTexSampler;
	FShaderResourceParameter _CustomStencilTex;
	FShaderParameter _StencilValue;
	FShaderParameter _TextureSize;
	FShaderParameter _MaskStrength;
};
class FLGUIPostProcessMobileCustomDepthStencilMaskPS :public FLGUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLGUIPostProcessMobileCustomDepthStencilMaskPS, Global);
public:
	FLGUIPostProcessMobileCustomDepthStencilMaskPS() {}
	FLGUIPostProcessMobileCustomDepthStencilMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIPostProcessShader(Initializer)
	{
		_ScreenTex.Bind(Initializer.ParameterMap, TEXT("_ScreenTex"));
		_ScreenTexSampler.Bind(Initializer.ParameterMap, TEXT("_ScreenTexSampler"));
		_OriginScreenTex.Bind(Initializer.ParameterMap, TEXT("_OriginScreenTex"));
		_OriginScreenTexSampler.Bind(Initializer.ParameterMap, TEXT("_OriginScreenTexSampler"));
		_MobileCustomStencilTex.Bind(Initializer.ParameterMap, TEXT("_MobileCustomStencilTex"));
		_MobileCustomStencilTexSampler.Bind(Initializer.ParameterMap, TEXT("_MobileCustomStencilTexSampler"));
		_StencilValue.Bind(Initializer.ParameterMap, TEXT("_StencilValue"));
		_MaskStrength.Bind(Initializer.ParameterMap, TEXT("_MaskStrength"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList
		, FTextureRHIRef ScreenTexture, FRHISamplerState* ScreenTextureSampler
		, FTextureRHIRef OriginScreenTexture, FRHISamplerState* OriginScreenTextureSampler
		, FTextureRHIRef CustomDepthTexture, FRHISamplerState* CustomDepthTextureSampler
		, int StencilValue
		, float MaskStrength
	)
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), _ScreenTex, _ScreenTexSampler, ScreenTextureSampler, ScreenTexture);
		SetTextureParameter(RHICmdList, GetPixelShader(), _OriginScreenTex, _OriginScreenTexSampler, OriginScreenTextureSampler, OriginScreenTexture);
		SetTextureParameter(RHICmdList, GetPixelShader(), _MobileCustomStencilTex, _MobileCustomStencilTexSampler, CustomDepthTextureSampler, CustomDepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), _StencilValue, StencilValue);
		SetShaderValue(RHICmdList, GetPixelShader(), _MaskStrength, MaskStrength);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLGUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << _ScreenTex;
		Ar << _ScreenTexSampler;
		Ar << _OriginScreenTex;
		Ar << _OriginScreenTexSampler;
		Ar << _MobileCustomStencilTex;
		Ar << _MobileCustomStencilTexSampler;
		Ar << _StencilValue;
		Ar << _MaskStrength;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter _ScreenTex;
	FShaderResourceParameter _ScreenTexSampler;
	FShaderResourceParameter _OriginScreenTex;
	FShaderResourceParameter _OriginScreenTexSampler;
	FShaderResourceParameter _MobileCustomStencilTex;
	FShaderResourceParameter _MobileCustomStencilTexSampler;
	FShaderParameter _StencilValue;
	FShaderParameter _MaskStrength;
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
		MVPParameter.Bind(Initializer.ParameterMap, TEXT("_MVP"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix& MVP, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), MVPParameter, MVP);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		Ar << MVPParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
	FShaderParameter MVPParameter;
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
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix& MVP)
	{
		SetShaderValue(RHICmdList, GetVertexShader(), MVPParameter, MVP);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MVPParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter MVPParameter;
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
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
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
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshPS::Serialize(Ar);
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthTextureSamplerParameter;
		Ar << SceneDepthBlendParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
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
		SetTextureParameter(RHICmdList, GetPixelShader(), MainTextureParameter, MainTextureSamplerParameter, MainTextureSampler, MainTexture);
		SetTextureParameter(RHICmdList, GetPixelShader(), MaskTextureParameter, MaskTextureSamplerParameter, MaskTextureSampler, MaskTexture);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIPostProcessShader::Serialize(Ar);
		Ar << MainTextureParameter;
		Ar << MainTextureSamplerParameter;
		Ar << MaskTextureParameter;
		Ar << MaskTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter MainTextureParameter;
	FShaderResourceParameter MainTextureSamplerParameter;
	FShaderResourceParameter MaskTextureParameter;
	FShaderResourceParameter MaskTextureSamplerParameter;
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
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshWithMaskPS::Serialize(Ar);
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthTextureSamplerParameter;
		Ar << SceneDepthBlendParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
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
	void SetClipParameters(FRHICommandListImmediate& RHICmdList, const FVector4& OffsetAndSize, const FVector4& Feather)
	{
		SetShaderValue(RHICmdList, GetPixelShader(), OffsetAndSizeParameter, OffsetAndSize);
		SetShaderValue(RHICmdList, GetPixelShader(), FeatherParameter, Feather);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshPS::Serialize(Ar);
		Ar << OffsetAndSizeParameter;
		Ar << FeatherParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter OffsetAndSizeParameter;
	FShaderParameter FeatherParameter;
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
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshPS_RectClip::Serialize(Ar);
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthTextureSamplerParameter;
		Ar << SceneDepthBlendParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
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
	void SetClipParameters(FRHICommandListImmediate& RHICmdList, const FVector4& OffsetAndSize, const FVector4& Feather)
	{
		SetShaderValue(RHICmdList, GetPixelShader(), OffsetAndSizeParameter, OffsetAndSize);
		SetShaderValue(RHICmdList, GetPixelShader(), FeatherParameter, Feather);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshWithMaskPS::Serialize(Ar);
		Ar << OffsetAndSizeParameter;
		Ar << FeatherParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter OffsetAndSizeParameter;
	FShaderParameter FeatherParameter;
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
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshWithMaskPS_RectClip::Serialize(Ar);
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthTextureSamplerParameter;
		Ar << SceneDepthBlendParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
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
	void SetClipParameters(FRHICommandListImmediate& RHICmdList, const FVector4& OffsetAndSize
		, FTextureRHIRef ClipTexture
		, FRHISamplerState* ClipTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetShaderValue(RHICmdList, GetPixelShader(), OffsetAndSizeParameter, OffsetAndSize);
		SetTextureParameter(RHICmdList, GetPixelShader(), ClipTextureParameter, ClipTextureSamplerParameter, ClipTextureSampler, ClipTexture);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshPS::Serialize(Ar);
		Ar << OffsetAndSizeParameter;
		Ar << ClipTextureParameter;
		Ar << ClipTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter OffsetAndSizeParameter;
	FShaderResourceParameter ClipTextureParameter;
	FShaderResourceParameter ClipTextureSamplerParameter;
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
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshPS_TextureClip::Serialize(Ar);
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthTextureSamplerParameter;
		Ar << SceneDepthBlendParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
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
	void SetClipParameters(FRHICommandListImmediate & RHICmdList, const FVector4 & OffsetAndSize
		, FTextureRHIRef ClipTexture
		, FRHISamplerState * ClipTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetShaderValue(RHICmdList, GetPixelShader(), OffsetAndSizeParameter, OffsetAndSize);
		SetTextureParameter(RHICmdList, GetPixelShader(), ClipTextureParameter, ClipTextureSamplerParameter, ClipTextureSampler, ClipTexture);
	}
	virtual bool Serialize(FArchive & Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshWithMaskPS::Serialize(Ar);
		Ar << OffsetAndSizeParameter;
		Ar << ClipTextureParameter;
		Ar << ClipTextureSamplerParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderParameter OffsetAndSizeParameter;
	FShaderResourceParameter ClipTextureParameter;
	FShaderResourceParameter ClipTextureSamplerParameter;
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
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4& DepthTextureScaleOffset, const FTexture2DRHIRef& DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		SetTextureParameter(RHICmdList, GetPixelShader(), SceneDepthTextureParameter, SceneDepthTextureSamplerParameter, DepthTextureSampler, DepthTexture);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(RHICmdList, GetPixelShader(), SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
	}
	virtual bool Serialize(FArchive& Ar)override
	{
		bool bShaderHasOutdatedParameters = FLGUIRenderMeshWithMaskPS_TextureClip::Serialize(Ar);
		Ar << SceneDepthTextureParameter;
		Ar << SceneDepthTextureSamplerParameter;
		Ar << SceneDepthBlendParameter;
		return bShaderHasOutdatedParameters;
	}
private:
	FShaderResourceParameter SceneDepthTextureParameter;
	FShaderResourceParameter SceneDepthTextureSamplerParameter;
	FShaderParameter SceneDepthTextureScaleOffsetParameter;
	FShaderParameter SceneDepthBlendParameter;
};
#pragma endregion