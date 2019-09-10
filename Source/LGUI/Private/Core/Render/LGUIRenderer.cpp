﻿// Copyright 2019 LexLiu. All Rights Reserved.

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
#include "DrawingPolicy.h"
#if WITH_EDITOR
#include "Engine.h"
#include "Editor/EditorEngine.h"
#endif


class FLGUIHudRenderPolicy : public FMeshDrawingPolicy
{
public:
	FLGUIHudRenderPolicy(
		const FVertexFactory* InVertexFactory,
		const FMaterialRenderProxy* InMaterialRenderProxy,
		const FMaterial& InMaterialResource,
		const FMeshDrawingPolicyOverrideSettings& InOverrideSettings
	)
		:FMeshDrawingPolicy(InVertexFactory, InMaterialRenderProxy, InMaterialResource, InOverrideSettings)
	{
		PixelShader = InMaterialResource.GetShader<FLGUIHudRenderPS>(InVertexFactory->GetType());
		VertexShader = InMaterialResource.GetShader<FLGUIHudRenderVS>(InVertexFactory->GetType());
		BaseVertexShader = VertexShader;
	}
	FDrawingPolicyMatchResult Matches(const FLGUIHudRenderPolicy& Other, bool bForReals = false)
	{
		DRAWING_POLICY_MATCH_BEGIN
			DRAWING_POLICY_MATCH(FMeshDrawingPolicy::Matches(Other, bForReals)) &&
			DRAWING_POLICY_MATCH(VertexShader == Other.VertexShader) &&
			DRAWING_POLICY_MATCH(PixelShader == Other.PixelShader);
		DRAWING_POLICY_MATCH_END
	}
	void SetupPipelineState(FDrawingPolicyRenderState& DrawRenderState, const FSceneView& View)const
	{
		auto BlendMode = MaterialResource->GetBlendMode();
		switch (BlendMode)
		{
		default:
		case BLEND_Opaque:
			DrawRenderState.SetBlendState(TStaticBlendState<>::GetRHI());
			break;
		case BLEND_Masked:
			DrawRenderState.SetBlendState(TStaticBlendState<>::GetRHI());
			break;
		case BLEND_Translucent:
			// Note: alpha channel used by separate translucency, storing how much of the background should be added when doing the final composite
			// The Alpha channel is also used by non-separate translucency when rendering to scene captures, which store the final opacity
			DrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI());
			break;
		case BLEND_Additive:
			// Add to the existing scene color
			// Note: alpha channel used by separate translucency, storing how much of the background should be added when doing the final composite
			// The Alpha channel is also used by non-separate translucency when rendering to scene captures, which store the final opacity
			DrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_One, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI());
			break;
		case BLEND_Modulate:
			// Modulate with the existing scene color, preserve destination alpha.
			DrawRenderState.SetBlendState(TStaticBlendState<CW_RGB, BO_Add, BF_DestColor, BF_Zero>::GetRHI());
			break;
		case BLEND_AlphaComposite:
			// Blend with existing scene color. New color is already pre-multiplied by alpha.
			DrawRenderState.SetBlendState(TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI());
			break;
		};
	}
	void SetSharedState(FRHICommandList& RHICmdList, const FDrawingPolicyRenderState& DrawRenderState, const FSceneView* View, const FMeshDrawingPolicy::ContextDataType PolicyContext)const
	{
		VertexShader->SetParameters(RHICmdList, MaterialRenderProxy, *MaterialResource, *View, DrawRenderState.GetViewUniformBuffer());
		PixelShader->SetParameters(RHICmdList, MaterialRenderProxy, *MaterialResource, *View, DrawRenderState.GetViewUniformBuffer());
		FMeshDrawingPolicy::SetSharedState(RHICmdList, DrawRenderState, View, PolicyContext);
	}
	FBoundShaderStateInput GetBoundShaderStateInput(ERHIFeatureLevel::Type InFeatureLevel)const
	{
		return FBoundShaderStateInput(
			FMeshDrawingPolicy::GetVertexDeclaration(),
			VertexShader->GetVertexShader(),
			NULL,
			NULL,
			PixelShader->GetPixelShader(),
			NULL);
	}
	void SetMeshRenderState(
		FRHICommandList& RHICmdList,
		const FSceneView& View,
		const FMeshBatch& Mesh, int32 BatchElementIndex,
		const FDrawingPolicyRenderState& DrawRenderState,
		const ElementDataType& ElementData,
		const ContextDataType PolicyContext
	)const
	{
		const FMeshBatchElement& BatchElement = Mesh.Elements[BatchElementIndex];
		VertexShader->SetMesh(RHICmdList, VertexFactory, View, BatchElement, DrawRenderState);
		PixelShader->SetMesh(RHICmdList, VertexFactory, View, BatchElement, DrawRenderState);
	}
	friend int32 CompareDrawingPolicy(const FLGUIHudRenderPolicy& A, const FLGUIHudRenderPolicy& B)
	{
		COMPAREDRAWINGPOLICYMEMBERS(VertexShader);
		COMPAREDRAWINGPOLICYMEMBERS(PixelShader);
		COMPAREDRAWINGPOLICYMEMBERS(VertexFactory);
		COMPAREDRAWINGPOLICYMEMBERS(MaterialRenderProxy);
		return 0;
	}
	bool IsInitialized()
	{
		return VertexFactory != nullptr && VertexFactory->IsInitialized();
	}
