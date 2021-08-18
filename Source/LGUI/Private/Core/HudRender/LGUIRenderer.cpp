// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/HudRender/LGUIRenderer.h"
#include "Core/HudRender/LGUIHudShaders.h"
#include "Core/HudRender/LGUIHudVertex.h"
#include "Core/HudRender/LGUIPostProcessShaders.h"
#include "Modules/ModuleManager.h"
#include "LGUI.h"
#include "SceneView.h"
#include "Widgets/SWindow.h"
#include "StaticMeshVertexData.h"
#include "PipelineStateCache.h"
#include "SceneRendering.h"
#include "Core/HudRender/ILGUIHudPrimitive.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "MeshPassProcessor.inl"
#include "ScenePrivate.h"
#if WITH_EDITOR
#include "Engine/Engine.h"
#include "Editor/EditorEngine.h"
#endif
#include "Slate/SceneViewport.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/LGUISettings.h"
#include "Engine/TextureRenderTarget2D.h"
class FLGUIMeshElementCollector : FMeshElementCollector
{
public:
	FLGUIMeshElementCollector(ERHIFeatureLevel::Type InFeatureLevel) :FMeshElementCollector(InFeatureLevel)
	{

	}
};


#if WITH_EDITORONLY_DATA
uint32 FLGUIHudRenderer::EditorPreview_ViewKey = 0;
#endif
FLGUIHudRenderer::FLGUIHudRenderer(const FAutoRegister& AutoRegister, UWorld* InWorld, UTextureRenderTarget2D* InCustomRenderTarget)
	:FSceneViewExtensionBase(AutoRegister)
{
	World = InWorld;
	CustomRenderTarget = InCustomRenderTarget;

#if WITH_EDITORONLY_DATA
	bIsEditorPreview = !World->IsGameWorld();
#endif
}
FLGUIHudRenderer::~FLGUIHudRenderer()
{
	
}

void FLGUIHudRenderer::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (!World.IsValid())return;
	if (World.Get() != InView.Family->Scene->GetWorld())return;
	//world space
	{
		FScopeLock scopeLock(&RenderCanvasParameterArray_Mutex);
		for (auto& canvasItem : RenderCanvasParameterArray)
		{
			if (IsValid(canvasItem.RenderCanvas))
			{
				canvasItem.ViewLocation = InView.SceneViewInitOptions.ViewOrigin;
				canvasItem.ViewRotationMatrix = InView.SceneViewInitOptions.ViewRotationMatrix;
				canvasItem.ProjectionMatrix = InView.SceneViewInitOptions.ProjectionMatrix;
				canvasItem.ViewProjectionMatrix = FTranslationMatrix(-canvasItem.ViewLocation) * (canvasItem.ViewRotationMatrix) * canvasItem.ProjectionMatrix;
				canvasItem.BlendDepth = canvasItem.RenderCanvas->GetActualBlendDepth();

				//canvasItem.ViewLocation = canvasItem.RenderCanvas->GetViewLocation();
				//canvasItem.ViewRotationMatrix = canvasItem.RenderCanvas->GetViewRotationMatrix();
				//canvasItem.ProjectionMatrix = canvasItem.RenderCanvas->GetProjectionMatrix();
				//canvasItem.ViewProjectionMatrix = canvasItem.RenderCanvas->GetViewProjectionMatrix();
				//canvasItem.BlendDepth = canvasItem.RenderCanvas->GetActualBlendDepth();
			}
		}
	}
	//screen space
	if (ScreenSpaceRenderParameter.RenderCanvas.IsValid())
	{
		ScreenSpaceRenderParameter.ViewLocation = ScreenSpaceRenderParameter.RenderCanvas->GetViewLocation();
		ScreenSpaceRenderParameter.ViewRotationMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetViewRotationMatrix().InverseFast();
		ScreenSpaceRenderParameter.ProjectionMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetProjectionMatrix();
		ScreenSpaceRenderParameter.ViewProjectionMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetViewProjectionMatrix();
	}
	MultiSampleCount = (uint16)ULGUISettings::GetAntiAliasingSampleCount();
}
void FLGUIHudRenderer::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{
	
}
void FLGUIHudRenderer::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}

