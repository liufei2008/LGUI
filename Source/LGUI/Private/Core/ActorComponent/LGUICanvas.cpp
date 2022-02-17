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

void ULGUICanvas::UpdateCanvas()
{
	if (!bCanTickUpdate)return;

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
			if (this == RootCanvas)
			{
				if (bCurrentIsLGUIRendererOrUERenderer)
				{
					if (!bHasAddToLGUIScreenSpaceRenderer && GetActualRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
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
			}

			if (!this->IsRenderByOtherCanvas())//if is render by other canvas then skip update
			{
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
	if (!this->IsRenderByOtherCanvas())
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
		}
		this->MarkCanvasUpdate(false, false, true, true);
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
		return RootCanvas->renderMode == ELGUIRenderMode::RenderTarget && IsValid(RootCanvas->renderTarget);
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
			|| (RootCanvas->renderMode == ELGUIRenderMode::RenderTarget && IsValid(RootCanvas->renderTarget))
			|| RootCanvas->renderMode == ELGUIRenderMode::WorldSpace_LGUI
			;
	}
	return false;
}

void ULGUICanvas::MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall)
{
	this->bCanTickUpdate = true;
	this->GetActualRenderCanvas()->bCanTickUpdate = true;
	if (bMaterialOrTextureChanged || bTransformOrVertexPositionChanged || bHierarchyOrderChanged || bForceRebuildDrawcall)
	{
		this->bShouldRebuildDrawcall = true;
		this->GetActualRenderCanvas()->bShouldRebuildDrawcall = true;
	}
	if (bHierarchyOrderChanged)
	{
		this->bShouldSortRenderableOrder = true;
		this->GetActualRenderCanvas()->bShouldSortRenderableOrder = true;
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
	auto& ActualUIDrawcallList = this->GetActualRenderCanvas()->UIDrawcallList;
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
					Drawcall->needToRebuildMesh = true;
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
			if (ChildRenderCanvas->IsRenderByOtherCanvas())
			{
				ChildRenderCanvas->UpdateGeometry_Implement();
			}
		}
		else
		{
			auto UIRenderableItem = (UUIBaseRenderable*)(Item);
			UIRenderableItem->UpdateGeometry();
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas DrawcallBatch"), STAT_DrawcallBatch, STATGROUP_LGUI);
void ULGUICanvas::UpdateDrawcall_Implement(ULGUICanvas* InRenderCanvas, TArray<TSharedPtr<UUIDrawcall>>& InUIDrawcallList, TArray<TSharedPtr<UUIDrawcall>>& InCacheUIDrawcallList, bool& OutNeedToSortRenderPriority)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawcallBatch);

	//use a set to store already processed UIItem. good for overlap calculation
	TSet<UUIBaseRenderable*> ProcessedUIItemSet;
	ProcessedUIItemSet.Reserve(UIRenderableList.Num());

	auto IntersectBounds = [](FVector2D aMin, FVector2D aMax, FVector2D bMin, FVector2D bMax) {
		return !(bMin.X >= aMax.X
			|| bMax.X <= aMin.X
			|| bMax.Y <= aMin.Y
			|| bMin.Y >= aMax.Y
			);
	};
	auto OverlapWithOtherDrawcall = [&](const FLGUICacheTransformContainer& ItemToCanvasTf, TSharedPtr<UUIDrawcall> DrawcallItem) {
		switch (DrawcallItem->type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			//compare drawcall item's bounds
			for (auto& otherItem : DrawcallItem->renderObjectList)
			{
				if (!ProcessedUIItemSet.Contains(otherItem.Get()))continue;//only calculate processed UIItem
				FLGUICacheTransformContainer OtherItemToCanvasTf;
				this->GetCacheUIItemToCanvasTransform(otherItem.Get(), true, OtherItemToCanvasTf);
				//check bounds overlap
				if (IntersectBounds(ItemToCanvasTf.BoundsMin2D, ItemToCanvasTf.BoundsMax2D, OtherItemToCanvasTf.BoundsMin2D, OtherItemToCanvasTf.BoundsMax2D))
				{
					return true;
				}
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			FLGUICacheTransformContainer OtherItemToCanvasTf;
			DrawcallItem->postProcessRenderableObject->GetRenderCanvas()->GetCacheUIItemToCanvasTransform(DrawcallItem->postProcessRenderableObject.Get(), true, OtherItemToCanvasTf);
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

	auto CanFitInDrawcall = [&](UUIBatchGeometryRenderable* InUIItem, bool InIs2DUI, const FLGUICacheTransformContainer& InUIItemToCanvasTf, int32& OutDrawcallIndexToFitin)
	{
		auto LastDrawcallIndex = InUIDrawcallList.Num() - 1;
		if (LastDrawcallIndex < 0)
		{
			return false;
		}
		//first step, check last drawcall, because 3d UI can only batch into last drawcall
		{
			auto LastDrawcall = InUIDrawcallList[LastDrawcallIndex];
			if (
				LastDrawcall->type == EUIDrawcallType::BatchGeometry
				&& LastDrawcall->material == InUIItem->GetGeometry()->material
				&& LastDrawcall->texture == InUIItem->GetGeometry()->texture
				)
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

			if (
				DrawcallItem->type != EUIDrawcallType::BatchGeometry
				|| DrawcallItem->material != InUIItem->GetGeometry()->material
				|| DrawcallItem->texture != InUIItem->GetGeometry()->texture
				)//can't fit in this drawcall, should check overlap
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
	auto PushSingleDrawcall = [&](UUIItem* InUIItem, bool InSearchInCacheList, TSharedPtr<UIGeometry> InItemGeo, bool InIs2DSpace, EUIDrawcallType InDrawcallType) {
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
			DrawcallItem = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
			DrawcallItem->type = InDrawcallType;

			switch (InDrawcallType)
			{
			case EUIDrawcallType::BatchGeometry:
			{
				DrawcallItem->texture = InItemGeo->texture;
				DrawcallItem->material = InItemGeo->material.Get();
				DrawcallItem->renderObjectList.Add((UUIBatchGeometryRenderable*)InUIItem);
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
		DrawcallItem->bIs2DSpace = InIs2DSpace;

		if (InDrawcallType == EUIDrawcallType::BatchGeometry
			|| InDrawcallType == EUIDrawcallType::PostProcess
			|| InDrawcallType == EUIDrawcallType::DirectMesh)
		{
			((UUIBaseRenderable*)InUIItem)->drawcall = DrawcallItem;
		}
		DrawcallItem->manageCanvas = this;
		InUIDrawcallList.Add(DrawcallItem);

		if (FoundDrawcallIndex != 0)//if not find drawcall or found drawcall not at head of array, means drawcall list's order is changed compare to cache list, then we need to sort render order
		{
			OutNeedToSortRenderPriority = true;
		}
		//OutNeedToSortRenderPriority = true;//@todo: this line could make it sort every time, which is not good performance
	};
	auto ClearDrawcall = [&](TSharedPtr<UUIDrawcall> InDrawcallItem, UUIBatchGeometryRenderable* InUIBatchGeometryRenderable) {
		if (InDrawcallItem->drawcallMesh.IsValid())
		{
			if (InDrawcallItem->drawcallMeshSection.IsValid())
			{
				InDrawcallItem->drawcallMesh->DeleteMeshSection(InDrawcallItem->drawcallMeshSection.Pin());
				InDrawcallItem->drawcallMeshSection.Reset();
			}
		}
		InDrawcallItem->needToRebuildMesh = true;
		InDrawcallItem->materialNeedToReassign = true;
		InDrawcallItem->renderObjectList.Remove(InUIBatchGeometryRenderable);
		InUIBatchGeometryRenderable->drawcall = nullptr;
	};

	//for sorted ui items, iterate from head to tail, compare drawcall from tail to head
	for (int i = 0; i < UIRenderableList.Num(); i++)
	{
		auto& Item = UIRenderableList[i];
		//check(Item->GetIsUIActiveInHierarchy());
		if (!Item->GetIsUIActiveInHierarchy())continue;
		
		if (Item->IsCanvasUIItem() && Item->GetRenderCanvas() != this)//is child canvas
		{
			auto ChildRenderCanvas = Item->GetRenderCanvas();
			if (ChildRenderCanvas->IsRenderByOtherCanvas())
			{
				ChildRenderCanvas->UpdateDrawcall_Implement(InRenderCanvas, InUIDrawcallList, InCacheUIDrawcallList, OutNeedToSortRenderPriority);
				FitInDrawcallMinIndex = InUIDrawcallList.Num();//after children drawcall, update this index so parent's UI element will not batch into children's drawcall
			}
		}
		else
		{
			auto UIRenderableItem = (UUIBaseRenderable*)(Item);
			FLGUICacheTransformContainer UIItemToCanvasTf;
			this->GetCacheUIItemToCanvasTransform(UIRenderableItem, true, UIItemToCanvasTf);
			bool is2DUIItem = Is2DUITransform(UIItemToCanvasTf.Transform);
			switch (UIRenderableItem->GetUIRenderableType())
			{
			default:
			case EUIRenderableType::UIBatchGeometryRenderable:
			{
				auto UIBatchGeometryRenderableItem = (UUIBatchGeometryRenderable*)UIRenderableItem;
				auto ItemGeo = UIBatchGeometryRenderableItem->GetGeometry();
				if (ItemGeo.IsValid() == false)continue;
				if (ItemGeo->vertices.Num() == 0)continue;

				int DrawcallIndexToFitin;
				if (CanFitInDrawcall(UIBatchGeometryRenderableItem, is2DUIItem, UIItemToCanvasTf, DrawcallIndexToFitin))
				{
					auto DrawcallItem = InUIDrawcallList[DrawcallIndexToFitin];
					DrawcallItem->bIs2DSpace = DrawcallItem->bIs2DSpace && is2DUIItem;
					if (UIBatchGeometryRenderableItem->drawcall == DrawcallItem)//already exist in this drawcall (added previoursly)
					{
							
					}
					else//not exist in this drawcall
					{
						auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
						if (OldDrawcall.IsValid())//maybe exist in other drawcall, should remove from that drawcall
						{
							ClearDrawcall(OldDrawcall, UIBatchGeometryRenderableItem);
						}
						//add to this drawcall
						DrawcallItem->renderObjectList.Add(UIBatchGeometryRenderableItem);
						DrawcallItem->needToRebuildMesh = true;
						UIBatchGeometryRenderableItem->drawcall = DrawcallItem;
						//copy update state from old to new
						if (OldDrawcall.IsValid())
						{
							OldDrawcall->CopyUpdateState(DrawcallItem.Get());
						}
					}
				}
				else//cannot fit in any other drawcall
				{
					auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
					if (OldDrawcall.IsValid())//maybe exist in other drawcall, should remove from that drawcall
					{
						if (InUIDrawcallList.Contains(OldDrawcall))//if this drawcall already exist (added previoursly), then remove the object from the drawcall. if not exist, then just add the entire drawcall to list
						{
							ClearDrawcall(OldDrawcall, UIBatchGeometryRenderableItem);
						}
					}
					//make a new drawacll
					PushSingleDrawcall(UIBatchGeometryRenderableItem, true, ItemGeo, is2DUIItem, EUIDrawcallType::BatchGeometry);
				}
			}
			break;
			case EUIRenderableType::UIPostProcessRenderable:
			{
				auto UIPostProcessRenderableItem = (UUIPostProcessRenderable*)UIRenderableItem;
				auto ItemGeo = UIPostProcessRenderableItem->GetGeometry();
				if (ItemGeo.IsValid() == false)continue;
				if (ItemGeo->vertices.Num() == 0)continue;
				//every postprocess is a drawcall
				PushSingleDrawcall(UIRenderableItem, true, ItemGeo, is2DUIItem, EUIDrawcallType::PostProcess);
				//no need to copy drawcall's update data for UIPostProcessRenderable, because UIPostProcessRenderable's drawcall should be the same as previours one
			}
			break;
			case EUIRenderableType::UIDirectMeshRenderable:
			{
				auto UIDirectMeshRenderableItem = (UUIDirectMeshRenderable*)UIRenderableItem;
				//every direct mesh is a drawcall
				PushSingleDrawcall(UIRenderableItem, true, nullptr, is2DUIItem, EUIDrawcallType::DirectMesh);
				//no need to copy drawcall's update data for UIDirectMeshRenderable, because UIDirectMeshRenderable's drawcall should be the same as previours one
			}
			break;
			}

			ProcessedUIItemSet.Add(UIRenderableItem);
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
			DrawcallItem->needToRebuildMesh = true;
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

ULGUIMeshComponent* ULGUICanvas::FindNextValidMeshInDrawcallList(int32 InStartIndex)
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
		{
			return nullptr;
		}
		break;
		}
	}
	return nullptr;
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
	//update children canvas
	for (auto item : ChildrenCanvasArray)
	{
		if (item.IsValid() && item->GetIsUIActive())
		{
			item->UpdateCanvasDrawcallRecursive();
		}
	}

	//reset transform map, because transform change
	CacheUIItemToCanvasTransformMap.Reset();

	if (this->IsRenderByOtherCanvas())//if is render by other canvas then skip drawcall creation
	{
		return;
	}

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

	//update drawcall
	if (bCanTickUpdate)
	{
		bCanTickUpdate = false;
#if !UE_BUILD_SHIPPING
		check(!this->IsRenderByOtherCanvas());
#endif

		UpdateGeometry_Implement();

		if (bShouldRebuildDrawcall)
		{
			//store prev created drawcall to cache list, so when we create drawcall, we can search in the cache list and use existing one
			for (auto Item : UIDrawcallList)
			{
				CacheUIDrawcallList.Add(Item);
			}
			UIDrawcallList.Reset();

			bool bOutNeedToSortRenderPriority = bNeedToSortRenderPriority;
			UpdateDrawcall_Implement(this, UIDrawcallList, CacheUIDrawcallList
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
					if (DrawcallInCache->manageCanvas.IsValid())//manageCanvas could be destroyed before UpdateDrawcall
					{
						DrawcallInCache->manageCanvas->AddUIMaterialToPool(DrawcallInCache->materialInstanceDynamic.Get());
					}
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
	}

	//update drawcall mesh
	UpdateDrawcallMesh_Implement();

	//update drawcall material
	UpdateDrawcallMaterial_Implement();

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

void ULGUICanvas::UpdateDrawcallMesh_Implement()
{
	ULGUIMeshComponent* prevUIMesh = nullptr;
	if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace)//WorldSpace-UE-Renderer only have one drawcall mesh, so we just get the first one
	{
		check(UsingUIMeshList.Num() <= 1);
		if (UsingUIMeshList.Num() == 1)
		{
			prevUIMesh = UsingUIMeshList[0].Get();
		}
		else
		{
			prevUIMesh = this->GetUIMeshFromPool().Get();
		}
	}


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
				if (prevUIMesh == nullptr)
				{
					//if drawcall mesh is not valid, we need to search in next drawcalls and find mesh drawcall object (not post process drawcall)
					if (auto foundMesh = FindNextValidMeshInDrawcallList(i + 1))
					{
						prevUIMesh = foundMesh;
					}
					else//not find valid mesh, then get from pool
					{
						prevUIMesh = this->GetUIMeshFromPool().Get();
					}
				}
				DrawcallItem->drawcallMesh = prevUIMesh;
			}
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
			}
			prevUIMesh = DrawcallItem->drawcallMesh.Get();
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
			if (DrawcallItem->needToRebuildMesh)
			{
				if (!MeshSection.IsValid())
				{
					MeshSection = UIMesh->GetMeshSection();
					DrawcallItem->drawcallMeshSection = MeshSection;
				}
				auto MeshSectionPtr = MeshSection.Pin();
				MeshSectionPtr->vertices.Reset();
				MeshSectionPtr->triangles.Reset();
				DrawcallItem->GetCombined(MeshSectionPtr->vertices, MeshSectionPtr->triangles);
				MeshSectionPtr->prevVertexCount = MeshSectionPtr->vertices.Num();
				MeshSectionPtr->prevIndexCount = MeshSectionPtr->triangles.Num();
				UIMesh->CreateMeshSectionData(MeshSectionPtr);
				DrawcallItem->needToRebuildMesh = false;
				DrawcallItem->needToUpdateVertex = false;
				DrawcallItem->vertexPositionChanged = false;
			}
			else if (DrawcallItem->needToUpdateVertex)
			{
				auto MeshSectionPtr = MeshSection.Pin();
				DrawcallItem->UpdateData(MeshSectionPtr->vertices, MeshSectionPtr->triangles);
				if (MeshSectionPtr->prevVertexCount == MeshSectionPtr->vertices.Num() && MeshSectionPtr->prevIndexCount == MeshSectionPtr->triangles.Num())
				{
					UIMesh->UpdateMeshSectionData(MeshSectionPtr, true, GetActualAdditionalShaderChannelFlags());
				}
				else
				{
					check(0);//this should not happen
					//meshSection->prevVertexCount = meshSection->vertices.Num();
					//meshSection->prevIndexCount = meshSection->triangles.Num();
					//UIMesh->CreateMeshSectionData(meshSection);
				}
				DrawcallItem->needToUpdateVertex = false;
				DrawcallItem->vertexPositionChanged = false;
			}
			prevUIMesh = DrawcallItem->drawcallMesh.Get();
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
					prevUIMesh = nullptr;//set to null so a new mesh will be created for next drawcall
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
					prevUIMesh = nullptr;//set to null so a new mesh will be created for next drawcall
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
								if (this->IsRenderToWorldSpace())
								{
									uiPostProcessPrimitive->AddToLGUIWorldSpaceRenderer(this, this->GetActualSortOrder(), ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true));
								}
								else
								{
									uiPostProcessPrimitive->AddToLGUIScreenSpaceRenderer(ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true));
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
						if (this->IsRenderToWorldSpace())
						{
							uiPostProcessPrimitive->AddToLGUIWorldSpaceRenderer(this, this->GetActualSortOrder(), ALGUIManagerActor::GetViewExtension(GetWorld(), true));
						}
						else
						{
							uiPostProcessPrimitive->AddToLGUIScreenSpaceRenderer(ALGUIManagerActor::GetViewExtension(GetWorld(), true));
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

bool ULGUICanvas::IsRenderByOtherCanvas()const
{
	if (this->IsRootCanvas())
	{
		return false;
	}
	else
	{
		return
			this->ParentCanvas.IsValid()

			&& !this->GetForceSelfRender()
			&& !this->GetOverrideSorting()
			&& !this->GetOverrideAddionalShaderChannel()
			;
	}
}

ULGUICanvas* ULGUICanvas::GetActualRenderCanvas()const
{
	if (this->IsRenderByOtherCanvas())
	{
		return this->ParentCanvas->GetActualRenderCanvas();
	}
	else
	{
		return (ULGUICanvas*)this;
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
					if (this->IsRenderToWorldSpace())
					{
						UIMesh->SetSupportLGUIRenderer(GetWorld()->WorldType != EWorldType::EditorPreview, ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true), this, true);
						UIMesh->SetSupportUERenderer(GetWorld()->WorldType == EWorldType::EditorPreview);//screen space UI can appear in editor preview
					}
					else
					{
						UIMesh->SetSupportLGUIRenderer(GetWorld()->WorldType != EWorldType::EditorPreview, ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true), RootCanvas.Get(), false);
						UIMesh->SetSupportUERenderer(true);//screen space UI should appear in editor's viewport
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
				if (this->IsRenderToWorldSpace())
				{
					UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), this, true);
				}
				else
				{
					UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), RootCanvas.Get(), false);
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

void ULGUICanvas::SortDrawcall(int32& InOutRenderPriority)
{
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
		auto TempClipType = DrawcallItem->manageCanvas->GetActualClipType();
		if (DrawcallItem->type == EUIDrawcallType::BatchGeometry)
		{
			auto UIMat = DrawcallItem->materialInstanceDynamic;
			if (!UIMat.IsValid() || DrawcallItem->materialChanged || DrawcallItem->manageCanvas->bClipTypeChanged)
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
					UIMat = DrawcallItem->manageCanvas->GetUIMaterialFromPool(TempClipType);
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
			if (DrawcallItem->manageCanvas->bClipTypeChanged
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
			if (DrawcallItem->manageCanvas->bClipTypeChanged
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
				|| DrawcallItem->manageCanvas->bRectClipParameterChanged)
			{
				auto TempRectClipOffsetAndSize = DrawcallItem->manageCanvas->GetRectClipOffsetAndSize();
				auto TempRectClipFeather = DrawcallItem->manageCanvas->GetRectClipFeather();

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
				|| DrawcallItem->manageCanvas->bTextureClipParameterChanged)
			{
				auto TempTextureClipOffsetAndSize = DrawcallItem->manageCanvas->GetTextureClipOffsetAndSize();
				auto TempClipTexture = DrawcallItem->manageCanvas->GetClipTexture();

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

bool ULGUICanvas::IsPointVisible(FVector InWorldPoint)
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
		//if render by other canvas, we should use that canvas to check point visibility, because clip range is transformed to other canvas.
		ULGUICanvas* CheckCanvas = this;
		if (this->IsRenderByOtherCanvas())
		{
			CheckCanvas = this->GetActualRenderCanvas();
		}
		//transform to local space
		auto LocalPoint = CheckCanvas->UIItem->GetComponentTransform().InverseTransformPosition(InWorldPoint);
		//out of range
		if (LocalPoint.Y < clipRectMin.X) return false;
		if (LocalPoint.Z < clipRectMin.Y) return false;
		if (LocalPoint.Y > clipRectMax.X) return false;
		if (LocalPoint.Z > clipRectMax.Y) return false;
	}
	break;
	case ELGUICanvasClipType::Texture://@todo: support this!
		return true;
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
		//if render by other canvas, we should transform rect to that canvas space
		if (this->IsRenderByOtherCanvas())
		{
			auto RenderCanvas = this->GetActualRenderCanvas();
			auto RenderCanvasTfInv = RenderCanvas->UIItem->GetComponentTransform().Inverse();
			auto ThisTf = this->UIItem->GetComponentTransform();
			auto ClipRectMinPoint = RenderCanvasTfInv.TransformPosition(ThisTf.TransformPosition(FVector(0, clipRectMin.X, clipRectMin.Y)));
			clipRectMin = FVector2D(ClipRectMinPoint.Y, ClipRectMinPoint.Z);
			auto ClipRectMaxPoint = RenderCanvasTfInv.TransformPosition(ThisTf.TransformPosition(FVector(0, clipRectMax.X, clipRectMax.Y)));
			clipRectMax = FVector2D(ClipRectMaxPoint.Y, ClipRectMaxPoint.Z);
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
	//if render by other canvas, we should transform offset to that canvas space
	if (this->IsRenderByOtherCanvas())
	{
		auto RenderCanvas = this->GetActualRenderCanvas();
		auto RenderCanvasTfInv = RenderCanvas->UIItem->GetComponentTransform().Inverse();
		auto ThisTf = this->UIItem->GetComponentTransform();
		auto OffsetPoint = RenderCanvasTfInv.TransformPosition(ThisTf.TransformPosition(FVector(0, Offset.X, Offset.Y)));
		Offset = FVector2D(OffsetPoint.Y, OffsetPoint.Z);
	}
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
void ULGUICanvas::SetClipTexture(UTexture* newTexture) 
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
	//@todo:test this!!!
	if (renderTarget != value)
	{
		renderTarget = value;
		if (renderMode == ELGUIRenderMode::RenderTarget)
		{
			if (IsValid(renderTarget))
			{
				/*if (ViewExtension.IsValid())
				{
					ViewExtension.Reset();
				}*/
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


bool ULGUICanvas::GetCacheUIItemToCanvasTransform(UUIBaseRenderable* item, bool createIfNotExist, FLGUICacheTransformContainer& outResult)
{
	if (auto tfPtr = this->CacheUIItemToCanvasTransformMap.Find(item))
	{
		outResult = *tfPtr;
		return true;
	}
	else
	{
		if (createIfNotExist)
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
			return true;
		}
		return false;
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