// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Utils/LGUIUtils.h"
#include "CoreGlobals.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "EditorViewportClient.h"
#endif
#include "Utils/LGUIUtils.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/HudRender/LGUIRenderer.h"
#include "Core/LGUIMesh/LGUIMeshComponent.h"
#include "Core/UIDrawcall.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Core/ActorComponent/UIDirectMeshRenderable.h"
#include "Core/ActorComponent/UIItem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "SceneViewExtension.h"
#include "Engine/Engine.h"
#include "Core/UIPostProcessRenderProxy.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/TransformCalculus2D.h"

#define LOCTEXT_NAMESPACE "LGUICanvas"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

ULGUICanvas::ULGUICanvas()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bHasAddToLGUIManager = false;
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;

	bHasAddToLGUIScreenSpaceRenderer = false;
	bHasAddToLGUIWorldSpaceRenderer = false;
	bOverrideViewLocation = false;
	bOverrideViewRotation = false;
	bOverrideProjectionMatrix = false;
	bOverrideFovAngle = false;
	bPrevUIItemIsActive = true;

	bCanTickUpdate = true;
	bShouldRebuildDrawcall = true;
	bShouldSortRenderableOrder = true;

	bIsViewProjectionMatrixDirty = true;

	DefaultMeshType = ULGUIMeshComponent::StaticClass();
}

void ULGUICanvas::BeginPlay()
{
	Super::BeginPlay();
	CheckRootCanvas();
	bCurrentIsLGUIRendererOrUERenderer = IsRenderByLGUIRendererOrUERenderer();
	if (CheckUIItem())
	{
		bPrevUIItemIsActive = UIItem->GetIsUIActiveInHierarchy();
	}
	else
	{
		bPrevUIItemIsActive = false;
	}
	MarkCanvasUpdate(true, true, true, true);

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;	
	bNeedToSortRenderPriority = true;
}
void ULGUICanvas::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ULGUICanvas::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ULGUICanvas::GetRenderTargetViewExtension()
{
	if (!RenderTargetViewExtension.IsValid())
	{
		RenderTargetViewExtension = FSceneViewExtensions::NewExtension<FLGUIHudRenderer>(GetWorld());
	}
	return RenderTargetViewExtension;
}

void ULGUICanvas::UpdateRootCanvas()
{
	if (ensure(this == RootCanvas))
	{
		if (bCurrentIsLGUIRendererOrUERenderer)
		{
			switch (GetActualRenderMode())
			{
			case ELGUIRenderMode::ScreenSpaceOverlay:
			{
				if (!bHasAddToLGUIScreenSpaceRenderer)
				{
					TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension = nullptr;
#if WITH_EDITOR
					if (!GetWorld()->IsGameWorld())//editor world, screen space UI not need to add to ViewExtension
					{

					}
					else
#endif
					{
						ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), true);
					}

					if (ViewExtension.IsValid())//only root canvas can add screen space UI to LGUIRenderer
					{
						ViewExtension->SetScreenSpaceRenderCanvas(this);
						bHasAddToLGUIScreenSpaceRenderer = true;
					}
				}
			}
			break;
			case ELGUIRenderMode::RenderTarget:
			{
				if (!bHasAddToLGUIScreenSpaceRenderer)
				{
					if (IsValid(renderTarget))
					{
						GetRenderTargetViewExtension();
						RenderTargetViewExtension->SetScreenSpaceRenderCanvas(this);
						RenderTargetViewExtension->SetRenderToRenderTarget(true, renderTarget);
						bHasAddToLGUIScreenSpaceRenderer = true;
					}
				}
			}
			break;
			}
		}

		if (CheckUIItem())
		{
			/**
			 * Why use bPrevUIItemIsActive?:
			 * If Canvas is rendering in frame 1, but when in frame 2 the Canvas is disabled(by disable UIItem), then the Canvas will not do drawcall calculation, and the prev existing drawcall mesh is still there and render,
			 * so we check bPrevUIItemIsActive, then we can still do drawcall calculation at this frame, and the prev existing drawcall will be removed.
			 */
			bool bNowUIItemIsActive = UIItem->GetIsUIActiveInHierarchy();
			if (bNowUIItemIsActive || bPrevUIItemIsActive)
			{
				bPrevUIItemIsActive = bNowUIItemIsActive;

				UpdateCanvasDrawcallRecursive();
				MarkFinishRenderFrameRecursive();
			}
		}
	}
}

void ULGUICanvas::EnsureDrawcallObjectReference()
{
	for (int i = 0; i < UIRenderableList.Num(); i++)
	{
		if (!IsValid(UIRenderableList[i]))
		{
			UIRenderableList.RemoveAt(i);
			i--;
		}
	}

	for (auto& item : UIDrawcallList)
	{
		switch (item->type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			for (int i = 0; i < item->renderObjectList.Num(); i++)
			{
				if (!item->renderObjectList[i].IsValid())
				{
					item->renderObjectList.RemoveAt(i);
					i--;
				}
			}
		}
		break;
		}
	}
}

void ULGUICanvas::OnRegister()
{
	Super::OnRegister();
	if (CheckUIItem())
	{
		if (auto world = this->GetWorld())
		{
			if (!bHasAddToLGUIManager)
			{
				bHasAddToLGUIManager = true;
#if WITH_EDITOR
				if (!world->IsGameWorld())
				{
					ULGUIEditorManagerObject::AddCanvas(this);
				}
				else
#endif
				{
					ALGUIManagerActor::AddCanvas(this);
				}
			}
		}
		//tell UIItem
		UIItem->RegisterRenderCanvas(this);
		UIHierarchyChangedDelegateHandle = UIItem->RegisterUIHierarchyChanged(FSimpleDelegate::CreateUObject(this, &ULGUICanvas::OnUIHierarchyChanged));
		UIActiveStateChangedDelegateHandle = UIItem->RegisterUIActiveStateChanged(FSimpleDelegate::CreateUObject(this, &ULGUICanvas::OnUIActiveStateChanged));

		OnUIHierarchyChanged();
	}
}
void ULGUICanvas::OnUnregister()
{
	Super::OnUnregister();
	if (auto world = this->GetWorld())
	{
		if (bHasAddToLGUIManager)
		{
			bHasAddToLGUIManager = false;
#if WITH_EDITOR
			if (!world->IsGameWorld())
			{
				ULGUIEditorManagerObject::RemoveCanvas(this);
			}
			else
#endif
			{
				ALGUIManagerActor::RemoveCanvas(this);
			}
		}
	}

	//clear
	ClearDrawcall();

	//remove from parent canvas
	if (ParentCanvas.IsValid())
	{
		SetParentCanvas(nullptr);
	}

	//tell UIItem
	if (UIItem.IsValid())
	{
		UIItem->UnregisterRenderCanvas();
		UIItem->UnregisterUIHierarchyChanged(UIHierarchyChangedDelegateHandle);
		UIItem->UnregisterUIActiveStateChanged(UIActiveStateChangedDelegateHandle);
	}

	RemoveFromViewExtension();
}
void ULGUICanvas::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void ULGUICanvas::ClearDrawcall()
{
	for (auto item : PooledUIMeshList)
	{
		if (item.IsValid())
		{
			item->ClearAllMeshSection();
			item->DestroyComponent();
		}
	}
	PooledUIMeshList.Empty();
	for (auto item : UsingUIMeshList)
	{
		if (item.IsValid())
		{
			item->DestroyComponent();
		}
	}
	UsingUIMeshList.Empty();

	PooledUIMaterialList.Empty();
	UIDrawcallList.Empty();
	CacheUIDrawcallList.Empty();
}

void ULGUICanvas::RemoveFromViewExtension()
{
	if (bHasAddToLGUIWorldSpaceRenderer)
	{
		bHasAddToLGUIWorldSpaceRenderer = false;
		TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension = nullptr;
#if WITH_EDITOR
		if (!GetWorld()->IsGameWorld())
		{
			ViewExtension = ULGUIEditorManagerObject::GetViewExtension(GetWorld(), false);
		}
		else
#endif
		{
			ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
		}
		if (ViewExtension.IsValid())
		{
			ViewExtension->RemoveWorldSpaceRenderCanvas(this);
		}
	}

	if (bHasAddToLGUIScreenSpaceRenderer)
	{
		bHasAddToLGUIScreenSpaceRenderer = false;
		TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension = nullptr;
#if WITH_EDITOR
		if (!GetWorld()->IsGameWorld())
		{
			ViewExtension = ULGUIEditorManagerObject::GetViewExtension(GetWorld(), false);
		}
		else
#endif
		{
			ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
		}
		if (ViewExtension.IsValid())
		{
			ViewExtension->ClearScreenSpaceRenderCanvas();
		}
	}

	if (RenderTargetViewExtension.IsValid())
	{
		RenderTargetViewExtension->SetRenderToRenderTarget(false, nullptr);
	}
}

bool ULGUICanvas::CheckRootCanvas()const
{
	if (RootCanvas.IsValid())return true;
	if (this->GetWorld() == nullptr)return false;
	ULGUICanvas* ResultCanvas = nullptr;
	LGUIUtils::FindRootCanvas(this->GetOwner(), ResultCanvas);
	RootCanvas = ResultCanvas;
	if (RootCanvas.IsValid()) return true;
	return false;
}

void ULGUICanvas::SetParentCanvas(ULGUICanvas* InParentCanvas)
{
	if (ParentCanvas != InParentCanvas)
	{
		this->ClearDrawcall();
		this->MarkCanvasUpdate(false, false, true, true);
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->ChildrenCanvasArray.Remove(this);
			ParentCanvas->UIRenderableList.Remove(this->UIItem.Get());
			ParentCanvas->MarkCanvasUpdate(false, false, true, true);
		}
		ParentCanvas = InParentCanvas;
		if (ParentCanvas.IsValid())
		{
#if !UE_BUILD_SHIPPING
			check(!ParentCanvas->ChildrenCanvasArray.Contains(this));
			check(!ParentCanvas->UIRenderableList.Contains(this->UIItem.Get()));
#endif
			if (this->UIItem->GetIsUIActiveInHierarchy())
			{
				ParentCanvas->UIRenderableList.AddUnique(this->UIItem.Get());
			}
			ParentCanvas->ChildrenCanvasArray.AddUnique(this);
			ParentCanvas->MarkCanvasUpdate(false, false, true, true);
		}
	}
}

bool ULGUICanvas::CheckUIItem()const
{
	if (UIItem.IsValid())return true;
	if (this->GetWorld() == nullptr)return false;
	UIItem = Cast<UUIItem>(GetOwner()->GetRootComponent());
	if (!UIItem.IsValid())
	{
		UE_LOG(LGUI, Error, TEXT("LGUICanvas component should only attach to a actor which have UIItem as RootComponent!"));
		return false;
	}
	else
	{
		return true;
	}
}
void ULGUICanvas::CheckRenderMode()
{
	RemoveFromViewExtension();

	bool oldIsLGUIRendererOrUERenderer = bCurrentIsLGUIRendererOrUERenderer;
	if (RootCanvas.IsValid())
	{
		bCurrentIsLGUIRendererOrUERenderer = RootCanvas->IsRenderByLGUIRendererOrUERenderer();
	}
	//if hierarchy changed from World/Hud to Hud/World, then we need to recreate all
	if (bCurrentIsLGUIRendererOrUERenderer != oldIsLGUIRendererOrUERenderer)
	{
		if (CheckUIItem())
		{
			UIItem->MarkAllDirtyRecursive();
		}
		//clear drawcall, delete mesh, because UE/LGUI render's mesh data not compatible
		this->ClearDrawcall();
	}

	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		ChildCanvas->CheckRenderMode();
	}
}
void ULGUICanvas::OnUIHierarchyChanged()
{
	this->bCanTickUpdate = true;
	RootCanvas = nullptr;
	CheckRootCanvas();
	CheckRenderMode();

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;

	ULGUICanvas* NewParentCanvas = nullptr;
	if (this->IsRegistered())
	{
		NewParentCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(this->GetOwner()->GetAttachParentActor(), true);
	}
	SetParentCanvas(NewParentCanvas);
}

void ULGUICanvas::OnUIActiveStateChanged()
{
	if (this->UIItem->GetIsUIActiveInHierarchy())
	{
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->UIRenderableList.AddUnique(this->UIItem.Get());
		}
	}
	else
	{
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->UIRenderableList.Remove(this->UIItem.Get());
		}
	}
}

bool ULGUICanvas::IsRenderToScreenSpace()
{
	if (RootCanvas.IsValid())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay;
	}
	return false;
}
bool ULGUICanvas::IsRenderToRenderTarget()
{
	if (RootCanvas.IsValid())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::RenderTarget;
	}
	return false;
}
bool ULGUICanvas::IsRenderToWorldSpace()
{
	if (RootCanvas.IsValid())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::WorldSpace
			|| RootCanvas->renderMode == ELGUIRenderMode::WorldSpace_LGUI
			;
	}
	return false;
}

