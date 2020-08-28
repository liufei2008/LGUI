// Copyright 2019-2020 LexLiu. All Rights Reserved.

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
#include "Core/ActorComponent/UIRenderable.h"
#include "Core/ActorComponent/UIItem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "SceneViewExtension.h"
#include "Engine.h"

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
	TopMostCanvas = nullptr;
	CheckTopMostCanvas();
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

	if (this->IsDefaultSubobject())
	{
		ALGUIManagerActor::AddCanvas(this);
	}
	else
	{
		ALGUIManagerActor::SortCanvasOnOrder();
	}
}
void ULGUICanvas::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ULGUICanvas::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULGUICanvas::CustomTick(float DeltaTime)
{
	if (bCanTickUpdate)
	{
		bCanTickUpdate = false;
		if (CheckUIItem() && UIItem->IsUIActiveInHierarchy())
		{
			this->UpdateTopMostCanvas();
		}
	}
}

void ULGUICanvas::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (auto world = this->GetWorld())
	{
		if (!world->IsGameWorld())
		{
			ULGUIEditorManagerObject::AddCanvas(this);
		}
		else
		{
			ALGUIManagerActor::AddCanvas(this);
		}
	}
#else
	if (!this->IsDefaultSubobject())//if is default subobject, then call AddCanvas in BeginPlay, because sortOrder value may not set if use PrefabSystem
	{
		ALGUIManagerActor::AddCanvas(this);
	}
#endif
	OnUIHierarchyChanged();
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
	OnUIHierarchyChanged();
	//tell UIItem
	if (IsValid(UIItem))
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
	if (UIMeshList.Num() > 0)
	{
		for (UUIDrawcallMesh* item : UIMeshList)
		{
			item->DestroyComponent();
		}
	}
}

void ULGUICanvas::OnUIActiveStateChange(bool active)
{
	for (auto uiMesh : UIMeshList)
	{
		uiMesh->SetVisibility(active);
	}
}

bool ULGUICanvas::CheckTopMostCanvas()
{
	if (IsValid(TopMostCanvas))return true;
	if (this->GetWorld() == nullptr)return false;
	LGUIUtils::FindTopMostCanvas(this->GetOwner(), TopMostCanvas);
	if (IsValid(TopMostCanvas))return true;
	return false;
}
bool ULGUICanvas::CheckParentCanvas()
{
	if (IsValid(ParentCanvas))return true;
	if (this->GetWorld() == nullptr)return false;
	LGUIUtils::FindParentCanvas(this->GetOwner(), ParentCanvas);
	if (IsValid(ParentCanvas))return true;
	return false;
}
bool ULGUICanvas::CheckUIItem()
{
	if (IsValid(UIItem))return true;
	if (this->GetWorld() == nullptr)return false;
	UIItem = Cast<UUIItem>(GetOwner()->GetRootComponent());
	if (!IsValid(UIItem))
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
	if (IsValid(TopMostCanvas))
	{
		currentIsRenderToRenderTargetOrWorld = TopMostCanvas->IsRenderToScreenSpaceOrRenderTarget();
	}

	//if hierarchy changed from World/Hud to Hud/World, then we need to recreate all
	if (currentIsRenderToRenderTargetOrWorld != oldIsRenderToRenderTargetOrWorld)
	{
		if (CheckUIItem())
		{
			UIItem->MarkAllDirtyRecursive();
		}
		//clear UIMeshList and delete mesh components, so new mesh will be created. because hud and world mesh not compatible
		for (auto uiMesh : UIMeshList)
		{
			uiMesh->DestroyComponent();
		}
		UIMeshList.Reset();
	}
}
void ULGUICanvas::OnUIHierarchyChanged()
{
	//recheck top most canvas
	TopMostCanvas = nullptr;
	CheckTopMostCanvas();

	CheckRenderMode();

	ParentCanvas = nullptr;
	CheckParentCanvas();
	//rebuild drawcall
	MarkRebuildAllDrawcall();
}
bool ULGUICanvas::IsScreenSpaceOverlayUI()
{
	if (IsValid(TopMostCanvas))
	{
		return TopMostCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay;
	}
	return false;
}
bool ULGUICanvas::IsRenderToScreenSpace()
{
	if (IsValid(TopMostCanvas))
	{
		return TopMostCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay;
	}
	return false;
}
bool ULGUICanvas::IsRenderToScreenSpaceOrRenderTarget()
{
	if (IsValid(TopMostCanvas))
	{
		return TopMostCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay
			|| (TopMostCanvas->renderMode == ELGUIRenderMode::RenderTarget && IsValid(TopMostCanvas->renderTarget))
				;
	}
	return false;
}
bool ULGUICanvas::IsRenderToRenderTarget()
{
	if (IsValid(TopMostCanvas))
	{
		return TopMostCanvas->renderMode == ELGUIRenderMode::RenderTarget && IsValid(TopMostCanvas->renderTarget);
	}
	return false;
}
bool ULGUICanvas::IsRenderToWorldSpace()
{
	if (IsValid(TopMostCanvas))
	{
		return TopMostCanvas->renderMode == ELGUIRenderMode::WorldSpace;
	}
	return false;
}

