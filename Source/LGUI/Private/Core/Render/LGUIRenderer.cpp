// Copyright 2019 LexLiu. All Rights Reserved.

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
#include "MeshPassProcessor.inl"
#if WITH_EDITOR
#include "Engine.h"
#include "Editor/EditorEngine.h"
#endif


class FLGUIHudMeshProcessor : public FMeshPassProcessor
{
public:
	FLGUIHudMeshProcessor(
		const FScene* InScene,
		ERHIFeatureLevel::Type InFeatureLevel,
		const FSceneView* InViewIfDynamicMeshCommand,
		const FMeshPassProcessorRenderState& InDrawRenderState,
		FMeshPassDrawListContext* InDrawListContext
	) : FMeshPassProcessor(InScene, InFeatureLevel, InViewIfDynamicMeshCommand, InDrawListContext)
		, PassDrawRenderState(InDrawRenderState)
	{

	}
	virtual void AddMeshBatch(const FMeshBatch& RESTRICT MeshBatch, uint64 BatchElementMask, const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy, int32 StaticMeshId = -1) override
	{
		if (!MeshBatch.bUseForMaterial)
		{
			return;
		}

		Process(MeshBatch, BatchElementMask, PrimitiveSceneProxy, *MeshBatch.MaterialRenderProxy, *MeshBatch.MaterialRenderProxy->GetMaterial(FeatureLevel));
	}

	FMeshPassProcessorRenderState PassDrawRenderState;

	void SetupPipelineState(FMeshPassProcessorRenderState& DrawRenderState, const FMaterial* MaterialResource)const
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
private:
	void Process(
		const FMeshBatch& MeshBatch,
		uint64 BatchElementMask,
		const FPrimitiveSceneProxy* RESTRICT PrimitiveSceneProxy,
		const FMaterialRenderProxy& RESTRICT MaterialRenderProxy,
		const FMaterial& RESTRICT MaterialResource
		)
	{
		const FVertexFactory* VertexFactory = MeshBatch.VertexFactory;

		TMeshProcessorShaders<
			FLGUIHudRenderVS,
			FMeshMaterialShader,
			FMeshMaterialShader,
			FLGUIHudRenderPS> PassShaders;

		PassShaders.VertexShader = MaterialResource.GetShader<FLGUIHudRenderVS>(VertexFactory->GetType());
		PassShaders.PixelShader = MaterialResource.GetShader<FLGUIHudRenderPS>(VertexFactory->GetType());

		FMeshPassProcessorRenderState DrawRenderState(PassDrawRenderState);
		SetupPipelineState(DrawRenderState, &MaterialResource);

		FMeshMaterialShaderElementData ShaderElementData;
		ShaderElementData.InitializeMeshMaterialData(ViewIfDynamicMeshCommand, PrimitiveSceneProxy, MeshBatch, -1, false);

		const FMeshDrawCommandSortKey SortKey = CalculateMeshStaticSortKey(PassShaders.VertexShader, PassShaders.PixelShader);

		ERasterizerFillMode MeshFillMode = ERasterizerFillMode::FM_Solid;
		ERasterizerCullMode MeshCullMode = ERasterizerCullMode::CM_None;

		BuildMeshDrawCommands(
			MeshBatch,
			BatchElementMask,
			PrimitiveSceneProxy,
			MaterialRenderProxy,
			MaterialResource,
			DrawRenderState,
			PassShaders,
			MeshFillMode,
			MeshCullMode,
			SortKey,
			EMeshPassFeatures::Default,
			ShaderElementData);
	}
};


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
	if (NeedToSortPrimitive)
	{
		HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
		{
			return A.GetRenderPriority() < B.GetRenderPriority();
		});
		NeedToSortPrimitive = false;
	}

	FTexture2DRHIRef RenderTarget = InView.Family->RenderTarget->GetRenderTargetTexture();
	FRHIRenderPassInfo RPInfo(RenderTarget, ERenderTargetActions::DontLoad_Store);
	RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
	const FSceneViewFamily* ViewFamily = InView.Family;
	const FScene* Scene = nullptr;
	if (ViewFamily->Scene)
	{
		Scene = ViewFamily->Scene->GetRenderScene();
	}

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

	FMeshPassProcessorRenderState drawRenderState(InView);
	drawRenderState.SetDepthStencilState(TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI());
	drawRenderState.SetViewUniformBuffer(InView.ViewUniformBuffer);
	FLGUIMeshElementCollector meshCollector(InView.GetFeatureLevel());
	for (int i = 0; i < HudPrimitiveArray.Num(); i++)
	{
		auto hudPrimitive = HudPrimitiveArray[i];
		if (hudPrimitive != nullptr && hudPrimitive->CanRender())
		{
			const FMeshBatch& Mesh = hudPrimitive->GetMeshElement((FMeshElementCollector*)&meshCollector);
			if (Mesh.VertexFactory != nullptr && Mesh.VertexFactory->IsInitialized())
			{
				DrawDynamicMeshPass(InView, RHICmdList,
					[Scene, &InView, &drawRenderState, &Mesh](FMeshPassDrawListContext* InDrawListContext)
				{
					FLGUIHudMeshProcessor meshProcessor(
						Scene,
						InView.GetFeatureLevel(),
						&InView,
						drawRenderState,
						InDrawListContext
					);
					const uint64 DefaultBatchElementMask = ~0ull;
					meshProcessor.AddMeshBatch(Mesh, DefaultBatchElementMask, nullptr);
				});
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
	if (InPrimitive == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIViewExtension::AddHudPrimitive]Add nullptr as ILGUIHudPrimitive!"));
		return;
	}
	HudPrimitiveArray.Add(InPrimitive);
}
void FLGUIViewExtension::RemoveHudPrimitive(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		HudPrimitiveArray.Remove(InPrimitive);
	}
}

void FLGUIViewExtension::MarkSortRenderPriority_RenderThread()
{
	NeedToSortPrimitive = true;
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