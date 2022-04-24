// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
#include "ClearQuad.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITORONLY_DATA
uint32 FLGUIHudRenderer::EditorPreview_ViewKey = 0;
#endif
FLGUIHudRenderer::FLGUIHudRenderer(const FAutoRegister& AutoRegister, UWorld* InWorld)
	:FSceneViewExtensionBase(AutoRegister)
{
	World = InWorld;

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

	}
	//screen space
	if (ScreenSpaceRenderParameter.RenderCanvas.IsValid())
	{
		//@todo: these parameters should use ENQUE_RENDER_COMMAND to pass to render thread
		auto ViewLocation = ScreenSpaceRenderParameter.RenderCanvas->GetViewLocation();
		auto ViewRotationMatrix = FInverseRotationMatrix(ScreenSpaceRenderParameter.RenderCanvas->GetViewRotator()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		auto ProjectionMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetProjectionMatrix();
		auto ViewProjectionMatrix = ScreenSpaceRenderParameter.RenderCanvas->GetViewProjectionMatrix();

		ScreenSpaceRenderParameter.ViewOrigin = ViewLocation;
		ScreenSpaceRenderParameter.ViewRotationMatrix = ViewRotationMatrix;
		ScreenSpaceRenderParameter.ProjectionMatrix = ProjectionMatrix;
		ScreenSpaceRenderParameter.ViewProjectionMatrix = ViewProjectionMatrix;
	}
}
void FLGUIHudRenderer::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{

}
void FLGUIHudRenderer::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}
void FLGUIHudRenderer::BeginRenderViewFamily(FSceneViewFamily& InViewFamily)
{

}
void FLGUIHudRenderer::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	RenderLGUI_RenderThread(RHICmdList, InView);
}
int32 FLGUIHudRenderer::GetPriority() const
{
#if WITH_EDITOR
	auto Priority = ULGUISettings::GetPriorityInSceneViewExtension();
#else
	static auto Priority = ULGUISettings::GetPriorityInSceneViewExtension();
#endif
	return Priority;
}
bool FLGUIHudRenderer::IsActiveThisFrame(class FViewport* InViewport) const
{
#if WITH_EDITOR
	if (GEngine == nullptr) return false;
	bCanRenderScreenSpace = true;
	bIsPlaying = ALGUIManagerActor::GetIsPlaying(World.Get());
	//check if simulation
	if (UEditorEngine* editor = Cast<UEditorEngine>(GEngine))
	{
		if (editor->bIsSimulatingInEditor)bCanRenderScreenSpace = false;
	}

	if (bIsPlaying == bIsEditorPreview)bCanRenderScreenSpace = false;
#endif


	if (!World.IsValid())return false;
	if (InViewport == nullptr)return false;
	if (InViewport->GetClient() == nullptr)return false;
	if (World.Get() != InViewport->GetClient()->GetWorld())return false;//only render self world
	if (bIsRenderToRenderTarget)
	{
		if (!CustomRenderTarget.IsValid())return false;
	}
	return true;
}
void FLGUIHudRenderer::PreRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	
}
void FLGUIHudRenderer::PostRenderBasePass_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{

}

void FLGUIHudRenderer::CopyRenderTarget(FRHICommandListImmediate& RHICmdList, FGlobalShaderMap* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Load_Store), TEXT("LGUICopyRenderTarget"));
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

