// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "MaterialShader.h"
#include "Engine/Texture2D.h"

class FLGUIResolveShaderVS :public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLGUIResolveShaderVS, Global);
public:
	FLGUIResolveShaderVS() {}
	FLGUIResolveShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
	}
};

class FLGUIResolveShader2xPS :public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLGUIResolveShader2xPS, Global);
public:
	FLGUIResolveShader2xPS() {}
	FLGUIResolveShader2xPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		Tex.Bind(Initializer.ParameterMap, TEXT("Tex"), SPF_Mandatory);
	}
	void SetParameters(FRHICommandList& RHICmdList, FRHITexture* Texture2DMS)
	{
		FRHIPixelShader* PixelShaderRHI = RHICmdList.GetBoundPixelShader();
		SetTextureParameter(RHICmdList, PixelShaderRHI, Tex, Texture2DMS);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("LGUI_RESOLVE_2X"), 1);
	}
protected:
	LAYOUT_FIELD(FShaderResourceParameter, Tex);
};
class FLGUIResolveShader4xPS :public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLGUIResolveShader4xPS, Global);
public:
	FLGUIResolveShader4xPS() {}
	FLGUIResolveShader4xPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		Tex.Bind(Initializer.ParameterMap, TEXT("Tex"), SPF_Mandatory);
	}
	void SetParameters(FRHICommandList& RHICmdList, FRHITexture* Texture2DMS)
	{
		FRHIPixelShader* PixelShaderRHI = RHICmdList.GetBoundPixelShader();
		SetTextureParameter(RHICmdList, PixelShaderRHI, Tex, Texture2DMS);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("LGUI_RESOLVE_4X"), 1);
	}
protected:
	LAYOUT_FIELD(FShaderResourceParameter, Tex);
};
class FLGUIResolveShader8xPS :public FGlobalShader
{
	DECLARE_SHADER_TYPE(FLGUIResolveShader8xPS, Global);
public:
	FLGUIResolveShader8xPS() {}
	FLGUIResolveShader8xPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FGlobalShader(Initializer)
	{
		Tex.Bind(Initializer.ParameterMap, TEXT("Tex"), SPF_Mandatory);
	}
	void SetParameters(FRHICommandList& RHICmdList, FRHITexture* Texture2DMS)
	{
		FRHIPixelShader* PixelShaderRHI = RHICmdList.GetBoundPixelShader();
		SetTextureParameter(RHICmdList, PixelShaderRHI, Tex, Texture2DMS);
	}
	static void ModifyCompilationEnvironment(const FGlobalShaderPermutationParameters& Parameters, FShaderCompilerEnvironment& OutEnvironment)
	{
		FGlobalShader::ModifyCompilationEnvironment(Parameters, OutEnvironment);
		OutEnvironment.SetDefine(TEXT("LGUI_RESOLVE_8X"), 1);
	}
protected:
	LAYOUT_FIELD(FShaderResourceParameter, Tex);
};
