// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Utils/LGUIUtils.h"
#include "CoreGlobals.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#endif
#include "Utils/LGUIUtils.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/HudRender/LGUIRenderer.h"
#include "Core/LGUIMesh/UIDrawcallMesh.h"
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

ULGUICanvas::ULGUICanvas()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
	bShouldUpdateLayout = true;
}

void ULGUICanvas::BeginPlay()
{
	Super::BeginPlay();
	RootCanvas = nullptr;
	CheckRootCanvas();
	currentIsRenderToRenderTargetOrWorld = IsRenderToScreenSpaceOrRenderTarget();
	ParentCanvas = nullptr;
	CheckParentCanvas();
	CheckUIItem();
	MarkCanvasUpdate();

	bShouldRebuildAllDrawcall = true;
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;	
	bShouldUpdateLayout = true;
}
void ULGUICanvas::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ULGUICanvas::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULGUICanvas::UpdateCanvas(float DeltaTime)
{
	if (bCanTickUpdate)
	{
		bCanTickUpdate = false;
		if (CheckUIItem() && UIItem->IsUIActiveInHierarchy())
		{
			this->UpdateRootCanvas();
		}
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
	//OnUIHierarchyChanged();
	//tell UIItem
	if (CheckUIItem())
	{
		UIItem->UIHierarchyChanged();
	}
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
	//OnUIHierarchyChanged();
	//tell UIItem
	if (UIItem.IsValid())
	{
		UIItem->UIHierarchyChanged();
	}
}
void ULGUICanvas::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
	if (ViewExtension.IsValid())
	{
		ViewExtension.Reset();
	}
	for (auto item : UIDrawcallPrimitiveList)
	{
		if (item.UIDrawcallMesh.IsValid())
		{
			item.UIDrawcallMesh->DestroyComponent();
		}
		if (item.UIPostProcessRenderable.IsValid())
		{
			item.UIPostProcessRenderable.Pin()->RemoveFromHudRenderer();
		}
	}
	UIDrawcallPrimitiveList.Empty();
	CacheUIMeshList.Empty();
}

