// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "Engine/Texture2D.h"
#include "RHIStaticStates.h"

// Uniform Buffer Declarations for Metal Shader Compilation
// Using BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT to properly bind textures/samplers
// PostProcess shaders uniform buffers
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FLexUIPostProcessMainTexUB, )
	SHADER_PARAMETER_TEXTURE(Texture2D, _MainTex)
	SHADER_PARAMETER_SAMPLER(SamplerState, _MainTexSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

// RenderMesh shaders uniform buffers
BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FLexUIRenderMeshMainTexUB, )
	SHADER_PARAMETER_TEXTURE(Texture2D, _MainTex)
	SHADER_PARAMETER_SAMPLER(SamplerState, _MainTexSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FLexUIRenderMeshMaskTexUB, )
	SHADER_PARAMETER_TEXTURE(Texture2D, _MaskTex)
	SHADER_PARAMETER_SAMPLER(SamplerState, _MaskTexSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FLexUIRenderMeshClipDataTexUB, )
	SHADER_PARAMETER_TEXTURE(Texture2D, _ClipDataTex)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

BEGIN_GLOBAL_SHADER_PARAMETER_STRUCT(FLexUIRenderMeshDepthTexUB, )
	SHADER_PARAMETER_TEXTURE(Texture2D, _SceneDepthTex)
	SHADER_PARAMETER_SAMPLER(SamplerState, _SceneDepthTexSampler)
END_GLOBAL_SHADER_PARAMETER_STRUCT()

class FLexUIPostProcessShader :public FGlobalShader
{
public:
	FLexUIPostProcessShader() {}
	FLexUIPostProcessShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
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
class FLexUISimplePostProcessVS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUISimplePostProcessVS, Global);
public:
	FLexUISimplePostProcessVS() {}
	FLexUISimplePostProcessVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{

	}
	void SetParameters(FRHICommandListImmediate& RHICmdList)
	{

	}
private:
};
class FLexUISimpleCopyTargetPS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUISimpleCopyTargetPS, Global);
public:
	FLexUISimpleCopyTargetPS() {}
	FLexUISimpleCopyTargetPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTextureRHIRef SceneTexture, FRHISamplerState* SceneTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		
		FLexUIPostProcessMainTexUB UB;
		UB._MainTex = SceneTexture;
		UB._MainTexSampler = SceneTextureSampler;
		TUniformBufferRef<FLexUIPostProcessMainTexUB> UniformBuffer = TUniformBufferRef<FLexUIPostProcessMainTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIPostProcessMainTexUB>(), UniformBuffer);
		
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
};
class FLexUISimpleCopyTargetPS_ColorCorrect : public FLexUISimpleCopyTargetPS
{
	DECLARE_SHADER_TYPE(FLexUISimpleCopyTargetPS_ColorCorrect, Global);
public:
	FLexUISimpleCopyTargetPS_ColorCorrect() {}
	FLexUISimpleCopyTargetPS_ColorCorrect(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUISimpleCopyTargetPS(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_COLORCORRECT"), true);
		FLexUISimpleCopyTargetPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
};
class FLexUISimpleCopyTargetPS_BlendAlpha : public FLexUISimpleCopyTargetPS
{
	DECLARE_SHADER_TYPE(FLexUISimpleCopyTargetPS_BlendAlpha, Global);
public:
	FLexUISimpleCopyTargetPS_BlendAlpha() {}
	FLexUISimpleCopyTargetPS_BlendAlpha(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUISimpleCopyTargetPS(Initializer)
	{
		BlendAlphaParameter.Bind(Initializer.ParameterMap, TEXT("_BlendAlpha"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_BLENDALPHA"), true);
		FLexUISimpleCopyTargetPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetBlendAlpha(FRHICommandListImmediate& RHICmdList, float BlendAlpha)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, BlendAlphaParameter, BlendAlpha);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, BlendAlphaParameter);
};
class FLexUIPostProcessGaussianBlurPS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUIPostProcessGaussianBlurPS, Global);
public:
	FLexUIPostProcessGaussianBlurPS() {}
	FLexUIPostProcessGaussianBlurPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
		BlurStrengthParameter.Bind(Initializer.ParameterMap, TEXT("_BlurStrength"));
	}
	void SetMainTexture(FRHICommandListImmediate& RHICmdList, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		
		FLexUIPostProcessMainTexUB UB;
		UB._MainTex = MainTexture;
		UB._MainTexSampler = MainTextureSampler;
		TUniformBufferRef<FLexUIPostProcessMainTexUB> UniformBuffer = TUniformBufferRef<FLexUIPostProcessMainTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIPostProcessMainTexUB>(), UniformBuffer);
		
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FLexUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetBlurStrength(FRHICommandListImmediate& RHICmdList, const FVector2f& BlurStrength)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, BlurStrengthParameter, BlurStrength);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, BlurStrengthParameter);
};




