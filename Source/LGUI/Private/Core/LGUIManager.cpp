// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIManager.h"
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
#include "Core/ActorComponent/UIBatchMeshRenderable.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "Engine/Engine.h"
#include "Core/LGUIRender/LGUIRenderer.h"
#include "Core/ILGUICultureChangedInterface.h"
#include "Core/LGUILifeCycleBehaviour.h"
#include "Layout/ILGUILayoutInterface.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "PrefabSystem/LGUIPrefabHelperObject.h"
#if WITH_EDITOR
#include "Editor.h"
#include "DrawDebugHelpers.h"
#include "Engine/Selection.h"
#include "EditorViewportClient.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "EngineUtils.h"
#include "Layout/LGUICanvasScaler.h"
#include "Core/LGUISpriteData.h"
#include "Core/Actor/UIContainerActor.h"
#endif

#define LOCTEXT_NAMESPACE "LGUIManagerObject"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

ULGUIEditorManagerObject* ULGUIEditorManagerObject::Instance = nullptr;
#if WITH_EDITOR
int ULGUIEditorManagerObject::IndexOfClickSelectUI = INDEX_NONE;
#endif
ULGUIEditorManagerObject::ULGUIEditorManagerObject()
{
	if (this == GetDefault<ULGUIEditorManagerObject>())
	{
#if WITH_EDITOR
		ULGUIPrefabManagerObject::OnSerialize_SortChildrenActors.BindStatic([](TArray<AActor*>& ChildrenActors) {
			//Actually normal UIItem's hierarchyIndex property can do the job, but sub prefab's root actor not, so sort it to make sure.
			Algo::Sort(ChildrenActors, [](const AActor* A, const AActor* B) {
				auto ARoot = A->GetRootComponent();
				auto BRoot = B->GetRootComponent();
				if (ARoot != nullptr && BRoot != nullptr)
				{
					auto AUIRoot = Cast<UUIItem>(ARoot);
					auto BUIRoot = Cast<UUIItem>(BRoot);
					if (AUIRoot != nullptr && BUIRoot != nullptr)
					{
						return AUIRoot->GetHierarchyIndex() < BUIRoot->GetHierarchyIndex();//compare hierarch index for UI actor
					}
				}
				else
				{
					//sort on ActorLabel so the Tick function can be predictable because deserialize order is determinate.
					return A->GetActorLabel().Compare(B->GetActorLabel()) < 0;//compare name for normal actor
				}
				return false;
				});
			});
		ULGUIPrefabManagerObject::OnDeserialize_ProcessComponentsBeforeRerunConstructionScript.BindStatic([](const TArray<UActorComponent*>& Components) {
			for (auto& Comp : Components)
			{
				if (auto UIItem = Cast<UUIItem>(Comp))
				{
					UIItem->CalculateTransformFromAnchor();
				}
			}
			});

		ULGUIPrefabManagerObject::OnPrefabEditorViewport_MouseClick.BindStatic([](UWorld* World, const FVector& RayOrigin, const FVector& RayDirection, AActor*& ClickHitActor) {
			if (auto LGUIManager = ULGUIManagerWorldSubsystem::GetInstance(World))
			{
				float LineTraceLength = 100000;
				//find hit UIBatchMeshRenderable
				auto LineStart = RayOrigin;
				auto LineEnd = RayOrigin + RayDirection * LineTraceLength;
				UUIBaseRenderable* ClickHitUI = nullptr;
				static TArray<UUIItem*> AllUIItemArray;
				AllUIItemArray.Reset();
				{
					for (auto& CanvasItem : LGUIManager->GetCanvasArray(ELGUIRenderMode::ScreenSpaceOverlay))
					{
						AllUIItemArray.Append(CanvasItem->GetUIItemArray());
					}
					for (auto& CanvasItem : LGUIManager->GetCanvasArray(ELGUIRenderMode::RenderTarget))
					{
						AllUIItemArray.Append(CanvasItem->GetUIItemArray());
					}
					for (auto& CanvasItem : LGUIManager->GetCanvasArray(ELGUIRenderMode::WorldSpace))
					{
						AllUIItemArray.Append(CanvasItem->GetUIItemArray());
					}
					for (auto& CanvasItem : LGUIManager->GetCanvasArray(ELGUIRenderMode::WorldSpace_LGUI))
					{
						AllUIItemArray.Append(CanvasItem->GetUIItemArray());
					}
				}
				if (ULGUIManagerWorldSubsystem::RaycastHitUI(World, AllUIItemArray, LineStart, LineEnd, ClickHitUI, ULGUIEditorManagerObject::IndexOfClickSelectUI))
				{
					ClickHitActor = ClickHitUI->GetOwner();
				}
			}
			});
		ULGUIPrefabManagerObject::OnPrefabEditorViewport_MouseMove.BindStatic([](UWorld* World) {
			ULGUIEditorManagerObject::IndexOfClickSelectUI = INDEX_NONE;
			});

		ULGUIPrefabManagerObject::OnPrefabEditor_CreateRootAgent.BindStatic([](UWorld* World, UClass* RootActorClass, ULGUIPrefab* Prefab, AActor*& OutCreatedRootAgentActor)
			{
				if (RootActorClass->IsChildOf(AUIBaseActor::StaticClass()))//ui
				{
					auto CanvasSize = Prefab->PrefabDataForPrefabEditor.CanvasSize;
					//create Canvas for UI
					auto RootUICanvasActor = (AUIContainerActor*)(World->SpawnActor<AActor>(AUIContainerActor::StaticClass(), FTransform::Identity));
					RootUICanvasActor->GetRootComponent()->SetWorldLocationAndRotationNoPhysics(FVector::ZeroVector, FRotator(0, 0, 0));

					if (Prefab->PrefabDataForPrefabEditor.bNeedCanvas)
					{
						auto RenderMode = (ELGUIRenderMode)Prefab->PrefabDataForPrefabEditor.CanvasRenderMode;
						auto CanvasComp = NewObject<ULGUICanvas>(RootUICanvasActor);
						CanvasComp->RegisterComponent();
						RootUICanvasActor->AddInstanceComponent(CanvasComp);
						CanvasComp->SetRenderMode(RenderMode);
					}

					RootUICanvasActor->GetUIItem()->SetWidth(CanvasSize.X);
					RootUICanvasActor->GetUIItem()->SetHeight(CanvasSize.Y);
					RootUICanvasActor->GetUIItem()->SetHierarchyIndex(0);

					OutCreatedRootAgentActor = RootUICanvasActor;
				}
				else//not ui
				{
					auto CreatedActor = World->SpawnActor<AActor>(AActor::StaticClass(), FTransform::Identity, FActorSpawnParameters());
					//create SceneComponent
					{
						USceneComponent* RootComponent = NewObject<USceneComponent>(CreatedActor, USceneComponent::GetDefaultSceneRootVariableName(), RF_Transactional);
						RootComponent->Mobility = EComponentMobility::Static;
						RootComponent->bVisualizeComponent = false;

						CreatedActor->SetRootComponent(RootComponent);
						RootComponent->RegisterComponent();
						CreatedActor->AddInstanceComponent(RootComponent);
					}
					OutCreatedRootAgentActor = CreatedActor;
				}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_GetBounds.BindStatic([](USceneComponent* SceneComp, FBox& OutBounds, bool& OutValidBounds)
			{
				if (auto UIItem = Cast<UUIItem>(SceneComp))
				{
					if (UIItem->GetIsUIActiveInHierarchy())
					{
						OutBounds = UIItem->Bounds.GetBox();
						OutValidBounds = true;
					}
				}
				else if (auto PrimitiveComp = Cast<UPrimitiveComponent>(SceneComp))
				{
					OutBounds = PrimitiveComp->Bounds.GetBox();
					OutValidBounds = true;
				}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_SavePrefab.BindStatic([](AActor* RootAgentActor, ULGUIPrefab* Prefab)
			{
				if (auto UIItem = Cast<UUIItem>(RootAgentActor->GetRootComponent()))
				{
					Prefab->PrefabDataForPrefabEditor.CanvasSize = FIntPoint(UIItem->GetWidth(), UIItem->GetHeight());
				}
				if (auto Canvas = RootAgentActor->FindComponentByClass<ULGUICanvas>())
				{
					Prefab->PrefabDataForPrefabEditor.bNeedCanvas = true;
					Prefab->PrefabDataForPrefabEditor.CanvasRenderMode = (uint8)Canvas->GetRenderMode();
				}
				else
				{
					Prefab->PrefabDataForPrefabEditor.bNeedCanvas = false;
				}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_Refresh.BindStatic([]() {
			ULGUIManagerWorldSubsystem::RefreshAllUI();
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_ReplaceObjectPropertyForApplyOrRevert.BindStatic([](ULGUIPrefabHelperObject* PrefabHelper, UObject* InObject, FName& InPropertyName) {
			if (auto UIItem = Cast<UUIItem>(InObject))
			{
				if (InPropertyName == USceneComponent::GetRelativeLocationPropertyName())
				{
					InPropertyName = UUIItem::GetAnchorDataPropertyName();
				}
			}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_AfterObjectPropertyApplyOrRevert.BindStatic([](ULGUIPrefabHelperObject* PrefabHelper, UObject* InObject, FName InPropertyName) {
			if (auto UIItem = Cast<UUIItem>(InObject))
			{
				if (InPropertyName == UUIItem::GetAnchorDataPropertyName())
				{
					UIItem->CalculateTransformFromAnchor();//calculate transform here, because when NotifyPropertyChanged the PostActorConstruction->MoveComponent will call then anchor will calculate from transform value which is wrong
					PrefabHelper->RemoveMemberPropertyFromSubPrefab(UIItem->GetOwner(), InObject, USceneComponent::GetRelativeLocationPropertyName());//remove RelativeLocation override because UIItem use AnchorData to calculate RelativeLocation
				}
			}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_AfterMakePrefabAsSubPrefab.BindStatic([](ULGUIPrefabHelperObject* PrefabHelper, AActor* InRootActor) {
			//mark HierarchyIndex as default override parameter
			auto RootComp = InRootActor->GetRootComponent();
			if (auto RootUIComp = Cast<UUIItem>(RootComp))
			{
				PrefabHelper->AddMemberPropertyToSubPrefab(InRootActor, RootUIComp, UUIItem::GetHierarchyIndexPropertyName());
			}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_AfterCollectPropertyToOverride.BindStatic([](ULGUIPrefabHelperObject* PrefabHelper, UObject* InObject, FName InPropertyName) {
			if (auto UIItem = Cast<UUIItem>(InObject))
			{
				if (InPropertyName == USceneComponent::GetRelativeLocationPropertyName())//if UI's relative location change, then record anchor data too
				{
					PrefabHelper->AddMemberPropertyToSubPrefab(UIItem->GetOwner(), InObject, UUIItem::GetAnchorDataPropertyName());
				}
				else if (InPropertyName == UUIItem::GetAnchorDataPropertyName())//if UI's anchor data change, then record relative location too
				{
					PrefabHelper->AddMemberPropertyToSubPrefab(UIItem->GetOwner(), InObject, USceneComponent::GetRelativeLocationPropertyName());
				}
			}
			});
		ULGUIPrefabManagerObject::OnPrefabEditor_CopyRootObjectParentAnchorData.BindStatic([](ULGUIPrefabHelperObject* PrefabHelper, UObject* InObject, UObject* OriginObject) {
			auto InObjectUIItem = Cast<UUIItem>(InObject);
			auto OriginObjectUIItem = Cast<UUIItem>(OriginObject);
			if (InObjectUIItem != nullptr && OriginObjectUIItem != nullptr)//if is UI item, we need to copy parent's property to origin object's parent property, to make anchor & location calculation right
			{
				auto InObjectParent = InObjectUIItem->GetParentUIItem();
				auto OriginObjectParent = OriginObjectUIItem->GetParentUIItem();
				if (InObjectParent != nullptr && OriginObjectParent != nullptr)
				{
					//copy relative location
					auto RelativeLocationProperty = FindFProperty<FProperty>(InObjectParent->GetClass(), USceneComponent::GetRelativeLocationPropertyName());
					RelativeLocationProperty->CopyCompleteValue_InContainer(OriginObjectParent, InObjectParent);
					LGUIUtils::NotifyPropertyChanged(OriginObjectParent, RelativeLocationProperty);
					//copy anchor data
					auto AnchorDataProperty = FindFProperty<FProperty>(InObjectParent->GetClass(), UUIItem::GetAnchorDataPropertyName());
					AnchorDataProperty->CopyCompleteValue_InContainer(OriginObjectParent, InObjectParent);
					LGUIUtils::NotifyPropertyChanged(OriginObjectParent, AnchorDataProperty);
				}
			}
			});
#endif
	}
}
void ULGUIEditorManagerObject::BeginDestroy()
{
#if WITH_EDITORONLY_DATA
	if (OnAssetReimportDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			if (auto ImportSubsystem = GEditor->GetEditorSubsystem<UImportSubsystem>())
			{
				ImportSubsystem->OnAssetReimport.Remove(OnAssetReimportDelegateHandle);
			}
		}
	}
	if (OnActorLabelChangedDelegateHandle.IsValid())
	{
		FCoreDelegates::OnActorLabelChanged.Remove(OnActorLabelChangedDelegateHandle);
	}
	if (OnMapOpenedDelegateHandle.IsValid())
	{
		FEditorDelegates::OnMapOpened.Remove(OnMapOpenedDelegateHandle);
	}
	if (OnPackageReloadedDelegateHandle.IsValid())
	{
		FCoreUObjectDelegates::OnPackageReloaded.Remove(OnPackageReloadedDelegateHandle);
	}
	if (OnBlueprintPreCompileDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintPreCompile().Remove(OnBlueprintPreCompileDelegateHandle);
		}
	}
	if (OnBlueprintCompiledDelegateHandle.IsValid())
	{
		if (GEditor)
		{
			GEditor->OnBlueprintCompiled().Remove(OnBlueprintCompiledDelegateHandle);
		}
	}
#endif
	Instance = nullptr;
	Super::BeginDestroy();
}

void ULGUIEditorManagerObject::Tick(float DeltaTime)
{
#if WITH_EDITOR
	CheckEditorViewportIndexAndKey();
#endif
}
TStatId ULGUIEditorManagerObject::GetStatId() const
{
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIEditorManagerObject, STATGROUP_Tickables);
}

#if WITH_EDITOR
FDelegateHandle ULGUIEditorManagerObject::RegisterEditorViewportIndexAndKeyChange(const TFunction<void()>& InFunction)
{
	InitCheck();
	return Instance->EditorViewportIndexAndKeyChange.AddLambda(InFunction);
}
void ULGUIEditorManagerObject::UnregisterEditorViewportIndexAndKeyChange(const FDelegateHandle& InDelegateHandle)
{
	if (Instance != nullptr)
	{
		Instance->EditorViewportIndexAndKeyChange.Remove(InDelegateHandle);
	}
}

ULGUIEditorManagerObject* ULGUIEditorManagerObject::GetInstance(bool CreateIfNotValid)
{
	if (CreateIfNotValid)
	{
		InitCheck();
	}
	return Instance;
}
bool ULGUIEditorManagerObject::InitCheck()
{
	if (Instance == nullptr)
	{
		Instance = NewObject<ULGUIEditorManagerObject>();
		Instance->AddToRoot();
		UE_LOG(LGUI, Log, TEXT("[ULGUIManagerObject::InitCheck]No Instance for LGUIManagerObject, create!"));
		Instance->OnActorLabelChangedDelegateHandle = FCoreDelegates::OnActorLabelChanged.AddUObject(Instance, &ULGUIEditorManagerObject::OnActorLabelChanged);
		//open map
		Instance->OnMapOpenedDelegateHandle = FEditorDelegates::OnMapOpened.AddUObject(Instance, &ULGUIEditorManagerObject::OnMapOpened);
		Instance->OnPackageReloadedDelegateHandle = FCoreUObjectDelegates::OnPackageReloaded.AddUObject(Instance, &ULGUIEditorManagerObject::OnPackageReloaded);
		if (GEditor)
		{
			//reimport asset
			Instance->OnAssetReimportDelegateHandle = GEditor->GetEditorSubsystem<UImportSubsystem>()->OnAssetReimport.AddUObject(Instance, &ULGUIEditorManagerObject::OnAssetReimport);
			//blueprint recompile
			Instance->OnBlueprintPreCompileDelegateHandle = GEditor->OnBlueprintPreCompile().AddUObject(Instance, &ULGUIEditorManagerObject::OnBlueprintPreCompile);
			Instance->OnBlueprintCompiledDelegateHandle = GEditor->OnBlueprintCompiled().AddUObject(Instance, &ULGUIEditorManagerObject::OnBlueprintCompiled);
		}
	}
	return true;
}

void ULGUIEditorManagerObject::OnBlueprintPreCompile(UBlueprint* InBlueprint)
{
	
}
void ULGUIEditorManagerObject::OnBlueprintCompiled()
{
	ULGUIPrefabManagerObject::AddOneShotTickFunction([] {
		ULGUIManagerWorldSubsystem::RefreshAllUI();
		});
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
				ULGUIManagerWorldSubsystem::RefreshAllUI();
			}
		}
	}
}

void ULGUIEditorManagerObject::OnMapOpened(const FString& FileName, bool AsTemplate)
{

}

void ULGUIEditorManagerObject::OnPackageReloaded(EPackageReloadPhase Phase, FPackageReloadedEvent* Event)
{
	if (Phase == EPackageReloadPhase::PostBatchPostGC && Event != nullptr && Event->GetNewPackage() != nullptr)
	{
		auto Asset = Event->GetNewPackage()->FindAssetInPackage();
		if (auto PrefabAsset = Cast<ULGUIPrefab>(Asset))
		{
			
		}
	}
}

void ULGUIEditorManagerObject::OnActorLabelChanged(AActor* actor)
{
	if (!IsValid(actor))return;
	auto World = actor->GetWorld();
	if (!IsValid(World))return;
	if (World->IsGameWorld())return;
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
#endif



#if WITH_EDITOR
void ULGUIManagerWorldSubsystem::DrawFrameOnUIItem(UUIItem* item, bool IsScreenSpace)
{
	auto RectExtends = FVector(0.1f, item->GetWidth(), item->GetHeight()) * 0.5f;
	bool bCanDrawRect = false;
	auto RectDrawColor = FColor(128, 128, 128, 128);//gray means normal object
	if (ULGUIPrefabManagerObject::IsSelected(item->GetOwner()))//select self
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
				ULGUIManagerWorldSubsystem::DrawDebugRectOnScreenSpace(item->GetWorld(), WorldLocation, GeometryBoundsExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), GeometryBoundsDrawColor);
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
			if (ULGUIPrefabManagerObject::IsSelected(item->GetParentUIItem()->GetOwner()))
			{
				bCanDrawRect = true;
			}
		}
		//child selected
		auto& childrenCompArray = item->GetAttachUIChildren();
		for (auto& uiComp : childrenCompArray)
		{
			if (IsValid(uiComp) && IsValid(uiComp->GetOwner()) && ULGUIPrefabManagerObject::IsSelected(uiComp->GetOwner()))
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
				if (IsValid(uiComp) && IsValid(uiComp->GetOwner()) && ULGUIPrefabManagerObject::IsSelected(uiComp->GetOwner()))
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
				if (ULGUIPrefabManagerObject::AnySelectedIsChildOf(item->GetOwner()))
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
			ULGUIManagerWorldSubsystem::DrawDebugRectOnScreenSpace(item->GetWorld(), WorldLocation, RectExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), RectDrawColor);
		}
		else
		{
			DrawDebugBox(item->GetWorld(), WorldLocation, RectExtends * WorldTransform.GetScale3D(), WorldTransform.GetRotation(), RectDrawColor);
		}
	}
}

void ULGUIManagerWorldSubsystem::DrawNavigationArrow(UWorld* InWorld, const TArray<FVector>& InControlPoints, const FVector& InArrowPointA, const FVector& InArrowPointB, FColor const& InColor, bool IsScreenSpace)
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
		auto ViewExtension = ULGUIManagerWorldSubsystem::GetViewExtension(InWorld, false);
		if (ViewExtension.IsValid())
		{
			TArray<FLGUIHelperLineVertex> Lines;
			//lines
			FVector prevPoint = ResultPoints[0];
			for (int i = 1; i < ResultPoints.Num(); i++)
			{
				new(Lines) FLGUIHelperLineVertex((FVector3f)prevPoint, InColor);
				new(Lines) FLGUIHelperLineVertex((FVector3f)ResultPoints[i], InColor);
				prevPoint = ResultPoints[i];
			}
			//arrow
			new(Lines) FLGUIHelperLineVertex((FVector3f)InControlPoints[3], InColor);
			new(Lines) FLGUIHelperLineVertex((FVector3f)InArrowPointA, InColor);
			new(Lines) FLGUIHelperLineVertex((FVector3f)InControlPoints[3], InColor);
			new(Lines) FLGUIHelperLineVertex((FVector3f)InArrowPointB, InColor);

			ViewExtension->AddLineRender(FLGUIHelperLineRenderParameter(Lines));
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

void ULGUIManagerWorldSubsystem::DrawNavigationVisualizerOnUISelectable(UWorld* InWorld, UUISelectableComponent* InSelectable, bool IsScreenSpace)
{
	auto SourceUIItem = InSelectable->GetRootUIComponent();
	if (!IsValid(SourceUIItem))return;
	const FColor Drawcall = ULGUIPrefabManagerObject::IsSelected(SourceUIItem->GetOwner()) ? FColor(255, 255, 0, 255) : FColor(140, 140, 0, 255);
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

void ULGUIManagerWorldSubsystem::DrawDebugBoxOnScreenSpace(UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color)
{
	auto ViewExtension = ULGUIManagerWorldSubsystem::GetViewExtension(InWorld, false);
	if (ViewExtension.IsValid())
	{
		TArray<FLGUIHelperLineVertex> Lines;

		FTransform const Transform(Rotation);
		FVector Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		FVector End = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		new(Lines)FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines)FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(-Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(-Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(-Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		ViewExtension->AddLineRender(FLGUIHelperLineRenderParameter(Lines));
	}
}
void ULGUIManagerWorldSubsystem::DrawDebugRectOnScreenSpace(UWorld* InWorld, FVector const& Center, FVector const& Box, const FQuat& Rotation, FColor const& Color)
{
	auto ViewExtension = ULGUIManagerWorldSubsystem::GetViewExtension(InWorld, false);
	if (ViewExtension.IsValid())
	{
		TArray<FLGUIHelperLineVertex> Lines;

		FTransform const Transform(Rotation);
		FVector Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		FVector End = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		Start = Transform.TransformPosition(FVector(Box.X, -Box.Y, Box.Z));
		End = Transform.TransformPosition(FVector(Box.X, -Box.Y, -Box.Z));
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + Start), Color);
		new(Lines) FLGUIHelperLineVertex(FVector3f(Center + End), Color);

		ViewExtension->AddLineRender(FLGUIHelperLineRenderParameter(Lines));
	}
}

bool ULGUIManagerWorldSubsystem::RaycastHitUI(UWorld* InWorld, const TArray<UUIItem*>& InUIItems, const FVector& LineStart, const FVector& LineEnd
	, UUIBaseRenderable*& ResultSelectTarget, int& InOutTargetIndexInHitArray
)
{
	TArray<FHitResult> HitResultArray;
	for (auto uiItem : InUIItems)
	{
		if (!IsValid(uiItem))continue;
		if (uiItem->GetWorld() == InWorld)
		{
			if (auto uiRenderable = Cast<UUIBaseRenderable>(uiItem))
			{
				if (uiRenderable->GetIsUIActiveInHierarchy() && uiRenderable->GetRenderCanvas() != nullptr)
				{
					FHitResult hitInfo;
					auto OriginRaycastType = uiRenderable->GetRaycastType();
					auto OriginRaycastTarget = uiRenderable->IsRaycastTarget();
					uiRenderable->SetRaycastType(EUIRenderableRaycastType::Mesh);//in editor selection, make the ray hit actural triangle
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

void ULGUIManagerWorldSubsystem::Initialize(FSubsystemCollectionBase& Collection)
{
	Super::Initialize(Collection);
#if WITH_EDITOR
	InstanceArray.Add(this);
	if (this->GetWorld()->WorldType == EWorldType::EditorPreview//EditorPreview world don't tick, so mannually tick it
		|| this->GetWorld()->WorldType == EWorldType::Editor)
	{
		EditorTickDelegateHandle = FTSTicker::GetCoreTicker().AddTicker(TEXT("LGUIManagerEditorTick"), 0, [WeakThis = MakeWeakObjectPtr(this)](float DeltaTime) {
			if (WeakThis.IsValid())
			{
				WeakThis->Tick(DeltaTime);
				return true;
			}
			return false;
			});
	}
	if (this->GetWorld()->IsGameWorld())
	{
		bIsPlaying = true;
	}
#endif
	//localization
	OnCultureChangedDelegateHandle = FInternationalization::Get().OnCultureChanged().AddUObject(this, &ULGUIManagerWorldSubsystem::OnCultureChanged);
}
void ULGUIManagerWorldSubsystem::PostInitialize()
{
	auto PrefabManager = ULGUIPrefabWorldSubsystem::GetInstance(this->GetWorld());
	check(PrefabManager);
	PrefabManager->OnBeginDeserializeSession.AddUObject(this, &ULGUIManagerWorldSubsystem::BeginPrefabSystemProcessingActor);
	PrefabManager->OnEndDeserializeSession.AddUObject(this, &ULGUIManagerWorldSubsystem::EndPrefabSystemProcessingActor);
}
void ULGUIManagerWorldSubsystem::Deinitialize()
{
	Super::Deinitialize();
#if WITH_EDITOR
	InstanceArray.Remove(this);
	if (EditorTickDelegateHandle.IsValid())
	{
		FTSTicker::GetCoreTicker().RemoveTicker(EditorTickDelegateHandle);
		EditorTickDelegateHandle.Reset();
	}
	if (this->GetWorld()->IsGameWorld())
	{
		bIsPlaying = false;
	}
#endif
	if (MainViewportViewExtension.IsValid())
	{
		MainViewportViewExtension.Reset();
	}
	if (OnCultureChangedDelegateHandle.IsValid())
	{
		FInternationalization::Get().OnCultureChanged().Remove(OnCultureChangedDelegateHandle);
	}
}
TStatId ULGUIManagerWorldSubsystem::GetStatId() const
{
	//return GetStatID();
	RETURN_QUICK_DECLARE_CYCLE_STAT(ULGUIManagerWorldSubsystem, STATGROUP_Tickables);
}
bool ULGUIManagerWorldSubsystem::IsTickableWhenPaused() const
{
	return true;
}

void ULGUIManagerWorldSubsystem::OnCultureChanged()
{
	bShouldUpdateOnCultureChanged = true;
}

ULGUIManagerWorldSubsystem* ULGUIManagerWorldSubsystem::GetInstance(UWorld* InWorld)
{
	if (FWorldContext* WorldContext = GEngine->GetWorldContextFromWorld(InWorld))
	{
		return InWorld->GetSubsystem<ULGUIManagerWorldSubsystem>();
	}
	return nullptr;
}

#if WITH_EDITOR
TArray<ULGUIManagerWorldSubsystem*> ULGUIManagerWorldSubsystem::InstanceArray;
bool ULGUIManagerWorldSubsystem::bIsPlaying = false;
#endif

DECLARE_CYCLE_STAT(TEXT("LGUILifeCycleBehaviour Update"), STAT_LGUILifeCycleBehaviourUpdate, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("LGUILifeCycleBehaviour Start"), STAT_LGUILifeCycleBehaviourStart, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("UpdateLayoutInterface"), STAT_UpdateLayoutInterface, STATGROUP_LGUI);
DECLARE_CYCLE_STAT(TEXT("Canvas Update"), STAT_UpdateCanvas, STATGROUP_LGUI);
void ULGUIManagerWorldSubsystem::Tick(float DeltaTime)
{
	//editor draw helper frame
#if WITH_EDITOR
	if (IsValid(GEditor))
	{
		auto Settings = GetDefault<ULGUIEditorSettings>();
		if (Settings->bDrawHelperFrame && GEditor->GetSelectedActorCount() > 0)
		{
			if (this->GetWorld()->WorldType == EWorldType::Game
				|| this->GetWorld()->WorldType == EWorldType::PIE
				|| this->GetWorld()->WorldType == EWorldType::Editor
				|| this->GetWorld()->WorldType == EWorldType::EditorPreview
				)
			{
				auto bIsGameWorld = this->GetWorld()->IsGameWorld();
				auto DrawFrame = [bIsGameWorld](const TArray<TWeakObjectPtr<ULGUICanvas>>& CanvasArray) {
					for (auto& CanvasItem : CanvasArray)
					{
						auto& UIItemArray = CanvasItem->GetUIItemArray();
						for (auto& UIItem : UIItemArray)
						{
							if (!IsValid(UIItem))continue;

							ULGUIManagerWorldSubsystem::DrawFrameOnUIItem(UIItem, bIsGameWorld ? UIItem->IsScreenSpaceOverlayUI() : false);
						}
					}
					};
				DrawFrame(ScreenSpaceCanvasArray);
				DrawFrame(WorldSpaceUECanvasArray);
				DrawFrame(WorldSpaceLGUICanvasArray);
				DrawFrame(RenderTargetSpaceLGUICanvasArray);
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

				ULGUIManagerWorldSubsystem::DrawNavigationVisualizerOnUISelectable(item->GetWorld(), item.Get(), this->GetWorld()->IsGameWorld() ? item->GetRootUIComponent()->IsScreenSpaceOverlayUI() : false);
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
				ILGUICultureChangedInterface::Execute_OnCultureChanged(item.Get());
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
		auto bIsGamePaused = GetWorld()->IsPaused();
		auto Settings = GetDefault<ULGUISettings>();
		SCOPE_CYCLE_COUNTER(STAT_LGUILifeCycleBehaviourUpdate);
		for (int i = 0; i < LGUILifeCycleBehavioursForUpdate.Num(); i++)
		{
			CurrentExecutingUpdateIndex = i;
			auto item = LGUILifeCycleBehavioursForUpdate[i];
			if (item.IsValid())
			{
				if (item->GetRootSceneComponent() && item->GetRootSceneComponent()->IsA(UUIItem::StaticClass()))
				{
					auto uiItem = (UUIItem*)item->GetRootSceneComponent();
					bool bAffectByGamePause;
					if (uiItem->IsScreenSpaceOverlayUI())
					{
						bAffectByGamePause = Settings->bScreenSpaceUIAffectByGamePause;
					}
					else
					{
						bAffectByGamePause = Settings->bWorldSpaceUIAffectByGamePause;
					}
					if (!bIsGamePaused || (bIsGamePaused && !bAffectByGamePause))
					{
						item->Update(DeltaTime);
					}
				}
				else
				{
					if (!bIsGamePaused || (bIsGamePaused && item->PrimaryComponentTick.bTickEvenWhenPaused))
					{
						item->Update(DeltaTime);
					}
				}
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
	for (auto& item : ScreenSpaceCanvasArray)
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
			auto errMsg = FText::Format(LOCTEXT("MultipleLGUICanvasRenderScreenSpaceOverlay", "[{0}].{1} Detect multiple LGUICanvas renderred with ScreenSpaceOverlay mode, this is not allowed! There should be only one ScreenSpace UI in a world!\
\n	World: {2}, type: {3}")
			, FText::FromString(ANSI_TO_TCHAR(__FUNCTION__)), __LINE__, FText::FromString(this->GetWorld()->GetPathName()), (int)(this->GetWorld()->WorldType));
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
		auto UpdateCanvas = [](TArray<TWeakObjectPtr<ULGUICanvas>>& InCanvasArray) {
			for (auto& item : InCanvasArray)
			{
				if (item.IsValid())
				{
					item->UpdateRootCanvas();
				}
			}
		};
		UpdateCanvas(ScreenSpaceCanvasArray);
		UpdateCanvas(WorldSpaceUECanvasArray);
		UpdateCanvas(WorldSpaceLGUICanvasArray);
		UpdateCanvas(RenderTargetSpaceLGUICanvasArray);
	}

	//sort render order
	{
		auto SortCanvas = [](TArray<TWeakObjectPtr<ULGUICanvas>>& InCanvasArray) {
			InCanvasArray.Sort([](const TWeakObjectPtr<ULGUICanvas>& A, const TWeakObjectPtr<ULGUICanvas>& B)
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
		};
		if (bShouldSortScreenSpaceCanvas)
		{
			bShouldSortScreenSpaceCanvas = false;
			SortCanvas(ScreenSpaceCanvasArray);
			SortDrawcallOnRenderMode(ELGUIRenderMode::ScreenSpaceOverlay, this->ScreenSpaceCanvasArray);
			if (MainViewportViewExtension.IsValid())
			{
				MainViewportViewExtension->MarkNeedToSortScreenSpacePrimitiveRenderPriority();
			}
		}
		if (bShouldSortWorldSpaceLGUICanvas)
		{
			bShouldSortWorldSpaceLGUICanvas = false;
			SortCanvas(WorldSpaceLGUICanvasArray);
			SortDrawcallOnRenderMode(ELGUIRenderMode::WorldSpace_LGUI, this->WorldSpaceLGUICanvasArray);
			if (MainViewportViewExtension.IsValid())
			{
				MainViewportViewExtension->MarkNeedToSortWorldSpacePrimitiveRenderPriority();
			}
		}
		if (bShouldSortWorldSpaceCanvas)
		{
			bShouldSortWorldSpaceCanvas = false;
			SortCanvas(WorldSpaceUECanvasArray);
			SortDrawcallOnRenderMode(ELGUIRenderMode::WorldSpace, this->WorldSpaceUECanvasArray);
		}
		if (bShouldSortRenderTargetSpaceCanvas)
		{
			bShouldSortRenderTargetSpaceCanvas = false;
			SortCanvas(RenderTargetSpaceLGUICanvasArray);
			SortDrawcallOnRenderMode(ELGUIRenderMode::RenderTarget, this->RenderTargetSpaceLGUICanvasArray);
		}
	}
}

void ULGUIManagerWorldSubsystem::SortDrawcallOnRenderMode(ELGUIRenderMode InRenderMode, const TArray<TWeakObjectPtr<ULGUICanvas>>& InCanvasArray)
{
	for (int i = 0; i < InCanvasArray.Num(); i++)
	{
		auto canvasItem = InCanvasArray[i];
		if (canvasItem.IsValid() && canvasItem->GetIsUIActive())
		{
			if (canvasItem->IsRootCanvas() || canvasItem->GetOverrideSorting())
			{
				canvasItem->SortDrawcall();
			}
		}
	}
}

void ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehaviourForLifecycleEvent(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			auto SessionId = ULGUIPrefabWorldSubsystem::GetInstance(InComp->GetWorld())->GetPrefabSystemSessionIdForActor(InComp->GetOwner());
			if (SessionId.IsValid())//processing by prefab system, collect for further operation
			{
				if (auto ArrayPtr = Instance->LGUILifeCycleBehaviours_PrefabSystemProcessing.Find(SessionId))
				{
					auto& CompArray = ArrayPtr->LGUILifeCycleBehaviourArray;
#if !UE_BUILD_SHIPPING
					if (CompArray.Contains(InComp))
					{
						UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
						FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
						return;
					}
#endif
					CompArray.Add(InComp);
				}
			}
			else//not processed by prefab system, just do immediately
			{
				ProcessLGUILifecycleEvent(InComp);
			}
		}
	}
}

void ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehavioursForUpdate(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			int32 index = INDEX_NONE;
			if (!Instance->LGUILifeCycleBehavioursForUpdate.Find(InComp, index))
			{
				Instance->LGUILifeCycleBehavioursForUpdate.Add(InComp);
				return;
			}
			UE_LOG(LGUI, Warning, TEXT("[ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehavioursForUpdate]Already exist, comp:%s"), *(InComp->GetPathName()));
		}
	}
}
void ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromUpdate(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
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
				UE_LOG(LGUI, Warning, TEXT("[ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromUpdate]Not exist, comp:%s"), *(InComp->GetPathName()));
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
				UE_LOG(LGUI, Warning, TEXT("[ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromUpdate]Cleanup %d invalid LGUILifeCycleBehaviour"), inValidCount);
			}
		}
	}
}

void ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehavioursForStart(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
		{
			int32 index = INDEX_NONE;
			if (!Instance->LGUILifeCycleBehavioursForStart.Find(InComp, index))
			{
				Instance->LGUILifeCycleBehavioursForStart.Add(InComp);
				return;
			}
			UE_LOG(LGUI, Warning, TEXT("[ULGUIManagerWorldSubsystem::AddLGUILifeCycleBehavioursForStart]Already exist, comp:%s"), *(InComp->GetPathName()));
		}
	}
}
void ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromStart(ULGUILifeCycleBehaviour* InComp)
{
	if (IsValid(InComp))
	{
		if (auto Instance = GetInstance(InComp->GetWorld()))
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
				UE_LOG(LGUI, Warning, TEXT("[ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromStart]Not exist, comp:%s"), *(InComp->GetPathName()));
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
				UE_LOG(LGUI, Warning, TEXT("[ULGUIManagerWorldSubsystem::RemoveLGUILifeCycleBehavioursFromStart]Cleanup %d invalid LGUILifeCycleBehaviour"), inValidCount);
			}
		}
	}
}

void ULGUIManagerWorldSubsystem::RegisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
		Instance->AllCultureChangedArray.AddUnique(InItem.GetObject());
	}
}
void ULGUIManagerWorldSubsystem::UnregisterLGUICultureChangedEvent(TScriptInterface<ILGUICultureChangedInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
		Instance->AllCultureChangedArray.RemoveSingle(InItem.GetObject());
	}
}

void ULGUIManagerWorldSubsystem::UpdateLayout()
{
	SCOPE_CYCLE_COUNTER(STAT_UpdateLayoutInterface);

	//update Layout
	if (bNeedUpdateLayout)
	{
		bNeedUpdateLayout = false;
		for (auto& RootUIItem : AllRootUIItemArray)
		{
			if (RootUIItem.IsValid())
			{
				RebuildLayout(RootUIItem.Get());
			}
		}
	}
}
void ULGUIManagerWorldSubsystem::ForceUpdateLayout(UObject* WorldContextObject)
{
	if (auto world = GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::LogAndReturnNull))
	{
		if (auto Instance = GetInstance(world))
		{
			Instance->UpdateLayout();
		}
	}
}
void ULGUIManagerWorldSubsystem::RebuildLayout(UUIItem* InItem)
{
	auto& Children = InItem->GetAttachUIChildren();
	for (auto& Child : Children)
	{
		RebuildLayout(Child);
	}

	auto& Components = InItem->GetOwner()->GetComponents();
	for (auto Component : Components)
	{
		if (Component && Component->GetClass()->ImplementsInterface(ULGUILayoutInterface::StaticClass()))
		{
			ILGUILayoutInterface::Execute_OnUpdateLayout(Component);
		}
	}
}

#if WITH_EDITOR
void ULGUIManagerWorldSubsystem::RefreshAllUI(UWorld* InWorld)
{
	struct LOCAL
	{
		static void MarkRebuildLayoutRecursive(UUIItem* InUIItem)
		{
			auto& Children = InUIItem->GetAttachUIChildren();
			for (auto& Child : Children)
			{
				RebuildLayout(Child);
			}
			if (auto LayoutComp = InUIItem->GetOwner()->FindComponentByInterface(ULGUILayoutInterface::StaticClass()))
			{
				ILGUILayoutInterface::Execute_MarkRebuildLayout(LayoutComp);
			}
		}
	};

	for (auto InstanceItem : InstanceArray)
	{
		if (InstanceItem != nullptr)
		{
			if (InWorld != nullptr && InstanceItem->GetWorld() != InWorld)
			{
				continue;
			}
		}
		auto Instance = InstanceItem;
		for (auto& RootUIItem : Instance->AllRootUIItemArray)
		{
			LOCAL::MarkRebuildLayoutRecursive(RootUIItem.Get());
		}
		for (auto& RootUIItem : Instance->AllRootUIItemArray)
		{
			if (RootUIItem.IsValid())
			{
				RootUIItem->EnsureDataForRebuild();
				RootUIItem->EditorForceUpdate();
			}
		}
	}
}
#endif

void ULGUIManagerWorldSubsystem::AddRootUIItem(UUIItem* InItem)
{
	if (auto Instance = GetInstance(InItem->GetWorld()))
	{
#if !UE_BUILD_SHIPPING
		if (Instance->AllRootUIItemArray.Contains(InItem))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
#endif
		Instance->AllRootUIItemArray.AddUnique(InItem);
	}
}
void ULGUIManagerWorldSubsystem::RemoveRootUIItem(UUIItem* InItem)
{
	if (auto Instance = GetInstance(InItem->GetWorld()))
	{
#if !UE_BUILD_SHIPPING
		if (!Instance->AllRootUIItemArray.Contains(InItem))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
#endif
		Instance->AllRootUIItemArray.RemoveSingle(InItem);
	}
}

void ULGUIManagerWorldSubsystem::AddCanvas(ULGUICanvas* InCanvas, ELGUIRenderMode InCurrentRenderMode)
{
	if (auto Instance = GetInstance(InCanvas->GetWorld()))
	{
		if (InCurrentRenderMode != ELGUIRenderMode::None)//none means canvas's property not ready yet, so no need to collect it, because it will be collected in "CanvasRenderModeChange" function
		{
			switch (InCurrentRenderMode)
			{
			case ELGUIRenderMode::ScreenSpaceOverlay:
				Instance->ScreenSpaceCanvasArray.Add(InCanvas);
				break;
			case ELGUIRenderMode::WorldSpace:
				Instance->WorldSpaceUECanvasArray.Add(InCanvas);
				break;
			case ELGUIRenderMode::WorldSpace_LGUI:
				Instance->WorldSpaceLGUICanvasArray.Add(InCanvas);
				break;
			case ELGUIRenderMode::RenderTarget:
				Instance->RenderTargetSpaceLGUICanvasArray.Add(InCanvas);
				break;
			}
		}
	}
}
void ULGUIManagerWorldSubsystem::RemoveCanvas(ULGUICanvas* InCanvas, ELGUIRenderMode InCurrentRenderMode)
{
	if (auto Instance = GetInstance(InCanvas->GetWorld()))
	{
		switch (InCurrentRenderMode)
		{
		case ELGUIRenderMode::ScreenSpaceOverlay:
			Instance->ScreenSpaceCanvasArray.Remove(InCanvas);
			break;
		case ELGUIRenderMode::WorldSpace:
			Instance->WorldSpaceUECanvasArray.Remove(InCanvas);
			break;
		case ELGUIRenderMode::WorldSpace_LGUI:
			Instance->WorldSpaceLGUICanvasArray.Remove(InCanvas);
			break;
		case ELGUIRenderMode::RenderTarget:
			Instance->RenderTargetSpaceLGUICanvasArray.Remove(InCanvas);
			break;
		}
	}
}
void ULGUIManagerWorldSubsystem::CanvasRenderModeChange(ULGUICanvas* InCanvas, ELGUIRenderMode InOldRenderMode, ELGUIRenderMode InNewRenderMode)
{
	if (auto Instance = GetInstance(InCanvas->GetWorld()))
	{
		//remove from old
		switch (InOldRenderMode)
		{
		case ELGUIRenderMode::ScreenSpaceOverlay:
			Instance->ScreenSpaceCanvasArray.Remove(InCanvas);
			break;
		case ELGUIRenderMode::WorldSpace:
			Instance->WorldSpaceUECanvasArray.Remove(InCanvas);
			break;
		case ELGUIRenderMode::WorldSpace_LGUI:
			Instance->WorldSpaceLGUICanvasArray.Remove(InCanvas);
			break;
		case ELGUIRenderMode::RenderTarget:
			Instance->RenderTargetSpaceLGUICanvasArray.Remove(InCanvas);
			break;
		}
		//add to new
		switch (InNewRenderMode)
		{
		case ELGUIRenderMode::ScreenSpaceOverlay:
			Instance->ScreenSpaceCanvasArray.Add(InCanvas);
			break;
		case ELGUIRenderMode::WorldSpace:
			Instance->WorldSpaceUECanvasArray.Add(InCanvas);
			break;
		case ELGUIRenderMode::WorldSpace_LGUI:
			Instance->WorldSpaceLGUICanvasArray.Add(InCanvas);
			break;
		case ELGUIRenderMode::RenderTarget:
			Instance->RenderTargetSpaceLGUICanvasArray.Add(InCanvas);
			break;
		}
	}
}
const TArray<TWeakObjectPtr<ULGUICanvas>>& ULGUIManagerWorldSubsystem::GetCanvasArray(ELGUIRenderMode RenderMode)
{
	switch (RenderMode)
	{
	default://this should not happen
#if !UE_BUILD_SHIPPING
		UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
		FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
#endif
		return ScreenSpaceCanvasArray;
	case ELGUIRenderMode::ScreenSpaceOverlay:
		return ScreenSpaceCanvasArray;
	case ELGUIRenderMode::WorldSpace:
		return WorldSpaceUECanvasArray;
	case ELGUIRenderMode::WorldSpace_LGUI:
		return WorldSpaceLGUICanvasArray;
	case ELGUIRenderMode::RenderTarget:
		return RenderTargetSpaceLGUICanvasArray;
	}
}
void ULGUIManagerWorldSubsystem::MarkSortScreenSpaceCanvas()
{
	bShouldSortScreenSpaceCanvas = true;
}
void ULGUIManagerWorldSubsystem::MarkSortWorldSpaceLGUICanvas()
{
	bShouldSortWorldSpaceLGUICanvas = true;
}
void ULGUIManagerWorldSubsystem::MarkSortWorldSpaceCanvas()
{
	bShouldSortWorldSpaceCanvas = true;
}
void ULGUIManagerWorldSubsystem::MarkSortRenderTargetSpaceCanvas()
{
	bShouldSortRenderTargetSpaceCanvas = true;
}

TSharedPtr<class FLGUIRenderer, ESPMode::ThreadSafe> ULGUIManagerWorldSubsystem::GetViewExtension(UWorld* InWorld, bool InCreateIfNotExist)
{
	if (auto Instance = GetInstance(InWorld))
	{
		if (!Instance->MainViewportViewExtension.IsValid())
		{
			if (InCreateIfNotExist)
			{
				Instance->MainViewportViewExtension = FSceneViewExtensions::NewExtension<FLGUIRenderer>(InWorld, ELGUIRendererType::ScreenSpace_and_WorldSpace);
			}
		}
		return Instance->MainViewportViewExtension;
	}
	return nullptr;
}

void ULGUIManagerWorldSubsystem::AddRaycaster(ULGUIBaseRaycaster* InRaycaster)
{
	if (auto Instance = GetInstance(InRaycaster->GetWorld()))
	{
		auto& AllRaycasterArray = Instance->AllRaycasterArray;
		if (AllRaycasterArray.Contains(InRaycaster))return;
		//check multiple racaster
		for (auto& item : AllRaycasterArray)
		{
			if (InRaycaster->GetDepth() == item->GetDepth() && InRaycaster->GetTraceChannel() == item->GetTraceChannel())
			{
#if WITH_EDITOR
				auto ErrorNotifyMsg = LOCTEXT("MultipleLGUIBaseRaycasterWithSameDepthAndTraceChannel"
					, "Detect multiple LGUIBaseRaycaster components with same depth and traceChannel, this may cause wrong interaction results! See output log for details.");
				LGUIUtils::EditorNotification(ErrorNotifyMsg, 10);
#endif
				UE_LOG(LGUI, Warning, TEXT("[%s].%d \
\nDetect multiple LGUIBaseRaycaster components with same depth and traceChannel, this may cause wrong interaction results!\
\neg: Want use mouse to click object A but get object B.\
\nPlease note : \
\n	For LGUIBaseRaycasters with same depth, LGUI will line trace them all and sort result on hit distance.\
\n	For LGUIBaseRaycasters with different depth, LGUI will sort raycasters on depth, and line trace from highest depth to lowest, if hit anything then stop line trace.\
\n	LGUIWorldSpaceInteraction is for all WorldSpaceUI in current level.\
"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);

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
void ULGUIManagerWorldSubsystem::RemoveRaycaster(ULGUIBaseRaycaster* InRaycaster)
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

void ULGUIManagerWorldSubsystem::SetCurrentInputModule(ULGUIBaseInputModule* InInputModule)
{
	if (auto Instance = GetInstance(InInputModule->GetWorld()))
	{
		Instance->CurrentInputModule = InInputModule;
	}
}
void ULGUIManagerWorldSubsystem::ClearCurrentInputModule(ULGUIBaseInputModule* InInputModule)
{
	if (auto Instance = GetInstance(InInputModule->GetWorld()))
	{
		Instance->CurrentInputModule = nullptr;
	}
}

void ULGUIManagerWorldSubsystem::AddSelectable(UUISelectableComponent* InSelectable)
{
	if (auto Instance = GetInstance(InSelectable->GetWorld()))
	{
		auto& AllSelectableArray = Instance->AllSelectableArray;
#if !UE_BUILD_SHIPPING
		if (AllSelectableArray.Contains(InSelectable))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
#endif
		AllSelectableArray.AddUnique(InSelectable);
	}
}
void ULGUIManagerWorldSubsystem::RemoveSelectable(UUISelectableComponent* InSelectable)
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
void ULGUIManagerWorldSubsystem::RegisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
#if !UE_BUILD_SHIPPING
		if (Instance->AllLayoutArray.Contains(InItem.GetObject()))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
#endif
		Instance->AllLayoutArray.AddUnique(InItem.GetObject());
	}
}
void ULGUIManagerWorldSubsystem::UnregisterLGUILayout(TScriptInterface<ILGUILayoutInterface> InItem)
{
	if (auto Instance = GetInstance(InItem.GetObject()->GetWorld()))
	{
#if !UE_BUILD_SHIPPING
		if (!Instance->AllLayoutArray.Contains(InItem.GetObject()))
		{
			UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			FDebug::DumpStackTraceToLog(ELogVerbosity::Warning);
		}
#endif
		Instance->AllLayoutArray.RemoveSingle(InItem.GetObject());
	}
}
void ULGUIManagerWorldSubsystem::MarkUpdateLayout(UWorld* InWorld)
{
	if (auto Instance = GetInstance(InWorld))
	{
		Instance->bNeedUpdateLayout = true;
	}
}


void ULGUIManagerWorldSubsystem::ProcessLGUILifecycleEvent(ULGUILifeCycleBehaviour* InComp)
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
void ULGUIManagerWorldSubsystem::BeginPrefabSystemProcessingActor(const FGuid& InSessionId)
{
	FLGUILifeCycleBehaviourArrayContainer Container;
	LGUILifeCycleBehaviours_PrefabSystemProcessing.Add(InSessionId, Container);
}
void ULGUIManagerWorldSubsystem::EndPrefabSystemProcessingActor(const FGuid& InSessionId)
{
	if (auto ArrayPtr = LGUILifeCycleBehaviours_PrefabSystemProcessing.Find(InSessionId))
	{
		auto& LateFunctions = ArrayPtr->Functions;
		for (auto& Function : LateFunctions)
		{
			Function();
		}

		auto& LGUILifeCycleBehaviourArray = ArrayPtr->LGUILifeCycleBehaviourArray;
		auto Count = LGUILifeCycleBehaviourArray.Num();
		for (int i = 0; i < Count; i++)
		{
			auto& Item = LGUILifeCycleBehaviourArray[i];
			if (Item.IsValid())
			{
				ProcessLGUILifecycleEvent(Item.Get());
			}
#if !UE_BUILD_SHIPPING
			if (LGUILifeCycleBehaviourArray.Num() != Count)
			{
				UE_LOG(LGUI, Error, TEXT("[%s].%d break here for debug"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__);
			}
#endif
		}

		LGUILifeCycleBehaviours_PrefabSystemProcessing.Remove(InSessionId);
	}
}
void ULGUIManagerWorldSubsystem::AddFunctionForPrefabSystemExecutionBeforeAwake(AActor* InPrefabActor, const TFunction<void()>& InFunction)
{
	auto SessionId = ULGUIPrefabWorldSubsystem::GetInstance(InPrefabActor->GetWorld())->GetPrefabSystemSessionIdForActor(InPrefabActor);
	if (SessionId.IsValid())
	{
		auto& Container = LGUILifeCycleBehaviours_PrefabSystemProcessing[SessionId];
		Container.Functions.Add(InFunction);
	}
}
#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
#undef LOCTEXT_NAMESPACE