// Copyright 2019 LexLiu. All Rights Reserved.

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
#include "Core/Render/LGUIRenderer.h"
#include "Core/LGUIMesh/UIDrawcallMesh.h"
#include "Core/UIDrawcall.h"
#include "Core/ActorComponent/UIRenderable.h"
#include "Core/ActorComponent/UIItem.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "GameFramework/PlayerController.h"
#include "Engine/LocalPlayer.h"
#include "SceneViewExtension.h"
#include "Engine.h"

DECLARE_CYCLE_STAT(TEXT("Canvas UpdateDrawcall"), STAT_UpdateDrawcall, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas TotalUpdate"), STAT_TotalUpdate, STATGROUP_LGUI);

ULGUICanvas::ULGUICanvas()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
}

void ULGUICanvas::BeginPlay()
{
	Super::BeginPlay();
	TopMostCanvas = nullptr;
	CheckTopMostCanvas();
	ParentCanvas = nullptr;
	CheckParentCanvas();
	CheckUIItem();
	MarkCanvasUpdate();

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;	

	if (this->IsDefaultSubobject())
	{
		LGUIManager::AddCanvas(this);
	}
	else
	{
		LGUIManager::SortCanvasOnOrder(this->GetWorld());
	}
}
void ULGUICanvas::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void ULGUICanvas::BeginDestroy()
{
	Super::BeginDestroy();
	if (ViewExtension.IsValid())
	{
		ViewExtension.Reset();
	}
}

void ULGUICanvas::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void ULGUICanvas::CustomTick(float DeltaTime)
{
	if (bCanTickUpdate)
	{
		if (CheckUIItem() && UIItem->IsUIActiveInHierarchy())
		{
			this->UpdateCanvasGeometry();
		}
	}
}

void ULGUICanvas::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	LGUIManager::AddCanvas(this);
#else
	if (!this->IsDefaultSubobject())//if is default subobject, then call AddCanvas in BeginPlay, because widget.depth value may not set if use PrefabSystem
	{
		LGUIManager::AddCanvas(this);
	}
#endif
	CheckTopMostCanvas();
	CheckParentCanvas();
}
void ULGUICanvas::OnUnregister()
{
	Super::OnUnregister();
	LGUIManager::RemoveCanvas(this);
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
	if (TopMostCanvas != nullptr)return true;
	LGUIUtils::FindTopMostCanvas(this->GetOwner(), TopMostCanvas);
	if (TopMostCanvas != nullptr)return true;
	return false;
}
bool ULGUICanvas::CheckParentCanvas()
{
	if (ParentCanvas != nullptr)return true;
	LGUIUtils::FindParentCanvas(this->GetOwner(), ParentCanvas);
	if (ParentCanvas != nullptr)return true;
	return false;
}
bool ULGUICanvas::CheckUIItem()
{
	if (UIItem != nullptr)return true;
	UIItem = Cast<UUIItem>(GetOwner()->GetRootComponent());
	if (UIItem == nullptr)
	{
		UE_LOG(LGUI, Error, TEXT("LGUICanvas component should only attach to a actor which have UIItem as RootComponent!"));
		return false;
	}
	else
	{
		return true;
	}
}

void ULGUICanvas::OnUIHierarchyChanged()
{
	//recheck top most canvas
	auto oldCanvas = TopMostCanvas;
	LGUIUtils::FindTopMostCanvas(this->GetOwner(), TopMostCanvas);
	if (oldCanvas != TopMostCanvas)
	{
		if (IsValid(oldCanvas))//remove from old uiroot
		{
			oldCanvas->AllCanvasBelongToThis.Remove(this);
		}
		if (IsValid(TopMostCanvas))//add to new uiroot
		{
			TopMostCanvas->AllCanvasBelongToThis.Add(this);
		}

		if (IsScreenSpaceOverlayUI())
		{
			for (auto uiMesh : UIMeshList)
			{
				uiMesh->SetToLGUIHud(TopMostCanvas->GetViewExtension());
			}
		}
		else
		{
			for (auto uiMesh : UIMeshList)
			{
				uiMesh->SetToLGUIWorld();
			}
		}
	}

	ParentCanvas = nullptr;
	CheckParentCanvas();
	//rebuild drawcall
	MarkRebuildAllDrawcall();
}
bool ULGUICanvas::IsScreenSpaceOverlayUI()
{
#if WITH_EDITOR
	if (!GetWorld()->IsGameWorld())
	{
		return false;
	}
	else
#endif
	{
		if (IsValid(TopMostCanvas))
		{
			return TopMostCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay;
		}
	}
	return false;
}