//render mesh region 
class FLexUICopyMeshRegionVS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUICopyMeshRegionVS, Global);
public:
	FLexUICopyMeshRegionVS() {}
	FLexUICopyMeshRegionVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
		
	}
};

//render mesh pixel shader
class FLexUICopyMeshRegionPS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUICopyMeshRegionPS, Global);
public:
	FLexUICopyMeshRegionPS() {}
	FLexUICopyMeshRegionPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
		MainTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_MainTextureScaleOffset"));
		MVPParameter.Bind(Initializer.ParameterMap, TEXT("_MVP"));
		IsRenderTargetParameter.Bind(Initializer.ParameterMap, TEXT("_IsRenderTarget"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix44f& MVP
		, bool bIsRenderTarget
		, FTextureRHIRef MainTexture, const FVector4f& MainTextureScaleOffset
		, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
		)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		
		FLexUIRenderMeshMainTexUB UB;
		UB._MainTex = MainTexture;
		UB._MainTexSampler = MainTextureSampler;
		TUniformBufferRef<FLexUIRenderMeshMainTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshMainTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshMainTexUB>(), UniformBuffer);
		
		SetShaderValue(BatchedParameters, MVPParameter, MVP);
		SetShaderValue(BatchedParameters, MainTextureScaleOffsetParameter, MainTextureScaleOffset);
		SetShaderValue(BatchedParameters, IsRenderTargetParameter, bIsRenderTarget ? 1.0f : 0.0f);
		
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, MainTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, MVPParameter);
	LAYOUT_FIELD(FShaderParameter, IsRenderTargetParameter);
};
class FLexUICopyMeshRegionPS_ColorCorrect : public FLexUICopyMeshRegionPS
{
	DECLARE_SHADER_TYPE(FLexUICopyMeshRegionPS_ColorCorrect, Global);
public:
	FLexUICopyMeshRegionPS_ColorCorrect() {}
	FLexUICopyMeshRegionPS_ColorCorrect(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUICopyMeshRegionPS(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_COLORCORRECT"), true);
		FLexUISimpleCopyTargetPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
};




//common render mesh vertex shader
class FLexUIRenderMeshVS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUIRenderMeshVS, Global);
public:
	FLexUIRenderMeshVS() {}
	FLexUIRenderMeshVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
		MVPParameter.Bind(Initializer.ParameterMap, TEXT("_MVP"));
		MParameter.Bind(Initializer.ParameterMap, TEXT("_M"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix44f& MVP, const FMatrix44f& M)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, MVPParameter, MVP);
		SetShaderValue(BatchedParameters, MParameter, M);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundVertexShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, MVPParameter);
	LAYOUT_FIELD(FShaderParameter, MParameter);
};
class FLexUIRenderMeshWorldVS : public FLexUIRenderMeshVS
{
public:
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWorldVS, Global);

	FLexUIRenderMeshWorldVS() {}
	FLexUIRenderMeshWorldVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshVS(Initializer)
	{

	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_BLEND_DEPTH"), true);
		FLexUIRenderMeshVS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
private:
};




