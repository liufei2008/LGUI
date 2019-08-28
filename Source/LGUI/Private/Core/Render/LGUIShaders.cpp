// Copyright 2019 LexLiu. All Rights Reserved.

#include "Render/LGUIShaders.h"
#include "LGUI.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"

IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIHudRenderVS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainVS"), SF_Vertex);
IMPLEMENT_MATERIAL_SHADER_TYPE(, FLGUIHudRenderPS, TEXT("/Plugin/LGUI/Private/LGUIHudShader.usf"), TEXT("MainPS"), SF_Pixel);


FLGUIHudRenderVS::FLGUIHudRenderVS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer) 
	: FMeshMaterialShader(Initializer)
{
	PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTexturesUniformParameters::StaticStruct.GetShaderVariableName());
}
bool FLGUIHudRenderVS::ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType)
{
	return true;
}
void FLGUIHudRenderVS::SetParameters(FRHICommandList& RHICmdList, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial& Material, const FSceneView& View, const TUniformBufferRef<FViewUniformShaderParameters>& ViewUniformBuffer)
{
	FMeshMaterialShader::SetParameters(RHICmdList, GetVertexShader(), MaterialRenderProxy, Material, View, ViewUniformBuffer, nullptr);
}
void FLGUIHudRenderVS::SetMesh(FRHICommandList& RHICmdList, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, const FDrawingPolicyRenderState& DrawRenderState, uint32 DataFlags)
{
	FMeshMaterialShader::SetMesh(RHICmdList, GetVertexShader(), VertexFactory, View, nullptr, BatchElement, DrawRenderState, DataFlags);
}


FLGUIHudRenderPS::FLGUIHudRenderPS(const FMeshMaterialShaderType::CompiledShaderInitializerType& Initializer)
	:FMeshMaterialShader(Initializer)
{
	PassUniformBuffer.Bind(Initializer.ParameterMap, FSceneTexturesUniformParameters::StaticStruct.GetShaderVariableName());
}
bool FLGUIHudRenderPS::ShouldCompilePermutation(EShaderPlatform Platform, const FMaterial* Material, const FVertexFactoryType* VertexFactoryType)
{
	return true;
}
void FLGUIHudRenderPS::SetParameters(FRHICommandList& RHICmdList, const FMaterialRenderProxy* MaterialRenderProxy, const FMaterial& Material, const FSceneView& View, const TUniformBufferRef<FViewUniformShaderParameters>& ViewUniformBuffer)
{
	FMeshMaterialShader::SetParameters(RHICmdList, GetPixelShader(), MaterialRenderProxy, Material, View, ViewUniformBuffer, nullptr);
}
void FLGUIHudRenderPS::SetMesh(FRHICommandList& RHICmdList, const FVertexFactory* VertexFactory, const FSceneView& View, const FMeshBatchElement& BatchElement, const FDrawingPolicyRenderState& DrawRenderState, uint32 DataFlags)
{
	FMeshMaterialShader::SetMesh(RHICmdList, GetPixelShader(), VertexFactory, View, nullptr, BatchElement, DrawRenderState, DataFlags);
}