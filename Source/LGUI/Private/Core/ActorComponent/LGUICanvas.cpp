// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Utils/LGUIUtils.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#endif
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
#include "Core/UIPostProcessRenderProxy.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Math/TransformCalculus2D.h"
#include "Core/LGUICanvasCustomClip.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "LGUICanvas"

ULGUICanvas::ULGUICanvas()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bHasAddToLGUIManager = false;
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bNeedToUpdateCustomClipParameter = true;
	bRectRangeCalculated = false;

	bHasAddToLGUIScreenSpaceRenderer = false;
	bHasSetIntialStateforLGUIWorldSpaceRenderer = false;
	bOverrideViewLocation = false;
	bOverrideViewRotation = false;
	bOverrideProjectionMatrix = false;
	bOverrideFovAngle = false;
	bPrevUIItemIsActive = true;

	bCanTickUpdate = true;
	bShouldRebuildDrawcall = true;
	bShouldSortRenderableOrder = true;
	bAnythingChangedForRenderTarget = true;

	bIsViewProjectionMatrixDirty = true;

	DefaultMeshType = ULGUIMeshComponent::StaticClass();
}

void ULGUICanvas::BeginPlay()
{
	Super::BeginPlay();
	CheckRootCanvas();
	CurrentRenderMode = this->GetActualRenderMode();
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
	bNeedToUpdateCustomClipParameter = true;
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
	CheckRootCanvas();
	if (this == RootCanvas)
	{
		bool bIsRenderTargetRenderer = false;
		if (RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode))
		{
			auto ActualRenderMode = GetActualRenderMode();
#if WITH_EDITOR
			if (previewWithLGUIRenderer)
			{
				if (!GetWorld()->IsGameWorld())//edit mode
				{
					if (ActualRenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
						ActualRenderMode = ELGUIRenderMode::WorldSpace_LGUI;
				}
			}
#endif
			switch (ActualRenderMode)
			{
			case ELGUIRenderMode::ScreenSpaceOverlay:
			{
				if (!bHasAddToLGUIScreenSpaceRenderer)
				{
					auto ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), true);

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
						RenderTargetViewExtension->SetRenderToRenderTarget(true);
						bHasAddToLGUIScreenSpaceRenderer = true;
					}
				}
				bIsRenderTargetRenderer = true;
			}
			break;
			case ELGUIRenderMode::WorldSpace_LGUI:
			{
				if (!bHasSetIntialStateforLGUIWorldSpaceRenderer)
				{
					auto ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), true);

					if (ViewExtension.IsValid())//only root canvas can add screen space UI to LGUIRenderer
					{
						//put initial code here
						bHasSetIntialStateforLGUIWorldSpaceRenderer = true;
					}
				}
			}
			break;
			}
		}
		
		if (CheckUIItem())
		{
			if (UpdateCanvasDrawcallRecursive())
			{
				MarkFinishRenderFrameRecursive();
			}
		}

		if (bIsRenderTargetRenderer
			&& bAnythingChangedForRenderTarget
			)
		{
			bAnythingChangedForRenderTarget = false;
			if (RenderTargetViewExtension.IsValid() && IsValid(renderTarget))
			{
				RenderTargetViewExtension->UpdateRenderTargetRenderer(renderTarget);
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

	for (const auto& DrawcallItem : UIDrawcallList)
	{
		switch (DrawcallItem->Type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			for (int i = 0; i < DrawcallItem->RenderObjectList.Num(); i++)
			{
				if (!DrawcallItem->RenderObjectList[i].IsValid())
				{
					DrawcallItem->RenderObjectList.RemoveAt(i);
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
		if (!bHasAddToLGUIManager)
		{
			bHasAddToLGUIManager = true;
			ALGUIManagerActor::AddCanvas(this);
		}
		//tell UIItem
		UIItem->RegisterRenderCanvas(this);
		UIHierarchyChangedDelegateHandle = UIItem->RegisterUIHierarchyChanged(FSimpleDelegate::CreateUObject(this, &ULGUICanvas::OnUIHierarchyChanged));
		UIActiveStateChangedDelegateHandle = UIItem->RegisterUIActiveStateChanged(FUIItemActiveInHierarchyStateChangedDelegate::CreateUObject(this, &ULGUICanvas::OnUIActiveStateChanged));

		OnUIHierarchyChanged();
	}
}
void ULGUICanvas::OnUnregister()
{
	Super::OnUnregister();
	if (bHasAddToLGUIManager)
	{
		bHasAddToLGUIManager = false;
		ALGUIManagerActor::RemoveCanvas(this);
	}

	//clear
	ClearDrawcall();

	//remove from parent canvas
	if (ParentCanvas.IsValid())
	{
		SetParentCanvas(nullptr);
	}

	if (this == RootCanvas)
	{
		for (auto& ChildCanvas : ChildrenCanvasArray)
		{
			if (ChildCanvas.IsValid())
			{
				ChildCanvas->RootCanvas = nullptr;//force ChildCanvas to find new RootCanvas
			}
		}
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
	for (const auto& UIMeshItem : PooledUIMeshList)
	{
		if (UIMeshItem.IsValid())
		{
			UIMeshItem->ClearAllMeshSection();
			UIMeshItem->DestroyComponent();
		}
	}
	PooledUIMeshList.Empty();
	for (const auto& UIMeshItem : UsingUIMeshList)
	{
		if (UIMeshItem.IsValid())
		{
			UIMeshItem->DestroyComponent();
		}
	}
	UsingUIMeshList.Empty();

	PooledUIMaterialList.Empty();
	UIDrawcallList.Empty();
	CacheUIDrawcallList.Empty();
}

void ULGUICanvas::RemoveFromViewExtension()
{
	if (bHasAddToLGUIScreenSpaceRenderer)
	{
		bHasAddToLGUIScreenSpaceRenderer = false;
		auto ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
		if (ViewExtension.IsValid())
		{
			ViewExtension->ClearScreenSpaceRenderCanvas();
		}
	}
	if (bHasSetIntialStateforLGUIWorldSpaceRenderer)
	{
		bHasSetIntialStateforLGUIWorldSpaceRenderer = false;
	}

	if (RenderTargetViewExtension.IsValid())
	{
		RenderTargetViewExtension->SetRenderToRenderTarget(false);
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
			ParentCanvas->UIRenderableList.AddUnique(this->UIItem.Get());
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

	const auto OldRenderMode = CurrentRenderMode;
	if (this->IsRegistered())
	{
		if (CheckRootCanvas())
		{
			CurrentRenderMode = RootCanvas->GetRenderMode();
		}
		else
		{
			CurrentRenderMode = ELGUIRenderMode::None;
		}
	}
	else
	{
		CurrentRenderMode = ELGUIRenderMode::None;
	}
	//if render space changed, we need to change recreate all render data
	if (CurrentRenderMode != OldRenderMode)
	{
		if (CheckUIItem())
		{
			UIItem->MarkRenderModeChangeRecursive(this, OldRenderMode, CurrentRenderMode);
		}
		//clear drawcall, delete mesh, because UE/LGUI render's mesh data not compatible
		this->ClearDrawcall();

		OnRenderModeChanged.Broadcast(this, OldRenderMode, CurrentRenderMode);
	}

	for (const auto& ChildCanvas : ChildrenCanvasArray)
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
	bNeedToUpdateCustomClipParameter = true;

	ULGUICanvas* NewParentCanvas = nullptr;
	if (this->IsRegistered())
	{
		NewParentCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(this->GetOwner()->GetAttachParentActor(), true);
	}
	SetParentCanvas(NewParentCanvas);
}

void ULGUICanvas::OnUIActiveStateChanged(bool value)
{
	if (value)
	{
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->UIRenderableList.AddUnique(this->UIItem.Get());
			ParentCanvas->MarkCanvasUpdate(false, false, true//why make this to true? becase we need to sort UIRenderableList, and set bShouldSortRenderableOrder to true can do it
				, true);

		}
	}
	else
	{
		if (ParentCanvas.IsValid())
		{
			ParentCanvas->UIRenderableList.Remove(this->UIItem.Get());
			ParentCanvas->MarkCanvasUpdate(false, false, false, true);
		}
	}
}

bool ULGUICanvas::IsRenderToScreenSpace()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay;
	}
	return false;
}
bool ULGUICanvas::IsRenderToRenderTarget()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::RenderTarget;
	}
	return false;
}
bool ULGUICanvas::IsRenderToWorldSpace()const
{
	if (CheckRootCanvas())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::WorldSpace
			|| RootCanvas->renderMode == ELGUIRenderMode::WorldSpace_LGUI
			;
	}
	return false;
}

