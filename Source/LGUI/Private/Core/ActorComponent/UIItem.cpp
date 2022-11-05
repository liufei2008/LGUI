﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIItem.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UICanvasGroup.h"
#include "Core/LGUISettings.h"
#include "Core/LGUILifeCycleUIBehaviour.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PhysicsEngine/BodySetup.h"
#include "Layout/LGUICanvasScaler.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#include "EditorViewportClient.h"
#include "UObject/UnrealType.h"
#endif

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#if WITH_EDITORONLY_DATA
TSet<FName> UUIItem::PersistentOverridePropertyNameSet =
{
	GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex)
};
#endif

UUIItem::UUIItem(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetMobility(EComponentMobility::Movable);
	SetUsingAbsoluteLocation(false);
	SetUsingAbsoluteRotation(false);
	SetUsingAbsoluteScale(false);
	SetVisibility(false);
	bCanSetAnchorFromTransform = false;//skip construction

	bWantsOnUpdateTransform = true;
	bFlattenHierarchyIndexDirty = true;
	bWidthCached = false;
	bHeightCached = false;
	bAnchorLeftCached = false;
	bAnchorRightCached = false;
	bAnchorBottomCached = false;
	bAnchorTopCached = false;

	bIsCanvasUIItem = false;
}

void UUIItem::BeginPlay()
{
	Super::BeginPlay();
}

void UUIItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

#pragma region LGUILifeCycleUIBehaviour
void UUIItem::CallUILifeCycleBehavioursActiveInHierarchyStateChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	bool TempIsUIActive = GetIsUIActiveInHierarchy();
	OnUIActiveInHierachy(TempIsUIActive);
	if (this->GetOwner()->GetRootComponent() != this)return;
	if (UIActiveInHierarchyStateChangedDelegate.IsBound())UIActiveInHierarchyStateChangedDelegate.Broadcast(TempIsUIActive);
}
void UUIItem::CallUILifeCycleBehavioursChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
	}
}
void UUIItem::CallUILifeCycleBehavioursChildActiveInHierarchyStateChanged(UUIItem* child, bool activeOrInactive)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildAcitveInHierarchy(child, activeOrInactive);
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIChildAcitveInHierarchy(child, activeOrInactive);
	}
}
void UUIItem::CallUILifeCycleBehavioursDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIDimensionsChanged(positionChanged, sizeChanged);
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIDimensionsChanged(positionChanged, sizeChanged);
	}

	//call parent
	if (ParentUIItem.IsValid())
	{
		ParentUIItem->CallUILifeCycleBehavioursChildDimensionsChanged(this, positionChanged, sizeChanged);
	}
}
void UUIItem::CallUILifeCycleBehavioursAttachmentChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIAttachmentChanged();
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIAttachmentChanged();
	}
}
void UUIItem::CallUILifeCycleBehavioursChildAttachmentChanged(UUIItem* child, bool attachOrDettach)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildAttachmentChanged(child, attachOrDettach);
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIChildAttachmentChanged(child, attachOrDettach);
	}
}
void UUIItem::CallUILifeCycleBehavioursInteractionStateChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	auto interactable = this->IsGroupAllowInteraction();
	OnUIInteractionStateChanged(interactable);
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIInteractionStateChanged(interactable);
	}
}
void UUIItem::CallUILifeCycleBehavioursChildHierarchyIndexChanged(UUIItem* child)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildHierarchyIndexChanged(child);
	if (this->GetOwner()->GetRootComponent() != this)return;
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUILifeCycleUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUILifeCycleUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUILifeCycleUIBehaviourArray[i];
		CompItem->Call_OnUIChildHierarchyIndexChanged(child);
	}
}
#pragma endregion LGUILifeCycleUIBehaviour


void UUIItem::CalculateFlattenHierarchyIndex_Recursive(int& index)const
{
	if (this->flattenHierarchyIndex != index)
	{
		this->flattenHierarchyIndex = index;
	}
	for (auto& child : UIChildren)
	{
		if (IsValid(child))
		{
			index++;
			child->CalculateFlattenHierarchyIndex_Recursive(index);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIItem CalculateFlattenHierarchyIndex"), STAT_UIItemCalculateFlattenHierarchyIndex, STATGROUP_LGUI);
void UUIItem::RecalculateFlattenHierarchyIndex()const
{
	SCOPE_CYCLE_COUNTER(STAT_UIItemCalculateFlattenHierarchyIndex);

	this->bFlattenHierarchyIndexDirty = false;
	int tempIndex = this->flattenHierarchyIndex;
	this->CalculateFlattenHierarchyIndex_Recursive(tempIndex);
}

int32 UUIItem::GetFlattenHierarchyIndex()const
{
	if (RootUIItem.IsValid())
	{
		if (RootUIItem->bFlattenHierarchyIndexDirty)
		{
			RootUIItem->RecalculateFlattenHierarchyIndex();
		}
	}
	return this->flattenHierarchyIndex;
}

void UUIItem::MarkFlattenHierarchyIndexDirty()
{
	if (RootUIItem.IsValid())
	{
		RootUIItem->bFlattenHierarchyIndexDirty = true;
	}
	//tell canvas to update
	if (RenderCanvas.IsValid()
		&& RenderCanvas->IsRegistered()//@todo: why need to check IsRegistered? the only way to set RenderCanvas is SetRenderCanvas function, but when I debug on SetRenderCanvas it's not called at all, so RenderCanvas should not valid here, no clue yet
		)
	{
		RenderCanvas->MarkCanvasUpdate(false, false, true);
	}
}

void UUIItem::SetHierarchyIndex(int32 InInt) 
{ 
	if (InInt != hierarchyIndex)
	{
		hierarchyIndex = InInt;
		ApplyHierarchyIndex();
	}
}

void UUIItem::ApplyHierarchyIndex()
{
	if (ParentUIItem.IsValid())
	{
		if (ParentUIItem->UIChildren.Num() == 0)
		{
			ParentUIItem->UIChildren.Add(this);
			this->hierarchyIndex = 0;
			ParentUIItem->CallUILifeCycleBehavioursChildHierarchyIndexChanged(this);
		}
		else
		{
			ParentUIItem->CheckCacheUIChildren();
			hierarchyIndex = FMath::Clamp(hierarchyIndex, 0, ParentUIItem->UIChildren.Num() - 1);
			ParentUIItem->UIChildren.Remove(this);
			ParentUIItem->UIChildren.Insert(this, hierarchyIndex);
			bool anythingChange = false;
			for (int i = 0; i < ParentUIItem->UIChildren.Num(); i++)
			{
				if (ParentUIItem->UIChildren[i]->hierarchyIndex != i)
				{
					ParentUIItem->UIChildren[i]->hierarchyIndex = i;
					anythingChange = true;
				}
			}
			//flatten hierarchy index
			if (anythingChange)
			{
				MarkFlattenHierarchyIndexDirty();
				ParentUIItem->CallUILifeCycleBehavioursChildHierarchyIndexChanged(this);
			}
		}
	}
	else
	{
		hierarchyIndex = 0;
	}
}

void UUIItem::SetAsFirstHierarchy()
{
	SetHierarchyIndex(-1);
}
void UUIItem::SetAsLastHierarchy()
{
	if (ParentUIItem.IsValid())
	{
		SetHierarchyIndex(ParentUIItem->GetAttachUIChildren().Num());
	}
}

UUIItem* UUIItem::FindChildByDisplayName(const FString& InName, bool IncludeChildren)const
{
	int indexOfFirstSlash;
	if (InName.FindChar('/', indexOfFirstSlash))
	{
		auto firstLayerName = InName.Left(indexOfFirstSlash);
		for (auto& childItem : UIChildren)
		{
			if (childItem->displayName.Equals(firstLayerName, ESearchCase::CaseSensitive))
			{
				auto restName = InName.Right(InName.Len() - indexOfFirstSlash - 1);
				return childItem->FindChildByDisplayName(restName);
			}
		}
	}
	else
	{
		if (IncludeChildren)
		{
			return FindChildByDisplayNameWithChildren_Internal(InName);
		}
		else
		{
			for (auto& childItem : UIChildren)
			{
				if (childItem->displayName.Equals(InName, ESearchCase::CaseSensitive))
				{
					return childItem;
				}
			}
		}
	}
	return nullptr;
}
UUIItem* UUIItem::FindChildByDisplayNameWithChildren_Internal(const FString& InName)const
{
	for (auto& childItem : UIChildren)
	{
		if (childItem->displayName.Equals(InName, ESearchCase::CaseSensitive))
		{
			return childItem;
		}
		else
		{
			auto result = childItem->FindChildByDisplayNameWithChildren_Internal(InName);
			if (result)
			{
				return result;
			}
		}
	}
	return nullptr;
}
TArray<UUIItem*> UUIItem::FindChildArrayByDisplayName(const FString& InName, bool IncludeChildren)const
{
	TArray<UUIItem*> resultArray;
	int indexOfLastSlash;
	if (InName.FindLastChar('/', indexOfLastSlash))
	{
		auto parentLayerName = InName.Left(indexOfLastSlash);
		auto parentItem = this->FindChildByDisplayName(parentLayerName, false);
		if (IsValid(parentItem))
		{
			auto matchName = InName.Right(InName.Len() - indexOfLastSlash - 1);
			return parentItem->FindChildArrayByDisplayName(matchName, IncludeChildren);
		}
	}
	else
	{
		if (IncludeChildren)
		{
			FindChildArrayByDisplayNameWithChildren_Internal(InName, resultArray);
		}
		else
		{
			for (auto& childItem : UIChildren)
			{
				if (childItem->displayName.Equals(InName, ESearchCase::CaseSensitive))
				{
					resultArray.Add(childItem);
				}
			}
		}
	}
	return resultArray;
}
void UUIItem::FindChildArrayByDisplayNameWithChildren_Internal(const FString& InName, TArray<UUIItem*>& OutResultArray)const
{
	for (auto& childItem : UIChildren)
	{
		if (childItem->displayName.Equals(InName, ESearchCase::CaseSensitive))
		{
			OutResultArray.Add(childItem);
		}
		else
		{
			childItem->FindChildArrayByDisplayNameWithChildren_Internal(InName, OutResultArray);
		}
	}
}

void UUIItem::MarkAllDirtyRecursive()
{
	MarkAllDirty();
	
	for (auto& uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{
			uiChild->MarkAllDirtyRecursive();
		}
	}
}

void UUIItem::MarkAllDirty()
{
	bFlattenHierarchyIndexDirty = true;
	bWidthCached = false;
	bHeightCached = false;
	bAnchorLeftCached = false;
	bAnchorRightCached = false;
	bAnchorTopCached = false;
	bAnchorBottomCached = false;
}

void UUIItem::MarkRenderModeChangeRecursive(ULGUICanvas* Canvas, ELGUIRenderMode OldRenderMode, ELGUIRenderMode NewRenderMode)
{
	if (this->RenderCanvas == Canvas)
	{
		MarkAllDirty();
		for (auto& uiChild : UIChildren)
		{
			if (IsValid(uiChild))
			{
				uiChild->MarkRenderModeChangeRecursive(Canvas, OldRenderMode, NewRenderMode);
			}
		}
	}
}

void UUIItem::ForceRefreshRenderCanvasRecursive()
{
	if (RenderCanvas.IsValid())
	{
		OnRenderCanvasChanged(RenderCanvas.Get(), RenderCanvas.Get());
	}

	for (auto& uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{
			uiChild->ForceRefreshRenderCanvasRecursive();
		}
	}
}

void UUIItem::PostLoad()
{
	Super::PostLoad();
}

#if WITH_EDITOR
void UUIItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		MarkAllDirtyRecursive();
		auto PropertyName = PropertyChangedEvent.Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive))
		{
			bIsUIActive = !bIsUIActive;//make it work
			SetIsUIActive(!bIsUIActive);
		}

		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex))
		{
			ApplyHierarchyIndex();
		}
		else if (PropertyName == FName(TEXT("RelativeLocation")))
		{
			CalculateAnchorFromTransform();
			UpdateComponentToWorld();
		}
		else if (PropertyName == FName(TEXT("AnchorData")))
		{
			CalculateTransformFromAnchor();
			UpdateComponentToWorld();
		}
		EditorForceUpdate();
		UpdateBounds();
	}
}