bool ULGUICanvas::IsRenderByLGUIRendererOrUERenderer()
{
	if (RootCanvas.IsValid())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay
			|| RootCanvas->renderMode == ELGUIRenderMode::RenderTarget
			|| RootCanvas->renderMode == ELGUIRenderMode::WorldSpace_LGUI
			;
	}
	return false;
}

void ULGUICanvas::MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall)
{
	this->bCanTickUpdate = true;
	if (bMaterialOrTextureChanged || bTransformOrVertexPositionChanged || bHierarchyOrderChanged || bForceRebuildDrawcall)
	{
		this->bShouldRebuildDrawcall = true;
	}
	if (bHierarchyOrderChanged)
	{
		this->bShouldSortRenderableOrder = true;
	}
}

#if WITH_EDITOR
void ULGUICanvas::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (CheckUIItem())
	{
		UIItem->MarkAllDirtyRecursive();
	}
	if (CheckRootCanvas())
	{
		RootCanvas->MarkCanvasUpdate(true, true, true);
	}
}
void ULGUICanvas::PostLoad()
{
	Super::PostLoad();
}
#endif

UMaterialInterface** ULGUICanvas::GetMaterials()
{
	auto CheckDefaultMaterialsFunction = [=] 
	{
		for (int i = 0; i < LGUI_DEFAULT_MATERIAL_COUNT; i++)
		{
			if (DefaultMaterials[i] == nullptr)
			{
				FString matPath;
				switch (i)
				{
				default:
				case 0: matPath = TEXT("/LGUI/LGUI_Standard"); break;
				case 1: matPath = TEXT("/LGUI/LGUI_Standard_RectClip"); break;
				case 2: matPath = TEXT("/LGUI/LGUI_Standard_TextureClip"); break;
				}
				auto mat = LoadObject<UMaterialInterface>(NULL, *matPath);
				if (mat == nullptr)
				{
					auto errMsg = LOCTEXT("AssignMaterialError_MissingSourceMaterial", "[ULGUICanvas/CheckMaterials]Assign material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure.");
					UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
					LGUIUtils::EditorNotification(errMsg, 10);
#endif
					continue;
				}
				DefaultMaterials[i] = mat;
			}
		}
	};
	if(IsRootCanvas())
	{
		CheckDefaultMaterialsFunction();
	}
	else
	{
		if (GetOverrideDefaultMaterials())
		{
			CheckDefaultMaterialsFunction();
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetMaterials();
			}
			else
			{
				CheckDefaultMaterialsFunction();
			}
		}
	}
	return &DefaultMaterials[0];
}


ULGUICanvas* ULGUICanvas::GetRootCanvas() const
{ 
	CheckRootCanvas(); 
	return RootCanvas.Get(); 
}
bool ULGUICanvas::IsRootCanvas()const
{
	return RootCanvas == this;
}

bool ULGUICanvas::GetIsUIActive()const
{
	if (UIItem.IsValid())
	{
		return UIItem->GetIsUIActiveInHierarchy();
	}
	return false;
}

void ULGUICanvas::AddUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	UIRenderableList.AddUnique(InUIRenderable);
	MarkCanvasUpdate(false, false, true);
}

void ULGUICanvas::RemoveUIRenderable(UUIBaseRenderable* UIRenderableItem)
{
	if (UIRenderableList.Remove(UIRenderableItem) > 0)
	{
		auto Drawcall = UIRenderableItem->drawcall;
		if (Drawcall.IsValid())
		{
			switch (UIRenderableItem->GetUIRenderableType())
			{
			case EUIRenderableType::UIBatchGeometryRenderable:
			{
				auto UIBatchGeometryRenderable = (UUIBatchGeometryRenderable*)UIRenderableItem;
				auto index = Drawcall->renderObjectList.IndexOfByKey(UIBatchGeometryRenderable);
				if (index != INDEX_NONE)
				{
					Drawcall->renderObjectList.RemoveAt(index);
					Drawcall->needToUpdateVertex = true;
				}
			}
			break;
			case EUIRenderableType::UIPostProcessRenderable:
			{
				if (Drawcall->postProcessRenderableObject.IsValid())
				{
					if (Drawcall->postProcessRenderableObject->IsRenderProxyValid())
					{
						Drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
					}
				}
			}
			break;
			case EUIRenderableType::UIDirectMeshRenderable:
			{
				if (Drawcall->directMeshRenderableObject.IsValid())
				{
					Drawcall->directMeshRenderableObject->ClearMeshData();
				}
			}
			break;
			}
			UIRenderableItem->drawcall = nullptr;
		}
		MarkCanvasUpdate(false, false, true);
	}
}

bool ULGUICanvas::Is2DUITransform(const FTransform& Transform)
{
#if WITH_EDITOR
	float threshold = ULGUISettings::GetAutoBatchThreshold();
#else
	static float threshold = ULGUISettings::GetAutoBatchThreshold();
#endif
	if (FMath::Abs(Transform.GetLocation().X) > threshold)//location X moved
	{
		return false;
	}
	auto rotation = Transform.GetRotation().Rotator();
	if (FMath::Abs(rotation.Yaw) > threshold || FMath::Abs(rotation.Pitch) > threshold)//rotate
	{
		return false;
	}
	return true;
}

