// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/Render/LGUIShaders.h"
#include "LGUI.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "ScenePrivate.h"

IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIHudRenderVS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIHudRenderPS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainPS"), SF_Pixel);


FLGUIHudRenderVS::FLGUIHudRenderVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer) 
	: FMeshMaterialShader(Initializer)
{
	PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTexturesUniformParameters::StaticStructMetadata.GetShaderVariableName());
}
bool FLGUIHudRenderVS::ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType)
{
	return true;
}
void FLGUIHudRenderVS::GetShaderBindings(
	const FScene* Scene,
	ERHIFeatureLevel::Type FeatureLevel,
	const FPrimitiveSceneProxy* PrimitiveSceneProxy,
	const FMaterialRenderProxy& MaterialRenderProxy,
	const FMaterial& Material,
	const FMeshPassProcessorRenderState& DrawRenderState,
	const FMeshMaterialShaderElementData& ShaderElementData,
	FMeshDrawSingleShaderBindings& ShaderBindings) const
{
	FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
}
void FLGUIHudRenderVS::GetElementShaderBindings(
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
	FVertexInputStreamArray& VertexStreams) const
{
	checkSlow(ShaderBindings.Frequency == GetType()->GetFrequency());

	GetVertexFactoryParameterRef()->GetElementShaderBindings(Scene, ViewIfDynamicMeshCommand, this, bShaderRequiresPositionOnlyStream, FeatureLevel, VertexFactory, BatchElement, ShaderBindings, VertexStreams);

	if (BatchElement.PrimitiveUniformBuffer)
	{
		ShaderBindings.Add(GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), BatchElement.PrimitiveUniformBuffer);
	}
	else
	{
		checkf(BatchElement.PrimitiveUniformBufferResource, TEXT("%s expected a primitive uniform buffer but none was set on BatchElement.PrimitiveUniformBuffer or BatchElement.PrimitiveUniformBufferResource"), GetType()->GetName());
		ShaderBindings.Add(GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), BatchElement.PrimitiveUniformBufferResource->GetUniformBufferRHI());
	}
}

FLGUIHudRenderPS::FLGUIHudRenderPS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FMeshMaterialShader(Initializer)
{
	PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTexturesUniformParameters::StaticStructMetadata.GetShaderVariableName());
}
bool FLGUIHudRenderPS::ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType)
{
	return true;
}
void FLGUIHudRenderPS::GetShaderBindings(
	const FScene* Scene,
	ERHIFeatureLevel::Type FeatureLevel,
	const FPrimitiveSceneProxy* PrimitiveSceneProxy,
	const FMaterialRenderProxy& MaterialRenderProxy,
	const FMaterial& Material,
	const FMeshPassProcessorRenderState& DrawRenderState,
	const FMeshMaterialShaderElementData& ShaderElementData,
	FMeshDrawSingleShaderBindings& ShaderBindings) const
{
	FMeshMaterialShader::GetShaderBindings(Scene, FeatureLevel, PrimitiveSceneProxy, MaterialRenderProxy, Material, DrawRenderState, ShaderElementData, ShaderBindings);
}
void FLGUIHudRenderPS::GetElementShaderBindings(
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
	FVertexInputStreamArray& VertexStreams) const
{
	checkSlow(ShaderBindings.Frequency == GetType()->GetFrequency());

	GetVertexFactoryParameterRef()->GetElementShaderBindings(Scene, ViewIfDynamicMeshCommand, this, bShaderRequiresPositionOnlyStream, FeatureLevel, VertexFactory, BatchElement, ShaderBindings, VertexStreams);

	if (BatchElement.PrimitiveUniformBuffer)
	{
		ShaderBindings.Add(GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), BatchElement.PrimitiveUniformBuffer);
	}
	else
	{
		checkf(BatchElement.PrimitiveUniformBufferResource, TEXT("%s expected a primitive uniform buffer but none was set on BatchElement.PrimitiveUniformBuffer or BatchElement.PrimitiveUniformBufferResource"), GetType()->GetName());
		ShaderBindings.Add(GetUniformBufferParameter<FPrimitiveUniformShaderParameters>(), BatchElement.PrimitiveUniformBufferResource->GetUniformBufferRHI());
	}
}