void UUIItem::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	EditorForceUpdate();
}

void UUIItem::PostEditUndo()
{
	CheckCacheUIChildren();
	Super::PostEditUndo();
	ApplyHierarchyIndex();
	CheckUIActiveState();

	SetOnAnchorChange(true, true);

#if WITH_EDITOR
	//Renew render canvas, so add UIBaseRenderable will not exist in wrong canvas
	if (ULGUIEditorManagerObject::Instance != nullptr)
	{
		for (auto TempRootUIItem : ULGUIEditorManagerObject::Instance->GetAllRootUIItemArray())
		{
			auto OldRenderCanvas = TempRootUIItem->RenderCanvas;
			TempRootUIItem->RenderCanvas = nullptr;
			TempRootUIItem->RenewRenderCanvasRecursive(OldRenderCanvas.Get());
		}
	}
	ULGUIEditorManagerObject::RefreshAllUI();
#endif
}

//void UUIItem::PostEditUndo(TSharedPtr<ITransactionObjectAnnotation> TransactionAnnotation)
//{
//	Super::PostEditUndo(TransactionAnnotation);
//	EditorForceUpdate();
//}

void UUIItem::PostTransacted(const FTransactionObjectEvent& TransactionEvent)
{
	Super::PostTransacted(TransactionEvent);
}

FBoxSphereBounds UUIItem::CalcBounds(const FTransform& LocalToWorld) const
{
	auto Center = this->GetLocalSpaceCenter();
	auto Origin = FVector(0, Center.X, Center.Y);
	return FBoxSphereBounds(Origin, FVector(1, this->GetWidth() * 0.5f, this->GetHeight() * 0.5f), (this->GetWidth() > this->GetHeight() ? this->GetWidth() : this->GetHeight()) * 0.5f).TransformBy(LocalToWorld);
}
void UUIItem::EditorForceUpdate()
{
	MarkCanvasUpdate(true, true, true, true);
}
#endif

bool UUIItem::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{
	auto result = Super::MoveComponentImpl(Delta, NewRotation, bSweep, Hit, MoveFlags, Teleport);
	if (bCanSetAnchorFromTransform
		&& this->IsRegistered()//check if registerred, because it may called from reconstruction.
		)
	{
		CalculateAnchorFromTransform();
	}
	return result;
}
void UUIItem::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
}
void UUIItem::CalculateAnchorFromTransform()
{
#if WITH_EDITOR
	if (this->GetWorld() && !this->GetWorld()->IsGameWorld())
	{
		if (!ALGUIManagerActor::GetIsPlaying(this->GetWorld()))
		{
			if (!GetDefault<ULGUIEditorSettings>()->AnchorControlPosition)
			{
				return;
			}
		}
	}
#endif
	auto TempRelativeLocation = this->GetRelativeLocation();
	FVector2D CalculatedAnchoredPosition;
	if (ParentUIItem.IsValid())
	{
		//just a reverse operation from CalculateTransformFromAnchor
		float LocalLeftPoint =
			ParentUIItem->GetLocalSpaceLeft()
			+ (ParentUIItem->GetWidth() * this->AnchorData.AnchorMin.X);

		float LocalBottomPoint =
			ParentUIItem->GetLocalSpaceBottom()
			+ (ParentUIItem->GetHeight() * this->AnchorData.AnchorMin.Y);

		CalculatedAnchoredPosition.X = TempRelativeLocation.Y
			- LocalLeftPoint
			- +(ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X)) * this->AnchorData.Pivot.X;
		CalculatedAnchoredPosition.Y = TempRelativeLocation.Z
			- LocalBottomPoint
			- (ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y)) * this->AnchorData.Pivot.Y;
	}
	else
	{
		CalculatedAnchoredPosition.X = TempRelativeLocation.Y;
		CalculatedAnchoredPosition.Y = TempRelativeLocation.Z;
	}

	AnchorData.AnchoredPosition = CalculatedAnchoredPosition;

	bAnchorLeftCached = false;
	bAnchorRightCached = false;
	bAnchorBottomCached = false;
	bAnchorTopCached = false;
	SetOnTransformChange();
}

void UUIItem::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;
	if (UUIItem* childUIItem = Cast<UUIItem>(ChildComponent))
	{
		childUIItem->OnUIAttachedToParent();
		//hierarchy index
		CheckCacheUIChildren();//check
		bool bNeedSortChildren = true;
#if WITH_EDITORONLY_DATA
		if (!GetWorld()->IsGameWorld())
		{
			if (ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(this->GetOwner()))//load from prefab or duplicated by LGUI PrefabSystem, then not set hierarchy index
			{
				bNeedSortChildren = false;//if is load from prefab system, then we don't need to sort children, because children is already sorted when save prefab
			}
			else
			{
				if (childUIItem->IsRegistered())
				{
					childUIItem->hierarchyIndex = UIChildren.Num();
					this->CallUILifeCycleBehavioursChildHierarchyIndexChanged(childUIItem);
				}
				else//not registered means is loading from level. then no need to set hierarchy index
				{

				}
			}
		}
		else
#endif
		{
			auto ManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetOwner());
			if (ManagerActor && ManagerActor->IsPrefabSystemProcessingActor(this->GetOwner()))//load from prefab or duplicated by LGUI PrefabSystem, then not set hierarchy index
			{
				bNeedSortChildren = false;//if is load from prefab system, then we don't need to sort children, because children is already sorted when save prefab
			}
			else
			{
				if (childUIItem->IsRegistered())
				{
					childUIItem->hierarchyIndex = UIChildren.Num();
					this->CallUILifeCycleBehavioursChildHierarchyIndexChanged(childUIItem);
				}
				else//not registered means is loading from level. then no need to set hierarchy index
				{

				}
			}
		}
		UIChildren.Add(childUIItem);
		//make sure hierarchyindex all good
		if (childUIItem->hierarchyIndex == INDEX_NONE)
		{
			for (int i = 0; i < UIChildren.Num(); i++)
			{
				auto& UIChild = UIChildren[i];
				if (UIChild->hierarchyIndex != i)
				{
					UIChild->hierarchyIndex = i;
					this->CallUILifeCycleBehavioursChildHierarchyIndexChanged(UIChild);
				}
			}
		}

		if (bNeedSortChildren)
		{
			UIChildren.Sort([](const UUIItem& A, const UUIItem& B)
				{
					if (A.GetHierarchyIndex() < B.GetHierarchyIndex())
						return true;
					return false;
				});
		}
		MarkCanvasUpdate(false, false, false);
	}
}