void ULGUICanvas::UpdateGeometry_Implement()
{
	//hierarchy change, need to sort it
	if (bShouldSortRenderableOrder)
	{
		bShouldSortRenderableOrder = false;
		UIRenderableList.Sort([](const UUIItem& A, const UUIItem& B) {
			return A.GetFlattenHierarchyIndex() < B.GetFlattenHierarchyIndex();
			});
	}
	//for sorted ui items, iterate from head to tail, compare drawcall from tail to head
	for (int i = 0; i < UIRenderableList.Num(); i++)
	{
		auto& Item = UIRenderableList[i];
		//check(Item->GetIsUIActiveInHierarchy());
		if (!Item->GetIsUIActiveInHierarchy())continue;
		if (!Item->GetRenderCanvas())continue;

		if (Item->IsCanvasUIItem() && Item->GetRenderCanvas() != this)//is child canvas
		{
			auto ChildRenderCanvas = Item->GetRenderCanvas();
		}
		else
		{
			auto UIRenderableItem = (UUIBaseRenderable*)(Item);
			UIRenderableItem->UpdateGeometry();
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas BatchDrawcall"), STAT_BatchDrawcall, STATGROUP_LGUI);
void ULGUICanvas::BatchDrawcall_Implement(const FVector2D& InCanvasLeftBottom, const FVector2D& InCanvasRightTop, TArray<TSharedPtr<UUIDrawcall>>& InUIDrawcallList, TArray<TSharedPtr<UUIDrawcall>>& InCacheUIDrawcallList, bool& OutNeedToSortRenderPriority)
{
	SCOPE_CYCLE_COUNTER(STAT_BatchDrawcall);
	
	auto CanvasRect = UIQuadTree::Rectangle(InCanvasLeftBottom, InCanvasRightTop);

	auto IntersectBounds = [](FVector2D aMin, FVector2D aMax, FVector2D bMin, FVector2D bMax) {
		return !(bMin.X >= aMax.X
			|| bMax.X <= aMin.X
			|| bMax.Y <= aMin.Y
			|| bMin.Y >= aMax.Y
			);
	};
	FLGUICacheTransformContainer OtherItemToCanvasTf;
	auto OverlapWithOtherDrawcall = [&](const FLGUICacheTransformContainer& ItemToCanvasTf, TSharedPtr<UUIDrawcall> DrawcallItem) {
		switch (DrawcallItem->type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			//compare drawcall item's bounds
			if (DrawcallItem->renderObjectListTreeRootNode->Overlap(UIQuadTree::Rectangle(ItemToCanvasTf.BoundsMin2D, ItemToCanvasTf.BoundsMax2D)))
			{
				return true;
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			DrawcallItem->postProcessRenderableObject->GetRenderCanvas()->GetCacheUIItemToCanvasTransform(DrawcallItem->postProcessRenderableObject.Get(), OtherItemToCanvasTf);
			//check bounds overlap
			if (IntersectBounds(ItemToCanvasTf.BoundsMin2D, ItemToCanvasTf.BoundsMax2D, OtherItemToCanvasTf.BoundsMin2D, OtherItemToCanvasTf.BoundsMax2D))
			{
				return true;
			}
		}
		break;
		case EUIDrawcallType::DirectMesh://mostly direct mesh are difficult to calculate 2d bounds (particles or static-mesh), so just return true-overlap
		{
			return true;
		}
		break;
		}

		return false;
	};

	int FitInDrawcallMinIndex = InUIDrawcallList.Num();//0 means the first canvas that processing drawcall. if not 0 means this is child canvas, then we should skip the previours canvas when batch drawcall, because child canvas's UI element can't batch into other canvas's drawcall
	auto CanFitInDrawcall = [&](UUIBatchGeometryRenderable* InUIItem, bool InIs2DUI, int32 InUIItemVerticesCount, const FLGUICacheTransformContainer& InUIItemToCanvasTf, int32& OutDrawcallIndexToFitin)
	{
		auto LastDrawcallIndex = InUIDrawcallList.Num() - 1;
		if (LastDrawcallIndex < 0)
		{
			return false;
		}
		//first step, check last drawcall, because 3d UI can only batch into last drawcall
		{
			auto LastDrawcall = InUIDrawcallList[LastDrawcallIndex];
			if (LastDrawcall->CanConsumeUIBatchGeometryRenderable(InUIItem->GetGeometry(), InUIItemVerticesCount))
			{
				OutDrawcallIndexToFitin = LastDrawcallIndex;
				return true;
			}
		}
		//3d UI is already processed in prev step, so any 3d UI is not able to batch
		if (!InIs2DUI)
		{
			return false;
		}
		for (int i = LastDrawcallIndex; i >= FitInDrawcallMinIndex; i--)//from tail to head
		{
			auto DrawcallItem = InUIDrawcallList[i];
			if (!DrawcallItem->bIs2DSpace)//drawcall is 3d, can't batch
			{
				return false;
			}

			if (!DrawcallItem->CanConsumeUIBatchGeometryRenderable(InUIItem->GetGeometry(), InUIItemVerticesCount))//can't fit in this drawcall, should check overlap
			{
				if (OverlapWithOtherDrawcall(InUIItemToCanvasTf, DrawcallItem))//overlap with other drawcall, can't batch
				{
					return false;
				}
				continue;//not overlap with other drawcall, keep searching
			}
			OutDrawcallIndexToFitin = i;
			return true;//can fit in drawcall
		}
		return false;
	};
	auto PushSingleDrawcall = [&](UUIItem* InUIItem, bool InSearchInCacheList, UIGeometry* InItemGeo, bool InIs2DSpace, EUIDrawcallType InDrawcallType, const FLGUICacheTransformContainer& InItemToCanvasTf) {
		TSharedPtr<UUIDrawcall> DrawcallItem = nullptr;
		//if this UIItem exist in InCacheUIDrawcallList, then grab the entire drawcall item (may include other UIItem in renderObjectList). No need to worry other UIItem, because they could be cleared in further operation, or exist in the same drawcall
		int32 FoundDrawcallIndex = INDEX_NONE;
		if (InSearchInCacheList)
		{
			FoundDrawcallIndex = InCacheUIDrawcallList.IndexOfByPredicate([=](const TSharedPtr<UUIDrawcall>& DrawcallItem) {
				if (DrawcallItem->type == InDrawcallType)
				{
					switch (InDrawcallType)
					{
					case EUIDrawcallType::BatchGeometry:
					{
						if (DrawcallItem->renderObjectList.Contains(InUIItem))
						{
							return true;
						}
					}
					break;
					case EUIDrawcallType::PostProcess:
					{
						if (DrawcallItem->postProcessRenderableObject == InUIItem)
						{
							return true;
						}
					}
					break;
					case EUIDrawcallType::DirectMesh:
					{
						if (DrawcallItem->directMeshRenderableObject == InUIItem)
						{
							return true;
						}
					}
					break;
					}
				}
				return false;
				});
		}
		if (FoundDrawcallIndex != INDEX_NONE)
		{
			DrawcallItem = InCacheUIDrawcallList[FoundDrawcallIndex];
			InCacheUIDrawcallList.RemoveAt(FoundDrawcallIndex);//cannot use "RemoveAtSwap" here, because we need the right order to tell if we should sort render order, see "bNeedToSortRenderPriority"

			switch (InDrawcallType)
			{
			case EUIDrawcallType::BatchGeometry:
			{
				DrawcallItem->texture = InItemGeo->texture;
				DrawcallItem->material = InItemGeo->material.Get();

				if (DrawcallItem->renderObjectListTreeRootNode != nullptr)
				{
					delete DrawcallItem->renderObjectListTreeRootNode;
				}
				DrawcallItem->renderObjectListTreeRootNode = new UIQuadTree::Node(CanvasRect);
				DrawcallItem->renderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(InItemToCanvasTf.BoundsMin2D, InItemToCanvasTf.BoundsMax2D));
				DrawcallItem->verticesCount = InItemGeo->vertices.Num();
				DrawcallItem->indicesCount = InItemGeo->triangles.Num();
			}
			break;
			case EUIDrawcallType::PostProcess:
			{
				DrawcallItem->postProcessRenderableObject = (UUIPostProcessRenderable*)InUIItem;
			}
			break;
			case EUIDrawcallType::DirectMesh:
			{
				DrawcallItem->directMeshRenderableObject = (UUIDirectMeshRenderable*)InUIItem;
			}
			break;
			}
		}
		else
		{

			switch (InDrawcallType)
			{
			default:
			case EUIDrawcallType::BatchGeometry:
			{
				DrawcallItem = TSharedPtr<UUIDrawcall>(new UUIDrawcall(CanvasRect));
				DrawcallItem->needToUpdateVertex = true;
				DrawcallItem->texture = InItemGeo->texture;
				DrawcallItem->material = InItemGeo->material.Get();
				DrawcallItem->renderObjectList.Add((UUIBatchGeometryRenderable*)InUIItem);
				DrawcallItem->verticesCount = InItemGeo->vertices.Num();
				DrawcallItem->indicesCount = InItemGeo->triangles.Num();
				DrawcallItem->renderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(InItemToCanvasTf.BoundsMin2D, InItemToCanvasTf.BoundsMax2D));
			}
			break;
			case EUIDrawcallType::PostProcess:
			{
				DrawcallItem = TSharedPtr<UUIDrawcall>(new UUIDrawcall(InDrawcallType));
				DrawcallItem->postProcessRenderableObject = (UUIPostProcessRenderable*)InUIItem;
			}
			break;
			case EUIDrawcallType::DirectMesh:
			{
				DrawcallItem = TSharedPtr<UUIDrawcall>(new UUIDrawcall(InDrawcallType));
				DrawcallItem->directMeshRenderableObject = (UUIDirectMeshRenderable*)InUIItem;
			}
			break;
			}
		}
		DrawcallItem->bIs2DSpace = InIs2DSpace;

		if (InDrawcallType == EUIDrawcallType::BatchGeometry
			|| InDrawcallType == EUIDrawcallType::PostProcess
			|| InDrawcallType == EUIDrawcallType::DirectMesh)
		{
			((UUIBaseRenderable*)InUIItem)->drawcall = DrawcallItem;
		}
		InUIDrawcallList.Add(DrawcallItem);

		if (FoundDrawcallIndex != 0)//if not find drawcall or found drawcall not at head of array, means drawcall list's order is changed compare to cache list, then we need to sort render order
		{
			OutNeedToSortRenderPriority = true;
		}
		//OutNeedToSortRenderPriority = true;//@todo: this line could make it sort every time, which is not good performance
	};
	auto ClearObjectFromDrawcall = [&](TSharedPtr<UUIDrawcall> InDrawcallItem, UUIBatchGeometryRenderable* InUIBatchGeometryRenderable, const FLGUICacheTransformContainer& InItemToCanvasTf) {
		if (InDrawcallItem->drawcallMesh.IsValid())
		{
			if (InDrawcallItem->drawcallMeshSection.IsValid())
			{
				InDrawcallItem->drawcallMesh->DeleteMeshSection(InDrawcallItem->drawcallMeshSection.Pin());
				InDrawcallItem->drawcallMeshSection.Reset();
			}
		}
		InDrawcallItem->needToUpdateVertex = true;
		InDrawcallItem->materialNeedToReassign = true;
		int index = InDrawcallItem->renderObjectList.IndexOfByKey(InUIBatchGeometryRenderable);
		InDrawcallItem->renderObjectList.RemoveAt(index);
		InUIBatchGeometryRenderable->drawcall = nullptr;
	};
	auto RemoveDrawcallWhenBreakMeshSequence = [&](int InStartIndex) {
		for (int DrawcallIndex = InStartIndex; DrawcallIndex < InUIDrawcallList.Num(); DrawcallIndex++)
		{
			auto DrawcallItem = InUIDrawcallList[DrawcallIndex];
			if (DrawcallItem->drawcallMesh.IsValid())
			{
				for (int CacheDrawcallIndex = 0; CacheDrawcallIndex < InCacheUIDrawcallList.Num(); CacheDrawcallIndex++)
				{
					auto CacheDrawcallItem = InCacheUIDrawcallList[CacheDrawcallIndex];
					if (DrawcallItem->drawcallMesh == CacheDrawcallItem->drawcallMesh)
					{
						if (CacheDrawcallItem->drawcallMeshSection.IsValid())
						{
							DrawcallItem->drawcallMesh->DeleteMeshSection(CacheDrawcallItem->drawcallMeshSection.Pin());
						}
						CacheDrawcallItem->drawcallMesh = nullptr;
						CacheDrawcallItem->drawcallMeshSection = nullptr;
					}
				}
			}
		}
	};

	int MeshStartDrawcallIndex = 0;
	//for sorted ui items, iterate from head to tail, compare drawcall from tail to head
	for (int i = 0; i < UIRenderableList.Num(); i++)
	{
		auto& Item = UIRenderableList[i];
		//check(Item->GetIsUIActiveInHierarchy());
		if (!Item->GetIsUIActiveInHierarchy())continue;
		
		if (Item->IsCanvasUIItem() && Item->GetRenderCanvas() != this)//is child canvas
		{
			auto ChildCanvas = Item->GetRenderCanvas();
			if (!ChildCanvas->GetOverrideSorting())
			{
				RemoveDrawcallWhenBreakMeshSequence(MeshStartDrawcallIndex);

				if (InCacheUIDrawcallList.Num() > 0)
				{
					int FoundIndex = InCacheUIDrawcallList.IndexOfByPredicate([ChildCanvas](const TSharedPtr<UUIDrawcall>& CacheDrawcallItem) {
						return CacheDrawcallItem->type == EUIDrawcallType::ChildCanvas && CacheDrawcallItem->childCanvas == ChildCanvas;
						});
					if (FoundIndex != INDEX_NONE)
					{
						InUIDrawcallList.Add(InCacheUIDrawcallList[FoundIndex]);
						InCacheUIDrawcallList.RemoveAt(FoundIndex);
					}
					else
					{
						auto ChildCanvasDrawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall(EUIDrawcallType::ChildCanvas));
						ChildCanvasDrawcall->childCanvas = ChildCanvas;
						InUIDrawcallList.Add(ChildCanvasDrawcall);
					}
					if (FoundIndex != 0)//if not find drawcall or found drawcall not at head of array, means drawcall list's order is changed compare to cache list, then we need to sort render order
					{
						OutNeedToSortRenderPriority = true;
					}
				}
				else
				{
					auto ChildCanvasDrawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall(EUIDrawcallType::ChildCanvas));
					ChildCanvasDrawcall->childCanvas = ChildCanvas;
					InUIDrawcallList.Add(ChildCanvasDrawcall);
					OutNeedToSortRenderPriority = true;
				}

				FitInDrawcallMinIndex = InUIDrawcallList.Num();
				MeshStartDrawcallIndex = InUIDrawcallList.Num();
			}
		}
		else
		{
			auto UIRenderableItem = (UUIBaseRenderable*)(Item);
			FLGUICacheTransformContainer UIItemToCanvasTf;
			this->GetCacheUIItemToCanvasTransform(UIRenderableItem, UIItemToCanvasTf);
			bool is2DUIItem = Is2DUITransform(UIItemToCanvasTf.Transform);
			switch (UIRenderableItem->GetUIRenderableType())
			{
			default:
			case EUIRenderableType::UIBatchGeometryRenderable:
			{
				auto UIBatchGeometryRenderableItem = (UUIBatchGeometryRenderable*)UIRenderableItem;
				auto ItemGeo = UIBatchGeometryRenderableItem->GetGeometry();
				check(ItemGeo);
				if (ItemGeo->vertices.Num() == 0)continue;
				if (ItemGeo->vertices.Num() > LGUI_MAX_VERTEX_COUNT)continue;

				int DrawcallIndexToFitin;
				if (CanFitInDrawcall(UIBatchGeometryRenderableItem, is2DUIItem, ItemGeo->vertices.Num(), UIItemToCanvasTf, DrawcallIndexToFitin))
				{
					auto DrawcallItem = InUIDrawcallList[DrawcallIndexToFitin];
					DrawcallItem->bIs2DSpace = DrawcallItem->bIs2DSpace && is2DUIItem;
					if (UIBatchGeometryRenderableItem->drawcall == DrawcallItem)//already exist in this drawcall (added previoursly)
					{
						//need to update tree
						DrawcallItem->renderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(UIItemToCanvasTf.BoundsMin2D, UIItemToCanvasTf.BoundsMax2D));
						DrawcallItem->verticesCount += ItemGeo->vertices.Num();
						DrawcallItem->indicesCount += ItemGeo->triangles.Num();
					}
					else//not exist in this drawcall
					{
						auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
						if (OldDrawcall.IsValid())//maybe exist in other drawcall, should remove from that drawcall
						{
							ClearObjectFromDrawcall(OldDrawcall, UIBatchGeometryRenderableItem, UIItemToCanvasTf);
						}
						//add to this drawcall
						DrawcallItem->renderObjectList.Add(UIBatchGeometryRenderableItem);
						DrawcallItem->renderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(UIItemToCanvasTf.BoundsMin2D, UIItemToCanvasTf.BoundsMax2D));
						DrawcallItem->verticesCount += ItemGeo->vertices.Num();
						DrawcallItem->indicesCount += ItemGeo->triangles.Num();
						DrawcallItem->needToUpdateVertex = true;
						UIBatchGeometryRenderableItem->drawcall = DrawcallItem;
						//copy update state from old to new
						if (OldDrawcall.IsValid())
						{
							OldDrawcall->CopyUpdateState(DrawcallItem.Get());
						}
					}
					check(DrawcallItem->verticesCount < LGUI_MAX_VERTEX_COUNT);
				}
				else//cannot fit in any other drawcall
				{
					auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
					if (OldDrawcall.IsValid())//maybe exist in other drawcall, should remove from that drawcall
					{
						if (InUIDrawcallList.Contains(OldDrawcall))//if this drawcall already exist (added previoursly), then remove the object from the drawcall.
						{
							ClearObjectFromDrawcall(OldDrawcall, UIBatchGeometryRenderableItem, UIItemToCanvasTf);
						}
					}
					//make a new drawacll
					PushSingleDrawcall(UIBatchGeometryRenderableItem, true, ItemGeo, is2DUIItem, EUIDrawcallType::BatchGeometry, UIItemToCanvasTf);
					check(UIBatchGeometryRenderableItem->drawcall->verticesCount < LGUI_MAX_VERTEX_COUNT);
				}
			}
			break;
			case EUIRenderableType::UIPostProcessRenderable:
			{
				auto UIPostProcessRenderableItem = (UUIPostProcessRenderable*)UIRenderableItem;
				auto ItemGeo = UIPostProcessRenderableItem->GetGeometry();
				check(ItemGeo);
				if (ItemGeo->vertices.Num() == 0)continue;
				//every postprocess is a drawcall
				PushSingleDrawcall(UIRenderableItem, true, ItemGeo, is2DUIItem, EUIDrawcallType::PostProcess, UIItemToCanvasTf);
				//no need to copy drawcall's update data for UIPostProcessRenderable, because UIPostProcessRenderable's drawcall should be the same as previours one

				RemoveDrawcallWhenBreakMeshSequence(MeshStartDrawcallIndex);
				FitInDrawcallMinIndex = InUIDrawcallList.Num();
				MeshStartDrawcallIndex = InUIDrawcallList.Num();
			}
			break;
			case EUIRenderableType::UIDirectMeshRenderable:
			{
				auto UIDirectMeshRenderableItem = (UUIDirectMeshRenderable*)UIRenderableItem;
				//every direct mesh is a drawcall
				PushSingleDrawcall(UIRenderableItem, true, nullptr, is2DUIItem, EUIDrawcallType::DirectMesh, UIItemToCanvasTf);
				//no need to copy drawcall's update data for UIDirectMeshRenderable, because UIDirectMeshRenderable's drawcall should be the same as previours one
			}
			break;
			}
		}
	}
}