//render mesh pixel shader
class FLexUIRenderMeshPS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUIRenderMeshPS, Global);
public:
	FLexUIRenderMeshPS() {}
	FLexUIRenderMeshPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_MASK"), 0);
		FLexUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, FTextureRHIRef MainTexture, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		
		FLexUIRenderMeshMainTexUB UB;
		UB._MainTex = MainTexture;
		UB._MainTexSampler = MainTextureSampler;
		TUniformBufferRef<FLexUIRenderMeshMainTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshMainTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshMainTexUB>(), UniformBuffer);
		
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
};

//render mesh pixel shader, use a mask texture
class FLexUIRenderMeshWithMaskPS :public FLexUIPostProcessShader
{
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWithMaskPS, Global);
public:
	FLexUIRenderMeshWithMaskPS() {}
	FLexUIRenderMeshWithMaskPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIPostProcessShader(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_MASK"), 1);
		FLexUIPostProcessShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList
		, FTextureRHIRef MainTexture
		, FTextureRHIRef MaskTexture
		, FRHISamplerState* MainTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
		, FRHISamplerState* MaskTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI()
	)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();

		{
			FLexUIRenderMeshMainTexUB UB;
			UB._MainTex = MainTexture;
			UB._MainTexSampler = MainTextureSampler;
			TUniformBufferRef<FLexUIRenderMeshMainTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshMainTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
			SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshMainTexUB>(), UniformBuffer);
		}

		{
			FLexUIRenderMeshMaskTexUB UB;
			UB._MaskTex = MaskTexture;
			UB._MaskTexSampler = MaskTextureSampler;
			TUniformBufferRef<FLexUIRenderMeshMaskTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshMaskTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
			SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshMaskTexUB>(), UniformBuffer);
		}
		
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
};

#pragma region Clip
//render mesh pixel shader
class FLexUIRenderMeshPS_Clip :public FLexUIRenderMeshPS
{
	DECLARE_SHADER_TYPE(FLexUIRenderMeshPS_Clip, Global);
public:
	FLexUIRenderMeshPS_Clip() {}
	FLexUIRenderMeshPS_Clip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshPS(Initializer)
	{
		InvMParameter.Bind(Initializer.ParameterMap, TEXT("_Inv_M"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_CLIP"), true);
		FLexUIRenderMeshPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetClipParameters(FRHICommandListImmediate& RHICmdList
		, const FMatrix44f& InvM
		, FTextureRHIRef ClipTexture)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();

		FLexUIRenderMeshClipDataTexUB UB;
		UB._ClipDataTex = ClipTexture;
		TUniformBufferRef<FLexUIRenderMeshClipDataTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshClipDataTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshClipDataTexUB>(), UniformBuffer);
		
		SetShaderValue(BatchedParameters, InvMParameter, InvM);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, InvMParameter);
};
class FLexUIRenderMeshWorldPS_Clip : public FLexUIRenderMeshPS_Clip
{
public:
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWorldPS_Clip, Global);

	FLexUIRenderMeshWorldPS_Clip() {}
	FLexUIRenderMeshWorldPS_Clip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshPS_Clip(Initializer)
	{
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_BLEND_DEPTH"), true);
		FLexUIRenderMeshPS_Clip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();

		FLexUIRenderMeshDepthTexUB UB;
		UB._SceneDepthTex = DepthTexture;
		UB._SceneDepthTexSampler = DepthTextureSampler;
		TUniformBufferRef<FLexUIRenderMeshDepthTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshDepthTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshDepthTexUB>(), UniformBuffer);
		
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLexUIRenderMeshWorldDepthFadePS_Clip : public FLexUIRenderMeshWorldPS_Clip
{
public:
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWorldDepthFadePS_Clip, Global);

