// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/Components/LexWidget.h"
#include "LGUI.h"
#include "Core/Components/LexCanvas.h"
#include "Core/LexUISettings.h"
#include "Core/LexUIManager.h"
#include "LTweenManager.h"
#include "Core/LexUIClipData.h"
#include "Core/Components/LexLayout.h"
#include "Core/Components/LexVisual.h"
#include "Components/SceneComponent.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexWidgetPresenterComponentBase.h"
#if WITH_EDITOR
#include "UObject/UnrealType.h"
#endif



ULexWidget::ULexWidget()
{
	bFlattenHierarchyIndexDirty = true;
	bNeedSortUIChildren = true;
	bIsCanvasWidget = false;
	bCacheWidthDirty = true;
	bCacheHeightDirty = true;
	bCacheAnchorOffsetBottomDirty = true;
	bCacheAnchorOffsetTopDirty = true;
	bCacheAnchorOffsetLeftDirty = true;
	bCacheAnchorOffsetRightDirty = true;

	bClipDirty = true;
	bNeedRecreateClip = true;
}

void ULexWidget::BeginPlay()
{
	check(!bHasBegunPlay);
	bHasBegunPlay = true;

	/**
	 * Use Count here instead of using Components.Num(), in case:
	 * When add new component in Awake, the new component will automatically do BeginPlay because Widget's bHasBegunPlay is already set to true,
	 * so use Count will skip those components which are added in Awake.
	 */
	for (int i = 0, Count = Components.Num(); i < Count; i++)
	{
		auto& Component = Components[i];
		Component->BeginPlay();
	}
	
	if (IsValid(LayoutContainer))
	{
		LayoutContainer->BeginPlay();
	}
	if (IsValid(LayoutSelf))
	{
		LayoutSelf->BeginPlay();
	}
	if (IsValid(Visual))
	{
		Visual->BeginPlay();
	}
}

void ULexWidget::EndPlay()
{
	bHasBegunPlay = false;

	for (auto Component : Components)
	{
		Component->EndPlay();
	}
	
	if (IsValid(LayoutContainer))
	{
		LayoutContainer->EndPlay();
	}
	if (IsValid(LayoutSelf))
	{
		LayoutSelf->EndPlay();
	}
	if (IsValid(Visual))
	{
		Visual->EndPlay();
	}
}

#pragma region CallbackEvents
void ULexWidget::Call_InteractableChanged()
{
	OnInteractableChangedEvent.Broadcast(this->GetInteractableInHierarchy());
}
void ULexWidget::Call_TransformChanged()
{
	OnTransformChangedEvent.Broadcast();
}

void ULexWidget::Call_DimensionsChanged(bool InPivotChanged, bool InWidthChanged, bool InHeightChanged)
{
	OnDimensionChangedEvent.Broadcast(InPivotChanged, InWidthChanged, InHeightChanged);

	if (Parent.IsValid())
	{
		Parent->Call_ChildDimensionsChanged(this, InPivotChanged, InWidthChanged, InHeightChanged);
	}
}

void ULexWidget::Call_ChildDimensionsChanged(ULexWidget* Child, bool InPivotChanged, bool InWidthChanged, bool InHeightChanged)
{
	OnChildDimensionChangedEvent.Broadcast(Child, InPivotChanged, InWidthChanged, InHeightChanged);
}

void ULexWidget::Call_AttachmentChanged()
{
	OnAttachmentChangedEvent.Broadcast();
}

void ULexWidget::Call_SiblingIndexChanged()
{
	OnSiblingIndexChangedEvent.Broadcast();
}

void ULexWidget::CollectChildrenWidgets(ULexWidget* Target, TArray<ULexWidget*>& OutAllChildrenWidgets, bool IncludeTarget)
{
	if (IncludeTarget)
	{
		OutAllChildrenWidgets.Add(Target);
	}
	for (auto& Child : Target->GetChildren())
	{
		CollectChildrenWidgets(Child, OutAllChildrenWidgets, true);
	}
}

void ULexWidget::Call_WidgetActiveChanged()
{
	OnWidgetActiveChangedEvent.Broadcast(this->GetWidgetActiveInHierarchy());
}
void ULexWidget::Call_RaycastableChanged()
{
	OnRaycastableChangedEvent.Broadcast(this->GetRaycastableInHierarchy());
}
#pragma endregion


void ULexWidget::CalculateFlattenHierarchyIndex_Recursive(int& index)const
{
	if (this->FlattenHierarchyIndex != index)
	{
		this->FlattenHierarchyIndex = index;
	}
	EnsureUIChildrenSorted();
	for (auto& child : Children)
	{
		if (IsValid(child))
		{
			index++;
			child->CalculateFlattenHierarchyIndex_Recursive(index);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("LexWidget CalculateFlattenHierarchyIndex"), STAT_LexWidgetCalculateFlattenHierarchyIndex, STATGROUP_LGUI);
void ULexWidget::RecalculateFlattenHierarchyIndex()const
{
	SCOPE_CYCLE_COUNTER(STAT_LexWidgetCalculateFlattenHierarchyIndex);

	this->bFlattenHierarchyIndexDirty = false;
	int tempIndex = this->FlattenHierarchyIndex;
	this->CalculateFlattenHierarchyIndex_Recursive(tempIndex);
}

int32 ULexWidget::GetFlattenHierarchyIndex()const
{
	if (RootWidget.IsValid())
	{
		if (RootWidget->bFlattenHierarchyIndexDirty)
		{
			RootWidget->RecalculateFlattenHierarchyIndex();
		}
	}
	return this->FlattenHierarchyIndex;
}

void ULexWidget::MarkFlattenHierarchyIndexDirty()
{
	if (RootWidget.IsValid())
	{
		RootWidget->bFlattenHierarchyIndexDirty = true;
	}
	//tell canvas to update
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkCanvasUpdate(true);
		//if this LexWidget have a LGUICanvas, then we need to tell the upper canvas that hierarchy order change, in order to sort render order between canvas
		if (this->bIsCanvasWidget)
		{
			if (RenderCanvas->GetParentCanvas().IsValid())
			{
				RenderCanvas->GetParentCanvas()->MarkCanvasUpdate(true);
			}
		}
	}
}



void ULexWidget::ApplySiblingIndex()
{
	if (Parent.IsValid())
	{
		if (Parent->Children.Num() == 0)
		{
			Parent->Children.Add(this);
			if (SiblingIndex != 0)
			{
				this->SiblingIndex = 0;
				this->Call_SiblingIndexChanged();
			}
		}
		else
		{
			Parent->EnsureUIChildrenValid();
			Parent->EnsureUIChildrenSorted();
			SiblingIndex = FMath::Clamp(SiblingIndex, 0, Parent->Children.Num() - 1);
			Parent->Children.Remove(this);
			Parent->Children.Insert(this, SiblingIndex);
			bool anythingChange = false;
			for (int i = 0; i < Parent->Children.Num(); i++)
			{
				if (Parent->Children[i]->SiblingIndex != i)
				{
					Parent->Children[i]->SiblingIndex = i;
					Parent->Children[i]->Call_SiblingIndexChanged();
					anythingChange = true;
				}
			}
			//flatten hierarchy index
			if (anythingChange)
			{
				MarkFlattenHierarchyIndexDirty();
			}
		}
	}
	else
	{
		if (SiblingIndex != 0)
		{
			SiblingIndex = 0;
			this->Call_SiblingIndexChanged();
		}
	}
}

void ULexWidget::SetAsFirstSibling()
{
	SetSiblingIndex(0);
}
void ULexWidget::SetAsLastSibling()
{
	if (Parent.IsValid())
	{
		SetSiblingIndex(Parent->Children.Num() - 1);
	}
}

FString ULexWidget::GetPathDisplayName(const UObject* StopOuter) const
{
	auto OuterPathName = GetOuter()->GetPathName(StopOuter);
	TStringBuilder<256> Result;
	Result.Append(OuterPathName);
	Result.AppendChar('/');
	TArray<const ULexWidget*> WidgetChain;
	auto TempParent = this;
	while (TempParent != nullptr)
	{
		WidgetChain.Add(TempParent);
		TempParent = TempParent->GetParent();
	}
	for (int i = WidgetChain.Num() - 1; i >= 0; i--)
	{
		auto Widget = WidgetChain[i];
		Result.Append(Widget->GetDisplayName());
		if (i != 0)
		{
			Result.AppendChar('/');
		}
	}
	return Result.ToString();
}

ULexWidget* ULexWidget::FindChildByDisplayName(const FString& InName, bool IncludeChildren)const
{
	int indexOfFirstSlash;
	if (InName.FindChar('/', indexOfFirstSlash))
	{
		auto firstLayerName = InName.Left(indexOfFirstSlash);
		for (auto& childItem : Children)
		{
			if (childItem->DisplayName.Equals(firstLayerName, ESearchCase::CaseSensitive))
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
			for (auto& childItem : Children)
			{
				if (childItem->DisplayName.Equals(InName, ESearchCase::CaseSensitive))
				{
					return childItem;
				}
			}
		}
	}
	return nullptr;
}
ULexWidget* ULexWidget::FindChildByDisplayNameWithChildren_Internal(const FString& InName)const
{
	for (auto& childItem : Children)
	{
		if (childItem->DisplayName.Equals(InName, ESearchCase::CaseSensitive))
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
TArray<ULexWidget*> ULexWidget::FindChildArrayByDisplayName(const FString& InName, bool IncludeChildren)const
{
	TArray<ULexWidget*> resultArray;
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
			EnsureUIChildrenSorted();//make sure sorted, so result is predictable
			for (auto& childItem : Children)
			{
				if (childItem->DisplayName.Equals(InName, ESearchCase::CaseSensitive))
				{
					resultArray.Add(childItem);
				}
			}
		}
	}
	return resultArray;
}
void ULexWidget::FindChildArrayByDisplayNameWithChildren_Internal(const FString& InName, TArray<ULexWidget*>& OutResultArray)const
{
	EnsureUIChildrenSorted();//make sure sorted, so result is predictable
	for (auto& childItem : Children)
	{
		if (childItem->DisplayName.Equals(InName, ESearchCase::CaseSensitive))
		{
			OutResultArray.Add(childItem);
		}
		else
		{
			childItem->FindChildArrayByDisplayNameWithChildren_Internal(InName, OutResultArray);
		}
	}
}

void ULexWidget::MarkAllDirtyRecursive()
{
	MarkAllDirty();
	
	for (auto& uiChild : Children)
	{
		if (IsValid(uiChild))
		{
			uiChild->MarkAllDirtyRecursive();
		}
	}
}

void ULexWidget::MarkAllDirty()
{
	bFlattenHierarchyIndexDirty = true;
	bClipDirty = true;

	bCacheWidthDirty = true;
	bCacheHeightDirty = true;
	bCacheAnchorOffsetLeftDirty = true;
	bCacheAnchorOffsetRightDirty = true;
	bCacheAnchorOffsetBottomDirty = true;
	bCacheAnchorOffsetTopDirty = true;
	
	if (IsValid(Visual))
	{
		Visual->MarkAllDirty();
	}
}

void ULexWidget::MarkRenderModeChangeRecursive(ULexCanvas* Canvas, ELexRenderMode OldRenderMode, ELexRenderMode NewRenderMode)
{
	if (this->RenderCanvas == Canvas)
	{
		MarkAllDirty();
		for (auto& uiChild : Children)
		{
			if (IsValid(uiChild))
			{
				uiChild->MarkRenderModeChangeRecursive(Canvas, OldRenderMode, NewRenderMode);
			}
		}
	}
}


void ULexWidget::PostLoad()
{
	Super::PostLoad();
}

void ULexWidget::BeginDestroy()
{
	if (bHasBegunPlay || bIsRegistered)
	{
		auto World = this->GetWorld();
		if (World->WorldType == EWorldType::Inactive)
		{
			DestroyWidget();
			Super::BeginDestroy();
			return;
		}
		auto WorldName = World ? World->GetName() : TEXT("null");
		auto Manager = ULexUIManagerWorldSubsystem::GetInstance(World);
		auto ManagerName = Manager ? Manager->GetName() : TEXT("null");

		UE_LOG(LGUI, Error, TEXT("ULexWidget %s is not properly destroyed! Missing DestroyWidget call. World:%s, WorldType:%d, Manager:%s. Auto cleanup in BeginDestroy."),
			*GetPathDisplayName(), *WorldName, World ? World->WorldType : -1, *ManagerName);

		if (GEngine)
		{
			GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red
				, FString::Printf(TEXT("ULexWidget %s is not properly destroyed! Missing DestroyWidget call, auto cleaned up in BeginDestroy. World:%s, Manager:%s")
					, *GetPathDisplayName(), *WorldName, *ManagerName));
		}

		// Fallback for cases where caller forgot DestroyWidget().
		DestroyWidget();
	}
	Super::BeginDestroy();
}

void ULexWidget::DestroyWidget()
{
	struct LOCAL
	{
		static void UnregisterRecursive(ULexWidget* Widget)
		{
			if (Widget->bIsRegistered)
			{
				Widget->OnUnregister();
			}
			for (auto Child : Widget->GetChildren())
			{
				UnregisterRecursive(Child);
			}
		}
		static void EndPlayRecursive(ULexWidget* Widget)
		{
			if (Widget->bHasBegunPlay)
			{
				Widget->EndPlay();
			}
			for (auto Child : Widget->GetChildren())
			{
				EndPlayRecursive(Child);
			}
			// Mark all sub-objects (Components, Layout, Visual, etc.) as garbage so TWeakObjectPtr/IsValid returns false immediately.
			TArray<UObject*> SubObjects;
			GetObjectsWithOuter(Widget, SubObjects, true);
			for (UObject* SubObj : SubObjects)
			{
				if (IsValid(SubObj))
				{
					SubObj->MarkAsGarbage();
				}
			}
			Widget->MarkAsGarbage();
		}
	};
	LOCAL::UnregisterRecursive(this);
	this->SetParent(nullptr);
	LOCAL::EndPlayRecursive(this);
}

