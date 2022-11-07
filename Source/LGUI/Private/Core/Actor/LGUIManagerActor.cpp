// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/Actor/LGUIManagerActor.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIItem.h"
#include "Core/ActorComponent/UIText.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Event/LGUIBaseRaycaster.h"
#include "Engine/World.h"
#include "Interaction/UISelectableComponent.h"
#include "Core/LGUISettings.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "Core/Actor/UIBaseActor.h"
#include "Core/ActorComponent/UIBatchGeometryRenderable.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Engine/Engine.h"
#include "Core/HudRender/LGUIRenderer.h"
#include "Core/ILGUICultureChangedInterface.h"
#include "Core/LGUILifeCycleBehaviour.h"
#include "Layout/ILGUILayoutInterface.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Selection.h"
#include "EditorViewportClient.h"
#include "PrefabSystem/LGUIPrefabHelperActor.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "EngineUtils.h"
#include "Layout/LGUICanvasScaler.h"
#endif

#define LOCTEXT_NAMESPACE "LGUIManagerObject"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

ULGUIEditorManagerObject* ULGUIEditorManagerObject::Instance = nullptr;
ULGUIEditorManagerObject::ULGUIEditorManagerObject()
{

}
void ULGUIEditorManagerObject::BeginDestroy()
{
#if WITH_EDITORONLY_DATA
	if (OnAssetReimportDelegateHandle.IsValid())
	{
		GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.Remove(OnAssetReimportDelegateHandle);
	}
	if (OnActorLabelChangedDelegateHandle.IsValid())
	{
		FCoreDelegates::OnActorLabelChanged.Remove(OnActorLabelChangedDelegateHandle);
	}
	if (OnMapOpenedDelegateHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	}
	if (OnBlueprintCompiledDelegateHandle.IsValid())
	{
		GEditor->OnBlueprintCompiled().Remove(OnBlueprintCompiledDelegateHandle);
	}
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

void ULGUIEditorManagerObject::Tick(float DeltaTime)
{
#if WITH_EDITORONLY_DATA
	if (OneShotFunctionsToExecuteInTick.Num() > 0)
	{
		for (int i = 0; i < OneShotFunctionsToExecuteInTick.Num(); i++)
		{
			auto& Item = OneShotFunctionsToExecuteInTick[i];
			if (Item.Key <= 0)
			{
				Item.Value();
				OneShotFunctionsToExecuteInTick.RemoveAt(i);
				i--;
			}
			else
			{
				Item.Key--;
			}
		}
	}

	//draw frame
	auto Settings = GetDefault<ULGUIEditorSettings>();
	if (Settings->bDrawHelperFrame)
	{
		for (auto& CanvasItem : AllCanvasArray)
		{
			auto& UIItemArray = CanvasItem->GetUIItemArray();
			for (auto& UIItem : UIItemArray)
			{
				if (!IsValid(UIItem))continue;
				if (!IsValid(UIItem->GetWorld()))continue;
				if (
					UIItem->GetWorld()->WorldType != EWorldType::Editor//actually, ULGUIEditorManagerObject only collect editor mode UIItem, so only this Editor condition will trigger.
																	//so only Editor mode will draw frame. the modes below will not work, just leave it as a reference.
					&& UIItem->GetWorld()->WorldType != EWorldType::Game
					&& UIItem->GetWorld()->WorldType != EWorldType::PIE
					&& UIItem->GetWorld()->WorldType != EWorldType::EditorPreview
					)continue;

				ALGUIManagerActor::DrawFrameOnUIItem(UIItem);
			}
		}
	}

	//draw navigation visualizer
	if (Settings->bDrawSelectableNavigationVisualizer)
	{
		for (auto& item : AllSelectableArray)
		{
			if (!item.IsValid())continue;
			if (!IsValid(item->GetWorld()))continue;
			if (!IsValid(item->GetRootUIComponent()))continue;
			if (!item->GetRootUIComponent()->GetIsUIActiveInHierarchy())continue;
			if (
				item->GetWorld()->WorldType != EWorldType::Editor//actually, ULGUIEditorManagerObject only collect editor mode object, so only this Editor condition will trigger.
																//so only Editor mode will draw frame. the modes below will not work, just leave it as a reference.
				&& item->GetWorld()->WorldType != EWorldType::Game
				&& item->GetWorld()->WorldType != EWorldType::PIE
				&& item->GetWorld()->WorldType != EWorldType::EditorPreview
				)continue;

			ALGUIManagerActor::DrawNavigationVisualizerOnUISelectable(item->GetWorld(), item.Get());
		}
	}

	bool canUpdateLayout = true;
	if (!GetDefault<ULGUIEditorSettings>()->AnchorControlPosition)
	{
		canUpdateLayout = false;
	}

	if (canUpdateLayout)
	{
		for (auto& item : AllLayoutArray)
		{
			ILGUILayoutInterface::Execute_OnUpdateLayout(item.GetObject());
		}
	}
	
	int ScreenSpaceOverlayCanvasCount = 0;
	for (auto& item : AllCanvasArray)
	{
		if (item.IsValid())
		{
			if (item->IsRootCanvas())
			{
				if (item->GetWorld() == GWorld)
				{
					if (item->GetActualRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
					{
						ScreenSpaceOverlayCanvasCount++;
					}
				}
			}
		}
	}
	for (auto& item : AllCanvasArray)
	{
		if (item.IsValid() && item->IsRootCanvas())
		{
			item->UpdateRootCanvas();
		}
	}

	if (ScreenSpaceOverlayCanvasCount > 1)
	{
		if (PrevScreenSpaceOverlayCanvasCount != ScreenSpaceOverlayCanvasCount)//only show message when change
		{
			PrevScreenSpaceOverlayCanvasCount = ScreenSpaceOverlayCanvasCount;
			auto errMsg = LOCTEXT("MultipleLGUICanvasRenderScreenSpaceOverlay", "Detect multiply LGUICanvas renderred with ScreenSpaceOverlay mode, this is not allowed! There should be only one ScreenSpace UI in a world!");
			UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
			LGUIUtils::EditorNotification(errMsg, 10.0f);
		}
	}
	else
	{
		PrevScreenSpaceOverlayCanvasCount = 0;
	}

	if (EditorTick.IsBound())
	{
		EditorTick.Broadcast(DeltaTime);
	}

	if (AllCanvasArray.Num() > 0)
	{
		if (bShouldSortLGUIRenderer || bShouldSortWorldSpaceCanvas || bShouldSortRenderTargetSpaceCanvas)
		{
			AllCanvasArray.Sort([](const TWeakObjectPtr<ULGUICanvas>& A, const TWeakObjectPtr<ULGUICanvas>& B)
				{
					auto ASortOrder = A->GetActualSortOrder();
					auto BSortOrder = B->GetActualSortOrder();
					if (ASortOrder == BSortOrder)
					{
						if (A->GetUIItem() != nullptr && B->GetUIItem() != nullptr)
						{
							return A->GetUIItem()->GetFlattenHierarchyIndex() < B->GetUIItem()->GetFlattenHierarchyIndex();
						}
					}
					return ASortOrder < BSortOrder;
				});
		}
		if (bShouldSortLGUIRenderer)
		{
			SortDrawcallOnRenderMode(ELGUIRenderMode::ScreenSpaceOverlay);
			SortDrawcallOnRenderMode(ELGUIRenderMode::WorldSpace_LGUI);
			if (ScreenSpaceOverlayViewExtension.IsValid())
			{
				ScreenSpaceOverlayViewExtension->SortScreenAndWorldSpacePrimitiveRenderPriority();
			}
		}
		if (bShouldSortWorldSpaceCanvas)
		{
			SortDrawcallOnRenderMode(ELGUIRenderMode::WorldSpace);
		}
		if (bShouldSortRenderTargetSpaceCanvas)
		{
			SortDrawcallOnRenderMode(ELGUIRenderMode::RenderTarget);
		}

		bShouldSortLGUIRenderer = false;
		bShouldSortWorldSpaceCanvas = false;
		bShouldSortRenderTargetSpaceCanvas = false;
	}
#endif
#if WITH_EDITOR
	CheckEditorViewportIndexAndKey();

	if (bShouldBroadcastLevelActorListChanged)
	{
		bShouldBroadcastLevelActorListChanged = false;
		if (IsValid(GEditor))
		{
			GEditor->BroadcastLevelActorListChanged();
		}
	}
#endif
}
TStatId ULGUIEditorManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}

#if WITH_EDITOR
TArray<TTuple<int, TFunction<void()>>> ULGUIEditorManagerObject::OneShotFunctionsToExecuteInTick;

void ULGUIEditorManagerObject::AddOneShotTickFunction(TFunction<void()> InFunction, int InDelayFrameCount)
{
	if (ensure(InDelayFrameCount >= 0))
	{
		TTuple<int, TFunction<void()>> Item;
		Item.Key = InDelayFrameCount;
		Item.Value = InFunction;
		OneShotFunctionsToExecuteInTick.Add(Item);
	}
}

ULGUIEditorManagerObject* ULGUIEditorManagerObject::GetInstance(UWorld* InWorld, bool CreateIfNotValid)
{
	if (CreateIfNotValid)
	{
		InitCheck(InWorld);
	}
	return Instance;
}
bool ULGUIEditorManagerObject::InitCheck(UWorld* InWorld)
{
	if (Instance == nullptr)
	{
		if (IsValid(InWorld))
		{
			if (InWorld->IsGameWorld())
			{
				auto msg = LOCTEXT("CreateLGUIEditorManagerObjectInGameMode", "[ULGUIEditorManagerObject::InitCheck]Trying to create a LGUIEditorManagerObject in game mode, this is not allowed!");
				UE_LOG(LGUI, Error, TEXT("%s"), *msg.ToString());
				LGUIUtils::EditorNotification(msg);
				return false;
			}
			
			Instance = NewObject<ULGUIEditorManagerObject>();
			Instance->AddToRoot();
			UE_LOG(LGUI, Log, TEXT("[ULGUIManagerObject::InitCheck]No Instance for LGUIManagerObject, create!"));
			Instance->OnActorLabelChangedDelegateHandle = FCoreDelegates::OnActorLabelChanged.AddUObject(Instance, &ULGUIEditorManagerObject::OnActorLabelChanged);
			//reimport asset
			Instance->OnAssetReimportDelegateHandle = GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddUObject(Instance, &ULGUIEditorManagerObject::OnAssetReimport);
			//open map
			Instance->OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddUObject(Instance, &ULGUIEditorManagerObject::OnMapOpened);
			//blueprint recompile
			Instance->OnBlueprintCompiledDelegateHandle = GEditor->OnBlueprintCompiled().AddUObject(Instance, &ULGUIEditorManagerObject::RefreshOnBlueprintCompiled);
		}
		else
		{
			return false;
		}
	}
	return true;
}

#include "Event/LGUIEventDelegate.h"
#include "LGUIComponentReference.h"
#include "PrefabAnimation/LGUIPrefabSequenceObjectReference.h"
void ULGUIEditorManagerObject::RefreshOnBlueprintCompiled()
{
	RefreshAllUI();
	FLGUIEventDelegateData::RefreshAllOnBlueprintRecompile();
	FLGUIComponentReference::RefreshAllOnBlueprintRecompile();
	FLGUIPrefabSequenceObjectReference::RefreshAllOnBlueprintRecompile();
}

void ULGUIEditorManagerObject::SortDrawcallOnRenderMode(ELGUIRenderMode InRenderMode)
{
	int32 RenderPriority = 0;
	static TSet<ULGUICanvas*> ProcessedCanvasArray;
	ProcessedCanvasArray.Reset();
	for (int i = 0; i < AllCanvasArray.Num(); i++)
	{
		auto canvasItem = this->AllCanvasArray[i];
		if (canvasItem.IsValid() && canvasItem->GetIsUIActive())
		{
			if (canvasItem->GetActualRenderMode() == InRenderMode)
			{
				if (!ProcessedCanvasArray.Contains(canvasItem.Get()))
				{
					canvasItem->SortDrawcall(RenderPriority, ProcessedCanvasArray);
				}
			}
		}
	}
}
void ULGUIEditorManagerObject::MarkSortLGUIRenderer()
{
	bShouldSortLGUIRenderer = true;
}
void ULGUIEditorManagerObject::MarkSortWorldSpaceCanvas()
{
	bShouldSortWorldSpaceCanvas = true;
}
void ULGUIEditorManagerObject::MarkSortRenderTargetSpaceCanvas()
{
	bShouldSortRenderTargetSpaceCanvas = true;
}

TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ULGUIEditorManagerObject::GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist)
{
	if (Instance != nullptr)
	{
		if (!Instance->ScreenSpaceOverlayViewExtension.IsValid())
		{
			if (InCreateIfNotExist)
			{
				Instance->ScreenSpaceOverlayViewExtension = FSceneViewExtensions::NewExtension<FLGUIHudRenderer>(InWorld);
			}
		}
		else
		{
			if (!Instance->ScreenSpaceOverlayViewExtension->GetWorld().IsValid())
			{
				Instance->ScreenSpaceOverlayViewExtension.Reset();
				if (InCreateIfNotExist)
				{
					Instance->ScreenSpaceOverlayViewExtension = FSceneViewExtensions::NewExtension<FLGUIHudRenderer>(InWorld);
				}
			}
		}
		return Instance->ScreenSpaceOverlayViewExtension;
	}
	return nullptr;
}