bool ULGUICanvas::IsRenderByLGUIRendererOrUERenderer()const
{
	if (CheckRootCanvas())
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
void ULGUICanvas::MarkItemTransformOrVertexPositionChanged(UUIBaseRenderable* InRenderable)
{
	if (CacheUIItemToCanvasTransformMap.Contains(InRenderable))
	{
		CacheUIItemToCanvasTransformMap.Remove(InRenderable);
	}
}

#if WITH_EDITOR
bool ULGUICanvas::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{

	}

	return Super::CanEditChange(InProperty);
}
void ULGUICanvas::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
	bNeedToUpdateCustomClipParameter = true;
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
void ULGUICanvas::PostEditUndo()
{
	Super::PostEditUndo();

	struct LOCAL
	{
		static void RecheckRootCanvasRecursive(ULGUICanvas* Target)
		{
			Target->RootCanvas = nullptr;
			Target->CheckRootCanvas();
			Target->MarkCanvasUpdate(true, true, true);

			for (auto childCanvas : Target->ChildrenCanvasArray)
			{
				RecheckRootCanvasRecursive(childCanvas.Get());
			}
		}
	};
	ULGUIEditorManagerObject::AddOneShotTickFunction([=]() {
		LOCAL::RecheckRootCanvasRecursive(this);
		}, 0);
}
#endif

UMaterialInterface** ULGUICanvas::GetMaterials()
{
	if(IsRootCanvas())
	{
		CheckDefaultMaterials();
	}
	else
	{
		if (GetOverrideDefaultMaterials())
		{
			CheckDefaultMaterials();
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetMaterials();
			}
			else
			{
				CheckDefaultMaterials();
			}
		}
	}
	return &DefaultMaterials[0];
}
void ULGUICanvas::CheckDefaultMaterials()
{
	for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
	{
		if (DefaultMaterials[i] == nullptr)
		{
			FString matPath;
			switch (i)
			{
			default:
			case 0: matPath = TEXT("/LGUI/Materials/LGUI_NoClip"); break;
			case 1: matPath = TEXT("/LGUI/Materials/LGUI_RectClip"); break;
			case 2: matPath = TEXT("/LGUI/Materials/LGUI_TextureClip"); break;
			}
			auto mat = LoadObject<UMaterialInterface>(NULL, *matPath);
			if (!IsValid(mat))
			{
				auto errMsg = LOCTEXT("MissingDefaultContent", "[ULGUICanvas/CheckDefaultMaterials] Load material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.");
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errMsg, 10);
#endif
				continue;
			}
			DefaultMaterials[i] = mat;
			this->MarkPackageDirty();
		}
	}
}


ULGUICanvas* ULGUICanvas::GetRootCanvas() const
{ 
	CheckRootCanvas(); 
	return RootCanvas.Get(); 
}
bool ULGUICanvas::IsRootCanvas()const
{
	return GetRootCanvas() == this;
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
				auto index = Drawcall->RenderObjectList.IndexOfByKey(UIBatchGeometryRenderable);
				if (index != INDEX_NONE)
				{
					Drawcall->RenderObjectList.RemoveAt(index);
					Drawcall->bNeedToUpdateVertex = true;
				}
			}
			break;
			case EUIRenderableType::UIPostProcessRenderable:
			{
				if (Drawcall->PostProcessRenderableObject.IsValid())
				{
					if (Drawcall->PostProcessRenderableObject->IsRenderProxyValid())
					{
						Drawcall->PostProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
					}
				}
			}
			break;
			case EUIRenderableType::UIDirectMeshRenderable:
			{
				if (Drawcall->DirectMeshRenderableObject.IsValid())
				{
					Drawcall->DirectMeshRenderableObject->ClearMeshData();
				}
			}
			break;
			}
			UIRenderableItem->drawcall = nullptr;
		}
		MarkCanvasUpdate(false, false, true);
	}
}

void ULGUICanvas::AddUIItem(UUIItem* InUIItem)
{
	UIItemList.AddUnique(InUIItem);
	MarkCanvasUpdate(false, false, false);
}
void ULGUICanvas::RemoveUIItem(UUIItem* InUIItem)
{
	UIItemList.Remove(InUIItem);
	MarkCanvasUpdate(false, false, false);
}

void ULGUICanvas::SetRequireAdditionalShaderChannels(uint8 InFlags)
{
	this->additionalShaderChannels |= InFlags;
}
void ULGUICanvas::SetActualRequireAdditionalShaderChannels(uint8 InFlags)
{	
	ULGUICanvas* TargetCanvas = this;
	while (true)
	{
		if (TargetCanvas == this->GetRootCanvas() || TargetCanvas->GetOverrideAddionalShaderChannel())
		{
			break;
		}
		else
		{
			if (TargetCanvas->GetParentCanvas().IsValid())
			{
				TargetCanvas = TargetCanvas->GetParentCanvas().Get();
			}
			else
			{
				break;
			}
		}
	}
	TargetCanvas->SetRequireAdditionalShaderChannels(InFlags);
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
	const auto rotation = Transform.GetRotation().Rotator();
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
			//auto ChildRenderCanvas = Item->GetRenderCanvas();
		}
		else
		{
			const auto UIRenderableItem = (UUIBaseRenderable*)(Item);
			UIRenderableItem->UpdateGeometry();
			if (bClipTypeChanged)
			{
				UIRenderableItem->UpdateMaterialClipType();
			}
		}
	}
}