UWorld* ULexWidget::GetWorld() const
{
	auto OuterWorld = GetTypedOuter<UWorld>();
	return OuterWorld;
}

#if WITH_EDITOR
void ULexWidget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		//MarkAllDirtyRecursive();
		auto MemberName = PropertyChangedEvent.GetMemberPropertyName();
		auto PropertyName = PropertyChangedEvent.GetPropertyName();

		static const FName AnchorDataName = GET_MEMBER_NAME_CHECKED(ULexWidget, AnchorData);
		static const FName WidgetActiveName = GET_MEMBER_NAME_CHECKED(ULexWidget, bWidgetActive);
		static const FName RaycastableName = GET_MEMBER_NAME_CHECKED(ULexWidget, Raycastable);
		static const FName ClippingName = GET_MEMBER_NAME_CHECKED(ULexWidget, Clipping);
		static const FName ClippingCornerRadiusName = GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingCornerRadius);
		static const FName ClippingMarginName = GET_MEMBER_NAME_CHECKED(ULexWidget, ClippingMargin);
		static const FName VisualName = GET_MEMBER_NAME_CHECKED(ULexWidget, Visual);
		static const FName LayoutContainerName = GET_MEMBER_NAME_CHECKED(ULexWidget, LayoutContainer);
		static const FName LayoutSelfName = GET_MEMBER_NAME_CHECKED(ULexWidget, LayoutSelf);
		static const FName IgnoreLayoutName = GET_MEMBER_NAME_CHECKED(ULexWidget, bIgnoreLayout);
		static const FName InteractableName = GET_MEMBER_NAME_CHECKED(ULexWidget, Interactable);
		static const FName RenderOpacityName = GET_MEMBER_NAME_CHECKED(ULexWidget, RenderOpacity);
		static const FName IgnoreParentRenderOpacityName = GET_MEMBER_NAME_CHECKED(ULexWidget, bIgnoreParentRenderOpacity);

		if (MemberName == AnchorDataName
		|| MemberName == WidgetActiveName
		|| MemberName == ClippingCornerRadiusName
		|| MemberName == ClippingMarginName
		)
		{
			this->MarkAnchorDataChanged_Recursive(true, true, true);
			this->MarkLayoutForRebuild(this);
			this->MarkClipDirty(false);
		}
		else if (MemberName == ClippingName)
		{
			MarkClipDirty(true);
		}
		else if (MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, SiblingIndex))
		{
			this->Call_SiblingIndexChanged();
			ApplySiblingIndex();
		}
		else if (MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, RelativeLocation) || MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, RelativeRotation) || MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, RelativeScale))
		{
			CalculateAnchorFromTransform();
			CalculateObjectToWorldTransform();
			OnUpdateTransform();
			MarkTransformChanged();
			MarkLayoutForRebuild(this);
		}
		else if (MemberName == VisualName)
		{
			if (IsValid(Visual))
			{
				if (RenderCanvas.IsValid())
				{
					RenderCanvas->RegisterVisual(Visual);
				}
				if (bHasBegunPlay)
				{
					Visual->BeginPlay();
				}
				Visual->Call_OnRegister();
			}
			MarkDimensionChanged(false, true, true);//change Visual could cause LayoutSelf size change
			MarkLayoutForRebuild(this);
		}
		else if (MemberName == LayoutContainerName)
		{
			if (IsValid(LayoutContainer))
			{
				if (bHasBegunPlay)
				{
					LayoutContainer->BeginPlay();
				}
				LayoutContainer->Call_OnRegister();
			}
			MarkDimensionChanged(false, true, true);//change LayoutContainer could cause LayoutSelf size change
			MarkLayoutForRebuild(this);
		}
		else if (MemberName == LayoutSelfName)
		{
			if (IsValid(LayoutSelf))
			{
				if (bHasBegunPlay)
				{
					LayoutSelf->BeginPlay();
				}
				LayoutSelf->Call_OnRegister();
			}
			MarkDimensionChanged(false, true, true);//change LayoutSelf could cause size change
			MarkLayoutForRebuild(this);
		}
		else if (MemberName == IgnoreLayoutName)
		{
			MarkDimensionChanged(false, true, true);//change LayoutSelf could cause size change
			MarkLayoutForRebuild(this);
		}
		if (MemberName == AnchorDataName)
		{
			CalculateTransformFromAnchor();
			this->CalculateObjectToWorldTransform();
		}
		if (MemberName == WidgetActiveName)
		{
			CalculateWidgetActive_Recursive();
		}
		if (MemberName == RaycastableName)
		{
			CalculateRaycastable_Recursive();
		}
		if (MemberName == InteractableName)
		{
			CalculateInteractable_Recursive();
		}
		if (MemberName == RenderOpacityName || MemberName == IgnoreParentRenderOpacityName)
		{
			MarkRenderOpacityDirty_Recursive();
		}
		ULexUIManagerObject::AddOneShotTickFunction([WeakThis = MakeWeakObjectPtr(this)]()
		{
			if (WeakThis.IsValid())
			{
				WeakThis->MarkCanvasUpdate(true);
			}
		}, 1);
	}
}

void ULexWidget::PostEditChangeChainProperty(struct FPropertyChangedChainEvent& PropertyChangedEvent)
{
	Super::PostEditChangeChainProperty(PropertyChangedEvent);
}

void ULexWidget::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);

	const FName MemberName = PropertyAboutToChange->GetFName();
	if (MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, Visual))
	{
		if (IsValid(Visual))
		{
			if (RenderCanvas.IsValid())
			{
				RenderCanvas->MarkVisualWillChange(Visual);
				RenderCanvas->UnregisterVisual(Visual);
			}
			if (bHasBegunPlay)
			{
				Visual->EndPlay();
			}
			Visual->Call_OnUnregister();
		}
	}
	else if (MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, LayoutContainer))
	{
		if (IsValid(LayoutContainer))
		{
			if (bHasBegunPlay)
			{
				LayoutContainer->EndPlay();
			}
			LayoutContainer->Call_OnUnregister();
		}
	}
	else if (MemberName == GET_MEMBER_NAME_CHECKED(ULexWidget, LayoutSelf))
	{
		if (IsValid(LayoutSelf))
		{
			if (bHasBegunPlay)
			{
				LayoutSelf->EndPlay();
			}
			LayoutSelf->Call_OnUnregister();
		}
	}
}

bool ULexWidget::CanEditChange(const FProperty* InProperty) const
{
	bool bIsEditable = Super::CanEditChange(InProperty);
	return bIsEditable;
}

bool ULexWidget::CanEditChange(const FEditPropertyChain& PropertyChain) const
{
	bool bIsEditable = UObject::CanEditChange( PropertyChain );
	return bIsEditable;
}

void ULexWidget::PostEditUndo()
{
	Super::PostEditUndo();
	if (Parent.IsValid())
	{
		//restore SiblingIndex
		Parent->Children.Remove(this);
		Parent->Children.Insert(this, SiblingIndex);
		for (int i = 0; i < Parent->Children.Num(); i++)
		{
			auto& UIChild = Parent->Children[i];
			if (UIChild->SiblingIndex != i)
			{
				UIChild->SiblingIndex = i;
			}
		}
	}
	// Re-register if unregistered (e.g., undo of a delete operation via DeleteForUndo).
	// bIsRegistered is not a UPROPERTY so it is not saved/restored by the undo system;
	// after soft-delete it remains false, so we need to call OnRegister() explicitly.
	if (!bIsRegistered)
	{
		struct LOCAL
		{
			static void RegisterRecursive(ULexWidget* Widget)
			{
				if (!Widget->bIsRegistered)
				{
					Widget->OnRegister();
				}
				for (auto& Child : Widget->Children)
				{
					if (IsValid(Child))
					{
						RegisterRecursive(Child);
					}
				}
			}
		};
		LOCAL::RegisterRecursive(this);
	}
}

void ULexWidget::PostRename(UObject* OldOuter, const FName OldName)
{
	Super::PostRename(OldOuter, OldName);
}

void ULexWidget::EnsureChildrenAfterTransaction()
{
	struct LOCAL
	{
		static void CheckIt(ULexWidget* Widget)
		{
			for (int i = 0;i < Widget->Children.Num(); i++)
			{
				auto Child = Widget->Children[i];
				if (!IsValid(Child))
				{
					Widget->Children.RemoveAt(i);
					i--;
					continue;
				}
				Child->SiblingIndex = i;
				CheckIt(Child);
			}
		}
	};
	LOCAL::CheckIt(this);
}

void ULexWidget::EnsureDataForRebuild()
{
	check(this == RootWidget);
	struct LOCAL
	{
		static void MarkLayoutDirtyRecursive(ULexWidget* Widget)
		{
			MarkLayoutForRebuild(Widget);
			for (auto& uiChild : Widget->Children)
			{
				if (IsValid(uiChild))
				{
					MarkLayoutDirtyRecursive(uiChild);
				}
			}
		}
		static void RenewRenderCanvas(ULexWidget* Widget)
		{
			auto ThisRenderCanvas = Widget->GetComponent<ULexCanvas>();
			Widget->RenewRenderCanvasRecursive(ThisRenderCanvas);
		}
		static void EnsureDataForRebuildRecursive(ULexWidget* Widget)
		{
			Widget->EnsureUIChildrenValid();
			Widget->bNeedSortUIChildren = true;
			Widget->EnsureUIChildrenSorted();
			if (Widget->bIsCanvasWidget && Widget->RenderCanvas.IsValid())
			{
				Widget->RenderCanvas->EnsureDataForRebuild();
			}

			for (auto& uiChild : Widget->Children)
			{
				if (IsValid(uiChild))
				{
					EnsureDataForRebuildRecursive(uiChild);
				}
			}
		}
		/** force refresh render canvas, remove from old and add to new */
		static void ForceRefreshRenderCanvasRecursive(ULexWidget* Widget)
		{
			auto NewRenderCanvas = Widget->GetComponentInParent<ULexCanvas>(true);
			Widget->SetRenderCanvas(NewRenderCanvas);

			for (auto& uiChild : Widget->Children)
			{
				if (IsValid(uiChild))
				{
					ForceRefreshRenderCanvasRecursive(uiChild);
				}
			}
		}
	};
	MarkAllDirtyRecursive();
	LOCAL::MarkLayoutDirtyRecursive(this);
	LOCAL::RenewRenderCanvas(this);
	LOCAL::EnsureDataForRebuildRecursive(this);
	LOCAL::ForceRefreshRenderCanvasRecursive(this);
	CalculateWidgetActive_Recursive();
	CalculateRaycastable_Recursive();
	CalculateInteractable_Recursive();
	CalculateObjectToWorldTransform();
}

#endif


#pragma region Transform
FVector ULexWidget::GetWorldLocation()const
{
	return GetWorldTransform().GetLocation();
}
FQuat ULexWidget::GetWorldRotation()const
{
	return GetWorldTransform().GetRotation();
}
FVector ULexWidget::GetWorldScale()const
{
	return GetWorldTransform().GetScale3D();
}

FVector ULexWidget::GetForwardVector() const
{
	return GetWorldTransform().GetRotation().GetForwardVector();
}

FVector ULexWidget::GetRightVector() const
{
	return GetWorldTransform().GetRotation().GetRightVector();
}

FVector ULexWidget::GetUpVector() const
{
	return GetWorldTransform().GetRotation().GetUpVector();
}

void ULexWidget::SetRelativeLocation(const FVector& Value)
{
	if (this->RelativeLocation != Value)
	{
		this->RelativeLocation = Value;
		this->CalculateObjectToWorldTransform();
		
		if (bCanSetAnchorFromTransform)
		{
			if (CalculateAnchorFromTransform())
			{
				if (Parent.IsValid() && Parent->GetLayoutContainer())//only position change, if parent contains LayoutContainer then we should rebuild layout, otherwise not
				{
					MarkLayoutForRebuild(this);
				}
			}
		}
	}
}
void ULexWidget::SetRelativeRotation(const FQuat& Value)
{
	if (this->RelativeRotation != Value)
	{
		this->RelativeRotation = Value;
		this->CalculateObjectToWorldTransform();
	}
}
void ULexWidget::SetRelativeScale(const FVector& Value)
{
	if (this->RelativeScale != Value)
	{
		this->RelativeScale = Value;
		this->CalculateObjectToWorldTransform();
	}
}
void ULexWidget::SetRelativeLocationAndRotation(const FVector& InLocation, const FQuat& InRotation)
{
	if (this->RelativeLocation != InLocation || this->RelativeRotation != InRotation)
	{
		this->RelativeLocation = InLocation;
		this->RelativeRotation = InRotation;
		this->CalculateObjectToWorldTransform();

		if (bCanSetAnchorFromTransform)
		{
			if (CalculateAnchorFromTransform())
			{
				if (Parent.IsValid() && Parent->GetLayoutContainer())//only position change, if parent contains LayoutContainer then we should rebuild layout, otherwise not
				{
					MarkLayoutForRebuild(this);
				}
			}
		}
	}
}