void ULGUICanvas::SetOverrideViewLoation(bool InOverride, FVector InValue)
{
	bOverrideViewLocation = InOverride;
	OverrideViewLocation = InValue;
}
void ULGUICanvas::SetOverrideViewRotation(bool InOverride, FRotator InValue)
{
	bOverrideViewRotation = InOverride;
	OverrideViewRotation = InValue;
}
void ULGUICanvas::SetOverrideFovAngle(bool InOverride, float InValue)
{
	bOverrideFovAngle = InOverride;
	OverrideFovAngle = InValue;
}
void ULGUICanvas::SetOverrideProjectionMatrix(bool InOverride, FMatrix InValue)
{
	bOverrideProjectionMatrix = InOverride;
	OverrideProjectionMatrix = InValue;
}

void ULGUICanvas::MarkCanvasLayoutDirty()
{
	bIsViewProjectionMatrixDirty = true;
	switch (GetActualClipType())
	{
	default:
		break;
	case ELGUICanvasClipType::Rect:
	{
		bRectClipParameterChanged = true;
		bRectRangeCalculated = false;
	}
		break;
	case ELGUICanvasClipType::Texture:
		bTextureClipParameterChanged = true;
		break;
	}
}

void ULGUICanvas::SetDefaultMeshType(TSubclassOf<ULGUIMeshComponent> InValue)
{
	if (DefaultMeshType != InValue)
	{
		DefaultMeshType = InValue;

		for (int i = 0; i < UIDrawcallList.Num(); i++)
		{
			auto DrawcallItem = UIDrawcallList[i];
			DrawcallItem->needToUpdateVertex = true;
			DrawcallItem->drawcallMeshSection = nullptr;
			DrawcallItem->drawcallMesh = nullptr;
			DrawcallItem->materialChanged = true;//material is directly used by mesh
		}
		//clear mesh
		for (auto& Mesh : PooledUIMeshList)
		{
			Mesh->ClearAllMeshSection();
			Mesh->DestroyComponent();
		}
		PooledUIMeshList.Reset();
		for (auto& Mesh : UsingUIMeshList)
		{
			Mesh->DestroyComponent();
		}
		UsingUIMeshList.Reset();

		MarkCanvasUpdate(true, false, false);
	}
}

void ULGUICanvas::MarkFinishRenderFrameRecursive()
{
	//mark children canvas
	for (auto item : ChildrenCanvasArray)
	{
		if (item.IsValid() && item->GetIsUIActive())
		{
			item->MarkFinishRenderFrameRecursive();
		}
	}

	bShouldRebuildDrawcall = false;
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;
}

void ULGUICanvas::UpdateCanvasDrawcallRecursive()
{
	/**
	 * Why use bPrevUIItemIsActive?:
	 * If Canvas is rendering in frame 1, but when in frame 2 the Canvas is disabled(by disable UIItem), then the Canvas will not do drawcall calculation, and the prev existing drawcall mesh is still there and render,
	 * so we check bPrevUIItemIsActive, then we can still do drawcall calculation at this frame, and the prev existing drawcall will be removed.
	 */
	bool bNowUIItemIsActive = UIItem->GetIsUIActiveInHierarchy();
	if (bNowUIItemIsActive || bPrevUIItemIsActive)
	{
		bPrevUIItemIsActive = bNowUIItemIsActive;
		//update children canvas
		for (auto& item : ChildrenCanvasArray)
		{
			if (item.IsValid())
			{
				item->UpdateCanvasDrawcallRecursive();
			}
		}
	}

	//update drawcall
	if (bCanTickUpdate)
	{
		bCanTickUpdate = false;

		//reset transform map, because transform change
		CacheUIItemToCanvasTransformMap.Reset();

		//check if add to renderer
		if (bCurrentIsLGUIRendererOrUERenderer)
		{
			if (!bHasAddToLGUIWorldSpaceRenderer && GetActualRenderMode() == ELGUIRenderMode::WorldSpace_LGUI)
			{
				TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension = nullptr;
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())//editor world
				{
					if (GetWorld()->WorldType != EWorldType::EditorPreview)
					{
						ViewExtension = ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true);
					}
				}
				else
#endif
				{
					ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), true);
				}

				if (ViewExtension.IsValid())//all WorldSpace_LGUI canvas should add to LGUIRenderer
				{
					ViewExtension->AddWorldSpaceRenderCanvas(this);
					bHasAddToLGUIWorldSpaceRenderer = true;
				}
			}
		}

		UpdateGeometry_Implement();

		if (bShouldRebuildDrawcall)
		{
			//store prev created drawcall to cache list, so when we create drawcall, we can search in the cache list and use existing one
			for (auto Item : UIDrawcallList)
			{
				CacheUIDrawcallList.Add(Item);
			}
			UIDrawcallList.Reset();

			//rect size minimal at 100, so UIQuadTree can work properly (prevent too small rect)
			auto Width = FMath::Max(UIItem->GetWidth(), 100.0f);
			auto Height = FMath::Max(UIItem->GetHeight(), 100.0f);
			FVector2D LeftBottomPoint;
			LeftBottomPoint.X = Width * -UIItem->GetPivot().X;
			LeftBottomPoint.Y = Height * -UIItem->GetPivot().Y;
			FVector2D RightTopPoint;
			RightTopPoint.X = Width * (1.0f - UIItem->GetPivot().X);
			RightTopPoint.Y = Height * (1.0f - UIItem->GetPivot().Y);
			bool bOutNeedToSortRenderPriority = bNeedToSortRenderPriority;
			if (FMath::Abs(RightTopPoint.X - LeftBottomPoint.X) <= 0.01f
				|| FMath::Abs(RightTopPoint.Y - LeftBottomPoint.Y) <= 0.01f
				)
			{
				UE_LOG(LGUI, Error, TEXT(""));
			}
			BatchDrawcall_Implement(LeftBottomPoint, RightTopPoint, UIDrawcallList, CacheUIDrawcallList
				, bOutNeedToSortRenderPriority//cannot pass a uint32:1 here, so use a temp bool
			);
			bNeedToSortRenderPriority = bOutNeedToSortRenderPriority;

			auto IsUsingUIMesh = [&](TWeakObjectPtr<ULGUIMeshComponent> InMesh) {
				for (auto& Drawcall : UIDrawcallList)
				{
					if (Drawcall->drawcallMesh == InMesh)
					{
						return true;
					}
				}
				return false;
			};

			//for not used drawcalls, clear data
			for (int i = 0; i < CacheUIDrawcallList.Num(); i++)
			{
				auto DrawcallInCache = CacheUIDrawcallList[i];
				//check(DrawcallInCache->renderObjectList.Num() == 0);//why comment this?: need to wait until UUIBaseRenderable::OnRenderCanvasChanged.todo finish
				if (DrawcallInCache->drawcallMesh.IsValid())
				{
					if (DrawcallInCache->drawcallMeshSection.IsValid())
					{
						DrawcallInCache->drawcallMesh->DeleteMeshSection(DrawcallInCache->drawcallMeshSection.Pin());
						DrawcallInCache->drawcallMeshSection.Reset();
					}
					if (!IsUsingUIMesh(DrawcallInCache->drawcallMesh))
					{
						this->AddUIMeshToPool(DrawcallInCache->drawcallMesh);
					}
					DrawcallInCache->drawcallMesh.Reset();
				}
				if (DrawcallInCache->materialInstanceDynamic.IsValid())
				{
					this->AddUIMaterialToPool(DrawcallInCache->materialInstanceDynamic.Get());
					DrawcallInCache->materialInstanceDynamic.Reset();
				}
				if (DrawcallInCache->postProcessRenderableObject.IsValid())
				{
					if (DrawcallInCache->postProcessRenderableObject->IsRenderProxyValid())
					{
						DrawcallInCache->postProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
					}
				}
				if (DrawcallInCache->directMeshRenderableObject.IsValid())
				{
					DrawcallInCache->directMeshRenderableObject->ClearMeshData();
				}
			}
			CacheUIDrawcallList.Reset();
		}

		//update drawcall mesh
		UpdateDrawcallMesh_Implement();

		//update drawcall material
		UpdateDrawcallMaterial_Implement();
	}

	//sort render priority
	{
		if (bNeedToSortRenderPriority)
		{
			bNeedToSortRenderPriority = false;
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					switch (this->GetActualRenderMode())
					{
					default:
					case ELGUIRenderMode::ScreenSpaceOverlay:
					case ELGUIRenderMode::WorldSpace_LGUI:
						ULGUIEditorManagerObject::Instance->MarkSortLGUIRenderer();
						break;
					case ELGUIRenderMode::WorldSpace:
						ULGUIEditorManagerObject::Instance->MarkSortWorldSpaceCanvas();
						break;
					case ELGUIRenderMode::RenderTarget:
						ULGUIEditorManagerObject::Instance->MarkSortRenderTargetSpaceCanvas();
						break;
					}
				}
			}
			else
#endif
			{
				if (auto Instance = ALGUIManagerActor::GetLGUIManagerActorInstance(this))
				{
					switch (this->GetActualRenderMode())
					{
					default:
					case ELGUIRenderMode::ScreenSpaceOverlay:
					case ELGUIRenderMode::WorldSpace_LGUI:
						Instance->MarkSortLGUIRenderer();
						break;
					case ELGUIRenderMode::WorldSpace:
						Instance->MarkSortWorldSpaceCanvas();
						break;
					case ELGUIRenderMode::RenderTarget:
						Instance->MarkSortRenderTargetSpaceCanvas();
						break;
					}
				}
			}
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawcallMesh"), STAT_UpdateDrawcallMesh, STATGROUP_LGUI);
void ULGUICanvas::UpdateDrawcallMesh_Implement()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcallMesh);

	auto FindNextValidMeshInDrawcallList = [&](int32 InStartIndex)
	{
		for (auto i = InStartIndex; i < UIDrawcallList.Num(); i++)
		{
			auto DrawcallItem = UIDrawcallList[i];
			switch (DrawcallItem->type)
			{
			case EUIDrawcallType::DirectMesh:
			case EUIDrawcallType::BatchGeometry:
			{
				if (DrawcallItem->drawcallMesh.IsValid())
				{
					return DrawcallItem->drawcallMesh.Get();
				}
			}
			break;
			case EUIDrawcallType::PostProcess:
			case EUIDrawcallType::ChildCanvas:
			{
				return (ULGUIMeshComponent*)nullptr;
			}
			break;
			}
		}
		return (ULGUIMeshComponent*)nullptr;
	};

	ULGUIMeshComponent* CurrentUIMesh = nullptr;
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		//check drawcall mesh first
		switch (DrawcallItem->type)
		{
		case EUIDrawcallType::DirectMesh:
		case EUIDrawcallType::BatchGeometry:
		{
			if (!DrawcallItem->drawcallMesh.IsValid())
			{
				if (CurrentUIMesh == nullptr)
				{
					//if drawcall mesh is not valid, we need to search in next drawcalls and find mesh drawcall object (not post process drawcall)
					if (auto foundMesh = FindNextValidMeshInDrawcallList(i + 1))
					{
						CurrentUIMesh = foundMesh;
					}
					else//not find valid mesh, then get from pool
					{
						CurrentUIMesh = this->GetUIMeshFromPool().Get();
					}
				}
				DrawcallItem->drawcallMesh = CurrentUIMesh;
				//create new mesh, need to sort it
				bNeedToSortRenderPriority = true;
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		case EUIDrawcallType::ChildCanvas:
		{
			CurrentUIMesh = nullptr;
		}
		break;
		}


		switch (DrawcallItem->type)
		{
		case EUIDrawcallType::DirectMesh:
		{
			auto MeshSection = DrawcallItem->drawcallMeshSection;
			if (!MeshSection.IsValid())
			{
				auto UIMesh = DrawcallItem->drawcallMesh;
				MeshSection = UIMesh->GetMeshSection();

				DrawcallItem->drawcallMeshSection = MeshSection;
				DrawcallItem->directMeshRenderableObject->SetMeshData(UIMesh, MeshSection);
				UIMesh->CreateMeshSectionData(MeshSection.Pin());
				//create new mesh section, need to sort it
				bNeedToSortRenderPriority = true;
			}
			CurrentUIMesh = DrawcallItem->drawcallMesh.Get();
		}
		break;
		case EUIDrawcallType::BatchGeometry:
		{
			auto UIMesh = DrawcallItem->drawcallMesh;
			auto MeshSection = DrawcallItem->drawcallMeshSection;
			if (DrawcallItem->shouldSortRenderObjectList)
			{
				DrawcallItem->shouldSortRenderObjectList = false;
				DrawcallItem->renderObjectList.Sort([](const TWeakObjectPtr<UUIBatchGeometryRenderable>& A, const TWeakObjectPtr<UUIBatchGeometryRenderable>& B) {
					return A->GetFlattenHierarchyIndex() < B->GetFlattenHierarchyIndex();
					});
				DrawcallItem->needToUpdateVertex = true;
			}
			if (DrawcallItem->needToUpdateVertex)
			{
				if (!MeshSection.IsValid())
				{
					MeshSection = UIMesh->GetMeshSection();
					DrawcallItem->drawcallMeshSection = MeshSection;
					//create new mesh section, need to sort it
					bNeedToSortRenderPriority = true;
				}
				auto MeshSectionPtr = MeshSection.Pin();
				MeshSectionPtr->vertices.Reset();
				MeshSectionPtr->triangles.Reset();
				DrawcallItem->GetCombined(MeshSectionPtr->vertices, MeshSectionPtr->triangles);
				if (MeshSectionPtr->prevVertexCount != MeshSectionPtr->vertices.Num() || MeshSectionPtr->prevIndexCount != MeshSectionPtr->triangles.Num())
				{
					MeshSectionPtr->prevVertexCount = MeshSectionPtr->vertices.Num();
					MeshSectionPtr->prevIndexCount = MeshSectionPtr->triangles.Num();
					UIMesh->CreateMeshSectionData(MeshSectionPtr);
				}
				else
				{
					UIMesh->UpdateMeshSectionData(MeshSectionPtr, true, GetActualAdditionalShaderChannelFlags());
				}
				DrawcallItem->needToUpdateVertex = false;
				DrawcallItem->vertexPositionChanged = false;
			}
			CurrentUIMesh = DrawcallItem->drawcallMesh.Get();
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())
			{
				//editor world, post process not work with WorldSpace-UERenderer and ScreenSpace, ignore it
				if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace || this->GetActualRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
				{
					continue;
				}
				else//lgui renderer, create a PostProcessRenderable object to handle it. then the next UI objects should render by new mesh
				{
					CurrentUIMesh = nullptr;//set to null so a new mesh will be created for next drawcall
				}
			}
			else
#endif
			{
				//game world, only LGUI renderer can render post process
				if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace)
				{
					continue;
				}
				else//lgui renderer, create a PostProcessRenderable object to handle it. then the next UI objects should render by new mesh
				{
					CurrentUIMesh = nullptr;//set to null so a new mesh will be created for next drawcall
				}
			}

			if (!DrawcallItem->postProcessRenderableObject->IsRenderProxyValid())
			{
				DrawcallItem->needToAddPostProcessRenderProxyToRender = true;
			}
			if (DrawcallItem->needToAddPostProcessRenderProxyToRender)
			{
				DrawcallItem->needToAddPostProcessRenderProxyToRender = false;
				auto uiPostProcessPrimitive = DrawcallItem->postProcessRenderableObject->GetRenderProxy();
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())
				{
					if (GetWorld()->WorldType == EWorldType::EditorPreview)
					{
						//no post process in EditorPreview
					}
					else
					{
						if (bCurrentIsLGUIRendererOrUERenderer)
						{
							if (GetWorld()->WorldType != EWorldType::EditorPreview)//editor preview not visible
							{
								switch (GetActualRenderMode())
								{
								case ELGUIRenderMode::RenderTarget:
								{
									uiPostProcessPrimitive->AddToLGUIScreenSpaceRenderer(this->GetRootCanvas()->GetRenderTargetViewExtension());
								}
								break;
								case ELGUIRenderMode::ScreenSpaceOverlay:
								{
									uiPostProcessPrimitive->AddToLGUIScreenSpaceRenderer(ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true));
								}
								break;
								case ELGUIRenderMode::WorldSpace_LGUI:
								{
									uiPostProcessPrimitive->AddToLGUIWorldSpaceRenderer(this, this->GetActualSortOrder(), ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true));
								}
								break;
								}
								uiPostProcessPrimitive->SetVisibility(true);
							}
						}
					}
				}
				else