void ULGUIEditorManagerObject::OnAssetReimport(UObject* asset)
{
	if (IsValid(asset))
	{
		auto textureAsset = Cast<UTexture2D>(asset);
		if (IsValid(textureAsset))
		{
			bool needToRebuildUI = false;
			//find sprite data that reference this texture
			for (TObjectIterator<ULGUISpriteData> Itr; Itr; ++Itr)
			{
				ULGUISpriteData* spriteData = *Itr;
				if (IsValid(spriteData))
				{
					if (spriteData->GetSpriteTexture() == textureAsset)
					{
						spriteData->ReloadTexture();
						spriteData->MarkPackageDirty();
						needToRebuildUI = true;
					}
				}
			}
			//Refresh ui
			if (needToRebuildUI)
			{
				RefreshAllUI();
			}
		}
	}
}

void ULGUIEditorManagerObject::OnMapOpened(const FString& FileName, bool AsTemplate)
{

}

UWorld* ULGUIEditorManagerObject::PreviewWorldForPrefabPackage = nullptr;
UWorld* ULGUIEditorManagerObject::GetPreviewWorldForPrefabPackage()
{
	if (PreviewWorldForPrefabPackage == nullptr)
	{
		FName UniqueWorldName = MakeUniqueObjectName(GetTransientPackage(), UWorld::StaticClass(), FName("LGUI_PreviewWorldForPrefabPackage"));
		PreviewWorldForPrefabPackage = NewObject<UWorld>(GetTransientPackage(), UniqueWorldName);
		PreviewWorldForPrefabPackage->AddToRoot();
		PreviewWorldForPrefabPackage->WorldType = EWorldType::EditorPreview;

		FWorldContext& WorldContext = GEngine->CreateNewWorldContext(PreviewWorldForPrefabPackage->WorldType);
		WorldContext.SetCurrentWorld(PreviewWorldForPrefabPackage);

		PreviewWorldForPrefabPackage->InitializeNewWorld(UWorld::InitializationValues()
			.AllowAudioPlayback(false)
			.CreatePhysicsScene(false)
			.RequiresHitProxies(false)
			.CreateNavigation(false)
			.CreateAISystem(false)
			.ShouldSimulatePhysics(false)
			.SetTransactional(false));
	}
	return PreviewWorldForPrefabPackage;
}

void ULGUIEditorManagerObject::OnActorLabelChanged(AActor* actor)
{
	if (actor->GetWorld()->IsGameWorld())return;
	if (auto rootComp = actor->GetRootComponent())
	{
		if (auto rootUIComp = Cast<UUIItem>(rootComp))
		{
			auto actorLabel = actor->GetActorLabel();
			if (actorLabel.StartsWith("//"))
			{
				actorLabel = actorLabel.Right(actorLabel.Len() - 2);
			}
			rootUIComp->SetDisplayName(actorLabel);

			LGUIUtils::NotifyPropertyChanged(rootUIComp, FName(TEXT("displayName")));
		}
	}
}

void ULGUIEditorManagerObject::RefreshAllUI()
{
	struct Local
	{
		static void UpdateComponentToWorldRecursive(UUIItem* UIItem)
		{
			if (!IsValid(UIItem))return;
			UIItem->CalculateTransformFromAnchor();
			UIItem->UpdateComponentToWorld();
			auto& Children = UIItem->GetAttachUIChildren();
			for (auto& Child : Children)
			{
				UpdateComponentToWorldRecursive(Child);
			}
		}
	};

	if (Instance != nullptr)
	{
		for (auto& Layout : Instance->AllLayoutArray)
		{
			if (Layout.GetObject() != nullptr)
			{
				ILGUILayoutInterface::Execute_MarkRebuildLayout(Layout.GetObject());
			}
		}
		for (auto& RootUIItem : Instance->AllRootUIItemArray)
		{
			if (RootUIItem.IsValid())
			{
				RootUIItem->MarkAllDirtyRecursive();
				RootUIItem->ForceRefreshRenderCanvasRecursive();
				Local::UpdateComponentToWorldRecursive(RootUIItem.Get());
				RootUIItem->EditorForceUpdate();
			}
		}
		for (auto& CanvasItem : Instance->AllCanvasArray)
		{
			CanvasItem->EnsureDrawcallObjectReference();
		}
	}
}