void FLGUIHudRenderer::CopyRenderTargetOnMeshRegion(
	FRHICommandListImmediate& RHICmdList
	, FTextureRHIRef Dst
	, FTextureRHIRef Src
	, FGlobalShaderMap* GlobalShaderMap
	, const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData
	, const FMatrix& MVP
	, const FIntRect& ViewRect
	, const FVector4& SrcTextureScaleOffset
)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Load_Store), TEXT("LGUICopyRenderTargetOnMeshRegion"));
	RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

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
	PixelShader->SetParameters(RHICmdList, MVP, SrcTextureScaleOffset, Src);

	uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessCopyMeshRegionVertex);
	FRHIResourceCreateInfo CreateInfo;
	FVertexBufferRHIRef VertexBufferRHI = RHICreateVertexBuffer(VertexBufferSize, BUF_Volatile, CreateInfo);
	void* VoidPtr = RHILockVertexBuffer(VertexBufferRHI, 0, VertexBufferSize, RLM_WriteOnly);
	FPlatformMemory::Memcpy(VoidPtr, RegionVertexData.GetData(), VertexBufferSize);
	RHIUnlockVertexBuffer(VertexBufferRHI);

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
void FLGUIHudRenderer::SetGraphicPipelineState(FGraphicsPipelineStateInitializer& GraphicsPSOInit, EBlendMode BlendMode, bool bIsWireFrame, bool bIsTwoSided)
{
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
	
	if (!bIsWireFrame)
	{
		if (bIsTwoSided)
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
		if (bIsTwoSided)
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
void FLGUIHudRenderer::RenderLGUI_RenderThread(
	FRHICommandListImmediate& RHICmdList
	, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_Hud_RHIRender);
	if (ScreenSpaceRenderParameter.HudPrimitiveArray.Num() <= 0 && WorldSpaceRenderCanvasParameterArray.Num() <= 0)return;//nothing to render

	FSceneView RenderView(InView);//use a copied view
	auto GlobalShaderMap = GetGlobalShaderMap(RenderView.GetFeatureLevel());

	//create render target
	FTextureRHIRef ScreenColorRenderTargetTexture = nullptr;
	FTextureRHIRef OriginScreenColorTexture = nullptr;//@todo: can use a normal texture here?
	TRefCountPtr<IPooledRenderTarget> OriginScreenColorRenderTarget = nullptr;

	uint8 NumSamples = 1;
	auto ViewRect = RenderView.UnscaledViewRect;
	if (bIsRenderToRenderTarget)
	{
		if (CustomRenderTarget.IsValid())
		{
			ScreenColorRenderTargetTexture = (FTextureRHIRef)CustomRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
			NumSamples = ScreenColorRenderTargetTexture->GetNumSamples();
			//clear render target
#if WITH_EDITOR
			if (bIsPlaying)
#endif
			{
				FRHIRenderPassInfo RPInfo(ScreenColorRenderTargetTexture, ERenderTargetActions::DontLoad_Store);
				TransitionRenderPassTargets(RHICmdList, RPInfo);
				RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender_ClearRenderTarget"));
				DrawClearQuad(RHICmdList, FLinearColor(0, 0, 0, 0));
				RHICmdList.EndRenderPass();
			}

			ViewRect = FIntRect(0, 0, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y);
		}
	}
	else
	{
		ScreenColorRenderTargetTexture = (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture();
		NumSamples = ScreenColorRenderTargetTexture->GetNumSamples();

		if (bNeedOriginScreenColorTextureOnPostProcess)
		{
			FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(RenderView.Family->RenderTarget->GetRenderTargetTexture()->GetSizeXY(), RenderView.Family->RenderTarget->GetRenderTargetTexture()->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_ShaderResource, false));
			desc.NumSamples = NumSamples;
			GRenderTargetPool.FindFreeElement(RHICmdList, desc, OriginScreenColorRenderTarget, TEXT("LGUISceneColorRenderTarget"));
			if (!OriginScreenColorRenderTarget.IsValid())
				return;
			OriginScreenColorTexture = OriginScreenColorRenderTarget->GetRenderTargetItem().ShaderResourceTexture;
			RHICmdList.CopyTexture(ScreenColorRenderTargetTexture, OriginScreenColorTexture, FRHICopyTextureInfo());
		}
	}

	auto RPInfo = FRHIRenderPassInfo(ScreenColorRenderTargetTexture, ERenderTargetActions::Load_Store);
	FSceneRenderTargets& SceneContext = FSceneRenderTargets::Get(RHICmdList);
	FVector4 DepthTextureScaleOffset;
	FVector4 ViewTextureScaleOffset;
	switch (RenderView.StereoPass)
	{
	case EStereoscopicPass::eSSP_FULL:
	{
		DepthTextureScaleOffset = FVector4(
			(float)ScreenColorRenderTargetTexture->GetSizeXYZ().X / SceneContext.GetSceneDepthSurface()->GetSizeX(),
			(float)ScreenColorRenderTargetTexture->GetSizeXYZ().Y / SceneContext.GetSceneDepthSurface()->GetSizeY(),
			0, 0
		);
		ViewTextureScaleOffset = FVector4(1, 1, 0, 0);
	}
	break;
	case EStereoscopicPass::eSSP_LEFT_EYE:
	{
		DepthTextureScaleOffset = FVector4(
			//(float)ViewRect.Width() / SceneContext.GetSceneDepthSurface()->GetSizeX(),
			//(float)ViewRect.Height() / SceneContext.GetSceneDepthSurface()->GetSizeY(),
			0.5f, 1.0f,
			0, 0
		);
		ViewTextureScaleOffset = DepthTextureScaleOffset;
	}
	break;
	case EStereoscopicPass::eSSP_RIGHT_EYE:
	{
		DepthTextureScaleOffset = FVector4(
			//(float)ViewRect.Width() / SceneContext.GetSceneDepthSurface()->GetSizeX(),
			//(float)ViewRect.Height() / SceneContext.GetSceneDepthSurface()->GetSizeY(),
			0.5f, 1.0f,
			0.5f, 0
		);
		ViewTextureScaleOffset = DepthTextureScaleOffset;
	}
	break;
	}

	RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
	RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);

	//Render world space
	if (WorldSpaceRenderCanvasParameterArray.Num() > 0)
	{
		auto ViewLocation = RenderView.ViewLocation;
		auto ViewRotationMatrix = FInverseRotationMatrix(RenderView.ViewRotation) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
		
		auto ProjectionMatrix = RenderView.SceneViewInitOptions.ProjectionMatrix;
		auto ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * (ViewRotationMatrix)*ProjectionMatrix;

		RenderView.SceneViewInitOptions.ViewOrigin = ViewLocation;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(ProjectionMatrix);

		FViewUniformShaderParameters ViewUniformShaderParameters;
		RenderView.SetupCommonViewUniformBufferParameters(
			ViewUniformShaderParameters,
			ViewRect.Size(),
			1,
			ViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);

		RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

		FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.NumSamples = NumSamples;

		for (int canvasIndex = 0; canvasIndex < WorldSpaceRenderCanvasParameterArray.Num(); canvasIndex++)
		{
			auto& canvasParamItem = WorldSpaceRenderCanvasParameterArray[canvasIndex];
			auto hudPrimitive = canvasParamItem.HudPrimitive;
			if (hudPrimitive != nullptr)
			{
				if (hudPrimitive->CanRender())
				{
					switch (hudPrimitive->GetPrimitiveType())
					{
					case ELGUIHudPrimitiveType::PostProcess://render post process
					{
						RHICmdList.EndRenderPass();
						hudPrimitive->OnRenderPostProcess_RenderThread(
							RHICmdList,
							this,
							OriginScreenColorTexture,
							ScreenColorRenderTargetTexture,
							GlobalShaderMap,
							ViewProjectionMatrix,
							true,
							canvasParamItem.BlendDepth,
							ViewRect,
							DepthTextureScaleOffset,
							ViewTextureScaleOffset
						);
						RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
						RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);
						GraphicsPSOInit.NumSamples = NumSamples;
					}break;
					case ELGUIHudPrimitiveType::Mesh://render mesh
					{
						MeshBatchArray.Reset();
						hudPrimitive->GetMeshElements(*RenderView.Family, (FMeshElementCollector*)&meshCollector, MeshBatchArray);
						for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
						{
							auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
							const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 26
							auto Material = Mesh.MaterialRenderProxy->GetMaterial(RenderView.GetFeatureLevel());
#else
							auto Material = &Mesh.MaterialRenderProxy->GetIncompleteMaterialWithFallback(RenderView.GetFeatureLevel());
#endif
							const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
							auto VertexShader = MaterialShaderMap->GetShader<FLGUIHudRenderVS>();

							auto PixelShader = MaterialShaderMap->GetShader<FLGUIWorldRenderPS>();
							if (VertexShader.IsValid() && PixelShader.IsValid())
							{
								FLGUIHudRenderer::SetGraphicPipelineState(GraphicsPSOInit, Material->GetBlendMode(), Material->IsWireframe(), Material->IsTwoSided());

								GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
								GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
								GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
								GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
								GraphicsPSOInit.NumSamples = NumSamples;
								SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

								VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
								PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
								PixelShader->SetDepthBlendParameter(RHICmdList, canvasParamItem.BlendDepth, DepthTextureScaleOffset, SceneContext.GetSceneDepthSurface());

								RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
								RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
							}
						}
					}break;
					}
				}
			}
		}
	}
	//Render screen space
	if (ScreenSpaceRenderParameter.HudPrimitiveArray.Num() > 0)
	{
#if WITH_EDITOR
		if (!bCanRenderScreenSpace)goto END_LGUI_RENDER;
		if (bIsPlaying)
		{
			if (!InView.bIsGameView)goto END_LGUI_RENDER;
		}
		else//editor viewport preview
		{
			if (InView.GetViewKey() != EditorPreview_ViewKey)goto END_LGUI_RENDER;//only preview in specific viewport in editor
		}
#endif

		RenderView.SceneViewInitOptions.ViewOrigin = ScreenSpaceRenderParameter.ViewOrigin;
		RenderView.SceneViewInitOptions.ViewRotationMatrix = ScreenSpaceRenderParameter.ViewRotationMatrix;
		RenderView.UpdateProjectionMatrix(ScreenSpaceRenderParameter.ProjectionMatrix);

		FViewUniformShaderParameters ViewUniformShaderParameters;
		RenderView.SetupCommonViewUniformBufferParameters(
			ViewUniformShaderParameters,
			ViewRect.Size(),
			1,
			ViewRect,
			RenderView.ViewMatrices,
			FViewMatrices()
		);

		RenderView.ViewUniformBuffer = TUniformBufferRef<FViewUniformShaderParameters>::CreateUniformBufferImmediate(ViewUniformShaderParameters, UniformBuffer_SingleFrame);

		FLGUIMeshElementCollector meshCollector(RenderView.GetFeatureLevel());
		FGraphicsPipelineStateInitializer GraphicsPSOInit;
		RHICmdList.ApplyCachedRenderTargets(GraphicsPSOInit);
		GraphicsPSOInit.NumSamples = NumSamples;

		for (int primitiveIndex = 0; primitiveIndex < ScreenSpaceRenderParameter.HudPrimitiveArray.Num(); primitiveIndex++)
		{
			auto hudPrimitive = ScreenSpaceRenderParameter.HudPrimitiveArray[primitiveIndex];
			if (hudPrimitive != nullptr)
			{
				if (hudPrimitive->CanRender())
				{
					switch (hudPrimitive->GetPrimitiveType())
					{
					case ELGUIHudPrimitiveType::PostProcess://render post process
					{
						RHICmdList.EndRenderPass();
						hudPrimitive->OnRenderPostProcess_RenderThread(
							RHICmdList,
							this,
							OriginScreenColorTexture,
							ScreenColorRenderTargetTexture,
							GlobalShaderMap,
							ScreenSpaceRenderParameter.ViewProjectionMatrix,
							false,
							0.0f,
							ViewRect,
							DepthTextureScaleOffset,
							ViewTextureScaleOffset
						);
						RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
						RHICmdList.SetViewport(ViewRect.Min.X, ViewRect.Min.Y, 0.0f, ViewRect.Max.X, ViewRect.Max.Y, 1.0f);
						GraphicsPSOInit.NumSamples = NumSamples;
					}break;
					case ELGUIHudPrimitiveType::Mesh://render mesh
					{
						MeshBatchArray.Reset();
						hudPrimitive->GetMeshElements(*RenderView.Family, (FMeshElementCollector*)&meshCollector, MeshBatchArray);
						for (int MeshIndex = 0; MeshIndex < MeshBatchArray.Num(); MeshIndex++)
						{
							auto& MeshBatchContainer = MeshBatchArray[MeshIndex];
							const FMeshBatch& Mesh = MeshBatchContainer.Mesh;
#if ENGINE_MAJOR_VERSION == 4 && ENGINE_MINOR_VERSION <= 26
							auto Material = Mesh.MaterialRenderProxy->GetMaterial(RenderView.GetFeatureLevel());
#else
							auto Material = &Mesh.MaterialRenderProxy->GetIncompleteMaterialWithFallback(RenderView.GetFeatureLevel());
#endif
							const FMaterialShaderMap* MaterialShaderMap = Material->GetRenderingThreadShaderMap();
							auto VertexShader = MaterialShaderMap->GetShader<FLGUIHudRenderVS>();

							auto PixelShader = MaterialShaderMap->GetShader<FLGUIHudRenderPS>();
							if (VertexShader.IsValid() && PixelShader.IsValid())
							{
								FLGUIHudRenderer::SetGraphicPipelineState(GraphicsPSOInit, Material->GetBlendMode(), Material->IsWireframe(), Material->IsTwoSided());

								GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
								GraphicsPSOInit.BoundShaderState.VertexShaderRHI = VertexShader.GetVertexShader();
								GraphicsPSOInit.BoundShaderState.PixelShaderRHI = PixelShader.GetPixelShader();
								GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
								GraphicsPSOInit.NumSamples = NumSamples;
								SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

								VertexShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);
								PixelShader->SetMaterialShaderParameters(RHICmdList, RenderView, Mesh.MaterialRenderProxy, Material, Mesh);

								RHICmdList.SetStreamSource(0, MeshBatchContainer.VertexBufferRHI, 0);
								RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, MeshBatchContainer.NumVerts, 0, Mesh.GetNumPrimitives(), 1);
							}
						}
					}break;
					}
				}
			}
		}
	}
#if WITH_EDITOR
	END_LGUI_RENDER :
#endif

	RHICmdList.EndRenderPass();

	if (OriginScreenColorRenderTarget.IsValid())
	{
		OriginScreenColorRenderTarget.SafeRelease();
	}
}

void FLGUIHudRenderer::AddWorldSpacePrimitive_RenderThread(ULGUICanvas* InCanvas, ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		FWorldSpaceRenderParameter RenderParameter;
		RenderParameter.BlendDepth = InCanvas->GetActualBlendDepth();
		RenderParameter.RenderCanvas = InCanvas;
		RenderParameter.HudPrimitive = InPrimitive;

		WorldSpaceRenderCanvasParameterArray.Add(RenderParameter);
		SortPrimitiveRenderPriority_RenderThread();
		//check if we have postprocess
		CheckContainsPostProcess_RenderThread();
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
		int existIndex = WorldSpaceRenderCanvasParameterArray.IndexOfByPredicate([InPrimitive](const FWorldSpaceRenderParameter& item) {
			return item.HudPrimitive == InPrimitive;
			});
		if (existIndex == INDEX_NONE)
		{
			UE_LOG(LGUI, Log, TEXT("[FLGUIHudRenderer::RemoveHudPrimitive]Canvas already removed."));
		}
		else
		{
			WorldSpaceRenderCanvasParameterArray.RemoveAt(existIndex);
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
	WorldSpaceRenderCanvasParameterArray.Sort([](const FWorldSpaceRenderParameter& A, const FWorldSpaceRenderParameter& B) {
			return A.HudPrimitive->GetRenderPriority() < B.HudPrimitive->GetRenderPriority();
		});
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
void FLGUIHudRenderer::SetRenderCanvasBlendDepth(ULGUICanvas* InRenderCanvas, float InBlendDepth)
{
	auto viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension, InRenderCanvas, InBlendDepth](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SetRenderCanvasBlendDepth_RenderThread(InRenderCanvas, InBlendDepth);
		}
	);
}
void FLGUIHudRenderer::SetRenderCanvasBlendDepth_RenderThread(ULGUICanvas* InRenderCanvas, float InBlendDepth)
{
	for (auto& RenderParameter : WorldSpaceRenderCanvasParameterArray)
	{
		if (RenderParameter.RenderCanvas == InRenderCanvas)
		{
			RenderParameter.BlendDepth = InBlendDepth;
		}
	}
}