void ULGUICanvas::MarkCanvasUpdate()
{
	if (CheckTopMostCanvas())
	{
		this->bCanTickUpdate = false;//if need upper Canvas to update, then this Canvas don't need to, because upper Canvas will go down hierarchy to update this Canvas
		TopMostCanvas->bCanTickUpdate = true;//incase this Canvas's parent have layout component, so mark TopMostCanvas to update
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
	if(IsRootCanvas())
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
	}
	else
	{
		if (GetOverrideDefaultMaterials())
		{
			return &DefaultMaterials[0];
		}
		else
		{
			if (IsValid(ParentCanvas))
			{
				return &ParentCanvas->DefaultMaterials[0];
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
void ULGUICanvas::SetDrawcallTexture(int drawcallIndex, UTexture* drawcallTexture, bool isFontTexture)
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
	uiDrawcall->isFontTexture = isFontTexture;
	//material
	auto& uiMat = UIMaterialList[drawcallIndex];
	uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture);
	uiMat->SetScalarParameterValue(FName("IsFontTexture"), uiDrawcall->isFontTexture);
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
	if (!accommodate)//cannot fit to list, then mark rebuild all drawcall. and mark UIItem as not render yet
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
	return UIDrawcallList[drawcallIndex]->materialInstanceDynamic;
}

TSharedPtr<class FLGUIViewExtension, ESPMode::ThreadSafe> ULGUICanvas::GetViewExtension()
{
	if (!ViewExtension.IsValid())
	{
		if (renderMode == ELGUIRenderMode::ScreenSpaceOverlay)
		{
			if (GEngine)
			{
				ViewExtension = FSceneViewExtensions::NewExtension<FLGUIViewExtension>(this);
			}
		}
	}
	return ViewExtension;
}

void ULGUICanvas::UpdateChildRecursive(UUIItem* target, bool parentTransformChanged, bool parentLayoutChanged)
{
	const auto& childrenList = target->GetAttachChildren();
	for (auto child : childrenList)
	{
		auto uiChild = Cast<UUIItem>(child);
		if (uiChild != nullptr)
		{
			if (uiChild->IsUIActiveInHierarchy() == false)continue;

			auto layoutChanged = parentLayoutChanged;
			auto transformChanged = parentTransformChanged;
			uiChild->UpdateLayoutAndGeometry(layoutChanged, transformChanged);
			if (uiChild->IsCanvasUIItem())
			{
				uiChild->GetRenderCanvas()->bCanTickUpdate = false;
				uiChild->GetRenderCanvas()->UpdateCanvasGeometry();
			}
			else
			{
				auto childItemType = uiChild->GetUIItemType();
				if (childItemType == UIItemType::UIRenderable)
				{
					auto uiChildRenderable = (UUIRenderable*)uiChild;
					if (uiChildRenderable->GetGeometry().IsValid() && uiChildRenderable->GetGeometry()->vertices.Num() > 0)
					{
						UIRenderableItemList.Add(uiChildRenderable);
					}
				}
				UpdateChildRecursive(uiChild, transformChanged, layoutChanged);
			}			
		}
	}
}
void ULGUICanvas::UpdateCanvasGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_TotalUpdate);
	if (!CheckUIItem())return;
	if (!IsValid(TopMostCanvas))return;
	if (prevFrameNumber == GFrameNumber)
	{
		return;//ignore if not at new render frame
	}
	prevFrameNumber = GFrameNumber;
	
	//update geometry
	{
		int updateCountOfThisFrame = 0;
	UPDATE_GEOMETRY:
		updateCountOfThisFrame++;
		
		//update first Canvas
		if (TopMostCanvas == this)
		{
			UIItem->calculatedParentAlpha = UUIItem::Color255To1_Table[UIItem->widget.color.A];
		}
		//update layout and geometry
		if (bCanTickUpdate)//if Canvas is update from Tick, then update self's layout first
		{
			bCanTickUpdate = false;
			bool parentLayoutChanged = false;
			bool parentTransformChanged = false;
			UIItem->UpdateLayoutAndGeometry(parentLayoutChanged, parentTransformChanged);
		}

		UIRenderableItemList.Reset();
		if (UIItem->GetUIItemType() == UIItemType::UIRenderable)
		{
			auto uiRenderable = (UUIRenderable*)UIItem;
			if (uiRenderable->GetGeometry().IsValid() && uiRenderable->GetGeometry()->vertices.Num() > 0)
			{
				UIRenderableItemList.Add(uiRenderable);
			}
		}
		const auto& childrenList = UIItem->GetAttachChildren();
		for (auto child : childrenList)
		{
			auto uiChild = Cast<UUIItem>(child);
			if (uiChild != nullptr)
			{
				if(uiChild->IsUIActiveInHierarchy() == false)continue;

				bool layoutChanged = UIItem->cacheForThisUpdate_VertexPositionChanged;
				bool transformChanged = false;
				uiChild->UpdateLayoutAndGeometry(layoutChanged, transformChanged);
				if (uiChild->isCanvasUIItem)
				{
					uiChild->GetRenderCanvas()->bCanTickUpdate = false;
					uiChild->GetRenderCanvas()->UpdateCanvasGeometry();
				}
				else
				{
					if (uiChild->GetUIItemType() == UIItemType::UIRenderable)//only UIRenderable have geometry
					{
						auto uiChildRenderable = (UUIRenderable*)uiChild;
						if (uiChildRenderable->GetGeometry().IsValid() && uiChildRenderable->GetGeometry()->vertices.Num() > 0)
						{
							UIRenderableItemList.Add(uiChildRenderable);
						}
					}
					UpdateChildRecursive(uiChild, transformChanged, layoutChanged);
				}
			}
		}
		if (bCanTickUpdate)
		{
#if WITH_EDITOR
			int32 maxCanvasUpdateTimeInOneFrame = GetDefault<ULGUISettings>()->maxCanvasUpdateTimeInOneFrame;
#else
			static int32 maxCanvasUpdateTimeInOneFrame = GetDefault<ULGUISettings>()->maxCanvasUpdateTimeInOneFrame;
#endif
			if (updateCountOfThisFrame < maxCanvasUpdateTimeInOneFrame)
			{
				goto UPDATE_GEOMETRY;
			}
			else
			{
				if (GetOwner())
				{
					UE_LOG(LGUI, Warning, TEXT("LGUICanvas:%s, update geometry more than %d times in one frame, this is weird, must be something wrong.\
						But if you absolutely understand how this LGUICanvas work, you can increase the \"maxCanvasUpdateTimeInOneFrame\" in LGUISettings, so this warning will be gone.")
						, *(GetOwner()->GetPathName()), updateCountOfThisFrame);
				}
			}
		}
	}

	{
		//draw triangle
		SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcall);
		if (bShouldRebuildAllDrawcall)
		{
			LGUIUtils::SortUIItemDepth(UIRenderableItemList);//sort on depty
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
					auto meshName = FString::Printf(TEXT("Drawcall_%d"), i);
					auto uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), FName(*meshName), RF_Transient);
					if (IsScreenSpaceOverlayUI())
					{
						uiMesh->SetToLGUIHud(TopMostCanvas->GetViewExtension());
					}
					uiMesh->RegisterComponent();
					uiMesh->AttachToComponent(UIItem, FAttachmentTransformRules::KeepRelativeTransform);
					uiMesh->SetRelativeTransform(FTransform::Identity);
					UIMeshList[i] = uiMesh;
				}
			}
			else if (meshCount > drawcallCount)//set needless mesh invisible
			{
				int additionalMeshCount = meshCount - drawcallCount;
				for (int i = drawcallCount; i < meshCount; i++)
				{
					auto& meshItem = UIMeshList[i];
					if (meshItem == nullptr)continue;
					meshItem->SetMeshVisible(false);
				}
			}
			for (int i = 0; i < drawcallCount; i++)//set data for all valid UIMesh
			{
				auto& uiMesh = UIMeshList[i];
				if (uiMesh == nullptr)continue;
				auto& uiDrawcall = UIDrawcallList[i];
				if (!uiDrawcall.IsValid())continue;
				uiMesh->SetMeshVisible(true);//some UIMesh may set to invisible on prev frame, set to visible
				auto& meshSection = uiMesh->MeshSection;
				meshSection.Reset();
				uiDrawcall->GetCombined(meshSection.vertices, meshSection.uvs, meshSection.colors, meshSection.triangles
					, meshSection.normals
					, meshSection.tangents
					, meshSection.uvs1
					, meshSection.uvs2
					, meshSection.uvs3);
				uiMesh->GenerateOrUpdateMesh(true, additionalShaderChannels);
			}

			//after geometry created, need to sort UIMesh render order
			SortDrawcallRenderPriority();
		}
		else//no need to rebuild all drawcall
		{
			int drawcallCount = UIDrawcallList.Num();
			bool needToSortRenderPriority = false;
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
					uiDrawcall->GetCombined(meshSection.vertices, meshSection.uvs, meshSection.colors, meshSection.triangles
						, meshSection.normals
						, meshSection.tangents
						, meshSection.uvs1
						, meshSection.uvs2
						, meshSection.uvs3);
					uiMesh->GenerateOrUpdateMesh(true, additionalShaderChannels);
					uiDrawcall->needToBeRebuild = false;
					uiDrawcall->needToUpdateVertex = false;
					needToSortRenderPriority = true;
				}
				else if (uiDrawcall->needToUpdateVertex)
				{
					auto& meshSection = uiMesh->MeshSection;
					uiDrawcall->UpdateData(meshSection.vertices, meshSection.uvs, meshSection.colors, meshSection.triangles
						, meshSection.normals
						, meshSection.tangents
						, meshSection.uvs1
						, meshSection.uvs2
						, meshSection.uvs3);
					uiMesh->GenerateOrUpdateMesh(uiDrawcall->vertexPositionChanged, additionalShaderChannels);
					uiDrawcall->needToUpdateVertex = false;
					uiDrawcall->vertexPositionChanged = false;
				}
			}
			if (!IsScreenSpaceOverlayUI())
			{
				needToSortRenderPriority = false;
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
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;
	bRectRangeCalculated = false;

	bShouldRebuildAllDrawcall = false;
}
void ULGUICanvas::SortDrawcallRenderPriority()
{
	if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
	auto& allCanvasArray = LGUIManager::GetAllCanvas(this->GetWorld());
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
	if (IsScreenSpaceOverlayUI())
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
	if (bClipTypeChanged || bShouldRebuildAllDrawcall)
	{
		for (auto uiMat : UIMaterialList)
		{
			if (IsValid(uiMat))
			{
				uiMat->ConditionalBeginDestroy();
			}
		}
		UIMaterialList.Empty();
	}
	//check if material count is enough, or create enough material
	int materialCount = UIMaterialList.Num();
	auto tempClipType = GetClipType();
	if (materialCount < drawcallCount)
	{
		bClipTypeChanged = true;//mark to true, set materials clip property
		UIMaterialList.AddUninitialized(drawcallCount - materialCount);
		auto matArray = GetMaterials();
		for (int i = materialCount; i < drawcallCount; i++)
		{
			auto uiDrawcall = UIDrawcallList[i];
			UMaterialInterface* SrcMaterial = nullptr;
			if (uiDrawcall->material != nullptr)//custom material
			{
				SrcMaterial = uiDrawcall->material;
			}
			else
			{
				SrcMaterial = matArray[(int)tempClipType];
				if (SrcMaterial == nullptr)
				{
					UE_LOG(LGUI, Log, TEXT("[ULGUICanvas::UpdateAndApplyMaterial]Material asset from Plugin/Content is missing! Reinstall this plugin may fix the issure"));
					UIMaterialList.Empty();
					return;
				}
			}
			auto uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this->GetOwner());
			uiMat->SetFlags(RF_Transient);
			uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture);
			uiMat->SetScalarParameterValue(FName("IsFontTexture"), uiDrawcall->isFontTexture);
			UIMaterialList[i] = uiMat;
			uiDrawcall->materialInstanceDynamic = uiMat;
		}
	}
	if (bShouldRebuildAllDrawcall)
	{
		for (int i = 0; i < drawcallCount; i++)//set material property
		{
			auto uiDrawcall = UIDrawcallList[i];
			auto uiMat = UIMaterialList[i];
			uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture);
			uiMat->SetScalarParameterValue(FName("IsFontTexture"), uiDrawcall->isFontTexture);
			auto& uiMesh = UIMeshList[i];
			if (uiMesh == nullptr)continue;
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
		if (bClipTypeChanged || bRectClipParameterChanged)
		{
			SetParameterForRectClip(drawcallCount);
		}
	}
	break;
	case ELGUICanvasClipType::Texture:
	{
		if (bClipTypeChanged || bTextureClipParameterChanged)
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
		if (inheritRectClip && ParentCanvas != nullptr && ParentCanvas->GetClipType() == ELGUICanvasClipType::Rect)
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
void ULGUICanvas::SetSortOrder(int32 newDepth, bool propagateToChildrenCanvas)
{
	if (sortOrder != newDepth)
	{
		int32 diff = newDepth - sortOrder;
		sortOrder = newDepth;
		MarkCanvasUpdate();
		if (propagateToChildrenCanvas)
		{
			if (IsScreenSpaceOverlayUI())
			{
				for (ULGUICanvas* itemCanvas : AllCanvasBelongToThis)
				{
					if (IsValid(itemCanvas))
					{
						itemCanvas->sortOrder += diff;
						MarkCanvasUpdate();
					}
				}
			}
			if (LGUIManager::IsManagerValid(this->GetWorld()))
			{
				auto& allCanvasArray = LGUIManager::GetAllCanvas(this->GetWorld());
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
		}
		if (!IsScreenSpaceOverlayUI())
		{
			LGUIManager::SortCanvasOnOrder(this->GetWorld());
		}
		SortDrawcallRenderPriority();
	}
}
void ULGUICanvas::SetSortOrderToHighestOfHierarchy(bool propagateToChildrenCanvas)
{
	if (CheckTopMostCanvas())
	{
		int32 maxSortOrder = 0;
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allCanvasArray = IsScreenSpaceOverlayUI() ? TopMostCanvas->AllCanvasBelongToThis : LGUIManager::GetAllCanvas(this->GetWorld());
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
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allCanvasArray = IsScreenSpaceOverlayUI() ? TopMostCanvas->AllCanvasBelongToThis : LGUIManager::GetAllCanvas(this->GetWorld());

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
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allCanvasArray = IsScreenSpaceOverlayUI() ? TopMostCanvas->AllCanvasBelongToThis : LGUIManager::GetAllCanvas(this->GetWorld());

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
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allCanvasArray = IsScreenSpaceOverlayUI() ? TopMostCanvas->AllCanvasBelongToThis : LGUIManager::GetAllCanvas(this->GetWorld());

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
	//delete old material
	for (auto uiMat : UIMaterialList)
	{
		if (IsValid(uiMat))
		{
			uiMat->ConditionalBeginDestroy();
		}
	}
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
				return ParentCanvas->dynamicPixelsPerUnit;
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
				return ParentCanvas->clipType;
			}
		}
	}
	return clipType;
}

bool ULGUICanvas::GetRequireNormal()const 
{
	int8 tempAddionalShaderChannels = 0;
	if (IsRootCanvas())
	{
		tempAddionalShaderChannels = additionalShaderChannels;
	}
	else
	{
		if (GetOverrideAddionalShaderChannel())
		{
			tempAddionalShaderChannels = additionalShaderChannels;
		}
		else
		{
			if (IsValid(ParentCanvas))
			{
				tempAddionalShaderChannels = ParentCanvas->additionalShaderChannels;
			}
		}
	}
	return tempAddionalShaderChannels & (1 << 0);
}
bool ULGUICanvas::GetRequireTangent()const 
{ 
	int8 tempAddionalShaderChannels = 0;
	if (IsRootCanvas())
	{
		tempAddionalShaderChannels = additionalShaderChannels;
	}
	else
	{
		if (GetOverrideAddionalShaderChannel())
		{
			tempAddionalShaderChannels = additionalShaderChannels;
		}
		else
		{
			if (IsValid(ParentCanvas))
			{
				tempAddionalShaderChannels = ParentCanvas->additionalShaderChannels;
			}
		}
	}
	return tempAddionalShaderChannels & (1 << 1);
}
bool ULGUICanvas::GetRequireUV1()const 
{ 
	int8 tempAddionalShaderChannels = 0;
	if (IsRootCanvas())
	{
		tempAddionalShaderChannels = additionalShaderChannels;
	}
	else
	{
		if (GetOverrideAddionalShaderChannel())
		{
			tempAddionalShaderChannels = additionalShaderChannels;
		}
		else
		{
			if (IsValid(ParentCanvas))
			{
				tempAddionalShaderChannels = ParentCanvas->additionalShaderChannels;
			}
		}
	}
	return tempAddionalShaderChannels & (1 << 2);
}
bool ULGUICanvas::GetRequireUV2()const 
{
	int8 tempAddionalShaderChannels = 0;
	if (IsRootCanvas())
	{
		tempAddionalShaderChannels = additionalShaderChannels;
	}
	else
	{
		if (GetOverrideAddionalShaderChannel())
		{
			tempAddionalShaderChannels = additionalShaderChannels;
		}
		else
		{
			if (IsValid(ParentCanvas))
			{
				tempAddionalShaderChannels = ParentCanvas->additionalShaderChannels;
			}
		}
	}
	return tempAddionalShaderChannels & (1 << 3);
}
bool ULGUICanvas::GetRequireUV3()const 
{ 
	int8 tempAddionalShaderChannels = 0;
	if (IsRootCanvas())
	{
		tempAddionalShaderChannels = additionalShaderChannels;
	}
	else
	{
		if (GetOverrideAddionalShaderChannel())
		{
			tempAddionalShaderChannels = additionalShaderChannels;
		}
		else
		{
			if (IsValid(ParentCanvas))
			{
				tempAddionalShaderChannels = ParentCanvas->additionalShaderChannels;
			}
		}
	}
	return tempAddionalShaderChannels & (1 << 4);
}


void ULGUICanvas::BuildProjectionMatrix(FIntPoint InViewportSize, ECameraProjectionMode::Type InProjectionType, float InFOV, float InOrthoWidth, FMatrix& OutProjectionMatrix)
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

	if (InProjectionType == ECameraProjectionMode::Orthographic)
	{
		check((int32)ERHIZBuffer::IsInverted);
		const float tempOrthoWidth = InOrthoWidth / 2.0f;
		const float tempOrthoHeight = InOrthoWidth / 2.0f * YAxisMultiplier;

		const float NearPlane = 0;
		const float FarPlane = WORLD_MAX / 8.0f;

		const float ZScale = 1.0f / (FarPlane - NearPlane);
		const float ZOffset = -NearPlane;

		OutProjectionMatrix = FReversedZOrthoMatrix(
			tempOrthoWidth,
			tempOrthoHeight,
			ZScale,
			ZOffset
		);
	}
	else
	{
		if ((int32)ERHIZBuffer::IsInverted)
		{
			OutProjectionMatrix = FReversedZPerspectiveMatrix(
				InFOV,
				InFOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
		else
		{
			OutProjectionMatrix = FPerspectiveMatrix(
				InFOV,
				InFOV,
				XAxisMultiplier,
				YAxisMultiplier,
				GNearClippingPlane,
				GNearClippingPlane
			);
		}
	}
}
FMatrix ULGUICanvas::GetViewProjectionMatrix()
{
	FMatrix ViewProjectionMatrix = FMatrix::Identity;
	if (!CheckUIItem())
	{
		UE_LOG(LGUI, Error, TEXT("[LGUICanvas::GetViewProjectionMatrix]UIItem not valid!"));
		return ViewProjectionMatrix;
	}
	FVector ViewLocation = UIItem->GetComponentLocation() - UIItem->GetUpVector() * DistanceToCamera;
	auto Transform = UIItem->GetComponentToWorld();
	Transform.SetTranslation(FVector::ZeroVector);
	Transform.SetScale3D(FVector::OneVector);
	FMatrix ViewRotationMatrix = Transform.ToInverseMatrixWithScale();

	const float FOV = FOVAngle * (float)PI / 360.0f;

	FMatrix ProjectionMatrix;
	BuildProjectionMatrix(GetViewportSize(), ProjectionType, FOV, OrthoWidth, ProjectionMatrix);
	ViewProjectionMatrix = FTranslationMatrix(-ViewLocation) * ViewRotationMatrix * ProjectionMatrix;
	return ViewProjectionMatrix;
}
FMatrix ULGUICanvas::GetProjectionMatrix()
{
	FMatrix ProjectionMatrix = FMatrix::Identity;
	const float FOV = FOVAngle * (float)PI / 360.0f;
	BuildProjectionMatrix(GetViewportSize(), ProjectionType, FOV, OrthoWidth, ProjectionMatrix);
	return ProjectionMatrix;
}
FVector ULGUICanvas::GetViewLocation()
{
	return UIItem->GetComponentLocation() - UIItem->GetUpVector() * DistanceToCamera;
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
	return UIItem->GetComponentToWorld().Rotator().Add(90, 90, 0);
}
FIntPoint ULGUICanvas::GetViewportSize()
{
	FIntPoint viewportSize = FIntPoint(2, 2);
	if (!GetWorld()->IsGameWorld())
	{
		if (CheckUIItem())
		{
			viewportSize.X = UIItem->GetWidth();
			viewportSize.Y = UIItem->GetHeight();
		}
	}
	else
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
	}
}

bool ULGUICanvas::GetPixelPerfect()const
{
	if (IsRootCanvas())
	{
		return renderMode == ELGUIRenderMode::ScreenSpaceOverlay && pixelPerfect;
	}
	else
	{
		if (IsValid(TopMostCanvas))
		{
			if (GetOverridePixelPerfect())
			{
				return TopMostCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay && this->pixelPerfect;
			}
			else
			{
				if (IsValid(ParentCanvas))
				{
					return TopMostCanvas->renderMode == ELGUIRenderMode::ScreenSpaceOverlay && ParentCanvas->pixelPerfect;
				}
			}
		}
	}
	return false;
}