	FLexUIRenderMeshWorldDepthFadePS_Clip() {}
	FLexUIRenderMeshWorldDepthFadePS_Clip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshWorldPS_Clip(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_DEPTH_FADE"), true);
		FLexUIRenderMeshWorldPS_Clip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
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
class FLexUIRenderMeshWithMaskPS_Clip :public FLexUIRenderMeshWithMaskPS
{
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWithMaskPS_Clip, Global);
public:
	FLexUIRenderMeshWithMaskPS_Clip() {}
	FLexUIRenderMeshWithMaskPS_Clip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshWithMaskPS(Initializer)
	{
		InvMParameter.Bind(Initializer.ParameterMap, TEXT("_Inv_M"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters & Parameters, FShaderCompilerEnvironment & OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_CLIP"), true);
		FLexUIRenderMeshWithMaskPS::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetClipParameters(FRHICommandListImmediate & RHICmdList
		, const FMatrix44f& InvM
		, FTextureRHIRef ClipTexture
		, FRHISamplerState * ClipTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();

		FLexUIRenderMeshClipDataTexUB UB;
		UB._ClipDataTex = ClipTexture;
		TUniformBufferRef<FLexUIRenderMeshClipDataTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshClipDataTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshClipDataTexUB>(), UniformBuffer);
		
		SetShaderValue(BatchedParameters, InvMParameter, InvM);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, InvMParameter);
};
class FLexUIRenderMeshWithMaskWorldPS_Clip : public FLexUIRenderMeshWithMaskPS_Clip
{
public:
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWithMaskWorldPS_Clip, Global);

	FLexUIRenderMeshWithMaskWorldPS_Clip() {}
	FLexUIRenderMeshWithMaskWorldPS_Clip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshWithMaskPS_Clip(Initializer)
	{
		SceneDepthTextureScaleOffsetParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthTextureScaleOffset"));
		SceneDepthBlendParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthBlend"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_BLEND_DEPTH"), true);
		FLexUIRenderMeshWithMaskPS_Clip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
	void SetDepthBlendParameter(FRHICommandList& RHICmdList, float DepthBlend, const FVector4f& DepthTextureScaleOffset, FRHITexture* DepthTexture, FRHISamplerState* DepthTextureSampler = TStaticSamplerState<SF_Bilinear, AM_Clamp, AM_Clamp, AM_Clamp>::GetRHI())
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		
		FLexUIRenderMeshDepthTexUB UB;
		UB._SceneDepthTex = DepthTexture;
		UB._SceneDepthTexSampler = DepthTextureSampler;
		TUniformBufferRef<FLexUIRenderMeshDepthTexUB> UniformBuffer = TUniformBufferRef<FLexUIRenderMeshDepthTexUB>::CreateUniformBufferImmediate(UB, UniformBuffer_SingleFrame);
		SetUniformBufferParameter(BatchedParameters, GetUniformBufferParameter<FLexUIRenderMeshDepthTexUB>(), UniformBuffer);
		
		SetShaderValue(BatchedParameters, SceneDepthBlendParameter, DepthBlend);
		SetShaderValue(BatchedParameters, SceneDepthTextureScaleOffsetParameter, DepthTextureScaleOffset);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundPixelShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, SceneDepthTextureScaleOffsetParameter);
	LAYOUT_FIELD(FShaderParameter, SceneDepthBlendParameter);
};
class FLexUIRenderMeshWithMaskWorldDepthFadePS_Clip : public FLexUIRenderMeshWithMaskWorldPS_Clip
{
public:
	DECLARE_SHADER_TYPE(FLexUIRenderMeshWithMaskWorldDepthFadePS_Clip, Global);

	FLexUIRenderMeshWithMaskWorldDepthFadePS_Clip() {}
	FLexUIRenderMeshWithMaskWorldDepthFadePS_Clip(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLexUIRenderMeshWithMaskWorldPS_Clip(Initializer)
	{
		SceneDepthFadeParameter.Bind(Initializer.ParameterMap, TEXT("_SceneDepthFade"));
		ViewSizeInvParameter.Bind(Initializer.ParameterMap, TEXT("_ViewSizeInv"));
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		OutEnvironment.SetDefine(TEXT("LEXUI_DEPTH_FADE"), true);
		FLexUIRenderMeshWithMaskWorldPS_Clip::ModifyCompilationEnvironment(Parameters, OutEnvironment);
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