private:
	FLGUIHudRenderVS* VertexShader;
	FLGUIHudRenderPS* PixelShader;
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
	if (!UICanvas.IsValid())return;
	ViewLocation = UICanvas->GetViewLocation();
	ViewRotationMatrix = UICanvas->GetViewRotationMatrix();
	ProjectionMatrix = UICanvas->GetProjectionMatrix();
}
void FLGUIViewExtension::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{
	
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
	if (UEditorEngine* editor = CastChecked<UEditorEngine>(GEngine))
	{
		if (editor->bIsSimulatingInEditor)return;
	}
#endif
	TArray<ILGUIHudPrimitive*> primitiveArray;
	Mutex.Lock();
	primitiveArray = HudPrimitiveArray;
	Mutex.Unlock();

	FTexture2DRHIRef RenderTarget = InView.Family->RenderTarget->GetRenderTargetTexture();
	SetRenderTarget(RHICmdList, RenderTarget, FTextureRHIRef());

	InView.SceneViewInitOptions.ViewOrigin = ViewLocation;
	InView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
	InView.UpdateProjectionMatrix(ProjectionMatrix);

	FViewUniformShaderParameters viewUniformShaderParameters;
	InView.SetupCommonViewUniformBufferParameters(
		viewUniformShaderParameters,
		InView.UnscaledViewRect.Size(),
		1,
		InView.UnscaledViewRect,
		InView.ViewMatrices,
		FViewMatrices()
	);
	InView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(viewUniformShaderParameters, UniformBuffer_SingleFrame);

	FDrawingPolicyRenderState drawRenderState(InView);
	drawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI());
	drawRenderState.SetViewUniformBuffer(InView.ViewUniformBuffer);
	
	for (int i = 0; i < primitiveArray.Num(); i++)
	{
		auto hudPrimitive = primitiveArray[i];
		if (hudPrimitive != nullptr && hudPrimitive->CanRender())
		{
			const FMeshBatch& Mesh = hudPrimitive->GetMeshElement();
			FLGUIHudRenderPolicy drawingPolicy(Mesh.VertexFactory, Mesh.MaterialRenderProxy
				, *Mesh.MaterialRenderProxy->GetMaterial(InView.GetFeatureLevel())
				, ComputeMeshOverrideSettings(Mesh));
			if (drawingPolicy.IsInitialized())
			{
				drawingPolicy.SetupPipelineState(drawRenderState, InView);
				CommitGraphicsPipelineState(RHICmdList, drawingPolicy
					, drawRenderState, drawingPolicy.GetBoundShaderStateInput(InView.GetFeatureLevel())
					, drawingPolicy.GetMaterialRenderProxy());
				drawingPolicy.SetSharedState(RHICmdList, drawRenderState, &InView, FLGUIHudRenderPolicy::ContextDataType());

				drawingPolicy.SetMeshRenderState(RHICmdList, InView, Mesh, 0, drawRenderState, FMeshDrawingPolicy::ElementDataType(), FLGUIHudRenderPolicy::ContextDataType());
				drawingPolicy.DrawMesh(RHICmdList, InView, Mesh, 0);
			}
		}
	}
}
void FLGUIViewExtension::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

void FLGUIViewExtension::AddHudPrimitive(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIViewExtension::AddHudPrimitive]Add nullptr as ILGUIHudPrimitive!"));
		return;
	}
	Mutex.Lock();
	HudPrimitiveArray.Add(InPrimitive);
	HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
	{
		return A.GetRenderPriority() < B.GetRenderPriority();
	});
	Mutex.Unlock();
}
void FLGUIViewExtension::RemoveHudPrimitive(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		Mutex.Lock();
		HudPrimitiveArray.Remove(InPrimitive);
		Mutex.Unlock();
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