void FLGUIHudRenderer::CopyRenderTarget(FRHICommandListImmediate& RHICmdList, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Clear_DontStore), TEXT("LGUICopyRenderTarget"));
	RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);

	TShaderMapRef<FLGUISimplePostProcessVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FLGUISimpleCopyTargetPS> PixelShader(GlobalShaderMap);
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessVertexDeclaration();
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
	GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	VertexShader->SetParameters(RHICmdList);
	PixelShader->SetParameters(RHICmdList, Src);

	DrawFullScreenQuad(RHICmdList);

	RHICmdList.EndRenderPass();
}
void FLGUIHudRenderer::CopyRenderTargetOnMeshRegion(FRHICommandListImmediate& RHICmdList, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst, const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData, const FMatrix& MVP)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Clear_DontStore), TEXT("LGUICopyRenderTargetOnMeshRegion"));
	RHICmdList.SetViewport(0, 0, 0, Dst->GetSizeXYZ().X, Dst->GetSizeXYZ().Y, 1.0f);

	TShaderMapRef<FLGUICopyMeshRegionVS> VertexShader(GlobalShaderMap);
	TShaderMapRef<FLGUICopyMeshRegionPS> PixelShader(GlobalShaderMap);
	FGraphicsPipelineStateInitializer GraphicsPSOInit;
	RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
	GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
	GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIPostProcessCopyMeshRegionVertexDeclaration();
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
	GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
	GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	//VertexShader->SetParameters(RHICmdList);
	PixelShader->SetParameters(RHICmdList, MVP, Src);

	uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessCopyMeshRegionVertex);
	FRHIResourceCreateInfo CreateInfo(TEXT("CopyRenderTargetOnMeshRegion"));
	FBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, RegionVertexData.GetData(), VertexBufferSize);
	RHIUnlockBuffer(VertexBufferRHI);

	RHICmdList.SetStreamSource(0, VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);

	VertexBufferRHI.SafeRelease();

	RHICmdList.EndRenderPass();
}
void FLGUIHudRenderer::DrawFullScreenQuad(FRHICommandListImmediate& RHICmdList)
{
	RHICmdList.SetStreamSource(0, GLGUIFullScreenQuadVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
}
void FLGUIHudRenderer::SetGraphicPipelineStateFromMaterial(FGraphicsPipelineStateInitializer& GraphicsPSOInit, const FMaterial* Material)
{
	EBlendMode BlendMode = Material->GetBlendMode();
	switch (BlendMode)
	{
	default:
	case BLEND_Opaque:
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		break;
	case BLEND_Masked:
		GraphicsPSOInit.BlendState = TStaticBlendState<>::GetRHI();
		break;
	case BLEND_Translucent:
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_InverseSourceAlpha, BO_Add, BF_InverseDestAlpha, BF_One>::GetRHI();
		break;
	case BLEND_Additive:
		// Add to the existing scene color
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_One, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		//GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_SourceAlpha, BF_One, BO_Add, BF_One, BF_One>::GetRHI();
		break;
	case BLEND_Modulate:
		// Modulate with the existing scene color
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGB, BO_Add, BF_Zero, BF_SourceColor>::GetRHI();
		break;
	case BLEND_AlphaComposite:
		// Blend with existing scene color. New color is already pre-multiplied by alpha.
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_One, BF_InverseSourceAlpha, BO_Add, BF_One, BF_InverseSourceAlpha>::GetRHI();
		break;
	case BLEND_AlphaHoldout:
		// Blend by holding out the matte shape of the source alpha
		GraphicsPSOInit.BlendState = TStaticBlendState<CW_RGBA, BO_Add, BF_Zero, BF_InverseSourceAlpha, BO_Add, BF_Zero, BF_InverseSourceAlpha>::GetRHI();
		break;
	};

	GraphicsPSOInit.DepthStencilState = TStaticDepthStencilState<false, ECompareFunction::CF_Always>::GetRHI();
	
	if (!Material->IsWireframe())
	{
		if (Material->IsTwoSided())
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false>::GetRHI();
		}
		else
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_CW, false>::GetRHI();
		}
	}
	else
	{
		if (Material->IsTwoSided())
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_None, true>::GetRHI();
		}
		else
		{
			GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Wireframe, CM_CW, true>::GetRHI();
		}
	}
}
DECLARE_CYCLE_STAT(TEXT("Hud RHIRender"), STAT_Hud_RHIRender, STATGROUP_LGUI);
void FLGUIHudRenderer::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_Hud_RHIRender);
	check(IsInRenderingThread());
	if (!World.IsValid())return;
	if (World.Get() != InView.Family->Scene->GetWorld())return;
