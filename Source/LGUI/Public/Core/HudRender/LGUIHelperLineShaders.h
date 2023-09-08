// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "MaterialShader.h"
#include "Engine/Texture2D.h"

class FLGUIHelperLineShader :public FGlobalShader
{
public:
	FLGUIHelperLineShader() {}
	FLGUIHelperLineShader(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
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


class FLGUIHelperLineShaderVS :public FLGUIHelperLineShader
{
	DECLARE_SHADER_TYPE(FLGUIHelperLineShaderVS, Global);
public:
	FLGUIHelperLineShaderVS() {}
	FLGUIHelperLineShaderVS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIHelperLineShader(Initializer)
	{
		VPParameter.Bind(Initializer.ParameterMap, TEXT("_VP"));
	}
	void SetParameters(FRHICommandListImmediate& RHICmdList, const FMatrix44f& VP)
	{
		FRHIBatchedShaderParameters& BatchedParameters = RHICmdList.GetScratchShaderParameters();
		SetShaderValue(BatchedParameters, VPParameter, VP);
		RHICmdList.SetBatchedShaderParameters(RHICmdList.GetBoundVertexShader(), BatchedParameters);
	}
private:
	LAYOUT_FIELD(FShaderParameter, VPParameter);
};
class FLGUIHelperLineShaderPS :public FLGUIHelperLineShader
{
	DECLARE_SHADER_TYPE(FLGUIHelperLineShaderPS, Global);
public:
	FLGUIHelperLineShaderPS() {}
	FLGUIHelperLineShaderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer)
		: FLGUIHelperLineShader(Initializer)
	{

	}
private:
};

