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

class FLGUIHudRenderVS :public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FLGUIHudRenderVS, MeshMaterial);
public:
	FLGUIHudRenderVS() {}
	FLGUIHudRenderVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer);

	static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType);
	void SetParameters(FRHICommandList& RHICmdList, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial& Material, const FSceneView& View, const TUniformBufferRef<FViewUniformShaderParameters>& ViewUniformBuffer);
	void SetMesh(FRHICommandList& RHICmdList, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, const FDrawingPolicyRenderState& DrawRenderState, uint32 DataFlags = 0);
};
class FLGUIHudRenderPS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FLGUIHudRenderPS, MeshMaterial);
public:
	FLGUIHudRenderPS() {}
	FLGUIHudRenderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType);
	void SetParameters(FRHICommandList& RHICmdList, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial& Material, const FSceneView& View, const TUniformBufferRef<FViewUniformShaderParameters>& ViewUniformBuffer);
	void SetMesh(FRHICommandList& RHICmdList, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, const FDrawingPolicyRenderState& DrawRenderState, uint32 DataFlags = 0);
};