void FLGUIHudRenderer::SetScreenSpaceRenderCanvas(ULGUICanvas* InCanvas)
{
	ScreenSpaceRenderParameter.RenderCanvas = InCanvas;
}
void FLGUIHudRenderer::ClearScreenSpaceRenderCanvas()
{
	ScreenSpaceRenderParameter.RenderCanvas = nullptr;
}

void FLGUIHudRenderer::SetRenderToRenderTarget(bool InValue, UTextureRenderTarget2D* InRenderTarget)
{
	bIsRenderToRenderTarget = InValue;
	CustomRenderTarget = InRenderTarget;
}

void FLGUIHudRenderer::CheckContainsPostProcess_RenderThread()
{
	bNeedOriginScreenColorTextureOnPostProcess = false;
	for (auto& item : ScreenSpaceRenderParameter.HudPrimitiveArray)
	{
		if (item->GetPrimitiveType() == ELGUIHudPrimitiveType::PostProcess
			&& item->PostProcessRequireOriginScreenColorTexture()
			)
		{
			bNeedOriginScreenColorTextureOnPostProcess = true;
			return;
		}
	}
	for (auto& renderItem : WorldSpaceRenderCanvasParameterArray)
	{
		if (renderItem.HudPrimitive->GetPrimitiveType() == ELGUIHudPrimitiveType::PostProcess
			&& renderItem.HudPrimitive->PostProcessRequireOriginScreenColorTexture()
			)
		{
			bNeedOriginScreenColorTextureOnPostProcess = true;
			return;
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

	FRHIResourceCreateInfo CreateInfo(&Vertices);
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

	FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}
void FLGUIFullScreenSlicedQuadIndexBuffer::InitRHI()
{
	uint16 Indices[54];
	int wSeg = 3, hSeg = 3;
	int vStartIndex = 0;
	int triangleArrayIndex = 0;
	for (int h = 0; h < hSeg; h++)
	{
		for (int w = 0; w < wSeg; w++)
		{
			int vIndex = vStartIndex + w;
			Indices[triangleArrayIndex++] = vIndex;
			Indices[triangleArrayIndex++] = vIndex + wSeg + 2;
			Indices[triangleArrayIndex++] = vIndex + wSeg + 1;

			Indices[triangleArrayIndex++] = vIndex;
			Indices[triangleArrayIndex++] = vIndex + 1;
			Indices[triangleArrayIndex++] = vIndex + wSeg + 2;
		}
		vStartIndex += wSeg + 1;
	}

	TResourceArray<uint16, INDEXBUFFER_ALIGNMENT> IndexBuffer;
	uint32 NumIndices = UE_ARRAY_COUNT(Indices);
	IndexBuffer.AddUninitialized(NumIndices);
	FMemory::Memcpy(IndexBuffer.GetData(), Indices, NumIndices * sizeof(uint16));

	FRHIResourceCreateInfo CreateInfo(&IndexBuffer);
	IndexBufferRHI = RHICreateIndexBuffer(sizeof(uint16), IndexBuffer.GetResourceDataSize(), BUF_Static, CreateInfo);
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
