// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "SceneViewExtension.h"
#include "RendererInterface.h"
#include "RenderResource.h"
#include "Shader.h"
#include "ShaderParameterUtils.h"
#include "MaterialShaderType.h"
#include "MaterialShader.h"
#include "Engine/Texture2D.h"
#include "MeshMaterialShader.h"

class FLGUIVertexDeclaration : public FRenderResource
{
public:
	FVertexDeclarationRHIRef VertexDeclarationRHI;
	virtual ~FLGUIVertexDeclaration() {}
	virtual void InitRHI()override;
	virtual void ReleaseRHI()override;
};

class FLGUIHudRenderVS :public FMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FLGUIHudRenderVS, Material);

	FLGUIHudRenderVS() {}
	FLGUIHudRenderVS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);

	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const class FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment);
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material);
	
	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);
	virtual bool Serialize(FArchive& Ar)override;
};
class FLGUIHudRenderPS : public FMaterialShader
{
public:
	DECLARE_SHADER_TYPE(FLGUIHudRenderPS, Material);

	FLGUIHudRenderPS() {}
	FLGUIHudRenderPS(const FMaterialShaderType::CompiledShaderInitializerType& Initializer);
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material);
	static void ModifyCompilationEnvironment(EShaderPlatform Platform, const FMaterial* Material, FShaderCompilerEnvironment& OutEnvironment);

	void SetBlendState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material);
	void SetMaterialShaderParameters(FRHICommandList& RHICmdList, const FSceneView& View, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial* Material, const FMeshBatch& Mesh);

	virtual bool Serialize(FArchive& Ar) override;
};


extern TGlobalResource<FLGUIVertexDeclaration> GLGUIVertexDeclaration;