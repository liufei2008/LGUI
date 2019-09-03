// Copyright 2019 LexLiu. All Rights Reserved.

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
	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const;
	void GetElementShaderBindings(
		const FScene* Scene,
		const FSceneView* ViewIfDynamicMeshCommand,
		const FVertexFactory* VertexFactory,
		bool bShaderRequiresPositionOnlyStream,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMeshBatch& MeshBatch,
		const FMeshBatchElement& BatchElement,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const;
};
class FLGUIHudRenderPS : public FMeshMaterialShader
{
	DECLARE_SHADER_TYPE(FLGUIHudRenderPS, MeshMaterial);
public:
	FLGUIHudRenderPS() {}
	FLGUIHudRenderPS(const ShaderMetaType::CompiledShaderInitializerType& Initializer);
	static bool ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType);
	void GetShaderBindings(
		const FScene* Scene,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMaterialRenderProxy& MaterialRenderProxy,
		const FMaterial& Material,
		const FMeshPassProcessorRenderState& DrawRenderState,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings) const;
	void GetElementShaderBindings(
		const FScene* Scene,
		const FSceneView* ViewIfDynamicMeshCommand,
		const FVertexFactory* VertexFactory,
		bool bShaderRequiresPositionOnlyStream,
		ERHIFeatureLevel::Type FeatureLevel,
		const FPrimitiveSceneProxy* PrimitiveSceneProxy,
		const FMeshBatch& MeshBatch,
		const FMeshBatchElement& BatchElement,
		const FMeshMaterialShaderElementData& ShaderElementData,
		FMeshDrawSingleShaderBindings& ShaderBindings,
		FVertexInputStreamArray& VertexStreams) const;
};