#endif
				{
					if (bCurrentIsLGUIRendererOrUERenderer)
					{
						switch (GetActualRenderMode())
						{
						case ELGUIRenderMode::RenderTarget:
						{
							uiPostProcessPrimitive->AddToLGUIScreenSpaceRenderer(this->GetRootCanvas()->GetRenderTargetViewExtension());
						}
						break;
						case ELGUIRenderMode::ScreenSpaceOverlay:
						{
							uiPostProcessPrimitive->AddToLGUIScreenSpaceRenderer(ALGUIManagerActor::GetViewExtension(GetWorld(), true));
						}
						break;
						case ELGUIRenderMode::WorldSpace_LGUI:
						{
							uiPostProcessPrimitive->AddToLGUIWorldSpaceRenderer(this, this->GetActualSortOrder(), ALGUIManagerActor::GetViewExtension(GetWorld(), true));
						}
						break;
						}
						uiPostProcessPrimitive->SetVisibility(true);
					}
				}
			}
		}
		break;
		}
	}
}

TWeakObjectPtr<ULGUIMeshComponent> ULGUICanvas::GetUIMeshFromPool()
{
	if (PooledUIMeshList.Num() > 0)
	{
		auto UIMesh = PooledUIMeshList[0];
		PooledUIMeshList.RemoveAt(0);
		check(UIMesh.IsValid());
		UsingUIMeshList.Add(UIMesh);
		return UIMesh;
	}
	else
	{
		auto MeshType = DefaultMeshType.Get();
		if (MeshType == nullptr)MeshType = ULGUIMeshComponent::StaticClass();
		auto UIMesh = NewObject<ULGUIMeshComponent>(this->GetOwner(), MeshType, NAME_None, RF_Transient);
		UIMesh->RegisterComponent();
		UIMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
		UIMesh->SetRelativeTransform(FTransform::Identity);

#if WITH_EDITOR
		if (!GetWorld()->IsGameWorld())
		{
			if (GetWorld()->WorldType == EWorldType::EditorPreview)
			{
				UIMesh->SetSupportUERenderer(true);
			}
			else
			{
				if (bCurrentIsLGUIRendererOrUERenderer)
				{
					switch (GetActualRenderMode())
					{
					case ELGUIRenderMode::RenderTarget:
					{
						UIMesh->SetSupportLGUIRenderer(GetWorld()->WorldType != EWorldType::EditorPreview, this->GetRootCanvas()->GetRenderTargetViewExtension(), RootCanvas.Get(), false);
						UIMesh->SetSupportUERenderer(true);
					}
					break;
					case ELGUIRenderMode::ScreenSpaceOverlay:
					{
						UIMesh->SetSupportLGUIRenderer(true, ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true), RootCanvas.Get(), false);
						UIMesh->SetSupportUERenderer(true);
					}
					break;
					case ELGUIRenderMode::WorldSpace_LGUI:
					{
						UIMesh->SetSupportLGUIRenderer(true, ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true), this, true);
						UIMesh->SetSupportUERenderer(false);
					}
					break;
					}
				}
				else
				{
					UIMesh->SetSupportUERenderer(true);
				}
			}
		}
		else
#endif
		{
			if (bCurrentIsLGUIRendererOrUERenderer)
			{
				switch (GetActualRenderMode())
				{
				case ELGUIRenderMode::RenderTarget:
				{
					UIMesh->SetSupportLGUIRenderer(true, this->GetRootCanvas()->GetRenderTargetViewExtension(), RootCanvas.Get(), false);
				}
				break;
				case ELGUIRenderMode::ScreenSpaceOverlay:
				{
					UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), RootCanvas.Get(), false);
				}
				break;
				case ELGUIRenderMode::WorldSpace_LGUI:
				{
					UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), this, true);
				}
				break;
				}
				UIMesh->SetSupportUERenderer(false);
			}
			else
			{
				UIMesh->SetSupportLGUIRenderer(false, nullptr, nullptr, false);
				UIMesh->SetSupportUERenderer(true);
			}
		}
		UsingUIMeshList.Add(UIMesh);
		return UIMesh;
	}
}

void ULGUICanvas::AddUIMeshToPool(TWeakObjectPtr<ULGUIMeshComponent> InUIMesh)
{
	if (!InUIMesh.IsValid())return;
	if (!PooledUIMeshList.Contains(InUIMesh))
	{
		InUIMesh->ClearAllMeshSection();
		PooledUIMeshList.Add(InUIMesh);
		UsingUIMeshList.Remove(InUIMesh);
	}
}

void ULGUICanvas::SortDrawcall(int32& InOutRenderPriority, TSet<ULGUICanvas*>& InOutProcessedCanvasArray)
{
	InOutProcessedCanvasArray.Add(this);

	ULGUIMeshComponent* prevUIMesh = nullptr;
	int drawcallIndex = 0;
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		switch (DrawcallItem->type)
		{
		case EUIDrawcallType::BatchGeometry:
		case EUIDrawcallType::DirectMesh:
		{
			if (DrawcallItem->drawcallMesh != prevUIMesh)
			{
				if (prevUIMesh != nullptr)//when get difference mesh, we sort last mesh, because we need to wait until mesh's all section have set render proirity in "SetMeshSectionRenderPriority"
				{
					prevUIMesh->SortMeshSectionRenderPriority();
					drawcallIndex = 0;
				}
				if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace)
				{
					DrawcallItem->drawcallMesh->SetUITranslucentSortPriority(this->GetActualSortOrder());//WorldSpace UERenderer commonly use single mesh(with multi section) to render UI, it directly use SortOrder as TranslucentSortPriority
				}
				else
				{
					DrawcallItem->drawcallMesh->SetUITranslucentSortPriority(InOutRenderPriority++);//LGUIRenderer use multiple mesh to render UI, it need to sort mesh
				}
				prevUIMesh = DrawcallItem->drawcallMesh.Get();
			}
			DrawcallItem->drawcallMesh->SetMeshSectionRenderPriority(DrawcallItem->drawcallMeshSection.Pin(), drawcallIndex++);
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			DrawcallItem->postProcessRenderableObject->GetRenderProxy()->SetUITranslucentSortPriority(InOutRenderPriority++);
		}
		break;
		case EUIDrawcallType::ChildCanvas:
		{
			if (ensure(DrawcallItem->childCanvas.IsValid()))
			{
				DrawcallItem->childCanvas->SortDrawcall(InOutRenderPriority, InOutProcessedCanvasArray);
			}
		}
		break;
		}
	}
	if (prevUIMesh != nullptr)//since "SortMeshSectionRenderPriority" action is called when get different mesh, so the last one won't call, so do it here
	{
		prevUIMesh->SortMeshSectionRenderPriority();
	}
}

FName ULGUICanvas::LGUI_MainTextureMaterialParameterName = FName(TEXT("MainTexture"));
FName ULGUICanvas::LGUI_RectClipOffsetAndSize_MaterialParameterName = FName(TEXT("RectClipOffsetAndSize"));
FName ULGUICanvas::LGUI_RectClipFeather_MaterialParameterName = FName(TEXT("RectClipFeather"));
FName ULGUICanvas::LGUI_TextureClip_MaterialParameterName = FName(TEXT("ClipTexture"));
FName ULGUICanvas::LGUI_TextureClipOffsetAndSize_MaterialParameterName = FName(TEXT("TextureClipOffsetAndSize"));