#if WITH_EDITOR
	//check if simulation
	if (GEngine == nullptr)return;
	if (UEditorEngine* editor = Cast<UEditorEngine>(GEngine))
	{
		if (editor->bIsSimulatingInEditor)return;
	}
#endif
	//the following two lines can prevent duplicated ui in viewport when "Number of Players" > 1
#if WITH_EDITOR
	if (InView.Family == nullptr || InView.Family->Scene == nullptr || InView.Family->Scene->GetWorld() == nullptr)return;
#endif

	FSceneView RenderView(InView);//use a copied view
	auto GlobalShaderMap = GetGlobalShaderMap(RenderView.GetFeatureLevel());

	//create render target
	FTextureRHIRef ScreenColorRenderTargetTexture = nullptr;
	FTextureRHIRef ScreenColorRenderTargetResolveTexture = nullptr;
	TRefCountPtr<IPooledRenderTarget> ScreenColorRenderTarget;

	if (CustomRenderTarget.IsValid())
	{
		ScreenColorRenderTargetTexture = (FTextureRHIRef)CustomRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
		//clear render target;
		RHICmdList.BeginRenderPass(FRHIRenderPassInfo(ScreenColorRenderTargetTexture, ERenderTargetActions::Clear_DontStore), TEXT("LGUIHudRender_ClearRenderTarget"));
		RHICmdList.EndRenderPass();
	}
	else
	{
		if (bContainsPostProcess || MultiSampleCount > 1)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(RenderView.UnscaledViewRect.Size(), RenderView.Family->RenderTarget->GetRenderTargetTexture()->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable, false));
			desc.NumSamples = MultiSampleCount;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, ScreenColorRenderTarget, TEXT("LGUISceneColorRenderTarget"));
			if (!ScreenColorRenderTarget.IsValid())
				return;
			ScreenColorRenderTargetTexture = ScreenColorRenderTarget->GetRenderTargetItem().TargetableTexture;
			ScreenColorRenderTargetResolveTexture = ScreenColorRenderTarget->GetRenderTargetItem().ShaderResourceTexture;