void ULGUIEditorManagerObject::MarkBroadcastLevelActorListChanged()
{
	if (Instance != nullptr)
	{
		Instance->bShouldBroadcastLevelActorListChanged = true;
	}
}

bool ULGUIEditorManagerObject::IsSelected(AActor* InObject)
{
	if (!IsValid(GEditor))return false;
	for (FSelectionIterator itr(GEditor->GetSelectedActorIterator()); itr; ++itr)
	{
		if (*itr != nullptr)
		{
			auto itrActor = Cast<AActor>(*itr);
			if (itrActor == InObject)
			{
				return true;
			}
		}
	}
	return false;
}

bool ULGUIEditorManagerObject::AnySelectedIsChildOf(AActor* InObject)
{
	if (!IsValid(GEditor))return false;
	for (FSelectionIterator itr(GEditor->GetSelectedActorIterator()); itr; ++itr)
	{
		auto itrActor = Cast<AActor>(*itr);
		if (IsValid(itrActor) && itrActor->IsAttachedTo(InObject))
		{
			return true;
		}
	}
	return false;
}

const TArray<TWeakObjectPtr<UUIItem>>& ULGUIEditorManagerObject::GetAllRootUIItemArray()
{
	return Instance->AllRootUIItemArray;
}

void ULGUIEditorManagerObject::RemoveCanvas(ULGUICanvas* InCanvas)
{
	if (Instance != nullptr)
	{
#if !UE_BUILD_SHIPPING
		check(Instance->AllCanvasArray.Contains(InCanvas));
#endif
		Instance->AllCanvasArray.RemoveSingle(InCanvas);
	}
}
void ULGUIEditorManagerObject::AddCanvas(ULGUICanvas* InCanvas)
{
	if (GetInstance(InCanvas->GetWorld(), true))
	{
#if !UE_BUILD_SHIPPING
		check(!Instance->AllCanvasArray.Contains(InCanvas));
#endif
		Instance->AllCanvasArray.AddUnique(InCanvas);
	}
}

void ULGUIEditorManagerObject::AddRootUIItem(UUIItem* InItem)
{
	if (GetInstance(InItem->GetWorld(), true))
	{
#if !UE_BUILD_SHIPPING
		check(!Instance->AllRootUIItemArray.Contains(InItem));
#endif
		Instance->AllRootUIItemArray.AddUnique(InItem);
	}
}
void ULGUIEditorManagerObject::RemoveRootUIItem(UUIItem* InItem)
{
	if (Instance != nullptr)
	{
#if !UE_BUILD_SHIPPING
		check(Instance->AllRootUIItemArray.Contains(InItem));
#endif
		Instance->AllRootUIItemArray.RemoveSingle(InItem);
	}
}

void ULGUIEditorManagerObject::AddSelectable(UUISelectableComponent* InSelectable)
{
	if (GetInstance(InSelectable->GetWorld(), true))
	{
		auto& AllSelectableArray = Instance->AllSelectableArray;
		if (AllSelectableArray.Contains(InSelectable))return;
		AllSelectableArray.Add(InSelectable);
	}
}
void ULGUIEditorManagerObject::RemoveSelectable(UUISelectableComponent* InSelectable)
{
	if (GetInstance(InSelectable->GetWorld()))
	{
		int32 index;
		if (Instance->AllSelectableArray.Find(InSelectable, index))
		{
			Instance->AllSelectableArray.RemoveAt(index);
		}
	}
}

void ULGUIEditorManagerObject::RegisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem)
{
	if (InitCheck(InItem.GetObject()->GetWorld()))
	{
#if !UE_BUILD_SHIPPING
		check(!Instance->AllLayoutArray.Contains(InItem));
#endif
		Instance->AllLayoutArray.AddUnique(InItem);
	}
}
void ULGUIEditorManagerObject::UnregisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem)
{
	if (Instance != nullptr)
	{
#if !UE_BUILD_SHIPPING
		check(Instance->AllLayoutArray.Contains(InItem));
#endif
		Instance->AllLayoutArray.RemoveSingle(InItem);
	}
}


void ULGUIEditorManagerObject::CheckEditorViewportIndexAndKey()
{
	if (!IsValid(GEditor))return;
	auto& viewportClients = GEditor->GetAllViewportClients();
	if (PrevEditorViewportCount != viewportClients.Num())
	{
		PrevEditorViewportCount = viewportClients.Num();
		EditorViewportIndexToKeyMap.Reset();
		for (FEditorViewportClient* viewportClient : viewportClients)
		{
			auto viewKey = viewportClient->ViewState.GetReference()->GetViewKey();
			EditorViewportIndexToKeyMap.Add(viewportClient->ViewIndex, viewKey);
		}

		if (EditorViewportIndexAndKeyChange.IsBound())
		{
			EditorViewportIndexAndKeyChange.Broadcast();
		}
	}

	if (auto viewport = GEditor->GetActiveViewport())
	{
		if (auto viewportClient = viewport->GetClient())
		{
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				auto editorViewportClient = (FEditorViewportClient*)viewportClient;
				CurrentActiveViewportIndex = editorViewportClient->ViewIndex;
				CurrentActiveViewportKey = ULGUIEditorManagerObject::Instance->GetViewportKeyFromIndex(editorViewportClient->ViewIndex);
			}
		}
	}
}
uint32 ULGUIEditorManagerObject::GetViewportKeyFromIndex(int32 InViewportIndex)
{
	if (auto key = EditorViewportIndexToKeyMap.Find(InViewportIndex))
	{
		return *key;
	}
	return 0;
}

void ULGUIEditorManagerObject::BeginPrefabSystemProcessingActor(UWorld* InWorld, AActor* InRootActor)
{
	if (InitCheck(InWorld))
	{
		//nothing for edit mode
	}
}
void ULGUIEditorManagerObject::EndPrefabSystemProcessingActor(UWorld* InWorld, AActor* InRootActor)
{
	if (Instance != nullptr)
	{
		auto& LateFunctions = Instance->LateFunctionsAfterPrefabSerialization;
		for (auto& Function : LateFunctions)
		{
			Function();
		}
	}
}
void ULGUIEditorManagerObject::AddActorForPrefabSystem(AActor* InActor)
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.Add(InActor);
	}
}
void ULGUIEditorManagerObject::RemoveActorForPrefabSystem(AActor* InActor)
{
	if (Instance != nullptr)
	{
		Instance->AllActors_PrefabSystemProcessing.Remove(InActor);
	}
}
bool ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(AActor* InActor)
{
	if (Instance != nullptr)
	{
		return Instance->AllActors_PrefabSystemProcessing.Contains(InActor);
	}
	return false;
}

void ULGUIEditorManagerObject::AddFunctionForPrefabSystemExecutionBeforeAwake(AActor* InPrefabActor, const TFunction<void()>& InFunction)
{
	if (Instance != nullptr)
	{
		Instance->LateFunctionsAfterPrefabSerialization.Add(InFunction);
	}
}