void ULexWidget::SetWorldLocation(const FVector& Value)
{
	auto WorldRotation = GetWorldRotation();
	SetWorldLocationAndRotation(Value, WorldRotation);
}
void ULexWidget::SetWorldRotation(const FQuat& Value)
{
	auto WorldPosition = GetWorldLocation();
	SetWorldLocationAndRotation(WorldPosition, Value);
}
void ULexWidget::SetWorldLocationAndRotation(const FVector& InLocation, const FQuat& InRotation)
{
	if (Parent.IsValid())
	{
		auto WorldToParentTransform = Parent->GetWorldTransform().Inverse();
		auto NewPosition = WorldToParentTransform.TransformPosition(InLocation);
		auto NewRotation = WorldToParentTransform.TransformRotation(InRotation);
		this->SetRelativeLocationAndRotation(NewPosition, NewRotation);
	}
	else
	{
		if (auto WidgetPresenterComponent = GetWidgetPresenterComponent())
		{
			auto WorldToParentTransform = WidgetPresenterComponent->GetComponentTransform().Inverse();
			auto NewPosition = WorldToParentTransform.TransformPosition(InLocation);
			auto NewRotation = WorldToParentTransform.TransformRotation(InRotation);
			this->SetRelativeLocationAndRotation(NewPosition, NewRotation);
		}
		else
		{
			this->SetRelativeLocationAndRotation(InLocation, InRotation);
		}
	}
}

FTransform ULexWidget::GetRelativeTransform()const
{
	return FTransform(RelativeRotation, RelativeLocation, RelativeScale);
}
const FTransform& ULexWidget::GetWorldTransform()const
{
	return ObjectToWorldTransform;
}

void ULexWidget::SetWorldTransform(const FTransform& InWorldTransform)
{
	if (Parent.IsValid())
	{
		auto WorldToParentTransform = Parent->GetWorldTransform().Inverse();
		auto LocalTransform = WorldToParentTransform * InWorldTransform;
		this->RelativeLocation = LocalTransform.GetLocation();
		this->RelativeRotation = LocalTransform.GetRotation();
		this->RelativeScale = LocalTransform.GetScale3D();

		ObjectToWorldTransform = InWorldTransform;
	}
	else
	{
		auto LocalTransform = InWorldTransform;
		this->RelativeLocation = LocalTransform.GetLocation();
		this->RelativeRotation = LocalTransform.GetRotation();
		this->RelativeScale = LocalTransform.GetScale3D();
		
		if (auto WidgetPresenterComponent = GetWidgetPresenterComponent())
		{
			ObjectToWorldTransform = WidgetPresenterComponent->GetComponentTransform() * LocalTransform;			
		}
		else
		{
			ObjectToWorldTransform = InWorldTransform;
		}
	}
	this->MarkTransformChanged();
}

void ULexWidget::SetParentBeforeRegister(ULexWidget* InParent)
{
	check(!bIsRegistered);
	if (Parent != InParent)
	{
		if (Parent.IsValid())
		{
			Parent->Children.Remove(this);
		}
		Parent = InParent;
		Parent->Children.Add(this);
	}
}

void ULexWidget::ApplySiblingIndexBeforeRegister_Recursive()
{
	for (int i = 0; i < Children.Num(); i++)
	{
		auto& Child = Children[i];
		Child->SiblingIndex = i;
		Child->ApplySiblingIndexBeforeRegister_Recursive();
	}
}

ULexUIBehaviour* ULexWidget::AddComponent(TSubclassOf<ULexUIBehaviour> ComponentClass, ULexUIBehaviour* ComponentTemplate)
{
	if (!*ComponentClass)
	{
		return nullptr;
	}
	if (ComponentTemplate && ComponentTemplate->GetClass() != *ComponentClass)
	{
		return nullptr;
	}

	EObjectFlags NewComponentFlags = RF_Public | RF_Transactional;
	if (HasAnyFlags(RF_ClassDefaultObject | RF_ArchetypeObject))
	{
		// Components created while building class defaults must be archetype/default-subobjects.
		// They also need to be public, otherwise Blueprint-generated templates can end up
		// referencing parent CDO private archetype objects that SavePackage rejects.
		NewComponentFlags |= (RF_Public | RF_DefaultSubObject | RF_ArchetypeObject);
	}

	const FName NewComponentName = MakeUniqueObjectName(this, ComponentClass, ComponentClass->GetFName());
	auto NewComponent = NewObject<ULexUIBehaviour>(this, ComponentClass, NewComponentName, NewComponentFlags, ComponentTemplate);
	Components.Add(NewComponent);
	if (bIsRegistered)
	{
		NewComponent->OnRegister();
	}
	if (bHasBegunPlay)
	{
		NewComponent->BeginPlay();
	}
	OnComponentsChangedEvent.Broadcast(ELexWidgetComponentsChangedType::Added);
	return NewComponent;
}

ULexUIBehaviour* ULexWidget::AddComponent(TSubclassOf<ULexUIBehaviour> ComponentClass)
{
	return AddComponent(ComponentClass, nullptr);
}

ULexUIBehaviour* ULexWidget::AddComponentByTemplate(ULexUIBehaviour* ComponentTemplate)
{
	return AddComponent(ComponentTemplate->GetClass(), ComponentTemplate);
}

void ULexWidget::RemoveComponent(ULexUIBehaviour* Component)
{
	auto Index = Components.Find(Component);
	if (Index < 0)return;
	Components.RemoveAt(Index);
	if (bHasBegunPlay)
	{
		Component->EndPlay();
	}
	Component->OnUnregister();
	OnComponentsChangedEvent.Broadcast(ELexWidgetComponentsChangedType::Removed);
}

void ULexWidget::MoveComponentToIndex(ULexUIBehaviour* Component, int32 NewIndex)
{
	const int32 SourceIndex = Components.Find(Component);
	if (SourceIndex < 0)
	{
		return;
	}

	const int32 TargetIndex = FMath::Clamp(NewIndex, 0, Components.Num() - 1);
	if (SourceIndex == TargetIndex)
	{
		return;
	}

	ULexUIBehaviour* MovingComponent = Components[SourceIndex];
	Components.RemoveAt(SourceIndex);
	Components.Insert(MovingComponent, FMath::Clamp(TargetIndex, 0, Components.Num()));
	OnComponentsChangedEvent.Broadcast(ELexWidgetComponentsChangedType::Reorder);
}

void ULexWidget::UpdateObjectToWorldTransform()
{
	auto LocalTransform = GetRelativeTransform();
	if (Parent.IsValid())
	{
		ObjectToWorldTransform = LocalTransform * Parent->GetWorldTransform();
	}
	else
	{
		if (auto WidgetPresenterComponent = GetWidgetPresenterComponent())
		{
			ObjectToWorldTransform = WidgetPresenterComponent->GetComponentTransform() * LocalTransform;			
		}
		else
		{
			ObjectToWorldTransform = LocalTransform;
		}
	}
	this->MarkTransformChanged();
}
void ULexWidget::CalculateObjectToWorldTransform(bool bPropagateToChildren)
{
	this->UpdateObjectToWorldTransform();
	this->OnUpdateTransform();
	if (bPropagateToChildren)
	{
		for (auto Child : this->Children)
		{
			Child->CalculateObjectToWorldTransform(true);
		}
	}
}

void ULexWidget::SetParent(ULexWidget* InParent, bool InKeepWorldPosition, int InSiblingIndex)
{
	if (IsValid(InParent))//attach to parent
	{
		check(this != InParent);
		if (this->Parent == InParent)return;
		if (InParent->IsChildOf(this))return;
		if (InParent->Children.Contains(this))return;
		bIsAttaching = true;
		if (Parent.IsValid())
		{
			SetParent(nullptr, InKeepWorldPosition);
		}
		bIsAttaching = false;
		auto OldObjectToWorldTransform = this->GetWorldTransform();
		if (InSiblingIndex == -1 || !InParent->Children.IsValidIndex(InSiblingIndex))
		{
			InParent->Children.Add(this);
			this->SiblingIndex = InParent->Children.Num() - 1;
			this->Call_SiblingIndexChanged();
		}
		else
		{
			InParent->Children.Insert(this, InSiblingIndex);
			for (int i = InSiblingIndex; i < InParent->Children.Num(); i++)
			{
				auto Child = InParent->Children[i];
				Child->SiblingIndex = i;
			}
			this->Call_SiblingIndexChanged();
		}
		this->Parent = InParent;
		if (InKeepWorldPosition)
		{
			auto WorldToParentTransform = InParent->GetWorldTransform().Inverse();
			auto LocalTransform = WorldToParentTransform * OldObjectToWorldTransform;
			this->RelativeLocation = LocalTransform.GetLocation();
			this->RelativeRotation = LocalTransform.GetRotation();
			this->RelativeScale = LocalTransform.GetScale3D();
		}
		this->CalculateObjectToWorldTransform();
		this->OnAttachedToParent();
		InParent->OnChildAttached(this);
	}
	else//detach from parent
	{
		if (this->Parent == nullptr)return;
		auto OldParent = this->Parent;
		auto OldObjectToWorldTransform = this->GetWorldTransform();
		this->Parent->Children.Remove(this);
		this->Parent = nullptr;
		if (InKeepWorldPosition)
		{
			auto LocalTransform = OldObjectToWorldTransform;
			this->RelativeLocation = LocalTransform.GetLocation();
			this->RelativeRotation = LocalTransform.GetRotation();
			this->RelativeScale = LocalTransform.GetScale3D();
		}
		this->CalculateObjectToWorldTransform();
		this->OnDetachedFromParent();
		OldParent->OnChildDetached();
	}
}

void ULexWidget::SetSiblingIndex(int32 InInt) 
{ 
	if (InInt != SiblingIndex)
	{
		SiblingIndex = InInt;
		this->Call_SiblingIndexChanged();
		ApplySiblingIndex();
	}
}

bool ULexWidget::IsChildOf(const ULexWidget* InTarget)const
{
	auto TempParent = this->Parent;
	while (TempParent.IsValid())
	{
		if (TempParent == InTarget)
		{
			return true;
		}
		TempParent = TempParent->Parent;
	}
	return false;
}
#pragma endregion Transform

TArray<ULexUIBehaviour*> ULexWidget::GetComponents(TSubclassOf<ULexUIBehaviour> ComponentClass)const
{
	TArray<ULexUIBehaviour*> ResultArray;
	for (auto& Comp : Components)
	{
		if (Comp->IsA(ComponentClass))
		{
			ResultArray.Add(Comp);
		}
	}
	return ResultArray;
}

ULexUIBehaviour* ULexWidget::GetComponent(TSubclassOf<ULexUIBehaviour> ComponentClass)const
{
	for (auto& Comp : Components)
	{
		if (Comp && Comp->IsA(ComponentClass))
		{
			return Comp;
		}
	}
	return nullptr;
}

ULexUIBehaviour* ULexWidget::GetComponentByInterface(UClass* InterfaceClass)const
{
	for (auto& Component : GetAllComponents())
	{
		if (Component->GetClass()->ImplementsInterface(InterfaceClass))
		{
			return Component;
		}
	}
	return nullptr;
}

ULexUIBehaviour* ULexWidget::GetComponentInParent(TSubclassOf<ULexUIBehaviour> ComponentClass, bool bIncludeSelf, ULexWidget* InStopWidget)const
{
	auto ParentWidget = bIncludeSelf ? this : this->GetParent();
	while (IsValid(ParentWidget))
	{
		if (InStopWidget)
		{
			if (ParentWidget == InStopWidget)return nullptr;
		}
		if (auto ResultComp = ParentWidget->GetComponent(ComponentClass))
		{
			return ResultComp;
		}
		ParentWidget = ParentWidget->GetParent();
	}
	return nullptr;
}

DECLARE_CYCLE_STAT(TEXT("LexWidget OnUpdateTransform"), STAT_OnUpdateTransform, STATGROUP_LGUI);
void ULexWidget::OnUpdateTransform()
{
	SCOPE_CYCLE_COUNTER(STAT_OnUpdateTransform)
	// UE_LOG(LGUI, Error, TEXT("OnUpdateTransform Flag:%d %s"), (int)UpdateTransformFlags, *this->GetDisplayName());
		bool bPositionChanged = false, bRotationChanged = false, bScaleChanged = false;
	{
		auto Pos = this->GetRelativeLocation();
		auto Pos2D = FVector2D(Pos.Y, Pos.Z);
		if (Pos2D != PrevLocation2D)
		{
			PrevLocation2D = Pos2D;
			bPositionChanged = true;
		}
		auto CompScale3D = this->GetWorldScale();
		auto CompScale2D = FVector2D(CompScale3D.Y, CompScale3D.Z);
		if (PrevScale2D != CompScale2D)
		{
			PrevScale2D = CompScale2D;
			bScaleChanged = true;
		}
		if (LayoutContainer)
		{
			LayoutContainer->OnTransformChanged();
		}
		if (LayoutSelf)
		{
			LayoutSelf->OnTransformChanged();
		}
		if (Visual)
		{
			Visual->OnTransformChanged(bPositionChanged, bScaleChanged);
		}
	}
}

void ULexWidget::OnChildAttached(ULexWidget* ChildWidget)
{
	//make sure SiblingIndex all good
	if (ChildWidget->SiblingIndex == INDEX_NONE)
	{
		for (int i = 0; i < Children.Num(); i++)
		{
			auto& UIChild = Children[i];
			if (UIChild->SiblingIndex != i)
			{
				UIChild->SiblingIndex = i;
				UIChild->Call_SiblingIndexChanged();
			}
		}
	}
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkLexWidgetHierarchyChanged();
	}
	MarkLayoutForRebuild(this);//child added, need to rebuild layout
}