void ULGUICanvas::MarkCanvasUpdate()
{
	this->bCanTickUpdate = true;
	if (CheckTopMostCanvas())
	{
		TopMostCanvas->bCanTickUpdate = true;//incase this Canvas's parent have layout component, so mark TopMostCanvas to update
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
	if (CheckTopMostCanvas())
	{
		TopMostCanvas->MarkCanvasUpdate();
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
					UE_LOG(LGUI, Error, TEXT("[ULGUICanvas/CheckMaterials]Assign material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
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
			if (IsValid(ParentCanvas))
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


ULGUICanvas* ULGUICanvas::GetRootCanvas() 
{ 
	CheckTopMostCanvas(); 
	return TopMostCanvas; 
}
bool ULGUICanvas::IsRootCanvas()const
{
	return TopMostCanvas == this;
}

void ULGUICanvas::MarkRebuildAllDrawcall()
{
	bShouldRebuildAllDrawcall = true;
}
void ULGUICanvas::MarkRebuildSpecificDrawcall(int drawcallIndex)
{
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())
	{
		//UE_LOG(LGUI, Log, TEXT("MarkRebuildSpecificDrawcall rebuildDrawcall"));
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
		//UE_LOG(LGUI, Log, TEXT("SetDrawcallTexture rebuildDrawcall"));
		bShouldRebuildAllDrawcall = true;
		return;
	}
	auto& uiDrawcall = UIDrawcallList[drawcallIndex];
	uiDrawcall->texture = drawcallTexture;
	//material
	auto& uiMat = UIMaterialList[drawcallIndex];
	uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture.Get());
}
void ULGUICanvas::MarkUpdateSpecificDrawcallVertex(int drawcallIndex, bool vertexPositionChanged)
{
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())
	{
		//UE_LOG(LGUI, Log, TEXT("MarkUpdateSpecificDrawcallVertex rebuildDrawcall"));
		bShouldRebuildAllDrawcall = true;
		return;
	}
	auto& uiDrawcall = UIDrawcallList[drawcallIndex];
	uiDrawcall->needToUpdateVertex = true;
	if (vertexPositionChanged)uiDrawcall->vertexPositionChanged = vertexPositionChanged;
}
void ULGUICanvas::OnUIElementDepthChange(UUIRenderable* item)
{
	//shouldRebuildAllDrawcall = true; return;
	auto drawcallIndex = item->GetGeometry()->drawcallIndex;
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())//drawcall index not inside list, need to rebuild all drawcall
	{
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

TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> ULGUICanvas::GetViewExtension()
{
	if (!ViewExtension.IsValid())
	{
		if (GEngine)
		{
			if (renderMode == ELGUIRenderMode::ScreenSpaceOverlay)
			{
				ViewExtension = FSceneViewExtensions::NewExtension<FLGUIViewExtension>(this, nullptr);
			}
			else if (renderMode == ELGUIRenderMode::RenderTarget && IsValid(renderTarget))
			{
				ViewExtension = FSceneViewExtensions::NewExtension<FLGUIViewExtension>(this, renderTarget);
			}
		}
	}
	return ViewExtension;
}

void ULGUICanvas::AddUIRenderable(UUIRenderable* InUIRenderable)
{
	InsertIntoDrawcall(InUIRenderable);
	UIRenderableItemList.Add(InUIRenderable);
}
void ULGUICanvas::RemoveUIRenderable(UUIRenderable* InUIRenderable)
{
	RemoveFromDrawcall(InUIRenderable);
	UIRenderableItemList.Remove(InUIRenderable);
}
void ULGUICanvas::InsertIntoDrawcall(UUIRenderable* item)
{
	bool accommodate = false;//can this item fit into list?
	auto itemDepth = item->GetDepth();
	auto itemTexture = item->GetGeometry()->texture;
	int drawcallCount = UIDrawcallList.Num();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto drawcallItem = UIDrawcallList[i];
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
		bShouldRebuildAllDrawcall = true;
	}
}
void ULGUICanvas::RemoveFromDrawcall(UUIRenderable* item)
{
	auto drawcallIndex = item->GetGeometry()->drawcallIndex;
	if (drawcallIndex == -1)return;//-1 means not add to render yet
	if (drawcallIndex >= UIDrawcallList.Num())//drawcall index not inside list, need to rebuild all drawcall
	{
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
		bShouldRebuildAllDrawcall = true;
	}
}

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawcall"), STAT_UpdateDrawcall, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas TotalUpdate"), STAT_TotalUpdate, STATGROUP_LGUI);
void ULGUICanvas::UpdateChildRecursive(UUIItem* target, bool parentLayoutChanged)
{
	const auto& childrenList = target->GetAttachChildren();
	for (auto child : childrenList)
	{
		auto uiChild = Cast<UUIItem>(child);
		if (IsValid(uiChild))
		{
			if (uiChild->IsUIActiveInHierarchy() == false)continue;
			if (uiChild->IsCanvasUIItem() && IsValid(uiChild->GetRenderCanvas()) && uiChild->GetRenderCanvas() != this)
			{
				uiChild->GetRenderCanvas()->UpdateCanvasLayout(parentLayoutChanged);
				childrenCanvasArray.Add(uiChild->GetRenderCanvas());
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
	cacheForThisUpdate_ShouldUpdateLayout = bShouldUpdateLayout;
	cacheForThisUpdate_ClipTypeChanged = bClipTypeChanged;
	cacheForThisUpdate_RectClipParameterChanged = bRectClipParameterChanged;
	cacheForThisUpdate_TextureClipParameterChanged = bTextureClipParameterChanged;
	bShouldRebuildAllDrawcall = false;
	bShouldUpdateLayout = false;
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;

	childrenCanvasArray.Reset();
	//update layout and geometry
	UIItem->UpdateLayoutAndGeometry(parentLayoutChanged, cacheForThisUpdate_ShouldUpdateLayout);

	UpdateChildRecursive(UIItem, UIItem->cacheForThisUpdate_LayoutChanged);

	if (bShouldRebuildAllDrawcall)cacheForThisUpdate_ShouldRebuildAllDrawcall = true;
}
void ULGUICanvas::UpdateTopMostCanvas()
{
	SCOPE_CYCLE_COUNTER(STAT_TotalUpdate);
	if (this != TopMostCanvas) return;
	//update first Canvas
	if (TopMostCanvas == this)
	{
		UIItem->calculatedParentAlpha = UUIItem::Color255To1_Table[UIItem->widget.color.A];
	}

	UpdateCanvasLayout(false);
	if (prevFrameNumber != GFrameNumber)//ignore if not at new render frame
	{
		prevFrameNumber = GFrameNumber;
		UpdateCanvasGeometry();
	}
}
void ULGUICanvas::UpdateCanvasGeometry()
{
	for (auto item : childrenCanvasArray)
	{
		item->UpdateCanvasGeometry();
	}

	//draw triangle
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcall);
		if (cacheForThisUpdate_ShouldRebuildAllDrawcall)
		{
			LGUIUtils::SortUIItemDepth(UIRenderableItemList);//sort on depth
			LGUIUtils::CreateDrawcallFast(UIRenderableItemList, UIDrawcallList);//create drawcall
			for (auto item : UIDrawcallList)
			{
				item->UpdateDepthRange();
			}
			//create UIMesh based on Drawcall
			int meshCount = UIMeshList.Num();
			int drawcallCount = UIDrawcallList.Num();
			if (meshCount < drawcallCount)//need more mesh
			{
				int additionalMeshCount = drawcallCount - meshCount;
				UIMeshList.AddUninitialized(additionalMeshCount);
				for (int i = meshCount; i < drawcallCount; i++)
				{
#if WITH_EDITOR
					auto meshName = FString::Printf(TEXT("%s_Drawcall_%d"), *GetOwner()->GetActorLabel(), i);
#else
					auto meshName = FString::Printf(TEXT("Drawcall_%d"), i);
#endif
					auto uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), FName(*meshName), RF_Transient);
					uiMesh->RegisterComponent();
					uiMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
					uiMesh->SetRelativeTransform(FTransform::Identity);
#if WITH_EDITOR
					if (!GetWorld()->IsGameWorld())
					{
						if (currentIsRenderToRenderTargetOrWorld)
						{
							uiMesh->SetSupportScreenSpace(true, TopMostCanvas->GetViewExtension());
						}
						uiMesh->SetSupportWorldSpace(true);
					}
					else
#endif
					if (currentIsRenderToRenderTargetOrWorld)
					{
						uiMesh->SetSupportScreenSpace(true, TopMostCanvas->GetViewExtension());
						uiMesh->SetSupportWorldSpace(false);
					}

					UIMeshList[i] = uiMesh;
				}
			}
			else if (meshCount > drawcallCount)//set needless mesh invisible
			{
				int additionalMeshCount = meshCount - drawcallCount;
				for (int i = drawcallCount; i < meshCount; i++)
				{
					auto& meshItem = UIMeshList[i];
					if (!IsValid(meshItem))continue;
					meshItem->SetMeshVisible(false);
				}
			}

			for (int i = 0; i < drawcallCount; i++)//set data for all valid UIMesh
			{
				auto& uiMesh = UIMeshList[i];
				if (!IsValid(uiMesh))continue;
				auto& uiDrawcall = UIDrawcallList[i];
				if (!uiDrawcall.IsValid())continue;
				uiMesh->SetMeshVisible(true);//some UIMesh may set to invisible on prev frame, set to visible
				auto& meshSection = uiMesh->MeshSection;
				meshSection.Reset();
				uiDrawcall->GetCombined(meshSection.vertices, meshSection.triangles);
				uiMesh->GenerateOrUpdateMesh(true, additionalShaderChannels);
				if (uiDrawcall->postProcessObject.IsValid())
				{
					uiMesh->SetToPostProcess(uiDrawcall->postProcessObject.Get());
				}
				else
				{
					uiMesh->SetToPostProcess(nullptr);
				}
			}

			//after geometry created, need to sort UIMesh render order
			SortDrawcallRenderPriority();
		}
		else//no need to rebuild all drawcall
		{
			int drawcallCount = UIDrawcallList.Num();
			bool needToSortRenderPriority = false;
			{
				for (int i = drawcallCount - 1; i >= 0; i--)
				{
					auto uiMesh = UIMeshList[i];
					auto uiDrawcall = UIDrawcallList[i];
					if (uiDrawcall->needToBeRebuild)
					{
						LGUIUtils::SortUIItemDepth(uiDrawcall->geometryList);//sort needed for new add item
						uiDrawcall->UpdateDepthRange();
						auto& meshSection = uiMesh->MeshSection;
						meshSection.Reset();
						uiDrawcall->GetCombined(meshSection.vertices, meshSection.triangles);
						uiMesh->GenerateOrUpdateMesh(true, additionalShaderChannels);
						uiDrawcall->needToBeRebuild = false;
						uiDrawcall->needToUpdateVertex = false;
						if (uiDrawcall->postProcessObject.IsValid())
						{
							uiMesh->SetToPostProcess(uiDrawcall->postProcessObject.Get());
						}
						else
						{
							uiMesh->SetToPostProcess(nullptr);
						}
						needToSortRenderPriority = true;
					}
					else if (uiDrawcall->needToUpdateVertex)
					{
						auto& meshSection = uiMesh->MeshSection;
						uiDrawcall->UpdateData(meshSection.vertices, meshSection.triangles);
						uiMesh->GenerateOrUpdateMesh(uiDrawcall->vertexPositionChanged, additionalShaderChannels);
						uiDrawcall->needToUpdateVertex = false;
						uiDrawcall->vertexPositionChanged = false;
					}
				}
			}
			if (UIItem->cacheForThisUpdate_DepthChanged || needToSortRenderPriority)
			{
				//after geometry created, need to sort UIMesh render order
				SortDrawcallRenderPriority();
			}
		}
		//create or update material
		UpdateAndApplyMaterial();
	}


	//this frame is complete
	bRectRangeCalculated = false;
}
const TArray<ULGUICanvas*>& ULGUICanvas::GetAllCanvasArray()
{
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				return ULGUIEditorManagerObject::Instance->GetAllCanvas();
			}
		}
		else