bool ULGUIEditorManagerObject::RaycastHitUI(UWorld* InWorld, const TArray<UUIItem*>& InUIItems, const FVector& LineStart, const FVector& LineEnd
	, UUIBaseRenderable*& ResultSelectTarget, int& InOutTargetIndexInHitArray
)
{
	TArray<FHitResult> HitResultArray;
	for (auto& uiItem : InUIItems)
	{
		if (uiItem->GetWorld() == InWorld)
		{
			if (auto uiRenderable = Cast<UUIBaseRenderable>(uiItem))
			{
				if (uiRenderable->GetIsUIActiveInHierarchy() && uiRenderable->GetRenderCanvas() != nullptr)
				{
					FHitResult hitInfo;
					auto OriginRaycastType = uiRenderable->GetRaycastType();
					auto OriginRaycastTarget = uiRenderable->IsRaycastTarget();
					uiRenderable->SetRaycastType(EUIRenderableRaycastType::Geometry);//in editor selection, make the ray hit actural triangle
					uiRenderable->SetRaycastTarget(true);
					if (uiRenderable->LineTraceUI(hitInfo, LineStart, LineEnd))
					{
						if (uiRenderable->GetRenderCanvas()->CalculatePointVisibilityOnClip(hitInfo.Location))
						{
							HitResultArray.Add(hitInfo);
						}
					}
					uiRenderable->SetRaycastType(OriginRaycastType);
					uiRenderable->SetRaycastTarget(OriginRaycastTarget);
				}
			}
		}
	}
	if (HitResultArray.Num() > 0)//hit something
	{
		HitResultArray.Sort([](const FHitResult& A, const FHitResult& B)
			{
				auto AUIRenderable = (UUIBaseRenderable*)(A.Component.Get());
				auto BUIRenderable = (UUIBaseRenderable*)(B.Component.Get());
				if (AUIRenderable->GetRenderCanvas()->GetActualSortOrder() == BUIRenderable->GetRenderCanvas()->GetActualSortOrder())//if Canvas's sort order is equal then sort on item's depth
				{
					return AUIRenderable->GetFlattenHierarchyIndex() > BUIRenderable->GetFlattenHierarchyIndex();
				}
				else//if Canvas's depth not equal then sort on Canvas's SortOrder
				{
					return AUIRenderable->GetRenderCanvas()->GetActualSortOrder() > BUIRenderable->GetRenderCanvas()->GetActualSortOrder();
				}
			});
		InOutTargetIndexInHitArray++;
		if (InOutTargetIndexInHitArray >= HitResultArray.Num() || InOutTargetIndexInHitArray < 0)
		{
			InOutTargetIndexInHitArray = 0;
		}
		auto uiRenderableComp = (UUIBaseRenderable*)(HitResultArray[InOutTargetIndexInHitArray].Component.Get());//target need to select
		ResultSelectTarget = uiRenderableComp;
		return true;
	}
	return false;
}

#endif



#if WITH_EDITOR
void ALGUIManagerActor::DrawFrameOnUIItem(UUIItem* item, bool IsScreenSpace)
{
	auto RectExtends = FVector(0.1f, item->GetWidth(), item->GetHeight()) * 0.5f;
	bool bCanDrawRect = false;
	auto RectDrawColor = FColor(128, 128, 128, 128);//gray means normal object
	if (ULGUIEditorManagerObject::IsSelected(item->GetOwner()))//select self
	{
		RectDrawColor = FColor(0, 255, 0, 255);//green means selected object
		RectExtends.X = 1;
		bCanDrawRect = true;

		if (auto UIRenderable = Cast<UUIBaseRenderable>(item))
		{
			FVector Min, Max;
			UIRenderable->GetGeometryBounds3DInLocalSpace(Min, Max);
			auto WorldTransform = item->GetComponentTransform();
			FVector Center = (Max + Min) * 0.5f;
			auto WorldLocation = WorldTransform.TransformPosition(Center);

			auto GeometryBoundsDrawColor = FColor(255, 255, 0, 255);//yellow for geometry bounds
			auto GeometryBoundsExtends = (Max - Min) * 0.5f;
			if (IsScreenSpace)
			{
				ALGUIManagerActor::DrawDebugBoxOnScreenSpace(item->GetWorld(), WorldLocation, GeometryBoundsExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), GeometryBoundsDrawColor);
			}
			else
			{
				DrawDebugBox(item->GetWorld(), WorldLocation, GeometryBoundsExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), GeometryBoundsDrawColor);
			}
		}
	}
	else
	{
		//parent selected
		if (IsValid(item->GetParentUIItem()))
		{
			if (ULGUIEditorManagerObject::IsSelected(item->GetParentUIItem()->GetOwner()))
			{
				bCanDrawRect = true;
			}
		}
		//child selected
		auto& childrenCompArray = item->GetAttachUIChildren();
		for (auto& uiComp : childrenCompArray)
		{
			if (IsValid(uiComp) && IsValid(uiComp->GetOwner()) && ULGUIEditorManagerObject::IsSelected(uiComp->GetOwner()))
			{
				bCanDrawRect = true;
				break;
			}
		}
		//other object of same hierarchy is selected
		if (IsValid(item->GetParentUIItem()))
		{
			const auto& sameLevelCompArray = item->GetParentUIItem()->GetAttachUIChildren();
			for (auto& uiComp : sameLevelCompArray)
			{
				if (IsValid(uiComp) && IsValid(uiComp->GetOwner()) && ULGUIEditorManagerObject::IsSelected(uiComp->GetOwner()))
				{
					bCanDrawRect = true;
					break;
				}
			}
		}
	}
	//canvas scaler
	if (!bCanDrawRect)
	{
		if (item->IsCanvasUIItem())
		{
			if (auto canvasScaler = item->GetOwner()->FindComponentByClass<ULGUICanvasScaler>())
			{
				if (ULGUIEditorManagerObject::AnySelectedIsChildOf(item->GetOwner()))
				{
					bCanDrawRect = true;
					RectDrawColor = FColor(255, 227, 124);
				}
			}
		}
	}

	if (bCanDrawRect)
	{
		auto WorldTransform = item->GetComponentTransform();
		FVector RelativeOffset(0, 0, 0);
		RelativeOffset.Y = (0.5f - item->GetPivot().X) * item->GetWidth();
		RelativeOffset.Z = (0.5f - item->GetPivot().Y) * item->GetHeight();
		auto WorldLocation = WorldTransform.TransformPosition(RelativeOffset);

		if (IsScreenSpace)
		{
			ALGUIManagerActor::DrawDebugBoxOnScreenSpace(item->GetWorld(), WorldLocation, RectExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), RectDrawColor);
		}
		else
		{
			DrawDebugBox(item->GetWorld(), WorldLocation, RectExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), RectDrawColor);
		}
	}
}

void ALGUIManagerActor::DrawNavigationArrow(UWorld* InWorld, const TArray<FVector>& InControlPoints, const FVector& InArrowPointA, const FVector& InArrowPointB, FColor const& InColor, bool IsScreenSpace)
{
	if (InControlPoints.Num() != 4)return;
	TArray<FVector> ResultPoints;
	const int segment = FMath::Min(40, FMath::CeilToInt(FVector::Distance(InControlPoints[0], InControlPoints[3]) * 0.5f));

	auto CalculateCubicBezierPoint = [](float t, FVector p0, FVector p1, FVector p2, FVector p3)
	{
		float u = 1 - t;
		float tt = t * t;
		float uu = u * u;
		float uuu = uu * u;
		float ttt = tt * t;

		FVector p = uuu * p0;
		p += 3 * uu * t * p1;
		p += 3 * u * tt * p2;
		p += ttt * p3;

		return p;
	};

	ResultPoints.Add(InControlPoints[0]);
	for (int i = 1; i <= segment; i++)
	{
		float t = i / (float)segment;
		auto pixel = CalculateCubicBezierPoint(t, InControlPoints[0], InControlPoints[1], InControlPoints[2], InControlPoints[3]);
		ResultPoints.Add(pixel);
	}

	if (IsScreenSpace)
	{
		auto ViewExtension = ALGUIManagerActor::GetViewExtension(InWorld, false);
		if (ViewExtension.IsValid())
		{
			TArray<FLGUIHelperLineVertex> Lines;
			//lines
			FVector prevPoint = ResultPoints[0];
			for (int i = 1; i < ResultPoints.Num(); i++)
			{
				new(Lines) FLGUIHelperLineVertex(prevPoint, InColor);
				new(Lines) FLGUIHelperLineVertex(ResultPoints[i], InColor);
				prevPoint = ResultPoints[i];
			}
			//arrow
			new(Lines) FLGUIHelperLineVertex(InControlPoints[3], InColor);
			new(Lines) FLGUIHelperLineVertex(InArrowPointA, InColor);
			new(Lines) FLGUIHelperLineVertex(InControlPoints[3], InColor);
			new(Lines) FLGUIHelperLineVertex(InArrowPointB, InColor);

			ViewExtension->AddLineRender(FHelperLineRenderParameter(Lines));
		}
	}
	else
	{
		//lines
		FVector prevPoint = ResultPoints[0];
		for (int i = 1; i < ResultPoints.Num(); i++)
		{
			DrawDebugLine(InWorld, prevPoint, ResultPoints[i], InColor);
			prevPoint = ResultPoints[i];
		}
		//arrow
		DrawDebugLine(InWorld, InControlPoints[3], InArrowPointA, InColor);
		DrawDebugLine(InWorld, InControlPoints[3], InArrowPointB, InColor);
	}
}

