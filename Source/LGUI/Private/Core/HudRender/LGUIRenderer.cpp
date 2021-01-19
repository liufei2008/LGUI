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
#include "Engine.h"
#include "Editor/EditorEngine.h"
#endif
#include "Slate/SceneViewport.h"
#include "Core/ActorComponent/UIPostProcess.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/LGUISettings.h"
class FLGUIMeshElementCollector : FMeshElementCollector
{
public:
	FLGUIMeshElementCollector(ERHIFeatureLevel::Type InFeatureLevel) :FMeshElementCollector(InFeatureLevel)
	{

	}
};


#if WITH_EDITORONLY_DATA
uint32 FLGUIViewExtension::EditorPreview_ViewKey = 0;
#endif
FLGUIViewExtension::FLGUIViewExtension(const FAutoRegister& AutoRegister, ULGUICanvas* InLGUICanvas, UTextureRenderTarget2D* InCustomRenderTarget)
	:FSceneViewExtensionBase(AutoRegister)
{
	UICanvas = InLGUICanvas;
	World = UICanvas->GetWorld();
	CustomRenderTarget = InCustomRenderTarget;

#if WITH_EDITORONLY_DATA
	IsEditorPreview = !UICanvas->GetWorld()->IsGameWorld();
#endif
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
	ViewProjectionMatrix = UICanvas->GetViewProjectionMatrix();
	MultiSampleCount = (uint16)ULGUISettings::GetAntiAliasingSampleCount();
	
	FScopeLock scopeLock(&Mutex);
	for (int i = 0; i < HudPrimitiveArray.Num(); i++)
	{
		auto hudPrimitive = HudPrimitiveArray[i];
		if (hudPrimitive != nullptr)
		{
			if (hudPrimitive->GetIsPostProcess())
			{
				if (hudPrimitive->GetPostProcessObject().IsValid())
				{
					hudPrimitive->GetPostProcessObject()->OnBeforeRenderPostProcess_GameThread(InViewFamily, InView);
				}
			}
		}
	}
}
void FLGUIViewExtension::SetupViewPoint(APlayerController* Player, FMinimalViewInfo& InViewInfo)
{
	
}
void FLGUIViewExtension::SetupViewProjectionMatrix(FSceneViewProjectionData& InOutProjectionData)
{
	
}