void UUIItem::OnUIAttachedToParent()
{
#if WITH_EDITORONLY_DATA
	if (!GetWorld()->IsGameWorld())
	{
		if (ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(this->GetOwner()))//when load from prefab or duplicate from LGUICopier, the ChildAttachmentChanged callback should execute til prefab serialization ready
		{
			ParentUIItem = Cast<UUIItem>(this->GetAttachParent());
			check(ParentUIItem.IsValid());
			{
				ULGUIEditorManagerObject::AddFunctionForPrefabSystemExecutionBeforeAwake(this->GetOwner(), [Child = MakeWeakObjectPtr(this), Parent = ParentUIItem]() {
					if (Child.IsValid() && Parent.IsValid())
					{
						Parent->CallUILifeCycleBehavioursChildAttachmentChanged(Child.Get(), true);
					}});
			}
		}
		else
		{
			ParentUIItem = Cast<UUIItem>(this->GetAttachParent());
			check(ParentUIItem.IsValid());
			{
				ParentUIItem->CallUILifeCycleBehavioursChildAttachmentChanged(this, true);
			}

			if (this->IsRegistered())//not registered means is loading from level.
			{
				this->CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
			}
		}
	}
	else
#endif
	{
		auto ManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetOwner());
		if (ManagerActor && ManagerActor->IsPrefabSystemProcessingActor(this->GetOwner()))//when load from prefab or duplicate from LGUICopier, the ChildAttachmentChanged callback should execute til prefab serialization ready
		{
			ParentUIItem = Cast<UUIItem>(this->GetAttachParent());
			check(ParentUIItem.IsValid());
			{
				ManagerActor->AddFunctionForPrefabSystemExecutionBeforeAwake(this->GetOwner(), [Child = MakeWeakObjectPtr(this), Parent = ParentUIItem]() {
					if (Child.IsValid() && Parent.IsValid())
					{
						Parent->CallUILifeCycleBehavioursChildAttachmentChanged(Child.Get(), true);
					}});
			}
		}
		else
		{
			ParentUIItem = Cast<UUIItem>(this->GetAttachParent());
			check(ParentUIItem.IsValid());
			{
				ParentUIItem->CallUILifeCycleBehavioursChildAttachmentChanged(this, true);
			}

			if (this->IsRegistered())//not registered means is loading from level.
			{
				this->CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
			}
		}
	}

	ULGUICanvas* ParentCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(GetOwner()->GetAttachParentActor(), false);
	UUICanvasGroup* ParentCanvasGroup = LGUIUtils::GetComponentInParent<UUICanvasGroup>(GetOwner()->GetAttachParentActor(), false);
	UIHierarchyChanged(ParentCanvas, ParentCanvasGroup);
	//callback
	CallUILifeCycleBehavioursAttachmentChanged();
}

void UUIItem::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;

	if (auto childUIItem = Cast<UUIItem>(ChildComponent))
	{
		//hierarchy index
		CheckCacheUIChildren();
		UIChildren.Remove(childUIItem);
		for (int i = 0; i < UIChildren.Num(); i++)
		{
			auto& UIChild = UIChildren[i];
			if (UIChild->hierarchyIndex != i)
			{
				UIChild->hierarchyIndex = i;
				this->CallUILifeCycleBehavioursChildHierarchyIndexChanged(UIChild);
			}
		}
		MarkCanvasUpdate(false, false, false);

		childUIItem->OnUIDetachedFromParent();
	}
}

void UUIItem::OnUIDetachedFromParent()
{
#if WITH_EDITORONLY_DATA
	if (!GetWorld()->IsGameWorld())
	{
		if (ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(this->GetOwner()))//when load from prefab or duplicate from LGUICopier, the ChildAttachmentChanged callback should execute til prefab serialization ready
		{
			if (ParentUIItem.IsValid())//tell old parent
			{
				ULGUIEditorManagerObject::AddFunctionForPrefabSystemExecutionBeforeAwake(this->GetOwner(), [Child = MakeWeakObjectPtr(this), Parent = ParentUIItem]() {
					if (Child.IsValid() && Parent.IsValid())
					{
						Parent->CallUILifeCycleBehavioursChildAttachmentChanged(Child.Get(), false);
					}});
			}
		}
		else
		{
			if (ParentUIItem.IsValid())//tell old parent
			{
				ParentUIItem->CallUILifeCycleBehavioursChildAttachmentChanged(this, false);
			}

			if (this->IsRegistered())//not registered means is loading from level.
			{
				this->CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
			}
		}
	}
	else
#endif
	{
		auto ManagerActor = ALGUIManagerActor::GetLGUIManagerActorInstance(this->GetOwner());
		if (ManagerActor && ManagerActor->IsPrefabSystemProcessingActor(this->GetOwner()))//when load from prefab or duplicate from LGUICopier, the ChildAttachmentChanged callback should execute til prefab serialization ready
		{
			if (ParentUIItem.IsValid())//tell old parent
			{
				ManagerActor->AddFunctionForPrefabSystemExecutionBeforeAwake(this->GetOwner(), [Child = MakeWeakObjectPtr(this), Parent = ParentUIItem]() {
					if (Child.IsValid() && Parent.IsValid())
					{
						Parent->CallUILifeCycleBehavioursChildAttachmentChanged(Child.Get(), false);
					}});
			}
		}
		else
		{
			if (ParentUIItem.IsValid())//tell old parent
			{
				ParentUIItem->CallUILifeCycleBehavioursChildAttachmentChanged(this, false);
			}

			if (this->IsRegistered())//not registered means is loading from level.
			{
				this->CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
			}
		}
	}

	UIHierarchyChanged(nullptr, nullptr);
	//callback
	CallUILifeCycleBehavioursAttachmentChanged();
}

void UUIItem::OnRegister()
{
	Super::OnRegister();
	//UE_LOG(LGUI, Error, TEXT("OnRegister:%s, registered:%d"), *(this->GetOwner()->GetActorLabel()), this->IsRegistered());
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld() && GetOwner())
		{
			//create helper for root component
			if (this->GetOwner()->GetRootComponent() == this 
				&& this->GetOwner()->GetWorld() != nullptr
				&& !this->IsPendingKillOrUnreachable())
			{
				if (!HelperComp)
				{
					HelperComp = NewObject<UUIItemEditorHelperComp>(GetOwner(), NAME_None, RF_Transient | RF_TextExportTransient);
					HelperComp->Parent = this;
					HelperComp->Mobility = EComponentMobility::Movable;
					HelperComp->SetIsVisualizationComponent(true);
					HelperComp->SetupAttachment(this);
					HelperComp->RegisterComponent();
				}
			}
			//display name
			if (this->GetOwner()->GetRootComponent() == this)
			{
				auto actorLabel = FString(*this->GetOwner()->GetActorLabel());
				this->displayName = actorLabel;
			}
			else
			{
				this->displayName = this->GetName();
			}
		}
#endif
	}
#if WITH_EDITOR
	else
	{
		this->displayName = this->GetName();
	}
#endif

#if WITH_EDITOR
	//apply inactive actor's visibility state in editor scene outliner
	if (auto ownerActor = GetOwner())
	{
		if (!GetIsUIActiveInHierarchy())
		{
			ownerActor->SetIsTemporarilyHiddenInEditor(true);
		}
	}
#endif

	bCanSetAnchorFromTransform = true;
	CheckRootUIItem();
}
void UUIItem::OnUnregister()
{
	Super::OnUnregister();
	if (auto world = this->GetWorld())
	{
#if WITH_EDITOR
		if (!world->IsGameWorld())
		{
			if (this->GetName().StartsWith(TEXT("REINST_")))//when recompile a blueprint object, the old one will become REINST_XXX and not valid
			{
				if (RenderCanvas.IsValid())
				{
					OnRenderCanvasChanged(RenderCanvas.Get(), nullptr);
					RenderCanvas = nullptr;
				}
			}
		}
#endif
	}
#if WITH_EDITORONLY_DATA
	if (HelperComp)
	{
		HelperComp->DestroyComponent();
		HelperComp = nullptr;
	}
#endif
	CheckRootUIItem();
}

void UUIItem::OnComponentDestroyed(bool bDestroyingHierarchy)
{
	Super::OnComponentDestroyed(bDestroyingHierarchy);
}

void UUIItem::CheckCacheUIChildren()
{
	for (int i = UIChildren.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(UIChildren[i]))
		{
			UIChildren.RemoveAt(i);
		}
	}
}

void UUIItem::RegisterRenderCanvas(ULGUICanvas* InRenderCanvas)
{
	bIsCanvasUIItem = true;
	auto ParentCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(GetOwner()->GetAttachParentActor(), false);//@todo: replace with Canvas's ParentCanvas?
	if (RenderCanvas != InRenderCanvas)
	{
		SetRenderCanvas(InRenderCanvas);
	}
	InRenderCanvas->SetParentCanvas(ParentCanvas);
	for (auto& uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->RenewRenderCanvasRecursive(InRenderCanvas);
		}
	}
}
void UUIItem::RenewRenderCanvasRecursive(ULGUICanvas* InParentRenderCanvas)
{
	auto ThisRenderCanvas = GetOwner()->FindComponentByClass<ULGUICanvas>();
	if (ThisRenderCanvas != nullptr && !ThisRenderCanvas->IsRegistered())//ignore unregistered
	{
		ThisRenderCanvas = nullptr;
	}
	if (ThisRenderCanvas != nullptr)
	{
		if (InParentRenderCanvas != ThisRenderCanvas)
		{
			ThisRenderCanvas->SetParentCanvas(InParentRenderCanvas);//set parent Canvas for this actor's Canvas
		}
		return;//already have a CanvasGroup on this actor, no need to go further
	}

	if (RenderCanvas != InParentRenderCanvas)//if attach to new Canvas, need to remove from old and add to new
	{
		SetRenderCanvas(InParentRenderCanvas);
	}

	for (auto& uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->RenewRenderCanvasRecursive(InParentRenderCanvas);
		}
	}
}