void ALGUIManagerActor::DrawNavigationVisualizerOnUISelectable(UWorld* InWorld, UUISelectableComponent* InSelectable, bool IsScreenSpace)
{
	auto SourceUIItem = InSelectable->GetRootUIComponent();
	if (!IsValid(SourceUIItem))return;
	const FColor Drawcall = ULGUIEditorManagerObject::IsSelected(SourceUIItem->GetOwner()) ? FColor(255, 255, 0, 255) : FColor(140, 140, 0, 255);
	const float Offset = 2;
	const float ArrowSize = 2;

	if (auto ToLeftComp = InSelectable->FindSelectableOnLeft())
	{
		if (ToLeftComp != InSelectable)
		{
			auto SourceLeftPoint = FVector(0, SourceUIItem->GetLocalSpaceLeft(), 0.5f * (SourceUIItem->GetLocalSpaceTop() + SourceUIItem->GetLocalSpaceBottom()) + Offset);
			SourceLeftPoint = SourceUIItem->GetComponentTransform().TransformPosition(SourceLeftPoint);
			auto DestUIItem = ToLeftComp->GetRootUIComponent();
			auto LocalDestRightPoint = FVector(0, DestUIItem->GetLocalSpaceRight(), 0.5f * (DestUIItem->GetLocalSpaceTop() + DestUIItem->GetLocalSpaceBottom()) + Offset);
			auto DestRightPoint = DestUIItem->GetComponentTransform().TransformPosition(LocalDestRightPoint);
			float Distance = FVector::Distance(SourceLeftPoint, DestRightPoint);
			Distance *= 0.2f;
			auto ArrowPointA = DestUIItem->GetComponentTransform().TransformPosition(LocalDestRightPoint + FVector(0, ArrowSize, ArrowSize));
			auto ArrowPointB = DestUIItem->GetComponentTransform().TransformPosition(LocalDestRightPoint + FVector(0, ArrowSize, -ArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceLeftPoint,
					SourceLeftPoint - SourceUIItem->GetRightVector() * Distance,
					DestRightPoint + DestUIItem->GetRightVector() * Distance,
					DestRightPoint,
				}
				, ArrowPointA, ArrowPointB
				, Drawcall, IsScreenSpace);
		}
	}
	if (auto ToRightComp = InSelectable->FindSelectableOnRight())
	{
		if (ToRightComp != InSelectable)
		{
			auto SourceRightPoint = FVector(0, SourceUIItem->GetLocalSpaceRight(), 0.5f * (SourceUIItem->GetLocalSpaceTop() + SourceUIItem->GetLocalSpaceBottom()) - Offset);
			SourceRightPoint = SourceUIItem->GetComponentTransform().TransformPosition(SourceRightPoint);
			auto DestUIItem = ToRightComp->GetRootUIComponent();
			auto LocalDestLeftPoint = FVector(0, DestUIItem->GetLocalSpaceLeft(), 0.5f * (DestUIItem->GetLocalSpaceTop() + DestUIItem->GetLocalSpaceBottom()) - Offset);
			auto DestLeftPoint = DestUIItem->GetComponentTransform().TransformPosition(LocalDestLeftPoint);
			float Distance = FVector::Distance(SourceRightPoint, DestLeftPoint);
			Distance *= 0.2f;
			auto ArrowPointA = DestUIItem->GetComponentTransform().TransformPosition(LocalDestLeftPoint + FVector(0, -ArrowSize, ArrowSize));
			auto ArrowPointB = DestUIItem->GetComponentTransform().TransformPosition(LocalDestLeftPoint + FVector(0, -ArrowSize, -ArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceRightPoint,
					SourceRightPoint + SourceUIItem->GetRightVector() * Distance,
					DestLeftPoint - DestUIItem->GetRightVector() * Distance,
					DestLeftPoint,
				}
				, ArrowPointA, ArrowPointB
				, Drawcall, IsScreenSpace);
		}
	}
	if (auto ToDownComp = InSelectable->FindSelectableOnDown())
	{
		if (ToDownComp != InSelectable)
		{
			auto SourceDownPoint = FVector(0, 0.5f * (SourceUIItem->GetLocalSpaceLeft() + SourceUIItem->GetLocalSpaceRight()) - Offset, SourceUIItem->GetLocalSpaceBottom());
			SourceDownPoint = SourceUIItem->GetComponentTransform().TransformPosition(SourceDownPoint);
			auto DestUIItem = ToDownComp->GetRootUIComponent();
			auto LocalDestUpPoint = FVector(0, 0.5f * (DestUIItem->GetLocalSpaceLeft() + DestUIItem->GetLocalSpaceRight()) - Offset, DestUIItem->GetLocalSpaceTop());
			auto DestUpPoint = DestUIItem->GetComponentTransform().TransformPosition(LocalDestUpPoint);
			float Distance = FVector::Distance(SourceDownPoint, DestUpPoint);
			Distance *= 0.2f;
			auto ArrowPointA = DestUIItem->GetComponentTransform().TransformPosition(LocalDestUpPoint + FVector(0, ArrowSize, ArrowSize));
			auto ArrowPointB = DestUIItem->GetComponentTransform().TransformPosition(LocalDestUpPoint + FVector(0, -ArrowSize, ArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceDownPoint,
					SourceDownPoint - SourceUIItem->GetUpVector() * Distance,
					DestUpPoint + DestUIItem->GetUpVector() * Distance,
					DestUpPoint,
				}
				, ArrowPointA, ArrowPointB
				, Drawcall, IsScreenSpace);
		}
	}
	if (auto ToUpComp = InSelectable->FindSelectableOnUp())
	{
		if (ToUpComp != InSelectable)
		{
			auto SourceUpPoint = FVector(0, 0.5f * (SourceUIItem->GetLocalSpaceLeft() + SourceUIItem->GetLocalSpaceRight()) + Offset, SourceUIItem->GetLocalSpaceTop());
			SourceUpPoint = SourceUIItem->GetComponentTransform().TransformPosition(SourceUpPoint);
			auto DestUIItem = ToUpComp->GetRootUIComponent();
			auto LocalDestDownPoint = FVector(0, 0.5f * (DestUIItem->GetLocalSpaceLeft() + DestUIItem->GetLocalSpaceRight()) + Offset, DestUIItem->GetLocalSpaceBottom());
			auto DestDownPoint = DestUIItem->GetComponentTransform().TransformPosition(LocalDestDownPoint);
			float Distance = FVector::Distance(SourceUpPoint, DestDownPoint);
			Distance *= 0.2f;
			auto ArrowPointA = DestUIItem->GetComponentTransform().TransformPosition(LocalDestDownPoint + FVector(0, ArrowSize, -ArrowSize));
			auto ArrowPointB = DestUIItem->GetComponentTransform().TransformPosition(LocalDestDownPoint + FVector(0, -ArrowSize, -ArrowSize));
			DrawNavigationArrow(InWorld
				, {
					SourceUpPoint,
					SourceUpPoint + SourceUIItem->GetUpVector() * Distance,
					DestDownPoint - DestUIItem->GetUpVector() * Distance,
					DestDownPoint,
				}
				, ArrowPointA, ArrowPointB
				, Drawcall, IsScreenSpace);
		}
	}
}

void ALGUIManagerActor::DrawDebugBoxOnScreenSpace(UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color)
{
	auto ViewExtension = ALGUIManagerActor::GetViewExtension(InWorld, false);
	if (ViewExtension.IsValid())
	{
		TArray<FLGUIHelperLineVertex> Lines;

		FTransform const Transform(Rotation);
		FVector Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		FVector End = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		new(Lines)FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines)FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(Center + Start, Color);
		new(Lines) FLGUIHelperLineVertex(Center + End, Color);

		ViewExtension->AddLineRender(FHelperLineRenderParameter(Lines));
	}
}
#endif

TMap<UWorld*, ALGUIManagerActor*> ALGUIManagerActor::WorldToInstanceMap ;
ALGUIManagerActor::ALGUIManagerActor()
{
	PrimaryActorTick.bCanEverTick = true;
	PrimaryActorTick.TickGroup = ETickingGroup::TG_DuringPhysics;
	SpawnCollisionHandlingMethod = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
#if WITH_EDITOR
	FEditorDelegates::BeginPIE.AddLambda([=](const bool isSimulating) {
		for (auto& keyValue : WorldToInstanceMap)
		{
			if (IsValid(keyValue.Value))
			{
				LGUIUtils::DestroyActorWithHierarchy(keyValue.Value);//delete any instance before begin play
			}
		}
	});
#endif
}
ALGUIManagerActor* ALGUIManagerActor::GetLGUIManagerActorInstance(UObject* WorldContextObject)
{
	auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull);
	if (auto result = WorldToInstanceMap.Find(world))
	{
		return *result;
	}
	else
	{
		return nullptr;
	}
}
void ALGUIManagerActor::BeginPlay()
{
	Super::BeginPlay();
#if WITH_EDITORONLY_DATA
	bIsPlaying = true;
#endif
	//localization
	onCultureChangedDelegateHandle = FInternationalization::Get().OnCultureChanged().AddUObject(this, &ALGUIManagerActor::OnCultureChanged);
}
void ALGUIManagerActor::BeginDestroy()
{
	if (WorldToInstanceMap.Num() > 0 && bExistInInstanceMap)
	{
		bool removed = false;
		if (auto world = this->GetWorld())
		{
			WorldToInstanceMap.Remove(world);
			removed = true;
		}
		else
		{
			world = nullptr;
			for (auto& keyValue : WorldToInstanceMap)
			{
				if (keyValue.Value == this)
				{
					world = keyValue.Key;
				}
			}
			if (world != nullptr)
			{
				WorldToInstanceMap.Remove(world);
				removed = true;
			}
		}
		if (removed)
		{
			bExistInInstanceMap = false;
		}
		else
		{
			UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::BeginDestroy]Cannot remove instance!"));
		}
	}
	if (WorldToInstanceMap.Num() <= 0)
	{
		UE_LOG(LGUI, Log, TEXT("[ALGUIManagerActor::BeginDestroy]All instance removed."));
	}
	if (ScreenSpaceOverlayViewExtension.IsValid())
	{
		ScreenSpaceOverlayViewExtension.Reset();
	}
	Super::BeginDestroy();