#if PLATFORM_WINDOWS || PLATFORM_XBOXONE || PLATFORM_MAC || PLATFORM_PS4 || PLATFORM_LINUX
			CopyRenderTarget(RHICmdList, GlobalShaderMap, (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture(), ScreenColorRenderTargetTexture);
#else
			RHICmdList.CopyToResolveTarget((FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture(), ScreenColorRenderTargetTexture, FResolveParams());
#endif
		}
		else
		{
			ScreenColorRenderTargetTexture = (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture();
		}
	}

	auto RPInfo = FRHIRenderPassInfo(ScreenColorRenderTargetTexture, ERenderTargetActions::Load_DontStore);
	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	FVector4 DepthTextureScaleOffset = FVector4(
		(float)ScreenColorRenderTargetTexture->GetSizeXYZ().X / SceneContext.GetSceneDepthSurface()->GetSizeX(),
		(float)ScreenColorRenderTargetTexture->GetSizeXYZ().Y / SceneContext.GetSceneDepthSurface()->GetSizeY(),
		0, 0
	);

	RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
	RHICmdList.SetViewport(0, 0, 0.0f, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y, 1.0f);

	//Render world space
	for (int canvasIndex = 0; canvasIndex < RenderCanvasParameterArray.Num(); canvasIndex++)
	{
		auto& canvasParamItem = RenderCanvasParameterArray[canvasIndex];

		RenderView.SceneViewInitOptions.ViewOrigin = canvasParamItem.ViewLocation;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = canvasParamItem.ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(canvasParamItem.ProjectionMatrix);

		FViewUniformShaderParameters viewUniformShaderParameters;
		RenderView.SetupCommonViewUniformBufferParameters(
			viewUniformShaderParameters,
			RenderView.UnscaledViewRect.Size(),
			MultiSampleCount,
			RenderView.UnscaledViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);

		RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(viewUniformShaderParameters, UniformBuffer_SingleFrame);

		FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.NumSamples = MultiSampleCount;


		for (int primitiveIndex = 0; primitiveIndex < canvasParamItem.HudPrimitiveArray.Num(); primitiveIndex++)
		{
			auto hudPrimitive = canvasParamItem.HudPrimitiveArray[primitiveIndex];
			if (hudPrimitive != nullptr)
			{
				if (hudPrimitive->CanRender())
				{
					if (hudPrimitive->GetIsPostProcess())//render post process
					{
						RHICmdList.EndRenderPass();
						hudPrimitive->OnRenderPostProcess_RenderThread(
							RHICmdList,
							this,
							(FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture(),
							ScreenColorRenderTargetTexture,
							ScreenColorRenderTargetResolveTexture,
							GlobalShaderMap,
							canvasParamItem.ViewProjectionMatrix,
							true,
							canvasParamItem.BlendDepth,
							DepthTextureScaleOffset
						);
						RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
						RHICmdList.SetViewport(0, 0, 0.0f, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y, 1.0f);
						GraphicsPSOInit.NumSamples = MultiSampleCount;
					}
					else//render mesh
					{
						const FMeshBatch& Mesh = hudPrimitive->GetMeshElement((FMeshElementCollector*)&meshCollector);
						auto Material = &(Mesh.MaterialRenderProxy->GetIncompleteMaterialWithFallback(RenderView.GetFeatureLevel()));
						const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
						auto VertexShader = MaterialShaderMap->GetShader<FLGUIHudRenderVS>();

						auto PixelShader = MaterialShaderMap->GetShader<FLGUIWorldRenderPS>();
						if (VertexShader.IsValid() && PixelShader.IsValid())
						{
							SetGraphicPipelineStateFromMaterial(GraphicsPSOInit, Material);

							GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
							GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
							GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
							GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
							GraphicsPSOInit.NumSamples = MultiSampleCount;
							SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

							VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
							PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
							PixelShader->SetDepthBlendParameter(RHICmdList, canvasParamItem.BlendDepth, DepthTextureScaleOffset, SceneContext.GetSceneDepthSurface());

							RHICmdList.SetStreamSource(0, hudPrimitive->GetVertexBufferRHI(), 0);
							RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, hudPrimitive->GetNumVerts(), 0, Mesh.GetNumPrimitives(), 1);
						}
					}
				}
			}
		}
	}
	//Render screen space
	if (ScreenSpaceRenderParameter.RenderCanvas.IsValid())
	{
#if WITH_EDITOR
		if (ALGUIManagerActor::IsPlaying == bIsEditorPreview)goto END_LGUI_RENDER;
		if (ALGUIManagerActor::IsPlaying)
		{
			if (!InView.bIsGameView)goto END_LGUI_RENDER;
		}
		else//editor viewport preview
		{
			if (InView.GetViewKey() != EditorPreview_ViewKey)goto END_LGUI_RENDER;//only preview in specific viewport in editor
		}
#endif

		RenderView.SceneViewInitOptions.ViewOrigin = ScreenSpaceRenderParameter.ViewLocation;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = ScreenSpaceRenderParameter.ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(ScreenSpaceRenderParameter.ProjectionMatrix);

		FViewUniformShaderParameters viewUniformShaderParameters;
		RenderView.SetupCommonViewUniformBufferParameters(
			viewUniformShaderParameters,
			RenderView.UnscaledViewRect.Size(),
			MultiSampleCount,
			RenderView.UnscaledViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);

		RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(viewUniformShaderParameters, UniformBuffer_SingleFrame);

		FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.NumSamples = MultiSampleCount;

		for (int primitiveIndex = 0; primitiveIndex < ScreenSpaceRenderParameter.HudPrimitiveArray.Num(); primitiveIndex++)
		{
			auto hudPrimitive = ScreenSpaceRenderParameter.HudPrimitiveArray[primitiveIndex];
			if (hudPrimitive != nullptr)
			{
				if (hudPrimitive->CanRender())
				{
					if (hudPrimitive->GetIsPostProcess())//render post process
					{
						RHICmdList.EndRenderPass();
						hudPrimitive->OnRenderPostProcess_RenderThread(
							RHICmdList,
							this,
							(FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture(),
							ScreenColorRenderTargetTexture,
							ScreenColorRenderTargetResolveTexture,
							GlobalShaderMap,
							ScreenSpaceRenderParameter.ViewProjectionMatrix,
							false,
							0.0f,
							DepthTextureScaleOffset
						);
						RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
						RHICmdList.SetViewport(0, 0, 0.0f, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y, 1.0f);
						GraphicsPSOInit.NumSamples = MultiSampleCount;
					}
					else//render mesh
					{
						const FMeshBatch& Mesh = hudPrimitive->GetMeshElement((FMeshElementCollector*)&meshCollector);
						auto Material = Mesh.MaterialRenderProxy->GetMaterial(RenderView.GetFeatureLevel());
						const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
						auto VertexShader = MaterialShaderMap->GetShader<FLGUIHudRenderVS>();

						auto PixelShader = MaterialShaderMap->GetShader<FLGUIHudRenderPS>();
						if (VertexShader.IsValid() && PixelShader.IsValid())
						{
							SetGraphicPipelineStateFromMaterial(GraphicsPSOInit, Material);

							GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
							GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
							GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
							GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
							GraphicsPSOInit.NumSamples = MultiSampleCount;
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
	}
	END_LGUI_RENDER:

	RHICmdList.EndRenderPass();
	//copy back to screen
	if (CustomRenderTarget.IsValid())
	{
		
	}
	else
	{
		if (bContainsPostProcess || MultiSampleCount > 1)
		{
#if PLATFORM_WINDOWS || PLATFORM_XBOXONE || PLATFORM_MAC || PLATFORM_PS4 || PLATFORM_LINUX
			RHICmdList.CopyToResolveTarget(ScreenColorRenderTargetTexture, (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture(), FResolveParams());
#else
			CopyRenderTarget(RHICmdList, GlobalShaderMap, ScreenColorRenderTargetTexture, (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture());
#endif
		}
	}
	//release render target
	if (ScreenColorRenderTarget.IsValid())
	{
		ScreenColorRenderTarget.SafeRelease();
	}
}
void FLGUIHudRenderer::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

void FLGUIHudRenderer::AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, int32 InRenderCanvasSortOrder, ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		int existIndex = RenderCanvasParameterArray.IndexOfByPredicate([InCanvas](const FRenderCanvasParameter& item) {
			return item.RenderCanvas == InCanvas;
			});
		if (existIndex == INDEX_NONE)
		{
			UE_LOG(LGUI, Error, TEXT("[FLGUIHudRenderer::AddWorldSpacePrimitive_RenderThread]Canvas not exist!"));
		}
		else
		{
			auto& item = RenderCanvasParameterArray[existIndex];
			item.HudPrimitiveArray.AddUnique(InPrimitive);
			item.HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
			{
				return A.GetRenderPriority() < B.GetRenderPriority();
			});
			//check if we have postprocess
			CheckContainsPostProcess_RenderThread();
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::AddWorldSpacePrimitive_RenderThread]Add nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::RemoveWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		int existIndex = RenderCanvasParameterArray.IndexOfByPredicate([InCanvas](const FRenderCanvasParameter& item) {
			return item.RenderCanvas == InCanvas;
			});
		if (existIndex == INDEX_NONE)
		{
			UE_LOG(LGUI, Log, TEXT("[FLGUIHudRenderer::RemoveHudPrimitive]Canvas already removed."));
		}
		else
		{
			auto& item = RenderCanvasParameterArray[existIndex];
			item.HudPrimitiveArray.RemoveSingle(InPrimitive);
			//check if we have postprocess
			CheckContainsPostProcess_RenderThread();
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::RemoveWorldSpacePrimitive_RenderThread]Remove nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::AddScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.HudPrimitiveArray.AddUnique(InPrimitive);
		SortScreenSpacePrimitiveRenderPriority_RenderThread();
		CheckContainsPostProcess_RenderThread();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::AddScreenSpacePrimitive_RenderThread]Add nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::RemoveScreenSpacePrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		ScreenSpaceRenderParameter.HudPrimitiveArray.RemoveSingle(InPrimitive);
		CheckContainsPostProcess_RenderThread();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::RemoveScreenSpacePrimitive_RenderThread]Remove nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::SortScreenSpacePrimitiveRenderPriority_RenderThread()
{
	ScreenSpaceRenderParameter.HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
		{
			return A.GetRenderPriority() < B.GetRenderPriority();
		});
}
void FLGUIHudRenderer::SortPrimitiveRenderPriority_RenderThread()
{
	FScopeLock scopeLock(&RenderCanvasParameterArray_Mutex);
	for (auto& item : RenderCanvasParameterArray)
	{
		item.HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B) {
			return A.GetRenderPriority() < B.GetRenderPriority();
			});
	}
}
void FLGUIHudRenderer::SetRenderCanvasSortOrder_RenderThread(ULGUICanvas* InRenderCanvas, int32 InSortOrder)
{
	FScopeLock scopeLock(&RenderCanvasParameterArray_Mutex);
	int existIndex = RenderCanvasParameterArray.IndexOfByPredicate([InRenderCanvas](const FRenderCanvasParameter& item) {
		return item.RenderCanvas == InRenderCanvas;
		});
	if (existIndex == INDEX_NONE)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIHudRenderer::RemoveHudPrimitive]Canvas not exist!"));
	}
	else
	{
		RenderCanvasParameterArray[existIndex].RenderCanvasSortOrder = InSortOrder;
		RenderCanvasParameterArray.Sort([](const FRenderCanvasParameter& A, const FRenderCanvasParameter& B) {
			return A.RenderCanvasSortOrder < B.RenderCanvasSortOrder;
			});
	}
}
void FLGUIHudRenderer::SortPrimitiveRenderPriority()
{
	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortPrimitiveRenderPriority)(
		[viewExtension](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SortPrimitiveRenderPriority_RenderThread();
		}
	);
}
void FLGUIHudRenderer::AddWorldSpaceRenderCanvas(ULGUICanvas* InCanvas)
{
	FRenderCanvasParameter canvasItem;
	canvasItem.RenderCanvas = InCanvas;
	canvasItem.RenderCanvasSortOrder = InCanvas->GetSortOrder();
	canvasItem.ViewLocation = canvasItem.RenderCanvas->GetViewLocation();
	canvasItem.ViewRotationMatrix = canvasItem.RenderCanvas->GetViewRotationMatrix().InverseFast();
	canvasItem.ProjectionMatrix = canvasItem.RenderCanvas->GetProjectionMatrix();
	canvasItem.ViewProjectionMatrix = canvasItem.RenderCanvas->GetViewProjectionMatrix();

	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension, canvasItem](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->AddWorldSpaceRenderCanvas_RenderThread(canvasItem);
		}
	);
}
void FLGUIHudRenderer::RemoveWorldSpaceRenderCanvas(ULGUICanvas* InCanvas)
{
	auto viewExtension = this;
	auto canvas = InCanvas;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension, canvas](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->RemoveWorldSpaceRenderCanvas_RenderThread(canvas);
		}
	);
}