void FLGUIViewExtension::CopyRenderTarget(FRHICommandListImmediate& RHICmdList, TShaderMap<FGlobalShaderType>* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Load_DontStore), TEXT("CopyRenderTarget"));

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
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	VertexShader->SetParameters(RHICmdList);
	PixelShader->SetParameters(RHICmdList, Src);

	DrawFullScreenQuad(RHICmdList);

	RHICmdList.EndRenderPass();
}
void FLGUIViewExtension::CopyRenderTargetOnMeshRegion(FRHICommandListImmediate& RHICmdList, TShaderMap<FGlobalShaderType>* GlobalShaderMap, FTextureRHIRef Src, FTextureRHIRef Dst, const TArray<FLGUIPostProcessVertex>& RegionVertexData)
{
	RHICmdList.BeginRenderPass(FRHIRenderPassInfo(Dst, ERenderTargetActions::Load_DontStore), TEXT("CopyRenderTargetOnMeshRegion"));

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
	SetGraphicsPipelineState(RHICmdList, GraphicsPSOInit);

	VertexShader->SetParameters(RHICmdList);
	PixelShader->SetParameters(RHICmdList, Src);

	uint32 VertexBufferSize = 4 * sizeof(FLGUIPostProcessVertex);
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
void FLGUIViewExtension::DrawFullScreenQuad(FRHICommandListImmediate& RHICmdList)
{
	RHICmdList.SetStreamSource(0, GLGUIFullScreenQuadVertexBuffer.VertexBufferRHI, 0);
	RHICmdList.DrawIndexedPrimitive(GLGUIFullScreenQuadIndexBuffer.IndexBufferRHI, 0, 0, 4, 0, 2, 1);
}
DECLARE_CYCLE_STAT(TEXT("Hud RHIRender"), STAT_Hud_RHIRender, STATGROUP_LGUI);
void FLGUIViewExtension::PostRenderView_RenderThread(FRHICommandListImmediate& RHICmdList, FSceneView& InView)
{
	SCOPE_CYCLE_COUNTER(STAT_Hud_RHIRender);
	check(IsInRenderingThread());
	if (!World.IsValid())return;
	if (InView.Family == nullptr || InView.Family->Scene == nullptr || InView.Family->Scene->GetWorld() == nullptr)return;
	if (World.Get() != InView.Family->Scene->GetWorld())return;
#if WITH_EDITOR
	if (ALGUIManagerActor::IsPlaying == IsEditorPreview)return;
	if (ALGUIManagerActor::IsPlaying)
	{
		if (!InView.bIsGameView)return;
		//if (InView.State->GetViewKey() == EditorPreview_ViewKey)return;
	}
	else//editor viewport preview
	{
		if (InView.GetViewKey() != EditorPreview_ViewKey)return;//only preview in specific viewport in editor
	}
	//simulation
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

	//create render target
	{
		FPooledRenderTargetDesc desc(FPooledRenderTargetDesc::Create2DDesc(InView.UnscaledViewRect.Size(), InView.Family->RenderTarget->GetRenderTargetTexture()->GetFormat(), FClearValueBinding::Black, TexCreate_None, TexCreate_RenderTargetable | TexCreate_ShaderResource, false));
		desc.NumSamples = MultiSampleCount;
		GRenderTargetPool.FindFreeElement(RHICmdList, desc, SceneColorRenderTarget, TEXT("LGUISceneColorRenderTarget"));
		if (!SceneColorRenderTarget.IsValid())
			return;
	}
	auto ScreenColorRenderTexture = SceneColorRenderTarget->GetRenderTargetItem().TargetableTexture;

	FSceneView RenderView(InView);//use a copied view
	FRHIRenderPassInfo RPInfo;
	if (CustomRenderTarget.IsValid())
	{
		ScreenColorRenderTexture = CustomRenderTarget->GetRenderTargetResource()->GetRenderTargetTexture();
		RPInfo = FRHIRenderPassInfo(ScreenColorRenderTexture, ERenderTargetActions::Clear_DontStore);
	}
	else
	{
#if PLATFORM_ANDROID
		RHICmdList.CopyToResolveTarget((FTextureRHIRef)InView.Family->RenderTarget->GetRenderTargetTexture(), ScreenColorRenderTexture, FResolveParams());
#else
		CopyRenderTarget(RHICmdList, GetGlobalShaderMap(RenderView.GetFeatureLevel()), (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture(), ScreenColorRenderTexture);
#endif
		RPInfo = FRHIRenderPassInfo(ScreenColorRenderTexture, ERenderTargetActions::Load_DontStore);
	}
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
	if (MultiSampleCount > 0)
	{
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false, true>::GetRHI();
	}
	else
	{
		GraphicsPSOInit.RasterizerState = TStaticRasterizerState<FM_Solid, CM_None, false, false>::GetRHI();
	}
	GraphicsPSOInit.NumSamples = MultiSampleCount;

	auto GlobalShaderMap = GetGlobalShaderMap(RenderView.GetFeatureLevel());
	for (int i = 0; i < primitiveArray.Num(); i++)
	{
		auto hudPrimitive = primitiveArray[i];
		if (hudPrimitive != nullptr)
		{
			if (hudPrimitive->CanRender())
			{
				if (hudPrimitive->GetIsPostProcess())//render post process
				{
					auto postProcessObject = hudPrimitive->GetPostProcessObject();
					if (postProcessObject.IsValid())
					{
						RHICmdList.EndRenderPass();
						postProcessObject->OnRenderPostProcess_RenderThread(
							RHICmdList,
							ScreenColorRenderTexture,
							GlobalShaderMap,
							ViewProjectionMatrix,
							[&]()
							{
								const FMeshBatch& Mesh = hudPrimitive->GetMeshElement((FMeshElementCollector*)&meshCollector);
								RHICmdList.SetStreamSource(0, hudPrimitive->GetVertexBufferRHI(), 0);
								RHICmdList.DrawIndexedPrimitive(Mesh.Elements[0].IndexBuffer->IndexBufferRHI, 0, 0, hudPrimitive->GetNumVerts(), 0, Mesh.GetNumPrimitives(), 1);
							}
						);
						RHICmdList.BeginRenderPass(RPInfo, TEXT("LGUIHudRender"));
					}
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

						GraphicsPSOInit.BoundShaderState.VertexDeclarationRHI = GetLGUIHudVertexDeclaration();
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
	//copy back to screen
	CopyRenderTarget(RHICmdList, GetGlobalShaderMap(RenderView.GetFeatureLevel()), ScreenColorRenderTexture, (FTextureRHIRef)RenderView.Family->RenderTarget->GetRenderTargetTexture());
	//release render target
	SceneColorRenderTarget.SafeRelease();
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
		HudPrimitiveArray.RemoveSingle(InPrimitive);
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