#if WITH_EDITORONLY_DATA
	bIsPlaying = false;
#endif
	if (onCultureChangedDelegateHandle.IsValid())
	{
		FInternationalization::Get().OnCultureChanged().Remove(onCultureChangedDelegateHandle);
	}
}

void ALGUIManagerActor::OnCultureChanged()
{
	bShouldUpdateOnCultureChanged = true;
}

ALGUIManagerActor* ALGUIManagerActor::GetInstance(UWorld* InWorld, bool CreateIfNotValid)
{
	if (IsValid(InWorld))
	{
		if (auto instance = WorldToInstanceMap.Find(InWorld))
		{
			return *instance;
		}
		else
		{
			if (CreateIfNotValid)
			{
#if WITH_EDITOR
				if (!InWorld->IsGameWorld())
				{
					auto msg = LOCTEXT("TryToCreateLGUIManagerActorInEditMode", "[ALGUIManagerActor::GetInstance]Trying to create a LGUIManagerActor in edit mode, this is not allowed!");
					UE_LOG(LGUI, Error, TEXT("%s"), *msg.ToString());
					LGUIUtils::EditorNotification(msg);
					return nullptr;
				}
#endif
				FActorSpawnParameters param = FActorSpawnParameters();
				param.ObjectFlags = RF_Transient;
				auto newInstance = InWorld->SpawnActor<ALGUIManagerActor>(param);
				WorldToInstanceMap.Add(InWorld, newInstance);
				UE_LOG(LGUI, Log, TEXT("[ALGUIManagerActor::GetInstance]No Instance for LGUIManagerActor, create!"));
				newInstance->bExistInInstanceMap = true;
				return newInstance;
			}
			else
			{
				return nullptr;
			}
		}
	}
	else
	{
		return nullptr;
	}
}

DECLARE_CYCLE_STAT(TEXT("LGUILifeCycleBehaviour Update"), STAT_LGUILifeCycleBehaviourUpdate, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("LGUILifeCycleBehaviour Start"), STAT_LGUILifeCycleBehaviourStart, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("UpdateLayoutInterface"), STAT_UpdateLayoutInterface, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas Update"), STAT_UpdateCanvas, STATGROUP_LGUI);
void ALGUIManagerActor::Tick(float DeltaTime)
{
	//editor draw helper frame
#if WITH_EDITOR
	if (this->GetWorld())
	{
		auto Settings = GetDefault<ULGUIEditorSettings>();
		if (Settings->bDrawHelperFrame)
		{
			if (this->GetWorld()->WorldType == EWorldType::Game
				|| this->GetWorld()->WorldType == EWorldType::PIE
				)
			{
				for (auto& CanvasItem : AllCanvasArray)
				{
					auto& UIItemArray = CanvasItem->GetUIItemArray();
					for (auto& UIItem : UIItemArray)
					{
						if (!IsValid(UIItem))continue;
						if (!IsValid(UIItem->GetWorld()))continue;

						ALGUIManagerActor::DrawFrameOnUIItem(UIItem, UIItem->IsScreenSpaceOverlayUI());
					}
				}
			}
		}

		if (Settings->bDrawSelectableNavigationVisualizer)
		{
			for (auto& item : AllSelectableArray)
			{
				if (!item.IsValid())continue;
				if (!IsValid(item->GetWorld()))continue;
				if (!IsValid(item->GetRootUIComponent()))continue;
				if (!item->GetRootUIComponent()->GetIsUIActiveInHierarchy())continue;

				ALGUIManagerActor::DrawNavigationVisualizerOnUISelectable(item->GetWorld(), item.Get(), item->GetRootUIComponent()->IsScreenSpaceOverlayUI());
			}
		}
	}
#endif

	//Update culture
	{
		if (bShouldUpdateOnCultureChanged)
		{
			bShouldUpdateOnCultureChanged = false;
			for (auto& item : AllCultureChangedArray)
			{
				ILGUICultureChangedInterface::Execute_OnCultureChanged(item.GetObject());
			}
		}
	}

	//LGUILifeCycleBehaviour start
	{
		if (LGUILifeCycleBehavioursForStart.Num() > 0)
		{
			bIsExecutingStart = true;
			SCOPE_CYCLE_COUNTER(STAT_LGUILifeCycleBehaviourStart);
			for (int i = 0; i < LGUILifeCycleBehavioursForStart.Num(); i++)
			{
				auto item = LGUILifeCycleBehavioursForStart[i];
				if (item.IsValid())
				{
					item->Call_Start();
					if (item->bCanExecuteUpdate && !item->bIsAddedToUpdate)
					{
						item->bIsAddedToUpdate = true;
						LGUILifeCycleBehavioursForUpdate.Add(item);
					}
				}
			}
			LGUILifeCycleBehavioursForStart.Reset();
			bIsExecutingStart = false;
		}
	}

	//LGUILifeCycleBehaviour update
	{
		bIsExecutingUpdate = true;
		SCOPE_CYCLE_COUNTER(STAT_LGUILifeCycleBehaviourUpdate);
		for (int i = 0; i < LGUILifeCycleBehavioursForUpdate.Num(); i++)
		{
			CurrentExecutingUpdateIndex = i;
			auto item = LGUILifeCycleBehavioursForUpdate[i];
			if (item.IsValid())
			{
				item->Update(DeltaTime);
			}
		}
		bIsExecutingUpdate = false;
		CurrentExecutingUpdateIndex = -1;
		//remove these padding things
		if (LGUILifeCycleBehavioursNeedToRemoveFromUpdate.Num() > 0)
		{
			for (auto& item : LGUILifeCycleBehavioursNeedToRemoveFromUpdate)
			{
				LGUILifeCycleBehavioursForUpdate.Remove(item);
			}
			LGUILifeCycleBehavioursNeedToRemoveFromUpdate.Reset();
		}
	}

#if WITH_EDITOR
	int ScreenSpaceOverlayCanvasCount = 0;
	for (auto& item : AllCanvasArray)
	{
		if (item.IsValid())
		{
			if (item->IsRootCanvas())
			{
				if (item->GetActualRenderMode() == ELGUIRenderMode::ScreenSpaceOverlay)
				{
					ScreenSpaceOverlayCanvasCount++;
				}
			}
		}
	}
	if (ScreenSpaceOverlayCanvasCount > 1)
	{
		if (PrevScreenSpaceOverlayCanvasCount != ScreenSpaceOverlayCanvasCount)//only show message when change
		{
			PrevScreenSpaceOverlayCanvasCount = ScreenSpaceOverlayCanvasCount;
			auto errMsg = LOCTEXT("MultipleLGUICanvasRenderScreenSpaceOverlay", "Detect multiply LGUICanvas renderred with ScreenSpaceOverlay mode, this is not allowed! There should be only one ScreenSpace UI in a world!");
			UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
			LGUIUtils::EditorNotification(errMsg, 10.0f);
		}
	}
	else
	{
		PrevScreenSpaceOverlayCanvasCount = 0;
	}
#endif

	UpdateLayout();

	//update drawcall
	{
		SCOPE_CYCLE_COUNTER(STAT_UpdateCanvas);
		for (auto& item : AllCanvasArray)
		{
			if (item.IsValid() && item->IsRootCanvas())
			{
				item->UpdateRootCanvas();
			}
		}
	}

	//sort render order
	if (AllCanvasArray.Num() > 0)
	{
		if (bShouldSortLGUIRenderer || bShouldSortWorldSpaceCanvas || bShouldSortRenderTargetSpaceCanvas)
		{
			//@todo: no need to sort all canvas
			AllCanvasArray.Sort([](const TWeakObjectPtr<ULGUICanvas>& A, const TWeakObjectPtr<ULGUICanvas>& B)
				{
					auto ASortOrder = A->GetActualSortOrder();
					auto BSortOrder = B->GetActualSortOrder();
					if (ASortOrder == BSortOrder)
					{
						if (A->GetUIItem() != nullptr && B->GetUIItem() != nullptr)
						{
							return A->GetUIItem()->GetFlattenHierarchyIndex() < B->GetUIItem()->GetFlattenHierarchyIndex();
						}
					}
					return ASortOrder < BSortOrder;
				});
		}
		if (bShouldSortLGUIRenderer)
		{
			SortDrawcallOnRenderMode(ELGUIRenderMode::ScreenSpaceOverlay);
			SortDrawcallOnRenderMode(ELGUIRenderMode::WorldSpace_LGUI);
			if (ScreenSpaceOverlayViewExtension.IsValid())
			{
				ScreenSpaceOverlayViewExtension->SortScreenAndWorldSpacePrimitiveRenderPriority();
			}
			bShouldSortLGUIRenderer = false;
		}
		if (bShouldSortWorldSpaceCanvas)
		{
			SortDrawcallOnRenderMode(ELGUIRenderMode::WorldSpace);
			bShouldSortWorldSpaceCanvas = false;
		}
		if (bShouldSortRenderTargetSpaceCanvas)
		{
			SortDrawcallOnRenderMode(ELGUIRenderMode::RenderTarget);
			bShouldSortRenderTargetSpaceCanvas = false;
		}
	}
}

void ALGUIManagerActor::SortDrawcallOnRenderMode(ELGUIRenderMode InRenderMode)//@todo: cleanup this function
{
	int32 RenderPriority = 0;
	static TSet<ULGUICanvas*> ProcessedCanvasArray;
	ProcessedCanvasArray.Reset();
	for (int i = 0; i < AllCanvasArray.Num(); i++)
	{
		auto canvasItem = this->AllCanvasArray[i];
		if (canvasItem.IsValid() && canvasItem->GetIsUIActive())
		{
			if (canvasItem->GetActualRenderMode() == InRenderMode)
			{
				if (!ProcessedCanvasArray.Contains(canvasItem.Get()))
				{
					canvasItem->SortDrawcall(RenderPriority, ProcessedCanvasArray);
				}
			}
		}
	}
}

void ALGUIManagerActor::AddLGUILifeCycleBehaviourForLifecycleEvent(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld(), true))
		{
			if (auto RootActorDataPtr = Instance->AllActors_PrefabSystemProcessing.Find(InComp->GetOwner()))//processing by prefab system, collect for further operation
			{
				if (auto ArrayPtr = Instance->LGUILifeCycleBehaviours_PrefabSystemProcessing.Find(RootActorDataPtr->Key))
				{
					InComp->bIsSerializedFromLGUIPrefab = true;
					auto& CompArray = ArrayPtr->LGUILifeCycleBehaviourArray;
					auto FoundIndex = CompArray.IndexOfByPredicate([InComp](const TTuple<TWeakObjectPtr<ULGUILifeCycleBehaviour>, int32>& Item) {
						return InComp == Item.Key;
						});
					if (FoundIndex != INDEX_NONE)
					{
						UE_LOG(LGUI, Error, TEXT("[ALGUIManagerActor::AddLGUILifeCycleBehaviourForLifecycleEvent]already contains, comp:%s"), *(InComp->GetPathName()));
						FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
						return;
					}
					TTuple<TWeakObjectPtr<ULGUILifeCycleBehaviour>, int32> Item;
					Item.Key = InComp;
					Item.Value = RootActorDataPtr->Value;
					CompArray.Add(Item);
				}
			}
			else//not processed by prefab system, just do immediately
			{
				ProcessLGUILifecycleEvent(InComp);
			}
		}
	}
}