void FLGUIHudRenderer::SetScreenSpaceRenderCanvas(ULGUICanvas* InCanvas)
{
	ScreenSpaceRenderParameter.RenderCanvas = InCanvas;
}
void FLGUIHudRenderer::ClearScreenSpaceRenderCanvas()
{
	ScreenSpaceRenderParameter.RenderCanvas = nullptr;
}

void FLGUIHudRenderer::AddWorldSpaceRenderCanvas_RenderThread(FRenderCanvasParameter InCanvasParameter)
{
	FScopeLock scopeLock(&RenderCanvasParameterArray_Mutex);
	int existIndex = RenderCanvasParameterArray.IndexOfByPredicate([&InCanvasParameter](const FRenderCanvasParameter& item) {
		return item.RenderCanvas == InCanvasParameter.RenderCanvas;
		});
	if (existIndex != INDEX_NONE)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIHudRenderer::AddRootCanvas_RenderThread]Canvas already exist!"));
		return;
	}
	RenderCanvasParameterArray.Add(InCanvasParameter);
	RenderCanvasParameterArray.Sort([](const FRenderCanvasParameter& A, const FRenderCanvasParameter& B) {
		return A.RenderCanvasSortOrder < B.RenderCanvasSortOrder;
		});
}
void FLGUIHudRenderer::RemoveWorldSpaceRenderCanvas_RenderThread(ULGUICanvas* InCanvas)
{
	FScopeLock scopeLock(&RenderCanvasParameterArray_Mutex);
	int existIndex = RenderCanvasParameterArray.IndexOfByPredicate([InCanvas](const FRenderCanvasParameter& item) {
		return item.RenderCanvas == InCanvas;
		});
	if (existIndex == INDEX_NONE)
	{
		UE_LOG(LGUI, Error, TEXT("[FLGUIHudRenderer::RemoveRootCanvas_RenderThread]Canvas not exist!"));
		return;
	}
	RenderCanvasParameterArray.RemoveAt(existIndex);
}