#endif
		{
			if (ALGUIManagerActor::Instance != nullptr)
			{
				return ALGUIManagerActor::Instance->GetAllCanvas();
			}
		}
	}
	return ALGUIManagerActor::Instance->GetAllCanvas();
}
void ULGUICanvas::SortCanvasOnOrder()
{
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				ULGUIEditorManagerObject::SortCanvasOnOrder();
			}
		}
		else
#endif
		{
			if (ALGUIManagerActor::Instance != nullptr)
			{
				ALGUIManagerActor::SortCanvasOnOrder();
			}
		}
	}
}
void ULGUICanvas::SortDrawcallRenderPriority()
{
	auto& allCanvasArray = GetAllCanvasArray();
	if (allCanvasArray.Num() == 0)return;
	//set drawcall render order
	int32 startRenderPriority = 0;
	int32 prevSortOrder = allCanvasArray[0]->sortOrder - 1;//-1 is for first check of the loop
	int32 prevCanvasDrawcallCount = 0;//prev Canvas's drawcall count
	for (int i = 0; i < allCanvasArray.Num(); i++)
	{
		auto& canvasItem = allCanvasArray[i];
		if (canvasItem->sortOrder != prevSortOrder)
		{
			prevSortOrder = canvasItem->sortOrder;
			startRenderPriority += prevCanvasDrawcallCount;
		}
		int32 canvasItemDrawcallCount = canvasItem->SortDrawcall(startRenderPriority);

		if (canvasItem->sortOrder == prevSortOrder)//if Canvas's depth is equal, then take the max drawcall count
		{
			if (prevCanvasDrawcallCount < canvasItemDrawcallCount)
			{
				prevCanvasDrawcallCount = canvasItemDrawcallCount;
			}
		}
		else
		{
			prevCanvasDrawcallCount = canvasItemDrawcallCount;
		}
	}
//#if WITH_EDITOR
//	if (!GetWorld()->IsGameWorld())//editor world, do nothing
//	{
//
//	}
//	else
//#endif
	if (currentIsRenderToRenderTargetOrWorld)
	{
		TopMostCanvas->GetViewExtension()->SortRenderPriority();
	}
}
int32 ULGUICanvas::SortDrawcall(int32 InStartRenderPriority)
{
	for (int i = 0; i < UIDrawcallList.Num(); i++)
	{
		if (i < UIMeshList.Num())
		{
			UIMeshList[i]->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
	}
	return UIDrawcallList.Num();
}

void ULGUICanvas::UpdateAndApplyMaterial()
{
	int drawcallCount = UIDrawcallList.Num();
	//if clip type change, or need to rebuild all drawcall, then recreate material
	if (cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_ShouldRebuildAllDrawcall)
	{
		UIMaterialList.Reset();
	}
	//check if material count is enough, or create enough material
	int materialCount = UIMaterialList.Num();
	auto tempClipType = GetClipType();
	if (materialCount < drawcallCount)
	{
		cacheForThisUpdate_ClipTypeChanged = true;//mark to true, set materials clip property
		UIMaterialList.AddUninitialized(drawcallCount - materialCount);
		auto matArray = GetMaterials();
		for (int i = materialCount; i < drawcallCount; i++)
		{
			auto uiDrawcall = UIDrawcallList[i];
			UMaterialInterface* SrcMaterial = nullptr;
			if (uiDrawcall->material.IsValid())//custom material
			{
				SrcMaterial = uiDrawcall->material.Get();
			}
			else
			{
				SrcMaterial = matArray[(int)tempClipType];
				if (SrcMaterial == nullptr)
				{
					UE_LOG(LGUI, Log, TEXT("[ULGUICanvas::UpdateAndApplyMaterial]Material asset from Plugin/Content is missing! Reinstall this plugin may fix the issure"));
					UIMaterialList.Reset();
					return;
				}
			}

			UMaterialInstanceDynamic* uiMat = nullptr;
			if (SrcMaterial->IsA(UMaterialInstanceDynamic::StaticClass()))
			{
				uiMat = (UMaterialInstanceDynamic*)SrcMaterial;
			}
			else
			{
				uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
				uiMat->SetFlags(RF_Transient);
			}
			uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture.Get());
			UIMaterialList[i] = uiMat;
			uiDrawcall->materialInstanceDynamic = uiMat;
		}
	}
	if (cacheForThisUpdate_ShouldRebuildAllDrawcall)
	{
		for (int i = 0; i < drawcallCount; i++)//set material property
		{
			auto uiDrawcall = UIDrawcallList[i];
			auto uiMat = UIMaterialList[i];
			uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture.Get());
			auto& uiMesh = UIMeshList[i];
			if (!IsValid(uiMesh))continue;
			uiMesh->SetMaterial(0, uiMat);
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


void ULGUICanvas::SetParameterForStandard(int drawcallCount)
{

}
void ULGUICanvas::SetParameterForRectClip(int drawcallCount)
{
	auto rectClipOffsetAndSize = this->GetRectClipOffsetAndSize();
	auto rectClipFeather = this->GetRectClipFeather();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto uiMat = UIMaterialList[i];
		uiMat->SetVectorParameterValue(FName("RectClipOffsetAndSize"), rectClipOffsetAndSize);
		uiMat->SetVectorParameterValue(FName("RectClipFeather"), rectClipFeather);
	}
}
void ULGUICanvas::SetParameterForTextureClip(int drawcallCount)
{
	auto textureClipOffsetAndSize = this->GetTextureClipOffsetAndSize();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto uiMat = UIMaterialList[i];
		uiMat->SetTextureParameterValue(FName("ClipTexture"), clipTexture);
		uiMat->SetVectorParameterValue(FName("TextureClipOffsetAndSize"), textureClipOffsetAndSize);
	}
}
bool ULGUICanvas::IsPointVisible(FVector worldPoint)
{
	//if not use clip or use texture clip, then point is visible. texture clip not support this calculation yet.
	switch (GetClipType())
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
		if (localPoint.X < rectMin.X) return false;
		if (localPoint.Y < rectMin.Y) return false;
		if (localPoint.X > rectMax.X) return false;
		if (localPoint.Y > rectMax.Y) return false;
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
		auto& widget = UIItem->widget;
		//calculate sefl rect range
		rectMin.X = -widget.pivot.X * widget.width;
		rectMin.Y = -widget.pivot.Y * widget.height;
		rectMax.X = (1.0f - widget.pivot.X) * widget.width;
		rectMax.Y = (1.0f - widget.pivot.Y) * widget.height;
		//calculate parent rect range
		if (inheritRectClip && IsValid(ParentCanvas) && ParentCanvas->GetClipType() == ELGUICanvasClipType::Rect)
		{
			ParentCanvas->CalculateRectRange();
			auto parentRectMin = FVector(ParentCanvas->rectMin, 0);
			auto parentRectMax = FVector(ParentCanvas->rectMax, 0);
			//transform ParentCanvas's rect to this space
			auto& parentCanvasTf = ParentCanvas->UIItem->GetComponentTransform();
			auto thisTfInv = this->UIItem->GetComponentTransform().Inverse();
			parentRectMin = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMin));
			parentRectMax = thisTfInv.TransformPosition(parentCanvasTf.TransformPosition(parentRectMax));
			//inherit
			if (rectMin.X < parentRectMin.X)rectMin.X = parentRectMin.X;
			if (rectMin.Y < parentRectMin.Y)rectMin.Y = parentRectMin.Y;
			if (rectMax.X > parentRectMax.X)rectMax.X = parentRectMax.X;
			if (rectMax.Y > parentRectMax.Y)rectMax.Y = parentRectMax.Y;
		}

		bRectRangeCalculated = true;
	}
}