void UUIItem::UnregisterRenderCanvas()
{
	bIsCanvasUIItem = false;
	auto ParentCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(GetOwner()->GetAttachParentActor(), false);
	if (RenderCanvas != ParentCanvas)
	{
		SetRenderCanvas(ParentCanvas);
	}
	for (auto& uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->RenewRenderCanvasRecursive(ParentCanvas);
		}
	}
}

void UUIItem::SetRenderCanvas(ULGUICanvas* InNewCanvas)
{
	auto OldRenderCanvas = RenderCanvas;
	RenderCanvas = InNewCanvas;
	OnRenderCanvasChanged(OldRenderCanvas.Get(), RenderCanvas.Get());
}

void UUIItem::RegisterCanvasGroup(UUICanvasGroup* InCanvasGroup)
{
	auto ParentCanvasGroup = LGUIUtils::GetComponentInParent<UUICanvasGroup>(GetOwner()->GetAttachParentActor(), false);//@todo: replace with CanvasGroup's ParentCanvasGroup?
	InCanvasGroup->SetParentCanvasGroup(ParentCanvasGroup);
	SetCanvasGroup(InCanvasGroup);
	for (auto& uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->RenewCanvasGroupRecursive(InCanvasGroup);
		}
	}
}
void UUIItem::UnregisterCanvasGroup()
{
	auto ParentCanvasGroup = LGUIUtils::GetComponentInParent<UUICanvasGroup>(GetOwner()->GetAttachParentActor(), false);//@todo: replace with CanvasGroup's ParentCanvasGroup?
	SetCanvasGroup(ParentCanvasGroup);
	for (auto uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->RenewCanvasGroupRecursive(ParentCanvasGroup);
		}
	}
}

void UUIItem::RenewCanvasGroupRecursive(UUICanvasGroup* InParentCanvasGroup)
{
	auto ThisCanvasGroup = GetOwner()->FindComponentByClass<UUICanvasGroup>();
	if (ThisCanvasGroup != nullptr && !ThisCanvasGroup->IsRegistered())//ignore unregistered
	{
		ThisCanvasGroup = nullptr;
	}
	if (ThisCanvasGroup != nullptr)
	{
		if (InParentCanvasGroup != ThisCanvasGroup)
		{
			ThisCanvasGroup->SetParentCanvasGroup(InParentCanvasGroup);//set parent CanvasGroup for this actor's CanvasGroup
		}
		return;//already have a CanvasGroup on this actor, no need to go further
	}

	if (CanvasGroup != InParentCanvasGroup)//CanvasGroup changed
	{
		SetCanvasGroup(InParentCanvasGroup);
	}

	for (auto& uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->RenewCanvasGroupRecursive(InParentCanvasGroup);
		}
	}
}

void UUIItem::SetCanvasGroup(UUICanvasGroup* InNewCanvasGroup)
{
	//remove from old
	if (CanvasGroup.IsValid())
	{
		CanvasGroup->UnregisterAlphaChange(OnCanvasGroupAlphaChangeDelegateHandle);
		CanvasGroup->UnregisterInteractableStateChange(OnCanvasGroupInteractableStateChangeDelegateHandle);
		OnCanvasGroupAlphaChange();
		OnCanvasGroupInteractableStateChange();
	}
	//add to new
	CanvasGroup = InNewCanvasGroup;
	if (CanvasGroup.IsValid())
	{
		OnCanvasGroupAlphaChangeDelegateHandle = CanvasGroup->RegisterAlphaChange(FSimpleDelegate::CreateUObject(this, &UUIItem::OnCanvasGroupAlphaChange));
		OnCanvasGroupInteractableStateChangeDelegateHandle = CanvasGroup->RegisterInteractableStateChange(FSimpleDelegate::CreateUObject(this, &UUIItem::OnCanvasGroupInteractableStateChange));
		OnCanvasGroupAlphaChange();
		OnCanvasGroupInteractableStateChange();
	}
}

void UUIItem::UIHierarchyChanged(ULGUICanvas* ParentRenderCanvas, UUICanvasGroup* ParentCanvasGroup)
{
	auto ThisRenderCanvas = GetOwner()->FindComponentByClass<ULGUICanvas>();
	if (ThisRenderCanvas != nullptr)
	{
		ParentRenderCanvas = ThisRenderCanvas;
	}

	if (RenderCanvas != ParentRenderCanvas)//if attach to new Canvas, need to remove from old and add to new
	{
		SetRenderCanvas(ParentRenderCanvas);
	}

	auto ThisCanvasGroup = GetOwner()->FindComponentByClass<UUICanvasGroup>();
	if (ThisCanvasGroup != nullptr)
	{
		ParentCanvasGroup = ThisCanvasGroup;
	}
	if (CanvasGroup != ParentCanvasGroup)//CanvasGroup changed
	{
		//remove from old
		if (CanvasGroup.IsValid())
		{
			CanvasGroup->UnregisterAlphaChange(OnCanvasGroupAlphaChangeDelegateHandle);
			CanvasGroup->UnregisterInteractableStateChange(OnCanvasGroupInteractableStateChangeDelegateHandle);
			OnCanvasGroupAlphaChange();
			OnCanvasGroupInteractableStateChange();
		}
		//add to new
		CanvasGroup = ParentCanvasGroup;
		if (CanvasGroup.IsValid())
		{
			OnCanvasGroupAlphaChangeDelegateHandle = CanvasGroup->RegisterAlphaChange(FSimpleDelegate::CreateUObject(this, &UUIItem::OnCanvasGroupAlphaChange));
			OnCanvasGroupInteractableStateChangeDelegateHandle = CanvasGroup->RegisterInteractableStateChange(FSimpleDelegate::CreateUObject(this, &UUIItem::OnCanvasGroupInteractableStateChange));
			OnCanvasGroupAlphaChange();
			OnCanvasGroupInteractableStateChange();
		}
	}

	if (UIHierarchyChangedDelegate.IsBound())
	{
		UIHierarchyChangedDelegate.Broadcast();
	}

	for (auto& uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->UIHierarchyChanged(ParentRenderCanvas, ParentCanvasGroup);
		}
	}

	CheckRootUIItem();

	//flatten hierarchy index
	MarkFlattenHierarchyIndexDirty();

	if (ParentUIItem.IsValid())
	{
		//active state
		this->bAllUpParentUIActive = ParentUIItem->GetIsUIActiveInHierarchy();
		this->CheckUIActiveState();
	}
	else
	{
		this->bAllUpParentUIActive = true;
		this->CheckUIActiveState();
	}

	//if (this->IsRegistered())//not registerd means could be load from level
	{
		bWidthCached = false;
		bHeightCached = false;
		bAnchorLeftCached = false;
		bAnchorRightCached = false;
		bAnchorBottomCached = false;
		bAnchorTopCached = false;

		SetOnAnchorChange(false, true);
	}
}

void UUIItem::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveUIItem(this);
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddUIItem(this);
	}
}

void UUIItem::CheckRootUIItem()
{
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		auto oldRootUIItem = RootUIItem;
		if (oldRootUIItem == this && oldRootUIItem != nullptr)
		{
			ULGUIEditorManagerObject::RemoveRootUIItem(this);
		}
	}
#endif

	UUIItem* topUIItem = this;
	UUIItem* tempRootUIItem = nullptr;
	while (topUIItem != nullptr && topUIItem->IsRegistered())
	{
		tempRootUIItem = topUIItem;
		topUIItem = Cast<UUIItem>(topUIItem->GetAttachParent());
	}
	RootUIItem = tempRootUIItem;

#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		if (RootUIItem == this && RootUIItem != nullptr)
		{
			ULGUIEditorManagerObject::AddRootUIItem(this);
		}
	}
#endif
}

FDelegateHandle UUIItem::RegisterUIHierarchyChanged(const FSimpleDelegate& InCallback)
{
	return UIHierarchyChangedDelegate.Add(InCallback);
}
void UUIItem::UnregisterUIHierarchyChanged(const FDelegateHandle& InHandle)
{
	UIHierarchyChangedDelegate.Remove(InHandle);
}

bool UUIItem::CalculateTransformFromAnchor()
{
#if WITH_EDITOR
	if (this->GetWorld() && !this->GetWorld()->IsGameWorld())
	{
		if (!ALGUIManagerActor::GetIsPlaying(this->GetWorld()))
		{
			if (!GetDefault<ULGUIEditorSettings>()->AnchorControlPosition)
			{
				return false;
			}
		}
	}
#endif
	bCanSetAnchorFromTransform = false;
	FVector ResultLocation = this->GetRelativeLocation();
	if (ParentUIItem.IsValid())
	{
		float LocalLeftPoint = //this left point anchor position in parent's space
			ParentUIItem->GetLocalSpaceLeft()//parent's left position
			+ (ParentUIItem->GetWidth() * this->AnchorData.AnchorMin.X);//add anchor offset
		float LocalLeftPivotPoint = //to pivot point, with anchor offset
			LocalLeftPoint
			+ (ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X))//parent anchor width (width without SizeDelta)
				* this->AnchorData.Pivot.X
			+ this->AnchorData.AnchoredPosition.X;

		float LocalBottomPoint = //this bottom point anchor position in parent's space
			ParentUIItem->GetLocalSpaceBottom()//parent's bottom position
			+ (ParentUIItem->GetHeight() * this->AnchorData.AnchorMin.Y);//add anchor offset
		float LocalBottomPivotPoint = //to pivot point, with anchor offset
			LocalBottomPoint
			+ (ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y))//parent anchor width (width without SizeDelta)
				* this->AnchorData.Pivot.Y
			+ this->AnchorData.AnchoredPosition.Y;

		ResultLocation.Y = LocalLeftPivotPoint;
		ResultLocation.Z = LocalBottomPivotPoint;
	}
	else
	{
		ResultLocation.Y = this->AnchorData.AnchoredPosition.X;
		ResultLocation.Z = this->AnchorData.AnchoredPosition.Y;
	}

	bool TransformChange = false;
	if (!this->GetRelativeLocation().Equals(ResultLocation, 0.0f))
	{
		GetRelativeLocation_DirectMutable() = ResultLocation;
		UpdateComponentToWorld();
		TransformChange = true;
	}
	bCanSetAnchorFromTransform = true;

	return TransformChange;
}