void ULexWidget::OnAttachedToParent()
{
	if (this->bIsRegistered)//registered means not during prefab process
	{
		Call_TransformChanged();
		CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
	}

	ULexCanvas* ParentCanvas = this->GetComponentInParent<ULexCanvas>();
	OnHierarchyAttachmentChanged(ParentCanvas, Parent->RootWidget.Get());

	CalculateWidgetActive_Recursive();
	CalculateRaycastable_Recursive();
	CalculateInteractable_Recursive();
	
	// MarkLayoutForRebuild(this);//why comment this? because it already called in OnHierarchyAttachmentChanged
	MarkClipDirty(true);
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
	{
#if WITH_EDITOR
		LexUIManager->MarkLexUIWidgetOutlinerChanged();
#endif
		LexUIManager->MarkRebuildAllLayoutTree();
	}
}

void ULexWidget::OnChildDetached()
{
	for (int i = 0; i < Children.Num(); i++)
	{
		auto& UIChild = Children[i];
		if (UIChild->SiblingIndex != i)
		{
			UIChild->SiblingIndex = i;
			UIChild->Call_SiblingIndexChanged();
		}
	}
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkLexWidgetHierarchyChanged();
	}
	MarkLayoutForRebuild(this);//child removed, need to rebuild layout
}

void ULexWidget::OnDetachedFromParent()
{
	if (bIsAttaching)return;
	if (this->bIsRegistered)//registered means not during prefab process
	{
		Call_TransformChanged();
		CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
	}

	OnHierarchyAttachmentChanged(nullptr, nullptr);

	CalculateWidgetActive_Recursive();
	CalculateRaycastable_Recursive();
	CalculateInteractable_Recursive();

	// MarkLayoutForRebuild(this);//why comment this? because it already called in OnHierarchyAttachmentChanged
	MarkClipDirty(true);
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
	{
#if WITH_EDITOR
		LexUIManager->MarkLexUIWidgetOutlinerChanged();
#endif
		LexUIManager->MarkRebuildAllLayoutTree();
	}
}

void ULexWidget::OnRegister()
{
	bIsRegistered = true;
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
	{
		LexUIManager->AddWidget(this);
#if WITH_EDITOR
		LexUIManager->MarkLexUIWidgetOutlinerChanged();
#endif
	}
	CheckRootWidget();

	if (this->IsRootWidgetInHierarchy())
	{
		CalculateWidgetActive_Recursive();
		CalculateRaycastable_Recursive();
		CalculateInteractable_Recursive();
	}

	if (IsValid(LayoutContainer))
	{
		LayoutContainer->Call_OnRegister();
	}
	if (IsValid(LayoutSelf))
	{
		LayoutSelf->Call_OnRegister();
	}
	if (IsValid(Visual))
	{
		Visual->Call_OnRegister();
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->RegisterVisual(Visual);
		}
	}

	Components.Remove(nullptr);//clear null component
	for (auto Component : Components)
	{
		if (IsValid(Component))
		{
			Component->OnRegister();
		}
	}
}
void ULexWidget::OnUnregister()
{
	bIsRegistered = false;
	
	for (auto Component : Components)
	{
		Component->OnUnregister();
	}

	if (IsValid(LayoutContainer))
	{
		LayoutContainer->Call_OnUnregister();
	}
	if (IsValid(LayoutSelf))
	{
		LayoutSelf->Call_OnUnregister();
	}
	if (IsValid(Visual))
	{
		Visual->Call_OnUnregister();
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->MarkVisualWillChange(Visual);
			RenderCanvas->UnregisterVisual(Visual);
		}
	}
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(this->GetWorld()))
	{
		LexUIManager->RemoveWidget(this);
#if WITH_EDITOR
		LexUIManager->MarkLexUIWidgetOutlinerChanged();
#endif
	}
}

void ULexWidget::EnsureUIChildrenValid()
{
	for (int i = Children.Num() - 1; i >= 0; i--)
	{
		if (!IsValid(Children[i]))
		{
			Children.RemoveAt(i);
		}
	}
}

void ULexWidget::EnsureUIChildrenSorted()const
{
	if (bNeedSortUIChildren)
	{
		bNeedSortUIChildren = false;
		Children.Sort([](const ULexWidget& A, const ULexWidget& B)
			{
				if (A.GetSiblingIndex() < B.GetSiblingIndex())
					return true;
				return false;
			});
	}
}


bool ULexWidget::CalculateAnchorFromTransform()
{
	auto TempRelativeLocation = this->GetRelativeLocation();
	FVector2D CalculatedAnchoredPosition;
	if (Parent.IsValid())
	{
		//just a reverse operation from CalculateTransformFromAnchor
		float LocalLeftPoint =
			Parent->GetLocalSpaceLeft()
			+ (Parent->GetWidth() * this->AnchorData.AnchorMin.X);

		float LocalBottomPoint =
			Parent->GetLocalSpaceBottom()
			+ (Parent->GetHeight() * this->AnchorData.AnchorMin.Y);

		CalculatedAnchoredPosition.X = TempRelativeLocation.Y
			- LocalLeftPoint
			- +(Parent->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X)) * this->AnchorData.Pivot.X;
		CalculatedAnchoredPosition.Y = TempRelativeLocation.Z
			- LocalBottomPoint
			- (Parent->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y)) * this->AnchorData.Pivot.Y;
	}
	else
	{
		CalculatedAnchoredPosition.X = TempRelativeLocation.Y;
		CalculatedAnchoredPosition.Y = TempRelativeLocation.Z;
	}

	bCacheAnchorOffsetLeftDirty = true;
	bCacheAnchorOffsetRightDirty = true;
	bCacheAnchorOffsetBottomDirty = true;
	bCacheAnchorOffsetTopDirty = true;

	if (AnchorData.AnchoredPosition != CalculatedAnchoredPosition)
	{
		AnchorData.AnchoredPosition = CalculatedAnchoredPosition;
		return true;
	}
	return false;
}
void ULexWidget::CalculateTransformFromAnchor()
{
	bool HorizontalPositionChanged = false, VerticalPositionChanged = false;
	CalculateTransformFromAnchor(HorizontalPositionChanged, VerticalPositionChanged);
}
void ULexWidget::CalculateTransformFromAnchor(bool& OutHorizontalPositionChanged, bool& OutVerticalPositionChanged)
{
	bCanSetAnchorFromTransform = false;
	FVector ResultLocation = this->GetRelativeLocation();
	if (Parent.IsValid())
	{
		float LocalLeftPoint = //this left point anchor position in parent's space
			Parent->GetLocalSpaceLeft()//parent's left position
			+ (Parent->GetWidth() * this->AnchorData.AnchorMin.X);//add anchor offset
		float LocalLeftPivotPoint = //to pivot point, with anchor offset
			LocalLeftPoint
			+ (Parent->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X))//parent anchor width (width without SizeDelta)
				* this->AnchorData.Pivot.X
			+ this->AnchorData.AnchoredPosition.X;

		float LocalBottomPoint = //this bottom point anchor position in parent's space
			Parent->GetLocalSpaceBottom()//parent's bottom position
			+ (Parent->GetHeight() * this->AnchorData.AnchorMin.Y);//add anchor offset
		float LocalBottomPivotPoint = //to pivot point, with anchor offset
			LocalBottomPoint
			+ (Parent->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y))//parent anchor width (width without SizeDelta)
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

	auto OriginRelativeLocation = this->GetRelativeLocation();
	double Tolerance = 0.0f;
	if (FMath::Abs(OriginRelativeLocation.Y - ResultLocation.Y) > Tolerance)
	{
		OutHorizontalPositionChanged = true;
	}
	if (FMath::Abs(OriginRelativeLocation.Z - ResultLocation.Z) > Tolerance)
	{
		OutVerticalPositionChanged = true;
	}
	if (OutHorizontalPositionChanged || OutVerticalPositionChanged)
	{
		this->SetRelativeLocation(ResultLocation);
	}
	bCanSetAnchorFromTransform = true;
}

#pragma region AnchorData