bool ULGUICanvas::IsMaterialContainsLGUIParameter(UMaterialInterface* InMaterial)
{
	static TArray<FMaterialParameterInfo> parameterInfos;
	static TArray<FGuid> parameterIds;
	auto tempClipType = this->GetActualClipType();
	InMaterial->GetAllTextureParameterInfo(parameterInfos, parameterIds);
	int mainTextureParamIndex = parameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& item)
		{
			return item.Name == LGUI_MainTextureMaterialParameterName;
		});
	bool containsLGUIParam = mainTextureParamIndex != INDEX_NONE;
	if (!containsLGUIParam)
	{
		switch (tempClipType)
		{
		case ELGUICanvasClipType::Rect:
		{
			InMaterial->GetAllVectorParameterInfo(parameterInfos, parameterIds);
			int foundIndex = parameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& item)
				{
					return item.Name == LGUI_RectClipFeather_MaterialParameterName || item.Name == LGUI_RectClipOffsetAndSize_MaterialParameterName;
				});
			containsLGUIParam = foundIndex != INDEX_NONE;
		}
		break;
		case ELGUICanvasClipType::Texture:
		{
			int foundIndex1 = parameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& item)
				{
					return item.Name == LGUI_TextureClip_MaterialParameterName;
				});
			if (foundIndex1 == INDEX_NONE)
			{
				InMaterial->GetAllVectorParameterInfo(parameterInfos, parameterIds);
				int foundIndex2 = parameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& item)
					{
						return item.Name == LGUI_TextureClipOffsetAndSize_MaterialParameterName;
					});
				containsLGUIParam = foundIndex2 != INDEX_NONE;
			}
			else
			{
				containsLGUIParam = true;
			}
		}
		break;
		}
	}
	return containsLGUIParam;
}
void ULGUICanvas::UpdateDrawcallMaterial_Implement()
{
	bool bNeedToSetClipParameter = false;
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		auto TempClipType = this->GetActualClipType();
		if (DrawcallItem->type == EUIDrawcallType::BatchGeometry)
		{
			auto UIMat = DrawcallItem->materialInstanceDynamic;
			if (!UIMat.IsValid() || DrawcallItem->materialChanged || this->bClipTypeChanged)
			{
				if (DrawcallItem->material.IsValid())//custom material
				{
					auto SrcMaterial = DrawcallItem->material.Get();
					auto bContainsLGUIParam = IsMaterialContainsLGUIParameter(SrcMaterial);
					if (SrcMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
					{
						if (bContainsLGUIParam)
						{
							UIMat = (UMaterialInstanceDynamic*)SrcMaterial;
						}
						DrawcallItem->drawcallMesh->SetMeshSectionMaterial(DrawcallItem->drawcallMeshSection.Pin(), SrcMaterial);
					}
					else
					{
						if (bContainsLGUIParam)
						{
							UIMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
							UIMat->SetFlags(RF_Transient);
							DrawcallItem->drawcallMesh->SetMeshSectionMaterial(DrawcallItem->drawcallMeshSection.Pin(), UIMat.Get());
						}
						else
						{
							DrawcallItem->drawcallMesh->SetMeshSectionMaterial(DrawcallItem->drawcallMeshSection.Pin(), SrcMaterial);
						}
					}
				}
				else
				{
					UIMat = this->GetUIMaterialFromPool(TempClipType);
					DrawcallItem->drawcallMesh->SetMeshSectionMaterial(DrawcallItem->drawcallMeshSection.Pin(), UIMat.Get());
				}
				DrawcallItem->materialInstanceDynamic = UIMat;
				DrawcallItem->materialChanged = false;
				if (UIMat.IsValid())
				{
					UIMat->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, DrawcallItem->texture.Get());
				}
				DrawcallItem->textureChanged = false;
				DrawcallItem->materialNeedToReassign = false;
				bNeedToSetClipParameter = true;
			}
			if (DrawcallItem->textureChanged)
			{
				DrawcallItem->textureChanged = false;
				UIMat->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, DrawcallItem->texture.Get());
			}
			if (DrawcallItem->materialNeedToReassign)
			{
				DrawcallItem->materialNeedToReassign = false;
				DrawcallItem->drawcallMesh->SetMeshSectionMaterial(DrawcallItem->drawcallMeshSection.Pin(), UIMat.Get());
			}
		}
		else if (DrawcallItem->type == EUIDrawcallType::PostProcess)
		{
			if (this->bClipTypeChanged
				|| DrawcallItem->materialChanged//maybe it is newly created, so check the materialChanged parameter
				)
			{
				if (DrawcallItem->postProcessRenderableObject.IsValid())
				{
					DrawcallItem->postProcessRenderableObject->SetClipType(TempClipType);
					DrawcallItem->materialChanged = false;
				}
				bNeedToSetClipParameter = true;
			}
		}
		else if (DrawcallItem->type == EUIDrawcallType::DirectMesh)
		{
			if (this->bClipTypeChanged
				|| DrawcallItem->materialChanged//maybe it is newly created, so check the materialChanged parameter
				)
			{
				if (DrawcallItem->directMeshRenderableObject.IsValid())
				{
					DrawcallItem->directMeshRenderableObject->SetClipType(TempClipType);
					DrawcallItem->materialChanged = false;
				}
				bNeedToSetClipParameter = true;
			}
		}

		//clip parameter
		switch (TempClipType)
		{
		case ELGUICanvasClipType::None:
			break;
		case ELGUICanvasClipType::Rect:
		{
			if (bNeedToSetClipParameter
				|| this->bRectClipParameterChanged)
			{
				auto TempRectClipOffsetAndSize = this->GetRectClipOffsetAndSize();
				auto TempRectClipFeather = this->GetRectClipFeather();

				switch (DrawcallItem->type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				{
					auto UIMat = DrawcallItem->materialInstanceDynamic;
					if (UIMat.IsValid())
					{
						UIMat->SetVectorParameterValue(LGUI_RectClipOffsetAndSize_MaterialParameterName, TempRectClipOffsetAndSize);
						UIMat->SetVectorParameterValue(LGUI_RectClipFeather_MaterialParameterName, TempRectClipFeather);
					}
				}
				break;
				case EUIDrawcallType::PostProcess:
				{
					if (DrawcallItem->postProcessRenderableObject.IsValid())
					{
						DrawcallItem->postProcessRenderableObject->SetRectClipParameter(TempRectClipOffsetAndSize, TempRectClipFeather);
					}
				}
				break;
				case EUIDrawcallType::DirectMesh:
				{
					if (DrawcallItem->directMeshRenderableObject.IsValid())
					{
						DrawcallItem->directMeshRenderableObject->SetRectClipParameter(TempRectClipOffsetAndSize, TempRectClipFeather);
					}
				}
				break;
				}
			}
		}
			break;
		case ELGUICanvasClipType::Texture:
		{
			if (bNeedToSetClipParameter
				|| this->bTextureClipParameterChanged)
			{
				auto TempTextureClipOffsetAndSize = this->GetTextureClipOffsetAndSize();
				auto TempClipTexture = this->GetClipTexture();

				switch (DrawcallItem->type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				{
					auto UIMat = DrawcallItem->materialInstanceDynamic;
					if (UIMat.IsValid())
					{
						UIMat->SetTextureParameterValue(LGUI_TextureClip_MaterialParameterName, TempClipTexture);
						UIMat->SetVectorParameterValue(LGUI_TextureClipOffsetAndSize_MaterialParameterName, TempTextureClipOffsetAndSize);
					}
				}
				break;
				case EUIDrawcallType::PostProcess:
				{
					if (DrawcallItem->postProcessRenderableObject.IsValid())
					{
						DrawcallItem->postProcessRenderableObject->SetTextureClipParameter(TempClipTexture, TempTextureClipOffsetAndSize);
					}
				}
				break;
				case EUIDrawcallType::DirectMesh:
				{
					if (DrawcallItem->directMeshRenderableObject.IsValid())
					{
						DrawcallItem->directMeshRenderableObject->SetTextureClipParameter(TempClipTexture, TempTextureClipOffsetAndSize);
					}
				}
				break;
				}
			}
		}
			break;
		}
	}
}

UMaterialInstanceDynamic* ULGUICanvas::GetUIMaterialFromPool(ELGUICanvasClipType InClipType)
{
	if (PooledUIMaterialList.Num() == 0)
	{
		for (int i = 0; i < LGUI_DEFAULT_MATERIAL_COUNT; i++)
		{
			PooledUIMaterialList.Add({});
		}
	}
	auto& MatList = PooledUIMaterialList[(int)InClipType].MaterialList;
	if (MatList.Num() == 0)
	{
		auto SrcMaterial = GetMaterials()[(int)InClipType];
		auto UIMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
		UIMat->SetFlags(RF_Transient);
		return UIMat;
	}
	else
	{
		auto UIMat = MatList[MatList.Num() - 1];
		MatList.RemoveAt(MatList.Num() - 1);
		return UIMat;
	}
}
void ULGUICanvas::AddUIMaterialToPool(UMaterialInstanceDynamic* UIMat)
{
	int CacheMatTypeIndex = -1;
	auto TempDefaultMaterials = GetMaterials();
	for (int i = 0; i < LGUI_DEFAULT_MATERIAL_COUNT; i++)
	{
		if (UIMat->Parent == TempDefaultMaterials[i])
		{
			CacheMatTypeIndex = i;
			break;
		}
	}
	if (CacheMatTypeIndex != -1)
	{
		if (PooledUIMaterialList.Num() == 0)//PooledUIMaterialList could be cleared when hierarchy change, but we still willing to pool the material, so check it again
		{
			for (int i = 0; i < LGUI_DEFAULT_MATERIAL_COUNT; i++)
			{
				PooledUIMaterialList.Add({});
			}
		}

		auto& MatList = PooledUIMaterialList[CacheMatTypeIndex].MaterialList;
		MatList.Add(UIMat);
	}
}

bool ULGUICanvas::CalculatePointVisibilityOnClip(FVector InWorldPoint)
{
	//if not use clip or use texture clip, then point is visible. texture clip not support this calculation yet.
	switch (GetActualClipType())
	{
	case ELGUICanvasClipType::None:
		return true;
		break;
	case ELGUICanvasClipType::Rect:
	{
		ConditionalCalculateRectRange();
		//transform to local space
		auto LocalPoint = this->UIItem->GetComponentTransform().InverseTransformPosition(InWorldPoint);
		//out of range
		if (LocalPoint.Y < clipRectMin.X) return false;
		if (LocalPoint.Z < clipRectMin.Y) return false;
		if (LocalPoint.Y > clipRectMax.X) return false;
		if (LocalPoint.Z > clipRectMax.Y) return false;
	}
	break;
	case ELGUICanvasClipType::Texture:
	{
		if (IsValid(clipTexture))
		{
			auto PlatformData = clipTexture->PlatformData;
			if(PlatformData && PlatformData->Mips.Num() > 0)
			{
				//calcualte pixel position on hit point
				auto LocalPoint = this->UIItem->GetComponentTransform().InverseTransformPosition(InWorldPoint);
				auto LocalLeftBottomPoint = UIItem->GetLocalSpaceLeftBottomPoint();
				auto UVX01 = (LocalPoint.Y - UIItem->GetLocalSpaceLeft()) / UIItem->GetWidth();
				auto UVY01 = (LocalPoint.Z - UIItem->GetLocalSpaceBottom()) / UIItem->GetHeight();
				UVY01 = 1.0f - UVY01;
				auto TexPosX = (int)(UVX01 * PlatformData->SizeX);
				auto TexPosY = (int)(UVY01 * PlatformData->SizeY);
				auto TexPos = TexPosX + TexPosY * PlatformData->SizeX;

				bool Result = true;
				if (auto Pixels = static_cast<const FColor*>(PlatformData->Mips[0].BulkData.LockReadOnly()))
				{
					auto TransparentValue = Pixels[TexPos].R;
					Result = TransparentValue > (uint8)(clipTextureHitTestThreshold * 255);
				}
				PlatformData->Mips[0].BulkData.Unlock();
				return Result;
			}
		}
		return true;
	}
	break;
	}

	return true;
}
void ULGUICanvas::ConditionalCalculateRectRange()
{
	if (bRectRangeCalculated)return;
	bRectRangeCalculated = true;

	if (this->GetActualClipType() == ELGUICanvasClipType::Rect)
	{
		if (this->GetOverrideClipType())//override clip parameter
		{
			//calculate sefl rect range
			clipRectMin.X = -UIItem->GetPivot().X * UIItem->GetWidth();
			clipRectMin.Y = -UIItem->GetPivot().Y * UIItem->GetHeight();
			clipRectMax.X = (1.0f - UIItem->GetPivot().X) * UIItem->GetWidth();
			clipRectMax.Y = (1.0f - UIItem->GetPivot().Y) * UIItem->GetHeight();
			//add offset
			clipRectMin.X = clipRectMin.X - clipRectOffset.Left;
			clipRectMax.X = clipRectMax.X + clipRectOffset.Right;
			clipRectMin.Y = clipRectMin.Y - clipRectOffset.Bottom;
			clipRectMax.Y = clipRectMax.Y + clipRectOffset.Top;
			//calculate parent rect range
			if (inheritRectClip && ParentCanvas.IsValid() && ParentCanvas->GetActualClipType() == ELGUICanvasClipType::Rect)
			{
				ParentCanvas->ConditionalCalculateRectRange();
				auto parentRectMin = FVector(0, ParentCanvas->clipRectMin.X, ParentCanvas->clipRectMin.Y);
				auto parentRectMax = FVector(0, ParentCanvas->clipRectMax.X, ParentCanvas->clipRectMax.Y);
				//transform ParentCanvas's rect to this space
				auto& parentCanvasTf = ParentCanvas->UIItem->GetComponentTransform();
				auto thisTfInv = this->UIItem->GetComponentTransform().Inverse();
				parentRectMin = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMin));
				parentRectMax = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMax));
				//inherit
				if (clipRectMin.X < parentRectMin.Y)clipRectMin.X = parentRectMin.Y;
				if (clipRectMin.Y < parentRectMin.Z)clipRectMin.Y = parentRectMin.Z;
				if (clipRectMax.X > parentRectMax.Y)clipRectMax.X = parentRectMax.Y;
				if (clipRectMax.Y > parentRectMax.Z)clipRectMax.Y = parentRectMax.Z;
			}
		}
		else//use parent clip parameter
		{
			if (ParentCanvas.IsValid() && ParentCanvas->GetActualClipType() == ELGUICanvasClipType::Rect)//have parent, use parent clip parameter
			{
				ParentCanvas->ConditionalCalculateRectRange();
				auto parentRectMin = FVector(0, ParentCanvas->clipRectMin.X, ParentCanvas->clipRectMin.Y);
				auto parentRectMax = FVector(0, ParentCanvas->clipRectMax.X, ParentCanvas->clipRectMax.Y);
				//transform ParentCanvas's rect to this space
				auto& parentCanvasTf = ParentCanvas->UIItem->GetComponentTransform();
				auto thisTfInv = this->UIItem->GetComponentTransform().Inverse();
				parentRectMin = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMin));
				parentRectMax = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMax));

				clipRectMin.X = parentRectMin.Y;
				clipRectMin.Y = parentRectMin.Z;
				clipRectMax.X = parentRectMax.Y;
				clipRectMax.Y = parentRectMax.Z;
			}
			else//no parent, use self parameter
			{
				//calculate sefl rect range
				clipRectMin.X = -UIItem->GetPivot().X * UIItem->GetWidth();
				clipRectMin.Y = -UIItem->GetPivot().Y * UIItem->GetHeight();
				clipRectMax.X = (1.0f - UIItem->GetPivot().X) * UIItem->GetWidth();
				clipRectMax.Y = (1.0f - UIItem->GetPivot().Y) * UIItem->GetHeight();
				//add offset
				clipRectMin.X = clipRectMin.X - clipRectOffset.Left;
				clipRectMax.X = clipRectMax.X + clipRectOffset.Right;
				clipRectMin.Y = clipRectMin.Y - clipRectOffset.Bottom;
				clipRectMax.Y = clipRectMax.Y + clipRectOffset.Top;
			}
		}
	}
	else
	{
		//calculate self rect range
		clipRectMin.X = -UIItem->GetPivot().X * UIItem->GetWidth();
		clipRectMin.Y = -UIItem->GetPivot().Y * UIItem->GetHeight();
		clipRectMax.X = (1.0f - UIItem->GetPivot().X) * UIItem->GetWidth();
		clipRectMax.Y = (1.0f - UIItem->GetPivot().Y) * UIItem->GetHeight();
	}
}


