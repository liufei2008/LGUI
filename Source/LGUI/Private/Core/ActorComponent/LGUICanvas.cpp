﻿// Copyright 2019-2021 LexLiu. All Rights Reserved.

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

//PRAGMA_DISABLE_OPTIMIZATION

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

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;	
	bShouldUpdateLayout = true;
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
	//clear and delete mesh components
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcall = iter->GetValue();
		if (drawcall->drawcallMesh.IsValid())
		{
			drawcall->drawcallMesh->DestroyComponent();
		}
		drawcall->drawcallMesh = nullptr;
		if (drawcall->postProcessRenderableObject.IsValid())
		{
			if (drawcall->postProcessRenderableObject->IsRenderProxyValid())
			{
				drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromHudRenderer();
			}
		}
		if (drawcall->directMeshRenderableObject.IsValid())
		{
			drawcall->directMeshRenderableObject->ClearDrawcallMesh(true);
		}
	}
	UIDrawcallList.Empty();
	PooledUIMeshList.Empty();
}

void ULGUICanvas::OnUIActiveStateChanged(bool active)
{
	
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
		//clear and delete mesh components, so new mesh will be created. because hud and world mesh not compatible
		for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
		{
			auto drawcall = iter->GetValue();
			if (drawcall->drawcallMesh.IsValid())
			{
				drawcall->drawcallMesh->DestroyComponent();
			}
			drawcall->drawcallMesh = nullptr;
			if (drawcall->postProcessRenderableObject.IsValid())
			{
				if (drawcall->postProcessRenderableObject->IsRenderProxyValid())
				{
					drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromHudRenderer();
				}
			}
			if (drawcall->directMeshRenderableObject.IsValid())
			{
				drawcall->directMeshRenderableObject->ClearDrawcallMesh(true);
			}
		}
		UIDrawcallList.Empty();
		PooledUIMeshList.Reset();
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

void ULGUICanvas::MarkRebuildAllDrawcall()
{
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto item = iter->GetValue();
		item->needToRebuildMesh = true;
		item->materialChanged = true;
		item->textureChanged = true;
		item->materialInstanceDynamic.Reset();
	}
}
void ULGUICanvas::SetUIElementDepthChange(UUIBaseRenderable* item)
{
	RemoveUIRenderable(item);
	AddUIRenderable(item);
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

DECLARE_CYCLE_STAT(TEXT("Canvas DrawcallBatch"), STAT_DrawcallBatch, STATGROUP_LGUI);
void ULGUICanvas::AddUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawcallBatch);
	auto SplitDrawcall = [&](TDoubleLinkedList<TSharedPtr<UUIDrawcall>>::TDoubleLinkedListNode* nodePtrToInsert
		, int32 uiElementDepth
		, TSharedPtr<UUIDrawcall> prevNodeDrawcall
		, TFunction<TSharedPtr<UUIDrawcall>(UMaterialInterface*)> NewDrawcall
		)
	{
		//split first drawcall: from depthMin to split point depth(include).
		//split second drawcall: from split point depth(no include) to depthMax.

		auto firstDrawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
		firstDrawcall->type = EUIDrawcallType::BatchGeometry;
		firstDrawcall->texture = prevNodeDrawcall->texture;
		auto secondDrawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
		secondDrawcall->type = EUIDrawcallType::BatchGeometry;
		secondDrawcall->texture = prevNodeDrawcall->texture;

		int32 firstDrawcallDepthMin = prevNodeDrawcall->depthMin;
		int32 firstDrawcallDepthMax = uiElementDepth;
		//int32 secondDrawcallDepthMin = uiElementDepth + 1;
		//int32 secondDrawcallDepthMax = prevNodeDrawcall->depthMax;
		for (int i = 0; i < prevNodeDrawcall->geometryList.Num(); i++)
		{
			auto renderObject = prevNodeDrawcall->renderObjectList[i];
			if (renderObject->GetDepth() >= firstDrawcallDepthMin && renderObject->GetDepth() <= firstDrawcallDepthMax)
			{
				firstDrawcall->geometryList.Add(prevNodeDrawcall->geometryList[i]);
				firstDrawcall->renderObjectList.Add(prevNodeDrawcall->renderObjectList[i]);
				prevNodeDrawcall->renderObjectList[i]->drawcall = firstDrawcall;
			}
			else
			{
				secondDrawcall->geometryList.Add(prevNodeDrawcall->geometryList[i]);
				secondDrawcall->renderObjectList.Add(prevNodeDrawcall->renderObjectList[i]);
				prevNodeDrawcall->renderObjectList[i]->drawcall = secondDrawcall;
			}
		}

		auto middleDrawcall = NewDrawcall(nullptr);
		InUIRenderable->drawcall = middleDrawcall;

		UIDrawcallList.InsertNode(firstDrawcall, nodePtrToInsert);
		UIDrawcallList.InsertNode(middleDrawcall, nodePtrToInsert);
		UIDrawcallList.InsertNode(secondDrawcall, nodePtrToInsert);
		UIDrawcallList.RemoveNode(nodePtrToInsert);
	};
	check(InUIRenderable->drawcall == nullptr);
	bool hasAddNewDrawcall = false;
	
	switch (InUIRenderable->GetUIRenderableType())
	{
	case EUIRenderableType::UIBatchGeometryRenderable:
	{
		auto batchGeometryRenderable = (UUIBatchGeometryRenderable*)InUIRenderable;
		auto uiGeo = batchGeometryRenderable->GetGeometry();
		check(uiGeo.IsValid());

		auto NewDrawcall = [&](UMaterialInterface* mat)
		{
			hasAddNewDrawcall = true;
			auto drawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
			drawcall->texture = uiGeo->texture;
			drawcall->material = mat;
			drawcall->geometryList.Add(uiGeo);
			drawcall->renderObjectList.Add(batchGeometryRenderable);
			drawcall->type = EUIDrawcallType::BatchGeometry;
			drawcall->UpdateDepthRange();
			return drawcall;
		};

		if (UIDrawcallList.Num() == 0)
		{
			auto drawcall = NewDrawcall(uiGeo->material.IsValid() ? uiGeo->material.Get() : nullptr);
			UIDrawcallList.AddHead(drawcall);
			InUIRenderable->drawcall = drawcall;
		}
		else
		{
			if (uiGeo->material.IsValid())//consider every custom material as a drawcall
			{
				auto drawcall = NewDrawcall(uiGeo->material.Get());
				bool haveInsert = false;
				//find a place to insert this drawcall
				for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
				{
					auto item = iter->GetValue();
					if (drawcall->depthMin <= item->depthMin)
					{
						UIDrawcallList.InsertNode(drawcall, iter);
						haveInsert = true;
						break;
					}
				}
				if (!haveInsert)
				{
					UIDrawcallList.AddTail(drawcall);
				}
				InUIRenderable->drawcall = drawcall;
			}
			else//batch elements into drawcall by comparing depth and texture
			{
				auto uiElementDepth = InUIRenderable->GetDepth();
				TDoubleLinkedList<TSharedPtr<UUIDrawcall>>::TDoubleLinkedListNode* nodePtrToInsert = nullptr;
				for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
				{
					auto item = iter->GetValue();
					if (uiElementDepth <= item->depthMin)
					{
						nodePtrToInsert = iter;
						break;
					}
				}
				
				if (nodePtrToInsert == nullptr)//at tail
				{
					auto tailNode = UIDrawcallList.GetTail();
					auto nodeDrawcall = tailNode->GetValue();
					if (uiGeo->texture == nodeDrawcall->texture)//same texture, can batch
					{
						nodeDrawcall->geometryList.Add(uiGeo);
						nodeDrawcall->renderObjectList.Add(batchGeometryRenderable);
						nodeDrawcall->needToRebuildMesh = true;
						nodeDrawcall->UpdateDepthRange();
						InUIRenderable->drawcall = nodeDrawcall;
					}
					else//different texture, cannot batch. create new drawcall
					{
						auto drawcall = NewDrawcall(nullptr);
						UIDrawcallList.AddTail(drawcall);
						InUIRenderable->drawcall = drawcall;
					}
				}
				else//not tail, maybe head or middle
				{
					auto headNode = UIDrawcallList.GetHead();
					if (nodePtrToInsert == headNode)//head
					{
						auto nodeDrawcall = headNode->GetValue();
						if (uiGeo->texture == nodeDrawcall->texture)//same texture, can batch
						{
							nodeDrawcall->geometryList.Add(uiGeo);
							nodeDrawcall->renderObjectList.Add(batchGeometryRenderable);
							nodeDrawcall->needToRebuildMesh = true;
							nodeDrawcall->UpdateDepthRange();
							InUIRenderable->drawcall = nodeDrawcall;
						}
						else//different texture, cannot batch. create new drawcall
						{
							auto drawcall = NewDrawcall(nullptr);
							UIDrawcallList.AddHead(drawcall);
							InUIRenderable->drawcall = drawcall;
						}
					}
					else//middle
					{
						auto currentNodeDrawcall = nodePtrToInsert->GetValue();
						if (uiGeo->texture == currentNodeDrawcall->texture)//same texture, can batch
						{
							currentNodeDrawcall->geometryList.Add(uiGeo);
							currentNodeDrawcall->renderObjectList.Add(batchGeometryRenderable);
							currentNodeDrawcall->needToRebuildMesh = true;
							currentNodeDrawcall->UpdateDepthRange();
							InUIRenderable->drawcall = currentNodeDrawcall;
						}
						else//different texture, try prev node
						{
							auto prevNodeDrawcall = nodePtrToInsert->GetPrevNode()->GetValue();
							if (uiGeo->texture == prevNodeDrawcall->texture)//same texture, can batch
							{
								prevNodeDrawcall->geometryList.Add(uiGeo);
								prevNodeDrawcall->renderObjectList.Add(batchGeometryRenderable);
								prevNodeDrawcall->needToRebuildMesh = true;
								prevNodeDrawcall->UpdateDepthRange();
								InUIRenderable->drawcall = prevNodeDrawcall;
							}
							else//different texture, cannot batch. but may split the exist drawcall node
							{
								if (uiElementDepth >= prevNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
								{
									auto drawcall = NewDrawcall(nullptr);
									UIDrawcallList.InsertNode(drawcall, nodePtrToInsert);
									InUIRenderable->drawcall = drawcall;
								}
								else//inside depth range, must split drawcall
								{
									SplitDrawcall(nodePtrToInsert, uiElementDepth, prevNodeDrawcall, NewDrawcall);
								}
							}
						}
					}
				}
			}
		}
	}
	break;
	case EUIRenderableType::UIPostProcessRenderable:
	{
		auto postProcessRenderable = (UUIPostProcessRenderable*)InUIRenderable;
		auto uiGeo = postProcessRenderable->GetGeometry();
		check(uiGeo.IsValid());

		auto NewDrawcall = [&](UMaterialInterface* mat)
		{
			hasAddNewDrawcall = true;
			auto drawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
			drawcall->postProcessRenderableObject = postProcessRenderable;
			drawcall->type = EUIDrawcallType::PostProcess;
			return drawcall;
		};

		if (UIDrawcallList.Num() == 0)
		{
			auto drawcall = NewDrawcall(nullptr);
			UIDrawcallList.AddHead(drawcall);
			InUIRenderable->drawcall = drawcall;
		}
		else//find a place to insert this drawcall
		{
			auto uiElementDepth = InUIRenderable->GetDepth();
			TDoubleLinkedList<TSharedPtr<UUIDrawcall>>::TDoubleLinkedListNode* nodePtrToInsert = nullptr;
			for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
			{
				auto item = iter->GetValue();
				if (uiElementDepth <= item->depthMin)
				{
					nodePtrToInsert = iter;
					break;
				}
			}

			if (nodePtrToInsert == nullptr)//at tail
			{
				auto drawcall = NewDrawcall(nullptr);
				UIDrawcallList.AddTail(drawcall);
				InUIRenderable->drawcall = drawcall;
			}
			else//not tail, maybe head or middle
			{
				auto headNode = UIDrawcallList.GetHead();
				if (nodePtrToInsert == headNode)//head
				{
					auto drawcall = NewDrawcall(nullptr);
					UIDrawcallList.AddHead(drawcall);
					InUIRenderable->drawcall = drawcall;
				}
				else//middle
				{
					auto prevNodeDrawcall = nodePtrToInsert->GetPrevNode()->GetValue();
					if (uiElementDepth >= prevNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
					{
						auto drawcall = NewDrawcall(nullptr);
						UIDrawcallList.InsertNode(drawcall, nodePtrToInsert);
						InUIRenderable->drawcall = drawcall;
					}
					else//inside depth range, must split drawcall
					{
						SplitDrawcall(nodePtrToInsert, uiElementDepth, prevNodeDrawcall, NewDrawcall);
					}
				}
			}
		}
	}
	break;
	case EUIRenderableType::UIDirectMeshRenderable:
	{
		auto directMeshRenderable = (UUIDirectMeshRenderable*)InUIRenderable;

		auto NewDrawcall = [&](UMaterialInterface* mat)
		{
			hasAddNewDrawcall = true;
			auto drawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
			drawcall->directMeshRenderableObject = directMeshRenderable;
			drawcall->type = EUIDrawcallType::DirectMesh;
			return drawcall;
		};

		if (UIDrawcallList.Num() == 0)
		{
			auto drawcall = NewDrawcall(nullptr);
			UIDrawcallList.AddHead(drawcall);
			InUIRenderable->drawcall = drawcall;
		}
		else//find a place to insert this drawcall
		{
			auto uiElementDepth = InUIRenderable->GetDepth();
			TDoubleLinkedList<TSharedPtr<UUIDrawcall>>::TDoubleLinkedListNode* nodePtrToInsert = nullptr;
			for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
			{
				auto item = iter->GetValue();
				if (uiElementDepth <= item->depthMin)
				{
					nodePtrToInsert = iter;
					break;
				}
			}

			if (nodePtrToInsert == nullptr)//at tail
			{
				auto drawcall = NewDrawcall(nullptr);
				UIDrawcallList.AddTail(drawcall);
				InUIRenderable->drawcall = drawcall;
			}
			else//not tail, maybe head or middle
			{
				auto headNode = UIDrawcallList.GetHead();
				if (nodePtrToInsert == headNode)//head
				{
					auto drawcall = NewDrawcall(nullptr);
					UIDrawcallList.AddHead(drawcall);
					InUIRenderable->drawcall = drawcall;
				}
				else//middle
				{
					auto prevNodeDrawcall = nodePtrToInsert->GetPrevNode()->GetValue();
					if (uiElementDepth >= prevNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
					{
						auto drawcall = NewDrawcall(nullptr);
						UIDrawcallList.InsertNode(drawcall, nodePtrToInsert);
						InUIRenderable->drawcall = drawcall;
					}
					else//inside depth range, must split drawcall
					{
						SplitDrawcall(nodePtrToInsert, uiElementDepth, prevNodeDrawcall, NewDrawcall);
					}
				}
			}
		}
	}
	break;
	}

	if (hasAddNewDrawcall)
	{
		if (RootCanvas.IsValid())
		{
			RootCanvas->bNeedToSortRenderPriority = true;
		}
	}
	MarkCanvasUpdate();
}
void ULGUICanvas::RemoveUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	SCOPE_CYCLE_COUNTER(STAT_DrawcallBatch);
	bool shouldCheckDrawcallCombine = false;
	switch (InUIRenderable->GetUIRenderableType())
	{
	case EUIRenderableType::UIBatchGeometryRenderable:
	{
		auto batchGeometryRenderable = (UUIBatchGeometryRenderable*)InUIRenderable;
		auto drawcall = batchGeometryRenderable->drawcall;
		auto index = drawcall->renderObjectList.IndexOfByKey(batchGeometryRenderable);
		check(index != INDEX_NONE);
		drawcall->renderObjectList.RemoveAt(index);
		drawcall->geometryList.RemoveAt(index);
		drawcall->UpdateDepthRange();
		drawcall->needToRebuildMesh = true;
		batchGeometryRenderable->drawcall = nullptr;
		if (drawcall->geometryList.Num() == 0)
		{
			if (drawcall->drawcallMesh.IsValid())
			{
				drawcall->drawcallMesh->SetUIMeshVisibility(false);
				PooledUIMeshList.Add(drawcall->drawcallMesh);
			}
			drawcall->drawcallMesh = nullptr;
			UIDrawcallList.RemoveNode(drawcall);
			shouldCheckDrawcallCombine = true;
		}
	}
	break;
	case EUIRenderableType::UIPostProcessRenderable:
	case EUIRenderableType::UIDirectMeshRenderable:
	{
		auto drawcall = InUIRenderable->drawcall;
		UIDrawcallList.RemoveNode(drawcall);
		if (drawcall->drawcallMesh.IsValid())
		{
			drawcall->drawcallMesh->DestroyComponent();
		}
		drawcall->drawcallMesh = nullptr;
		if (drawcall->postProcessRenderableObject.IsValid())
		{
			if (drawcall->postProcessRenderableObject->IsRenderProxyValid())
			{
				drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromHudRenderer();
			}
		}
		if (drawcall->directMeshRenderableObject.IsValid())
		{
			drawcall->directMeshRenderableObject->ClearDrawcallMesh(true);
		}
		drawcall.Reset();
		InUIRenderable->drawcall = nullptr;
	}
	break;
	}

	if (shouldCheckDrawcallCombine)
	{
		//check drawcall to see if we can combine some
		for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
		{
			auto drawcallItem = iter->GetValue();
			UE_LOG(LGUI, Warning, TEXT("[TODO] Check drawcall combine"));
		}
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
	cacheForThisUpdate_ShouldUpdateLayout = bShouldUpdateLayout || parentLayoutChanged;
	cacheForThisUpdate_ClipTypeChanged = bClipTypeChanged;
	cacheForThisUpdate_RectClipParameterChanged = bRectClipParameterChanged || bShouldUpdateLayout || parentLayoutChanged;
	if (bRectRangeCalculated)
	{
		if (cacheForThisUpdate_RectClipParameterChanged)bRectRangeCalculated = false;
	}
	cacheForThisUpdate_TextureClipParameterChanged = bTextureClipParameterChanged;
	bShouldUpdateLayout = false;
	bClipTypeChanged = false;
	bRectClipParameterChanged = false;
	bTextureClipParameterChanged = false;

	//update layout and geometry
	UIItem->UpdateLayoutAndGeometry(parentLayoutChanged, cacheForThisUpdate_ShouldUpdateLayout);

	UpdateChildRecursive(UIItem.Get(), UIItem->cacheForThisUpdate_LayoutChanged);
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
		UpdateCanvasGeometry();
	}
}
void ULGUICanvas::UpdateCanvasGeometry()
{
	for (auto item : manageCanvasArray)
	{
		if (item == this)continue;//skip self
		if (item.IsValid() && item->GetIsUIActive())
		{
			item->UpdateCanvasGeometry();
		}
	}

	//update drawcall
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateDrawcall);
		for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
		{
			auto drawcallItem = iter->GetValue();
			switch (drawcallItem->type)
			{
			case EUIDrawcallType::DirectMesh:
			{
				auto uiMesh = drawcallItem->drawcallMesh;
				if (!uiMesh.IsValid())
				{
					uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), NAME_None, RF_Transient);
					uiMesh->SetOwnerNoSee(this->GetActualOwnerNoSee());
					uiMesh->SetOnlyOwnerSee(this->GetActualOnlyOwnerSee());
					uiMesh->RegisterComponent();
					uiMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
					uiMesh->SetRelativeTransform(FTransform::Identity);
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
					drawcallItem->drawcallMesh = uiMesh;
					drawcallItem->directMeshRenderableObject->SetDrawcallMesh(uiMesh.Get());
				}
			}
			break;
			case EUIDrawcallType::BatchGeometry:
			{
				auto uiMesh = drawcallItem->drawcallMesh;
				if (!uiMesh.IsValid())
				{
					if (PooledUIMeshList.Num() > 0)//get mesh from exist
					{
						do 
						{
							auto lastIndex = PooledUIMeshList.Num() - 1;
							uiMesh = PooledUIMeshList[lastIndex];
							uiMesh->SetUIMeshVisibility(true);
							PooledUIMeshList.RemoveAt(lastIndex);
						} while (!uiMesh.IsValid() && PooledUIMeshList.Num() > 0);
					}
					if (!uiMesh.IsValid())//not valid, create it
					{
//#if WITH_EDITOR
//						auto meshName = FString::Printf(TEXT("%s_Drawcall_%d"), *GetOwner()->GetActorLabel(), meshIndex);
//#else
//						auto meshName = FString::Printf(TEXT("Drawcall_%d"), meshIndex);
//#endif
						uiMesh = NewObject<UUIDrawcallMesh>(this->GetOwner(), NAME_None, RF_Transient);
						uiMesh->SetOwnerNoSee(this->GetActualOwnerNoSee());
						uiMesh->SetOnlyOwnerSee(this->GetActualOnlyOwnerSee());
						uiMesh->RegisterComponent();
						uiMesh->AttachToComponent(this->GetOwner()->GetRootComponent(), FAttachmentTransformRules::KeepRelativeTransform);
						uiMesh->SetRelativeTransform(FTransform::Identity);
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
					drawcallItem->drawcallMesh = uiMesh;
				}
				if (drawcallItem->needToRebuildMesh)
				{
					auto& meshSection = uiMesh->MeshSection;
					meshSection.Reset();
					drawcallItem->GetCombined(meshSection.vertices, meshSection.triangles);
					uiMesh->GenerateOrUpdateMesh(true, GetActualAdditionalShaderChannelFlags());
					drawcallItem->needToRebuildMesh = false;
					drawcallItem->needToUpdateVertex = false;
					drawcallItem->vertexPositionChanged = false;
				}
				else if (drawcallItem->needToUpdateVertex)
				{
					auto& meshSection = uiMesh->MeshSection;
					drawcallItem->UpdateData(meshSection.vertices, meshSection.triangles);
					uiMesh->GenerateOrUpdateMesh(drawcallItem->vertexPositionChanged, GetActualAdditionalShaderChannelFlags());
					drawcallItem->needToUpdateVertex = false;
					drawcallItem->vertexPositionChanged = false;
				}
			}
			break;
			case EUIDrawcallType::PostProcess:
			{
				if (!drawcallItem->postProcessRenderableObject->IsRenderProxyValid())
				{
					drawcallItem->postProcessRenderableObject->GetRenderProxy();
					drawcallItem->needToAddPostProcessRenderProxyToRender = true;
				}
				if (drawcallItem->needToAddPostProcessRenderProxyToRender)
				{
					drawcallItem->needToAddPostProcessRenderProxyToRender = false;
					auto uiPostProcessPrimitive = drawcallItem->postProcessRenderableObject->GetRenderProxy();
#if WITH_EDITOR
					if (!GetWorld()->IsGameWorld())
					{
						if (currentIsRenderToRenderTargetOrWorld)
						{
							if (RootCanvas->GetViewExtension())
							{
								uiPostProcessPrimitive->AddToHudRenderer(RootCanvas->GetViewExtension());
								uiPostProcessPrimitive->SetVisibility(true);
							}
						}
					}
					else
#endif
						if (currentIsRenderToRenderTargetOrWorld)
						{
							uiPostProcessPrimitive->AddToHudRenderer(RootCanvas->GetViewExtension());
							uiPostProcessPrimitive->SetVisibility(true);
						}
				}
			}
			break;
			}
		}

		//after geometry created, need to sort UIMesh render order
		if (UIItem->cacheForThisUpdate_DepthChanged)
		{
			RootCanvas->bNeedToSortRenderPriority = true;
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
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcallItem = iter->GetValue();
		switch (drawcallItem->type)
		{
		case EUIDrawcallType::BatchGeometry:
		{
			drawcallItem->drawcallMesh->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
		break;
		case EUIDrawcallType::DirectMesh:
		{
			drawcallItem->drawcallMesh->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			drawcallItem->postProcessRenderableObject->GetRenderProxy()->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
		break;
		}
	}
	return UIDrawcallList.Num();
}

void ULGUICanvas::UpdateAndApplyMaterial()
{
	auto tempClipType = GetActualClipType();
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcallItem = iter->GetValue();
		if (drawcallItem->type == EUIDrawcallType::BatchGeometry)
		{
			auto uiMat = drawcallItem->materialInstanceDynamic;
			if (!uiMat.IsValid() || drawcallItem->materialChanged || cacheForThisUpdate_ClipTypeChanged)
			{
				if (drawcallItem->material.IsValid())//custom material
				{
					auto SrcMaterial = drawcallItem->material.Get();
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
					uiMat = GetUIMaterialFromPool(clipType);
				}
				drawcallItem->drawcallMesh->SetMaterial(0, uiMat.Get());
				drawcallItem->materialInstanceDynamic = uiMat;
				drawcallItem->materialChanged = false;
				uiMat->SetTextureParameterValue(FName("MainTexture"), drawcallItem->texture.Get());
				drawcallItem->textureChanged = false;
			}
			else if (drawcallItem->textureChanged)
			{
				uiMat->SetTextureParameterValue(FName("MainTexture"), drawcallItem->texture.Get());
				drawcallItem->textureChanged = false;
			}
		}
		else if (drawcallItem->type == EUIDrawcallType::PostProcess)
		{
			if (cacheForThisUpdate_ClipTypeChanged
				|| drawcallItem->materialChanged//maybe it is newly created, so check the materialChanged parameter
				)
			{
				if (drawcallItem->postProcessRenderableObject.IsValid())
				{
					drawcallItem->postProcessRenderableObject->SetClipType(tempClipType);
					drawcallItem->materialChanged = false;
				}
			}
		}
		else if (drawcallItem->type == EUIDrawcallType::DirectMesh)
		{
			if (cacheForThisUpdate_ClipTypeChanged
				|| drawcallItem->materialChanged//maybe it is newly created, so check the materialChanged parameter
				)
			{
				if (drawcallItem->directMeshRenderableObject.IsValid())
				{
					drawcallItem->directMeshRenderableObject->SetClipType(tempClipType);
					drawcallItem->materialChanged = false;
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
		SetParameterForStandard();
	}
	break;
	case ELGUICanvasClipType::Rect:
	{
		if (cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_RectClipParameterChanged)
		{
			SetParameterForRectClip();
		}
	}
	break;
	case ELGUICanvasClipType::Texture:
	{
		if (cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_TextureClipParameterChanged)
		{
			SetParameterForTextureClip();
		}
	}
	break;
	}
}

UMaterialInstanceDynamic* ULGUICanvas::GetUIMaterialFromPool(ELGUICanvasClipType inClipType)
{
	if (PooledUIMaterialList.Num() == 0)
	{
		PooledUIMaterialList.Add({});
		PooledUIMaterialList.Add({});
		PooledUIMaterialList.Add({});
	}
	auto& matList = PooledUIMaterialList[(int)inClipType].MaterialList;
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
void ULGUICanvas::AddUIMaterialToPool(UMaterialInstanceDynamic* uiMat)
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
		auto& matList = PooledUIMaterialList[cacheMatTypeIndex].MaterialList;
		matList.Add(uiMat);
	}
}

void ULGUICanvas::SetParameterForStandard()
{

}
void ULGUICanvas::SetParameterForRectClip()
{
	auto rectClipOffsetAndSize = this->GetRectClipOffsetAndSize();
	auto rectClipFeather = this->GetRectClipFeather();
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcallItem = iter->GetValue();
		switch (drawcallItem->type)
		{
		default:
		case EUIDrawcallType::BatchGeometry:
		{
			auto uiMat = drawcallItem->materialInstanceDynamic;
			if (uiMat.IsValid())
			{
				uiMat->SetVectorParameterValue(FName("RectClipOffsetAndSize"), rectClipOffsetAndSize);
				uiMat->SetVectorParameterValue(FName("RectClipFeather"), rectClipFeather);
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			if (drawcallItem->postProcessRenderableObject.IsValid())
			{
				drawcallItem->postProcessRenderableObject->SetRectClipParameter(rectClipOffsetAndSize, rectClipFeather);
			}
		}
		break;
		case EUIDrawcallType::DirectMesh:
		{
			if (drawcallItem->directMeshRenderableObject.IsValid())
			{
				drawcallItem->directMeshRenderableObject->SetRectClipParameter(rectClipOffsetAndSize, rectClipFeather);
			}
		}
		break;
		}
	}
}
void ULGUICanvas::SetParameterForTextureClip()
{
	auto textureClipOffsetAndSize = this->GetTextureClipOffsetAndSize();
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcallItem = iter->GetValue();
		switch (drawcallItem->type)
		{
		default:
		case EUIDrawcallType::BatchGeometry:
		{
			auto uiMat = drawcallItem->materialInstanceDynamic;
			if (uiMat.IsValid())
			{
				uiMat->SetTextureParameterValue(FName("ClipTexture"), clipTexture);
				uiMat->SetVectorParameterValue(FName("TextureClipOffsetAndSize"), textureClipOffsetAndSize);
			}
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			if (drawcallItem->postProcessRenderableObject.IsValid())
			{
				drawcallItem->postProcessRenderableObject->SetTextureClipParameter(clipTexture, textureClipOffsetAndSize);
			}
		}
		break;
		case EUIDrawcallType::DirectMesh:
		{
			if (drawcallItem->directMeshRenderableObject.IsValid())
			{
				drawcallItem->directMeshRenderableObject->SetTextureClipParameter(clipTexture, textureClipOffsetAndSize);
			}
		}
		break;
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
	PooledUIMaterialList.Reset();
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
	auto tempOwnerNoSee = this->GetActualOwnerNoSee();
	auto tempOnlyOwnerSee = this->GetActualOnlyOwnerSee();

	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcall = iter->GetValue();
		if (drawcall->drawcallMesh.IsValid())
		{
			drawcall->drawcallMesh->SetOwnerNoSee(tempOwnerNoSee);
			drawcall->drawcallMesh->SetOnlyOwnerSee(tempOnlyOwnerSee);
		}
		if (drawcall->directMeshRenderableObject.IsValid())
		{
			drawcall->directMeshRenderableObject->ClearDrawcallMesh(true);
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

//PRAGMA_ENABLE_OPTIMIZATION