void ULGUICanvas::OnUIActiveStateChanged(bool active)
{
	for (auto item : UIDrawcallPrimitiveList)
	{
		if (item.UIDrawcallMesh.IsValid())
		{
			item.UIDrawcallMesh->SetUIMeshVisibility(active);
		}
		if (item.UIPostProcessRenderable.IsValid())
		{
			item.UIPostProcessRenderable.Pin()->SetVisibility(active);
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
bool ULGUICanvas::CheckParentCanvas()
{
	if (ParentCanvas.IsValid())return true;
	if (this->GetWorld() == nullptr)return false;
	ULGUICanvas* ResultCanvas = nullptr;
	LGUIUtils::FindParentCanvas(this->GetOwner(), ResultCanvas);
	ParentCanvas = ResultCanvas;
	if (ParentCanvas.IsValid())return true;
	return false;
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
	bool oldIsRenderToRenderTargetOrWorld = currentIsRenderToRenderTargetOrWorld;
	if (RootCanvas.IsValid())
	{
		currentIsRenderToRenderTargetOrWorld = RootCanvas->IsRenderToScreenSpaceOrRenderTarget();
	}

	//if hierarchy changed from World/Hud to Hud/World, then we need to recreate all
	if (currentIsRenderToRenderTargetOrWorld != oldIsRenderToRenderTargetOrWorld)
	{
		if (CheckUIItem())
		{
			UIItem->MarkAllDirtyRecursive();
		}
		//clear UIMeshList and delete mesh components, so new mesh will be created. because hud and world mesh not compatible
		for (auto item : UIDrawcallPrimitiveList)
		{
			if (item.UIDrawcallMesh.IsValid())
			{
				item.UIDrawcallMesh->DestroyComponent();
			}
			if (item.UIPostProcessRenderable.IsValid())
			{
				item.UIPostProcessRenderable.Pin()->RemoveFromHudRenderer();
			}
		}
		UIDrawcallPrimitiveList.Reset();
		CacheUIMeshList.Reset();
	}
}
void ULGUICanvas::OnUIHierarchyChanged()
{
	//remove from old
	if (RootCanvas.IsValid())
	{
		RootCanvas->manageCanvasArray.Remove(this);
	}
	RootCanvas = nullptr;
	CheckRootCanvas();
	//add to new
	if (RootCanvas.IsValid())
	{
		RootCanvas->manageCanvasArray.Add(this);
	}
	CheckRenderMode();

	ParentCanvas = nullptr;
	CheckParentCanvas();
	//rebuild drawcall
	MarkRebuildAllDrawcall();
}
void ULGUICanvas::OnUIHierarchyIndexChanged()
{
	
}
bool ULGUICanvas::IsRenderToScreenSpace()
{
	if (RootCanvas.IsValid())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay;
	}
	return false;
}
bool ULGUICanvas::IsRenderToScreenSpaceOrRenderTarget()
{
	if (RootCanvas.IsValid())
	{
		return RootCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay
			|| (RootCanvas->renderMode == ELGUIRenderMode::RenderTarget && IsValid(RootCanvas->renderTarget))
				;
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
		return RootCanvas->renderMode == ELGUIRenderMode::WorldSpace;
	}
	return false;
}

void ULGUICanvas::MarkCanvasUpdate()
{
	this->bCanTickUpdate = true;
	if (CheckRootCanvas())
	{
		RootCanvas->bCanTickUpdate = true;//incase this Canvas's parent have layout component, so mark TopMostCanvas to update
	}
}
void ULGUICanvas::MarkCanvasUpdateLayout()
{
	this->bShouldUpdateLayout = true;
}
#if WITH_EDITOR
void ULGUICanvas::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
	bShouldUpdateLayout = true;
	Super::PostEditChangeProperty(PropertyChangedEvent);

	this->MarkRebuildAllDrawcall();
	if (CheckUIItem())
	{
		UIItem->MarkAllDirtyRecursive();
	}
	if (CheckRootCanvas())
	{
		RootCanvas->MarkCanvasUpdate();
	}
	CheckParentCanvas();
}
void ULGUICanvas::PostLoad()
{
	Super::PostLoad();
}
#endif

UMaterialInterface** ULGUICanvas::GetMaterials()
{
	auto CheckDefaultMaterialsFunction = [this] 
	{
		for (int i = 0; i < 3; i++)
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

#define SHOULD_LOG_RebuildAllDrawcall 0

void ULGUICanvas::MarkRebuildAllDrawcall()
{
#if SHOULD_LOG_RebuildAllDrawcall
	UE_LOG(LGUI, Log, TEXT("MarkRebuildAllDrawcall rebuildDrawcall"));
#endif
	bShouldRebuildAllDrawcall = true;
}
void ULGUICanvas::MarkRebuildSpecificDrawcall(int drawcallIndex)
{
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("MarkRebuildSpecificDrawcall rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
		return;
	}
	auto& uiDrawcall = UIDrawcallList[drawcallIndex];
	uiDrawcall->needToBeRebuild = true;
}
void ULGUICanvas::SetDrawcallTexture(int drawcallIndex, UTexture* drawcallTexture)
{
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("SetDrawcallTexture rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
		return;
	}
	auto& uiDrawcall = UIDrawcallList[drawcallIndex];
	uiDrawcall->texture = drawcallTexture;
	//material
	auto& uiMat = UIMaterialList[drawcallIndex];
	if (IsValid(uiMat))
	{
		uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture.Get());
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[ULGUICanvas::SetDrawcallTexture]Material not valid!"));
	}
}
void ULGUICanvas::MarkUpdateSpecificDrawcallVertex(int drawcallIndex, bool vertexPositionChanged)
{
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("MarkUpdateSpecificDrawcallVertex rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
		return;
	}
	auto& uiDrawcall = UIDrawcallList[drawcallIndex];
	uiDrawcall->needToUpdateVertex = true;
	if (vertexPositionChanged)uiDrawcall->vertexPositionChanged = vertexPositionChanged;
}
void ULGUICanvas::OnUIElementDepthChange(UUIBatchGeometryRenderable* item)
{
	auto drawcallIndex = item->GetGeometry()->drawcallIndex;
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())//drawcall index not inside list, need to rebuild all drawcall
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("OnUIElementDepthChange rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
		return;
	}
	auto& uiDrawcall = UIDrawcallList[drawcallIndex];
	if (uiDrawcall->IsDepthInsideDrawcall(item->GetDepth()))//if new depth is still inside original drawcall, then only need to update original drawcall
	{
		uiDrawcall->needToBeRebuild = true;
	}
	else//or remove from old drawcall, and insert into new drawcall
	{
		RemoveFromDrawcall(item);
		InsertIntoDrawcall(item);
	}
}
UMaterialInstanceDynamic* ULGUICanvas::GetMaterialInstanceDynamicForDrawcall(int drawcallIndex)
{
	if (drawcallIndex == -1 || drawcallIndex >= UIDrawcallList.Num())return nullptr;
	return UIDrawcallList[drawcallIndex]->materialInstanceDynamic.Get();
}

TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ULGUICanvas::GetViewExtension()
{
	if (!ViewExtension.IsValid())
	{
		if (GEngine)
		{
			if (renderMode == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				ViewExtension = FSceneViewExtensions::NewExtension<FLGUIHudRenderer>(this, nullptr);
			}
			else if (renderMode == ELGUIRenderMode::RenderTarget && IsValid(renderTarget))
			{
				ViewExtension = FSceneViewExtensions::NewExtension<FLGUIHudRenderer>(this, renderTarget);
			}
		}
	}
	return ViewExtension;
}

bool ULGUICanvas::GetIsUIActive()const
{
	if (UIItem.IsValid())
	{
		return UIItem->IsUIActiveInHierarchy();
	}
	return false;
}

void ULGUICanvas::AddUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	if (!autoManageDepth && InUIRenderable->GetUIRenderableType() == EUIRenderableType::UIBatchGeometryRenderable)
	{
		InsertIntoDrawcall((UUIBatchGeometryRenderable*)InUIRenderable);
	}
	else
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("[ULGUICanvas::AddUIRenderable]AddUIRenderable rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
	}
	UIRenderableItemList.Add(InUIRenderable);
}
void ULGUICanvas::RemoveUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	if (!autoManageDepth && InUIRenderable->GetUIRenderableType() == EUIRenderableType::UIBatchGeometryRenderable)
	{
		RemoveFromDrawcall((UUIBatchGeometryRenderable*)InUIRenderable);
	}
	else
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("RemoveUIRenderable rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
	}
	UIRenderableItemList.Remove(InUIRenderable);
}
void ULGUICanvas::InsertIntoDrawcall(UUIBatchGeometryRenderable* item)
{
	bool accommodate = false;//can this item fit into list?
	auto itemDepth = item->GetDepth();
	auto itemTexture = item->GetGeometry()->texture;
	int drawcallCount = UIDrawcallList.Num();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto drawcallItem = UIDrawcallList[i];
		if (drawcallItem->type != EUIDrawcallType::BatchGeometry)continue;//can only inser into geometry drawcall
		if (drawcallItem->texture == itemTexture && drawcallItem->IsDepthInsideDrawcall(itemDepth))//if texture is equal and depth can insert into drawcall
		{
			drawcallItem->geometryList.Add(item->GetGeometry());
			item->GetGeometry()->drawcallIndex = i;
			drawcallItem->needToBeRebuild = true;
			accommodate = true;
			break;
		}
	}
	if (!accommodate)//cannot fit to list, then mark rebuild all drawcall. and mark geometry as not render yet
	{
		item->GetGeometry()->drawcallIndex = -1;
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("InsertIntoDrawcall rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
	}
}
void ULGUICanvas::RemoveFromDrawcall(UUIBatchGeometryRenderable* item)
{
	auto drawcallIndex = item->GetGeometry()->drawcallIndex;
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())//drawcall index not inside list, need to rebuild all drawcall
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("RemoveFromDrawcall rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
		return;
	}
	UIDrawcallList[drawcallIndex]->needToBeRebuild = true;
	auto& geometryList = UIDrawcallList[drawcallIndex]->geometryList;
	int geoCount = geometryList.Num();
	for (int i = 0; i < geoCount; i++)
	{
		auto geoItem = geometryList[i];
		if (geoItem == item->GetGeometry())
		{
			geometryList.RemoveAt(i);
			item->GetGeometry()->drawcallIndex = -1;
			geoCount -= 1;
			break;
		}
	}
	if (geoCount == 0)//if drawcall is empty after remove the UIItem, then rebuild all drawcall
	{
#if SHOULD_LOG_RebuildAllDrawcall
		UE_LOG(LGUI, Log, TEXT("RemoveFromDrawcall2 rebuildDrawcall"));
#endif
		bShouldRebuildAllDrawcall = true;
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawcall"), STAT_UpdateDrawcall, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas TotalUpdate"), STAT_TotalUpdate, STATGROUP_LGUI);
void ULGUICanvas::UpdateChildRecursive(UUIItem* target, bool parentLayoutChanged)
{
	const auto& childrenList = target->GetAttachUIChildren();
	for (auto uiChild : childrenList)
	{
		if (IsValid(uiChild) && uiChild->IsUIActiveInHierarchy())
		{
			if (uiChild->IsCanvasUIItem() && uiChild->GetRenderCanvas() != nullptr && uiChild->GetRenderCanvas() != this)
			{
				uiChild->GetRenderCanvas()->UpdateCanvasLayout(parentLayoutChanged);
			}
			else
			{
				auto layoutChanged = parentLayoutChanged;
				uiChild->UpdateLayoutAndGeometry(layoutChanged, cacheForThisUpdate_ShouldUpdateLayout);
				UpdateChildRecursive(uiChild, layoutChanged);
			}
		}
	}
}
void ULGUICanvas::UpdateCanvasLayout(bool parentLayoutChanged)
{
	cacheForThisUpdate_ShouldRebuildAllDrawcall = bShouldRebuildAllDrawcall;
	cacheForThisUpdate_ShouldUpdateLayout = bShouldUpdateLayout || parentLayoutChanged;
	cacheForThisUpdate_ClipTypeChanged = bClipTypeChanged;
	cacheForThisUpdate_RectClipParameterChanged = bRectClipParameterChanged || bShouldUpdateLayout || parentLayoutChanged;
	if (bRectRangeCalculated)
	{
		if (cacheForThisUpdate_RectClipParameterChanged)bRectRangeCalculated = false;
	}
	cacheForThisUpdate_TextureClipParameterChanged = bTextureClipParameterChanged;
	bShouldRebuildAllDrawcall = false;
	bShouldUpdateLayout = false;
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;

	//update layout and geometry
	UIItem->UpdateLayoutAndGeometry(parentLayoutChanged, cacheForThisUpdate_ShouldUpdateLayout);

	UpdateChildRecursive(UIItem.Get(), UIItem->cacheForThisUpdate_LayoutChanged);

	if (bShouldRebuildAllDrawcall)
	{
		cacheForThisUpdate_ShouldRebuildAllDrawcall = true;
	}
}
void ULGUICanvas::UpdateRootCanvas()
{
	SCOPE_CYCLE_COUNTER(STAT_TotalUpdate);
	if (this != RootCanvas) return;
	//update first Canvas
	if (RootCanvas == this)
	{
		UIItem->calculatedParentAlpha = UUIItem::Color255To1_Table[UIItem->widget.color.A];
	}

	CacheUIItemToCanvasTransformMap.Reset();
	UpdateCanvasLayout(false);
	if (prevFrameNumber != GFrameNumber)//ignore if not at new render frame
	{
		prevFrameNumber = GFrameNumber;
		if(autoManageDepth)
		{
			UpdateCanvasGeometryForAutoManageDepth();
		}
		else
		{
			UpdateCanvasGeometry();
		}
	}
}
void ULGUICanvas::UpdateCanvasGeometry()
{
	for (auto item : manageCanvasArray)
	{
		if (item == this)continue;//skip self
		if (item.IsValid() && item->GetIsUIActive())
		{
			if (item->autoManageDepth)
			{
				item->UpdateCanvasGeometryForAutoManageDepth();
			}
			else
			{
				item->UpdateCanvasGeometry();
			}
		}
	}

	//update drawcall
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcall);
		if (this->cacheForThisUpdate_ShouldRebuildAllDrawcall)
		{
			//check valid renderable, incase unnormally deleting actor, like undo
			{
				for (int i = UIRenderableItemList.Num() - 1; i >= 0; i--)
				{
					if (!UIRenderableItemList[i].IsValid())
					{
						UIRenderableItemList.RemoveAt(i);
					}
				}
			}
			//sort on depth
			UIRenderableItemList.Sort([](const TWeakObjectPtr<UUIBaseRenderable>& A, const TWeakObjectPtr<UUIBaseRenderable>& B)
				{
					return A->GetDepth() < B->GetDepth();
				});
			UUIDrawcall::CreateDrawcall(UIRenderableItemList, UIDrawcallList);//create drawcall
			for (auto item : UIDrawcallList)
			{
				item->UpdateDepthRange();
			}
			//create UIMesh based on Drawcall
			int drawcallCount = UIDrawcallList.Num();
			for (auto item : UIDrawcallPrimitiveList)
			{
				if (item.UIPostProcessRenderable.IsValid())
				{
					item.UIPostProcessRenderable.Pin()->RemoveFromHudRenderer();
					item.UIPostProcessRenderable.Reset();
				}
			}
			UIDrawcallPrimitiveList.SetNum(drawcallCount);
			int meshIndex = 0;
			for (int i = 0; i < drawcallCount; i++)
			{
				switch (UIDrawcallList[i]->type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				case EUIDrawcallType::DirectMesh:
				{
					UUIDrawcallMesh* uiMesh = nullptr;
					if (meshIndex < CacheUIMeshList.Num())//get mesh from exist
					{
						if (CacheUIMeshList[meshIndex].IsValid())
						{
							uiMesh = CacheUIMeshList[meshIndex].Get();
						}
					}
					else//create mesh
					{
#if WITH_EDITOR
						auto meshName = FString::Printf(TEXT("%s_Drawcall_%d"), *GetOwner()->GetActorLabel(), meshIndex);
#else
						auto meshName = FString::Printf(TEXT("Drawcall_%d"), meshIndex);
#endif
						uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), FName(*meshName), RF_Transient);
						uiMesh->SetOwnerNoSee(this->GetActualOwnerNoSee());
						uiMesh->SetOnlyOwnerSee(this->GetActualOnlyOwnerSee());
						uiMesh->RegisterComponent();
						//this->GetOwner()->AddInstanceComponent(uiMesh);
						uiMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
						uiMesh->SetRelativeTransform(FTransform::Identity);
						CacheUIMeshList.Add(uiMesh);
#if WITH_EDITOR
						if (!GetWorld()->IsGameWorld())
						{
							if (currentIsRenderToRenderTargetOrWorld)
							{
								uiMesh->SetSupportScreenSpace(true, RootCanvas->GetViewExtension());
							}
							uiMesh->SetSupportWorldSpace(true);
						}
						else
#endif
						if (currentIsRenderToRenderTargetOrWorld)
						{
							uiMesh->SetSupportScreenSpace(true, RootCanvas->GetViewExtension());
							uiMesh->SetSupportWorldSpace(false);
						}
					}
					//set data for mesh
					if (uiMesh != nullptr)
					{
						uiMesh->SetUIMeshVisibility(true);//some UIMesh may set to invisible on prev frame, set to visible
						if (UIDrawcallList[i]->type == EUIDrawcallType::BatchGeometry)
						{
							auto& meshSection = uiMesh->MeshSection;
							meshSection.Reset();
							UIDrawcallList[i]->GetCombined(meshSection.vertices, meshSection.triangles);
							uiMesh->GenerateOrUpdateMesh(true, GetActualAdditionalShaderChannelFlags());
						}
						else//direct mesh
						{
							UIDrawcallList[i]->directMeshRenderableObject->SetDrawcallMesh(uiMesh);
						}
					}

					UIDrawcallPrimitiveList[i].UIDrawcallMesh = uiMesh;

					UIDrawcallPrimitiveList[i].UIPostProcessRenderable = nullptr;

					meshIndex++;
				}
				break;
				case EUIDrawcallType::PostProcess:
				{
					auto uiPostProcessPrimitive = UIDrawcallList[i]->postProcessObject->GetRenderProxy();
#if WITH_EDITOR
					if (!GetWorld()->IsGameWorld())
					{
						if (currentIsRenderToRenderTargetOrWorld)
						{
							if (RootCanvas->GetViewExtension())
							{
								uiPostProcessPrimitive.Pin()->AddToHudRenderer(RootCanvas->GetViewExtension());
								uiPostProcessPrimitive.Pin()->SetVisibility(true);
							}
						}
					}
					else
#endif
					if (currentIsRenderToRenderTargetOrWorld)
					{
						uiPostProcessPrimitive.Pin()->AddToHudRenderer(RootCanvas->GetViewExtension());
						uiPostProcessPrimitive.Pin()->SetVisibility(true);
					}

					UIDrawcallPrimitiveList[i].UIPostProcessRenderable = uiPostProcessPrimitive;

					UIDrawcallPrimitiveList[i].UIDrawcallMesh = nullptr;
				}
				break;
				}
			}

			if (CacheUIMeshList.Num() > meshIndex)//set needless mesh invisible
			{
				for (int i = meshIndex; i < CacheUIMeshList.Num(); i++)
				{
					if (CacheUIMeshList[i].IsValid())
					{
						CacheUIMeshList[i]->SetUIMeshVisibility(false);
					}
				}
			}

			//after geometry created, need to sort UIMesh render order
			RootCanvas->bNeedToSortRenderPriority = true;
		}
		else//no need to rebuild all drawcall
		{
			int drawcallCount = UIDrawcallList.Num();
			bool needToSortRenderPriority = false;
			{
				for (int i = drawcallCount - 1; i >= 0; i--)
				{
					auto uiDrawcall = UIDrawcallList[i];
					if (!uiDrawcall.IsValid())continue;
					switch (uiDrawcall->type)
					{
					case EUIDrawcallType::BatchGeometry:
					{
						auto uiMesh = UIDrawcallPrimitiveList[i].UIDrawcallMesh;
						if (!uiMesh.IsValid())continue;
						if (uiDrawcall->needToBeRebuild)
						{
							//sort needed for new add item
							uiDrawcall->geometryList.Sort([](const TSharedPtr<UIGeometry>& A, const TSharedPtr<UIGeometry>& B)
								{
									return A->depth < B->depth;
								});

							uiDrawcall->UpdateDepthRange();
							auto& meshSection = uiMesh->MeshSection;
							meshSection.Reset();
							uiDrawcall->GetCombined(meshSection.vertices, meshSection.triangles);
							uiMesh->GenerateOrUpdateMesh(true, GetActualAdditionalShaderChannelFlags());
							uiDrawcall->needToBeRebuild = false;
							uiDrawcall->needToUpdateVertex = false;
							needToSortRenderPriority = true;
						}
						else if (uiDrawcall->needToUpdateVertex)
						{
							auto& meshSection = uiMesh->MeshSection;
							uiDrawcall->UpdateData(meshSection.vertices, meshSection.triangles);
							uiMesh->GenerateOrUpdateMesh(uiDrawcall->vertexPositionChanged, GetActualAdditionalShaderChannelFlags());
							uiDrawcall->needToUpdateVertex = false;
							uiDrawcall->vertexPositionChanged = false;
						}
					}
					break;
					case EUIDrawcallType::PostProcess:
					{
						//post process cannot update spicific drawcall, because every post process is a drawcall
					}
					break;
					}
				}
			}
			if (UIItem->cacheForThisUpdate_DepthChanged || needToSortRenderPriority)
			{
				RootCanvas->bNeedToSortRenderPriority = true;
			}
		}
		//create or update material
		UpdateAndApplyMaterial();
	}
	if (this == RootCanvas)//child canvas is already updated before this, so after all update, the topmost canvas should start the sort function
	{
		if (bNeedToSortRenderPriority)
		{
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					switch (this->GetRenderMode())
					{
					default:
					case ELGUIRenderMode::ScreenSpaceOverlay:
						ULGUIEditorManagerObject::Instance->MarkSortScreenSpaceCanvas();
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
						Instance->MarkSortScreenSpaceCanvas();
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


	//this frame is complete
	bRectRangeCalculated = false;
	bNeedToSortRenderPriority = false;
}
void ULGUICanvas::UpdateCanvasGeometryForAutoManageDepth()
{
	for (auto item : manageCanvasArray)
	{
		if (item == this)continue;//skip self
		if (item->autoManageDepth)
		{
			item->UpdateCanvasGeometryForAutoManageDepth();
		}
		else
		{
			item->UpdateCanvasGeometry();
		}
	}

	//update drawcall
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcall);
		//check valid renderable, incase unnormally deleting actor, like undo
		{
			for (int i = UIRenderableItemList.Num() - 1; i >= 0; i--)
			{
				if (!UIRenderableItemList[i].IsValid())
				{
					UIRenderableItemList.RemoveAt(i);
				}
			}
		}
		//sort on flatten hierarchy
		UIRenderableItemList.Sort([](const TWeakObjectPtr<UUIBaseRenderable>& A, const TWeakObjectPtr<UUIBaseRenderable>& B)
			{
				return A->GetFlattenHierarchyIndex() < B->GetFlattenHierarchyIndex();
			});
		UUIDrawcall::CreateDrawcallForAutoManageDepth(UIRenderableItemList, TempUIDrawcallList);//create drawcall
		CacheUIItemToCanvasTransformMap.Reset();
		//compare drawcall
		auto drawcallsEqual = UUIDrawcall::CompareDrawcallList(TempUIDrawcallList, UIDrawcallList);
		cacheForThisUpdate_ShouldRebuildAllDrawcall = (!drawcallsEqual) || cacheForThisUpdate_ShouldRebuildAllDrawcall;
		if (cacheForThisUpdate_ShouldRebuildAllDrawcall)
		{
			UUIDrawcall::CopyDrawcallList(TempUIDrawcallList, UIDrawcallList);
			//create UIMesh based on Drawcall
			int drawcallCount = UIDrawcallList.Num();
			for (auto item : UIDrawcallPrimitiveList)
			{
				if (item.UIPostProcessRenderable.IsValid())
				{
					item.UIPostProcessRenderable.Pin()->RemoveFromHudRenderer();
					item.UIPostProcessRenderable.Reset();
				}
			}
			UIDrawcallPrimitiveList.SetNum(drawcallCount);
			int meshIndex = 0;
			for (int i = 0; i < drawcallCount; i++)
			{
				switch (UIDrawcallList[i]->type)
				{
				default:
				case EUIDrawcallType::BatchGeometry:
				{
					UUIDrawcallMesh* uiMesh = nullptr;
					if (meshIndex < CacheUIMeshList.Num())//get mesh from exist
					{
						if (CacheUIMeshList[meshIndex].IsValid())
						{
							uiMesh = CacheUIMeshList[meshIndex].Get();
						}
					}
					else//create mesh
					{
#if WITH_EDITOR
						auto meshName = FString::Printf(TEXT("%s_Drawcall_%d"), *GetOwner()->GetActorLabel(), meshIndex);
#else
						auto meshName = FString::Printf(TEXT("Drawcall_%d"), meshIndex);
#endif
						uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), FName(*meshName), RF_Transient);
						uiMesh->SetOwnerNoSee(this->GetActualOwnerNoSee());
						uiMesh->SetOnlyOwnerSee(this->GetActualOnlyOwnerSee());
						uiMesh->RegisterComponent();
						//this->GetOwner()->AddInstanceComponent(uiMesh);
						uiMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
						uiMesh->SetRelativeTransform(FTransform::Identity);
						CacheUIMeshList.Add(uiMesh);
#if WITH_EDITOR
						if (!GetWorld()->IsGameWorld())
						{
							if (currentIsRenderToRenderTargetOrWorld)
							{
								uiMesh->SetSupportScreenSpace(true, RootCanvas->GetViewExtension());
							}
							uiMesh->SetSupportWorldSpace(true);
						}
						else
#endif
						if (currentIsRenderToRenderTargetOrWorld)
						{
							uiMesh->SetSupportScreenSpace(true, RootCanvas->GetViewExtension());
							uiMesh->SetSupportWorldSpace(false);
						}
					}
					//set data for mesh
					if (uiMesh != nullptr)
					{
						uiMesh->SetUIMeshVisibility(true);//some UIMesh may set to invisible on prev frame, set to visible
						auto& meshSection = uiMesh->MeshSection;
						meshSection.Reset();
						UIDrawcallList[i]->GetCombined(meshSection.vertices, meshSection.triangles);
						uiMesh->GenerateOrUpdateMesh(true, GetActualAdditionalShaderChannelFlags());
					}

					UIDrawcallPrimitiveList[i].UIDrawcallMesh = uiMesh;

					UIDrawcallPrimitiveList[i].UIPostProcessRenderable = nullptr;

					meshIndex++;
				}
				break;
				case EUIDrawcallType::PostProcess:
				{
					auto uiPostProcessPrimitive = UIDrawcallList[i]->postProcessObject->GetRenderProxy();
					uiPostProcessPrimitive.Pin()->AddToHudRenderer(RootCanvas->GetViewExtension());
					uiPostProcessPrimitive.Pin()->SetVisibility(true);

					UIDrawcallPrimitiveList[i].UIPostProcessRenderable = uiPostProcessPrimitive;

					UIDrawcallPrimitiveList[i].UIDrawcallMesh = nullptr;
				}
				break;
				}
			}

			if (CacheUIMeshList.Num() > meshIndex)//set needless mesh invisible
			{
				for (int i = meshIndex; i < CacheUIMeshList.Num(); i++)
				{
					if (CacheUIMeshList[i].IsValid())
					{
						CacheUIMeshList[i]->SetUIMeshVisibility(false);
					}
				}
			}

			//after geometry created, need to sort UIMesh render order
			RootCanvas->bNeedToSortRenderPriority = true;
		}
		else//no need to rebuild all drawcall
		{
			int drawcallCount = UIDrawcallList.Num();
			for (int i = 0; i < drawcallCount; i++)
			{
				auto uiDrawcall = UIDrawcallList[i];
				if (!uiDrawcall.IsValid())continue;
				if (uiDrawcall->type == EUIDrawcallType::BatchGeometry)
				{
					auto uiMesh = UIDrawcallPrimitiveList[i].UIDrawcallMesh;
					if (!uiMesh.IsValid())continue;
					if (uiDrawcall->needToBeRebuild)
					{
						//sort needed for new add item
						uiDrawcall->geometryList.Sort([](const TSharedPtr<UIGeometry>& A, const TSharedPtr<UIGeometry>& B)
							{
								return A->depth < B->depth;
							});

						auto& meshSection = uiMesh->MeshSection;
						meshSection.Reset();
						uiDrawcall->GetCombined(meshSection.vertices, meshSection.triangles);
						uiMesh->GenerateOrUpdateMesh(true, GetActualAdditionalShaderChannelFlags());
						uiDrawcall->needToBeRebuild = false;
						uiDrawcall->needToUpdateVertex = false;
					}
					else if (uiDrawcall->needToUpdateVertex)
					{
						auto& meshSection = uiMesh->MeshSection;
						uiDrawcall->UpdateData(meshSection.vertices, meshSection.triangles);
						uiMesh->GenerateOrUpdateMesh(uiDrawcall->vertexPositionChanged, GetActualAdditionalShaderChannelFlags());
						uiDrawcall->needToUpdateVertex = false;
						uiDrawcall->vertexPositionChanged = false;
					}
				}
				else if (uiDrawcall->type == EUIDrawcallType::PostProcess)
				{
					//post process cannot update spicific drawcall, because every post process is a drawcall
				}
			}
		}
		//create or update material
		UpdateAndApplyMaterial();
	}
	if (this == RootCanvas)//child canvas is already updated before this, so after all update, the topmost canvas should start the sort function
	{
		if (bNeedToSortRenderPriority)
		{
#if WITH_EDITOR
			if (!GetWorld()->IsGameWorld())
			{
				if (ULGUIEditorManagerObject::Instance != nullptr)
				{
					switch (this->GetRenderMode())
					{
					default:
					case ELGUIRenderMode::ScreenSpaceOverlay:
						ULGUIEditorManagerObject::Instance->MarkSortScreenSpaceCanvas();
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
						Instance->MarkSortScreenSpaceCanvas();
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


	//this frame is complete
	bRectRangeCalculated = false;
	bNeedToSortRenderPriority = false;
}
const TArray<TWeakObjectPtr<ULGUICanvas>>& ULGUICanvas::GetAllCanvasArray()
{
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				return ULGUIEditorManagerObject::Instance->GetCanvasArray();
			}
		}
		else
#endif
		{
			if (auto LGUIManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(world))
			{
				return LGUIManagerActor->GetCanvasArray();
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Error, TEXT("[ULGUICanvas::GetAllCanvasArray]World is null, this is wierd"));
	}
	static TArray<TWeakObjectPtr<ULGUICanvas>> staticArray;//just for the return value
	return staticArray;
}

int32 ULGUICanvas::SortDrawcall(int32 InStartRenderPriority)
{
	for (int i = 0; i < UIDrawcallPrimitiveList.Num(); i++)
	{
		if (UIDrawcallPrimitiveList[i].UIDrawcallMesh.IsValid())
		{
			UIDrawcallPrimitiveList[i].UIDrawcallMesh->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
		if (UIDrawcallPrimitiveList[i].UIPostProcessRenderable.IsValid())
		{
			UIDrawcallPrimitiveList[i].UIPostProcessRenderable.Pin()->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
	}
	return UIDrawcallPrimitiveList.Num();
}

void ULGUICanvas::UpdateAndApplyMaterial()
{
	int drawcallCount = UIDrawcallList.Num();
	//if clip type change, or need to rebuild all drawcall, then recreate material
	if (cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_ShouldRebuildAllDrawcall)
	{
		for (auto mat : UIMaterialList)
		{
			if (IsValid(mat))
			{
				AddUIMaterialToCache(mat);
			}
		}
		UIMaterialList.Reset();
	}
	//check if material count is enough, or create enough material
	int materialCount = UIMaterialList.Num();
	auto tempClipType = GetActualClipType();
	if (materialCount < drawcallCount)
	{
		cacheForThisUpdate_ClipTypeChanged = true;//mark to true, set materials clip property
		UIMaterialList.AddUninitialized(drawcallCount - materialCount);
		for (int i = materialCount; i < drawcallCount; i++)
		{
			auto uiDrawcall = UIDrawcallList[i];
			if (uiDrawcall->type == EUIDrawcallType::BatchGeometry)
			{
				UMaterialInstanceDynamic* uiMat = nullptr;
				if (uiDrawcall->material.IsValid())//custom material
				{
					auto SrcMaterial = uiDrawcall->material.Get();
					if (SrcMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
					{
						uiMat = (UMaterialInstanceDynamic*)SrcMaterial;
					}
					else
					{
						uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
						uiMat->SetFlags(RF_Transient);
					}
				}
				else
				{
					uiMat = GetUIMaterialFromCache(clipType);
				}
				uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture.Get());
				UIMaterialList[i] = uiMat;
				uiDrawcall->materialInstanceDynamic = uiMat;
			}
			else if (uiDrawcall->type == EUIDrawcallType::PostProcess)
			{
				UIMaterialList[i] = nullptr;
				if (uiDrawcall->postProcessObject.IsValid())
				{
					uiDrawcall->postProcessObject->SetClipType(tempClipType);
				}
			}
			else if (uiDrawcall->type == EUIDrawcallType::DirectMesh)
			{
				UIMaterialList[i] = nullptr;
			}
		}
	}
	if (cacheForThisUpdate_ShouldRebuildAllDrawcall)
	{
		for (int i = 0; i < drawcallCount; i++)//set material property
		{
			auto uiDrawcall = UIDrawcallList[i];
			if (uiDrawcall->type == EUIDrawcallType::BatchGeometry)
			{
				auto uiMat = UIMaterialList[i];
				if (IsValid(uiMat))
				{
					uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture.Get());
					auto uiMesh = UIDrawcallPrimitiveList[i].UIDrawcallMesh;
					if (!uiMesh.IsValid())continue;
					uiMesh->SetMaterial(0, uiMat);
				}
			}
		}
	}
	//set clip parameter
	switch (tempClipType)
	{
	default:
	case ELGUICanvasClipType::None:
	{
		SetParameterForStandard(drawcallCount);
	}
	break;
	case ELGUICanvasClipType::Rect:
	{
		if (cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_RectClipParameterChanged)
		{
			SetParameterForRectClip(drawcallCount);
		}
	}
	break;
	case ELGUICanvasClipType::Texture:
	{
		if (cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_TextureClipParameterChanged)
		{
			SetParameterForTextureClip(drawcallCount);
		}
	}
	break;
	}
}

UMaterialInstanceDynamic* ULGUICanvas::GetUIMaterialFromCache(ELGUICanvasClipType inClipType)
{
	if (CacheUIMaterialList.Num() == 0)
	{
		CacheUIMaterialList.Add({});
		CacheUIMaterialList.Add({});
		CacheUIMaterialList.Add({});
	}
	auto& matList = CacheUIMaterialList[(int)inClipType].MaterialList;
	if (matList.Num() == 0)
	{
		auto SrcMaterial = GetMaterials()[(int)inClipType];
		auto uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
		uiMat->SetFlags(RF_Transient);
		return uiMat;
	}
	else
	{
		auto uiMat = matList[matList.Num() - 1];
		matList.RemoveAt(matList.Num() - 1);
		return uiMat;
	}
}
void ULGUICanvas::AddUIMaterialToCache(UMaterialInstanceDynamic* uiMat)
{
	int cacheMatTypeIndex = -1;
	auto defaultMaterials = GetMaterials();
	for (int i = 0; i < 3; i++)
	{
		if (uiMat->Parent == defaultMaterials[i])
		{
			cacheMatTypeIndex = i;
			break;
		}
	}
	if (cacheMatTypeIndex != -1)
	{
		auto& matList = CacheUIMaterialList[cacheMatTypeIndex].MaterialList;
		matList.Add(uiMat);
	}
}

void ULGUICanvas::SetParameterForStandard(int drawcallCount)
{

}
void ULGUICanvas::SetParameterForRectClip(int drawcallCount)
{
	auto rectClipOffsetAndSize = this->GetRectClipOffsetAndSize();
	auto rectClipFeather = this->GetRectClipFeather();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto uiDrawcall = UIDrawcallList[i];
		if (uiDrawcall->type == EUIDrawcallType::BatchGeometry)
		{
			auto uiMat = UIMaterialList[i];
			if (IsValid(uiMat))
			{
				uiMat->SetVectorParameterValue(FName("RectClipOffsetAndSize"), rectClipOffsetAndSize);
				uiMat->SetVectorParameterValue(FName("RectClipFeather"), rectClipFeather);
			}
		}
		else if(uiDrawcall->type == EUIDrawcallType::PostProcess)
		{
			if (uiDrawcall->postProcessObject.IsValid())
			{
				uiDrawcall->postProcessObject->SetRectClipParameter(rectClipOffsetAndSize, rectClipFeather);
			}
		}
	}
}
void ULGUICanvas::SetParameterForTextureClip(int drawcallCount)
{
	auto textureClipOffsetAndSize = this->GetTextureClipOffsetAndSize();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto uiDrawcall = UIDrawcallList[i];
		if (uiDrawcall->type == EUIDrawcallType::BatchGeometry)
		{
			auto uiMat = UIMaterialList[i];
			if (IsValid(uiMat))
			{
				uiMat->SetTextureParameterValue(FName("ClipTexture"), clipTexture);
				uiMat->SetVectorParameterValue(FName("TextureClipOffsetAndSize"), textureClipOffsetAndSize);
			}
		}
		else if (uiDrawcall->type == EUIDrawcallType::PostProcess)
		{
			if (uiDrawcall->postProcessObject.IsValid())
			{
				uiDrawcall->postProcessObject->SetTextureClipParameter(clipTexture, textureClipOffsetAndSize);
			}
		}
	}
}
bool ULGUICanvas::IsPointVisible(FVector worldPoint)
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
		//transform to local space
		auto localPoint = this->UIItem->GetComponentTransform().InverseTransformPosition(worldPoint);
		//out of range
		if (localPoint.X < clipRectMin.X) return false;
		if (localPoint.Y < clipRectMin.Y) return false;
		if (localPoint.X > clipRectMax.X) return false;
		if (localPoint.Y > clipRectMax.Y) return false;
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
		if (GetActualClipType() == ELGUICanvasClipType::Rect)
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
				auto parentRectMin = FVector(ParentCanvas->clipRectMin, 0);
				auto parentRectMax = FVector(ParentCanvas->clipRectMax, 0);
				//transform ParentCanvas's rect to this space
				auto& parentCanvasTf = ParentCanvas->UIItem->GetComponentTransform();
				auto thisTfInv = this->UIItem->GetComponentTransform().Inverse();
				parentRectMin = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMin));
				parentRectMax = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMax));
				//inherit
				if (clipRectMin.X < parentRectMin.X)clipRectMin.X = parentRectMin.X;
				if (clipRectMin.Y < parentRectMin.Y)clipRectMin.Y = parentRectMin.Y;
				if (clipRectMax.X > parentRectMax.X)clipRectMax.X = parentRectMax.X;
				if (clipRectMax.Y > parentRectMax.Y)clipRectMax.Y = parentRectMax.Y;
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
	return FLinearColor(clipFeather.X, clipFeather.Y, 0, 0);
}
FLinearColor ULGUICanvas::GetTextureClipOffsetAndSize()
{
	auto& widget = UIItem->widget;
	return FLinearColor(widget.width * -widget.pivot.X, widget.height * -widget.pivot.Y, widget.width, widget.height);
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
void ULGUICanvas::SetSortOrder(int32 newSortOrder, bool propagateToChildrenCanvas)
{
	if (sortOrder != newSortOrder)
	{
		int32 diff = newSortOrder - sortOrder;
		sortOrder = newSortOrder;
		MarkCanvasUpdate();
		if (propagateToChildrenCanvas)
		{
			auto& allCanvasArray = GetAllCanvasArray();
			for (auto itemCanvas : allCanvasArray)
			{
				if (itemCanvas.IsValid())
				{
					if (itemCanvas->UIItem->IsAttachedTo(this->UIItem.Get()))
					{
						itemCanvas->sortOrder += diff;
					}
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
					ULGUIEditorManagerObject::Instance->MarkSortScreenSpaceCanvas();
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
					Instance->MarkSortScreenSpaceCanvas();
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
void ULGUICanvas::SetSortOrderToHighestOfHierarchy(bool propagateToChildrenCanvas)
{
	if (CheckRootCanvas())
	{
		int32 maxSortOrder = 0;
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		for (auto itemCanvas : allCanvasArray)
		{
			if (itemCanvas.IsValid() && itemCanvas != this && itemCanvas->GetIsUIActive())
			{
				if (itemCanvas->RootCanvas == this->RootCanvas)//on the same hierarchy
				{
					if (propagateToChildrenCanvas)//if propergate to children, then ignore children canvas
					{
						if (this->manageCanvasArray.Contains(itemCanvas))
						{
							continue;
						}
					}
					if (maxSortOrder < itemCanvas->sortOrder)
					{
						maxSortOrder = itemCanvas->sortOrder;
					}
				}
			}
		}
		int32 desireSortOrder = maxSortOrder + 1;
		SetSortOrder(desireSortOrder, propagateToChildrenCanvas);
	}
}
void ULGUICanvas::SetSortOrderToLowestOfHierarchy(bool propagateToChildrenCanvas)
{
	if (CheckRootCanvas())
	{
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		int32 minSortOrder = 0;
		for (auto itemCanvas : allCanvasArray)
		{
			if (itemCanvas.IsValid() && itemCanvas != this && itemCanvas->GetIsUIActive())
			{
				if (itemCanvas->RootCanvas == this->RootCanvas)//on the same hierarchy
				{
					if (propagateToChildrenCanvas)//if propergate to children, then ignore children canvas
					{
						if (this->manageCanvasArray.Contains(itemCanvas))
						{
							continue;
						}
					}
					if (minSortOrder > itemCanvas->sortOrder)
					{
						minSortOrder = itemCanvas->sortOrder;
					}
				}
			}
		}
		int32 desireSortOrder = minSortOrder - 1;
		SetSortOrder(desireSortOrder, propagateToChildrenCanvas);
	}
}
void ULGUICanvas::SetSortOrderToHighestOfAll(bool propagateToChildrenCanvas)
{
	if (CheckRootCanvas())
	{
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		int32 maxSortOrder = 0;
		for (auto itemCanvas : allCanvasArray)
		{
			if (itemCanvas.IsValid() && itemCanvas != this && itemCanvas->GetIsUIActive())
			{
				if (propagateToChildrenCanvas)//if propergate to children, then ignore children canvas
				{
					if (this->manageCanvasArray.Contains(itemCanvas))
					{
						continue;
					}
				}
				if (maxSortOrder < itemCanvas->sortOrder)
				{
					maxSortOrder = itemCanvas->sortOrder;
				}
			}
		}
		int32 desireSortOrder = maxSortOrder + 1;
		SetSortOrder(desireSortOrder, propagateToChildrenCanvas);
	}
}
void ULGUICanvas::SetSortOrderToLowestOfAll(bool propagateToChildrenCanvas)
{
	if (CheckRootCanvas())
	{
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		int32 minDepth = 0;
		for (auto itemCanvas : allCanvasArray)
		{
			if (itemCanvas.IsValid() && itemCanvas != this && itemCanvas->GetIsUIActive())
			{
				if (propagateToChildrenCanvas)//if propergate to children, then ignore children canvas
				{
					if (this->manageCanvasArray.Contains(itemCanvas))
					{
						continue;
					}
				}
				if (minDepth > itemCanvas->sortOrder)
				{
					minDepth = itemCanvas->sortOrder;
				}
			}
		}
		int32 desireDepth = minDepth - 1;
		SetSortOrder(desireDepth, propagateToChildrenCanvas);
	}
}


void ULGUICanvas::SetDefaultMaterials(UMaterialInterface* InMaterials[3])
{
	MarkRebuildAllDrawcall();
	for (int i = 0; i < 3; i++)
	{
		if (IsValid(InMaterials[i]))
		{
			DefaultMaterials[i] = InMaterials[i];
		}
	}
	//clear old material
	UIMaterialList.Reset();
	CacheUIMaterialList.Reset();
}

void ULGUICanvas::SetDynamicPixelsPerUnit(float newValue)
{
	if (dynamicPixelsPerUnit != newValue)
	{
		dynamicPixelsPerUnit = newValue;
		MarkRebuildAllDrawcall();
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


void ULGUICanvas::BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float InFOV, float InOrthoWidth, float InOrthoHeight, FMatrix& OutProjectionMatrix)const
{
	if (InViewportSize.X == 0 || InViewportSize.Y == 0)//in DebugCamera mode(toggle in editor by press ';'), viewport size is 0
	{
		InViewportSize.X = InViewportSize.Y = 1;
	}
	if (InProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float tempOrthoWidth = InOrthoWidth / 2.0f;
		const float tempOrthoHeight = InOrthoHeight / 2.0f;

		const float ZScale = 1.0f / (FarClipPlane - NearClipPlane);
		const float ZOffset = -NearClipPlane;

		OutProjectionMatrix = FReversedZOrthoMatrix(
			tempOrthoWidth,
			tempOrthoHeight,
			ZScale,
			ZOffset
		);
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
	if (cacheViewProjectionMatrixFrameNumber != GFrameNumber)
	{
		if (!CheckUIItem())
		{
			UE_LOG(LGUI, Error, TEXT("[LGUICanvas::GetViewProjectionMatrix]UIItem not valid!"));
			return cacheViewProjectionMatrix;
		}
		cacheViewProjectionMatrixFrameNumber = GFrameNumber;

		FVector ViewLocation = GetViewLocation();
		auto Transform = UIItem->GetComponentToWorld();
		Transform.SetTranslation(FVector::ZeroVector);
		Transform.SetScale3D(FVector::OneVector);
		FMatrix ViewRotationMatrix = Transform.ToInverseMatrixWithScale();

		const float FOV = FOVAngle * (float)PI / 360.0f;

		FMatrix ProjectionMatrix;
		BuildProjectionMatrix(GetViewportSize(), ProjectionType, FOV, UIItem->GetWidth(), UIItem->GetHeight(), ProjectionMatrix);
		cacheViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
	}
	return cacheViewProjectionMatrix;
}
FMatrix ULGUICanvas::GetProjectionMatrix()const
{
	FMatrix ProjectionMatrix = FMatrix::Identity;
	const float FOV = FOVAngle * (float)PI / 360.0f;
	BuildProjectionMatrix(GetViewportSize(), ProjectionType, FOV, UIItem->GetWidth(), UIItem->GetHeight(), ProjectionMatrix);
	return ProjectionMatrix;
}
FVector ULGUICanvas::GetViewLocation()const
{
	return UIItem->GetComponentLocation() - UIItem->GetUpVector() * CalculateDistanceToCamera();
}
FMatrix ULGUICanvas::GetViewRotationMatrix()const
{
	auto Transform = UIItem->GetComponentToWorld();
	Transform.SetTranslation(FVector::ZeroVector);
	Transform.SetScale3D(FVector::OneVector);
	return Transform.ToInverseMatrixWithScale();
}
FRotator ULGUICanvas::GetViewRotator()const
{
	return (UIItem->GetComponentQuat() * FQuat::MakeFromEuler(FVector(90, 90, 0))).Rotator();
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
		MarkRebuildAllDrawcall();
		if (renderMode == ELGUIRenderMode::WorldSpace)
		{
			if (ViewExtension.IsValid())
			{
				ViewExtension.Reset();
			}
		}
	}
}

void ULGUICanvas::SetPixelPerfect(bool value)
{
	if (pixelPerfect != value)
	{
		pixelPerfect = value;
		MarkCanvasUpdate();
	}
}

void ULGUICanvas::SetProjectionParameters(TEnumAsByte<ECameraProjectionMode::Type> InProjectionType, float InFovAngle, float InNearClipPlane, float InFarClipPlane)
{
	ProjectionType = InProjectionType;
	FOVAngle = InFovAngle;
	NearClipPlane = InNearClipPlane;
	FarClipPlane = InFarClipPlane;
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
				if (ViewExtension.IsValid())
				{
					ViewExtension.Reset();
				}
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
		return this->currentIsRenderToRenderTargetOrWorld
			&& pixelPerfect;
	}
	else
	{
		if (RootCanvas.IsValid())
		{
			if (GetOverridePixelPerfect())
			{
				return RootCanvas->currentIsRenderToRenderTargetOrWorld
					&& this->pixelPerfect;
			}
			else
			{
				if (ParentCanvas.IsValid())
				{
					return RootCanvas->currentIsRenderToRenderTargetOrWorld
						&& ParentCanvas->GetActualPixelPerfect();
				}
			}
		}
	}
	return false;
}
bool ULGUICanvas::GetActualOwnerNoSee()const
{
	if (IsRootCanvas())
	{
		return this->ownerNoSee;
	}
	else
	{
		if (GetOverrideOwnerNoSee())
		{
			return this->ownerNoSee;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualOwnerNoSee();
			}
		}
	}
	return this->ownerNoSee;
}
bool ULGUICanvas::GetActualOnlyOwnerSee()const
{
	if (IsRootCanvas())
	{
		return this->onlyOwnerSee;
	}
	else
	{
		if (GetOverrideOnlyOwnerSee())
		{
			return this->onlyOwnerSee;
		}
		else
		{
			if (ParentCanvas.IsValid())
			{
				return ParentCanvas->GetActualOnlyOwnerSee();
			}
		}
	}
	return this->onlyOwnerSee;
}
void ULGUICanvas::SetOwnerNoSee(bool value)
{
	if (ownerNoSee != value)
	{
		ownerNoSee = value;
		ApplyOwnerSeeRecursive();
	}
}
void ULGUICanvas::SetOnlyOwnerSee(bool value)
{
	if (onlyOwnerSee != value)
	{
		onlyOwnerSee = value;
		ApplyOwnerSeeRecursive();
	}
}
void ULGUICanvas::ApplyOwnerSeeRecursive()
{
	for (auto uiMesh : CacheUIMeshList)
	{
		if (uiMesh.IsValid())
		{
			uiMesh->SetOwnerNoSee(this->GetActualOwnerNoSee());
			uiMesh->SetOnlyOwnerSee(this->GetActualOnlyOwnerSee());
		}
	}

	for (auto item : manageCanvasArray)
	{
		if (item.IsValid() && item != this)
		{
			item->ApplyOwnerSeeRecursive();
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
			return true;
		}
		return false;
	}
}
FTransform2D ULGUICanvas::ConvertTo2DTransform(const FTransform& Transform)
{
	auto itemToCanvasMatrix = Transform.ToMatrixWithScale();
	auto itemToCanvasTf2D = FTransform2D(FMatrix2x2(itemToCanvasMatrix.M[0][0], itemToCanvasMatrix.M[0][1], itemToCanvasMatrix.M[1][0], itemToCanvasMatrix.M[1][1]), FVector2D(Transform.GetLocation()));
	return itemToCanvasTf2D;
}
void ULGUICanvas::CalculateUIItem2DBounds(UUIItem* item, const FTransform2D& transform, FVector2D& min, FVector2D& max)
{
	FVector2D localPoint1, localPoint2;
	item->GetLocalSpaceMinMaxPoint_ForAutoManageDepth(localPoint1, localPoint2);
	auto point1 = transform.TransformPoint(localPoint1);
	auto point2 = transform.TransformPoint(localPoint2);
	auto point3 = transform.TransformPoint(FVector2D(localPoint2.X, localPoint1.Y));
	auto point4 = transform.TransformPoint(FVector2D(localPoint1.X, localPoint2.Y));

	GetMinMax(point1.X, point2.X, point3.X, point4.X, min.X, max.X);
	GetMinMax(point1.Y, point2.Y, point3.Y, point4.Y, min.Y, max.Y);
}

void ULGUICanvas::GetMinMax(float a, float b, float c, float d, float& min, float& max)
{
	float abMin = FMath::Min(a, b);
	float abMax = FMath::Max(a, b);
	float cdMin = FMath::Min(c, d);
	float cdMax = FMath::Max(c, d);
	min = FMath::Min(abMin, cdMin);
	max = FMath::Max(abMax, cdMax);
}
