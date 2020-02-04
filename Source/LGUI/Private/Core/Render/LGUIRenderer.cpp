// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "Core/Render/LGUIRenderer.h"
#include "Core/Render/LGUIShaders.h"
#include "Modules/ModuleManager.h"
#include "LGUI.h"
#include "SceneView.h"
#include "Widgets/SWindow.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "Core/Render/ILGUIHudPrimitive.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "ScenePrivate.h"
#if WITH_EDITOR
#include "Engine.h"
#include "Editor/EditorEngine.h"
#endif

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
	FTexture2DRHIRef RenderTarget = RenderView.Family->RenderTarget->GetRenderTargetTexture();
	SetRenderTarget(RHICmdList, RenderTarget, FTextureRHIRef());

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
				const FMeshBatch& Mesh = hudPrimitive->GetMeshElement();
				auto Material = Mesh.MaterialRenderProxy->GetMaterial(RenderView.GetFeatureLevel());
				const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
				FLGUIHudRenderVS* VertexShader = (FLGUIHudRenderVS*)MaterialShaderMap->GetShader(&FLGUIHudRenderVS::StaticType);
				FLGUIHudRenderPS* PixelShader = (FLGUIHudRenderPS*)MaterialShaderMap->GetShader(&FLGUIHudRenderPS::StaticType);
				if (VertexShader && PixelShader)
				{
					PixelShader->SetBlendState(GraphicsPSOInit, Material);

					GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GLGUIVertexDeclaration.VertexDeclarationRHI;
					GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(VertexShader);
					GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(PixelShader);
					GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
					SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

					VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
					PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);

					RHICmdList.SetStreamSource(0, hudPrimitive->GetVertexBufferRHI(), 0);
					RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, PT_TriangleList, 0, 0, hudPrimitive->GetNumVerts(), 0, Mesh.GetNumPrimitives(), 1);
				}
			}
		}
	}
}
void FLGUIViewExtension::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

void FLGUIViewExtension::AddHudPrimitive(ILGUIHudPrimitive* InPrimitive)
{
	ENQUEUE_UNIQUE_RENDER_COMMAND_TWOPARAMETER(
		FLGUIRender_AddHudPrimitive,
		FLGUIViewExtension*, viewExtension, this,
		ILGUIHudPrimitive*, primitive, InPrimitive,
		{
			viewExtension->AddHudPrimitive_RenderThread(primitive);
		}
	)
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
	ENQUEUE_UNIQUE_RENDER_COMMAND_ONEPARAMETER(
		FLGUIRender_SortRenderPriority,
		FLGUIViewExtension*, viewExtension, this,
		{
			viewExtension->MarkSortRenderPriority_RenderThread();
		}
	)	
}