#define LGUI_Test_ResetRenderObjectList 0

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
		switch (DrawcallItem->Type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			//compare drawcall item's bounds
			if (DrawcallItem->RenderObjectListTreeRootNode->Overlap(UIQuadTree::Rectangle(ItemToCanvasTf.BoundsMin2D, ItemToCanvasTf.BoundsMax2D)))
			{
				return true;
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			DrawcallItem->PostProcessRenderableObject->GetRenderCanvas()->GetCacheUIItemToCanvasTransform(DrawcallItem->PostProcessRenderableObject.Get(), OtherItemToCanvasTf);
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
		const auto LastDrawcallIndex = InUIDrawcallList.Num() - 1;
		if (LastDrawcallIndex < 0)
		{
			return false;
		}
		//first step, check last drawcall, because 3d UI can only batch into last drawcall
		{
			const auto LastDrawcall = InUIDrawcallList[LastDrawcallIndex];
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
			const auto DrawcallItem = InUIDrawcallList[i];
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

	ULGUIMeshComponent* PrevMesh = nullptr;//prev using mesh
	bool bShouldMeshSectionContinue = false;//use continuous mesh section in same mesh. Default is false, because first PrevMesh is nullptr, can't continue mesh section
	auto PushSingleDrawcall = [&](UUIItem* InUIItem, bool InSearchInCacheList, UIGeometry* InItemGeo, bool InIs2DSpace, EUIDrawcallType InDrawcallType, const FLGUICacheTransformContainer& InItemToCanvasTf) {
		TSharedPtr<UUIDrawcall> DrawcallItem = nullptr;
		//if this UIItem exist in InCacheUIDrawcallList, then grab the entire drawcall item (may include other UIItem in RenderObjectList). No need to worry other UIItem, because they could be cleared in further operation, or exist in the same drawcall
		int32 FoundDrawcallIndex = INDEX_NONE;
		if (InSearchInCacheList)
		{
			FoundDrawcallIndex = InCacheUIDrawcallList.IndexOfByPredicate([=](const TSharedPtr<UUIDrawcall>& DrawcallItem) {
				if (DrawcallItem->Type == InDrawcallType)
				{
					switch (InDrawcallType)
					{
					case EUIDrawcallType::BatchGeometry:
					{
						if (DrawcallItem->RenderObjectList.Contains(InUIItem))
						{
							return true;
						}
					}
					break;
					case EUIDrawcallType::PostProcess:
					{
						if (DrawcallItem->PostProcessRenderableObject == InUIItem)
						{
							return true;
						}
					}
					break;
					case EUIDrawcallType::DirectMesh:
					{
						if (DrawcallItem->DirectMeshRenderableObject == InUIItem)
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
		if (FoundDrawcallIndex != INDEX_NONE)//find exist drawcall from old DrawcallList
		{
			DrawcallItem = InCacheUIDrawcallList[FoundDrawcallIndex];
			InCacheUIDrawcallList.RemoveAt(FoundDrawcallIndex);//cannot use "RemoveAtSwap" here, because we need the right order to tell if we should sort render order, see "bNeedToSortRenderPriority"

			switch (InDrawcallType)
			{
			case EUIDrawcallType::BatchGeometry:
			{
				DrawcallItem->Texture = InItemGeo->texture;
				DrawcallItem->Material = InItemGeo->material.Get();
#if LGUI_Test_ResetRenderObjectList
				DrawcallItem->RenderObjectList.Reset();
				DrawcallItem->RenderObjectList.Add((UUIBatchGeometryRenderable*)InUIItem);
#endif
				if (DrawcallItem->RenderObjectListTreeRootNode != nullptr)
				{
					delete DrawcallItem->RenderObjectListTreeRootNode;
				}
				DrawcallItem->RenderObjectListTreeRootNode = new UIQuadTree::Node(CanvasRect);
				DrawcallItem->RenderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(InItemToCanvasTf.BoundsMin2D, InItemToCanvasTf.BoundsMax2D));
				DrawcallItem->VerticesCount = InItemGeo->vertices.Num();
				DrawcallItem->IndicesCount = InItemGeo->triangles.Num();

				if (bShouldMeshSectionContinue)//If we should continue mesh section:
				{
					if (PrevMesh != DrawcallItem->DrawcallMesh.Get())//but the found drawcall contains different mesh, so we clear DrawcallMesh in order to use new mesh
					{
						if (DrawcallItem->DrawcallMeshSection.IsValid())
						{
							DrawcallItem->DrawcallMesh->DeleteMeshSection(DrawcallItem->DrawcallMeshSection.Pin());
						}
						DrawcallItem->DrawcallMesh = nullptr;
						DrawcallItem->DrawcallMeshSection = nullptr;
						DrawcallItem->bNeedToUpdateVertex = true;
						DrawcallItem->bMaterialNeedToReassign = true;
					}
					//else//found drawcall use same mesh as PrevMesh, means the continuous mesh section is good to use
				}
				//else//No need to continue mesh section, means we no need to concern PrevMesh
			}
			break;
			case EUIDrawcallType::PostProcess:
			{
				DrawcallItem->PostProcessRenderableObject = (UUIPostProcessRenderable*)InUIItem;
			}
			break;
			case EUIDrawcallType::DirectMesh:
			{
				DrawcallItem->DirectMeshRenderableObject = (UUIDirectMeshRenderable*)InUIItem;
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
				DrawcallItem = MakeShared<UUIDrawcall>(CanvasRect);
				DrawcallItem->bNeedToUpdateVertex = true;
				DrawcallItem->Texture = InItemGeo->texture;
				DrawcallItem->Material = InItemGeo->material.Get();
				DrawcallItem->RenderObjectList.Add((UUIBatchGeometryRenderable*)InUIItem);
				DrawcallItem->VerticesCount = InItemGeo->vertices.Num();
				DrawcallItem->IndicesCount = InItemGeo->triangles.Num();
				DrawcallItem->RenderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(InItemToCanvasTf.BoundsMin2D, InItemToCanvasTf.BoundsMax2D));
			}
			break;
			case EUIDrawcallType::PostProcess:
			{
				DrawcallItem = MakeShared<UUIDrawcall>(InDrawcallType);
				DrawcallItem->PostProcessRenderableObject = (UUIPostProcessRenderable*)InUIItem;
			}
			break;
			case EUIDrawcallType::DirectMesh:
			{
				DrawcallItem = MakeShared<UUIDrawcall>(InDrawcallType);
				DrawcallItem->DirectMeshRenderableObject = (UUIDirectMeshRenderable*)InUIItem;
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
		if (InDrawcallItem->DrawcallMesh.IsValid())
		{
			if (InDrawcallItem->DrawcallMeshSection.IsValid())
			{
				InDrawcallItem->DrawcallMesh->DeleteMeshSection(InDrawcallItem->DrawcallMeshSection.Pin());
				InDrawcallItem->DrawcallMeshSection = nullptr;
			}
		}
		InDrawcallItem->bNeedToUpdateVertex = true;
		InDrawcallItem->bMaterialNeedToReassign = true;
		int index = InDrawcallItem->RenderObjectList.IndexOfByKey(InUIBatchGeometryRenderable);
		InDrawcallItem->RenderObjectList.RemoveAt(index);
		InUIBatchGeometryRenderable->drawcall = nullptr;
	};
	//if ChildCanvas or PostProcess break mesh sequence (continuous mesh section in same mesh), then next drawcalls should use new mesh, so we need to clear mesh and sections which use prev mesh
	auto RemoveDrawcallWhenBreakMeshSequence = [&](int InStartDrawcallIndex) {
		if (InStartDrawcallIndex < InUIDrawcallList.Num())
		{
			const auto& DrawcallItem = InUIDrawcallList[InStartDrawcallIndex];
			if (DrawcallItem->DrawcallMesh.IsValid())
			{
				for (int CacheDrawcallIndex = 0; CacheDrawcallIndex < InCacheUIDrawcallList.Num(); CacheDrawcallIndex++)
				{
					const auto CacheDrawcallItem = InCacheUIDrawcallList[CacheDrawcallIndex];
					if (DrawcallItem->DrawcallMesh == CacheDrawcallItem->DrawcallMesh)
					{
						if (CacheDrawcallItem->DrawcallMeshSection.IsValid())
						{
							DrawcallItem->DrawcallMesh->DeleteMeshSection(CacheDrawcallItem->DrawcallMeshSection.Pin());
						}
						CacheDrawcallItem->bNeedToUpdateVertex = true;
						CacheDrawcallItem->bMaterialNeedToReassign = true;
						CacheDrawcallItem->DrawcallMesh = nullptr;
						CacheDrawcallItem->DrawcallMeshSection = nullptr;
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
						return CacheDrawcallItem->Type == EUIDrawcallType::ChildCanvas && CacheDrawcallItem->ChildCanvas == ChildCanvas;
						});
					if (FoundIndex != INDEX_NONE)
					{
						InUIDrawcallList.Add(InCacheUIDrawcallList[FoundIndex]);
						InCacheUIDrawcallList.RemoveAt(FoundIndex);
					}
					else
					{
						auto ChildCanvasDrawcall = MakeShared<UUIDrawcall>(EUIDrawcallType::ChildCanvas);
						ChildCanvasDrawcall->ChildCanvas = ChildCanvas;
						InUIDrawcallList.Add(ChildCanvasDrawcall);
					}
					if (FoundIndex != 0)//if not find drawcall or found drawcall not at head of array, means drawcall list's order is changed compare to cache list, then we need to sort render order
					{
						OutNeedToSortRenderPriority = true;
					}
				}
				else
				{
					auto ChildCanvasDrawcall = MakeShared<UUIDrawcall>(EUIDrawcallType::ChildCanvas);
					ChildCanvasDrawcall->ChildCanvas = ChildCanvas;
					InUIDrawcallList.Add(ChildCanvasDrawcall);
					OutNeedToSortRenderPriority = true;
				}

				FitInDrawcallMinIndex = InUIDrawcallList.Num();
				MeshStartDrawcallIndex = InUIDrawcallList.Num();
			}
			PrevMesh = nullptr;
			bShouldMeshSectionContinue = false;
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
				if (ItemGeo == nullptr)continue;
				if (ItemGeo->vertices.Num() == 0)continue;
				if (ItemGeo->vertices.Num() > LGUI_MAX_VERTEX_COUNT)continue;

				int DrawcallIndexToFitin;
				if (CanFitInDrawcall(UIBatchGeometryRenderableItem, is2DUIItem, ItemGeo->vertices.Num(), UIItemToCanvasTf, DrawcallIndexToFitin))
				{
					auto DrawcallItem = InUIDrawcallList[DrawcallIndexToFitin];
					DrawcallItem->bIs2DSpace = DrawcallItem->bIs2DSpace && is2DUIItem;
					if (UIBatchGeometryRenderableItem->drawcall == DrawcallItem)//already exist in this drawcall (added previoursly)
					{
#if LGUI_Test_ResetRenderObjectList
						DrawcallItem->RenderObjectList.Add(UIBatchGeometryRenderableItem);
#else
						//mark sort list
						DrawcallItem->bNeedToSortRenderObjectList = true;
#endif
						//update tree
						DrawcallItem->RenderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(UIItemToCanvasTf.BoundsMin2D, UIItemToCanvasTf.BoundsMax2D));
						DrawcallItem->VerticesCount += ItemGeo->vertices.Num();
						DrawcallItem->IndicesCount += ItemGeo->triangles.Num();
					}
					else//not exist in this drawcall
					{
						auto OldDrawcall = UIBatchGeometryRenderableItem->drawcall;
						if (OldDrawcall.IsValid())//maybe exist in other drawcall, should remove from that drawcall
						{
							ClearObjectFromDrawcall(OldDrawcall, UIBatchGeometryRenderableItem, UIItemToCanvasTf);
						}
						//add to this drawcall
						DrawcallItem->RenderObjectList.Add(UIBatchGeometryRenderableItem);
						DrawcallItem->RenderObjectListTreeRootNode->Insert(UIQuadTree::Rectangle(UIItemToCanvasTf.BoundsMin2D, UIItemToCanvasTf.BoundsMax2D));
						DrawcallItem->VerticesCount += ItemGeo->vertices.Num();
						DrawcallItem->IndicesCount += ItemGeo->triangles.Num();
						DrawcallItem->bNeedToUpdateVertex = true;
						UIBatchGeometryRenderableItem->drawcall = DrawcallItem;
						//copy update state from old to new
						if (OldDrawcall.IsValid())
						{
							OldDrawcall->CopyUpdateState(DrawcallItem.Get());
						}
					}
					check(DrawcallItem->VerticesCount < LGUI_MAX_VERTEX_COUNT);
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
					check(UIBatchGeometryRenderableItem->drawcall->VerticesCount < LGUI_MAX_VERTEX_COUNT);
				}
				PrevMesh = UIBatchGeometryRenderableItem->drawcall->DrawcallMesh.Get();
				bShouldMeshSectionContinue = true;
			}
			break;
			case EUIRenderableType::UIPostProcessRenderable:
			{
				auto UIPostProcessRenderableItem = (UUIPostProcessRenderable*)UIRenderableItem;
				if (!UIPostProcessRenderableItem->HaveValidData())continue;
				//every postprocess is a drawcall
				PushSingleDrawcall(UIRenderableItem, true, nullptr, is2DUIItem, EUIDrawcallType::PostProcess, UIItemToCanvasTf);
				//no need to copy drawcall's update data for UIPostProcessRenderable, because UIPostProcessRenderable's drawcall should be the same as previours one

				RemoveDrawcallWhenBreakMeshSequence(MeshStartDrawcallIndex);
				FitInDrawcallMinIndex = InUIDrawcallList.Num();
				MeshStartDrawcallIndex = InUIDrawcallList.Num();
				PrevMesh = nullptr;
				bShouldMeshSectionContinue = false;
			}
			break;
			case EUIRenderableType::UIDirectMeshRenderable:
			{
				auto UIDirectMeshRenderableItem = (UUIDirectMeshRenderable*)UIRenderableItem;
				if (!UIDirectMeshRenderableItem->HaveValidData())continue;
				//every direct mesh is a drawcall
				PushSingleDrawcall(UIRenderableItem, true, nullptr, is2DUIItem, EUIDrawcallType::DirectMesh, UIItemToCanvasTf);
				UIDirectMeshRenderableItem->drawcall->Material = UIDirectMeshRenderableItem->GetMaterial();
				//no need to copy drawcall's update data for UIDirectMeshRenderable, because UIDirectMeshRenderable's drawcall should be the same as previours one
				PrevMesh = nullptr;
				bShouldMeshSectionContinue = false;
			}
			break;
			}
		}
	}

	if (
		//MeshStartDrawcallIndex = 0 means there is action that break mesh sequence, but if UsingUIMeshList.Num() > 0 then means previours using multiple mesh, so we need to remove the mesh and clear drawcalls which use these mesh
		MeshStartDrawcallIndex == 0
		&& UsingUIMeshList.Num() > 0
		)
	{
		TSet<TWeakObjectPtr<ULGUIMeshComponent>> MeshSetNeedToPool;
		for (int i = 1//skip first, because we need a mesh to render
			; i < UsingUIMeshList.Num(); i++)
		{
			auto& UIMeshItem = UsingUIMeshList[i];
			for (auto& DrawcallItem : InUIDrawcallList)
			{
				if (DrawcallItem->DrawcallMesh == UIMeshItem)
				{
					if (DrawcallItem->DrawcallMeshSection.IsValid())
					{
						DrawcallItem->DrawcallMesh->DeleteMeshSection(DrawcallItem->DrawcallMeshSection.Pin());
					}
					DrawcallItem->DrawcallMesh = nullptr;
					DrawcallItem->DrawcallMeshSection = nullptr;
					DrawcallItem->bNeedToUpdateVertex = true;
					DrawcallItem->bMaterialNeedToReassign = true;
				}
			}
			MeshSetNeedToPool.Add(UIMeshItem);
		}
		for (auto& UIMeshItem : MeshSetNeedToPool)
		{
			this->AddUIMeshToPool(UIMeshItem);
		}
	}

	//@todo: the UIRenderableList is already sorted, so actually we better not to sort the RenderObjectList. But when I try to do it (LGUI_Test_ResetRenderObjectList), a RenderObjectList become "Invalid", that is very strange, a TArray can't just become "Invalid".
	//check if we need to sort RenderObjectList
#if !LGUI_Test_ResetRenderObjectList
	for (auto& DrawcallItem : InUIDrawcallList)
	{
		if (DrawcallItem->bNeedToSortRenderObjectList)
		{
			DrawcallItem->bNeedToSortRenderObjectList = false;
			DrawcallItem->RenderObjectList.Sort([](const TWeakObjectPtr<UUIBatchGeometryRenderable>& A, const TWeakObjectPtr<UUIBatchGeometryRenderable>& B) {
				return A->GetFlattenHierarchyIndex() < B->GetFlattenHierarchyIndex();
				});
		}
	}
#endif
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
	case ELGUICanvasClipType::Custom:
		bNeedToUpdateCustomClipParameter = true;
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
			const auto& DrawcallItem = UIDrawcallList[i];
			DrawcallItem->bNeedToUpdateVertex = true;
			DrawcallItem->DrawcallMeshSection = nullptr;
			DrawcallItem->DrawcallMesh = nullptr;
			DrawcallItem->bMaterialChanged = true;//material is directly used by mesh
		}
		//clear mesh
		for (const auto& Mesh : PooledUIMeshList)
		{
			Mesh->ClearAllMeshSection();
			Mesh->DestroyComponent();
		}
		PooledUIMeshList.Reset();
		for (const auto& Mesh : UsingUIMeshList)
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
	for (const auto& ChildCanvas : ChildrenCanvasArray)
	{
		if (ChildCanvas.IsValid() && ChildCanvas->GetIsUIActive())
		{
			ChildCanvas->MarkFinishRenderFrameRecursive();
		}
	}

	bShouldRebuildDrawcall = false;
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;
	bNeedToUpdateCustomClipParameter = false;
}

bool ULGUICanvas::UpdateCanvasDrawcallRecursive()
{
	/**
	 * Why use bPrevUIItemIsActive?:
	 * If Canvas is rendering in frame 1, but when in frame 2 the Canvas is disabled(by disable UIItem), then the Canvas will not do drawcall calculation, and the prev existing drawcall mesh is still there and render,
	 * so we check bPrevUIItemIsActive, then we can still do drawcall calculation at this frame, and the prev existing drawcall will be removed.
	 */
	bool bResult = false;
	const bool bNowUIItemIsActive = UIItem->GetIsUIActiveInHierarchy();
	if (bNowUIItemIsActive || bPrevUIItemIsActive)
	{
		bResult = true;
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
		RootCanvas->bAnythingChangedForRenderTarget = true;

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
			const auto Width = FMath::Max(UIItem->GetWidth(), 100.0f);
			const auto Height = FMath::Max(UIItem->GetHeight(), 100.0f);
			FVector2D LeftBottomPoint;
			LeftBottomPoint.X = Width * -UIItem->GetPivot().X;
			LeftBottomPoint.Y = Height * -UIItem->GetPivot().Y;
			FVector2D RightTopPoint;
			RightTopPoint.X = Width * (1.0f - UIItem->GetPivot().X);
			RightTopPoint.Y = Height * (1.0f - UIItem->GetPivot().Y);
			bool bOutNeedToSortRenderPriority = bNeedToSortRenderPriority;
			BatchDrawcall_Implement(LeftBottomPoint, RightTopPoint, UIDrawcallList, CacheUIDrawcallList
				, bOutNeedToSortRenderPriority//cannot pass a uint32:1 here, so use a temp bool
			);
			bNeedToSortRenderPriority = bOutNeedToSortRenderPriority;

			auto IsUsingUIMesh = [&](TWeakObjectPtr<ULGUIMeshComponent> InMesh) {
				for (const auto& Drawcall : UIDrawcallList)
				{
					if (Drawcall->DrawcallMesh == InMesh)
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
				//check(DrawcallInCache->RenderObjectList.Num() == 0);//why comment this?: need to wait until UUIBaseRenderable::OnRenderCanvasChanged.todo finish
				if (DrawcallInCache->DrawcallMesh.IsValid())
				{
					if (DrawcallInCache->DrawcallMeshSection.IsValid())
					{
						DrawcallInCache->DrawcallMesh->DeleteMeshSection(DrawcallInCache->DrawcallMeshSection.Pin());
						DrawcallInCache->DrawcallMeshSection = nullptr;
					}
					if (!IsUsingUIMesh(DrawcallInCache->DrawcallMesh))
					{
						this->AddUIMeshToPool(DrawcallInCache->DrawcallMesh);
					}
					DrawcallInCache->DrawcallMesh.Reset();
				}
				if (DrawcallInCache->RenderMaterial.IsValid())
				{
					if (DrawcallInCache->bMaterialContainsLGUIParameter)
					{
						this->AddUIMaterialToPool((UMaterialInstanceDynamic*)DrawcallInCache->RenderMaterial.Get());
					}
					DrawcallInCache->RenderMaterial = nullptr;
					DrawcallInCache->bMaterialContainsLGUIParameter = false;
				}
				if (DrawcallInCache->PostProcessRenderableObject.IsValid())
				{
					if (DrawcallInCache->PostProcessRenderableObject->IsRenderProxyValid())
					{
						DrawcallInCache->PostProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
					}
				}
				if (DrawcallInCache->DirectMeshRenderableObject.IsValid())
				{
					DrawcallInCache->DirectMeshRenderableObject->ClearMeshData();
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
			if (auto Instance = ALGUIManagerActor::GetInstance(this->GetWorld(), false))
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

	return bResult;
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawcallMesh"), STAT_UpdateDrawcallMesh, STATGROUP_LGUI);
void ULGUICanvas::UpdateDrawcallMesh_Implement()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcallMesh);

	auto FindNextValidMeshInDrawcallList = [&](int32 InStartIndex)
	{
		for (auto i = InStartIndex; i < UIDrawcallList.Num(); i++)
		{
			const auto& DrawcallItem = UIDrawcallList[i];
			switch (DrawcallItem->Type)
			{
			case EUIDrawcallType::DirectMesh:
			case EUIDrawcallType::BatchGeometry:
			{
				if (DrawcallItem->DrawcallMesh.IsValid())
				{
					return DrawcallItem->DrawcallMesh.Get();
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
		switch (DrawcallItem->Type)
		{
		case EUIDrawcallType::DirectMesh:
		case EUIDrawcallType::BatchGeometry:
		{
			if (!DrawcallItem->DrawcallMesh.IsValid())
			{
				if (CurrentUIMesh == nullptr)
				{
					//if drawcall mesh is not valid, we need to search in next drawcalls and find mesh drawcall object (not post process drawcall)
					if (const auto FoundMesh = FindNextValidMeshInDrawcallList(i + 1))
					{
						CurrentUIMesh = FoundMesh;
					}
					else//not find valid mesh, then get from pool
					{
						CurrentUIMesh = this->GetUIMeshFromPool().Get();
					}
				}
				DrawcallItem->DrawcallMesh = CurrentUIMesh;
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


		switch (DrawcallItem->Type)
		{
		case EUIDrawcallType::DirectMesh:
		{
			auto MeshSection = DrawcallItem->DrawcallMeshSection;
			if (!MeshSection.IsValid())
			{
				auto UIMesh = DrawcallItem->DrawcallMesh;
				MeshSection = UIMesh->GetMeshSection();

				DrawcallItem->DrawcallMeshSection = MeshSection;
				DrawcallItem->DirectMeshRenderableObject->OnMeshDataReady();
				UIMesh->CreateMeshSectionData(MeshSection.Pin());
				//create new mesh section, need to sort it
				bNeedToSortRenderPriority = true;
			}
			CurrentUIMesh = DrawcallItem->DrawcallMesh.Get();
		}
		break;
		case EUIDrawcallType::BatchGeometry:
		{
			auto UIMesh = DrawcallItem->DrawcallMesh;
			auto MeshSection = DrawcallItem->DrawcallMeshSection;
			if (!MeshSection.IsValid())
			{
				MeshSection = UIMesh->GetMeshSection();
				DrawcallItem->DrawcallMeshSection = MeshSection;
				//create new mesh section, need to sort it
				bNeedToSortRenderPriority = true;
				DrawcallItem->bNeedToUpdateVertex = true;
			}
			if (DrawcallItem->bNeedToUpdateVertex)
			{
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
				DrawcallItem->bNeedToUpdateVertex = false;
				DrawcallItem->bVertexPositionChanged = false;
			}
			CurrentUIMesh = DrawcallItem->DrawcallMesh.Get();
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			//only LGUI renderer can render post process
			if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace)
			{
				continue;
			}
			else//lgui renderer, create a PostProcessRenderable object to handle it. then the next UI objects should render by new mesh
			{
				CurrentUIMesh = nullptr;//set to null so a new mesh will be created for next drawcall
			}

			if (!DrawcallItem->PostProcessRenderableObject->IsRenderProxyValid())
			{
				DrawcallItem->bNeedToAddPostProcessRenderProxyToRender = true;
			}
			if (DrawcallItem->bNeedToAddPostProcessRenderProxyToRender)
			{
				DrawcallItem->bNeedToAddPostProcessRenderProxyToRender = false;
				auto uiPostProcessPrimitive = DrawcallItem->PostProcessRenderableObject->GetRenderProxy();
				if (RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode))
				{
					auto ActualRenderMode = GetActualRenderMode();
#if WITH_EDITOR
					if (previewWithLGUIRenderer)
					{
						if (!GetWorld()->IsGameWorld())//edit mode
						{
							if (ActualRenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
								ActualRenderMode = ELGUIRenderMode::WorldSpace_LGUI;
						}
					}
#endif
					switch (ActualRenderMode)
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

		if (RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode))
		{
			auto ActualRenderMode = GetActualRenderMode();
#if WITH_EDITOR
			if (previewWithLGUIRenderer)
			{
				if (!GetWorld()->IsGameWorld())//edit mode
				{
					if (ActualRenderMode == ELGUIRenderMode::ScreenSpaceOverlay)
						ActualRenderMode = ELGUIRenderMode::WorldSpace_LGUI;
				}
			}
#endif
			switch (ActualRenderMode)
			{
			case ELGUIRenderMode::RenderTarget:
			{
				UIMesh->SetSupportLGUIRenderer(true, this->GetRootCanvas()->GetRenderTargetViewExtension(), RootCanvas.Get(), false);
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())
				{
					UIMesh->SetSupportUERenderer(true);
				}
				else
#endif
				{
					UIMesh->SetSupportUERenderer(false);
				}
			}
			break;
			case ELGUIRenderMode::ScreenSpaceOverlay:
			{
#if WITH_EDITOR
				if (!GetWorld()->IsGameWorld())
				{
					UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), RootCanvas.Get(), false);
					UIMesh->SetSupportUERenderer(true);
				}
				else
#endif
				{
					UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), RootCanvas.Get(), false);
					UIMesh->SetSupportUERenderer(false);
				}
			}
			break;
			case ELGUIRenderMode::WorldSpace_LGUI:
			{
				UIMesh->SetSupportLGUIRenderer(true, ALGUIManagerActor::GetViewExtension(GetWorld(), true), this, true);
				UIMesh->SetSupportUERenderer(false);
			}
			break;
			}
		}
		else
		{
			UIMesh->SetSupportLGUIRenderer(false, nullptr, nullptr, false);
			UIMesh->SetSupportUERenderer(true);
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
	ULGUIMeshComponent* PrevUIMesh = nullptr;
	int MeshSectionIndex = 0;
	auto FinishPrevMesh = [&]() {
		if (PrevUIMesh != nullptr)
		{
			PrevUIMesh->SortMeshSectionRenderPriority();
			MeshSectionIndex = 0;
			PrevUIMesh->SetUITranslucentSortPriority(InOutRenderPriority++);
		}
	};
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		switch (DrawcallItem->Type)
		{
		case EUIDrawcallType::BatchGeometry:
		case EUIDrawcallType::DirectMesh:
		{
			if (DrawcallItem->DrawcallMesh != PrevUIMesh)
			{
				FinishPrevMesh();
				PrevUIMesh = DrawcallItem->DrawcallMesh.Get();
			}
			DrawcallItem->DrawcallMesh->SetMeshSectionRenderPriority(DrawcallItem->DrawcallMeshSection.Pin(), MeshSectionIndex++);
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			FinishPrevMesh();
			PrevUIMesh = nullptr;
			DrawcallItem->PostProcessRenderableObject->GetRenderProxy()->SetUITranslucentSortPriority(InOutRenderPriority++);
		}
		break;
		case EUIDrawcallType::ChildCanvas:
		{
			if (ensure(DrawcallItem->ChildCanvas.IsValid()))
			{
				FinishPrevMesh();
				PrevUIMesh = nullptr;
				DrawcallItem->ChildCanvas->SortDrawcall(InOutRenderPriority);
			}
		}
		break;
		}
	}
	FinishPrevMesh();
}

FName ULGUICanvas::LGUI_MainTextureMaterialParameterName = FName(TEXT("MainTexture"));
FName ULGUICanvas::LGUI_RectClipOffsetAndSize_MaterialParameterName = FName(TEXT("RectClipOffsetAndSize"));
FName ULGUICanvas::LGUI_RectClipFeather_MaterialParameterName = FName(TEXT("RectClipFeather"));
FName ULGUICanvas::LGUI_TextureClip_MaterialParameterName = FName(TEXT("ClipTexture"));
FName ULGUICanvas::LGUI_TextureClipOffsetAndSize_MaterialParameterName = FName(TEXT("TextureClipOffsetAndSize"));

bool ULGUICanvas::IsMaterialContainsLGUIParameter(UMaterialInterface* InMaterial, ELGUICanvasClipType InClipType, ULGUICanvasCustomClip* InCustomClip)
{
	if (InClipType == ELGUICanvasClipType::Custom)
	{
		if (IsValid(InCustomClip))
		{
			return true;
		}
		else
		{
			return false;
		}
	}
	static TArray<FMaterialParameterInfo> ParameterInfos;
	static TArray<FGuid> ParameterIds;
	InMaterial->GetAllTextureParameterInfo(ParameterInfos, ParameterIds);
	auto FoundIndex = ParameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& Item)
		{
			return
				Item.Name == LGUI_MainTextureMaterialParameterName
				|| Item.Name == LGUI_TextureClip_MaterialParameterName
				;
		});
	if (FoundIndex == INDEX_NONE)
	{
		InMaterial->GetAllVectorParameterInfo(ParameterInfos, ParameterIds);
		FoundIndex = ParameterInfos.IndexOfByPredicate([](const FMaterialParameterInfo& Item)
			{
				return
					Item.Name == LGUI_RectClipFeather_MaterialParameterName || Item.Name == LGUI_RectClipOffsetAndSize_MaterialParameterName
					|| Item.Name == LGUI_TextureClipOffsetAndSize_MaterialParameterName
					;
			});
	}
	return FoundIndex != INDEX_NONE;
}
void ULGUICanvas::UpdateDrawcallMaterial_Implement()
{
	bool bNeedToSetClipParameter = false;
	auto TempClipType = this->GetActualClipType();
	ULGUICanvasCustomClip* TempCustomClip = nullptr;
	if (TempClipType == ELGUICanvasClipType::Custom)
	{
		TempCustomClip = GetActualCustomClip();
		if (!IsValid(TempCustomClip))
		{
			TempClipType = ELGUICanvasClipType::None;
		}
	}
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		switch (DrawcallItem->Type)
		{
		case EUIDrawcallType::BatchGeometry:
		case EUIDrawcallType::DirectMesh:
		{
			auto RenderMat = DrawcallItem->RenderMaterial;
			if (!RenderMat.IsValid() || DrawcallItem->bMaterialChanged || this->bClipTypeChanged)
			{
				if (DrawcallItem->Material.IsValid())//custom material
				{
					//the prev RenderMaterial will not be used because we have custom material, so we can try to pool it
					if (RenderMat.IsValid()
						&& RenderMat->IsA(UMaterialInstanceDynamic::StaticClass()))
					{
						this->AddUIMaterialToPool((UMaterialInstanceDynamic*)RenderMat.Get());
					}
					auto SrcMaterial = DrawcallItem->Material.Get();
					if (TempClipType == ELGUICanvasClipType::Custom)
					{
						SrcMaterial = TempCustomClip->GetReplaceMaterial(SrcMaterial);
					}
					auto bContainsLGUIParam = IsMaterialContainsLGUIParameter(SrcMaterial
						, TempClipType, TempCustomClip
					);
					if (SrcMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))//if custom material is UMaterialInstanceDynamic then use it directly
					{
						if (bContainsLGUIParam)
						{
							RenderMat = (UMaterialInstanceDynamic*)SrcMaterial;
						}
						DrawcallItem->DrawcallMesh->SetMeshSectionMaterial(DrawcallItem->DrawcallMeshSection.Pin(), SrcMaterial);
					}
					else//if custom material is not UMaterialInstanceDynamic
					{
						if (bContainsLGUIParam)//if custom material contains LGUI parameters, then LGUI should control these parameters, then we need to create UMaterialInstanceDynamic with the custom material
						{
							RenderMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
							RenderMat->SetFlags(RF_Transient);
							DrawcallItem->DrawcallMesh->SetMeshSectionMaterial(DrawcallItem->DrawcallMeshSection.Pin(), RenderMat.Get());
							if (DrawcallItem->DirectMeshRenderableObject.IsValid())
							{
								DrawcallItem->DirectMeshRenderableObject->OnMaterialInstanceDynamicCreated((UMaterialInstanceDynamic*)RenderMat.Get());
							}
							for (auto& RenderObjectItem : DrawcallItem->RenderObjectList)
							{
								RenderObjectItem->OnMaterialInstanceDynamicCreated((UMaterialInstanceDynamic*)RenderMat.Get());
							}
						}
						else//if custom material not contains LGUI parameters, then use it directly
						{
							RenderMat = SrcMaterial;
							DrawcallItem->DrawcallMesh->SetMeshSectionMaterial(DrawcallItem->DrawcallMeshSection.Pin(), SrcMaterial);
						}
					}
					DrawcallItem->bMaterialContainsLGUIParameter = bContainsLGUIParam;
				}
				else
				{
					RenderMat = this->GetUIMaterialFromPool(TempClipType, TempCustomClip);
					DrawcallItem->DrawcallMesh->SetMeshSectionMaterial(DrawcallItem->DrawcallMeshSection.Pin(), RenderMat.Get());
					DrawcallItem->bMaterialContainsLGUIParameter = true;
				}
				DrawcallItem->RenderMaterial = RenderMat;
				DrawcallItem->bMaterialChanged = false;
				if (RenderMat.IsValid() && DrawcallItem->bMaterialContainsLGUIParameter)
				{
					((UMaterialInstanceDynamic*)RenderMat.Get())->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, DrawcallItem->Texture.Get());
				}
				DrawcallItem->bTextureChanged = false;
				DrawcallItem->bMaterialNeedToReassign = false;
				bNeedToSetClipParameter = true;
			}
			if (DrawcallItem->bTextureChanged)
			{
				DrawcallItem->bTextureChanged = false;
				if (RenderMat.IsValid() && DrawcallItem->bMaterialContainsLGUIParameter)
				{
					((UMaterialInstanceDynamic*)RenderMat.Get())->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, DrawcallItem->Texture.Get());
				}
			}
			if (DrawcallItem->bMaterialNeedToReassign)
			{
				DrawcallItem->bMaterialNeedToReassign = false;
				DrawcallItem->DrawcallMesh->SetMeshSectionMaterial(DrawcallItem->DrawcallMeshSection.Pin(), RenderMat.Get());
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			if (this->bClipTypeChanged
				|| DrawcallItem->bMaterialChanged//maybe it is newly created, so check the materialChanged parameter
				)
			{
				if (DrawcallItem->PostProcessRenderableObject.IsValid())
				{
					DrawcallItem->PostProcessRenderableObject->SetClipType(TempClipType);
					DrawcallItem->bMaterialChanged = false;
				}
				bNeedToSetClipParameter = true;
			}
		}
		break;
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

				switch (DrawcallItem->Type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				case EUIDrawcallType::DirectMesh:
				{
					auto RenderMaterial = DrawcallItem->RenderMaterial;
					if (RenderMaterial.IsValid() && DrawcallItem->bMaterialContainsLGUIParameter)
					{
						((UMaterialInstanceDynamic*)RenderMaterial.Get())->SetVectorParameterValue(LGUI_RectClipOffsetAndSize_MaterialParameterName, TempRectClipOffsetAndSize);
						((UMaterialInstanceDynamic*)RenderMaterial.Get())->SetVectorParameterValue(LGUI_RectClipFeather_MaterialParameterName, TempRectClipFeather);
					}
				}
				break;
				case EUIDrawcallType::PostProcess:
				{
					if (DrawcallItem->PostProcessRenderableObject.IsValid())
					{
						DrawcallItem->PostProcessRenderableObject->SetRectClipParameter(TempRectClipOffsetAndSize, TempRectClipFeather);
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

				switch (DrawcallItem->Type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				case EUIDrawcallType::DirectMesh:
				{
					auto RenderMaterial = DrawcallItem->RenderMaterial;
					if (RenderMaterial.IsValid())
					{
						((UMaterialInstanceDynamic*)RenderMaterial.Get())->SetTextureParameterValue(LGUI_TextureClip_MaterialParameterName, TempClipTexture);
						((UMaterialInstanceDynamic*)RenderMaterial.Get())->SetVectorParameterValue(LGUI_TextureClipOffsetAndSize_MaterialParameterName, TempTextureClipOffsetAndSize);
					}
				}
				break;
				case EUIDrawcallType::PostProcess:
				{
					if (DrawcallItem->PostProcessRenderableObject.IsValid())
					{
						DrawcallItem->PostProcessRenderableObject->SetTextureClipParameter(TempClipTexture, TempTextureClipOffsetAndSize);
					}
				}
				break;
				}
			}
		}
			break;
		case ELGUICanvasClipType::Custom:
		{
			if (bNeedToSetClipParameter
				|| this->bNeedToUpdateCustomClipParameter)
			{
				auto OffsetAndSize = this->GetTextureClipOffsetAndSize();

				switch (DrawcallItem->Type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				case EUIDrawcallType::DirectMesh:
				{
					auto RenderMaterial = DrawcallItem->RenderMaterial;
					if (RenderMaterial.IsValid())
					{
						TempCustomClip->ApplyMaterialParameter((UMaterialInstanceDynamic*)RenderMaterial.Get(), this, UIItem.Get());
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

UMaterialInstanceDynamic* ULGUICanvas::GetUIMaterialFromPool(ELGUICanvasClipType InClipType, ULGUICanvasCustomClip* InCustomClip)
{
	if (InClipType == ELGUICanvasClipType::Custom)
	{
		auto SrcMaterial = InCustomClip->GetReplaceMaterial(GetMaterials()[0]);//custom clip no need to pool
		auto UIMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
		UIMat->SetFlags(RF_Transient);
		return UIMat;
	}

	if (PooledUIMaterialList.Num() == 0)
	{
		for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
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
	for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
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
			for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
			{
				PooledUIMaterialList.Add({});
			}
		}

		auto& MatList = PooledUIMaterialList[CacheMatTypeIndex].MaterialList;
		MatList.Add(UIMat);
	}
}

bool ULGUICanvas::CalculatePointVisibilityOnClip(const FVector& InWorldPoint)
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
				if (auto Pixels = (FColor*)(PlatformData->Mips[0].BulkData.Lock(LOCK_READ_ONLY)))
				{
					auto AlphaValueValue = Pixels[TexPos].R;
					auto AlphaValueValue01 = LGUIUtils::Color255To1_Table[AlphaValueValue];
					Result = AlphaValueValue01 > clipTextureHitTestThreshold;
				}
				PlatformData->Mips[0].BulkData.Unlock();
				return Result;
			}
		}
		return true;
	}
	break;
	case ELGUICanvasClipType::Custom:
	{
		auto TempCustomClip = GetActualCustomClip();
		if (IsValid(TempCustomClip))
		{
			return TempCustomClip->CheckPointVisible(InWorldPoint, this, UIItem.Get());
		}
		return false;
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
void ULGUICanvas::SetCustomClip(ULGUICanvasCustomClip* value)
{
	if (customClip != value)
	{
		customClip = value;
		if (clipType == ELGUICanvasClipType::Custom)
		{
			bClipTypeChanged = true;
			bNeedToUpdateCustomClipParameter = true;
			MarkCanvasUpdate(true, true, false);
		}
	}
}

void ULGUICanvas::SetSortOrderAdditionalValueRecursive(int32 InAdditionalValue)
{
	if (FMath::Abs(this->sortOrder + InAdditionalValue) > MAX_int16)
	{
		auto errorMsg = FText::Format(LOCTEXT("SortOrderOutOfRange", "{0} sortOrder out of range!\nNOTE! sortOrder value is stored with int16 type, so valid range is -32768 to 32767")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
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
		if (CheckRootCanvas())
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
				auto errorMsg = FText::Format(LOCTEXT("SortOrderOutOfRange", "{0} sortOrder out of range!\nNOTE! sortOrder value is stored with int16 type, so valid range is -32768 to 32767")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
				UE_LOG(LGUI, Error, TEXT("%s"), *errorMsg.ToString());
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errorMsg);
#endif
				InSortOrder = FMath::Clamp(InSortOrder, (int32)MIN_int16, (int32)MAX_int16);
			}
			this->sortOrder = InSortOrder;
		}

		if (auto Instance = ALGUIManagerActor::GetInstance(this->GetWorld(), false))
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
	ResultArray.AddUninitialized((int)ELGUICanvasClipType::Custom);
	for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
	{
		ResultArray[i] = DefaultMaterials[i];
	}
	return ResultArray;
}

void ULGUICanvas::SetDefaultMaterials(const TArray<UMaterialInterface*>& InMaterialArray)
{
	if (InMaterialArray.Num() < (int)ELGUICanvasClipType::Custom)
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d InMaterialArray's count must be %d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, (int)ELGUICanvasClipType::Custom);
		return;
	}
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		auto DrawcallItem = UIDrawcallList[i];
		if (DrawcallItem->Type == EUIDrawcallType::BatchGeometry)
		{
			if (DrawcallItem->Material == nullptr)
			{
				DrawcallItem->bMaterialChanged = true;
			}
		}
	}
	for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
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
			UIDrawcallList[i]->bVertexPositionChanged = true;
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

float ULGUICanvas::GetActualDepthFade()const
{
	if (IsRootCanvas())
	{
		return depthFade;
	}
	else
	{
		if (GetOverrideDepthFade())
		{
			return depthFade;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualDepthFade();
			}
		}
	}
	return depthFade;
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
		if (CheckRootCanvas())
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

ULGUICanvasCustomClip* ULGUICanvas::GetActualCustomClip()const
{
	if (IsRootCanvas())
	{
		return customClip;
	}
	else
	{
		if (GetOverrideClipType())
		{
			return customClip;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->customClip;
			}
		}
	}
	return customClip;
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
	return GetActualAdditionalShaderChannelFlags() & (1 << (int)ELGUICanvasAdditionalChannelType::Normal);
}
bool ULGUICanvas::GetRequireTangent()const 
{ 
	return GetActualAdditionalShaderChannelFlags() & (1 << (int)ELGUICanvasAdditionalChannelType::Tangent);
}
bool ULGUICanvas::GetRequireUV1()const 
{ 
	return GetActualAdditionalShaderChannelFlags() & (1 << (int)ELGUICanvasAdditionalChannelType::UV1);
}
bool ULGUICanvas::GetRequireUV2()const 
{
	return GetActualAdditionalShaderChannelFlags() & (1 << (int)ELGUICanvasAdditionalChannelType::UV2);
}
bool ULGUICanvas::GetRequireUV3()const 
{ 
	return GetActualAdditionalShaderChannelFlags() & (1 << (int)ELGUICanvasAdditionalChannelType::UV3);
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
			UE_LOG(LGUI, Error, TEXT("[%s].%d UIItem not valid!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
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
			if (DrawcallItem->Type == EUIDrawcallType::BatchGeometry)
			{
				DrawcallItem->bNeedToUpdateVertex = true;
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
				RenderTargetViewExtension->SetRenderToRenderTarget(true);
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
		if (CheckRootCanvas())
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
		return this->RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode)
			&& pixelPerfect;
	}
	else
	{
		if (CheckRootCanvas())
		{
			if (GetOverridePixelPerfect())
			{
				return RootCanvas->RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode)
					&& this->pixelPerfect;
			}
			else
			{
				if (ParentCanvas.IsValid())
				{
					return RootCanvas->RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode)
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

		if (CheckRootCanvas())
		{
			if (RootCanvas->RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode))
			{
				if (RootCanvas->IsRenderToWorldSpace())
				{
					auto ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
					if (ViewExtension.IsValid())
					{
						ViewExtension->SetRenderCanvasDepthParameter(this, this->GetActualBlendDepth(), this->GetActualDepthFade());
					}
				}
			}
		}
	}
}

void ULGUICanvas::SetDepthFade(float value)
{
	if (depthFade != value)
	{
		depthFade = value;

		if (CheckRootCanvas())
		{
			if (RootCanvas->RenderModeIsLGUIRendererOrUERenderer(CurrentRenderMode))
			{
				if (RootCanvas->IsRenderToWorldSpace())
				{
					auto ViewExtension = ALGUIManagerActor::GetViewExtension(GetWorld(), false);
					if (ViewExtension.IsValid())
					{
						ViewExtension->SetRenderCanvasDepthParameter(this, this->GetActualBlendDepth(), this->GetActualDepthFade());
					}
				}
			}
		}
	}
}

void ULGUICanvas::SetEnableDepthTest(bool value)
{
	if (bEnableDepthTest != value)
	{
		bEnableDepthTest = value;
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
		if (CheckRootCanvas())
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
		if (Item->Type != EUIDrawcallType::ChildCanvas)
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
	FVector2D LocalPoint1, LocalPoint2;
	item->GetGeometryBoundsInLocalSpace(LocalPoint1, LocalPoint2);
	const auto Point1 = transform.TransformPoint(LocalPoint1);
	const auto Point2 = transform.TransformPoint(LocalPoint2);
	const auto Point3 = transform.TransformPoint(FVector2D(LocalPoint2.X, LocalPoint1.Y));
	const auto Point4 = transform.TransformPoint(FVector2D(LocalPoint1.X, LocalPoint2.Y));

	GetMinMax(Point1.X, Point2.X, Point3.X, Point4.X, min.X, max.X);
	GetMinMax(Point1.Y, Point2.Y, Point3.Y, Point4.Y, min.Y, max.Y);
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif