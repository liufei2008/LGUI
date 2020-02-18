﻿// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Render/LGUIRenderer.h"
#include "Core/Render/LGUIHudShaders.h"
#include "Core/Render/LGUIHudVertex.h"
#include "Modules/ModuleManager.h"
#include "LGUI.h"
#include "SceneView.h"
#include "Widgets/SWindow.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "Core/Render/ILGUIHudPrimitive.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "MeshPassProcessor.inl"
#include "ScenePrivate.h"
#if WITH_EDITOR
#include "Engine.h"
#include "Editor/EditorEngine.h"
#endif
#include "Slate/SceneViewport.h"
#include "Core/ActorComponent/UIPostProcess.h"
class FLGUIMeshElementCollector : FMeshElementCollector
{
public:
	FLGUIMeshElementCollector(ERHIFeatureLevel::Type InFeatureLevel) :FMeshElementCollector(InFeatureLevel)
	{

	}
};



FLGUIViewExtension::FLGUIViewExtension(const FAutoRegister& AutoRegister, ULGUICanvas* InLGUICanvas)
	:FSceneViewExtensionBase(AutoRegister)
{
	UICanvas = InLGUICanvas;
}
FLGUIViewExtension::~FLGUIViewExtension()
{
	
}

void FLGUIViewExtension::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	TArray<ILGUIHudPrimitive*> primitiveArray;
	{
		FScopeLock scopeLock(&Mutex);
		for (int i = 0; i < HudPrimitiveArray.Num(); i++)
		{
			auto hudPrimitive = HudPrimitiveArray[i];
			if (hudPrimitive != nullptr)
			{
				if (hudPrimitive->GetIsPostProcess())
				{
					hudPrimitive->GetPostProcessObject()->OnBeforeRenderPostProcess_GameThread(InViewFamily, InView);
				}
			}
		}
	}
}
void FLGUIViewExtension::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{
	if (!UICanvas.IsValid())return;
	ViewLocation = UICanvas->GetViewLocation();
	ViewRotationMatrix = UICanvas->GetViewRotationMatrix();
	ProjectionMatrix = UICanvas->GetProjectionMatrix();
	ViewProjectionMatrix = UICanvas->GetViewProjectionMatrix();
}
void FLGUIViewExtension::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}

static const uint16 FullScreenQuadIndices[6] =
{
	0, 2, 3,
	0, 3, 1
};
static TArray<FLGUIPostProcessVertex> FullScreenQuatdVertices =
{
	FLGUIPostProcessVertex(FVector(-1, -1, 0), FVector2D(0.0f, 0.0f)),
	FLGUIPostProcessVertex(FVector(1, -1, 0), FVector2D(1.0f, 0.0f)),
	FLGUIPostProcessVertex(FVector(-1, 1, 0), FVector2D(0.0f, 1.0f)),
	FLGUIPostProcessVertex(FVector(1, 1, 0), FVector2D(1.0f, 1.0f))
};

void FLGUIViewExtension::CopyRenderTarget(FRHICommandListImmediate& RHICmdList, TShaderMap<FGlobalShaderType>* GlobalShaderMap, FGraphicsPipelineStateInitializer& GraphicsPSOInit, FTexture2DRHIRef Src, FTexture2DRHIRef Dst, bool FlipY)
{
	CopyRenderTargetOnMeshRegion(RHICmdList, GlobalShaderMap, GraphicsPSOInit, Src, Dst, FlipY, FullScreenQuatdVertices);
}
void FLGUIViewExtension::CopyRenderTargetOnMeshRegion(FRHICommandListImmediate& RHICmdList, TShaderMap<FGlobalShaderType>* GlobalShaderMap, FGraphicsPipelineStateInitializer& GraphicsPSOInit, FTexture2DRHIRef Src, FTexture2DRHIRef Dst, bool FlipY, const TArray<FLGUIPostProcessVertex>& RegionVertexData)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Load_Store), TEXT("CopyRenderTargetOnMeshRegion"));

	TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FLGUICopyTargetSimplePS> PixelShader(GlobalShaderMap);
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	PixelShader->SetParameters(RHICmdList, Src, FlipY);

	uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessVertex);
	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, RegionVertexData.GetData(), VertexBufferSize);
	RHIUnlockVertexBuffer(VertexBufferRHI);

	FIndexBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(2, 12, BUF_Volatile, CreateInfo);
	void* VoidPtr2 = RHILockIndexBuffer(IndexBufferRHI, 0, 12, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr2, FullScreenQuadIndices, 12);
	RHIUnlockIndexBuffer(IndexBufferRHI);

	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, 0, 0, 4, 0, 2, 1);

	IndexBufferRHI.SafeRelease();
	VertexBufferRHI.SafeRelease();

	RHICmdList.EndRenderPass();
}
void FLGUIViewExtension::DrawFullScreenQuad(FRHICommandListImmediate& RHICmdList)
{
	uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessVertex);
	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, FullScreenQuatdVertices.GetData(), VertexBufferSize);
	RHIUnlockVertexBuffer(VertexBufferRHI);

	FIndexBufferRHIRef IndexBufferRHI = RHICreateIndexBuffer(2, 12, BUF_Volatile, CreateInfo);
	void* VoidPtr2 = RHILockIndexBuffer(IndexBufferRHI, 0, 12, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr2, FullScreenQuadIndices, 12);
	RHIUnlockIndexBuffer(IndexBufferRHI);

	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(IndexBufferRHI, 0, 0, 4, 0, 2, 1);

	IndexBufferRHI.SafeRelease();
	VertexBufferRHI.SafeRelease();
}
DECLARE_CYCLE_STAT(TEXT("Hud RHIRender"), STAT_Hud_RHIRender, STATGROUP_LGUI);
void FLGUIViewExtension::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_Hud_RHIRender);
	check(IsInRenderingThread());
	if (!InView.bIsGameView)return;
#if WITH_EDITOR
	if (GEngine == nullptr)return;
	if (UEditorEngine* editor = Cast<UEditorEngine>(GEngine))
	{
		if (editor->bIsSimulatingInEditor)return;
	}
#endif
	TArray<ILGUIHudPrimitive*> primitiveArray;
	{
		FScopeLock scopeLock(&Mutex);
		primitiveArray = HudPrimitiveArray;
	}

	FSceneView RenderView(InView);//use a copied view
	FTexture2DRHIRef ScreenImageRenderTexture = RenderView.Family->RenderTarget->GetRenderTargetTexture();
	FRHIRenderPassInfo RPInfo(ScreenImageRenderTexture, ERenderTargetActions::Load_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));

	RenderView.SceneViewInitOptions.ViewOrigin = ViewLocation;
	RenderView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
	RenderView.UpdateProjectionMatrix(ProjectionMatrix);

	FViewUniformShaderParameters viewUniformShaderParameters;
	RenderView.SetupCommonViewUniformBufferParameters(
		viewUniformShaderParameters,
		RenderView.UnscaledViewRect.Size(),
		1,
		RenderView.UnscaledViewRect,
		RenderView.ViewMatrices,
		FViewMatrices()
	);

	RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(viewUniformShaderParameters, UniformBuffer_SingleFrame);

	FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
	
	for (int i = 0; i < primitiveArray.Num(); i++)
	{
		auto hudPrimitive = primitiveArray[i];
		if (hudPrimitive != nullptr)
		{
			if (hudPrimitive->CanRender())
			{
				if (hudPrimitive->GetIsPostProcess())//render post process
				{
					RHICmdList.EndRenderPass();
					TShaderMap<FGlobalShaderType>* GlobalShaderMap = GetGlobalShaderMap(RenderView.GetFeatureLevel());
					auto postProcessObject = hudPrimitive->GetPostProcessObject();
					postProcessObject->OnRenderPostProcess_RenderThread(
						RHICmdList,
						ScreenImageRenderTexture,
						GlobalShaderMap,
						ViewProjectionMatrix,
						GraphicsPSOInit,
						[&]()
						{
							const FMeshBatch& Mesh = hudPrimitive->GetMeshElement((FMeshElementCollector*)&meshCollector);
							RHICmdList.SetStreamSource(0, hudPrimitive->GetVertexBufferRHI(), 0);
							RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, hudPrimitive->GetNumVerts(), 0, Mesh.GetNumPrimitives(), 1);
						}
					);
					RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
				}
				else//render mesh
				{
					const FMeshBatch& Mesh = hudPrimitive->GetMeshElement((FMeshElementCollector*)&meshCollector);
					auto Material = Mesh.MaterialRenderProxy->GetMaterial(RenderView.GetFeatureLevel());
					const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
					FLGUIHudRenderVS* VertexShader = (FLGUIHudRenderVS*)MaterialShaderMap->GetShader(&FLGUIHudRenderVS::StaticType);
					FLGUIHudRenderPS* PixelShader = (FLGUIHudRenderPS*)MaterialShaderMap->GetShader(&FLGUIHudRenderPS::StaticType);
					if (VertexShader && PixelShader)
					{
						PixelShader->SetBlendState(GraphicsPSOInit, Material);

						GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIVertexDeclaration();
						GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(VertexShader);
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(PixelShader);
						GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
						SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

						VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
						PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);

						RHICmdList.SetStreamSource(0, hudPrimitive->GetVertexBufferRHI(), 0);
						RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, hudPrimitive->GetNumVerts(), 0, Mesh.GetNumPrimitives(), 1);
					}
				}
			}
		}
	}
	RHICmdList.EndRenderPass();
}
void FLGUIViewExtension::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

void FLGUIViewExtension::AddHudPrimitive(ILGUIHudPrimitive* InPrimitive)
{
	ENQUEUE_RENDER_COMMAND(FLGUIRender_AddHudPrimitive)(
		[this, InPrimitive](FRHICommandListImmediate& RHICmdList)
		{
			this->AddHudPrimitive_RenderThread(InPrimitive);
		}
	);
}
void FLGUIViewExtension::AddHudPrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		FScopeLock scopeLock(&Mutex);
		HudPrimitiveArray.AddUnique(InPrimitive);
		HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
		{
			return A.GetRenderPriority() < B.GetRenderPriority();
		});
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIViewExtension::AddHudPrimitive]Add nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIViewExtension::RemoveHudPrimitive(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		FScopeLock scopeLock(&Mutex);
		HudPrimitiveArray.Remove(InPrimitive);
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIViewExtension::RemoveHudPrimitive]Remove nullptr as ILGUIHudPrimitive!"));
	}
}

void FLGUIViewExtension::MarkSortRenderPriority_RenderThread()
{
	HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
	{
		return A.GetRenderPriority() < B.GetRenderPriority();
	});
}
void FLGUIViewExtension::SortRenderPriority()
{
	FLGUIViewExtension* viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->MarkSortRenderPriority_RenderThread();
		}
	);
}