FLinearColor ULGUICanvas::GetRectClipOffsetAndSize()
{
	CalculateRectRange();
	return FLinearColor((rectMin.X - clipRectOffset.Left + rectMax.X + clipRectOffset.Right) * 0.5f, (rectMin.Y - clipRectOffset.Bottom + rectMax.Y + clipRectOffset.Top) * 0.5f, (rectMax.X - rectMin.X + clipRectOffset.Left + clipRectOffset.Right) * 0.5f, (rectMax.Y - rectMin.Y + clipRectOffset.Top + clipRectOffset.Bottom) * 0.5f);
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

void ULGUICanvas::OnWidthChanged()
{
	bRectClipParameterChanged = true;
	bRectRangeCalculated = false;
}
void ULGUICanvas::OnHeightChanged()
{
	bRectClipParameterChanged = true;
	bRectRangeCalculated = false;
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
			for (ULGUICanvas* itemCanvas : allCanvasArray)
			{
				if (IsValid(itemCanvas))
				{
					if (itemCanvas->UIItem->IsAttachedTo(this->UIItem))
					{
						itemCanvas->sortOrder += diff;
					}
				}
			}
		}
		SortCanvasOnOrder();
		SortDrawcallRenderPriority();
	}
}
void ULGUICanvas::SetSortOrderToHighestOfHierarchy(bool propagateToChildrenCanvas)
{
	if (CheckTopMostCanvas())
	{
		int32 maxSortOrder = 0;
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		for (ULGUICanvas* itemCanvas : allCanvasArray)
		{
			if (IsValid(itemCanvas))
			{
				if (itemCanvas->TopMostCanvas == this->TopMostCanvas)//on the same hierarchy
				{
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
	if (CheckTopMostCanvas())
	{
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		int32 minSortOrder = 0;
		for (ULGUICanvas* itemCanvas : allCanvasArray)
		{
			if (IsValid(itemCanvas))
			{
				if (itemCanvas->TopMostCanvas == this->TopMostCanvas)//on the same hierarchy
				{
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
	if (CheckTopMostCanvas())
	{
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		int32 maxSortOrder = 0;
		for (ULGUICanvas* itemCanvas : allCanvasArray)
		{
			if (IsValid(itemCanvas))
			{
				if (itemCanvas != this)
				{
					if (maxSortOrder < itemCanvas->sortOrder)
					{
						maxSortOrder = itemCanvas->sortOrder;
					}
				}
			}
		}
		int32 desireDepth = maxSortOrder + 1;
		SetSortOrder(desireDepth, propagateToChildrenCanvas);
	}
}
void ULGUICanvas::SetSortOrderToLowestOfAll(bool propagateToChildrenCanvas)
{
	if (CheckTopMostCanvas())
	{
		auto& allCanvasArray = GetAllCanvasArray();
		if (allCanvasArray.Num() == 0)return;
		int32 minDepth = 0;
		for (ULGUICanvas* itemCanvas : allCanvasArray)
		{
			if (IsValid(itemCanvas))
			{
				if (itemCanvas != this)
				{
					if (minDepth > itemCanvas->sortOrder)
					{
						minDepth = itemCanvas->sortOrder;
					}
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
}

void ULGUICanvas::SetDynamicPixelsPerUnit(float newValue)
{
	if (dynamicPixelsPerUnit != newValue)
	{
		dynamicPixelsPerUnit = newValue;
		MarkRebuildAllDrawcall();
	}
}
float ULGUICanvas::GetDynamicPixelsPerUnit()const
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
			if (IsValid(ParentCanvas))
			{
				return ParentCanvas->GetDynamicPixelsPerUnit();
			}
		}
	}
	return dynamicPixelsPerUnit;
}
ELGUICanvasClipType ULGUICanvas::GetClipType()const
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
			if (IsValid(ParentCanvas))
			{
				return ParentCanvas->GetClipType();
			}
		}
	}
	return clipType;
}

int8 ULGUICanvas::GetAdditionalShaderChannelFlags()const
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
			if (IsValid(ParentCanvas))
			{
				return ParentCanvas->GetAdditionalShaderChannelFlags();
			}
		}
	}
	return additionalShaderChannels;
}
bool ULGUICanvas::GetRequireNormal()const 
{
	return GetAdditionalShaderChannelFlags() & (1 << 0);
}
bool ULGUICanvas::GetRequireTangent()const 
{ 
	return GetAdditionalShaderChannelFlags() & (1 << 1);
}
bool ULGUICanvas::GetRequireUV1()const 
{ 
	return GetAdditionalShaderChannelFlags() & (1 << 2);
}
bool ULGUICanvas::GetRequireUV2()const 
{
	return GetAdditionalShaderChannelFlags() & (1 << 3);
}
bool ULGUICanvas::GetRequireUV3()const 
{ 
	return GetAdditionalShaderChannelFlags() & (1 << 4);
}


void ULGUICanvas::BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float InFOV, float InOrthoWidth, float InOrthoHeight, FMatrix& OutProjectionMatrix)
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

		if (InViewportSize.X > InViewportSize.Y)
		{
			// if the viewport is wider than it is tall
			XAxisMultiplier = 1.0f;
			YAxisMultiplier = InViewportSize.X / (float)InViewportSize.Y;
		}
		else
		{
			// if the viewport is taller than it is wide
			XAxisMultiplier = InViewportSize.Y / (float)InViewportSize.X;
			YAxisMultiplier = 1.0f;
		}

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
float ULGUICanvas::CalculateDistanceToCamera()
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
FMatrix ULGUICanvas::GetViewProjectionMatrix()
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
FMatrix ULGUICanvas::GetProjectionMatrix()
{
	FMatrix ProjectionMatrix = FMatrix::Identity;
	const float FOV = FOVAngle * (float)PI / 360.0f;
	BuildProjectionMatrix(GetViewportSize(), ProjectionType, FOV, UIItem->GetWidth(), UIItem->GetHeight(), ProjectionMatrix);
	return ProjectionMatrix;
}
FVector ULGUICanvas::GetViewLocation()
{
	return UIItem->GetComponentLocation() - UIItem->GetUpVector() * CalculateDistanceToCamera();
}
FMatrix ULGUICanvas::GetViewRotationMatrix()
{
	auto Transform = UIItem->GetComponentToWorld();
	Transform.SetTranslation(FVector::ZeroVector);
	Transform.SetScale3D(FVector::OneVector);
	return Transform.ToInverseMatrixWithScale();
}
FRotator ULGUICanvas::GetViewRotator()
{
	return UIItem->GetComponentToWorld().Rotator();
}
FIntPoint ULGUICanvas::GetViewportSize()
{
	FIntPoint viewportSize = FIntPoint(2, 2);
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
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
		if (auto world = this->GetWorld())
		{
			if (auto pc = world->GetFirstPlayerController())
			{
				pc->GetViewportSize(viewportSize.X, viewportSize.Y);
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

bool ULGUICanvas::GetPixelPerfect()const
{
	if (IsRootCanvas())
	{
		return this->currentIsRenderToRenderTargetOrWorld
			&& pixelPerfect;
	}
	else
	{
		if (IsValid(TopMostCanvas))
		{
			if (GetOverridePixelPerfect())
			{
				return TopMostCanvas->currentIsRenderToRenderTargetOrWorld
					&& this->pixelPerfect;
			}
			else
			{
				if (IsValid(ParentCanvas))
				{
					return TopMostCanvas->currentIsRenderToRenderTargetOrWorld
						&& ParentCanvas->GetPixelPerfect();
				}
			}
		}
	}
	return false;
}