// Copyright 2019-2021 LexLiu. All Rights Reserved.

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

PRAGMA_DISABLE_OPTIMIZATION

ULGUICanvas::ULGUICanvas()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

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

	bIsViewProjectionMatrixDirty = true;

	DefaultMeshType = ULGUIMeshComponent::StaticClass();
}

void ULGUICanvas::BeginPlay()
{
	Super::BeginPlay();
	RootCanvas = nullptr;
	CheckRootCanvas();
	bCurrentIsLGUIRendererOrUERenderer = IsRenderByLGUIRendererOrUERenderer();
	CheckUIItem();
	MarkCanvasUpdate();

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

void ULGUICanvas::UpdateRootCanvasDrawcall()
{
	if (!bCanTickUpdate)return;
	bCanTickUpdate = false;

	if (this != RootCanvas) return;

	if (CheckUIItem() && UIItem->GetIsUIActiveInHierarchy())
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

		UpdateCanvasDrawcallRecursive();
	}
}

void ULGUICanvas::OnRegister()
{
	Super::OnRegister();
	if (auto world = this->GetWorld())
	{
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
	//tell UIItem
	if (CheckUIItem())
	{
		UIItem->RegisterRenderCanvas(this);
		UIHierarchyChangedDelegateHandle = UIItem->RegisterUIHierarchyChanged(FSimpleDelegate::CreateUObject(this, &ULGUICanvas::OnUIHierarchyChanged));
		UIActiveStateChangedDelegateHandle = UIItem->RegisterUIActiveStateChanged(FSimpleDelegate::CreateUObject(this, &ULGUICanvas::OnUIActiveStateChanged));
	}
	OnUIHierarchyChanged();
}
void ULGUICanvas::OnUnregister()
{
	Super::OnUnregister();
	if (auto world = this->GetWorld())
	{
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

	//clear
	ClearDrawcall();

	RemoveFromViewExtension();
}
void ULGUICanvas::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void ULGUICanvas::ClearDrawcall()
{
	//clear drawcall
	for (auto i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		DrawcallItem->drawcallMeshSection = nullptr;
		if (DrawcallItem->postProcessRenderableObject.IsValid())
		{
			if (DrawcallItem->postProcessRenderableObject->IsRenderProxyValid())
			{
				DrawcallItem->postProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
			}
		}
		if (DrawcallItem->directMeshRenderableObject.IsValid())
		{
			DrawcallItem->directMeshRenderableObject->ClearMeshData();
		}
		if (DrawcallItem->drawcallMesh.IsValid())
		{
			DrawcallItem->drawcallMesh->DestroyComponent();
			DrawcallItem->drawcallMesh.Reset();
		}
		for (auto RenderObjectItem : DrawcallItem->renderObjectList)
		{
			if (RenderObjectItem.IsValid())
			{
				RenderObjectItem->drawcall = nullptr;
			}
		}
	}
	//clear renderable's drawcall
	for (auto item : UIRenderableList)
	{
		if (IsValid(item))
		{
			if (auto UIRenderableItem = Cast<UUIBaseRenderable>(item))
			{
				if (UIRenderableItem->drawcall.IsValid())
				{
					UIRenderableItem->drawcall = nullptr;
				}
			}
		}
	}

	for (auto item : PooledUIMeshList)
	{
		if (item.IsValid())
		{
			item->DestroyComponent();
		}
	}
	PooledUIMeshList.Empty();
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
		if (ParentCanvas.IsValid())
		{
#if !UE_BUILD_SHIPPING
			check(ParentCanvas->ChildrenCanvasArray.Contains(this));
			check(ParentCanvas->UIRenderableList.Contains(this->UIItem.Get()));
#endif
			ParentCanvas->ChildrenCanvasArray.Remove(this);
			ParentCanvas->UIRenderableList.Remove(this->UIItem.Get());
			ParentCanvas->bIsUIRenderableHierarchyChanged = true;
		}
		ParentCanvas = InParentCanvas;
		if (ParentCanvas.IsValid())
		{
#if !UE_BUILD_SHIPPING
			check(!ParentCanvas->ChildrenCanvasArray.Contains(this));
			check(!ParentCanvas->UIRenderableList.Contains(this->UIItem.Get()));
#endif
			if (this->IsRenderByOtherCanvas())//can render by other canvas, then we need to clear self's drawcall and let parent canvas create drawcall
			{
				this->ClearDrawcall();
			}
			if (this->UIItem->GetIsUIActiveInHierarchy())
			{
				ParentCanvas->UIRenderableList.Add(this->UIItem.Get());
			}
			ParentCanvas->ChildrenCanvasArray.Add(this);
			ParentCanvas->bIsUIRenderableHierarchyChanged = true;
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
		//clear and delete mesh components, so new mesh will be created. because hud and world mesh not compatible
		ClearDrawcall();
	}

	RemoveFromViewExtension();

	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		ChildCanvas->CheckRenderMode();
	}
}
void ULGUICanvas::OnUIHierarchyChanged()
{
	if (RootCanvas.IsValid())
	{
		//mark old RootCanvas update
		RootCanvas->bCanTickUpdate = true;
	}
	RootCanvas = nullptr;
	CheckRootCanvas();
	if (RootCanvas.IsValid())
	{
		//mark new RootCanvas update
		RootCanvas->bCanTickUpdate = true;
	}
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
#if !UE_BUILD_SHIPPING
			check(!ParentCanvas->UIRenderableList.Contains(this->UIItem.Get()));
#endif
			ParentCanvas->UIRenderableList.Add(this->UIItem.Get());
		}
	}
	else
	{
		if (ParentCanvas.IsValid())
		{
#if !UE_BUILD_SHIPPING
			check(ParentCanvas->UIRenderableList.Contains(this->UIItem.Get()));
#endif
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

void ULGUICanvas::MarkCanvasUpdate()
{
	if (CheckRootCanvas())
	{
		RootCanvas->bCanTickUpdate = true;//incase this Canvas's parent have layout component, so mark TopMostCanvas to update
	}
}

void ULGUICanvas::MarkUIRenderableItemHierarchyChange()
{
	this->bIsUIRenderableHierarchyChanged = true;
	if (this->IsRenderByOtherCanvas())
	{
		this->GetActualRenderCanvas()->bIsUIRenderableHierarchyChanged = true;
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
		RootCanvas->MarkCanvasUpdate();
	}

	if (auto Property = PropertyChangedEvent.Property)
	{
		auto PropertyName = Property->GetFName();
		//if (PropertyName == GET_MEMBER_NAME_CHECKED(ULGUICanvas, blendDepth))
		//{
		//	blendDepth = -blendDepth;
		//	this->SetBlendDepth(-blendDepth);
		//}
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
					FString errMsg = FString::Printf(TEXT("[ULGUICanvas/CheckMaterials]Assign material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
					UE_LOG(LGUI, Error, TEXT("%s"), *errMsg);
#if WITH_EDITOR
					LGUIUtils::EditorNotification(FText::FromString(errMsg), 10);
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
	if (RootCanvas.IsValid())
	{
		RootCanvas->bNeedToSortRenderPriority = true;
	}
	MarkCanvasUpdate();
	this->bIsLayoutChanged = true;
	this->bIsUIRenderableHierarchyChanged = true;
}
void ULGUICanvas::RemoveUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	//remove drawcall from list
	if (InUIRenderable->drawcall.IsValid())
	{
		switch (InUIRenderable->GetUIRenderableType())
		{
		case EUIRenderableType::UIBatchGeometryRenderable:
		{
			auto UIBatchGeometryRenderable = (UUIBatchGeometryRenderable*)InUIRenderable;
			auto Drawcall = UIBatchGeometryRenderable->drawcall;
			auto index = Drawcall->renderObjectList.IndexOfByKey(UIBatchGeometryRenderable);
#if !UE_BUILD_SHIPPING
			check(index != INDEX_NONE);
#endif
			Drawcall->renderObjectList.RemoveAt(index);
			Drawcall->needToRebuildMesh = true;
			if (Drawcall->renderObjectList.Num() == 0)
			{
				if (Drawcall->drawcallMeshSection.IsValid())
				{
					Drawcall->drawcallMesh->DeleteMeshSection(Drawcall->drawcallMeshSection.Pin());
					Drawcall->drawcallMeshSection.Reset();
					Drawcall->drawcallMesh.Reset();
				}
				if (Drawcall->materialInstanceDynamic.IsValid())
				{
					Drawcall->manageCanvas->AddUIMaterialToPool(Drawcall->materialInstanceDynamic.Get());
					Drawcall->materialInstanceDynamic.Reset();
				}
//#if !UE_BUILD_SHIPPING
//				check(UIDrawcallList.Contains(Drawcall));
//#endif
				UIDrawcallList.Remove(Drawcall);
			}
		}
		break;
		case EUIRenderableType::UIPostProcessRenderable:
		case EUIRenderableType::UIDirectMeshRenderable:
		{
			auto Drawcall = InUIRenderable->drawcall;
//#if !UE_BUILD_SHIPPING
//			check(UIDrawcallList.Contains(Drawcall));
//#endif
			UIDrawcallList.Remove(Drawcall);
			if (Drawcall->drawcallMeshSection.IsValid())
			{
				Drawcall->drawcallMesh->DeleteMeshSection(Drawcall->drawcallMeshSection.Pin());
				Drawcall->drawcallMeshSection.Reset();
			}
			if (Drawcall->postProcessRenderableObject.IsValid())
			{
				if (Drawcall->postProcessRenderableObject->IsRenderProxyValid())
				{
					Drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
				}
			}
			if (Drawcall->directMeshRenderableObject.IsValid())
			{
				Drawcall->directMeshRenderableObject->ClearMeshData();
			}
		}
		break;
		}
		InUIRenderable->drawcall = nullptr;
	}

	UIRenderableList.Remove(InUIRenderable);
	if (RootCanvas.IsValid())
	{
		RootCanvas->bNeedToSortRenderPriority = true;
	}
	MarkCanvasUpdate();
	this->bIsLayoutChanged = true;
	this->bIsUIRenderableHierarchyChanged = true;
}

bool ULGUICanvas::Is2DUITransform(const FTransform& Transform)
{
#if WITH_EDITOR
	float threshold = ULGUISettings::GetOrderManagementThreshold();
#else
	static float threshold = ULGUISettings::GetOrderManagementThreshold();
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

DECLARE_CYCLE_STAT(TEXT("Canvas DrawcallBatch"), STAT_DrawcallBatch, STATGROUP_LGUI);
void ULGUICanvas::UpdateDrawcall_Implement(TArray<TSharedPtr<UUIDrawcall>>& InUIDrawcallList, TArray<TSharedPtr<UUIDrawcall>>& InCacheUIDrawcallList, bool& OutNeedToSortRenderPriority)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawcallBatch);

	auto IntersectBounds = [](FVector2D aMin, FVector2D aMax, FVector2D bMin, FVector2D bMax) {
		return !(bMin.X >= aMax.X
			|| bMax.X <= aMin.X
			|| bMax.Y <= aMin.Y
			|| bMin.Y >= aMax.Y
			);
	};
	auto OverlapWithOtherDrawcall = [IntersectBounds](const FLGUICacheTransformContainer& ItemToCanvasTf, TSharedPtr<UUIDrawcall> DrawcallItem) {
		switch (DrawcallItem->type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			//compare drawcall item's bounds
			for (auto otherItem : DrawcallItem->renderObjectList)
			{
				FLGUICacheTransformContainer OtherItemToCanvasTf;
				otherItem->GetRenderCanvas()->GetCacheUIItemToCanvasTransform(otherItem.Get(), true, OtherItemToCanvasTf);
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

	auto CanFitInDrawcall = [&](UUIBatchGeometryRenderable* InUIItem, const FLGUICacheTransformContainer& InUIItemToCanvasTf, int32& OutDrawcallIndexToFitin)
	{
		for (int i = InUIDrawcallList.Num() - 1; i >= FitInDrawcallMinIndex; i--)//from tail to head
		{
			auto DrawcallItem = InUIDrawcallList[i];
			if (!DrawcallItem->bIs2DSpace)//drawcall is 3d, can't batch
			{
				return false;
			}

			if (DrawcallItem->material.IsValid()
				|| DrawcallItem->type != EUIDrawcallType::BatchGeometry
				|| DrawcallItem->texture != InUIItem->GetGeometry()->texture
				)//can't fit in this drawcall, should check overlap
			{
				if (OverlapWithOtherDrawcall(InUIItemToCanvasTf, DrawcallItem))//overlap with other drawcall, can't batch
				{
					return false;
				}
				continue;//keep searching
			}
			OutDrawcallIndexToFitin = i;
			return true;//can fit in drawcall
		}
		return false;
	};
	auto PushSingleDrawcall = [&](UUIItem* InUIItem, TSharedPtr<UIGeometry> InItemGeo, bool InIs2DSpace, EUIDrawcallType InDrawcallType) {
		TSharedPtr<UUIDrawcall> DrawcallItem = nullptr;
		auto FoundDrawcallIndex = InCacheUIDrawcallList.IndexOfByPredicate([=](const TSharedPtr<UUIDrawcall>& DrawcallItem) {
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
		if (FoundDrawcallIndex != INDEX_NONE)
		{
			DrawcallItem = InCacheUIDrawcallList[FoundDrawcallIndex];
			InCacheUIDrawcallList.RemoveAt(FoundDrawcallIndex);//cannot use "RemoveAtSwap" here, because we need the right order to tell if we should sort render order, see "bNeedToSortRenderPriority"
			DrawcallItem->bIs2DSpace = InIs2DSpace;
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
			
			DrawcallItem->bIs2DSpace = InIs2DSpace;
		}
		if (InDrawcallType == EUIDrawcallType::BatchGeometry
			|| InDrawcallType == EUIDrawcallType::PostProcess
			|| InDrawcallType == EUIDrawcallType::DirectMesh)
		{
			((UUIBaseRenderable*)InUIItem)->drawcall = DrawcallItem;
		}
		DrawcallItem->manageCanvas = this;
		InUIDrawcallList.Add(DrawcallItem);

		if (FoundDrawcallIndex != 0)//if not find drawcall no found drawcall not at head of array, means drawcall list's order is changed compare to cache list, then we need to sort render order
		{
			OutNeedToSortRenderPriority = true;
		}
	};
	auto ClearDrawcallInList = [&](TSharedPtr<UUIDrawcall> InDrawcallItem, TArray<TSharedPtr<UUIDrawcall>>& InDrawcallListToCheck, UUIBatchGeometryRenderable* InUIBatchGeometryRenderable) {
		auto FoundIndex = InDrawcallListToCheck.IndexOfByPredicate([=](const TSharedPtr<UUIDrawcall>& Item) {
			return Item == InDrawcallItem;
			});
		if (FoundIndex != INDEX_NONE)
		{
			auto DrawcallItem = InDrawcallListToCheck[FoundIndex];
			if (DrawcallItem->drawcallMesh.IsValid())
			{
				if (DrawcallItem->drawcallMeshSection.IsValid())//drawcallMeshSection could be deleted by other item when "ClearDrawcallInList"
				{
					DrawcallItem->drawcallMesh->DeleteMeshSection(DrawcallItem->drawcallMeshSection.Pin());
					DrawcallItem->drawcallMeshSection.Reset();
				}
				DrawcallItem->drawcallMesh.Reset();
			}
			if (DrawcallItem->materialInstanceDynamic.IsValid())
			{
				if (DrawcallItem->manageCanvas.IsValid())//manageCanvas could be destroyed before UpdateDrawcall
				{
					DrawcallItem->manageCanvas->AddUIMaterialToPool(DrawcallItem->materialInstanceDynamic.Get());
				}
				DrawcallItem->materialInstanceDynamic.Reset();
			}
			DrawcallItem->needToRebuildMesh = true;
			DrawcallItem->renderObjectList.Remove(InUIBatchGeometryRenderable);
			InUIBatchGeometryRenderable->drawcall = nullptr;
			return true;
		}
		return false;
	};

	//hierarchy change, need to sort it
	if (bIsUIRenderableHierarchyChanged)
	{
		bIsUIRenderableHierarchyChanged = false;
		UIRenderableList.Sort([](const UUIItem& A, const UUIItem& B) {
			return A.GetFlattenHierarchyIndex() < B.GetFlattenHierarchyIndex();
			});
	}
	//for sorted ui items, iterate from head to tail, compare drawcall from tail to head
	for (int i = 0; i < UIRenderableList.Num(); i++)
	{
		if (UIRenderableList[i]->IsCanvasUIItem() && UIRenderableList[i]->GetRenderCanvas() != this)//is child canvas
		{
			auto ChildRenderCanvas = UIRenderableList[i]->GetRenderCanvas();
			if (ChildRenderCanvas->IsRenderByOtherCanvas())
			{
				ChildRenderCanvas->UpdateDrawcall_Implement(InUIDrawcallList, InCacheUIDrawcallList, OutNeedToSortRenderPriority);
				FitInDrawcallMinIndex = InUIDrawcallList.Num();//after children drawcall, update this index so parent's UI element will not batch into children's drawcall
			}
		}
		else
		{
			auto UIRenderableItem = (UUIBaseRenderable*)(UIRenderableList[i]);
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

				if (!ItemGeo->material.IsValid()//consider every custom material as a drawcall
					&& !is2DUIItem//3d UI can't batch
					)
				{
					auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
					if (UIBatchGeometryRenderableItem->drawcall.IsValid())//maybe exist in other drawcall, should check that drawcall
					{
						//find the drawcall in list and remove from it, clear the mesh and material
						if (!ClearDrawcallInList(UIBatchGeometryRenderableItem->drawcall, InUIDrawcallList, UIBatchGeometryRenderableItem))
						{
							ClearDrawcallInList(UIBatchGeometryRenderableItem->drawcall, InCacheUIDrawcallList, UIBatchGeometryRenderableItem);
						}
					}
					//make a individural drawacll
					PushSingleDrawcall(UIBatchGeometryRenderableItem, ItemGeo, is2DUIItem, EUIDrawcallType::BatchGeometry);
					//copy update state from old to new
					if (OldDrawcall.IsValid())
					{
						OldDrawcall->CopyUpdateState(UIBatchGeometryRenderableItem->drawcall.Get());
					}
				}
				else//batch elements into drawcall
				{
					int DrawcallIndexToFitin;
					if (CanFitInDrawcall(UIBatchGeometryRenderableItem, UIItemToCanvasTf, DrawcallIndexToFitin))
					{
						auto DrawcallItem = InUIDrawcallList[DrawcallIndexToFitin];
						if (UIBatchGeometryRenderableItem->drawcall == DrawcallItem)//already exist in this drawcall (added in prev render frame)
						{
							if (UIBatchGeometryRenderableItem->GetFlatternHierarchyIndexChangeAtThisRenderFrame())
							{
								DrawcallItem->bShouldSortRenderObjectList = true;//mark for sort the list after drawcall creation
								DrawcallItem->needToUpdateVertex = true;//after sort the list, we should update vertex data too
							}
						}
						else//not exist in this drawcall
						{
							auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
							if (UIBatchGeometryRenderableItem->drawcall.IsValid())//maybe exist in other drawcall, should check that drawcall
							{
								//find the drawcall in list and remove from it, clear the mesh and material
								if (!ClearDrawcallInList(UIBatchGeometryRenderableItem->drawcall, InUIDrawcallList, UIBatchGeometryRenderableItem))
								{
									ClearDrawcallInList(UIBatchGeometryRenderableItem->drawcall, InCacheUIDrawcallList, UIBatchGeometryRenderableItem);
								}
							}
							//add to this drawcall
							DrawcallItem->renderObjectList.Add(UIBatchGeometryRenderableItem);
							DrawcallItem->needToRebuildMesh = true;
							UIBatchGeometryRenderableItem->drawcall = DrawcallItem;
							//copy update state from old to new
							if (OldDrawcall.IsValid())
							{
								OldDrawcall->CopyUpdateState(UIBatchGeometryRenderableItem->drawcall.Get());
							}
						}
					}
					else
					{
						auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
						if (UIBatchGeometryRenderableItem->drawcall.IsValid())//maybe exist in other drawcall, should check that drawcall
						{
							//find the drawcall in list and remove from it, clear the mesh and material
							ClearDrawcallInList(UIBatchGeometryRenderableItem->drawcall, InUIDrawcallList, UIBatchGeometryRenderableItem);
							//ClearDrawcallInList(UIBatchGeometryRenderableItem->drawcall, InCacheUIDrawcallList, UIBatchGeometryRenderableItem);//no need to clear from cache list, because it is a individural drawcall
						}
						//make a individural drawacll
						PushSingleDrawcall(UIBatchGeometryRenderableItem, ItemGeo, is2DUIItem, EUIDrawcallType::BatchGeometry);
						//copy update state from old to new
						if (OldDrawcall.IsValid())
						{
							OldDrawcall->CopyUpdateState(UIBatchGeometryRenderableItem->drawcall.Get());
						}
					}
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
				PushSingleDrawcall(UIRenderableItem, ItemGeo, is2DUIItem, EUIDrawcallType::PostProcess);
				//no need to copy drawcall's update data for UIPostProcessRenderable, because UIPostProcessRenderable's drawcall should be the same as previours one
			}
			break;
			case EUIRenderableType::UIDirectMeshRenderable:
			{
				auto UIDirectMeshRenderableItem = (UUIDirectMeshRenderable*)UIRenderableItem;
				//every direct mesh is a drawcall
				PushSingleDrawcall(UIRenderableItem, nullptr, is2DUIItem, EUIDrawcallType::BatchGeometry);
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

void ULGUICanvas::UpdateCanvasLayout(bool layoutChanged)
{
	if (!bIsLayoutChanged) bIsLayoutChanged = layoutChanged;
	cacheForThisUpdate_ClipTypeChanged = bClipTypeChanged;
	cacheForThisUpdate_RectClipParameterChanged = bRectClipParameterChanged || layoutChanged;
	if (bRectRangeCalculated)
	{
		if (cacheForThisUpdate_RectClipParameterChanged)bRectRangeCalculated = false;
	}
	cacheForThisUpdate_TextureClipParameterChanged = bTextureClipParameterChanged;
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;
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
		}
		//clear mesh
		PooledUIMeshList.Reset();
		UsingUIMeshList.Reset();
		MarkCanvasUpdate();
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

	if (this->IsRenderByOtherCanvas())
	{
		goto FRAME_COMPLETE;//if is render by other canvas
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

	//reset transform map, because transform change
	CacheUIItemToCanvasTransformMap.Reset();
	//update drawcall
	if (bIsLayoutChanged || bIsUIRenderableHierarchyChanged)
	{
		bIsLayoutChanged = false;
#if !UE_BUILD_SHIPPING
		check(!this->IsRenderByOtherCanvas());
#endif

		//store prev created drawcall to cache list, so when we create drawcall, we can search in the cache list and use existing one
		for (auto Item : UIDrawcallList)
		{
			CacheUIDrawcallList.Add(Item);
		}
		UIDrawcallList.Reset();

		bool bOutNeedToSortRenderPriority = bNeedToSortRenderPriority;
		UpdateDrawcall_Implement(UIDrawcallList, CacheUIDrawcallList
			, bOutNeedToSortRenderPriority//cannot pass a uint32:1 here, so use a temp bool
		);
		bNeedToSortRenderPriority = bOutNeedToSortRenderPriority;

		//for not used drawcalls, clear data
		for (int i = 0; i < CacheUIDrawcallList.Num(); i++)
		{
			auto DrawcallInCache = CacheUIDrawcallList[i];
			if (DrawcallInCache->drawcallMesh.IsValid())
			{
				if (DrawcallInCache->drawcallMeshSection.IsValid())//drawcallMeshSection could be deleted when "ClearDrawcallInList"
				{
					DrawcallInCache->drawcallMesh->DeleteMeshSection(DrawcallInCache->drawcallMeshSection.Pin());
					DrawcallInCache->drawcallMeshSection.Reset();
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

	//update drawcall mesh
	UpdateDrawcallMesh_Implement();

	//update drawcall material
	UpdateDrawcallMaterial_Implement();

	if (this == RootCanvas)//child canvas is already updated before this, so after all update, the topmost canvas should start the sort function
	{
		if (bNeedToSortRenderPriority)
		{
			bNeedToSortRenderPriority = false;
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					switch (this->GetRenderMode())
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
					switch (this->GetRenderMode())
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

	FRAME_COMPLETE:
	//this render frame is complete
	bRectRangeCalculated = false;//@todo: why make this to false?
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
			if (DrawcallItem->bShouldSortRenderObjectList)
			{
				DrawcallItem->bShouldSortRenderObjectList = false;
				DrawcallItem->renderObjectList.Sort([](const TWeakObjectPtr<UUIBatchGeometryRenderable>& A, const TWeakObjectPtr<UUIBatchGeometryRenderable>& B) {
					return A->GetFlattenHierarchyIndex() < B->GetFlattenHierarchyIndex();
					});
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
				DrawcallItem->postProcessRenderableObject->GetRenderProxy();
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
		return !this->GetOverrideSorting();
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
	InUIMesh->ClearAllMeshSection();
	PooledUIMeshList.Add(InUIMesh);
}

int32 ULGUICanvas::SortDrawcall(int32 InStartRenderPriority)//@todo: cleanup this code, looks confusion
{
	ULGUIMeshComponent* prevUIMesh = nullptr;
	int drawcallIndex = 0;
	int meshOrPostProcessCount = 0;
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
				meshOrPostProcessCount++;
				if (prevUIMesh != nullptr)//when get difference mesh, we sort last mesh, because we need to wait until mesh's all section have set render proirity in "SetMeshSectionRenderPriority"
				{
					prevUIMesh->SortMeshSectionRenderPriority();
					drawcallIndex = 0;
				}
				if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace)
				{
					DrawcallItem->drawcallMesh->SetUITranslucentSortPriority(this->GetActualSortOrder());
				}
				else
				{
					DrawcallItem->drawcallMesh->SetUITranslucentSortPriority(InStartRenderPriority++);
				}
				prevUIMesh = DrawcallItem->drawcallMesh.Get();
			}
			DrawcallItem->drawcallMesh->SetMeshSectionRenderPriority(DrawcallItem->drawcallMeshSection.Pin(), drawcallIndex++);
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			meshOrPostProcessCount++;
			DrawcallItem->postProcessRenderableObject->GetRenderProxy()->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
		break;
		}
	}
	if (prevUIMesh != nullptr)//since "SortMeshSectionRenderPriority" action is called when get different mesh, so the last one won't call, so do it here
	{
		prevUIMesh->SortMeshSectionRenderPriority();
	}
	return meshOrPostProcessCount;
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
			if (!UIMat.IsValid() || DrawcallItem->materialChanged || DrawcallItem->manageCanvas->cacheForThisUpdate_ClipTypeChanged)
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
				bNeedToSetClipParameter = true;
			}
			else if (DrawcallItem->textureChanged)
			{
				UIMat->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, DrawcallItem->texture.Get());
				DrawcallItem->textureChanged = false;
			}
		}
		else if (DrawcallItem->type == EUIDrawcallType::PostProcess)
		{
			if (DrawcallItem->manageCanvas->cacheForThisUpdate_ClipTypeChanged
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
			if (DrawcallItem->manageCanvas->cacheForThisUpdate_ClipTypeChanged
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
				//|| DrawcallItem->manageCanvas->cacheForThisUpdate_ClipTypeChanged//this is already tested when check "needToSetClipParameter"
				|| DrawcallItem->manageCanvas->cacheForThisUpdate_RectClipParameterChanged)
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
				//|| DrawcallItem->manageCanvas->cacheForThisUpdate_ClipTypeChanged//this is already tested when check "needToSetClipParameter"
				|| DrawcallItem->manageCanvas->cacheForThisUpdate_TextureClipParameterChanged)
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
		PooledUIMaterialList.Add({});
		PooledUIMaterialList.Add({});
		PooledUIMaterialList.Add({});
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
		CalculateRectRange();
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
	case ELGUICanvasClipType::Texture:
		return true;
		break;
	}

	return true;
}
void ULGUICanvas::CalculateRectRange()
{
	if (bRectRangeCalculated == false)//if not calculated yet
	{
		if (this->GetActualClipType() == ELGUICanvasClipType::Rect)
		{
			if (this->GetOverrideClipType())//override clip parameter
			{
				auto& widget = UIItem->widget;
				//calculate sefl rect range
				clipRectMin.X = -widget.pivot.X * widget.width;
				clipRectMin.Y = -widget.pivot.Y * widget.height;
				clipRectMax.X = (1.0f - widget.pivot.X) * widget.width;
				clipRectMax.Y = (1.0f - widget.pivot.Y) * widget.height;
				//add offset
				clipRectMin.X = clipRectMin.X - clipRectOffset.Left;
				clipRectMax.X = clipRectMax.X + clipRectOffset.Right;
				clipRectMin.Y = clipRectMin.Y - clipRectOffset.Bottom;
				clipRectMax.Y = clipRectMax.Y + clipRectOffset.Top;
				//calculate parent rect range
				if (inheritRectClip && ParentCanvas.IsValid() && ParentCanvas->GetActualClipType() == ELGUICanvasClipType::Rect)
				{
					ParentCanvas->CalculateRectRange();
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
					ParentCanvas->CalculateRectRange();
					auto parentRectMin = FVector(ParentCanvas->clipRectMin, 0);
					auto parentRectMax = FVector(ParentCanvas->clipRectMax, 0);
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
					auto& widget = UIItem->widget;
					//calculate sefl rect range
					clipRectMin.X = -widget.pivot.X * widget.width;
					clipRectMin.Y = -widget.pivot.Y * widget.height;
					clipRectMax.X = (1.0f - widget.pivot.X) * widget.width;
					clipRectMax.Y = (1.0f - widget.pivot.Y) * widget.height;
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
			auto& widget = UIItem->widget;
			//calculate sefl rect range
			clipRectMin.X = -widget.pivot.X * widget.width;
			clipRectMin.Y = -widget.pivot.Y * widget.height;
			clipRectMax.X = (1.0f - widget.pivot.X) * widget.width;
			clipRectMax.Y = (1.0f - widget.pivot.Y) * widget.height;
		}

		bRectRangeCalculated = true;
	}
}


FLinearColor ULGUICanvas::GetRectClipOffsetAndSize()
{
	CalculateRectRange();
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
	auto& widget = UIItem->widget;
	auto Offset = FVector2D(widget.width * -widget.pivot.X, widget.height * -widget.pivot.Y);
	//if render by other canvas, we should transform offset to that canvas space
	if (this->IsRenderByOtherCanvas())
	{
		auto RenderCanvas = this->GetActualRenderCanvas();
		auto RenderCanvasTfInv = RenderCanvas->UIItem->GetComponentTransform().Inverse();
		auto ThisTf = this->UIItem->GetComponentTransform();
		auto OffsetPoint = RenderCanvasTfInv.TransformPosition(ThisTf.TransformPosition(FVector(0, Offset.X, Offset.Y)));
		Offset = FVector2D(OffsetPoint.Y, OffsetPoint.Z);
	}
	return FLinearColor(Offset.X, Offset.Y, widget.width, widget.height);
}

void ULGUICanvas::SetClipType(ELGUICanvasClipType newClipType) 
{
	if (clipType != newClipType)
	{
		bClipTypeChanged = true;
		clipType = newClipType;
		MarkCanvasUpdate();
	}
}
void ULGUICanvas::SetRectClipFeather(FVector2D newFeather) 
{
	if (clipFeather != newFeather)
	{
		bRectClipParameterChanged = true;
		clipFeather = newFeather;
		MarkCanvasUpdate();
	}
}
void ULGUICanvas::SetRectClipOffset(FMargin newOffset)
{
	if (clipRectOffset != newOffset)
	{
		bRectClipParameterChanged = true;
		bRectRangeCalculated = false;
		MarkCanvasUpdate();
	}
}
void ULGUICanvas::SetClipTexture(UTexture* newTexture) 
{
	if (clipTexture != newTexture)
	{
		bTextureClipParameterChanged = true;
		clipTexture = newTexture;
		MarkCanvasUpdate();
	}
}
void ULGUICanvas::SetInheriRectClip(bool newBool)
{
	if (inheritRectClip != newBool)
	{
		inheritRectClip = newBool;
		bRectRangeCalculated = false;
		MarkCanvasUpdate();
	}
}

void ULGUICanvas::SetSortOrderAdditionalValueRecursive(int32 InAdditionalValue)
{
	if (FMath::Abs(this->SortOrder + InAdditionalValue) > MAX_int16)
	{
		auto errorMsg = FString::Printf(TEXT("[ULGUICanvas::SetSortOrder] SortOrder out of range!\nNOTE! SortOrder value is stored with int16 type, so valid range is -32768 to 32767"));
		LGUIUtils::EditorNotification(FText::FromString(errorMsg));
		return;
	}

	this->SortOrder += InAdditionalValue;
	for (auto ChildCanvas : ChildrenCanvasArray)
	{
		ChildCanvas->SetSortOrderAdditionalValueRecursive(InAdditionalValue);
	}
}

void ULGUICanvas::SetSortOrder(int32 InSortOrder, bool InPropagateToChildrenCanvas)
{
	if (SortOrder != InSortOrder)
	{
		MarkCanvasUpdate();
		if (InPropagateToChildrenCanvas)
		{
			int32 Diff = InSortOrder - SortOrder;
			SetSortOrderAdditionalValueRecursive(Diff);
		}
		else
		{
			if (FMath::Abs(InSortOrder) > MAX_int16)
			{
				auto errorMsg = FString::Printf(TEXT("[ULGUICanvas::SetSortOrder] SortOrder out of range!\nNOTE! SortOrder value is stored with int16 type, so valid range is -32768 to 32767"));
				LGUIUtils::EditorNotification(FText::FromString(errorMsg));
				InSortOrder = FMath::Clamp(InSortOrder, (int32)MIN_int16, (int32)MAX_int16);
			}
			this->SortOrder = InSortOrder;
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
					ViewExtension->SetRenderCanvasSortOrder(this, this->SortOrder);
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
	MarkCanvasUpdate();
}

void ULGUICanvas::SetDynamicPixelsPerUnit(float newValue)
{
	if (dynamicPixelsPerUnit != newValue)
	{
		dynamicPixelsPerUnit = newValue;
		for (int i = 0; i < UIDrawcallList.Num(); i++)
		{
			auto DrawcallItem = UIDrawcallList[i];
			if (DrawcallItem->type == EUIDrawcallType::BatchGeometry)
			{
				DrawcallItem->textureChanged = true;
			}
		}
		MarkCanvasUpdate();
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
		return SortOrder;
	}
	else
	{
		if (bOverrideSorting)
		{
			return SortOrder;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualSortOrder();
			}
		}
	}
	return SortOrder;
}

void ULGUICanvas::SetOverrideSorting(bool value)
{
	if (bOverrideSorting != value)
	{
		bOverrideSorting = value;
		MarkCanvasUpdate();
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
		FMatrix ViewRotationMatrix = FInverseRotationMatrix(GetViewRotator());
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
FMatrix ULGUICanvas::GetViewRotationMatrix()const
{
	return FRotationMatrix(this->GetViewRotator());
}
FRotator ULGUICanvas::GetViewRotator()const
{
	if (bOverrideViewRotation)
		return OverrideViewRotation;

	return UIItem->GetComponentRotation();
}
FIntPoint ULGUICanvas::GetViewportSize()const
{
	FIntPoint viewportSize = FIntPoint(2, 2);
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (CheckUIItem())
			{
				viewportSize.X = UIItem->GetWidth();
				viewportSize.Y = UIItem->GetHeight();
			}
		}
		else
#endif
		{
			if (renderMode == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				if (auto pc = world->GetFirstPlayerController())
				{
					pc->GetViewportSize(viewportSize.X, viewportSize.Y);
				}
			}
			else if (renderMode == ELGUIRenderMode::RenderTarget && IsValid(renderTarget))
			{
				viewportSize.X = renderTarget->SizeX;
				viewportSize.Y = renderTarget->SizeY;
			}
		}
	}
	return viewportSize;
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
		MarkCanvasUpdate();
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


bool ULGUICanvas::GetCacheUIItemToCanvasTransform(UUIItem* item, bool createIfNotExist, FLGUICacheTransformContainer& outResult)
{
	if (this->IsRenderByOtherCanvas())
	{
		return this->GetActualRenderCanvas()->GetCacheUIItemToCanvasTransform(item, createIfNotExist, outResult);
	}
	else
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
void ULGUICanvas::CalculateUIItem2DBounds(UUIItem* item, const FTransform2D& transform, FVector2D& min, FVector2D& max)
{
	FVector2D localPoint1, localPoint2;
	item->GetLocalSpaceMinMaxPoint(localPoint1, localPoint2);
	auto point1 = transform.TransformPoint(localPoint1);
	auto point2 = transform.TransformPoint(localPoint2);
	auto point3 = transform.TransformPoint(FVector2D(localPoint2.X, localPoint1.Y));
	auto point4 = transform.TransformPoint(FVector2D(localPoint1.X, localPoint2.Y));

	GetMinMax(point1.X, point2.X, point3.X, point4.X, min.X, max.X);
	GetMinMax(point1.Y, point2.Y, point3.Y, point4.Y, min.Y, max.Y);
}

PRAGMA_ENABLE_OPTIMIZATION