FLinearColor ULGUICanvas::GetRectClipOffsetAndSize()
{
	ConditionalCalculateRectRange();
	return FLinearColor((clipRectMin.X + clipRectMax.X) * 0.5f, (clipRectMin.Y + clipRectMax.Y) * 0.5f, (clipRectMax.X - clipRectMin.X) * 0.5f, (clipRectMax.Y - clipRectMin.Y) * 0.5f);
}
FLinearColor ULGUICanvas::GetRectClipFeather()
{
	if (this->GetOverrideClipType())//override clip parameter
	{
		return FLinearColor(clipFeather.X, clipFeather.Y, 0, 0);
	}
	else//use parent clip parameter
	{
		if (ParentCanvas.IsValid())//have parent, use parent clip parameter
		{
			return FLinearColor(ParentCanvas->clipFeather.X, ParentCanvas->clipFeather.Y, 0, 0);
		}
		else
		{
			return FLinearColor(clipFeather.X, clipFeather.Y, 0, 0);
		}
	}
}
FLinearColor ULGUICanvas::GetTextureClipOffsetAndSize()
{
	auto Offset = FVector2D(UIItem->GetWidth() * -UIItem->GetPivot().X, UIItem->GetHeight() * -UIItem->GetPivot().Y);
	return FLinearColor(Offset.X, Offset.Y, UIItem->GetWidth(), UIItem->GetHeight());
}

void ULGUICanvas::SetClipType(ELGUICanvasClipType newClipType) 
{
	if (clipType != newClipType)
	{
		bClipTypeChanged = true;
		clipType = newClipType;
		MarkCanvasUpdate(true, true, false);
	}
}
void ULGUICanvas::SetRectClipFeather(FVector2D newFeather) 
{
	if (clipFeather != newFeather)
	{
		bRectClipParameterChanged = true;
		clipFeather = newFeather;
		MarkCanvasUpdate(true, true, false);
	}
}
void ULGUICanvas::SetRectClipOffset(FMargin newOffset)
{
	if (clipRectOffset != newOffset)
	{
		bRectClipParameterChanged = true;
		bRectRangeCalculated = false;
		MarkCanvasUpdate(true, true, false);
	}
}
void ULGUICanvas::SetClipTexture(UTexture2D* newTexture) 
{
	if (clipTexture != newTexture)
	{
		bTextureClipParameterChanged = true;
		clipTexture = newTexture;
		MarkCanvasUpdate(true, true, false);
	}
}
void ULGUICanvas::SetInheriRectClip(bool newBool)
{
	if (inheritRectClip != newBool)
	{
		inheritRectClip = newBool;
		bRectRangeCalculated = false;
		MarkCanvasUpdate(true, true, false);
	}
}

void ULGUICanvas::SetSortOrderAdditionalValueRecursive(int32 InAdditionalValue)
{
	if (FMath::Abs(this->sortOrder + InAdditionalValue) > MAX_int16)
	{
		auto errorMsg = LOCTEXT("SortOrderOutOfRange", "[ULGUICanvas::SetSortOrder] sortOrder out of range!\nNOTE! sortOrder value is stored with int16 type, so valid range is -32768 to 32767");
		UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
#if WITH_EDITOR
		LGUIUtils::EditorNotification(errorMsg);
#endif
		return;
	}

	this->sortOrder += InAdditionalValue;
	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		ChildCanvas->SetSortOrderAdditionalValueRecursive(InAdditionalValue);
	}
}

void ULGUICanvas::SetSortOrder(int32 InSortOrder, bool InPropagateToChildrenCanvas)
{
	if (sortOrder != InSortOrder)
	{
		if (RootCanvas.IsValid())
		{
			RootCanvas->bNeedToSortRenderPriority = true;
		}
		MarkCanvasUpdate(false, false, false);
		if (InPropagateToChildrenCanvas)
		{
			int32 Diff = InSortOrder - sortOrder;
			SetSortOrderAdditionalValueRecursive(Diff);
		}
		else
		{
			if (FMath::Abs(InSortOrder) > MAX_int16)
			{
				auto errorMsg = LOCTEXT("SortOrderOutOfRange", "[ULGUICanvas::SetSortOrder] sortOrder out of range!\nNOTE! sortOrder value is stored with int16 type, so valid range is -32768 to 32767");
				UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errorMsg);
#endif
				InSortOrder = FMath::Clamp(InSortOrder, (int32)MIN_int16, (int32)MAX_int16);
			}
			this->sortOrder = InSortOrder;
		}
		if (this == RootCanvas)
		{
			if (bCurrentIsLGUIRendererOrUERenderer)
			{
				TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension = nullptr;
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())
				{
					ViewExtension = ULGUIEditorManagerObject::GetViewExtension(GetWorld(), false);
				}
				else
#endif
				{
					ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
				}
				if (ViewExtension.IsValid())
				{
					ViewExtension->SetRenderCanvasSortOrder(this, this->sortOrder);
				}
			}
		}
#if WITH_EDITOR
		if (!GetWorld()->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				switch (this->GetActualRenderMode())
				{
				default:
				case ELGUIRenderMode::ScreenSpaceOverlay:
				case ELGUIRenderMode::WorldSpace_LGUI:
					ULGUIEditorManagerObject::Instance->MarkSortLGUIRenderer();
					break;
				case ELGUIRenderMode::WorldSpace:
					ULGUIEditorManagerObject::Instance->MarkSortWorldSpaceCanvas();
					break;
				case ELGUIRenderMode::RenderTarget:
					ULGUIEditorManagerObject::Instance->MarkSortRenderTargetSpaceCanvas();
					break;
				}
			}
		}
		else
#endif
		{
			if (auto Instance = ALGUIManagerActor::GetLGUIManagerActorInstance(this))
			{
				switch (this->GetActualRenderMode())
				{
				default:
				case ELGUIRenderMode::ScreenSpaceOverlay:
				case ELGUIRenderMode::WorldSpace_LGUI:
					Instance->MarkSortLGUIRenderer();
					break;
				case ELGUIRenderMode::WorldSpace:
					Instance->MarkSortWorldSpaceCanvas();
					break;
				case ELGUIRenderMode::RenderTarget:
					Instance->MarkSortRenderTargetSpaceCanvas();
					break;
				}
			}
		}
	}
}
void ULGUICanvas::SetSortOrderToHighestOfHierarchy(bool InPropagateToChildrenCanvas)
{
	int32 Min = INT_MAX, Max = INT_MIN;
	GetMinMaxSortOrderOfHierarchy(Min, Max);
	SetSortOrder(Max + 1, InPropagateToChildrenCanvas);
}
void ULGUICanvas::SetSortOrderToLowestOfHierarchy(bool InPropagateToChildrenCanvas)
{
	int32 Min = INT_MAX, Max = INT_MIN;
	GetMinMaxSortOrderOfHierarchy(Min, Max);
	SetSortOrder(Min - 1, InPropagateToChildrenCanvas);
}

void ULGUICanvas::GetMinMaxSortOrderOfHierarchy(int32& OutMin, int32& OutMax)
{
	auto ThisCanvasSortOrder = this->GetActualSortOrder();
	if (ThisCanvasSortOrder < OutMin)
	{
		OutMin = ThisCanvasSortOrder;
	}
	if (ThisCanvasSortOrder > OutMax)
	{
		OutMax = ThisCanvasSortOrder;
	}
	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		ChildCanvas->GetMinMaxSortOrderOfHierarchy(OutMin, OutMax);
	}
}


TArray<UMaterialInterface*> ULGUICanvas::GetDefaultMaterials()const
{
	TArray<UMaterialInterface*> ResultArray;
	ResultArray.AddUninitialized(LGUI_DEFAULT_MATERIAL_COUNT);
	for (int i = 0; i < LGUI_DEFAULT_MATERIAL_COUNT; i++)
	{
		ResultArray[i] = DefaultMaterials[i];
	}
	return ResultArray;
}

void ULGUICanvas::SetDefaultMaterials(const TArray<UMaterialInterface*>& InMaterialArray)
{
	if (InMaterialArray.Num() < LGUI_DEFAULT_MATERIAL_COUNT)
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUICanvas::SetDefaultMaterials] InMaterialArray's count must be %d"), LGUI_DEFAULT_MATERIAL_COUNT);
		return;
	}
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		if (DrawcallItem->type == EUIDrawcallType::BatchGeometry)
		{
			if (DrawcallItem->material == nullptr)
			{
				DrawcallItem->materialChanged = true;
			}
		}
	}
	for (int i = 0; i < LGUI_DEFAULT_MATERIAL_COUNT; i++)
	{
		if (IsValid(InMaterialArray[i]))
		{
			DefaultMaterials[i] = InMaterialArray[i];
		}
	}
	//clear old material
	PooledUIMaterialList.Reset();
	MarkCanvasUpdate(true, false, false);
}

void ULGUICanvas::SetDynamicPixelsPerUnit(float newValue)
{
	if (dynamicPixelsPerUnit != newValue)
	{
		dynamicPixelsPerUnit = newValue;
		for (int i = 0; i < UIDrawcallList.Num(); i++)
		{
			UIDrawcallList[i]->vertexPositionChanged = true;
		}
		MarkCanvasUpdate(false, true, false);
	}
}
float ULGUICanvas::GetActualDynamicPixelsPerUnit()const
{
	if (IsRootCanvas())
	{
		return dynamicPixelsPerUnit;
	}
	else
	{
		if (GetOverrideDynamicPixelsPerUnit())
		{
			return dynamicPixelsPerUnit;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualDynamicPixelsPerUnit();
			}
		}
	}
	return dynamicPixelsPerUnit;
}

float ULGUICanvas::GetActualBlendDepth()const
{
	if (IsRootCanvas())
	{
		return blendDepth;
	}
	else
	{
		if (GetOverrideBlendDepth())
		{
			return blendDepth;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualBlendDepth();
			}
		}
	}
	return blendDepth;
}

int32 ULGUICanvas::GetActualSortOrder()const
{
	if (IsRootCanvas())
	{
		if (bOverrideSorting)
		{
			return sortOrder;
		}
		else
		{
			return 0;
		}
	}
	else
	{
		if (bOverrideSorting)
		{
			return sortOrder;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualSortOrder();
			}
		}
	}
	return sortOrder;
}

void ULGUICanvas::SetOverrideSorting(bool value)
{
	if (bOverrideSorting != value)
	{
		bOverrideSorting = value;
		if (RootCanvas.IsValid())
		{
			RootCanvas->bNeedToSortRenderPriority = true;
		}
		MarkCanvasUpdate(false, false, false);
	}
}

ELGUICanvasClipType ULGUICanvas::GetActualClipType()const
{
	if (IsRootCanvas())
	{
		return clipType;
	}
	else
	{
		if (GetOverrideClipType())
		{
			return clipType;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualClipType();
			}
		}
	}
	return clipType;
}