#pragma region AnchorData

float UUIItem::GetWidth() const
{
	if (!bWidthCached)
	{
		bWidthCached = true;
		if (ParentUIItem.IsValid())
		{
			if (AnchorData.IsHorizontalStretched())
			{
				CacheWidth = AnchorData.SizeDelta.X + ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X);
			}
			else
			{
				CacheWidth = AnchorData.SizeDelta.X;
			}
		}
		else
		{
			CacheWidth = AnchorData.SizeDelta.X;
		}
	}
	return CacheWidth;
}
float UUIItem::GetHeight() const
{
	if (!bHeightCached)
	{
		bHeightCached = true;
		if (ParentUIItem.IsValid())
		{
			if (AnchorData.IsVerticalStretched())
			{
				CacheHeight = AnchorData.SizeDelta.Y + ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y);
			}
			else
			{
				CacheHeight = AnchorData.SizeDelta.Y;
			}
		}
		else
		{
			CacheHeight = AnchorData.SizeDelta.Y;
		}
	}
	return CacheHeight;
}

void UUIItem::SetAnchorData(const FUIAnchorData& InAnchorData)
{
	AnchorData.Pivot = InAnchorData.Pivot;
	AnchorData.AnchorMin = InAnchorData.AnchorMin;
	AnchorData.AnchorMax = InAnchorData.AnchorMax;
	AnchorData.AnchoredPosition = InAnchorData.AnchoredPosition;
	AnchorData.SizeDelta = InAnchorData.SizeDelta;

	bWidthCached = false;
	bHeightCached = false;
	bAnchorLeftCached = false;
	bAnchorRightCached = false;
	bAnchorBottomCached = false;
	bAnchorTopCached = false;

	SetOnAnchorChange(true, true);
}

void UUIItem::SetPivot(FVector2D Value) 
{
	if (!AnchorData.Pivot.Equals(Value, 0.0f))
	{
		AnchorData.Pivot = Value;
		bAnchorLeftCached = false;
		bAnchorRightCached = false;
		bAnchorBottomCached = false;
		bAnchorTopCached = false;
		SetOnAnchorChange(true, false);
	}
}