void ALGUIManagerActor::AddLGUILifeCycleBehavioursForUpdate(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld(), true))
		{
			int32 index = INDEX_NONE;
			if (!Instance->LGUILifeCycleBehavioursForUpdate.Find(InComp, index))
			{
				Instance->LGUILifeCycleBehavioursForUpdate.Add(InComp);
				return;
			}
			UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::AddLGUILifeCycleBehavioursForUpdate]Already exist, comp:%s"), *(InComp->GetPathName()));
		}
	}
}
void ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromUpdate(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld(), false))
		{
			auto& updateArray = Instance->LGUILifeCycleBehavioursForUpdate;
			int32 index = INDEX_NONE;
			if (updateArray.Find(InComp, index))
			{
				if (Instance->bIsExecutingUpdate)
				{
					if (index > Instance->CurrentExecutingUpdateIndex)//not execute it yet, safe to remove
					{
						updateArray.RemoveAt(index);
					}
					else//already execute or current execute it, not safe to remove. should remove it after execute process complete
					{
						Instance->LGUILifeCycleBehavioursNeedToRemoveFromUpdate.Add(InComp);
					}
				}
				else//not executing update, safe to remove
				{
					updateArray.RemoveAt(index);
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromUpdate]Not exist, comp:%s"), *(InComp->GetPathName()));
			}

			//cleanup array
			int inValidCount = 0;
			for (int i = updateArray.Num() - 1; i >= 0; i--)
			{
				if (!updateArray[i].IsValid())
				{
					updateArray.RemoveAt(i);
					inValidCount++;
				}
			}
			if (inValidCount > 0)
			{
				UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromUpdate]Cleanup %d invalid LGUILifeCycleBehaviour"), inValidCount);
			}
		}
	}
}

void ALGUIManagerActor::AddLGUILifeCycleBehavioursForStart(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld(), true))
		{
			int32 index = INDEX_NONE;
			if (!Instance->LGUILifeCycleBehavioursForStart.Find(InComp, index))
			{
				Instance->LGUILifeCycleBehavioursForStart.Add(InComp);
				return;
			}
			UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::AddLGUILifeCycleBehavioursForStart]Already exist, comp:%s"), *(InComp->GetPathName()));
		}
	}
}
void ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromStart(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld(), false))
		{
			auto& startArray = Instance->LGUILifeCycleBehavioursForStart;
			int32 index = INDEX_NONE;
			if (startArray.Find(InComp, index))
			{
				if (Instance->bIsExecutingStart)
				{
					if (!InComp->bIsStartCalled)//if already called start then nothing to do, because start array will be cleared after execute start
					{
						startArray.RemoveAt(index);//not execute start yet, safe to remove
					}
				}
				else
				{
					startArray.RemoveAt(index);//not executing start, safe to remove
				}
			}
			else
			{
				UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromStart]Not exist, comp:%s"), *(InComp->GetPathName()));
			}

			//cleanup array
			int inValidCount = 0;
			for (int i = startArray.Num() - 1; i >= 0; i--)
			{
				if (!startArray[i].IsValid())
				{
					startArray.RemoveAt(i);
					inValidCount++;
				}
			}
			if (inValidCount > 0)
			{
				UE_LOG(LGUI, Warning, TEXT("[ALGUIManagerActor::RemoveLGUILifeCycleBehavioursFromStart]Cleanup %d invalid LGUILifeCycleBehaviour"), inValidCount);
			}
		}
	}
}

void ALGUIManagerActor::RegisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld(), true))
	{
		Instance->AllCultureChangedArray.AddUnique(InItem);
	}
}
void ALGUIManagerActor::UnregisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
		Instance->AllCultureChangedArray.RemoveSingle(InItem);
	}
}

void ALGUIManagerActor::UpdateLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateLayoutInterface);

	//update Layout
	if (bNeedUpdateLayout)
	{
		bNeedUpdateLayout = false;
		for (auto& item : AllLayoutArray)
		{
			ILGUILayoutInterface::Execute_OnUpdateLayout(item.GetObject());
		}
	}
}
void ALGUIManagerActor::ForceUpdateLayout(UObject* WorldContextObject)
{
	if (auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (auto Instance = GetInstance(world, false))
		{
			Instance->UpdateLayout();
		}
	}
}