void FLGUIHudRenderer::CheckContainsPostProcess_RenderThread()
{
	bContainsPostProcess = false;
	for (auto& item : ScreenSpaceRenderParameter.HudPrimitiveArray)
	{
		if (item->GetIsPostProcess())
		{
			bContainsPostProcess = true;
			return;
		}
	}
	for (auto& renderItem : RenderCanvasParameterArray)
	{
		for (auto& item : renderItem.HudPrimitiveArray)
		{
			if (item->GetIsPostProcess())
			{
				bContainsPostProcess = true;
				return;
			}
		}
	}
}

void FLGUIFullScreenQuadVertexBuffer::InitRHI()
{
	TResourceArray<FLGUIPostProcessVertex, VERTEXBUFFER_ALIGNMENT> Vertices;
	Vertices.SetNumUninitialized(4);

	Vertices[0] = FLGUIPostProcessVertex(FVector(-1, -1, 0), FVector2D(0.0f, 1.0f));
	Vertices[1] = FLGUIPostProcessVertex(FVector(1, -1, 0), FVector2D(1.0f, 1.0f));
	Vertices[2] = FLGUIPostProcessVertex(FVector(-1, 1, 0), FVector2D(0.0f, 0.0f));
	Vertices[3] = FLGUIPostProcessVertex(FVector(1, 1, 0), FVector2D(1.0f, 0.0f));

	FRHIResourceCreateInfo CreateInfo(TEXT("LGUIFullScreenQuadVertexBuffer"), &Vertices);
	VertexBufferRHI = RHICreateVertexBuffer(Vertices.GetResourceDataSize(), BUF_Static, CreateInfo);
}
void FLGUIFullScreenQuadIndexBuffer::InitRHI()
{
	const uint16 Indices[] =
	{
		0, 2, 3,
		0, 3, 1
	};

	TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
	uint32 NumIndices = UE_ARRAY_COUNT(Indices);
	IndexBuffer.AddUninitialized(NumIndices);
	FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

	FRHIResourceCreateInfo CreateInfo(TEXT("LGUIFullScreenQuadIndexBuffer") , &IndexBuffer);
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}