float ULexWidget::GetWidth() const
{
	if (bCacheWidthDirty)
	{
		bCacheWidthDirty = false;
		if (Parent.IsValid())
		{
			if (AnchorData.IsHorizontalStretched())
			{
				CacheWidth = AnchorData.SizeDelta.X + Parent->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X);
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
float ULexWidget::GetHeight() const
{
	if (bCacheHeightDirty)
	{
		bCacheHeightDirty = false;
		if (Parent.IsValid())
		{
			if (AnchorData.IsVerticalStretched())
			{
				CacheHeight = AnchorData.SizeDelta.Y + Parent->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y);
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

void ULexWidget::SetAnchorData(const FLexUIAnchorData& Value)
{
	AnchorData.Pivot = Value.Pivot;
	AnchorData.AnchorMin = Value.AnchorMin;
	AnchorData.AnchorMax = Value.AnchorMax;
	AnchorData.AnchoredPosition = Value.AnchoredPosition;
	AnchorData.SizeDelta = Value.SizeDelta;

	bCacheWidthDirty = true;
	bCacheHeightDirty = true;
	bCacheAnchorOffsetLeftDirty = true;
	bCacheAnchorOffsetRightDirty = true;
	bCacheAnchorOffsetBottomDirty = true;
	bCacheAnchorOffsetTopDirty = true;

	MarkAnchorDataChanged_Recursive(true, true, true, false);
	MarkLayoutForRebuild(this);
}

void ULexWidget::SetPivot(FVector2D Value) 
{
	if (!AnchorData.Pivot.Equals(Value, 0.0f))
	{
		AnchorData.Pivot = Value;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		MarkAnchorDataChanged_Recursive(true, false, false, false);
		if (Parent.IsValid() && Parent->GetLayoutContainer())//only position change, if parent contains LayoutContainer then we should rebuild layout, otherwise not
		{
			MarkLayoutForRebuild(this);
		}
	}
}

void ULexWidget::SetAnchorMin(FVector2D Value)
{
	if (this->Parent.IsValid())
	{
		if (!AnchorData.AnchorMin.Equals(Value, 0.0f))
		{
			auto CurrentLeft = this->GetAnchorOffsetLeft();
			auto CurrentBottom = this->GetAnchorOffsetBottom();

			AnchorData.AnchorMin = Value;
			
			//SetAnchorLeft
			{
				auto CurrentRight = this->GetAnchorOffsetRight();
				CacheWidth = -CurrentRight - CurrentLeft;
				//SetWidth
				AnchorData.SizeDelta.X = CacheWidth;
				this->AnchorData.AnchoredPosition.X = CurrentLeft + CacheWidth * this->AnchorData.Pivot.X;
			}

			//SetAnchorBottom
			{
				auto CurrentTop = this->GetAnchorOffsetTop();
				CacheHeight = -CurrentTop - CurrentBottom;
				//SetHeight
				AnchorData.SizeDelta.Y = CacheHeight;
				this->AnchorData.AnchoredPosition.Y = CurrentBottom + CacheHeight * this->AnchorData.Pivot.Y;
			}

			MarkAnchorDataChanged_Recursive(false, true, true, false);
			MarkLayoutForRebuild(this);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent! %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
	}
}
void ULexWidget::SetAnchorMax(FVector2D Value)
{
	if (this->Parent.IsValid())
	{
		if (!AnchorData.AnchorMax.Equals(Value, 0.0f))
		{
			auto CurrentRight = this->GetAnchorOffsetRight();
			auto CurrentTop = this->GetAnchorOffsetTop();

			AnchorData.AnchorMax = Value;

			//SetAnchorRight
			{
				auto CurrentLeft = this->GetAnchorOffsetLeft();
				CacheWidth = -CurrentRight - CurrentLeft;
				//SetWidth
				AnchorData.SizeDelta.X = CacheWidth;
				this->AnchorData.AnchoredPosition.X = CurrentLeft + CacheWidth * this->AnchorData.Pivot.X;
			}
			//SetAnchorTop
			{
				auto CurrentBottom = this->GetAnchorOffsetBottom();
				CacheHeight = -CurrentTop - CurrentBottom;
				//SetHeight
				AnchorData.SizeDelta.Y = CacheHeight;
				this->AnchorData.AnchoredPosition.Y = CurrentBottom + CacheHeight * this->AnchorData.Pivot.Y;
			}

			MarkAnchorDataChanged_Recursive(false, true, true, false);
			MarkLayoutForRebuild(this);;
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent! %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
	}
}

void ULexWidget::SetAnchorOffset(FMargin Value)
{
	if (this->Parent.IsValid())
	{
		bool bWidthChange = CacheAnchorOffsetLeft != Value.Left || CacheAnchorOffsetRight != Value.Right;
		bool bHeightChange = CacheAnchorOffsetBottom != Value.Bottom || CacheAnchorOffsetTop != Value.Top;
		if (bCacheAnchorOffsetLeftDirty || bCacheAnchorOffsetRightDirty || bWidthChange || bHeightChange)
		{
			bCacheAnchorOffsetLeftDirty = false;
			bCacheAnchorOffsetRightDirty = false;
			bCacheAnchorOffsetBottomDirty = false;
			bCacheAnchorOffsetTopDirty = false;
			CacheAnchorOffsetLeft = Value.Left;
			CacheAnchorOffsetRight = Value.Right;
			CacheAnchorOffsetBottom = Value.Bottom;
			CacheAnchorOffsetTop = Value.Top;
			
			CacheWidth = -Value.Right - Value.Left;
			//SetWidth
			AnchorData.SizeDelta.X = CacheWidth;
			AnchorData.AnchoredPosition.X = Value.Left + CacheWidth * AnchorData.Pivot.X;

			CacheHeight = -Value.Top - Value.Bottom;
			//SetHeight
			AnchorData.SizeDelta.Y = CacheHeight;
			AnchorData.AnchoredPosition.Y = Value.Bottom + CacheHeight * AnchorData.Pivot.Y;
			
			MarkAnchorDataChanged_Recursive(false, bWidthChange, bHeightChange, false);
			MarkLayoutForRebuild(this);
		}
		bCacheAnchorOffsetLeftDirty = false;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)
	}
}

void ULexWidget::SetHorizontalAndVerticalAnchorMinMax(FVector2D MinValue, FVector2D MaxValue, bool bKeepSize, bool bKeepRelativeLocation)
{
	if (this->Parent.IsValid())
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
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent! %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
	}
}

void ULexWidget::SetHorizontalAnchorMinMax(FVector2D Value, bool bKeepSize, bool bKeepRelativeLocation)
{
	if (this->Parent.IsValid())
	{
		if (AnchorData.AnchorMin.X != Value.X || AnchorData.AnchorMax.X != Value.Y)
		{
			auto CurrentLeft = this->GetAnchorOffsetLeft();
			auto CurrentRight = this->GetAnchorOffsetRight();

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
					CacheWidth = this->Parent->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - CurrentRight - CurrentLeft;
				}
				//SetWidth
				{
					auto CalculatedSizeDeltaX = CacheWidth - (Parent->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				this->AnchorData.AnchoredPosition.X = FMath::Lerp(CurrentLeft, -CurrentRight, this->AnchorData.Pivot.X);
			}
			if (bKeepRelativeLocation)
			{
				this->SetRelativeLocation(PrevRelativeLocation);
			}

			MarkAnchorDataChanged_Recursive(false, !bKeepSize, !bKeepSize, false);
			MarkLayoutForRebuild(this);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent! %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
	}
}
void ULexWidget::SetVerticalAnchorMinMax(FVector2D Value, bool bKeepSize, bool bKeepRelativeLocation)
{
	if (this->Parent.IsValid())
	{
		if (AnchorData.AnchorMin.Y != Value.X || AnchorData.AnchorMax.Y != Value.Y)
		{
			auto CurrentBottom = this->GetAnchorOffsetBottom();
			auto CurrentTop = this->GetAnchorOffsetTop();

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
					CacheHeight = this->Parent->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - CurrentTop - CurrentBottom;
				}
				//SetHeight
				{
					auto CalculatedSizeDeltaY = CacheHeight - (Parent->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				this->AnchorData.AnchoredPosition.Y = FMath::Lerp(CurrentBottom, -CurrentTop, this->AnchorData.Pivot.Y);
			}
			if (bKeepRelativeLocation)
			{
				this->SetRelativeLocation(PrevRelativeLocation);
			}

			MarkAnchorDataChanged_Recursive(false, !bKeepSize, !bKeepSize, false);
			MarkLayoutForRebuild(this);
		}
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent! %s"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, *this->GetPathName());
	}
}

void ULexWidget::SetAnchoredPosition(FVector2D Value)
{
	if (!AnchorData.AnchoredPosition.Equals(Value, 0.0f))
	{
		AnchorData.AnchoredPosition = Value;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChanged_Recursive(false, false, false, false);
		if (Parent.IsValid() && Parent->GetLayoutContainer())//only position change, if parent contains LayoutContainer then we should rebuild layout, otherwise not
		{
			MarkLayoutForRebuild(this);
		}
	}
}

void ULexWidget::SetHorizontalAnchoredPosition(float Value)
{
	if (AnchorData.AnchoredPosition.X != Value)
	{
		AnchorData.AnchoredPosition.X = Value;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChanged_Recursive(false, false, false, false);
		if (Parent.IsValid() && Parent->GetLayoutContainer())//only position change, if parent contains LayoutContainer then we should rebuild layout, otherwise not
		{
			MarkLayoutForRebuild(this);
		}
	}
}
void ULexWidget::SetVerticalAnchoredPosition(float Value)
{
	if (AnchorData.AnchoredPosition.Y != Value)
	{
		AnchorData.AnchoredPosition.Y = Value;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		MarkAnchorDataChanged_Recursive(false, false, false, false);
		if (Parent.IsValid() && Parent->GetLayoutContainer())//only position change, if parent contains LayoutContainer then we should rebuild layout, otherwise not
		{
			MarkLayoutForRebuild(this);
		}
	}
}

void ULexWidget::SetSizeDelta(FVector2D Value)
{
	if (!AnchorData.SizeDelta.Equals(Value, 0.0f))
	{
		AnchorData.SizeDelta = Value;
		bCacheWidthDirty = true;
		bCacheHeightDirty = true;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChanged_Recursive(false, true, true, false);
		MarkLayoutForRebuild(this);
	}
}

void ULexWidget::SetAnchoredPositionAndSizeDelta(FVector2D Position, FVector2D Size)
{
	bool bPosChange = false, bSizeChange = false;
	if (!AnchorData.AnchoredPosition.Equals(Position, 0.0f))
	{
		bPosChange = true;
		AnchorData.AnchoredPosition = Position;
	}
	if (!AnchorData.SizeDelta.Equals(Size, 0.0f))
	{
		bSizeChange = true;
		AnchorData.SizeDelta = Size;
		CacheWidth = Size.X;
		CacheHeight = Size.Y;
	}
	if (bPosChange || bSizeChange)
	{
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChanged_Recursive(false, bSizeChange, bSizeChange, false);
		MarkLayoutForRebuild(this);
	}
}

float ULexWidget::GetAnchorOffsetLeft()const
{
	if (bCacheAnchorOffsetLeftDirty)
	{
		bCacheAnchorOffsetLeftDirty = false;
		if (this->Parent.IsValid())
		{
			CacheAnchorOffsetLeft = this->AnchorData.AnchoredPosition.X - this->AnchorData.SizeDelta.X * this->AnchorData.Pivot.X;
		}
		else
		{
			CacheAnchorOffsetLeft = this->GetLocalSpaceLeft();//local space left
		}
	}
	return CacheAnchorOffsetLeft;
}
float ULexWidget::GetAnchorOffsetTop()const
{
	if (bCacheAnchorOffsetTopDirty)
	{
		bCacheAnchorOffsetTopDirty = false;
		if (this->Parent.IsValid())
		{
			CacheAnchorOffsetTop = -(this->AnchorData.AnchoredPosition.Y + this->AnchorData.SizeDelta.Y * (1.0f - this->AnchorData.Pivot.Y));
		}
		else
		{
			CacheAnchorOffsetTop = this->GetLocalSpaceTop();
		}
	}
	return CacheAnchorOffsetTop;
}
float ULexWidget::GetAnchorOffsetRight()const
{
	if (bCacheAnchorOffsetRightDirty)
	{
		bCacheAnchorOffsetRightDirty = false;
		if (this->Parent.IsValid())
		{
			CacheAnchorOffsetRight = -(this->AnchorData.AnchoredPosition.X + this->AnchorData.SizeDelta.X * (1.0f - this->AnchorData.Pivot.X));
		}
		else
		{
			CacheAnchorOffsetRight = this->GetLocalSpaceRight();
		}
	}
	return CacheAnchorOffsetRight;
}
float ULexWidget::GetAnchorOffsetBottom()const
{
	if (bCacheAnchorOffsetBottomDirty)
	{
		bCacheAnchorOffsetBottomDirty = false;
		if (this->Parent.IsValid())
		{
			CacheAnchorOffsetBottom = this->AnchorData.AnchoredPosition.Y - this->AnchorData.SizeDelta.Y * this->AnchorData.Pivot.Y;
		}
		else
		{
			CacheAnchorOffsetBottom = this->GetLocalSpaceBottom();
		}
	}
	return CacheAnchorOffsetBottom;
}

FMargin ULexWidget::GetAnchorOffset() const
{
	return FMargin(
		this->GetAnchorOffsetLeft(),
		this->GetAnchorOffsetTop(),
		this->GetAnchorOffsetRight(),
		this->GetAnchorOffsetBottom()
	);
}

void ULexWidget::SetAnchorOffsetLeft(float Value)
{
	if (this->Parent.IsValid())
	{
		if (CacheAnchorOffsetLeft != Value || bCacheAnchorOffsetLeftDirty)
		{
			bCacheAnchorOffsetLeftDirty = false;
			CacheAnchorOffsetLeft = Value;
			auto CurrentRight = this->GetAnchorOffsetRight();
			CacheWidth = this->Parent->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - CurrentRight - Value;
			//SetWidth
			{
				if (AnchorData.IsHorizontalStretched())
				{
					auto CalculatedSizeDeltaX = CacheWidth - (Parent->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				else
				{
					AnchorData.SizeDelta.X = CacheWidth;
				}
			}
			this->AnchorData.AnchoredPosition.X = FMath::Lerp(Value, -CurrentRight, this->AnchorData.Pivot.X);
			MarkAnchorDataChanged_Recursive(false, true, false, false);
			MarkLayoutForRebuild(this);
		}
		bCacheAnchorOffsetLeftDirty = false;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)
	}
}
void ULexWidget::SetAnchorOffsetTop(float Value)
{
	if (this->Parent.IsValid())
	{
		if (CacheAnchorOffsetTop != Value || bCacheAnchorOffsetTopDirty)
		{
			bCacheAnchorOffsetTopDirty = false;
			CacheAnchorOffsetTop = Value;
			auto CurrentBottom = this->GetAnchorOffsetBottom();
			CacheHeight = this->Parent->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - Value - CurrentBottom;
			//SetHeight
			{
				if (AnchorData.IsVerticalStretched())
				{
					auto CalculatedSizeDeltaY = CacheHeight - (Parent->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				else
				{
					AnchorData.SizeDelta.Y = CacheHeight;
				}
			}
			this->AnchorData.AnchoredPosition.Y = FMath::Lerp(CurrentBottom, -Value, this->AnchorData.Pivot.Y);
			MarkAnchorDataChanged_Recursive(false, false, true, false);
			MarkLayoutForRebuild(this);
		}
		bCacheAnchorOffsetTopDirty = false;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)
	}
}
void ULexWidget::SetAnchorOffsetRight(float Value)
{
	if (this->Parent.IsValid())
	{
		if (CacheAnchorOffsetRight != Value || bCacheAnchorOffsetRightDirty)
		{
			bCacheAnchorOffsetRightDirty = false;
			CacheAnchorOffsetRight = Value;
			auto CurrentLeft = this->GetAnchorOffsetLeft();
			CacheWidth = this->Parent->GetWidth() * (this->AnchorData.AnchorMax.X - this->AnchorData.AnchorMin.X) - Value - CurrentLeft;
			//SetWidth
			{
				if (AnchorData.IsHorizontalStretched())
				{
					auto CalculatedSizeDeltaX = CacheWidth - (Parent->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
				}
				else
				{
					AnchorData.SizeDelta.X = CacheWidth;
				}
			}
			this->AnchorData.AnchoredPosition.X = FMath::Lerp(CurrentLeft, -Value, this->AnchorData.Pivot.X);
			MarkAnchorDataChanged_Recursive(false, true, false, false);
			MarkLayoutForRebuild(this);
		}
		bCacheAnchorOffsetRightDirty = false;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)
	}
}
void ULexWidget::SetAnchorOffsetBottom(float Value)
{
	if (this->Parent.IsValid())
	{
		if (CacheAnchorOffsetBottom != Value || bCacheAnchorOffsetBottomDirty)
		{
			bCacheAnchorOffsetBottomDirty = false;
			CacheAnchorOffsetBottom = Value;
			auto CurrentTop = this->GetAnchorOffsetTop();
			CacheHeight = this->Parent->GetHeight() * (this->AnchorData.AnchorMax.Y - this->AnchorData.AnchorMin.Y) - CurrentTop - Value;
			//SetHeight
			{
				if (AnchorData.IsVerticalStretched())
				{
					auto CalculatedSizeDeltaY = CacheHeight - (Parent->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
				}
				else
				{
					AnchorData.SizeDelta.Y = CacheHeight;
				}
			}
			this->AnchorData.AnchoredPosition.Y = FMath::Lerp(Value, -CurrentTop, this->AnchorData.Pivot.Y);
			MarkAnchorDataChanged_Recursive(false, false, true, false);
			MarkLayoutForRebuild(this);
		}
		bCacheAnchorOffsetBottomDirty = false;
	}
	else
	{
		UE_LOG(LGUI, Warning, TEXT("[%s].%d This function only valid if LexWidget have parent!"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)
	}
}

void ULexWidget::SetWidth(float Value)
{
	if (CacheWidth != Value || bCacheWidthDirty)
	{
		bCacheWidthDirty = false;
		CacheWidth = Value;
		if (Parent.IsValid())
		{
			if (AnchorData.IsHorizontalStretched())
			{
				auto CalculatedSizeDeltaX = Value - (Parent->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
				if (AnchorData.SizeDelta.X != CalculatedSizeDeltaX)
				{
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
					MarkAnchorDataChanged_Recursive(false, true, false, false);
					MarkLayoutForRebuild(this);
				}
			}
			else
			{
				if (AnchorData.SizeDelta.X != Value)
				{
					AnchorData.SizeDelta.X = Value;
					MarkAnchorDataChanged_Recursive(false, true, false, false);
					MarkLayoutForRebuild(this);
				}
			}
		}
		else
		{
			if (AnchorData.SizeDelta.X != Value)
			{
				AnchorData.SizeDelta.X = Value;
				MarkAnchorDataChanged_Recursive(false, true, false, false);
				MarkLayoutForRebuild(this);
			}
		}
	}
}
void ULexWidget::SetHeight(float Value)
{
	if (CacheHeight != Value || bCacheHeightDirty)
	{
		bCacheHeightDirty = false;
		CacheHeight = Value;
		if (Parent.IsValid())
		{
			if (AnchorData.IsVerticalStretched())
			{
				auto CalculatedSizeDeltaY = Value - (Parent->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
				if (AnchorData.SizeDelta.Y != CalculatedSizeDeltaY)
				{
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
					MarkAnchorDataChanged_Recursive(false, false, true, false);
					MarkLayoutForRebuild(this);
				}
			}
			else
			{
				if (AnchorData.SizeDelta.Y != Value)
				{
					AnchorData.SizeDelta.Y = Value;
					MarkAnchorDataChanged_Recursive(false, false, true, false);
					MarkLayoutForRebuild(this);
				}
			}
		}
		else
		{
			if (AnchorData.SizeDelta.Y != Value)
			{
				AnchorData.SizeDelta.Y = Value;
				MarkAnchorDataChanged_Recursive(false, false, true, false);
				MarkLayoutForRebuild(this);
			}
		}
	}
}

void ULexWidget::SetSize(FVector2D Value)
{
	if (CacheWidth != Value.X || bCacheWidthDirty || CacheHeight != Value.Y || bCacheHeightDirty)
	{
		bCacheWidthDirty = false;
		CacheWidth = Value.X;
		bCacheHeightDirty = false;
		CacheHeight = Value.Y;
		if (Parent.IsValid())
		{
			if (AnchorData.IsHorizontalStretched() || AnchorData.IsVerticalStretched())
			{
				auto CalculatedSizeDeltaX = Value.X - (Parent->GetWidth() * (AnchorData.AnchorMax.X - AnchorData.AnchorMin.X));
				auto CalculatedSizeDeltaY = Value.Y - (Parent->GetHeight() * (AnchorData.AnchorMax.Y - AnchorData.AnchorMin.Y));
				bool bWidthChanged = AnchorData.SizeDelta.X != CalculatedSizeDeltaX;
				bool bHeightChanged = AnchorData.SizeDelta.Y != CalculatedSizeDeltaY;
				if (bWidthChanged || bHeightChanged)
				{
					AnchorData.SizeDelta.X = CalculatedSizeDeltaX;
					AnchorData.SizeDelta.Y = CalculatedSizeDeltaY;
					MarkAnchorDataChanged_Recursive(false, bWidthChanged, bHeightChanged, false);
					MarkLayoutForRebuild(this);
				}
			}
			else
			{
				auto bWidthChanged = AnchorData.SizeDelta.X != Value.X;
				auto bHeightChanged = AnchorData.SizeDelta.Y != Value.Y;
				if (bWidthChanged || bHeightChanged)
				{
					AnchorData.SizeDelta = Value;
					MarkAnchorDataChanged_Recursive(false, bWidthChanged, bHeightChanged, false);
					MarkLayoutForRebuild(this);
				}
			}
		}
		else
		{
			auto bWidthChanged = AnchorData.SizeDelta.X != Value.X;
			auto bHeightChanged = AnchorData.SizeDelta.Y != Value.Y;
			if (bWidthChanged || bHeightChanged)
			{
				AnchorData.SizeDelta = Value;
				MarkAnchorDataChanged_Recursive(false, bWidthChanged, bHeightChanged, false);
				MarkLayoutForRebuild(this);
			}
		}
	}
}

#pragma endregion

void ULexWidget::RegisterRenderCanvas(ULexCanvas* InRenderCanvas)
{
	bIsCanvasWidget = true;
	ULexCanvas* ParentCanvas = nullptr;
	if (auto ParentWidget = GetParent())
	{
		ParentCanvas = ParentWidget->GetComponentInParent<ULexCanvas>();//@todo: replace with Canvas's ParentCanvas?
	}
	if (RenderCanvas != InRenderCanvas)
	{
		SetRenderCanvas(InRenderCanvas);
	}
	InRenderCanvas->SetParentCanvas(ParentCanvas);
	for (auto& Child : Children)
	{
		if (IsValid(Child))
		{
			Child->RenewRenderCanvasRecursive(InRenderCanvas);
		}
	}
}
void ULexWidget::RenewRenderCanvasRecursive(ULexCanvas* InParentRenderCanvas)
{
	auto ThisRenderCanvas = this->GetComponent<ULexCanvas>();
	if (ThisRenderCanvas != nullptr)
	{
		if (InParentRenderCanvas != ThisRenderCanvas)
		{
			ThisRenderCanvas->SetParentCanvas(InParentRenderCanvas);//set parent Canvas for this actor's Canvas
		}
		return;
	}

	if (RenderCanvas != InParentRenderCanvas)//if attach to new Canvas, need to remove from old and add to new
	{
		SetRenderCanvas(InParentRenderCanvas);
	}

	for (auto& Child : Children)
	{
		if (IsValid(Child))
		{
			Child->RenewRenderCanvasRecursive(InParentRenderCanvas);
		}
	}
}

void ULexWidget::UnregisterRenderCanvas()
{
	bIsCanvasWidget = false;
	ULexCanvas* ParentCanvas = nullptr;
	if (auto ParentWidget = GetParent())
	{
		ParentCanvas = ParentWidget->GetComponentInParent<ULexCanvas>();//@todo: replace with Canvas's ParentCanvas?
	}
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->SetParentCanvas(nullptr);
	}
	if (RenderCanvas != ParentCanvas)
	{
		SetRenderCanvas(ParentCanvas);
	}
	for (auto& Child : Children)
	{
		if (IsValid(Child))
		{
			Child->RenewRenderCanvasRecursive(ParentCanvas);
		}
	}
}

void ULexWidget::UpdateLayout()
{
	if (IsValid(LayoutSelf))
	{
		LayoutSelf->CalculateSize();
	}
	if (IsValid(LayoutContainer))
	{
		LayoutContainer->CalculateLayout();
	}
}

void ULexWidget::UpdateClip(ULexUIDataAsTexture* ClipDataTexture, TArray<TSharedPtr<FLexUIClipData>>& ClipDataList)
{
	if (!bClipDirty)return;
	bClipDirty = false;
	
	if (bNeedRecreateClip && ClipData.IsValid())
	{
		if (ClipData.Pin()->GetWidget() == this)//remove old clip-data
		{
			ClipDataList.Remove(ClipData.Pin());
		}
		else
		{
			ClipData = nullptr;//will create new
		}
	}
	bNeedRecreateClip = false;
	
	TSharedPtr<FLexUIClipData> ParentClip = nullptr;
	if (Parent.IsValid())
	{
		ParentClip = Parent->ClipData.Pin();
	}
	switch (Clipping)
	{
	case ELexWidgetClipping::Inherit:
		this->ClipData = ParentClip;
		break;
	case ELexWidgetClipping::ClipToBounds:
		{
			if (!this->ClipData.IsValid())
			{
				auto NewClip = MakeShared<FLexUIClipData>(ParentClip, ClipDataTexture, this);
				ClipDataList.Add(NewClip);
				this->ClipData = NewClip;
			}
		}
		break;
	case ELexWidgetClipping::ClipToBoundsWithoutIntersecting:
		{
			if (!this->ClipData.IsValid())
			{
				auto NewClip = MakeShared<FLexUIClipData>(nullptr, ClipDataTexture, this);
				ClipDataList.Add(NewClip);
				this->ClipData = NewClip;
			}
		}
		break;
	case ELexWidgetClipping::Disabled:
		this->ClipData = nullptr;
		break;
	}
	if (ClipData.IsValid() && ClipData.Pin()->GetWidget() == this)
	{
		ClipData.Pin()->MarkNeedUpdateData();
	}
	if (Visual)
	{
		Visual->CheckClipDataStartPosition();
	}
}

void ULexWidget::SetRenderCanvas(ULexCanvas* InNewCanvas)
{
	auto OldRenderCanvas = RenderCanvas;
	RenderCanvas = InNewCanvas;
	if (ClipData.IsValid() && ClipData.Pin()->GetWidget() == this)//delete old clip-data
	{
		if (OldRenderCanvas.IsValid())
		{
			OldRenderCanvas->RemoveClipData(ClipData.Pin());//remove it from old canvas
		}
	}
	if (OldRenderCanvas.IsValid())
	{
		OldRenderCanvas->RemoveLexWidget(this);
		if (IsValid(Visual))
		{
			OldRenderCanvas->MarkVisualWillChange(Visual);
			OldRenderCanvas->UnregisterVisual(Visual);
		}
	}
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->AddLexWidget(this);
		bClipDirty = true;//mark it dirty so it will be added to new canvas
		if (IsValid(Visual))
		{
			RenderCanvas->RegisterVisual(Visual);
		}
	}
	OnRenderCanvasChanged(OldRenderCanvas.Get(), RenderCanvas.Get());
}

void ULexWidget::OnHierarchyAttachmentChanged(ULexCanvas* ParentRenderCanvas, ULexWidget* ParentRoot)
{
	auto ThisRenderCanvas = this->GetComponent<ULexCanvas>();
	if (ThisRenderCanvas != nullptr)
	{
		ParentRenderCanvas = ThisRenderCanvas;
	}

	if (RenderCanvas != ParentRenderCanvas)//if attach to new Canvas, need to remove from old and add to new
	{
		SetRenderCanvas(ParentRenderCanvas);
	}

	CheckRootWidget(ParentRoot);
	for (auto& Child : Children)
	{
		if (IsValid(Child))
		{
			Child->OnHierarchyAttachmentChanged(ParentRenderCanvas, ParentRoot);
		}
	}

	//flatten hierarchy index
	MarkFlattenHierarchyIndexDirty();

	{
		bCacheWidthDirty = true;
		bCacheHeightDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		
		MarkAnchorDataChanged_Recursive(false, true, true, false, false);
		MarkLayoutForRebuild(this);
	}

	Call_AttachmentChanged();
}

void ULexWidget::OnRenderCanvasChanged(ULexCanvas* OldCanvas, ULexCanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveLexWidget(this);
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddLexWidget(this);
	}
	if (IsValid(Visual))
	{
		Visual->OnRenderCanvasChanged(OldCanvas, NewCanvas);
	}
}

void ULexWidget::CheckRootWidget(ULexWidget* RootWidgetInParent)
{
	if (RootWidgetInParent == nullptr)
	{
		ULexWidget* TopWidget = this;
		ULexWidget* TempRootWidget = nullptr;
		while (TopWidget != nullptr)
		{
			TempRootWidget = TopWidget;
			TopWidget = TopWidget->GetParent();
		}
		RootWidgetInParent = TempRootWidget;
	}
	RootWidget = RootWidgetInParent;
}

void ULexWidget::CalculateWidgetActive_Recursive()
{
	struct LOCAL
	{
		static void CalculateWidgetActive(ULexWidget* Widget)
		{
			bool bResultActive = true;
			bool bSelfActiveForRender = Widget->bWidgetActive;
			if (!bSelfActiveForRender)
				bResultActive = false;
			else if (Widget->Parent.IsValid())
				bResultActive = Widget->Parent->GetWidgetActiveInHierarchy();
			else
				bResultActive = true;

			if (Widget->bCacheWidgetActiveInHierarchy != bResultActive)
			{
				Widget->bCacheWidgetActiveInHierarchy = bResultActive;
				//callback
				Widget->Call_WidgetActiveChanged();
				//canvas update
				Widget->MarkCanvasUpdate(true);
				//refresh layout tree
				if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(Widget->GetWorld()))
				{
					LexUIManager->MarkRebuildAllLayoutTree();
				}
				//tell layout
				MarkLayoutForRebuild(Widget);
			}
			for (auto& Child : Widget->GetChildren())
			{
				CalculateWidgetActive(Child);
			}
		}
	};
	LOCAL::CalculateWidgetActive(this);
}
void ULexWidget::CalculateInteractable_Recursive()
{
	struct LOCAL
	{
		static void CalculateInteractable(ULexWidget* Widget)
		{
			bool bResultInteractable = true;
			switch (Widget->Interactable)
			{
			case ELexWidgetInteractableType::Enabled:
				bResultInteractable = true;
				break;
			case ELexWidgetInteractableType::Disabled:
				bResultInteractable = false;
				break;
			case ELexWidgetInteractableType::Inherit:
				if (Widget->Parent.IsValid())
					bResultInteractable = Widget->Parent->GetInteractableInHierarchy();
				else
					bResultInteractable = true;
				break;
			}

			if (Widget->bCacheInteractableInHierarchy != bResultInteractable)
			{
				Widget->bCacheInteractableInHierarchy = bResultInteractable;
				Widget->Call_InteractableChanged();
			}
			for (auto& Child : Widget->GetChildren())
			{
				CalculateInteractable(Child);
			}
		}
	};
	LOCAL::CalculateInteractable(this);
}
void ULexWidget::CalculateRaycastable_Recursive()
{
	struct LOCAL
	{
		static void CalculateRaycastable(ULexWidget* Widget)
		{
			bool bResult = true;
			switch (Widget->Raycastable)
			{
			case ELexWidgetRaycastableType::Disabled:
				bResult = false;
				break;
			case ELexWidgetRaycastableType::Enabled:
				bResult = true;
				break;
			case ELexWidgetRaycastableType::Inherit:
				if (Widget->Parent.IsValid())
					bResult = Widget->Parent->GetRaycastableInHierarchy();
				else
					bResult = true;
				break;
			}

			if (Widget->bCacheRaycastableInHierarchy != bResult)
			{
				Widget->bCacheRaycastableInHierarchy = bResult;
				Widget->Call_RaycastableChanged();
			}
			for (auto& Child : Widget->GetChildren())
			{
				CalculateRaycastable(Child);
			}
		}
	};
	LOCAL::CalculateRaycastable(this);
}

ULexWidget* ULexWidget::GetChildByIndex(int index)const
{
	if (index < 0 || index >= Children.Num())
	{
		UE_LOG(LGUI, Error, TEXT("[%s].%d Index:%d out of range[%d, %d]"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, index, 0, Children.Num() - 1);
		return nullptr;
	}
	EnsureUIChildrenSorted();
	return Children[index];
}

ULexCanvas* ULexWidget::GetRootCanvas()const
{
	if (RenderCanvas.IsValid())
	{
		return RenderCanvas->GetRootCanvas();
	}
	return nullptr;
}

ULexWidgetPresenterComponentBase* ULexWidget::GetWidgetPresenterComponent() const
{
	if (auto RootCanvas = GetRootCanvas())
	{
		return RootCanvas->GetWidgetPresenterComponent();
	}
	return nullptr;
}

FVector2D ULexWidget::GetLocalSpaceLeftBottomPoint()const
{
	FVector2D leftBottomPoint;
	leftBottomPoint.X = GetWidth() * -AnchorData.Pivot.X;
	leftBottomPoint.Y = GetHeight() * -AnchorData.Pivot.Y;
	return leftBottomPoint;
}
FVector2D ULexWidget::GetLocalSpaceRightTopPoint()const
{
	FVector2D rightTopPoint;
	rightTopPoint.X = GetWidth() * (1.0f - AnchorData.Pivot.X);
	rightTopPoint.Y = GetHeight() * (1.0f - AnchorData.Pivot.Y);
	return rightTopPoint;
}
FVector2D ULexWidget::GetLocalSpaceCenter()const
{
	return FVector2D(this->GetWidth() * (0.5f - AnchorData.Pivot.X), this->GetHeight() * (0.5f - AnchorData.Pivot.Y));
}

float ULexWidget::GetLocalSpaceLeft()const
{
	return this->GetWidth() * -AnchorData.Pivot.X;
}
float ULexWidget::GetLocalSpaceRight()const
{
	return this->GetWidth() * (1.0f - AnchorData.Pivot.X);
}
float ULexWidget::GetLocalSpaceBottom()const
{
	return this->GetHeight() * -AnchorData.Pivot.Y;
}
float ULexWidget::GetLocalSpaceTop()const
{
	return this->GetHeight() * (1.0f - AnchorData.Pivot.Y);
}

void ULexWidget::MarkDimensionChanged(bool InPivotChanged, bool InWidthChanged, bool InHeightChanged)
{
	if (ClipData.IsValid() && ClipData.Pin()->GetWidget() == this)
	{
		ClipData.Pin()->MarkNeedUpdateData();
	}

	OnDimensionChangedEvent.Broadcast(InPivotChanged, InWidthChanged, InHeightChanged);
	if (IsValid(LayoutContainer))
	{
		LayoutContainer->OnDimensionChanged(InPivotChanged, InWidthChanged, InHeightChanged);
	}
	if (GetLayoutSelf())
	{
		LayoutSelf->OnDimensionChanged(InPivotChanged, InWidthChanged, InHeightChanged);
	}
	if (IsValid(Visual))
	{
		Visual->OnDimensionChanged(InPivotChanged, InWidthChanged, InHeightChanged);
	}

	if (this->RenderCanvas.IsValid())
	{
		this->RenderCanvas->MarkCanvasUpdate(InPivotChanged || InWidthChanged || InHeightChanged);//mark canvas to update
		if (this->IsCanvasWidget())
		{
			this->RenderCanvas->MarkTransformOrDimensionChanged();
		}
	}

	Call_DimensionsChanged(InPivotChanged, InWidthChanged, InHeightChanged);
}

void ULexWidget::MarkTransformChanged()
{
	if (ClipData.IsValid() && ClipData.Pin()->GetWidget() == this)
	{
		ClipData.Pin()->MarkNeedUpdateData();
	}
	if (this->RenderCanvas.IsValid())
	{
		this->RenderCanvas->MarkCanvasUpdate(true);//mark canvas to update
		if (this->IsCanvasWidget())
		{
			//This is mainly to mark LGUICanvas's bIsViewProjectionMatrixDirty to true.
			//For the condition LGUI_Tutorials/Tutorials/UIRenderTarget, when move LGUIRenderTarget at runtime, the LGUICanvas's RenderTarget's matrix not update, result in wrong interaction.
			this->RenderCanvas->MarkTransformOrDimensionChanged();
		}
	}

	Call_TransformChanged();
}

void ULexWidget::MarkAnchorDataChanged_Recursive(bool InPivotChanged, bool InWidthChanged, bool InHeightChanged, bool InDiscardCache, bool InPropagateToChildren)
{
	CalculateTransformFromAnchor();

	if (InDiscardCache)
	{
		if (InWidthChanged)
		{
			bCacheWidthDirty = true;
		}
		if (InHeightChanged)
		{
			bCacheHeightDirty = true;
		}
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
	}
	MarkDimensionChanged(InPivotChanged, InWidthChanged, InHeightChanged);

	if (!InPropagateToChildren)return;
	for (auto& Child : GetChildren())
	{
		if (!IsValid(Child))continue;
		bool ChildWidthChange = false, ChildHeightChange = false;
		if (InWidthChanged && Child->AnchorData.IsHorizontalStretched())
		{
			ChildWidthChange = true;
		}
		if (InHeightChanged && Child->AnchorData.IsVerticalStretched())
		{
			ChildHeightChange = true;
		}
		Child->MarkAnchorDataChanged_Recursive(false, ChildWidthChange, ChildHeightChange);

		//check if child need layout rebuild, the widget self is already marked outside of this function
		if (ChildWidthChange || ChildHeightChange//parent size change may cause child layout change
			|| ((InWidthChanged || InHeightChanged) && Child->GetLayoutSelf())//parent size changed and parent can affect child layout, need calculate child layout
			)
		{
			MarkLayoutForRebuild(Child);
		}
	}
}

void ULexWidget::MarkCanvasUpdate(bool bRebuildDrawCall)const
{
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkCanvasUpdate(bRebuildDrawCall);
	}
}

void ULexWidget::SetPositionAndSizeForLayoutAnimation(FVector2D Position, FVector2D Size)
{
	bool AnyChanged = false;
	if (!AnchorData.AnchoredPosition.Equals(Position, 0.0f))
	{
		AnyChanged = true;
		AnchorData.AnchoredPosition = Position;
	}
	if (!AnchorData.SizeDelta.Equals(Size, 0.0f))
	{
		AnyChanged = true;
		AnchorData.SizeDelta = Size;
		CacheWidth = Size.X;
		CacheHeight = Size.Y;
	}
	if (AnyChanged)
	{
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChangedByLayoutContainer_Recursive(false, true, true, true);
	}
}

void ULexWidget::SetPositionForLayoutAnimation(FVector2D Position)
{
	if (!AnchorData.AnchoredPosition.Equals(Position, 0.0f))
	{
		AnchorData.AnchoredPosition = Position;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChangedByLayoutContainer_Recursive(false, true, true, true);
	}
}

void ULexWidget::SetSizeForLayoutAnimation(FVector2D Size)
{
	if (!AnchorData.SizeDelta.Equals(Size, 0.0f))
	{
		AnchorData.SizeDelta = Size;
		CacheWidth = Size.X;
		CacheHeight = Size.Y;
		
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		MarkAnchorDataChangedByLayoutContainer_Recursive(false, true, true, true);
	}
}

void ULexWidget::MarkAnchorDataChangedByLayoutContainer_Recursive(bool InPivotChanged, bool InWidthChanged,
                                                                  bool InHeightChanged, bool InDiscardCache, bool InPropagateToChildren)
{
	CalculateTransformFromAnchor();

	if (InDiscardCache)
	{
		if (InWidthChanged)
		{
			bCacheWidthDirty = true;
		}
		if (InHeightChanged)
		{
			bCacheHeightDirty = true;
		}
		bCacheAnchorOffsetLeftDirty = true;
		bCacheAnchorOffsetRightDirty = true;
		bCacheAnchorOffsetBottomDirty = true;
		bCacheAnchorOffsetTopDirty = true;
	}
	MarkDimensionChanged(InPivotChanged, InWidthChanged, InHeightChanged);

	if (!InPropagateToChildren)return;
	for (auto& Child : GetChildren())
	{
		if (!IsValid(Child))continue;
		bool ChildWidthChange = false, ChildHeightChange = false;
		if (InWidthChanged && Child->AnchorData.IsHorizontalStretched())
		{
			ChildWidthChange = true;
		}
		if (InHeightChanged && Child->AnchorData.IsVerticalStretched())
		{
			ChildHeightChange = true;
		}
		Child->MarkAnchorDataChangedByLayoutContainer_Recursive(false, ChildWidthChange, ChildHeightChange);
	}
}

ULexCanvas* ULexWidget::GetRenderCanvas()const
{
	return RenderCanvas.Get();
}

bool ULexWidget::IsScreenSpaceOverlayUI()const
{
	if (!RenderCanvas.IsValid())return false;
	return RenderCanvas->IsRenderToScreenSpace();
}
bool ULexWidget::IsRenderTargetUI()const
{
	if (!RenderCanvas.IsValid())return false;
	return RenderCanvas->IsRenderToRenderTarget();
}
bool ULexWidget::IsWorldSpaceUI()const
{
	if (!RenderCanvas.IsValid())return false;
	return RenderCanvas->IsRenderToWorldSpace();
}

void ULexWidget::MarkLayoutForRebuild(ULexWidget* InWidget)
{
	auto RootWidgetOfLayoutTree = InWidget;
	//move up, find if parent widget affect by layout then mark dirty
	while (RootWidgetOfLayoutTree)
	{
		if (auto LayoutContainer = RootWidgetOfLayoutTree->GetLayoutContainer())
		{
			LayoutContainer->MarkLayoutDirty();
		}
		if (auto LayoutSelf = RootWidgetOfLayoutTree->GetLayoutSelf())
		{
			LayoutSelf->MarkLayoutDirty();
		}
		if (auto ParentWidget = RootWidgetOfLayoutTree->GetParent())
		{
			if (ParentWidget->GetLayoutContainer())//parent contains LayoutContainer, need calculate layout
			{
				RootWidgetOfLayoutTree = ParentWidget;
				continue;
			}
		}
		break;
	}
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(RootWidgetOfLayoutTree->GetWorld()))
	{
		LexUIManager->AddLayoutDirtyWidget(RootWidgetOfLayoutTree);
	}
}

void ULexWidget::RebuildLayoutImmediately(ULexWidget* InWidget)
{
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(InWidget->GetWorld()))
	{
		LexUIManager->RebuildLayoutImmediately(InWidget);
	}
}

void ULexWidget::MarkClipDirty(bool InClipTypeChanged) const
{
	bClipDirty = true;
	if (InClipTypeChanged)bNeedRecreateClip = true;
	struct LOCAL
	{
		static void MarkDirty(const ULexWidget* Widget, bool InClipTypeChanged)
		{
			switch (Widget->Clipping)
			{
			case ELexWidgetClipping::Inherit:
			case ELexWidgetClipping::ClipToBounds:
				Widget->bClipDirty = true;
				if (InClipTypeChanged)Widget->bNeedRecreateClip = true;
				break;
			case ELexWidgetClipping::ClipToBoundsWithoutIntersecting:
			case ELexWidgetClipping::Disabled:
				return;
			}

			for (auto& Child : Widget->GetChildren())
			{
				MarkDirty(Child, InClipTypeChanged);
			}
		}
	};
	for (auto& Child : this->GetChildren())
	{
		LOCAL::MarkDirty(Child, InClipTypeChanged);
	}
}
bool ULexWidget::IsPointVisibleOnClip(const FVector& Value) const
{
	if (ClipData.IsValid())
	{
		return ClipData.Pin()->IsPointVisible(Value);
	}
	return true;
}
void ULexWidget::SetClipping(ELexWidgetClipping Value)
{
	if (Clipping != Value)
	{
		Clipping = Value;
		MarkClipDirty(true);
	}
}
void ULexWidget::SetClippingCornerRadius(FVector4f Value)
{
	if (ClippingCornerRadius != Value)
	{
		ClippingCornerRadius = Value;
		MarkClipDirty(false);
	}
}

void ULexWidget::SetClippingMargin(FMargin Value)
{
	if (ClippingMargin != Value)
	{
		ClippingMargin = Value;
		MarkClipDirty(false);
	}
}

float ULexWidget::GetFinalRenderOpacity()const
{
	if (!bIgnoreParentRenderOpacity && Parent.IsValid())
	{
		return this->RenderOpacity * Parent->GetFinalRenderOpacity();
	}
	return this->RenderOpacity;
}
void ULexWidget::MarkRenderOpacityDirty_Recursive()const
{
	if (Visual)
	{
		Visual->MarkColorDirty();
	}
	for (auto& Child : Children)
	{
		Child->MarkRenderOpacityDirty_Recursive();
	}
}
void ULexWidget::SetRenderOpacity(float Value)
{
	Value = FMath::Clamp(Value, 0.0f, 1.0f);
	if (RenderOpacity != Value)
	{
		RenderOpacity = Value;
		MarkRenderOpacityDirty_Recursive();
	}
}
void ULexWidget::SetIgnoreParentRenderOpacity(bool Value)
{
	if (bIgnoreParentRenderOpacity != Value)
	{
		bIgnoreParentRenderOpacity = Value;
		MarkRenderOpacityDirty_Recursive();
	}
}

bool ULexWidget::GetPixelSnappingInHierarchy() const
{
	switch (this->PixelSnapping)
	{
	case EWidgetPixelSnapping::SnapToPixel:
		return true;
	case EWidgetPixelSnapping::Disabled:
		return false;
	case EWidgetPixelSnapping::Inherit:
		if (Parent.IsValid())
		{
			return Parent->GetPixelSnappingInHierarchy();
		}
		return false;
	}
	return false;
}

void ULexWidget::SetPixelSnapping(EWidgetPixelSnapping Value)
{
	if (PixelSnapping != Value)
	{
		PixelSnapping = Value;
		struct LOCAL
		{
			static void MarkChanged(const ULexWidget* Widget)
			{
				if (Widget->Visual)
				{
					Widget->Visual->OnPixelSnappingChanged();
				}
				for (auto& Child : Widget->GetChildren())
				{
					MarkChanged(Child);
				}
			}
		};
		LOCAL::MarkChanged(this);
	}
}

bool ULexWidget::GetWidgetActiveInHierarchy() const
{
	return bCacheWidgetActiveInHierarchy;
}

void ULexWidget::SetWidgetActive(bool Value)
{
	if (bWidgetActive != Value)
	{
		bWidgetActive = Value;
		CalculateWidgetActive_Recursive();
	}
}

void ULexWidget::SetRaycastable(ELexWidgetRaycastableType Value)
{
	if (Raycastable != Value)
	{
		Raycastable = Value;
		CalculateRaycastable_Recursive();
	}
}

void ULexWidget::SetInteractable(ELexWidgetInteractableType Value)
{
	if (Interactable != Value)
	{
		Interactable = Value;
		CalculateInteractable_Recursive();
	}
}

void ULexWidget::SetIgnoreLayout(bool Value)
{
	if (bIgnoreLayout != Value)
	{
		bIgnoreLayout = Value;
		MarkLayoutForRebuild(this);
		if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
		{
			LexUIManager->MarkRebuildAllLayoutTree();
		}
	}
}

const ULexWidget* ULexWidget::GetRestrictNavigationAreaWidget() const
{
	if (bRestrictNavigationArea)
	{
		return this;
	}
	if (Parent.IsValid())
	{
		return Parent->GetRestrictNavigationAreaWidget();
	}
	return nullptr;
}

void ULexWidget::SetRestrictNavigationArea(bool Value)
{
	bRestrictNavigationArea = Value;
}

ULexVisual* ULexWidget::GetVisualAs(TSubclassOf<ULexVisual> VisualClass) const
{
	if (Visual && Visual->IsA(VisualClass))
		return Visual;
	return nullptr;
}

ULexVisual* ULexWidget::CreateNewVisual(TSubclassOf<ULexVisual> VisualClass)
{
	auto OldVisual = Visual;
	auto NewVisual = NewObject<ULexVisual>(this, VisualClass, NAME_None, RF_Public | RF_Transactional);
	if (RenderCanvas.IsValid())
	{
		if (IsValid(OldVisual))
		{
			RenderCanvas->MarkVisualWillChange(OldVisual);
			RenderCanvas->UnregisterVisual(OldVisual);
		}
		if (NewVisual)
		{
			RenderCanvas->RegisterVisual(NewVisual);
		}
	}
	if (IsValid(OldVisual))
	{
		if (bHasBegunPlay)
		{
			OldVisual->EndPlay();
		}
		OldVisual->Call_OnUnregister();
	}
	
	NewVisual->Call_OnRegister();
	if (bHasBegunPlay)
	{
		NewVisual->BeginPlay();
	}
	Visual = NewVisual;
	return NewVisual;
}

void ULexWidget::RemoveVisual()
{
	auto OldVisual = Visual;
	Visual = nullptr;

	if (IsValid(OldVisual))
	{
		if (bHasBegunPlay)
		{
			OldVisual->EndPlay();
		}
		OldVisual->Call_OnUnregister();
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->MarkVisualWillChange(OldVisual);
			RenderCanvas->UnregisterVisual(OldVisual);
		}
	}
}

ULexLayoutContainer* ULexWidget::CreateNewLayoutContainer(TSubclassOf<ULexLayoutContainer> LayoutClass)
{
	auto OldLayout = LayoutContainer;
	auto NewLayout = NewObject<ULexLayoutContainer>(this, LayoutClass, NAME_None, RF_Public | RF_Transactional);
	if (IsValid(OldLayout))
	{
		if (bHasBegunPlay)
		{
			OldLayout->EndPlay();
		}
		OldLayout->Call_OnUnregister();
	}
	
	NewLayout->Call_OnRegister();
	if (bHasBegunPlay)
	{
		NewLayout->BeginPlay();
	}
	LayoutContainer = NewLayout;
	MarkLayoutForRebuild(this);
	MarkDimensionChanged(false, true, true);//change LayoutContainer could cause LayoutSelf size change
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->MarkRebuildAllLayoutTree();
	}
	return NewLayout;
}

void ULexWidget::RemoveLayoutContainer()
{
	auto OldLayout = LayoutContainer;
	LayoutContainer = nullptr;

	if (IsValid(OldLayout))
	{
		if (bHasBegunPlay)
		{
			OldLayout->EndPlay();
		}
		OldLayout->Call_OnUnregister();
	}
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->MarkRebuildAllLayoutTree();
	}
}

ULexLayoutSelf* ULexWidget::CreateNewLayoutSelf(TSubclassOf<ULexLayoutSelf> LayoutClass)
{
	auto OldLayout = LayoutSelf;
	auto NewLayout = NewObject<ULexLayoutSelf>(this, LayoutClass, NAME_None, RF_Public | RF_Transactional);
	if (IsValid(OldLayout))
	{
		if (bHasBegunPlay)
		{
			OldLayout->EndPlay();
		}
		OldLayout->Call_OnUnregister();
	}
	
	NewLayout->Call_OnRegister();
	if (bHasBegunPlay)
	{
		NewLayout->BeginPlay();
	}
	LayoutSelf = NewLayout;
	MarkLayoutForRebuild(this);
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->MarkRebuildAllLayoutTree();
	}
	return NewLayout;
}

void ULexWidget::RemoveLayoutSelf()
{
	auto OldLayout = LayoutSelf;
	LayoutSelf = nullptr;

	if (IsValid(OldLayout))
	{
		if (bHasBegunPlay)
		{
			OldLayout->EndPlay();
		}
		OldLayout->Call_OnUnregister();
	}
	if (auto LexUIManager = ULexUIManagerWorldSubsystem::GetInstance(GetWorld()))
	{
		LexUIManager->MarkRebuildAllLayoutTree();
	}
}

#pragma region TweenAnimation


#pragma region PositionXYZ
ULTweener* ULexWidget::LocalPositionXTo(double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenDoubleGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetRelativeLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(this, [this](auto value) {
		auto location = this->GetRelativeLocation();
		location.X = value;
		this->SetRelativeLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::LocalPositionYTo(double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenDoubleGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetRelativeLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(this, [this](auto value) {
		auto location = this->GetRelativeLocation();
		location.Y = value;
		this->SetRelativeLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::LocalPositionZTo(double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenDoubleGetterFunction::CreateWeakLambda(this, [this] 
	{
		return this->GetRelativeLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(this, [this](auto value) {
		auto location = this->GetRelativeLocation();
		location.Z = value;
		this->SetRelativeLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}



ULTweener* ULexWidget::WorldPositionXTo(double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenDoubleGetterFunction::CreateWeakLambda(this, [this] 
	{
		return this->GetWorldLocation().X;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(this, [this](auto value) {
		auto location = this->GetWorldLocation();
		location.X = value;
		this->SetWorldLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::WorldPositionYTo(double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenDoubleGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetWorldLocation().Y;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(this, [this](auto value) {
		auto location = this->GetWorldLocation();
		location.Y = value;
		this->SetWorldLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::WorldPositionZTo(double endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenDoubleGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetWorldLocation().Z;
	}), FLTweenDoubleSetterFunction::CreateWeakLambda(this, [this](auto value) {
		auto location = this->GetWorldLocation();
		location.Z = value;
		this->SetWorldLocation(location);
	}), endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
#pragma endregion PositionXYZ




#pragma region Position
ULTweener* ULexWidget::LocalPositionTo(FVector endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
	, FLTweenVectorGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetRelativeLocation();
	})
	, FLTweenVectorSetterFunction::CreateWeakLambda(this, [this](FVector value)
	{
		this->SetRelativeLocation(value);
	})
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::WorldPositionTo(FVector endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
	, FLTweenVectorGetterFunction::CreateUObject(this, &ULexWidget::GetWorldLocation)
	, FLTweenVectorSetterFunction::CreateWeakLambda(this, [this](FVector value)
	{
		return this->SetWorldLocation(value);
	})
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
#pragma endregion Position



ULTweener* ULexWidget::LocalScaleTo(FVector endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
	, FLTweenVectorGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetRelativeScale();
	})
	, FLTweenVectorSetterFunction::CreateWeakLambda(this, [this](FVector value)
	{
		this->SetRelativeScale(value);
	})
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::LocalUniformScaleTo(float endValue, float duration, float delay,	ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
	, FLTweenFloatGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetRelativeScale().X;
	})
	, FLTweenFloatSetterFunction::CreateWeakLambda(this, [this](float value)
	{
		this->SetRelativeScale(FVector(value));
	})
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}


#pragma region Rotation
ULTweener* ULexWidget::LocalRotationQuaternionTo(const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
	, FLTweenQuaternionGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetRelativeRotation();
	}), FLTweenQuaternionSetterFunction::CreateUObject(this, &ULexWidget::SetRelativeRotation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::LocalRotatorTo(FRotator endValue, bool shortestPath, float duration, float delay, ELTweenEase ease)
{
	if (shortestPath)
	{
		return LocalRotationQuaternionTo(endValue.Quaternion(), duration, delay, ease);
	}
	else
	{
		auto Tweener = ULTweenManager::To(this
		, FLTweenRotatorGetterFunction::CreateWeakLambda(this, [this]
		{
			return this->GetRelativeRotation().Rotator();
		})
		, FLTweenRotatorSetterFunction::CreateWeakLambda(this, [this] (FRotator value)
		{
			this->SetRelativeRotation(value.Quaternion());
		}), endValue, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->SetEase(ease);
			ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
		}
		return Tweener;
	}
}



ULTweener* ULexWidget::WorldRotationQuaternionTo(const FQuat& endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this, FLTweenQuaternionGetterFunction::CreateWeakLambda(this, [this]
	{
		return this->GetWorldRotation();
	}), FLTweenQuaternionSetterFunction::CreateUObject(this, &ULexWidget::SetWorldRotation)
	, endValue, duration);
	if (Tweener)
	{
		Tweener->SetDelay(delay)->SetEase(ease);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}
ULTweener* ULexWidget::WorldRotatorTo(FRotator endValue, bool shortestPath, float duration, float delay, ELTweenEase ease)
{
	if (shortestPath)
	{
		return WorldRotationQuaternionTo(endValue.Quaternion(), duration, delay, ease);
	}
	else
	{
		auto Tweener = ULTweenManager::To(this
		, FLTweenRotatorGetterFunction::CreateWeakLambda(this, [this]
		{
			return this->GetWorldRotation().Rotator();
		})
		, FLTweenRotatorSetterFunction::CreateWeakLambda(this, [this](FRotator value)
		{
			this->SetWorldRotation(value.Quaternion());
		}), endValue, duration);
		if (Tweener)
		{
			Tweener->SetDelay(delay)->SetEase(ease);
			ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
		}
		return Tweener;
	}
}

#pragma endregion Rotation


ULTweener* ULexWidget::RenderOpacityTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateUObject(this, &ULexWidget::GetRenderOpacity)
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULexWidget::SetRenderOpacity)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::SizeDeltaTo(const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenVector2DGetterFunction::CreateUObject(this, &ULexWidget::GetSizeDelta)
		, FLTweenVector2DSetterFunction::CreateUObject(this, &ULexWidget::SetSizeDelta)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::WidthTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateUObject(this, &ULexWidget::GetWidth)
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULexWidget::SetWidth)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::HeightTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateUObject(this, &ULexWidget::GetHeight)
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULexWidget::SetHeight)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::AnchoredPositionTo(const FVector2D& endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenVector2DGetterFunction::CreateUObject(this, &ULexWidget::GetSizeDelta)
		, FLTweenVector2DSetterFunction::CreateUObject(this, &ULexWidget::SetSizeDelta)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::HorizontalAnchoredPositionTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateUObject(this, &ULexWidget::GetHorizontalAnchoredPosition)
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULexWidget::SetHorizontalAnchoredPosition)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

ULTweener* ULexWidget::VerticalAnchoredPositionTo(float endValue, float duration, float delay, ELTweenEase ease)
{
	auto Tweener = ULTweenManager::To(this
		, FLTweenFloatGetterFunction::CreateUObject(this, &ULexWidget::GetVerticalAnchoredPosition)
		, FLTweenFloatSetterFunction::CreateUObject(this, &ULexWidget::SetVerticalAnchoredPosition)
		, endValue, duration);
	if (Tweener)
	{
		Tweener->SetEase(ease)->SetDelay(delay);
		ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(this, Tweener);
	}
	return Tweener;
}

void ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(ULexWidget* Widget, ULTweener* Tweener)
{
	if (Tweener)
	{
		bool bAffectByGamePause;
		bool bAffectByTimeDilation;
		if (Widget->IsScreenSpaceOverlayUI())
		{
			bAffectByGamePause = GetDefault<ULexUISettings>()->bScreenSpaceUIAffectByGamePause;
			bAffectByTimeDilation = GetDefault<ULexUISettings>()->bScreenSpaceUIAffectByTimeDilation;
		}
		else
		{
			bAffectByGamePause = GetDefault<ULexUISettings>()->bWorldSpaceUIAffectByGamePause;
			bAffectByTimeDilation = GetDefault<ULexUISettings>()->bWorldSpaceUIAffectByTimeDilation;
		}
		Tweener->SetAffectByGamePause(bAffectByGamePause)->SetAffectByTimeDilation(bAffectByTimeDilation);
	}
}
#pragma endregion