int8 ULGUICanvas::GetActualAdditionalShaderChannelFlags()const
{
	if (IsRootCanvas())
	{
		return additionalShaderChannels;
	}
	else
	{
		if (GetOverrideAddionalShaderChannel())
		{
			return additionalShaderChannels;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualAdditionalShaderChannelFlags();
			}
		}
	}
	return additionalShaderChannels;
}
bool ULGUICanvas::GetRequireNormal()const 
{
	return GetActualAdditionalShaderChannelFlags() & (1 << 0);
}
bool ULGUICanvas::GetRequireTangent()const 
{ 
	return GetActualAdditionalShaderChannelFlags() & (1 << 1);
}
bool ULGUICanvas::GetRequireUV1()const 
{ 
	return GetActualAdditionalShaderChannelFlags() & (1 << 2);
}
bool ULGUICanvas::GetRequireUV2()const 
{
	return GetActualAdditionalShaderChannelFlags() & (1 << 3);
}
bool ULGUICanvas::GetRequireUV3()const 
{ 
	return GetActualAdditionalShaderChannelFlags() & (1 << 4);
}


void ULGUICanvas::BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float InFOV, FMatrix& OutProjectionMatrix)const
{
	if (InViewportSize.X == 0 || InViewportSize.Y == 0)//in DebugCamera mode(toggle in editor by press ';'), viewport size is 0
	{
		InViewportSize.X = InViewportSize.Y = 1;
	}
	if (InProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float tempOrthoWidth = InViewportSize.X * 0.5f;
		const float tempOrthoHeight = InViewportSize.Y * 0.5f;

		const float ZScale = 1.0f / (FarClipPlane - NearClipPlane);
		const float ZOffset = -NearClipPlane;

		if ((int32)ERHIZBuffer::IsInverted)
		{
			OutProjectionMatrix = FReversedZOrthoMatrix(
				tempOrthoWidth,
				tempOrthoHeight,
				ZScale,
				ZOffset
			);
		}
		else
		{
			OutProjectionMatrix = FOrthoMatrix(
				tempOrthoWidth,
				tempOrthoHeight,
				ZScale,
				ZOffset
			);
		}
	}
	else
	{
		float XAxisMultiplier;
		float YAxisMultiplier;

		XAxisMultiplier = 1.0f;
		YAxisMultiplier = InViewportSize.X / (float)InViewportSize.Y;

		if ((int32)ERHIZBuffer::IsInverted)
		{
			OutProjectionMatrix = FReversedZPerspectiveMatrix(
				InFOV,
				InFOV,
				XAxisMultiplier,
				YAxisMultiplier,
				NearClipPlane,
				FarClipPlane
			);
		}
		else
		{
			OutProjectionMatrix = FPerspectiveMatrix(
				InFOV,
				InFOV,
				XAxisMultiplier,
				YAxisMultiplier,
				NearClipPlane,
				FarClipPlane
			);
		}
	}
}
float ULGUICanvas::CalculateDistanceToCamera()const
{
	if (ProjectionType == ECameraProjectionMode::Orthographic)
	{
		return 1000;
	}
	else
	{
		return UIItem->GetWidth() * 0.5f / FMath::Tan(FMath::DegreesToRadians(FOVAngle * 0.5f)) * UIItem->GetRelativeScale3D().X;
	}
}
FMatrix ULGUICanvas::GetViewProjectionMatrix()const
{
	if (bIsViewProjectionMatrixDirty)
	{
		if (!CheckUIItem())
		{
			UE_LOG(LGUI, Error, TEXT("[LGUICanvas::GetViewProjectionMatrix]UIItem not valid!"));
			return cacheViewProjectionMatrix;
		}
		bIsViewProjectionMatrixDirty = false;

		FVector ViewLocation = GetViewLocation();
		FMatrix ViewRotationMatrix = FInverseRotationMatrix(GetViewRotator())
			* FMatrix(
				FPlane(0, 0, 1, 0),
				FPlane(1, 0, 0, 0),
				FPlane(0, 1, 0, 0),
				FPlane(0, 0, 0, 1))
			;
		FMatrix ProjectionMatrix = GetProjectionMatrix();
		cacheViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
	}
	return cacheViewProjectionMatrix;
}
FMatrix ULGUICanvas::GetProjectionMatrix()const
{
	if (bOverrideProjectionMatrix)
		return OverrideProjectionMatrix;

	FMatrix ProjectionMatrix = FMatrix::Identity;
	const float FOV = (bOverrideFovAngle ? OverrideFovAngle : FOVAngle) * (float)PI / 360.0f;
	BuildProjectionMatrix(GetViewportSize(), ProjectionType, FOV, ProjectionMatrix);
	return ProjectionMatrix;
}
FVector ULGUICanvas::GetViewLocation()const
{
	if (bOverrideViewLocation)
		return OverrideViewLocation;

	return UIItem->GetComponentLocation() - UIItem->GetForwardVector() * CalculateDistanceToCamera();
}
FRotator ULGUICanvas::GetViewRotator()const
{
	if (bOverrideViewRotation)
		return OverrideViewRotation;

	return UIItem->GetComponentRotation();
}
FIntPoint ULGUICanvas::GetViewportSize()const
{
	FIntPoint ViewportSize = FIntPoint(2, 2);
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (CheckUIItem())
			{
				ViewportSize.X = UIItem->GetWidth();
				ViewportSize.Y = UIItem->GetHeight();
			}
		}
		else
#endif
		{
			if (renderMode == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				if (auto pc = world->GetFirstPlayerController())
				{
					pc->GetViewportSize(ViewportSize.X, ViewportSize.Y);
				}
			}
			else if (renderMode == ELGUIRenderMode::RenderTarget && IsValid(renderTarget))
			{
				ViewportSize.X = renderTarget->SizeX;
				ViewportSize.Y = renderTarget->SizeY;
			}
		}
	}
	return ViewportSize;
}

void ULGUICanvas::SetRenderMode(ELGUIRenderMode value)
{
	if (renderMode != value)
	{
		renderMode = value;

		CheckRenderMode();
	}
}

void ULGUICanvas::SetPixelPerfect(bool value)
{
	if (pixelPerfect != value)
	{
		pixelPerfect = value;
		for (int i = 0; i < UIDrawcallList.Num(); i++)
		{
			auto DrawcallItem = UIDrawcallList[i];
			if (DrawcallItem->type == EUIDrawcallType::BatchGeometry)
			{
				DrawcallItem->needToUpdateVertex = true;
			}
		}
		MarkCanvasUpdate(false, true, false);
	}
}

void ULGUICanvas::SetProjectionParameters(TEnumAsByte<ECameraProjectionMode::Type> InProjectionType, float InFovAngle, float InNearClipPlane, float InFarClipPlane)
{
	ProjectionType = InProjectionType;
	FOVAngle = InFovAngle;
	NearClipPlane = InNearClipPlane;
	FarClipPlane = InFarClipPlane;

	bIsViewProjectionMatrixDirty = true;
}

void ULGUICanvas::SetRenderTarget(UTextureRenderTarget2D* value)
{
	if (renderTarget != value)
	{
		renderTarget = value;
		if (renderMode == ELGUIRenderMode::RenderTarget)
		{
			if (RenderTargetViewExtension.IsValid())
			{
				RenderTargetViewExtension->SetRenderToRenderTarget(true, renderTarget);
			}
		}
	}
}

ELGUIRenderMode ULGUICanvas::GetActualRenderMode()const
{
	if (IsRootCanvas())
	{
		return this->renderMode;
	}
	else
	{
		if (RootCanvas.IsValid())
		{
			return RootCanvas->renderMode;
		}
	}
	return ELGUIRenderMode::WorldSpace;
}
bool ULGUICanvas::GetActualPixelPerfect()const
{
	if (IsRootCanvas())
	{
		return this->bCurrentIsLGUIRendererOrUERenderer
			&& pixelPerfect;
	}
	else
	{
		if (RootCanvas.IsValid())
		{
			if (GetOverridePixelPerfect())
			{
				return RootCanvas->bCurrentIsLGUIRendererOrUERenderer
					&& this->pixelPerfect;
			}
			else
			{
				if (ParentCanvas.IsValid())
				{
					return RootCanvas->bCurrentIsLGUIRendererOrUERenderer
						&& ParentCanvas->GetActualPixelPerfect();
				}
			}
		}
	}
	return false;
}

void ULGUICanvas::SetBlendDepth(float value)
{
	if (blendDepth != value)
	{
		blendDepth = value;

		if (RootCanvas.IsValid())
		{
			if (RootCanvas->bCurrentIsLGUIRendererOrUERenderer)
			{
				if (RootCanvas->IsRenderToWorldSpace())
				{
					TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ViewExtension = nullptr;
#if WITH_EDITOR
					if (!GetWorld()->IsGameWorld())
					{
						ViewExtension = ULGUIEditorManagerObject::GetViewExtension(GetWorld(), false);
					}
					else
#endif
					{
						ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
					}
					if (ViewExtension.IsValid())
					{
						ViewExtension->SetRenderCanvasBlendDepth(this, this->GetActualBlendDepth());
					}
				}
			}
		}
	}
}


UTextureRenderTarget2D* ULGUICanvas::GetActualRenderTarget()const
{
	if (IsRootCanvas())
	{
		return this->renderTarget;
	}
	else
	{
		if (RootCanvas.IsValid())
		{
			return RootCanvas->renderTarget;
		}
	}
	return nullptr;
}
int32 ULGUICanvas::GetDrawcallCount()const
{
	int32 Result = 0;
	for (auto& Item : UIDrawcallList)
	{
		if (Item->type != EUIDrawcallType::ChildCanvas)
		{
			Result++;
		}
	}
	return Result;
}


DECLARE_CYCLE_STAT(TEXT("Canvas 2DTransform"), STAT_Transform2D, STATGROUP_LGUI);
void ULGUICanvas::GetCacheUIItemToCanvasTransform(UUIBaseRenderable* item, FLGUICacheTransformContainer& outResult)
{
	//SCOPE_CYCLE_COUNTER(STAT_Transform2D);
	if (auto tfPtr = this->CacheUIItemToCanvasTransformMap.Find(item))
	{
		outResult = *tfPtr;
	}
	else
	{
		auto inverseCanvasTf = this->UIItem->GetComponentTransform().Inverse();
		const auto& itemTf = item->GetComponentTransform();

		FTransform itemToCanvasTf;
		FTransform::Multiply(&itemToCanvasTf, &itemTf, &inverseCanvasTf);
		outResult.Transform = itemToCanvasTf;

		auto itemToCanvasTf2D = ConvertTo2DTransform(itemToCanvasTf);
		FVector2D itemMin, itemMax;
		CalculateUIItem2DBounds(item, itemToCanvasTf2D, itemMin, itemMax);

		outResult.BoundsMin2D = itemMin;
		outResult.BoundsMax2D = itemMax;
		CacheUIItemToCanvasTransformMap.Add(item, outResult);
	}
}
FTransform2D ULGUICanvas::ConvertTo2DTransform(const FTransform& Transform)
{
	auto itemToCanvasMatrix = Transform.ToMatrixWithScale();
	auto itemLocation = Transform.GetLocation();
	auto itemToCanvasTf2D = FTransform2D(FMatrix2x2(itemToCanvasMatrix.M[1][1], itemToCanvasMatrix.M[1][2], itemToCanvasMatrix.M[2][1], itemToCanvasMatrix.M[2][2]), FVector2D(itemLocation.Y, itemLocation.Z));
	return itemToCanvasTf2D;
}

void GetMinMax(float a, float b, float c, float d, float& min, float& max)
{
	float abMin = FMath::Min(a, b);
	float abMax = FMath::Max(a, b);
	float cdMin = FMath::Min(c, d);
	float cdMax = FMath::Max(c, d);
	min = FMath::Min(abMin, cdMin);
	max = FMath::Max(abMax, cdMax);
}
void ULGUICanvas::CalculateUIItem2DBounds(UUIBaseRenderable* item, const FTransform2D& transform, FVector2D& min, FVector2D& max)
{
	FVector2D localPoint1, localPoint2;
	item->GetGeometryBoundsInLocalSpace(localPoint1, localPoint2);
	auto point1 = transform.TransformPoint(localPoint1);
	auto point2 = transform.TransformPoint(localPoint2);
	auto point3 = transform.TransformPoint(FVector2D(localPoint2.X, localPoint1.Y));
	auto point4 = transform.TransformPoint(FVector2D(localPoint1.X, localPoint2.Y));

	GetMinMax(point1.X, point2.X, point3.X, point4.X, min.X, max.X);
	GetMinMax(point1.Y, point2.Y, point3.Y, point4.Y, min.Y, max.Y);
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif