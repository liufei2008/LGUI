// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIPanel.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Utils/LGUIUtils.h"
#include "CoreGlobals.h"
#if WITH_EDITOR
#include "Editor.h"
#endif
#include "Core/UIRoot.h"
#include "Utils/LGUIUtils.h"
#include "Core/LGUISettings.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "Core/Render/LGUIRenderer.h"
#include "UIMesh.h"
#include "Core/UIDrawcall.h"
#include "UIRenderable.h"
#include "Materials/MaterialInstanceDynamic.h"

DECLARE_CYCLE_STAT(TEXT("UIPanel UpdateDrawcall"), STAT_UpdateDrawcall, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("UIPanel TotalUpdate"), STAT_TotalUpdate, STATGROUP_LGUI);

UUIPanel::UUIPanel()
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	itemType = UIItemType::UIPanel;

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
}

void UUIPanel::BeginPlay()
{
	Super::BeginPlay();
	FirstUIPanel = nullptr;
	CheckFirstUIPanel();
	ParentUIPanel = nullptr;
	CheckParentUIPanel();
	CheckUIRoot();
	MarkPanelUpdate();
	CheckMaterials();

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;	

	if (this->IsDefaultSubobject())
	{
		LGUIManager::AddUIPanel(this);
	}
	else
	{
		LGUIManager::SortUIPanelOnDepth(this->GetWorld());
		if (IsValid(uiRootComp))
		{
			uiRootComp->SortUIPanelOnDepth();
		}
	}
}
void UUIPanel::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

void UUIPanel::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIPanel::CustomTick(float DeltaTime)
{
	if (bCanTickUpdate && IsUIActiveInHierarchy())
	{
		this->UpdatePanelGeometry();
	}
}

void UUIPanel::OnRegister()
{
	Super::OnRegister();
#if WITH_EDITOR
	if (GetWorld() && GetWorld()->WorldType == EWorldType::Editor)
	{
		LGUIManager::AddUIPanel(this);
	}
#else
	if (!this->IsDefaultSubobject())//if is default subobject, then call AddUIPanel in BeginPlay, because widget.depth value may not set if use PrefabSystem
	{
		LGUIManager::AddUIPanel(this);
	}
#endif
#if WITH_EDITOR
	if (GetWorld())
	{
		if (GetWorld()->WorldType == EWorldType::Editor || GetWorld()->WorldType == EWorldType::EditorPreview)
		{
			CheckMaterials();
		}
	}
#endif
}
void UUIPanel::OnUnregister()
{
	Super::OnUnregister();
	LGUIManager::RemoveUIPanel(this);
}

void UUIPanel::ApplyUIActiveState()
{
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
	if (IsUIActiveInHierarchy() == false)
	{
		for (auto uiMesh : UIMeshList)
		{
			uiMesh->SetVisibility(false);
		}
	}
	Super::ApplyUIActiveState();
}

bool UUIPanel::CheckFirstUIPanel()
{
	if (FirstUIPanel != nullptr)return true;
	LGUIUtils::FindFirstUIPanel(this, FirstUIPanel);
	if (FirstUIPanel != nullptr)return true;
	return false;
}
bool UUIPanel::CheckParentUIPanel()
{
	if (ParentUIPanel != nullptr)return true;
	LGUIUtils::FindParentUIPanel(this, ParentUIPanel);
	if (ParentUIPanel != nullptr)return true;
	return false;
}

void UUIPanel::UIHierarchyChanged()
{
	Super::UIHierarchyChanged();
	FirstUIPanel = nullptr;//set to null, force recheck
	CheckFirstUIPanel();
	ParentUIPanel = nullptr;
	CheckParentUIPanel();
	//find UIRoot and get some data
	CheckUIRoot();
	//rebuild drawcall
	MarkRebuildAllDrawcall();
}
void UUIPanel::CheckUIRoot()
{
	if (!IsValid(GetOwner()))return;
	auto oldUIRoot = uiRootComp;
	uiRootComp = LGUIUtils::GetComponentInParent<UUIRoot>(GetOwner());
	if (oldUIRoot != uiRootComp)
	{
		if (IsValid(oldUIRoot))//remove from old uiroot
		{
			oldUIRoot->RemovePanel(this);
		}
		if (IsValid(uiRootComp))//add to new uiroot
		{
			uiRootComp->AddPanel(this);
		}

		if (IsScreenSpaceOverlayUI())
		{
			for (auto uiMesh : UIMeshList)
			{
				uiMesh->SetToLGUIHud(uiRootComp->GetViewExtension());
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
}
bool UUIPanel::IsScreenSpaceOverlayUI()
{
	if (IsValid(uiRootComp))
	{
#if WITH_EDITOR
		if (auto world = GetWorld())
		{
			if (!world->IsGameWorld())
			{
				return false;
			}
		}
#endif
		return uiRootComp->GetRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay;
	}
	return false;
}

void UUIPanel::MarkPanelUpdate()
{
	//Super::MarkPanelUpdate();
	if (CheckFirstUIPanel())
	{
		this->bCanTickUpdate = false;//if need upper panel to update, then this panel don't need to, because upper panel will go down hierarchy to update this panel
		FirstUIPanel->bCanTickUpdate = true;//incase this panel's parent have layout, so mark FirstUIPanel to update
	}
}
#if WITH_EDITOR
void UUIPanel::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUIPanel::EditorForceUpdateImmediately()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	CheckUIRoot();
	CheckMaterials();
	FirstUIPanel = nullptr;//set to null, force search
	CheckFirstUIPanel();
	ParentUIPanel = nullptr;
	CheckParentUIPanel();
	LGUIManager::SortUIPanelOnDepth(this->GetWorld());
	if (IsValid(uiRootComp))
	{
		uiRootComp->SortUIPanelOnDepth();
	}
	SortDrawcallRenderPriority();
	MarkPanelUpdate();
	MarkRebuildAllDrawcall();
	Super::EditorForceUpdateImmediately();
	UpdatePanelGeometry();
}
void UUIPanel::PostLoad()
{
	Super::PostLoad();
}
#endif

void UUIPanel::CheckMaterials()
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
				UE_LOG(LGUI, Error, TEXT("[UIPanel/CheckMaterials]Assign material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."));
				return;
			}
			DefaultMaterials[i] = mat;
		}
	}
}


UUIPanel* UUIPanel::GetFirstUIPanel() 
{ 
	CheckFirstUIPanel(); 
	return FirstUIPanel; 
}

void UUIPanel::MarkRebuildAllDrawcall()
{
	bShouldRebuildAllDrawcall = true;
}
void UUIPanel::MarkRebuildSpecificDrawcall(int drawcallIndex)
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
void UUIPanel::SetDrawcallTexture(int drawcallIndex, UTexture* drawcallTexture, bool isFontTexture)
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
void UUIPanel::MarkUpdateSpecificDrawcallVertex(int drawcallIndex, bool vertexPositionChanged)
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
void UUIPanel::InsertIntoDrawcall(UUIRenderable* item)
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
void UUIPanel::RemoveFromDrawcall(UUIRenderable* item)
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
void UUIPanel::DepthChangeForDrawcall(UUIRenderable* item)
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
UMaterialInstanceDynamic* UUIPanel::GetMaterialInstanceDynamicForDrawcall(int drawcallIndex)
{
	if (drawcallIndex == -1 || drawcallIndex >= UIDrawcallList.Num())return nullptr;
	return UIDrawcallList[drawcallIndex]->materialInstanceDynamic;
}

void UUIPanel::UpdateChildRecursive(UUIItem* target, bool parentTransformChanged, bool parentLayoutChanged)
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
			auto childItemType = uiChild->GetUIItemType();
			if (childItemType == UIItemType::UIRenderable)
			{
				auto uiChildRenderable = (UUIRenderable*)uiChild;
				if (uiChildRenderable->GetGeometry().IsValid() && uiChildRenderable->GetGeometry()->vertices.Num() > 0)
				{
					UIRenderableItemList.Add(uiChildRenderable);
				}
			}

			if (childItemType == UIItemType::UIPanel)//for UIPanel, need to call UpdatePanelGeometry
			{
				auto uiChildPanel = (UUIPanel*)uiChild;
				uiChildPanel->bCanTickUpdate = false;//don't need update form Tick, because of UpdatePanelGeometry
				uiChildPanel->UpdatePanelGeometry();
			}
			else//other type
			{
				UpdateChildRecursive(uiChild, transformChanged, layoutChanged);
			}
		}
	}
}
void UUIPanel::UpdatePanelGeometry()
{
	SCOPE_CYCLE_COUNTER(STAT_TotalUpdate);
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
		
		//update first UIPanel
		if (FirstUIPanel == this)
		{
			this->calculatedParentAlpha = UUIItem::Color255To1_Table[this->widget.color.A];
		}
		//update layout and geometry
		if (bCanTickUpdate)//if panel is update from Tick, then update self's layout first
		{
			bCanTickUpdate = false;
			bool parentLayoutChanged = false;
			bool parentTransformChanged = false;
			this->UpdateLayoutAndGeometry(parentLayoutChanged, parentTransformChanged);
		}

		UIRenderableItemList.Reset();
		const auto& childrenList = this->GetAttachChildren();
		for (auto child : childrenList)
		{
			auto uiChild = Cast<UUIItem>(child);
			if (uiChild != nullptr)
			{
				if(uiChild->IsUIActiveInHierarchy() == false)continue;

				bool layoutChanged = cacheForThisUpdate_VertexPositionChanged;
				bool transformChanged = false;
				uiChild->UpdateLayoutAndGeometry(layoutChanged, transformChanged);
				auto childItemType = uiChild->GetUIItemType();
				if (childItemType == UIItemType::UIRenderable)//only UIRenderable have geometry
				{
					auto uiChildRenderable = (UUIRenderable*)uiChild;
					if (uiChildRenderable->GetGeometry().IsValid() && uiChildRenderable->GetGeometry()->vertices.Num() > 0)
					{
						UIRenderableItemList.Add(uiChildRenderable);
					}
				}
				
				if (childItemType == UIItemType::UIPanel)//for UIPanel, need to call UpdatePanelGeometry
				{
					auto uiChildPanel = (UUIPanel*)uiChild;
					uiChildPanel->bCanTickUpdate = false;//don't need update form Tick, because of UpdatePanelGeometry
					uiChildPanel->UpdatePanelGeometry();
				}
				else//other type
				{
					UpdateChildRecursive(uiChild, transformChanged, layoutChanged);
				}
			}
		}
		if (bCanTickUpdate)
		{
#if WITH_EDITOR
			int32 maxPanelUpdateTimeInOneFrame = GetDefault<ULGUISettings>()->maxPanelUpdateTimeInOneFrame;
#else
			static int32 maxPanelUpdateTimeInOneFrame = GetDefault<ULGUISettings>()->maxPanelUpdateTimeInOneFrame;
#endif
			if (updateCountOfThisFrame < maxPanelUpdateTimeInOneFrame)
			{
				goto UPDATE_GEOMETRY;
			}
			else
			{
				if (GetOwner())
				{
					UE_LOG(LGUI, Warning, TEXT("panel:%s, update geometry more than %d times in one frame, this is weird, must be something wrong.\
						But if you absolutely understand how this UIPanel work, you can increase the \"maxPanelUpdateTimeInOneFrame\" in LGUISettings, so this warning will be gone.")
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
					auto uiMesh = NewObject<UUIMesh>(this->GetOwner(), FName(*meshName), RF_Transient);
					if (IsScreenSpaceOverlayUI())
					{
						uiMesh->SetToLGUIHud(uiRootComp->GetViewExtension());
					}
					uiMesh->RegisterComponent();
					uiMesh->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
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
					auto name = meshItem->GetName();
					if (!name.EndsWith("_NoUse"))
						name.Append("_NoUse");
					meshItem->Rename(*name);
				}
			}
			for (int i = 0; i < drawcallCount; i++)//set data for all valid UIMesh
			{
				auto& uiMesh = UIMeshList[i];
				if (uiMesh == nullptr)continue;
				auto& uiDrawcall = UIDrawcallList[i];
				if (!uiDrawcall.IsValid())continue;
				uiMesh->SetMeshVisible(true);//some UIMesh may set to invisible on prev frame, set to visible
				auto name = uiMesh->GetName();
				if (name.EndsWith("_NoUse"))
				{
					name = name.Left(name.Len() - 6);
					uiMesh->Rename(*name);
				}
				auto& meshSection = uiMesh->MeshSection;
				meshSection.Reset();
				uiDrawcall->GetCombined(meshSection.vertices, meshSection.uvs, meshSection.colors, meshSection.triangles
					, meshSection.normals
					, meshSection.tangents
					, meshSection.uvs1
					, meshSection.uvs2
					, meshSection.uvs3);
				uiMesh->GenerateOrUpdateMesh();
				uiMesh->SetOwnerNoSee(bOwnerNoSee);
				uiMesh->SetOnlyOwnerSee(bOnlyOwnerSee);
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
					uiMesh->GenerateOrUpdateMesh();
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
					uiMesh->GenerateOrUpdateMesh(uiDrawcall->vertexPositionChanged);
					uiDrawcall->needToUpdateVertex = false;
					uiDrawcall->vertexPositionChanged = false;
				}
			}
			if (!IsScreenSpaceOverlayUI())
			{
				needToSortRenderPriority = false;
			}
			if (cacheForThisUpdate_DepthChanged || needToSortRenderPriority)
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
void UUIPanel::SortDrawcallRenderPriority()
{
	if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
	auto& allPanelArray = LGUIManager::GetAllUIPanel(this->GetWorld());
	if (allPanelArray.Num() == 0)return;
	//set drawcall render order
	int32 startRenderPriority = 0;
	int32 prevDepth = allPanelArray[0]->widget.depth - 1;//-1 is for first check of the loop
	int32 prevPanelDrawcallCount = 0;//prev UIPanel's drawcall count
	for (int i = 0; i < allPanelArray.Num(); i++)
	{
		auto& panelItem = allPanelArray[i];
		if (panelItem->widget.depth != prevDepth)
		{
			prevDepth = panelItem->widget.depth;
			startRenderPriority += prevPanelDrawcallCount;
		}
		int32 panelItemDrawcallCount = panelItem->SortDrawcall(startRenderPriority);

		if (panelItem->widget.depth == prevDepth)//if panel's depth is equal, then take the max drawcall count
		{
			if (prevPanelDrawcallCount < panelItemDrawcallCount)
			{
				prevPanelDrawcallCount = panelItemDrawcallCount;
			}
		}
		else
		{
			prevPanelDrawcallCount = panelItemDrawcallCount;
		}
	}
	if (IsScreenSpaceOverlayUI())
	{
		uiRootComp->GetViewExtension()->SortRenderPriority();
	}
}
int32 UUIPanel::SortDrawcall(int32 InStartRenderPriority)
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

void UUIPanel::UpdateAndApplyMaterial()
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
	if (materialCount < drawcallCount)
	{
		bClipTypeChanged = true;//mark to true, set materials clip property
		UIMaterialList.AddUninitialized(drawcallCount - materialCount);
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
				SrcMaterial = DefaultMaterials[(int)clipType];
				if (SrcMaterial == nullptr)
				{
					UE_LOG(LGUI, Log, TEXT("[UIPanel::UpdateAndApplyMaterial]Material asset from Plugin/Content is missing! Reinstall this plugin may fix the issure"));
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
	if (clipType == UIPanelClipType::None)
	{
		SetParameterForStandard(drawcallCount);
	}
	else if (clipType == UIPanelClipType::Rect)
	{
		if (bClipTypeChanged || bRectClipParameterChanged)
		{
			SetParameterForRectClip(drawcallCount);
		}
	}
	else if (clipType == UIPanelClipType::Texture)
	{
		if (bClipTypeChanged || bTextureClipParameterChanged)
		{
			SetParameterForTextureClip(drawcallCount);
		}
	}
}


void UUIPanel::SetParameterForStandard(int drawcallCount)
{

}
void UUIPanel::SetParameterForRectClip(int drawcallCount)
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
void UUIPanel::SetParameterForTextureClip(int drawcallCount)
{
	auto textureClipOffsetAndSize = this->GetTextureClipOffsetAndSize();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto uiMat = UIMaterialList[i];
		uiMat->SetTextureParameterValue(FName("ClipTexture"), clipTexture);
		uiMat->SetVectorParameterValue(FName("TextureClipOffsetAndSize"), textureClipOffsetAndSize);
	}
}
bool UUIPanel::IsPointVisible(FVector worldPoint)
{
	//if not use clip or use texture clip, then point is visible. texture clip not support this calculation yet.
	switch (clipType)
	{
	case UIPanelClipType::None:
		return true;
		break;
	case UIPanelClipType::Rect:
	{
		CalculateRectRange();
		//transform to local space
		auto localPoint = this->GetComponentTransform().InverseTransformPosition(worldPoint);
		//out of range
		if (localPoint.X < rectMin.X) return false;
		if (localPoint.Y < rectMin.Y) return false;
		if (localPoint.X > rectMax.X) return false;
		if (localPoint.Y > rectMax.Y) return false;
	}
		break;
	case UIPanelClipType::Texture:
		return true;
		break;
	}
	
	return true;
}
void UUIPanel::CalculateRectRange()
{
	if (bRectRangeCalculated == false)//if not calculated yet
	{
		//calculate sefl rect range
		rectMin.X = -widget.pivot.X * widget.width;
		rectMin.Y = -widget.pivot.Y * widget.height;
		rectMax.X = (1.0f - widget.pivot.X) * widget.width;
		rectMax.Y = (1.0f - widget.pivot.Y) * widget.height;
		//calculate parent rect range
		if (inheritRectClip && ParentUIPanel != nullptr && ParentUIPanel->clipType == UIPanelClipType::Rect)
		{
			ParentUIPanel->CalculateRectRange();
			auto parentRectMin = FVector(ParentUIPanel->rectMin, 0);
			auto parentRectMax = FVector(ParentUIPanel->rectMax, 0);
			//transform ParentUIPanel's rect to this space
			auto& parentPanelTf = ParentUIPanel->GetComponentTransform();
			auto thisTfInv = this->GetComponentTransform().Inverse();
			parentRectMin = thisTfInv.TransformPosition(parentPanelTf.TransformPosition(parentRectMin));
			parentRectMax = thisTfInv.TransformPosition(parentPanelTf.TransformPosition(parentRectMax));
			//inherit
			if (rectMin.X < parentRectMin.X)rectMin.X = parentRectMin.X;
			if (rectMin.Y < parentRectMin.Y)rectMin.Y = parentRectMin.Y;
			if (rectMax.X > parentRectMax.X)rectMax.X = parentRectMax.X;
			if (rectMax.Y > parentRectMax.Y)rectMax.Y = parentRectMax.Y;
		}

		bRectRangeCalculated = true;
	}
}


FLinearColor UUIPanel::GetRectClipOffsetAndSize()
{
	CalculateRectRange();
	return FLinearColor((rectMin.X - clipRectOffset.Left + rectMax.X + clipRectOffset.Right) * 0.5f, (rectMin.Y - clipRectOffset.Bottom + rectMax.Y + clipRectOffset.Top) * 0.5f, (rectMax.X - rectMin.X + clipRectOffset.Left + clipRectOffset.Right) * 0.5f, (rectMax.Y - rectMin.Y + clipRectOffset.Top + clipRectOffset.Bottom) * 0.5f);
}
FLinearColor UUIPanel::GetRectClipFeather()
{
	return FLinearColor(clipFeather.X, clipFeather.Y, 0, 0);
}
FLinearColor UUIPanel::GetTextureClipOffsetAndSize()
{
	return FLinearColor(widget.width * -widget.pivot.X, widget.height * -widget.pivot.Y, widget.width, widget.height);
}

void UUIPanel::WidthChanged()
{
	Super::WidthChanged();
	bRectClipParameterChanged = true;
	bRectRangeCalculated = false;
}
void UUIPanel::HeightChanged() {
	Super::HeightChanged();
	bRectClipParameterChanged = true;
	bRectRangeCalculated = false;
}

void UUIPanel::SetClipType(UIPanelClipType newClipType) 
{
	if (clipType != newClipType)
	{
		bClipTypeChanged = true;
		clipType = newClipType;
		MarkPanelUpdate();
	}
}
void UUIPanel::SetRectClipFeather(FVector2D newFeather) 
{
	if (clipFeather != newFeather)
	{
		bRectClipParameterChanged = true;
		clipFeather = newFeather;
		MarkPanelUpdate();
	}
}
void UUIPanel::SetClipTexture(UTexture* newTexture) 
{
	if (clipTexture != newTexture)
	{
		bTextureClipParameterChanged = true;
		clipTexture = newTexture;
		MarkPanelUpdate();
	}
}
void UUIPanel::SetInheriRectClip(bool newBool)
{
	if (inheritRectClip != newBool)
	{
		inheritRectClip = newBool;
		bRectRangeCalculated = false;
		MarkPanelUpdate();
	}
}
void UUIPanel::SetUIPanelDepth(int32 newDepth, bool propagateToChildrenPanel)
{
	if (widget.depth != newDepth)
	{
		int32 diff = newDepth - widget.depth;
		this->SetDepth(newDepth);
		if (propagateToChildrenPanel)
		{
			if (LGUIManager::IsManagerValid(this->GetWorld()))
			{
				auto& allPanelArray = IsScreenSpaceOverlayUI() ? uiRootComp->GetPanelsBelongToThis() : LGUIManager::GetAllUIPanel(this->GetWorld());
				for (UUIPanel* itemPanel : allPanelArray)
				{
					if (IsValid(itemPanel))
					{
						if (itemPanel->IsAttachedTo(this))
						{
							itemPanel->SetDepth(itemPanel->GetDepth() + diff);
						}
					}
				}
			}
		}
		if (IsScreenSpaceOverlayUI())
		{
			uiRootComp->SortUIPanelOnDepth();
		}
		else
		{
			LGUIManager::SortUIPanelOnDepth(this->GetWorld());
			if (IsValid(uiRootComp))
			{
				uiRootComp->SortUIPanelOnDepth();
			}
			SortDrawcallRenderPriority();
		}
	}
}
void UUIPanel::SetUIPanelDepthToHighestOfHierarchy(bool propagateToChildrenPanel)
{
	if (CheckFirstUIPanel())
	{
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allPanelArray = IsScreenSpaceOverlayUI() ? uiRootComp->GetPanelsBelongToThis() : LGUIManager::GetAllUIPanel(this->GetWorld());

		int32 maxDepth = 0;
		for (UUIPanel* itemPanel : allPanelArray)
		{
			if (IsValid(itemPanel))
			{
				if (itemPanel->FirstUIPanel == this->FirstUIPanel)//on the same hierarchy
				{
					if (maxDepth < itemPanel->widget.depth)
					{
						maxDepth = itemPanel->widget.depth;
					}
				}
			}
		}
		int32 desireDepth = maxDepth + 1;
		SetUIPanelDepth(desireDepth, propagateToChildrenPanel);
	}
}
void UUIPanel::SetUIPanelDepthToLowestOfHierarchy(bool propagateToChildrenPanel)
{
	if (CheckFirstUIPanel())
	{
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allPanelArray = IsScreenSpaceOverlayUI() ? uiRootComp->GetPanelsBelongToThis() : LGUIManager::GetAllUIPanel(this->GetWorld());

		int32 minDepth = 0;
		for (UUIPanel* itemPanel : allPanelArray)
		{
			if (IsValid(itemPanel))
			{
				if (itemPanel->FirstUIPanel == this->FirstUIPanel)//on the same hierarchy
				{
					if (minDepth > itemPanel->widget.depth)
					{
						minDepth = itemPanel->widget.depth;
					}
				}
			}
		}
		int32 desireDepth = minDepth - 1;
		SetUIPanelDepth(desireDepth, propagateToChildrenPanel);
	}
}
void UUIPanel::SetUIPanelDepthToHighestOfAll(bool propagateToChildrenPanel)
{
	if (CheckFirstUIPanel())
	{
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allPanelArray = IsScreenSpaceOverlayUI() ? uiRootComp->GetPanelsBelongToThis() : LGUIManager::GetAllUIPanel(this->GetWorld());

		int32 maxDepth = 0;
		for (UUIPanel* itemPanel : allPanelArray)
		{
			if (IsValid(itemPanel))
			{
				if (itemPanel != this)
				{
					if (maxDepth < itemPanel->widget.depth)
					{
						maxDepth = itemPanel->widget.depth;
					}
				}
			}
		}
		int32 desireDepth = maxDepth + 1;
		SetUIPanelDepth(desireDepth, propagateToChildrenPanel);
	}
}
void UUIPanel::SetUIPanelDepthToLowestOfAll(bool propagateToChildrenPanel)
{
	if (CheckFirstUIPanel())
	{
		if (!LGUIManager::IsManagerValid(this->GetWorld()))return;
		auto& allPanelArray = IsScreenSpaceOverlayUI() ? uiRootComp->GetPanelsBelongToThis() : LGUIManager::GetAllUIPanel(this->GetWorld());

		int32 minDepth = 0;
		for (UUIPanel* itemPanel : allPanelArray)
		{
			if (IsValid(itemPanel))
			{
				if (itemPanel != this)
				{
					if (minDepth > itemPanel->widget.depth)
					{
						minDepth = itemPanel->widget.depth;
					}
				}
			}
		}
		int32 desireDepth = minDepth - 1;
		SetUIPanelDepth(desireDepth, propagateToChildrenPanel);
	}
}


void UUIPanel::SetDefaultMaterials(UMaterialInterface* InMaterials[3])
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
	//create new material
	int drawcallCount = UIDrawcallList.Num();
	for (int i = 0; i < drawcallCount; i++)
	{
		auto uiDrawcall = UIDrawcallList[i];
		UMaterialInterface* SrcMaterial = nullptr;
		if (uiDrawcall->material != nullptr)//custom material
		{
			SrcMaterial = uiDrawcall->material;
		}
		else
		{
			SrcMaterial = DefaultMaterials[(int)clipType];
			if (SrcMaterial == nullptr)
			{
				UE_LOG(LGUI, Log, TEXT("[UIPanel::SetDefaultMaterials]Material asset from Plugin/Content is missing! Reinstall this plugin may fix the issure"));
				UIMaterialList.Empty();
				return;
			}
		}
		auto uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this->GetOwner());
		uiMat->SetFlags(RF_Transient);
		UIMaterialList[i] = uiMat;
		uiMat->SetTextureParameterValue(FName("MainTexture"), uiDrawcall->texture);
		uiMat->SetScalarParameterValue(FName("IsFontTexture"), uiDrawcall->isFontTexture);
		UIMeshList[i]->SetMaterial(0, uiMat);
	}
	//set clip parameter
	if (clipType == UIPanelClipType::None)
	{
		SetParameterForStandard(drawcallCount);
	}
	else if (clipType == UIPanelClipType::Rect)
	{
		if (bClipTypeChanged || bRectClipParameterChanged)
		{
			SetParameterForRectClip(drawcallCount);
		}
	}
	else if (clipType == UIPanelClipType::Texture)
	{
		if (bClipTypeChanged || bTextureClipParameterChanged)
		{
			SetParameterForTextureClip(drawcallCount);
		}
	}
}
void UUIPanel::SetUIOnlyOwnerSee(bool InValue)
{
	if (bOnlyOwnerSee != InValue)
	{
		bOnlyOwnerSee = InValue;
		for (int i = 0; i < UIMeshList.Num(); i++)
		{
			UIMeshList[i]->SetOnlyOwnerSee(bOnlyOwnerSee);
		}
	}
}
void UUIPanel::SetUIOwnerNoSee(bool InValue)
{
	if (bOwnerNoSee != InValue)
	{
		bOwnerNoSee = InValue;
		for (int i = 0; i < UIMeshList.Num(); i++)
		{
			UIMeshList[i]->SetOwnerNoSee(bOwnerNoSee);
		}
	}
}
bool UUIPanel::ShouldSnapPixel()
{
	if (IsValid(uiRootComp))
	{
		return uiRootComp->ShouldSnapPixel();
	}
	return false;
}
UUIPanel* UUIPanel::GetUIRootPanel()
{
	if (IsValid(uiRootComp))
	{
		return uiRootComp->GetRootUIPanel();
	}
	return nullptr;
}

void UUIPanel::SetDynamicPixelsPerUnit(float newValue)
{
	if (dynamicPixelsPerUnit != newValue)
	{
		dynamicPixelsPerUnit = newValue;
		MarkRebuildAllDrawcall();
	}
}