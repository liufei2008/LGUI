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

//PRAGMA_DISABLE_OPTIMIZATION

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
}

void ULGUICanvas::BeginPlay()
{
	Super::BeginPlay();
	RootCanvas = nullptr;
	CheckRootCanvas();
	bCurrentIsLGUIRendererOrUERenderer = IsRenderByLGUIRendererOrUERenderer();
	ParentCanvas = nullptr;
	CheckParentCanvas();
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
#ifdef LGUI_DRAWCALLMODE_AUTO
		CacheUIItemToCanvasTransformMap.Reset();
#endif
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

		UpdateCanvasDrawcall();
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
	OnUIHierarchyChanged();
	//tell UIItem
	if (CheckUIItem())
	{
		UIItem->RegisterRenderCanvas();
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
	if (UIItem.IsValid())
	{
		UIItem->UnregisterRenderCanvas();
	}

	//clear and delete mesh components
	ClearDrawcall();

	RemoveFromViewExtension();
}
void ULGUICanvas::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void ULGUICanvas::ClearDrawcall()
{
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcall = iter->GetValue();
		drawcall->drawcallMeshSection = nullptr;
		if (drawcall->postProcessRenderableObject.IsValid())
		{
			if (drawcall->postProcessRenderableObject->IsRenderProxyValid())
			{
				drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
			}
		}
		if (drawcall->directMeshRenderableObject.IsValid())
		{
			drawcall->directMeshRenderableObject->ClearMeshData();
		}
		if (drawcall->drawcallMesh.IsValid())
		{
			drawcall->drawcallMesh->DestroyComponent();
			drawcall->drawcallMesh.Reset();
		}
		for (int i = 0; i < drawcall->renderObjectList.Num(); i++)
		{
			if (drawcall->renderObjectList[i].IsValid())
			{
				drawcall->renderObjectList[i]->drawcall = nullptr;
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
}
void ULGUICanvas::OnUIHierarchyChanged()
{
	if (RootCanvas.IsValid())
	{
		//remove from old
		RootCanvas->manageCanvasArray.Remove(this);
		//mark old RootCanvas update
		RootCanvas->bCanTickUpdate = true;
	}
	RootCanvas = nullptr;
	CheckRootCanvas();
	if (RootCanvas.IsValid())
	{
		//add to new
		RootCanvas->manageCanvasArray.Add(this);
		//mark new RootCanvas update
		RootCanvas->bCanTickUpdate = true;
	}
	CheckRenderMode();

	bClipTypeChanged = true;
	bRectClipParameterChanged = true;
	bTextureClipParameterChanged = true;
	bRectRangeCalculated = false;

	ParentCanvas = nullptr;
	CheckParentCanvas();
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
	CheckParentCanvas();

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

bool ULGUICanvas::GetIsUIActive()const
{
	if (UIItem.IsValid())
	{
		return UIItem->GetIsUIActiveInHierarchy();
	}
	return false;
}

//DECLARE_CYCLE_STAT(TEXT("Canvas DrawcallBatch"), STAT_DrawcallBatch, STATGROUP_LGUI);
void ULGUICanvas::AddUIRenderable(UUIBaseRenderable* InUIRenderable)
{
	//SCOPE_CYCLE_COUNTER(STAT_DrawcallBatch);
	auto SplitDrawcall = [&](TDoubleLinkedList<TSharedPtr<UUIDrawcall>>::TDoubleLinkedListNode* drawcallNodePtr
		, int32 uiElementDepth
		, TFunction<TSharedPtr<UUIDrawcall>(UMaterialInterface*)> NewDrawcall
		)
	{
		TSharedPtr<UUIDrawcall> drawcallToSplit = drawcallNodePtr->GetValue();
		//split first drawcall: from depthMin to split point depth(include).
		//split second drawcall: from split point depth(no include) to depthMax.

		auto firstDrawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
		firstDrawcall->type = EUIDrawcallType::BatchGeometry;
		firstDrawcall->texture = drawcallToSplit->texture;
		auto secondDrawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
		secondDrawcall->type = EUIDrawcallType::BatchGeometry;
		secondDrawcall->texture = drawcallToSplit->texture;

		int32 firstDrawcallDepthMin = drawcallToSplit->depthMin;
		int32 firstDrawcallDepthMax = uiElementDepth;
		//int32 secondDrawcallDepthMin = uiElementDepth + 1;
		//int32 secondDrawcallDepthMax = drawcallToSplit->depthMax;
		for (int i = 0; i < drawcallToSplit->renderObjectList.Num(); i++)
		{
			auto renderObject = drawcallToSplit->renderObjectList[i];
			if (renderObject->GetDepth() >= firstDrawcallDepthMin && renderObject->GetDepth() <= firstDrawcallDepthMax)
			{
				firstDrawcall->renderObjectList.Add(renderObject);
				renderObject->drawcall = firstDrawcall;
			}
			else
			{
				secondDrawcall->renderObjectList.Add(renderObject);
				renderObject->drawcall = secondDrawcall;
			}
		}

		auto middleDrawcall = NewDrawcall(nullptr);

		UIDrawcallList.InsertNode(firstDrawcall, drawcallNodePtr);
		UIDrawcallList.InsertNode(middleDrawcall, drawcallNodePtr);
		UIDrawcallList.InsertNode(secondDrawcall, drawcallNodePtr);
		if (drawcallToSplit->drawcallMeshSection.IsValid())
		{
			drawcallToSplit->drawcallMesh->DeleteMeshSection(drawcallToSplit->drawcallMeshSection.Pin());
			drawcallToSplit->drawcallMeshSection.Reset();
		}
		if (drawcallToSplit->materialInstanceDynamic.IsValid())
		{
			AddUIMaterialToPool(drawcallToSplit->materialInstanceDynamic.Get());
			drawcallToSplit->materialInstanceDynamic.Reset();
		}
		UIDrawcallList.RemoveNode(drawcallNodePtr);
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
			drawcall->renderObjectList.Add(batchGeometryRenderable);
			drawcall->type = EUIDrawcallType::BatchGeometry;
			drawcall->UpdateDepthRange();
			InUIRenderable->drawcall = drawcall;
			return drawcall;
		};
		auto InsertIntoRenderObjectList = [](TArray<TWeakObjectPtr<UUIBatchGeometryRenderable>>& list, UUIBatchGeometryRenderable* item)
		{
			bool insertSuccess = false;
			for (int i = 0; i < list.Num(); i++)
			{
				if (item->GetDepth() <= list[i]->GetDepth())
				{
					list.Insert(item, i);
					insertSuccess = true;
					break;
				}
			}
			if (!insertSuccess)
			{
				list.Add(item);
			}
		};

		if (UIDrawcallList.Num() == 0)
		{
			auto drawcall = NewDrawcall(uiGeo->material.IsValid() ? uiGeo->material.Get() : nullptr);
			UIDrawcallList.AddHead(drawcall);
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
				
				if (nodePtrToInsert == nullptr)//at tail, check: (batch, new dracall, new drawcall and split)
				{
					auto tailNode = UIDrawcallList.GetTail();
					auto tailNodeDrawcall = tailNode->GetValue();
					if (uiGeo->texture == tailNodeDrawcall->texture)//same texture, can batch
					{
						InsertIntoRenderObjectList(tailNodeDrawcall->renderObjectList, batchGeometryRenderable);
						tailNodeDrawcall->needToRebuildMesh = true;
						tailNodeDrawcall->UpdateDepthRange();
						InUIRenderable->drawcall = tailNodeDrawcall;
					}
					else//different texture, cannot batch. but may split the exist drawcall node
					{
						if (uiElementDepth >= tailNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
						{
							auto drawcall = NewDrawcall(nullptr);
							UIDrawcallList.AddTail(drawcall);
						}
						else//inside depth range, must split drawcall
						{
							SplitDrawcall(tailNode, uiElementDepth, NewDrawcall);
						}
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
							InsertIntoRenderObjectList(nodeDrawcall->renderObjectList, batchGeometryRenderable);
							nodeDrawcall->needToRebuildMesh = true;
							nodeDrawcall->UpdateDepthRange();
							InUIRenderable->drawcall = nodeDrawcall;
						}
						else//different texture, cannot batch. create new drawcall
						{
							auto drawcall = NewDrawcall(nullptr);
							UIDrawcallList.AddHead(drawcall);
						}
					}
					else//middle
					{
						auto currentNodeDrawcall = nodePtrToInsert->GetValue();
						if (uiGeo->texture == currentNodeDrawcall->texture)//same texture, can batch
						{
							InsertIntoRenderObjectList(currentNodeDrawcall->renderObjectList, batchGeometryRenderable);
							currentNodeDrawcall->needToRebuildMesh = true;
							currentNodeDrawcall->UpdateDepthRange();
							InUIRenderable->drawcall = currentNodeDrawcall;
						}
						else//different texture, try prev node
						{
							auto prevNodeDrawcall = nodePtrToInsert->GetPrevNode()->GetValue();
							if (uiGeo->texture == prevNodeDrawcall->texture)//same texture, can batch
							{
								InsertIntoRenderObjectList(prevNodeDrawcall->renderObjectList, batchGeometryRenderable);
								prevNodeDrawcall->needToRebuildMesh = true;
								prevNodeDrawcall->UpdateDepthRange();
								InUIRenderable->drawcall = prevNodeDrawcall;
							}
							else//different texture, cannot batch. but may split the existing drawcall node
							{
								if (uiElementDepth >= prevNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
								{
									auto drawcall = NewDrawcall(nullptr);
									UIDrawcallList.InsertNode(drawcall, nodePtrToInsert);
								}
								else//inside depth range, must split drawcall
								{
									SplitDrawcall(nodePtrToInsert->GetPrevNode(), uiElementDepth, NewDrawcall);
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
		auto NewDrawcall = [&](UMaterialInterface* mat)
		{
			hasAddNewDrawcall = true;
			auto drawcall = TSharedPtr<UUIDrawcall>(new UUIDrawcall);
			drawcall->postProcessRenderableObject = postProcessRenderable;
			drawcall->type = EUIDrawcallType::PostProcess;
			drawcall->UpdateDepthRange();
			InUIRenderable->drawcall = drawcall;
			return drawcall;
		};

		if (UIDrawcallList.Num() == 0)
		{
			auto drawcall = NewDrawcall(nullptr);
			UIDrawcallList.AddHead(drawcall);
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
			}
			else//not tail, maybe head or middle
			{
				auto headNode = UIDrawcallList.GetHead();
				if (nodePtrToInsert == headNode)//head
				{
					auto drawcall = NewDrawcall(nullptr);
					UIDrawcallList.AddHead(drawcall);
				}
				else//middle
				{
					auto prevNodeDrawcall = nodePtrToInsert->GetPrevNode()->GetValue();
					if (uiElementDepth >= prevNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
					{
						auto drawcall = NewDrawcall(nullptr);
						UIDrawcallList.InsertNode(drawcall, nodePtrToInsert);
					}
					else//inside depth range, must split drawcall
					{
						SplitDrawcall(nodePtrToInsert->GetPrevNode(), uiElementDepth, NewDrawcall);
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
			drawcall->UpdateDepthRange();
			InUIRenderable->drawcall = drawcall;
			return drawcall;
		};

		if (UIDrawcallList.Num() == 0)
		{
			auto drawcall = NewDrawcall(nullptr);
			UIDrawcallList.AddHead(drawcall);
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
			}
			else//not tail, maybe head or middle
			{
				auto headNode = UIDrawcallList.GetHead();
				if (nodePtrToInsert == headNode)//head
				{
					auto drawcall = NewDrawcall(nullptr);
					UIDrawcallList.AddHead(drawcall);
				}
				else//middle
				{
					auto prevNodeDrawcall = nodePtrToInsert->GetPrevNode()->GetValue();
					if (uiElementDepth >= prevNodeDrawcall->depthMax)//not in depth range, no need to split drawcall
					{
						auto drawcall = NewDrawcall(nullptr);
						UIDrawcallList.InsertNode(drawcall, nodePtrToInsert);
					}
					else//inside depth range, must split drawcall
					{
						SplitDrawcall(nodePtrToInsert->GetPrevNode(), uiElementDepth, NewDrawcall);
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
	//SCOPE_CYCLE_COUNTER(STAT_DrawcallBatch);
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
		drawcall->UpdateDepthRange();
		drawcall->needToRebuildMesh = true;
		InUIRenderable->drawcall = nullptr;
		if (drawcall->renderObjectList.Num() == 0)
		{
			if (drawcall->drawcallMeshSection.IsValid())
			{
				drawcall->drawcallMesh->DeleteMeshSection(drawcall->drawcallMeshSection.Pin());
				drawcall->drawcallMeshSection.Reset();
			}
			if (drawcall->materialInstanceDynamic.IsValid())
			{
				AddUIMaterialToPool(drawcall->materialInstanceDynamic.Get());
				drawcall->materialInstanceDynamic.Reset();
			}
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
		if (drawcall->drawcallMeshSection.IsValid())
		{
			drawcall->drawcallMesh->DeleteMeshSection(drawcall->drawcallMeshSection.Pin());
			drawcall->drawcallMeshSection.Reset();
		}
		if (drawcall->postProcessRenderableObject.IsValid())
		{
			if (drawcall->postProcessRenderableObject->IsRenderProxyValid())
			{
				drawcall->postProcessRenderableObject->GetRenderProxy()->RemoveFromLGUIRenderer();
			}
		}
		if (drawcall->directMeshRenderableObject.IsValid())
		{
			drawcall->directMeshRenderableObject->ClearMeshData();
		}
		drawcall.Reset();
		InUIRenderable->drawcall = nullptr;
		shouldCheckDrawcallCombine = true;
	}
	break;
	}

	if (shouldCheckDrawcallCombine)
	{
		//check drawcall to see if we can combine some
		CombineDrawcall();
	}
}
void ULGUICanvas::CombineDrawcall()
{
	EUIDrawcallType prevDrwacallType = EUIDrawcallType::None;
	UMaterialInterface* prevMaterial = nullptr;
	UTexture* prevTexture = nullptr;
	for (auto iter = UIDrawcallList.GetTail(); iter != nullptr; iter = iter->GetPrevNode())//from tail to head; so when add renderObjectList, prev obj line after
	{
		auto drawcallItem = iter->GetValue();
		if (drawcallItem->type == EUIDrawcallType::BatchGeometry && prevDrwacallType == EUIDrawcallType::BatchGeometry)//only batch geometry can be combined
		{
			if (drawcallItem->material == nullptr && prevMaterial == nullptr)//custom material cannot combine
			{
				if (drawcallItem->texture == prevTexture)//same texture, can combine
				{
					//combine next-node drawcall into current
					auto prevNode = iter->GetNextNode();
					auto prevDrawcall = prevNode->GetValue();
					auto currentDrawcall = drawcallItem;
					int additionalSize = prevDrawcall->renderObjectList.Num();
					currentDrawcall->renderObjectList.Reserve(currentDrawcall->renderObjectList.Num() + additionalSize);
					for (int i = 0; i < additionalSize; i++)
					{
						currentDrawcall->renderObjectList.Add(prevDrawcall->renderObjectList[i]);
						prevDrawcall->renderObjectList[i]->drawcall = currentDrawcall;
					}
					currentDrawcall->needToRebuildMesh = true;
					currentDrawcall->UpdateDepthRange();
					if (prevDrawcall->drawcallMeshSection.IsValid())
					{
						prevDrawcall->drawcallMesh->DeleteMeshSection(prevDrawcall->drawcallMeshSection.Pin());
						prevDrawcall->drawcallMeshSection.Reset();
					}
					if (prevDrawcall->materialInstanceDynamic.IsValid())
					{
						AddUIMaterialToPool(prevDrawcall->materialInstanceDynamic.Get());
					}
					UIDrawcallList.RemoveNode(prevNode);
				}
			}
		}
		prevDrwacallType = drawcallItem->type;
		prevTexture = drawcallItem->texture.Get();
		prevMaterial = drawcallItem->material.Get();
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

ULGUIMeshComponent* ULGUICanvas::FindNextValidMeshInDrawcallList(TDoubleLinkedList<TSharedPtr<UUIDrawcall>>::TDoubleLinkedListNode* InNode)
{
	for (auto searchItr = InNode; searchItr != nullptr; searchItr = searchItr->GetNextNode())
	{
		auto searchDrawcallItem = searchItr->GetValue();
		switch (searchDrawcallItem->type)
		{
		case EUIDrawcallType::DirectMesh:
		case EUIDrawcallType::BatchGeometry:
		{
			if (searchDrawcallItem->drawcallMesh.IsValid())
			{
				return searchDrawcallItem->drawcallMesh.Get();
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

void ULGUICanvas::UpdateCanvasDrawcall()
{
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

	for (auto item : manageCanvasArray)
	{
		if (item == this)continue;//skip self
		if (item.IsValid() && item->GetIsUIActive())
		{
			item->UpdateCanvasDrawcall();
		}
	}

	//update drawcall
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


		for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
		{
			auto drawcallItem = iter->GetValue();
			//check drawcall mesh first
			switch (drawcallItem->type)
			{
			case EUIDrawcallType::DirectMesh:
			case EUIDrawcallType::BatchGeometry:
			{
				if (!drawcallItem->drawcallMesh.IsValid())
				{
					if (prevUIMesh == nullptr)
					{
						//if drawcall mesh is not valid, we need to search in next drawcalls and find mesh drawcall object (not post process drawcall)
						if (auto foundMesh = FindNextValidMeshInDrawcallList(iter->GetNextNode()))
						{
							prevUIMesh = foundMesh;
						}
						else//not find valid mesh, then get from existing
						{
							prevUIMesh = this->GetUIMeshFromPool().Get();
						}
					}
					drawcallItem->drawcallMesh = prevUIMesh;
				}
			}
			break;
			}


			switch (drawcallItem->type)
			{
			case EUIDrawcallType::DirectMesh:
			{
				auto MeshSection = drawcallItem->drawcallMeshSection;
				if (!MeshSection.IsValid())
				{
					auto UIMesh = drawcallItem->drawcallMesh;
					MeshSection = UIMesh->GetMeshSection();

					drawcallItem->drawcallMeshSection = MeshSection;
					drawcallItem->directMeshRenderableObject->SetMeshData(UIMesh, MeshSection);
					UIMesh->CreateMeshSection(MeshSection.Pin());
				}
				prevUIMesh = drawcallItem->drawcallMesh.Get();
			}
			break;
			case EUIDrawcallType::BatchGeometry:
			{
				auto UIMesh = drawcallItem->drawcallMesh;
				auto MeshSection = drawcallItem->drawcallMeshSection;
				if (drawcallItem->needToRebuildMesh)
				{
					if (!MeshSection.IsValid())
					{
						MeshSection = UIMesh->GetMeshSection();
						drawcallItem->drawcallMeshSection = MeshSection;
					}
					auto MeshSectionPtr = MeshSection.Pin();
					MeshSectionPtr->vertices.Reset();
					MeshSectionPtr->triangles.Reset();
					drawcallItem->GetCombined(MeshSectionPtr->vertices, MeshSectionPtr->triangles);
					MeshSectionPtr->prevVertexCount = MeshSectionPtr->vertices.Num();
					MeshSectionPtr->prevIndexCount = MeshSectionPtr->triangles.Num();
					UIMesh->CreateMeshSection(MeshSectionPtr);
					drawcallItem->needToRebuildMesh = false;
					drawcallItem->needToUpdateVertex = false;
					drawcallItem->vertexPositionChanged = false;
				}
				else if (drawcallItem->needToUpdateVertex)
				{
					check(MeshSection.IsValid());
					auto MeshSectionPtr = MeshSection.Pin();
					drawcallItem->UpdateData(MeshSectionPtr->vertices, MeshSectionPtr->triangles);
					if (MeshSectionPtr->prevVertexCount == MeshSectionPtr->vertices.Num() && MeshSectionPtr->prevIndexCount == MeshSectionPtr->triangles.Num())
					{
						UIMesh->UpdateMeshSection(MeshSectionPtr, true, GetActualAdditionalShaderChannelFlags());
					}
					else
					{
						check(0);//this should not happen
						//meshSection->prevVertexCount = meshSection->vertices.Num();
						//meshSection->prevIndexCount = meshSection->triangles.Num();
						//UIMesh->CreateMeshSection(meshSection);
					}
					drawcallItem->needToUpdateVertex = false;
					drawcallItem->vertexPositionChanged = false;
				}
				prevUIMesh = drawcallItem->drawcallMesh.Get();
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
										uiPostProcessPrimitive->AddToLGUIWorldSpaceRenderer(this, this->GetSortOrder(), ULGUIEditorManagerObject::GetViewExtension(GetWorld(), true));
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
								uiPostProcessPrimitive->AddToLGUIWorldSpaceRenderer(this, this->GetSortOrder(), ALGUIManagerActor::GetViewExtension(GetWorld(), true));
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
		auto UIMesh = NewObject<ULGUIMeshComponent>(this->GetOwner(), NAME_None, RF_Transient);
		UIMesh->SetOwnerNoSee(this->GetActualOwnerNoSee());
		UIMesh->SetOnlyOwnerSee(this->GetActualOnlyOwnerSee());
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

int32 ULGUICanvas::SortDrawcall(int32 InStartRenderPriority)
{
	ULGUIMeshComponent* prevUIMesh = nullptr;
	int drawcallIndex = 0;
	int meshOrPostProcessCount = 0;
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcallItem = iter->GetValue();
		switch (drawcallItem->type)
		{
		case EUIDrawcallType::BatchGeometry:
		case EUIDrawcallType::DirectMesh:
		{
			if (drawcallItem->drawcallMesh != prevUIMesh)
			{
				meshOrPostProcessCount++;
				if (prevUIMesh != nullptr)
				{
					prevUIMesh->SortMeshSectionRenderPriority();
					drawcallIndex = 0;
				}
				if (this->GetActualRenderMode() == ELGUIRenderMode::WorldSpace)
				{
					drawcallItem->drawcallMesh->SetUITranslucentSortPriority(this->sortOrder);
				}
				else
				{
					drawcallItem->drawcallMesh->SetUITranslucentSortPriority(InStartRenderPriority++);
				}
				prevUIMesh = drawcallItem->drawcallMesh.Get();
			}
			drawcallItem->drawcallMesh->SetMeshSectionRenderPriority(drawcallItem->drawcallMeshSection.Pin(), drawcallIndex++);
		}
		break;
		case EUIDrawcallType::PostProcess:
		{
			meshOrPostProcessCount++;
			drawcallItem->postProcessRenderableObject->GetRenderProxy()->SetUITranslucentSortPriority(InStartRenderPriority++);
		}
		break;
		}
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
void ULGUICanvas::UpdateAndApplyMaterial()
{
	auto tempClipType = GetActualClipType();
	bool needToSetClipParameter = false;
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
						auto containsLGUIParam = IsMaterialContainsLGUIParameter(SrcMaterial);
						if (containsLGUIParam)
						{
							uiMat = (UMaterialInstanceDynamic*)SrcMaterial;
						}
						drawcallItem->drawcallMesh->SetMeshSectionMaterial(drawcallItem->drawcallMeshSection.Pin(), SrcMaterial);
					}
					else
					{
						auto containsLGUIParam = IsMaterialContainsLGUIParameter(SrcMaterial);
						if (containsLGUIParam)
						{
							uiMat = UMaterialInstanceDynamic::Create(SrcMaterial, this);
							uiMat->SetFlags(RF_Transient);
							drawcallItem->drawcallMesh->SetMeshSectionMaterial(drawcallItem->drawcallMeshSection.Pin(), uiMat.Get());
						}
						else
						{
							drawcallItem->drawcallMesh->SetMeshSectionMaterial(drawcallItem->drawcallMeshSection.Pin(), SrcMaterial);
						}
					}
				}
				else
				{
					uiMat = GetUIMaterialFromPool(tempClipType);
					drawcallItem->drawcallMesh->SetMeshSectionMaterial(drawcallItem->drawcallMeshSection.Pin(), uiMat.Get());
				}
				drawcallItem->materialInstanceDynamic = uiMat;
				drawcallItem->materialChanged = false;
				if (uiMat.IsValid())
				{
					uiMat->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, drawcallItem->texture.Get());
				}
				drawcallItem->textureChanged = false;
				needToSetClipParameter = true;
			}
			else if (drawcallItem->textureChanged)
			{
				uiMat->SetTextureParameterValue(LGUI_MainTextureMaterialParameterName, drawcallItem->texture.Get());
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
				needToSetClipParameter = true;
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
				needToSetClipParameter = true;
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
		if (needToSetClipParameter || cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_RectClipParameterChanged)
		{
			SetParameterForRectClip();
		}
	}
	break;
	case ELGUICanvasClipType::Texture:
	{
		if (needToSetClipParameter || cacheForThisUpdate_ClipTypeChanged || cacheForThisUpdate_TextureClipParameterChanged)
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
				uiMat->SetVectorParameterValue(LGUI_RectClipOffsetAndSize_MaterialParameterName, rectClipOffsetAndSize);
				uiMat->SetVectorParameterValue(LGUI_RectClipFeather_MaterialParameterName, rectClipFeather);
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
				uiMat->SetTextureParameterValue(LGUI_TextureClip_MaterialParameterName, clipTexture);
				uiMat->SetVectorParameterValue(LGUI_TextureClipOffsetAndSize_MaterialParameterName, textureClipOffsetAndSize);
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

					clipRectMin.X = parentRectMin.X;
					clipRectMin.Y = parentRectMin.Y;
					clipRectMax.X = parentRectMax.X;
					clipRectMax.Y = parentRectMax.Y;
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
	for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
	{
		auto drawcallItem = iter->GetValue();
		if (drawcallItem->type == EUIDrawcallType::BatchGeometry)
		{
			if (drawcallItem->material == nullptr)
			{
				drawcallItem->materialChanged = true;
			}
		}
	}
	for (int i = 0; i < 3; i++)
	{
		if (IsValid(InMaterials[i]))
		{
			DefaultMaterials[i] = InMaterials[i];
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
		for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
		{
			auto drawcallItem = iter->GetValue();
			if (drawcallItem->type == EUIDrawcallType::BatchGeometry)
			{
				drawcallItem->textureChanged = true;
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
	if (cacheViewProjectionMatrixFrameNumber != GFrameNumber)
	{
		if (!CheckUIItem())
		{
			UE_LOG(LGUI, Error, TEXT("[LGUICanvas::GetViewProjectionMatrix]UIItem not valid!"));
			return cacheViewProjectionMatrix;
		}
		cacheViewProjectionMatrixFrameNumber = GFrameNumber;

		FVector ViewLocation = GetViewLocation();
		FMatrix ViewRotationMatrix = FInverseRotationMatrix(GetViewRotator()) * FMatrix(
			FPlane(0, 0, 1, 0),
			FPlane(1, 0, 0, 0),
			FPlane(0, 1, 0, 0),
			FPlane(0, 0, 0, 1));
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
		for (auto iter = UIDrawcallList.GetHead(); iter != nullptr; iter = iter->GetNextNode())
		{
			auto drawcall = iter->GetValue();
			if (drawcall->type == EUIDrawcallType::BatchGeometry)
			{
				drawcall->needToUpdateVertex = true;
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
						for (auto canvasItem : RootCanvas->manageCanvasArray)
						{
							if (canvasItem.IsValid())
							{
								ViewExtension->SetRenderCanvasBlendDepth(canvasItem.Get(), canvasItem->GetActualBlendDepth());
							}
						}
					}
				}
			}
		}
	}
}
void ULGUICanvas::ApplyOwnerSeeRecursive()
{
	auto tempOwnerNoSee = this->GetActualOwnerNoSee();
	auto tempOnlyOwnerSee = this->GetActualOnlyOwnerSee();

	for (auto item : UsingUIMeshList)
	{
		item->SetOwnerNoSee(tempOwnerNoSee);
		item->SetOnlyOwnerSee(tempOnlyOwnerSee);
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

#ifdef LGUI_DRAWCALLMODE_AUTO
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
#endif

//PRAGMA_ENABLE_OPTIMIZATION