void ALGUIManagerActor::RemoveCanvas(ULGUICanvas* InCanvas)
{
	if (auto Instance = GetInstance(InCanvas->GetWorld(), false))
	{
#if !UE_BUILD_SHIPPING
		check(Instance->AllCanvasArray.Contains(InCanvas));
#endif
		Instance->AllCanvasArray.RemoveSingle(InCanvas);
	}
}
void ALGUIManagerActor::AddCanvas(ULGUICanvas* InCanvas)
{
	if (auto Instance = GetInstance(InCanvas->GetWorld(), true))
	{
#if !UE_BUILD_SHIPPING
		check(!Instance->AllCanvasArray.Contains(InCanvas));
#endif
		Instance->AllCanvasArray.AddUnique(InCanvas);
	}
}
void ALGUIManagerActor::MarkSortLGUIRenderer()
{
	bShouldSortLGUIRenderer = true;
}
void ALGUIManagerActor::MarkSortWorldSpaceCanvas()
{
	bShouldSortWorldSpaceCanvas = true;
}
void ALGUIManagerActor::MarkSortRenderTargetSpaceCanvas()
{
	bShouldSortRenderTargetSpaceCanvas = true;
}

TSharedPtr<class FLGUIHudRenderer, ESPMode::ThreadSafe> ALGUIManagerActor::GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist)
{
	if (auto Instance = GetInstance(InWorld, true))
	{
		if (!Instance->ScreenSpaceOverlayViewExtension.IsValid())
		{
			if (InCreateIfNotExist)
			{
				Instance->ScreenSpaceOverlayViewExtension = FSceneViewExtensions::NewExtension<FLGUIHudRenderer>(InWorld);
			}
		}
		return Instance->ScreenSpaceOverlayViewExtension;
	}
	return nullptr;
}

void ALGUIManagerActor::AddRaycaster(ULGUIBaseRaycaster* InRaycaster)
{
	if (auto Instance = GetInstance(InRaycaster->GetWorld(), true))
	{
		auto& AllRaycasterArray = Instance->AllRaycasterArray;
		if (AllRaycasterArray.Contains(InRaycaster))return;
		//check multiple racaster
		for (auto& item : AllRaycasterArray)
		{
			if (InRaycaster->GetDepth() == item->GetDepth() && InRaycaster->GetTraceChannel() == item->GetTraceChannel())
			{
				auto msg = FString(TEXT("\
\nDetect multiple LGUIBaseRaycaster components with same depth and traceChannel, this may cause wrong interaction results!\
\neg: Want use mouse to click object A but get object B.\
\nPlease note:\
\n	For LGUIBaseRaycasters with same depth, LGUI will line trace them all and sort result on hit distance.\
\n	For LGUIBaseRaycasters with different depth, LGUI will sort raycasters on depth, and line trace from highest depth to lowest, if hit anything then stop line trace."));
				UE_LOG(LGUI, Warning, TEXT("\n%s"), *msg);
				break;
			}
		}

		AllRaycasterArray.Add(InRaycaster);
		//sort depth
		AllRaycasterArray.Sort([](const TWeakObjectPtr<ULGUIBaseRaycaster>& A, const TWeakObjectPtr<ULGUIBaseRaycaster>& B)
		{
			return A->GetDepth() > B->GetDepth();
		});
	}
}
void ALGUIManagerActor::RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster)
{
	if (auto Instance = GetInstance(InRaycaster->GetWorld()))
	{
		int32 index;
		if (Instance->AllRaycasterArray.Find(InRaycaster, index))
		{
			Instance->AllRaycasterArray.RemoveAt(index);
		}
	}
}

void ALGUIManagerActor::SetCurrentInputModule(ULGUIBaseInputModule* InInputModule)
{
	if (auto Instance = GetInstance(InInputModule->GetWorld(), true))
	{
		Instance->CurrentInputModule = InInputModule;
	}
}
void ALGUIManagerActor::ClearCurrentInputModule(ULGUIBaseInputModule* InInputModule)
{
	if (auto Instance = GetInstance(InInputModule->GetWorld()))
	{
		Instance->CurrentInputModule = nullptr;
	}
}

void ALGUIManagerActor::AddSelectable(UUISelectableComponent* InSelectable)
{
	if (auto Instance = GetInstance(InSelectable->GetWorld(), true))
	{
		auto& AllSelectableArray = Instance->AllSelectableArray;
		if (AllSelectableArray.Contains(InSelectable))return;
		AllSelectableArray.Add(InSelectable);
	}
}
void ALGUIManagerActor::RemoveSelectable(UUISelectableComponent* InSelectable)
{
	if (auto Instance = GetInstance(InSelectable->GetWorld()))
	{
		int32 index;
		if (Instance->AllSelectableArray.Find(InSelectable, index))
		{
			Instance->AllSelectableArray.RemoveAt(index);
		}
	}
}

void ALGUIManagerActor::RegisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld(), true))
	{
#if !UE_BUILD_SHIPPING
		check(!Instance->AllLayoutArray.Contains(InItem));
#endif
		Instance->AllLayoutArray.AddUnique(InItem);
	}
}
void ALGUIManagerActor::UnregisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
#if !UE_BUILD_SHIPPING
		check(Instance->AllLayoutArray.Contains(InItem));
#endif
		Instance->AllLayoutArray.RemoveSingle(InItem);
	}
}
void ALGUIManagerActor::MarkUpdateLayout(UWorld* InWorld)
{
	if (auto Instance = GetInstance(InWorld, false))
	{
		Instance->bNeedUpdateLayout = true;
	}
}

#if WITH_EDITOR
bool ALGUIManagerActor::GetIsPlaying(UWorld* InWorld)
{
	if (auto Instance = GetInstance(InWorld, false))
	{
		return Instance->bIsPlaying;
	}
	return false;
}
#endif


void ALGUIManagerActor::ProcessLGUILifecycleEvent(ULGUILifeCycleBehaviour* InComp)
{
	if (InComp)
	{
		if (InComp->IsAllowedToCallAwake())
		{
			if (!InComp->bIsAwakeCalled)
			{
				InComp->Call_Awake();
			}
			if (InComp->IsAllowedToCallOnEnable() && InComp->GetEnable())
			{
				if (!InComp->bIsEnableCalled)
				{
					InComp->Call_OnEnable();
				}
			}
		}
	}
}
void ALGUIManagerActor::BeginPrefabSystemProcessingActor(AActor* InRootActor)
{
	FLGUILifeCycleBehaviourArrayContainer Container;
	LGUILifeCycleBehaviours_PrefabSystemProcessing.Add(InRootActor, Container);
}
void ALGUIManagerActor::EndPrefabSystemProcessingActor(AActor* InRootActor)
{
	if (auto ArrayPtr = LGUILifeCycleBehaviours_PrefabSystemProcessing.Find(InRootActor))
	{
		auto& LateFunctions = ArrayPtr->Functions;
		for (auto& Function : LateFunctions)
		{
			Function();
		}

		auto& LGUILifeCycleBehaviourArray = ArrayPtr->LGUILifeCycleBehaviourArray;
		//sort by hierarchy index
		LGUILifeCycleBehaviourArray.Sort([](const TTuple<TWeakObjectPtr<ULGUILifeCycleBehaviour>, int32>& A, const TTuple<TWeakObjectPtr<ULGUILifeCycleBehaviour>, int32>& B) {
			return A.Value < B.Value;
			});
		auto Count = LGUILifeCycleBehaviourArray.Num();
		for (int i = Count - 1; i >= 0; i--)//execute from tail to head, when in prefab the lower in hierarchy will execute earlier
		{
			auto Item = LGUILifeCycleBehaviourArray[i];
			if (Item.Key.IsValid())
			{
				ProcessLGUILifecycleEvent(Item.Key.Get());
			}
#if !UE_BUILD_SHIPPING
			check(LGUILifeCycleBehaviourArray.Num() == Count);
#endif
		}

		LGUILifeCycleBehaviours_PrefabSystemProcessing.Remove(InRootActor);
	}
}
void ALGUIManagerActor::AddActorForPrefabSystem(AActor* InActor, AActor* InRootActor, int32 InActorIndex)
{
	TTuple<AActor*, int32> Item;
	Item.Key = InRootActor;
	Item.Value = InActorIndex;
	AllActors_PrefabSystemProcessing.Add(InActor, Item);
}
void ALGUIManagerActor::RemoveActorForPrefabSystem(AActor* InActor, AActor* InRootActor)
{
	AllActors_PrefabSystemProcessing.Remove(InActor);
}
bool ALGUIManagerActor::IsPrefabSystemProcessingActor(AActor* InActor)
{
	return AllActors_PrefabSystemProcessing.Contains(InActor);
}
void ALGUIManagerActor::AddFunctionForPrefabSystemExecutionBeforeAwake(AActor* InPrefabActor, const TFunction<void()>& InFunction)
{
	if (auto RootActorDataPtr = AllActors_PrefabSystemProcessing.Find(InPrefabActor))
	{
		auto& Container = LGUILifeCycleBehaviours_PrefabSystemProcessing[RootActorDataPtr->Key];
		Container.Functions.Add(InFunction);
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
#undef LOCTEXT_NAMESPACE