void UUIItem::SetAnchorMin(FVector2D Value)
{
	if (this->ParentUIItem.IsValid())
	{
		if (!AnchorData.AnchorMin.Equals(Value, 0.0f))
		{
			auto CurrentLeft = this->GetAnchorLeft();
			auto CurrentBottom = this->GetAnchorBottom();

			AnchorData.AnchorMin = Value;
			
			//SetAnchorLeft
			{
				auto CurrentRight = this->GetAnchorRight();
				auto CalculatedWidth = this->ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - CurrentRight - CurrentLeft;
				//SetWidth
				{
					auto CalculatedSizeDeltaX = CalculatedWidth - (ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				this->AnchorData.AnchoredPosition.X = FMath::Lerp(CurrentLeft, -CurrentRight, this->AnchorData.Pivot.X);
			}

			//SetAnchorBottom
			{
				auto CurrentTop = this->GetAnchorTop();
				auto CalculatedHeight = this->ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - CurrentTop - CurrentBottom;
				//SetHeight
				{
					auto CalculatedSizeDeltaY = CalculatedHeight - (ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				this->AnchorData.AnchoredPosition.Y = FMath::Lerp(CurrentBottom, -CurrentTop, this->AnchorData.Pivot.Y);
			}

			SetOnAnchorChange(false, true);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetAnchorMin]This function only valid if UIItem have parent!"))
	}
}
void UUIItem::SetAnchorMax(FVector2D Value)
{
	if (this->ParentUIItem.IsValid())
	{
		if (!AnchorData.AnchorMax.Equals(Value, 0.0f))
		{
			auto CurrentRight = this->GetAnchorRight();
			auto CurrentTop = this->GetAnchorTop();

			AnchorData.AnchorMax = Value;

			//SetAnchorRight
			{
				auto CurrentLeft = this->GetAnchorLeft();
				CacheWidth = this->ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - CurrentRight - CurrentLeft;
				//SetWidth
				{
					auto CalculatedSizeDeltaX = CacheWidth - (ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				this->AnchorData.AnchoredPosition.X = FMath::Lerp(CurrentLeft, -CurrentRight, this->AnchorData.Pivot.X);
			}
			//SetAnchorTop
			{
				auto CurrentBottom = this->GetAnchorBottom();
				CacheHeight = this->ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - CurrentTop - CurrentBottom;
				//SetHeight
				{
					auto CalculatedSizeDeltaY = CacheHeight - (ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				this->AnchorData.AnchoredPosition.Y = FMath::Lerp(CurrentBottom, -CurrentTop, this->AnchorData.Pivot.Y);
			}

			SetOnAnchorChange(false, true);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetAnchorMax]This function only valid if UIItem have parent!"))
	}
}

void UUIItem::SetHorizontalAndVerticalAnchorMinMax(FVector2D MinValue, FVector2D MaxValue, bool bKeepSize, bool bKeepRelativeLocation)
{
	if (this->ParentUIItem.IsValid())
	{
		if (!AnchorData.AnchorMin.Equals(MinValue, 0.0f) || !AnchorData.AnchorMax.Equals(MaxValue, 0.0f))
		{
			auto PrevRelativeLocation = this->GetRelativeLocation();
			auto PrevWidth = this->GetWidth();
			auto PrevHeight = this->GetHeight();
			this->SetAnchorMin(MinValue);
			this->SetAnchorMax(MaxValue);
			if (bKeepSize)
			{
				this->SetWidth(PrevWidth);
				this->SetHeight(PrevHeight);
			}
			if (bKeepRelativeLocation)
			{
				this->SetRelativeLocation(PrevRelativeLocation);
			}
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetHorizontalAndVerticalAnchorMinMax]This function only valid if UIItem have parent!"))
	}
}

void UUIItem::SetHorizontalAnchorMinMax(FVector2D Value, bool bKeepSize, bool bKeepRelativeLocation)
{
	if (this->ParentUIItem.IsValid())
	{
		if (AnchorData.AnchorMin.X != Value.X || AnchorData.AnchorMax.X != Value.Y)
		{
			auto CurrentLeft = this->GetAnchorLeft();
			auto CurrentRight = this->GetAnchorRight();

			if (bKeepSize)
			{
				CacheWidth = this->GetWidth();
			}
			auto PrevRelativeLocation = this->GetRelativeLocation();

			AnchorData.AnchorMin.X = Value.X;
			AnchorData.AnchorMax.X = Value.Y;

			//SetAnchorLeft & SetAnchorRight
			{
				if (!bKeepSize)//recalculate size on new anchor if not keep size
				{
					CacheWidth = this->ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - CurrentRight - CurrentLeft;
				}
				//SetWidth
				{
					auto CalculatedSizeDeltaX = CacheWidth - (ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				this->AnchorData.AnchoredPosition.X = FMath::Lerp(CurrentLeft, -CurrentRight, this->AnchorData.Pivot.X);
			}
			if (bKeepRelativeLocation)
			{
				this->SetRelativeLocation(PrevRelativeLocation);
			}

			SetOnAnchorChange(false, !bKeepSize);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetHorizontalAnchorMinMax]This function only valid if UIItem have parent!"))
	}
}
void UUIItem::SetVerticalAnchorMinMax(FVector2D Value, bool bKeepSize, bool bKeepRelativeLocation)
{
	if (this->ParentUIItem.IsValid())
	{
		if (AnchorData.AnchorMin.Y != Value.X || AnchorData.AnchorMax.Y != Value.Y)
		{
			auto CurrentBottom = this->GetAnchorBottom();
			auto CurrentTop = this->GetAnchorTop();

			if (bKeepSize)
			{
				CacheHeight = this->GetHeight();
			}
			auto PrevRelativeLocation = this->GetRelativeLocation();

			AnchorData.AnchorMin.Y = Value.X;
			AnchorData.AnchorMax.Y = Value.Y;

			//SetAnchorBottom && SetAnchorTop
			{
				if (!bKeepSize)//recalculate size on new anchor if not keep size
				{
					CacheHeight = this->ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - CurrentTop - CurrentBottom;
				}
				//SetHeight
				{
					auto CalculatedSizeDeltaY = CacheHeight - (ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				this->AnchorData.AnchoredPosition.Y = FMath::Lerp(CurrentBottom, -CurrentTop, this->AnchorData.Pivot.Y);
			}
			if (bKeepRelativeLocation)
			{
				this->SetRelativeLocation(PrevRelativeLocation);
			}

			SetOnAnchorChange(false, !bKeepSize);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetVerticalAnchorMinMax]This function only valid if UIItem have parent!"))
	}
}

void UUIItem::SetAnchoredPosition(FVector2D Value)
{
	if (!AnchorData.AnchoredPosition.Equals(Value, 0.0f))
	{
		AnchorData.AnchoredPosition = Value;
		SetOnAnchorChange(false, false);
	}
}

void UUIItem::SetHorizontalAnchoredPosition(float Value)
{
	if (AnchorData.AnchoredPosition.X != Value)
	{
		AnchorData.AnchoredPosition.X = Value;
		SetOnAnchorChange(false, false);
	}
}
void UUIItem::SetVerticalAnchoredPosition(float Value)
{
	if (AnchorData.AnchoredPosition.Y != Value)
	{
		AnchorData.AnchoredPosition.Y = Value;
		SetOnAnchorChange(false, false);
	}
}

void UUIItem::SetSizeDelta(FVector2D Value)
{
	if (!AnchorData.SizeDelta.Equals(Value, 0.0f))
	{
		AnchorData.SizeDelta = Value;
		bWidthCached = false;
		bHeightCached = false;
		SetOnAnchorChange(false, true);
	}
}

float UUIItem::GetAnchorLeft()const
{
	if (!bAnchorLeftCached)
	{
		bAnchorLeftCached = true;
		if (this->ParentUIItem.IsValid())
		{
			CacheAnchorLeft =
				this->GetLocalSpaceLeft()//local space left
				+ this->GetRelativeLocation().Y//convert to parent space
				-
				(this->ParentUIItem->GetLocalSpaceLeft()//parent space left
					+ this->ParentUIItem->GetWidth() * this->AnchorData.AnchorMin.X)//to parent anchor min point
				;
		}
		else
		{
			CacheAnchorLeft = this->GetLocalSpaceLeft();//local space left
		}
	}
	return CacheAnchorLeft;
}
float UUIItem::GetAnchorTop()const
{
	if (!bAnchorTopCached)
	{
		bAnchorTopCached = true;
		if (this->ParentUIItem.IsValid())
		{
			CacheAnchorTop =
				-(
					this->GetLocalSpaceTop()
					+ this->GetRelativeLocation().Z
					-
					(this->ParentUIItem->GetLocalSpaceTop()
						- this->ParentUIItem->GetHeight() * (1.0f - this->AnchorData.AnchorMax.Y))
					)
				;
		}
		else
		{
			CacheAnchorTop = this->GetLocalSpaceTop();
		}
	}
	return CacheAnchorTop;
}
float UUIItem::GetAnchorRight()const
{
	if (!bAnchorRightCached)
	{
		bAnchorRightCached = true;
		if (this->ParentUIItem.IsValid())
		{
			CacheAnchorRight =
				-(
					this->GetLocalSpaceRight()
					+ this->GetRelativeLocation().Y
					-
					(this->ParentUIItem->GetLocalSpaceRight()
						- this->ParentUIItem->GetWidth() * (1.0f - this->AnchorData.AnchorMax.X))
					)
				;
		}
		else
		{
			CacheAnchorRight = this->GetLocalSpaceRight();
		}
	}
	return CacheAnchorRight;
}
float UUIItem::GetAnchorBottom()const
{
	if (!bAnchorBottomCached)
	{
		bAnchorBottomCached = true;
		if (this->ParentUIItem.IsValid())
		{
			CacheAnchorBottom =
				this->GetLocalSpaceBottom()
				+ this->GetRelativeLocation().Z
				-
				(this->ParentUIItem->GetLocalSpaceBottom()
					+ this->ParentUIItem->GetHeight() * this->AnchorData.AnchorMin.Y)
				;
		}
		else
		{
			CacheAnchorBottom = this->GetLocalSpaceBottom();
		}
	}
	return CacheAnchorBottom;
}

void UUIItem::SetAnchorLeft(float Value)
{
	if (this->ParentUIItem.IsValid())
	{
		if (CacheAnchorLeft != Value || !bAnchorLeftCached)
		{
			bAnchorLeftCached = true;
			CacheAnchorLeft = Value;
			auto CurrentRight = this->GetAnchorRight();
			CacheWidth = this->ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - CurrentRight - Value;
			//SetWdith
			{
				if (AnchorData.IsHorizontalStretched())
				{
					auto CalculatedSizeDeltaX = CacheWidth - (ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				else
				{
					AnchorData.SizeDelta.X = CacheWidth;
				}
			}
			this->AnchorData.AnchoredPosition.X = FMath::Lerp(Value, -CurrentRight, this->AnchorData.Pivot.X);
			SetOnAnchorChange(false, true);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetAnchorLeft]This function only valid if UIItem have parent!"))
	}
}
void UUIItem::SetAnchorTop(float Value)
{
	if (this->ParentUIItem.IsValid())
	{
		if (CacheAnchorTop != Value || !bAnchorTopCached)
		{
			bAnchorTopCached = true;
			CacheAnchorTop = Value;
			auto CurrentBottom = this->GetAnchorBottom();
			CacheHeight = this->ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - Value - CurrentBottom;
			//SetHeight
			{
				if (AnchorData.IsVerticalStretched())
				{
					auto CalculatedSizeDeltaY = CacheHeight - (ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				else
				{
					AnchorData.SizeDelta.Y = CacheHeight;
				}
			}
			this->AnchorData.AnchoredPosition.Y = FMath::Lerp(CurrentBottom, -Value, this->AnchorData.Pivot.Y);
			SetOnAnchorChange(false, true);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetAnchorTop]This function only valid if UIItem have parent!"))
	}
}
void UUIItem::SetAnchorRight(float Value)
{
	if (this->ParentUIItem.IsValid())
	{
		if (CacheAnchorRight != Value || !bAnchorRightCached)
		{
			bAnchorRightCached = true;
			CacheAnchorRight = Value;
			auto CurrentLeft = this->GetAnchorLeft();
			CacheWidth = this->ParentUIItem->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - Value - CurrentLeft;
			//SetWdith
			{
				if (AnchorData.IsHorizontalStretched())
				{
					auto CalculatedSizeDeltaX = CacheWidth - (ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				else
				{
					AnchorData.SizeDelta.X = CacheWidth;
				}
			}
			this->AnchorData.AnchoredPosition.X = FMath::Lerp(CurrentLeft, -Value, this->AnchorData.Pivot.X);
			SetOnAnchorChange(false, true);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetAnchorRight]This function only valid if UIItem have parent!"))
	}
}
void UUIItem::SetAnchorBottom(float Value)
{
	if (this->ParentUIItem.IsValid())
	{
		if (CacheAnchorBottom != Value || !bAnchorBottomCached)
		{
			bAnchorBottomCached = true;
			CacheAnchorBottom = Value;
			auto CurrentTop = this->GetAnchorTop();
			CacheHeight = this->ParentUIItem->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - CurrentTop - Value;
			//SetHeight
			{
				if (AnchorData.IsVerticalStretched())
				{
					auto CalculatedSizeDeltaY = CacheHeight - (ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				else
				{
					AnchorData.SizeDelta.Y = CacheHeight;
				}
			}
			this->AnchorData.AnchoredPosition.Y = FMath::Lerp(Value, -CurrentTop, this->AnchorData.Pivot.Y);
			SetOnAnchorChange(false, true);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIItem::SetAnchorBottom]This function only valid if UIItem have parent!"))
	}
}

void UUIItem::SetWidth(float Value)
{
	if (CacheWidth != Value || !bWidthCached)
	{
		bWidthCached = true;
		CacheWidth = Value;
		if (ParentUIItem.IsValid())
		{
			if (AnchorData.IsHorizontalStretched())
			{
				auto CalculatedSizeDeltaX = Value - (ParentUIItem->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
				if (AnchorData.SizeDelta.X != CalculatedSizeDeltaX)
				{
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
					SetOnAnchorChange(false, true);
				}
			}
			else
			{
				if (AnchorData.SizeDelta.X != Value)
				{
					AnchorData.SizeDelta.X = Value;
					SetOnAnchorChange(false, true);
				}
			}
		}
		else
		{
			if (AnchorData.SizeDelta.X != Value)
			{
				AnchorData.SizeDelta.X = Value;
				SetOnAnchorChange(false, true);
			}
		}
	}
}
void UUIItem::SetHeight(float Value)
{
	if (CacheHeight != Value || !bHeightCached)
	{
		bHeightCached = true;
		CacheHeight = Value;
		if (ParentUIItem.IsValid())
		{
			if (AnchorData.IsVerticalStretched())
			{
				auto CalculatedSizeDeltaY = Value - (ParentUIItem->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
				if (AnchorData.SizeDelta.Y != CalculatedSizeDeltaY)
				{
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
					SetOnAnchorChange(false, true);
				}
			}
			else
			{
				if (AnchorData.SizeDelta.Y != Value)
				{
					AnchorData.SizeDelta.Y = Value;
					SetOnAnchorChange(false, true);
				}
			}
		}
		else
		{
			if (AnchorData.SizeDelta.Y != Value)
			{
				AnchorData.SizeDelta.Y = Value;
				SetOnAnchorChange(false, true);
			}
		}
	}
}

#pragma endregion

UUIItem* UUIItem::GetAttachUIChild(int index)const
{
	if (index < 0 || index >= UIChildren.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[UUIItem::GetAttachUIChild]index:%d out of range[%d, %d]"), index, 0, UIChildren.Num() - 1);
		return nullptr;
	}
	return UIChildren[index];
}
ULGUICanvas* UUIItem::GetRootCanvas()const
{
	if (RenderCanvas.IsValid())
	{
		return RenderCanvas->GetRootCanvas();
	}
	return nullptr;
}
ULGUICanvasScaler* UUIItem::GetCanvasScaler()const
{
	if (auto canvas = GetRootCanvas())
	{
		return canvas->GetOwner()->FindComponentByClass<ULGUICanvasScaler>();
	}
	return nullptr;
}

FVector2D UUIItem::GetLocalSpaceLeftBottomPoint()const
{
	FVector2D leftBottomPoint;
	leftBottomPoint.X = GetWidth() * -AnchorData.Pivot.X;
	leftBottomPoint.Y = GetHeight() * -AnchorData.Pivot.Y;
	return leftBottomPoint;
}
FVector2D UUIItem::GetLocalSpaceRightTopPoint()const
{
	FVector2D rightTopPoint;
	rightTopPoint.X = GetWidth() * (1.0f - AnchorData.Pivot.X);
	rightTopPoint.Y = GetHeight() * (1.0f - AnchorData.Pivot.Y);
	return rightTopPoint;
}
FVector2D UUIItem::GetLocalSpaceCenter()const
{
	return FVector2D(this->GetWidth() * (0.5f - AnchorData.Pivot.X), this->GetHeight() * (0.5f - AnchorData.Pivot.Y));
}

float UUIItem::GetLocalSpaceLeft()const
{
	return this->GetWidth() * -AnchorData.Pivot.X;
}
float UUIItem::GetLocalSpaceRight()const
{
	return this->GetWidth() * (1.0f - AnchorData.Pivot.X);
}
float UUIItem::GetLocalSpaceBottom()const
{
	return this->GetHeight() * -AnchorData.Pivot.Y;
}
float UUIItem::GetLocalSpaceTop()const
{
	return this->GetHeight() * (1.0f - AnchorData.Pivot.Y);
}

void UUIItem::SetOnAnchorChange(bool InPivotChange, bool InSizeChange)
{
	OnAnchorChange(InPivotChange, InSizeChange, false);
}

void UUIItem::SetOnTransformChange()
{
	if (this->RenderCanvas.IsValid())
	{
		this->RenderCanvas->MarkCanvasUpdate(false, true, false);//mark canvas to update
		if (this->IsCanvasUIItem())
		{
			this->RenderCanvas->MarkCanvasLayoutDirty();
		}
	}

	CallUILifeCycleBehavioursDimensionsChanged(true, false);

	for (auto& UIChild : UIChildren)
	{
		if (IsValid(UIChild))
		{
			UIChild->SetOnTransformChange();
		}
	}
}

void UUIItem::OnAnchorChange(bool InPivotChange, bool InSizeChange, bool InDiscardCache)
{
	bool TransformChange = CalculateTransformFromAnchor();

	if (InDiscardCache)
	{
		if (InSizeChange)
		{
			bWidthCached = false;
			bHeightCached = false;
		}
		bAnchorLeftCached = false;
		bAnchorRightCached = false;
		bAnchorBottomCached = false;
		bAnchorTopCached = false;
	}

	if (this->RenderCanvas.IsValid())
	{
		this->RenderCanvas->MarkCanvasUpdate(false, TransformChange, false);//mark canvas to update
		if (this->IsCanvasUIItem())
		{
			this->RenderCanvas->MarkCanvasLayoutDirty();
		}
	}

	CallUILifeCycleBehavioursDimensionsChanged(TransformChange, InSizeChange);

	for (auto& UIChild : UIChildren)
	{
		if (IsValid(UIChild))
		{
			bool ChildSizeChange = false;
			if (InSizeChange)
			{
				if (UIChild->AnchorData.IsHorizontalStretched() || UIChild->AnchorData.IsVerticalStretched())
				{
					ChildSizeChange = true;
				}
			}
			UIChild->OnAnchorChange(false, ChildSizeChange);
		}
	}
}

void UUIItem::MarkCanvasUpdate(bool bMaterialOrTextureChanged, bool bTransformOrVertexPositionChanged, bool bHierarchyOrderChanged, bool bForceRebuildDrawcall)
{
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkCanvasUpdate(bMaterialOrTextureChanged, bTransformOrVertexPositionChanged, bHierarchyOrderChanged, bForceRebuildDrawcall);
	}
}

void UUIItem::SetRaycastTarget(bool NewBool)
{
	if (bRaycastTarget != NewBool)
	{
		bRaycastTarget = NewBool;
	}
}

void UUIItem::SetTraceChannel(TEnumAsByte<ETraceTypeQuery> InTraceChannel)
{
	if (traceChannel != InTraceChannel)
	{
		traceChannel = InTraceChannel;
	}
}

bool UUIItem::LineTraceUI(FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	auto inverseTf = GetComponentTransform().Inverse();
	auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
	auto localSpaceRayEnd = inverseTf.TransformPosition(End);

	//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false);//just for test
	//start and end point must be different side of X plane
	if (FMath::Sign(localSpaceRayOrigin.X) != FMath::Sign(localSpaceRayEnd.X))
	{
		auto result = FMath::LinePlaneIntersection(localSpaceRayOrigin, localSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
		//hit point inside rect area
		if (result.Y > GetLocalSpaceLeft() && result.Y < GetLocalSpaceRight() && result.Z > GetLocalSpaceBottom() && result.Z < GetLocalSpaceTop())
		{
			OutHit.TraceStart = Start;
			OutHit.TraceEnd = End;
			OutHit.Actor = GetOwner();
			OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
			OutHit.Location = GetComponentTransform().TransformPosition(result);
			OutHit.Normal = GetComponentTransform().TransformVector(FVector(1, 0, 0));
			OutHit.Normal.Normalize();
			OutHit.Distance = FVector::Distance(Start, OutHit.Location);
			OutHit.ImpactPoint = OutHit.Location;
			OutHit.ImpactNormal = OutHit.Normal;
			return true;
		}
	}
	return false;
}

ULGUICanvas* UUIItem::GetRenderCanvas()const
{
	return RenderCanvas.Get();
}

bool UUIItem::IsScreenSpaceOverlayUI()const
{
	if (!RenderCanvas.IsValid())return false;
	return RenderCanvas->IsRenderToScreenSpace();
}
bool UUIItem::IsRenderTargetUI()const
{
	if (!RenderCanvas.IsValid())return false;
	return RenderCanvas->IsRenderToRenderTarget();
}
bool UUIItem::IsWorldSpaceUI()const
{
	if (!RenderCanvas.IsValid())return false;
	return RenderCanvas->IsRenderToWorldSpace();
}

#pragma region UICanvasGroup
void UUIItem::OnCanvasGroupInteractableStateChange()
{
	auto NewInteractable = CanvasGroup.IsValid() ? CanvasGroup->GetFinalInteractable() : true;
	if (bIsGroupAllowInteraction != NewInteractable)
	{
		bIsGroupAllowInteraction = NewInteractable;
		CallUILifeCycleBehavioursInteractionStateChanged();
	}
}
#pragma endregion UICanvasGroup

#pragma region UIActive

void UUIItem::OnChildActiveStateChanged(UUIItem* child)
{
	CallUILifeCycleBehavioursChildActiveInHierarchyStateChanged(child, child->GetIsUIActiveInHierarchy());
}

void UUIItem::CheckUIActiveState()
{
	auto thisUIActiveState = this->GetIsUIActiveInHierarchy();
	CheckChildrenUIActiveRecursive(thisUIActiveState);
}

void UUIItem::CheckChildrenUIActiveRecursive(bool InUpParentUIActive)
{
	for (auto& uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{		//state is changed
			if (uiChild->bIsUIActive &&//when child is active, then parent's active state can affect child
				(uiChild->bAllUpParentUIActive != InUpParentUIActive)//state change
				)
			{
				uiChild->bAllUpParentUIActive = InUpParentUIActive;
				//apply for state change
				uiChild->ApplyUIActiveState(true);
				//affect children
				uiChild->CheckChildrenUIActiveRecursive(uiChild->GetIsUIActiveInHierarchy());
				//callback for parent
				this->OnChildActiveStateChanged(uiChild);
			}
			//state not changed
			else
			{
				uiChild->bAllUpParentUIActive = InUpParentUIActive;
				//apply for state change
				uiChild->ApplyUIActiveState(false);
				//affect children
				uiChild->CheckChildrenUIActiveRecursive(uiChild->GetIsUIActiveInHierarchy());
			}
		}
	}
}
void UUIItem::SetIsUIActive(bool active)
{
	if (bIsUIActive != active)
	{
		bIsUIActive = active;
		if (bAllUpParentUIActive)//state change only happens when up parent is active
		{
			ApplyUIActiveState(true);
			//affect children
			CheckChildrenUIActiveRecursive(bIsUIActive);
			//callback for parent
			if (ParentUIItem.IsValid())
			{
				ParentUIItem->OnChildActiveStateChanged(this);
			}
		}
		else
		{
			//nothing
		}
	}
}

void UUIItem::ApplyUIActiveState(bool InStateChange)
{
#if WITH_EDITOR
	//modify inactive actor's name
	auto Actor = GetOwner();
	if (Actor != nullptr && this == Actor->GetRootComponent())
	{
		auto bHiddenEdTemporary_Property = FindFProperty<FBoolProperty>(AActor::StaticClass(), TEXT("bHiddenEdTemporary"));
		bHiddenEdTemporary_Property->SetPropertyValue_InContainer(Actor, !GetIsUIActiveInHierarchy());
		//Actor->SetIsTemporarilyHiddenInEditor(!GetIsUIActiveInHierarchy());
	}
#endif
	if (InStateChange)
	{
		//callback
		CallUILifeCycleBehavioursActiveInHierarchyStateChanged();
		//canvas update
		MarkCanvasUpdate(false, false, false, true);
	}
}

#if WITH_EDITOR
void UUIItem::SetIsTemporarilyHiddenInEditor_Recursive_By_IsUIActiveState()
{
	ApplyUIActiveState(this->GetIsUIActiveInHierarchy());
	//affect children
	for (auto& uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{
			uiChild->SetIsTemporarilyHiddenInEditor_Recursive_By_IsUIActiveState();
		}
	}
}
#endif

FDelegateHandle UUIItem::RegisterUIActiveStateChanged(const FUIItemActiveInHierarchyStateChangedDelegate& InCallback)\
{
	return UIActiveInHierarchyStateChangedDelegate.Add(InCallback);
}
FDelegateHandle UUIItem::RegisterUIActiveStateChanged(const TFunction<void(bool)>& InCallback)
{
	return UIActiveInHierarchyStateChangedDelegate.AddLambda(InCallback);
}
void UUIItem::UnregisterUIActiveStateChanged(const FDelegateHandle& InHandle)
{
	UIActiveInHierarchyStateChangedDelegate.Remove(InHandle);
}

#pragma endregion UIActive

void UUIItem::SetWidget(const FUIWidget& inWidget)
{
	SetPivot(inWidget.pivot);

	switch (inWidget.anchorHAlign)
	{
	default:
	case UIAnchorHorizontalAlign::Left:
		SetHorizontalAnchorMinMax(FVector2D(0.0f, 0.0f));
		break;
	case UIAnchorHorizontalAlign::Center:
		SetHorizontalAnchorMinMax(FVector2D(0.5f, 0.5f));
		break;
	case UIAnchorHorizontalAlign::Right:
		SetHorizontalAnchorMinMax(FVector2D(1.0f, 1.0f));
		break;
	case UIAnchorHorizontalAlign::Stretch:
		SetHorizontalAnchorMinMax(FVector2D(0.0f, 1.0f));
		break;
	}

	switch (inWidget.anchorVAlign)
	{
	default:
	case UIAnchorVerticalAlign::Top:
		SetVerticalAnchorMinMax(FVector2D(1.0f, 1.0f));
		break;
	case UIAnchorVerticalAlign::Middle:
		SetVerticalAnchorMinMax(FVector2D(0.5f, 0.5f));
		break;
	case UIAnchorVerticalAlign::Bottom:
		SetVerticalAnchorMinMax(FVector2D(0.0f, 0.0f));
		break;
	case UIAnchorVerticalAlign::Stretch:
		SetVerticalAnchorMinMax(FVector2D(0.0f, 1.0f));
		break;
	}

	if (inWidget.anchorHAlign == UIAnchorHorizontalAlign::Stretch)
	{
		SetHorizontalAnchoredPosition(inWidget.anchorOffsetX);
		SetWidth(inWidget.width);
		SetAnchorLeft(inWidget.stretchLeft);
		SetAnchorRight(inWidget.stretchRight);
	}
	else
	{
		SetAnchorLeft(inWidget.stretchLeft);
		SetAnchorRight(inWidget.stretchRight);
		SetHorizontalAnchoredPosition(inWidget.anchorOffsetX);
		SetWidth(inWidget.width);
	}
	if (inWidget.anchorVAlign == UIAnchorVerticalAlign::Stretch)
	{
		SetVerticalAnchoredPosition(inWidget.anchorOffsetY);
		SetHeight(inWidget.height);
		SetAnchorTop(inWidget.stretchTop);
		SetAnchorBottom(inWidget.stretchBottom);
	}
	else
	{
		SetAnchorTop(inWidget.stretchTop);
		SetAnchorBottom(inWidget.stretchBottom);
		SetVerticalAnchoredPosition(inWidget.anchorOffsetY);
		SetHeight(inWidget.height);
	}
}
void UUIItem::SetAnchorHAlign(UIAnchorHorizontalAlign align)
{
	switch (align)
	{
	default:
	case UIAnchorHorizontalAlign::Left:
		SetHorizontalAnchorMinMax(FVector2D(0.0f, 0.0f));
		break;
	case UIAnchorHorizontalAlign::Center:
		SetHorizontalAnchorMinMax(FVector2D(0.5f, 0.5f));
		break;
	case UIAnchorHorizontalAlign::Right:
		SetHorizontalAnchorMinMax(FVector2D(1.0f, 1.0f));
		break;
	case UIAnchorHorizontalAlign::Stretch:
		SetHorizontalAnchorMinMax(FVector2D(0.0f, 1.0f));
		break;
	}
}
void UUIItem::SetAnchorVAlign(UIAnchorVerticalAlign align)
{
	switch (align)
	{
	default:
	case UIAnchorVerticalAlign::Top:
		SetVerticalAnchorMinMax(FVector2D(1.0f, 1.0f));
		break;
	case UIAnchorVerticalAlign::Middle:
		SetVerticalAnchorMinMax(FVector2D(0.5f, 0.5f));
		break;
	case UIAnchorVerticalAlign::Bottom:
		SetVerticalAnchorMinMax(FVector2D(0.0f, 0.0f));
		break;
	case UIAnchorVerticalAlign::Stretch:
		SetVerticalAnchorMinMax(FVector2D(0.0f, 1.0f));
		break;
	}
}

#pragma region TweenAnimation
#include "LTweenManager.h"
ULTweener* UUIItem::WidthTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetWidth), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetWidth), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::HeightTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetHeight), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetHeight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::HorizontalAnchoredPositionTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetHorizontalAnchoredPosition), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetHorizontalAnchoredPosition), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::VerticalAnchoredPositionTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetVerticalAnchoredPosition), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetVerticalAnchoredPosition), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::AnchoredPositionTo(FVector2D endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenVector2DGetterFunction::CreateUObject(this, &UUIItem::GetAnchoredPosition), FLTweenVector2DSetterFunction::CreateUObject(this, &UUIItem::SetAnchoredPosition), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::PivotTo(FVector2D endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenVector2DGetterFunction::CreateUObject(this, &UUIItem::GetPivot), FLTweenVector2DSetterFunction::CreateUObject(this, &UUIItem::SetPivot), endValue, duration)->SetEase(ease)->SetDelay(delay);
}

ULTweener* UUIItem::AnchorLeftTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetAnchorLeft), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetAnchorLeft), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::AnchorRightTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetAnchorRight), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetAnchorRight), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::AnchorTopTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetAnchorTop), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetAnchorTop), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
ULTweener* UUIItem::AnchorBottomTo(float endValue, float duration, float delay, LTweenEase ease)
{
	return ULTweenManager::To(this, FLTweenFloatGetterFunction::CreateUObject(this, &UUIItem::GetAnchorBottom), FLTweenFloatSetterFunction::CreateUObject(this, &UUIItem::SetAnchorBottom), endValue, duration)->SetEase(ease)->SetDelay(delay);
}
#pragma endregion




UUIItemEditorHelperComp::UUIItemEditorHelperComp()
{
	bSelectable = false;
	this->bIsEditorOnly = true;
}

#if WITH_EDITOR
#include "Layout/LGUICanvasScaler.h"
FPrimitiveSceneProxy* UUIItemEditorHelperComp::CreateSceneProxy()
{
	class FUIItemSceneProxy : public FPrimitiveSceneProxy
	{
	public:
		SIZE_T GetTypeHash() const override
		{
			static size_t UniquePointer;
			return reinterpret_cast<size_t>(&UniquePointer);
		}

		FUIItemSceneProxy(UUIItem* InComponent, UPrimitiveComponent* InPrimitive)
			: FPrimitiveSceneProxy(InPrimitive)
		{
			bWillEverBeLit = false;
			Component = InComponent;
		}

		virtual void GetDynamicMeshElements(const TArray<const FSceneView*>& Views, const FSceneViewFamily& ViewFamily, uint32 VisibilityMap, FMeshElementCollector& Collector) const override
		{
			return;
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			Result.bDrawRelevance = true;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }
	private:
		TWeakObjectPtr<UUIItem> Component;
	};

	return new FUIItemSceneProxy(this->Parent, this);
}
#endif

UBodySetup* UUIItemEditorHelperComp::GetBodySetup()
{
	UpdateBodySetup();
	return BodySetup;
}
void UUIItemEditorHelperComp::UpdateBodySetup()
{
	if (!IsValid(Parent))return;
	if (!IsValid(BodySetup))
	{
		BodySetup = NewObject<UBodySetup>(this, NAME_None, RF_Transient);
		BodySetup->CollisionTraceFlag = CTF_UseSimpleAsComplex;
		FKBoxElem Box = FKBoxElem();
		Box.SetTransform(FTransform::Identity);
		BodySetup->AggGeom.BoxElems.Add(Box);
	}
	FKBoxElem* BoxElem = BodySetup->AggGeom.BoxElems.GetData();

	auto Center = Parent->GetLocalSpaceCenter();
	auto Origin = FVector(0, Center.X, Center.Y);

	BoxElem->X = 0.0f;
	BoxElem->Y = Parent->GetWidth();
	BoxElem->Z = Parent->GetHeight();

	BoxElem->Center = Origin;
}
FBoxSphereBounds UUIItemEditorHelperComp::CalcBounds(const FTransform& LocalToWorld) const
{
	if (!IsValid(Parent))return FBoxSphereBounds(EForceInit::ForceInit);
	auto Center = Parent->GetLocalSpaceCenter();
	auto Origin = FVector(0, Center.X, Center.Y);
	return FBoxSphereBounds(Origin, FVector(1, Parent->GetWidth() * 0.5f, Parent->GetHeight() * 0.5f), (Parent->GetWidth() > Parent->GetHeight() ? Parent->GetWidth() : Parent->GetHeight()) * 0.5f).TransformBy(LocalToWorld);
}

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
