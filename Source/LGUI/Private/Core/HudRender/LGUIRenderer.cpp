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
FLGUIHudRenderer::FLGUIHudRenderer(const FAutoRegister& AutoRegister, ULGUICanvas* InLGUICanvas, UTextureRenderTarget2D* InCustomRenderTarget)
	:FSceneViewExtensionBase(AutoRegister)
{
	UICanvas = InLGUICanvas;
	World = UICanvas->GetWorld();
	CustomRenderTarget = InCustomRenderTarget;

#if WITH_EDITORONLY_DATA
	bIsEditorPreview = !UICanvas->GetWorld()->IsGameWorld();
#endif
}
FLGUIHudRenderer::~FLGUIHudRenderer()
{
	
}

void FLGUIHudRenderer::SetupView(FSceneViewFamily& InViewFamily, FSceneView& InView)
{
	if (!UICanvas.IsValid())return;

	ViewLocation = UICanvas->GetViewLocation();
	ViewRotationMatrix = UICanvas->GetViewRotationMatrix();
	ProjectionMatrix = UICanvas->GetProjectionMatrix();
	ViewProjectionMatrix = UICanvas->GetViewProjectionMatrix();
	MultiSampleCount = (uint16)ULGUISettings::GetAntiAliasingSampleCount();
}
void FLGUIHudRenderer::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{
	
}
void FLGUIHudRenderer::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}

void FLGUIHudRenderer::CopyRenderTarget(FRHICommandListImmediate& RHICmdList, TShaderMap<FGlobalShaderType>* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst)
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
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
	GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	VertexShader->SetParameters(RHICmdList);
	PixelShader->SetParameters(RHICmdList, Src);

	DrawFullScreenQuad(RHICmdList);

	RHICmdList.EndRenderPass();
}
void FLGUIHudRenderer::CopyRenderTargetOnMeshRegion(FRHICommandListImmediate& RHICmdList, TShaderMap<FGlobalShaderType>* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst, const TArray<FLGUIPostProcessCopyMeshRegionVertex>& RegionVertexData, const FMatrix& MVP)
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
	GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(*VertexShader);
	GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(*PixelShader);
	GraphicsPSOInit.PrimitiveType = EPrimitiveType::PT_TriangleList;
	GraphicsPSOInit.NumSamples = Dst->GetNumSamples();
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	//VertexShader->SetParameters(RHICmdList);
	PixelShader->SetParameters(RHICmdList, MVP, Src);

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
	if (!UICanvas.IsValid())return;
#if WITH_EDITOR
	if (ALGUIManagerActor::IsPlaying == bIsEditorPreview)return;
	if (ALGUIManagerActor::IsPlaying)
	{
		if (!InView.bIsGameView)return;
		//if (InView.State->GetViewKey() == EditorPreview_ViewKey)return;
	}
	else//editor viewport preview
	{
		if (InView.GetViewKey() != EditorPreview_ViewKey)return;//only preview in specific viewport in editor
	}
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
	if (World.Get() != InView.Family->Scene->GetWorld())return;
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
		if (bHasPostProcess)
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

	RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
	RHICmdList.SetViewport(0, 0, 0.0f, ScreenColorRenderTargetTexture->GetSizeXYZ().X, ScreenColorRenderTargetTexture->GetSizeXYZ().Y, 1.0f);

	RenderView.SceneViewInitOptions.ViewOrigin = ViewLocation;
	RenderView.SceneViewInitOptions.ViewRotationMatrix = ViewRotationMatrix;
	RenderView.UpdateProjectionMatrix(ProjectionMatrix);

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

	for (int i = 0; i < HudPrimitiveArray.Num(); i++)
	{
		auto hudPrimitive = HudPrimitiveArray[i];
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
						ViewProjectionMatrix
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
					FLGUIHudRenderVS* VertexShader = (FLGUIHudRenderVS*)MaterialShaderMap->GetShader(&FLGUIHudRenderVS::StaticType);
					FLGUIHudRenderPS* PixelShader = (FLGUIHudRenderPS*)MaterialShaderMap->GetShader(&FLGUIHudRenderPS::StaticType);
					if (VertexShader && PixelShader)
					{
						SetGraphicPipelineStateFromMaterial(GraphicsPSOInit, Material);

						GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
						GraphicsPSOInit.BoundShaderState.VertexShaderRHI = GETSAFERHISHADER_VERTEX(VertexShader);
						GraphicsPSOInit.BoundShaderState.PixelShaderRHI = GETSAFERHISHADER_PIXEL(PixelShader);
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
	RHICmdList.EndRenderPass();
	//copy back to screen
	if (CustomRenderTarget.IsValid())
	{
		
	}
	else
	{
		if (bHasPostProcess)
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

void FLGUIHudRenderer::AddHudPrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		HudPrimitiveArray.AddUnique(InPrimitive);
		SortRenderPriority_RenderThread();//I don't know which time the primitive is added, because I don't know when SceneProxy or RenderProxy is created, so I need to sort it every time a new one added.
		//check if we have postprocess
		CheckHasPostProcess();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::AddHudPrimitive_RenderThread]Add nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::RemoveHudPrimitive_RenderThread(ILGUIHudPrimitive* InPrimitive)
{
	if (InPrimitive != nullptr)
	{
		HudPrimitiveArray.RemoveSingle(InPrimitive);
		//check if we have postprocess
		CheckHasPostProcess();
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[FLGUIHudRenderer::RemoveHudPrimitive]Remove nullptr as ILGUIHudPrimitive!"));
	}
}
void FLGUIHudRenderer::CheckHasPostProcess()
{
	bHasPostProcess = false;
	for (auto item : HudPrimitiveArray)
	{
		if (item->CanRender() && item->GetIsPostProcess())
		{
			bHasPostProcess = true;
			break;
		}
	}
}

void FLGUIHudRenderer::SortRenderPriority_RenderThread()
{
	HudPrimitiveArray.Sort([](ILGUIHudPrimitive& A, ILGUIHudPrimitive& B)
	{
		return A.GetRenderPriority() < B.GetRenderPriority();
	});
}
void FLGUIHudRenderer::SortRenderPriority()
{
	FLGUIHudRenderer* viewExtension = this;
	ENQUEUE_RENDER_COMMAND(FLGUIRender_SortRenderPriority)(
		[viewExtension](FRHICommandListImmediate& RHICmdList)
		{
			viewExtension->SortRenderPriority_RenderThread();
		}
	);
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
