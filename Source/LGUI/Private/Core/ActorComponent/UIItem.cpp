// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
#endif

//PRAGMA_DISABLE_OPTIMIZATION

UUIItem::UUIItem(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;
	SetMobility(EComponentMobility::Movable);
	SetUsingAbsoluteLocation(false);
	SetUsingAbsoluteRotation(false);
	SetUsingAbsoluteScale(false);
	SetVisibility(false);
	bWantsOnUpdateTransform = true;
	bCanSetAnchorFromTransform = false;//skip construction

	bFlattenHierarchyIndexChanged = true;
	bLayoutChanged = true;
	bSizeChanged = true;
	bShouldUpdateRootUIItemLayout = true;
	bNeedUpdateRootUIItem = true;
	bFlattenHierarchyIndexDirty = true;

	bIsCanvasUIItem = false;

	traceChannel = GetDefault<ULGUISettings>()->defaultTraceChannel;
}

void UUIItem::BeginPlay()
{
	Super::BeginPlay();

	GetParentAsUIItem();
	CheckRootUIItem();

	bFlattenHierarchyIndexChanged = true;
	bLayoutChanged = true;
	bSizeChanged = true;
	bShouldUpdateRootUIItemLayout = true;
	bNeedUpdateRootUIItem = true;
	MarkLayoutDirty(true);
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
	OnUIActiveInHierachy(GetIsUIActiveInHierarchy());
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
		//if (!CanExecuteOnUIBehaviour(CompItem))continue;//why comment this? because UIActiveInHierarchy is related to Awake
		CompItem->OnUIActiveInHierachy(GetIsUIActiveInHierarchy());
	}
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIChildAcitveInHierarchy(child, activeOrInactive);
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIDimensionsChanged(positionChanged, sizeChanged);
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIAttachmentChanged();
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIChildAttachmentChanged(child, attachOrDettach);
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIInteractionStateChanged(interactable);
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
		if (!CanExecuteOnUIBehaviour(CompItem))continue;
		CompItem->OnUIChildHierarchyIndexChanged(child);
	}
}
bool UUIItem::CanExecuteOnUIBehaviour(class ULGUILifeCycleUIBehaviour* InComp)
{
	return InComp->bIsAwakeCalled;//Awake not called, means not initialized yet
}
#pragma endregion LGUILifeCycleUIBehaviour


void UUIItem::OnChildHierarchyIndexChanged(UUIItem* child)
{
	CallUILifeCycleBehavioursChildHierarchyIndexChanged(child);
}

void UUIItem::CalculateFlattenHierarchyIndex_Recursive(int& index)const
{
	if (this->flattenHierarchyIndex != index)
	{
		this->flattenHierarchyIndex = index;
	}
	for (auto child : UIChildren)
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
#if !UE_BUILD_SHIPPING
	check(this == RootUIItem.Get());
#endif
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
	bFlattenHierarchyIndexChanged = true;
	if (RootUIItem.IsValid())
	{
		RootUIItem->bFlattenHierarchyIndexDirty = true;
	}
}

void UUIItem::SetHierarchyIndex(int32 InInt) 
{ 
	if (InInt != hierarchyIndex)
	{
		if (ParentUIItem.IsValid())
		{
			ParentUIItem->CheckCacheUIChildren();
			UUIItem* existChildOfIndex = nullptr;
			for (int i = 0; i < ParentUIItem->UIChildren.Num(); i++)
			{
				if (ParentUIItem->UIChildren[i]->hierarchyIndex == InInt)
				{
					existChildOfIndex = ParentUIItem->UIChildren[i];
					break;
				}
			}
			if (existChildOfIndex != nullptr)//already exist
			{
				if (InInt < hierarchyIndex)//move to prev
				{
					for (int i = InInt; i < ParentUIItem->UIChildren.Num(); i++)
					{
						ParentUIItem->UIChildren[i]->hierarchyIndex++;
					}
				}
				else//move to next
				{
					for (int i = 0; i <= InInt; i++)
					{
						ParentUIItem->UIChildren[i]->hierarchyIndex--;
					}
				}
			}
			hierarchyIndex = InInt;
			ParentUIItem->SortCacheUIChildren();
			for (int i = 0; i < ParentUIItem->UIChildren.Num(); i++)
			{
				ParentUIItem->UIChildren[i]->hierarchyIndex = i;
			}
			//flatten hierarchy index
			MarkFlattenHierarchyIndexDirty();

			ParentUIItem->OnChildHierarchyIndexChanged(this);
		}
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
		for (auto childItem : this->UIChildren)
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
			for (auto childItem : this->UIChildren)
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
	for (auto childItem : this->UIChildren)
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
			for (auto childItem : this->UIChildren)
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
	for (auto childItem : this->UIChildren)
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
	bFlattenHierarchyIndexChanged = true;
	bLayoutChanged = true;
	bSizeChanged = true;

	for (auto uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{
			uiChild->MarkAllDirtyRecursive();
		}
	}
}

#if WITH_EDITOR
void UUIItem::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
	
}
void UUIItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	bCanSetAnchorFromTransform = false;
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		auto propetyName = PropertyChangedEvent.Property->GetFName();
		if (propetyName == GET_MEMBER_NAME_CHECKED(UUIItem, bIsUIActive))
		{
			bIsUIActive = !bIsUIActive;//make it work
			SetIsUIActive(!bIsUIActive);
		}

		else if (propetyName == GET_MEMBER_NAME_CHECKED(UUIItem, hierarchyIndex))
		{
			hierarchyIndex = hierarchyIndex + 1;//make it work
			SetHierarchyIndex(hierarchyIndex - 1);
		}

		if (PropertyChangedEvent.MemberProperty->GetFName() == GET_MEMBER_NAME_CHECKED(UUIItem, widget))
		{
			if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, width))
			{
				auto width = widget.width;
				widget.width += 1;//+1 just for make it work
				SetWidth(width);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, height))
			{
				auto height = widget.height;
				widget.height += 1;//+1 just for make it work
				SetHeight(height);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, anchorOffsetX))
			{
				auto originValue = widget.anchorOffsetX;
				widget.anchorOffsetX += 1;
				SetAnchorOffsetY(originValue);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, anchorOffsetY))
			{
				auto originValue = widget.anchorOffsetY;
				widget.anchorOffsetY += 1;
				SetAnchorOffsetZ(originValue);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, stretchLeft))
			{
				auto originValue = widget.stretchLeft;
				widget.stretchLeft += 1;
				SetStretchLeft(originValue);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, stretchRight))
			{
				auto originValue = widget.stretchRight;
				widget.stretchRight += 1;
				SetStretchRight(originValue);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, stretchTop))
			{
				auto originValue = widget.stretchTop;
				widget.stretchTop += 1;
				SetStretchTop(originValue);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, stretchBottom))
			{
				auto originValue = widget.stretchBottom;
				widget.stretchBottom += 1;
				SetStretchBottom(originValue);
			}

			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, anchorHAlign))
			{
				auto newAlign = widget.anchorHAlign;
				widget.anchorHAlign = prevAnchorHAlign;
				SetAnchorHAlign(newAlign);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, anchorVAlign))
			{
				auto newAlign = widget.anchorVAlign;
				widget.anchorVAlign = prevAnchorVAlign;
				SetAnchorVAlign(newAlign);
				prevAnchorVAlign = newAlign;
			}

			else if (propetyName == TEXT("X") || propetyName == TEXT("Y"))
			{
				auto newPivot = widget.pivot;
				widget.pivot += FVector2D(1, 1);
				SetPivot(newPivot);
			}
		}

		MarkAllDirtyRecursive();
		EditorForceUpdateImmediately();
		UpdateBounds();
	}
	bCanSetAnchorFromTransform = true;
}

void UUIItem::PostEditComponentMove(bool bFinished)
{
	Super::PostEditComponentMove(bFinished);
	EditorForceUpdateImmediately();
}

FBoxSphereBounds UUIItem::CalcBounds(const FTransform& LocalToWorld) const
{
	auto origin = FVector(widget.width * (0.5f - widget.pivot.X), widget.height * (0.5f - widget.pivot.Y), 0);
	return FBoxSphereBounds(origin, FVector(widget.width * 0.5f, widget.height * 0.5f, 1), (widget.width > widget.height ? widget.width : widget.height) * 0.5f).TransformBy(LocalToWorld);
}
void UUIItem::EditorForceUpdateImmediately()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	MarkCanvasUpdate();
	MarkUpdateLayout();
}
#endif

bool UUIItem::MoveComponentImpl(const FVector& Delta, const FQuat& NewRotation, bool bSweep, FHitResult* Hit, EMoveComponentFlags MoveFlags, ETeleportType Teleport)
{
	auto result = Super::MoveComponentImpl(Delta, NewRotation, bSweep, Hit, MoveFlags, Teleport);
	if (!Delta.IsNearlyZero(KINDA_SMALL_NUMBER))
	{
		if (bCanSetAnchorFromTransform
			&& this->IsRegistered()//check if registerred, because it may called from reconstruction.
			)
		{
			CalculateAnchorFromTransform();
		}
	}
	return result;
}
void UUIItem::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	MarkLayoutDirty(false);
}
void UUIItem::CalculateAnchorFromTransform()
{
	if (ParentUIItem.IsValid())
	{
		const auto& parentWidget = ParentUIItem->widget;
		if (widget.anchorHAlign != UIAnchorHorizontalAlign::None)
		{
			switch (widget.anchorHAlign)
			{
			case UIAnchorHorizontalAlign::Left:
			{
				float anchorOffsetX = this->GetRelativeLocation().Y + parentWidget.width * parentWidget.pivot.X;
				ApplyAnchorOffsetX(anchorOffsetX);
			}
			break;
			case UIAnchorHorizontalAlign::Center:
			{
				float anchorOffsetX = this->GetRelativeLocation().Y + parentWidget.width * (parentWidget.pivot.X - 0.5f);
				ApplyAnchorOffsetX(anchorOffsetX);
			}
			break;
			case UIAnchorHorizontalAlign::Right:
			{
				float anchorOffsetX = this->GetRelativeLocation().Y + parentWidget.width * (parentWidget.pivot.X - 1.0f);
				ApplyAnchorOffsetX(anchorOffsetX);
			}
			break;
			case UIAnchorHorizontalAlign::Stretch:
			{
				//parent
				float parentLeft, parentRight;
				parentLeft = parentWidget.width * -parentWidget.pivot.X;
				parentRight = parentWidget.width * (1.0f - parentWidget.pivot.X);
				//self, relative to parent
				float selfLeft, selfRight;
				selfLeft = this->GetRelativeLocation().Y + widget.width * -widget.pivot.X;
				selfRight = this->GetRelativeLocation().Y + widget.width * (1.0f - widget.pivot.X);
				//stretch
				ApplyHorizontalStretch(FVector2D(selfLeft - parentLeft, parentRight - selfRight));
			}
			break;
			}
		}
		if (widget.anchorVAlign != UIAnchorVerticalAlign::None)
		{
			switch (widget.anchorVAlign)
			{
			case UIAnchorVerticalAlign::Top:
			{
				float anchorOffsetY = this->GetRelativeLocation().Z + parentWidget.height * (parentWidget.pivot.Y - 1.0f);
				ApplyAnchorOffsetY(anchorOffsetY);
			}
			break;
			case UIAnchorVerticalAlign::Middle:
			{
				float anchorOffsetY = this->GetRelativeLocation().Z + parentWidget.height * (parentWidget.pivot.Y - 0.5f);
				ApplyAnchorOffsetY(anchorOffsetY);
			}
			break;
			case UIAnchorVerticalAlign::Bottom:
			{
				float anchorOffsetY = this->GetRelativeLocation().Z + parentWidget.height * parentWidget.pivot.Y;
				ApplyAnchorOffsetY(anchorOffsetY);
			}
			break;
			case UIAnchorVerticalAlign::Stretch:
			{
				//parent
				float parentBottom, parentTop;
				parentBottom = parentWidget.height * -parentWidget.pivot.Y;
				parentTop = parentWidget.height * (1.0f - parentWidget.pivot.Y);
				//self, relative to parent
				float selfBottom, selfTop;
				selfBottom = this->GetRelativeLocation().Z + widget.height * -widget.pivot.Y;
				selfTop = this->GetRelativeLocation().Z + widget.height * (1.0f - widget.pivot.Y);
				//stretch
				ApplyVerticalStretch(FVector2D(selfBottom - parentBottom, parentTop - selfTop));
			}
			break;
			}
		}
	}
}
void UUIItem::ApplyAnchorOffsetX(float newOffset)
{
	if (FMath::Abs(widget.anchorOffsetX - newOffset) > KINDA_SMALL_NUMBER)
	{
		widget.anchorOffsetX = newOffset;
		if (ParentUIItem.IsValid())
		{
			const auto& parentWidget = ParentUIItem->widget;
			widget.stretchLeft = (parentWidget.width - widget.width) * 0.5f + widget.anchorOffsetX;
			widget.stretchRight = parentWidget.width - widget.width - widget.stretchLeft;
		}
		MarkLayoutDirty(false);
	}
}
void UUIItem::ApplyAnchorOffsetY(float newOffset)
{
	if (FMath::Abs(widget.anchorOffsetY - newOffset) > KINDA_SMALL_NUMBER)
	{
		widget.anchorOffsetY = newOffset;
		if (ParentUIItem.IsValid())
		{
			const auto& parentWidget = ParentUIItem->widget;
			widget.stretchBottom = (parentWidget.height - widget.height) * 0.5f + widget.anchorOffsetY;
			widget.stretchTop = parentWidget.height - widget.height - widget.stretchBottom;
		}
		MarkLayoutDirty(false);
	}
}
void UUIItem::ApplyHorizontalStretch(FVector2D newStretch)
{
	if (FMath::Abs(widget.stretchLeft - newStretch.X) > KINDA_SMALL_NUMBER || FMath::Abs(widget.stretchRight - newStretch.Y) > KINDA_SMALL_NUMBER)
	{
		widget.stretchLeft = newStretch.X;
		widget.stretchRight = newStretch.Y;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
	}
}
void UUIItem::ApplyVerticalStretch(FVector2D newStretch)
{
	if (FMath::Abs(widget.stretchBottom - newStretch.X) > KINDA_SMALL_NUMBER || FMath::Abs(widget.stretchTop - newStretch.Y) > KINDA_SMALL_NUMBER)
	{
		widget.stretchBottom = newStretch.X;
		widget.stretchTop = newStretch.Y;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
	}
}

void UUIItem::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;
	if (UUIItem* childUIItem = Cast<UUIItem>(ChildComponent))
	{
		//UE_LOG(LGUI, Error, TEXT("OnChildAttached:%s, registered:%d"), *(childUIItem->GetOwner()->GetActorLabel()), childUIItem->IsRegistered());
		MarkLayoutDirty(true);
		//hierarchy index
		CheckCacheUIChildren();//check
#if WITH_EDITORONLY_DATA
		if (!GetWorld()->IsGameWorld())
		{
			if (!ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(this->GetOwner()))//when load from prefab or duplicate from LGUICopier, then not set hierarchy index
			{
				if (childUIItem->IsRegistered())//when load from level, then not set hierarchy index
				{
					childUIItem->hierarchyIndex = UIChildren.Num();
				}
			}
			if (!ULGUIEditorManagerObject::IsPrefabSystemProcessingActor(childUIItem->GetOwner()))
			{
				if (childUIItem->IsRegistered())//when load from level, then not set hierarchy index
				{
					childUIItem->CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
				}
			}
		}
		else
#endif
		{
			if (!ALGUIManagerActor::IsPrefabSystemProcessingActor(this->GetOwner()))//when load from prefab or duplicate from LGUICopier, then not set hierarchy index
			{
				if (childUIItem->IsRegistered())//when load from level, then not set hierarchy index
				{
					childUIItem->hierarchyIndex = UIChildren.Num();
				}
			}
			if (!ALGUIManagerActor::IsPrefabSystemProcessingActor(childUIItem->GetOwner()))//when load from prefab or duplicate from LGUICopier, then not set hierarchy index
			{
				if (childUIItem->IsRegistered())//load from level
				{
					childUIItem->CalculateAnchorFromTransform();//if not from PrefabSystem, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
				}
			}
		}
		UIChildren.Add(childUIItem);
		SortCacheUIChildren();
		//flatten hierarchy index
		MarkFlattenHierarchyIndexDirty();
		//active
		childUIItem->allUpParentUIActive = this->GetIsUIActiveInHierarchy();
		childUIItem->SetUIActiveStateChange();

		CallUILifeCycleBehavioursChildAttachmentChanged(childUIItem, true);
	}
	MarkCanvasUpdate();
}

void UUIItem::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;
	if (auto childUIItem = Cast<UUIItem>(ChildComponent))
	{
		MarkLayoutDirty(true);
		//hierarchy index
		CheckCacheUIChildren();
		UIChildren.Remove(childUIItem);
		for (int i = 0; i < UIChildren.Num(); i++)
		{
			UIChildren[i]->hierarchyIndex = i;
		}
		//flatten hierarchy index
		MarkFlattenHierarchyIndexDirty();
		//active
		childUIItem->allUpParentUIActive = true;
		childUIItem->SetUIActiveStateChange();

		CallUILifeCycleBehavioursChildAttachmentChanged(childUIItem, false);
	}
	MarkCanvasUpdate();
}

void UUIItem::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;

	ULGUICanvas* ParentCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(GetOwner()->GetAttachParentActor(), false);
	UUICanvasGroup* ParentCanvasGroup = LGUIUtils::GetComponentInParent<UUICanvasGroup>(GetOwner()->GetAttachParentActor(), false);
	UIHierarchyChanged(ParentCanvas, ParentCanvasGroup);
	//Because hierarchy changed, then mark position and size as changed too
	MarkLayoutDirty(true);
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
		if (!world->IsGameWorld())
		{
			ULGUIEditorManagerObject::AddUIItem(this);
			//create helper
			if (!IsValid(HelperComp))
			{
				HelperComp = NewObject<UUIItemEditorHelperComp>(GetOwner());
				HelperComp->Parent = this;
				HelperComp->SetupAttachment(this);
			}
			//display name
			if (this->GetOwner()->GetRootComponent() == this)
			{
				auto actorLabel = this->GetOwner()->GetActorLabel();
				if (actorLabel.StartsWith("//"))
				{
					actorLabel = actorLabel.Right(actorLabel.Len() - 2);
				}
				this->displayName = actorLabel;
			}
			else
			{
				this->displayName = this->GetName();
			}
		}
		else
#endif
		{
			ALGUIManagerActor::AddUIItem(this);
		}
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
			ULGUIEditorManagerObject::RemoveUIItem(this);
		}
		else
#endif
		{
			ALGUIManagerActor::RemoveUIItem(this);
		}
	}
#if WITH_EDITOR
	if (IsValid(HelperComp))
	{
		HelperComp->DestroyComponent();
		HelperComp = nullptr;
	}
#endif

	CheckRootUIItem();
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

void UUIItem::SortCacheUIChildren()
{
	UIChildren.Sort([](const UUIItem& A, const UUIItem& B)
	{
		if (A.GetHierarchyIndex() < B.GetHierarchyIndex())
			return true;
		return false;
	});
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
	for (auto uiItem : UIChildren)
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

	for (auto uiItem : UIChildren)
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
	SetRenderCanvas(ParentCanvas);
	for (auto uiItem : UIChildren)
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
	for (auto uiItem : UIChildren)
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

	for (auto uiItem : UIChildren)
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
		auto OldRenderCanvas = RenderCanvas;
		RenderCanvas = ParentRenderCanvas;
		OnRenderCanvasChanged(OldRenderCanvas.Get(), RenderCanvas.Get());
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

	for (auto uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->UIHierarchyChanged(ParentRenderCanvas, ParentCanvasGroup);
		}
	}

	MarkLayoutDirty(true);

	ParentUIItem = nullptr;
	GetParentAsUIItem();

	CheckRootUIItem();
	if (ParentUIItem.IsValid())
	{
		if (this->IsRegistered())//not registerd, could be load from level
		{
			//calculate dimensions
			switch (widget.anchorHAlign)
			{
			case UIAnchorHorizontalAlign::None:
				break;
			case UIAnchorHorizontalAlign::Left:
			case UIAnchorHorizontalAlign::Center:
			case UIAnchorHorizontalAlign::Right:
			{
				CalculateHorizontalStretchFromAnchorAndSize();
			}
			break;
			case UIAnchorHorizontalAlign::Stretch:
			{
				CalculateHorizontalAnchorAndSizeFromStretch();
			}
			break;
			}
			switch (widget.anchorVAlign)
			{
			case UIAnchorVerticalAlign::None:
				break;
			case UIAnchorVerticalAlign::Bottom:
			case UIAnchorVerticalAlign::Middle:
			case UIAnchorVerticalAlign::Top:
			{
				CalculateVerticalStretchFromAnchorAndSize();
			}
			break;
			case UIAnchorVerticalAlign::Stretch:
			{
				CalculateVerticalAnchorAndSizeFromStretch();
			}
			break;
			}
		}
	}
}
void UUIItem::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	//mark UIItem update geometry
	if (this->RootUIItem.IsValid())
	{
		this->RootUIItem->bNeedUpdateRootUIItem = true;
	}
	//mark canvas update drawcall
	if (IsValid(OldCanvas))
	{
		OldCanvas->MarkCanvasUpdate();
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->MarkCanvasUpdate();
	}
}

void UUIItem::CalculateHorizontalStretchFromAnchorAndSize()
{
	float parentWidthMultiply = 0.5f;
	switch (widget.anchorHAlign)
	{
	case UIAnchorHorizontalAlign::Left:
	{
		parentWidthMultiply = 0.0f;
	}
	break;
	case UIAnchorHorizontalAlign::Center:
	case UIAnchorHorizontalAlign::Stretch:
	{
		parentWidthMultiply = 0.5f;
	}
	break;
	case UIAnchorHorizontalAlign::Right:
	{
		parentWidthMultiply = 1.0f;
	}
	break;
	}

	widget.stretchLeft = widget.anchorOffsetX + ParentUIItem->widget.width * parentWidthMultiply - widget.width * widget.pivot.X;
	widget.stretchRight = ParentUIItem->widget.width - widget.width - widget.stretchLeft;
}
void UUIItem::CalculateVerticalStretchFromAnchorAndSize()
{
	float parentHeightMultiply = 0.5f;
	switch (widget.anchorVAlign)
	{
	case UIAnchorVerticalAlign::Bottom:
	{
		parentHeightMultiply = 0.0f;
	}
	break;
	case UIAnchorVerticalAlign::Middle:
	case UIAnchorVerticalAlign::Stretch:
	{
		parentHeightMultiply = 0.5f;
	}
	break;
	case UIAnchorVerticalAlign::Top:
	{
		parentHeightMultiply = 1.0f;
	}
	break;
	}

	widget.stretchBottom = widget.anchorOffsetY + ParentUIItem->widget.height * parentHeightMultiply - widget.height * widget.pivot.Y;
	widget.stretchTop = ParentUIItem->widget.height - widget.height - widget.stretchBottom;
}
bool UUIItem::CalculateHorizontalAnchorAndSizeFromStretch()
{
	bool sizeChanged = false;
	const auto& parentWidget = ParentUIItem->widget;
	float width = parentWidget.width - widget.stretchLeft - widget.stretchRight;
	if (FMath::Abs(widget.width - width) > KINDA_SMALL_NUMBER)
	{
		widget.width = width;
		WidthChanged();
		sizeChanged = true;
	}

	float parentWidthMultiply = 0.5f;
	switch (widget.anchorHAlign)
	{
	case UIAnchorHorizontalAlign::Left:
	{
		parentWidthMultiply = 0;
	}
	break;
	case UIAnchorHorizontalAlign::Stretch:
	case UIAnchorHorizontalAlign::Center:
	{
		parentWidthMultiply = 0.5f;
	}
	break;
	case UIAnchorHorizontalAlign::Right:
	{
		parentWidthMultiply = 1.0f;
	}
	break;
	}
	widget.anchorOffsetX = widget.stretchLeft - (parentWidget.width * parentWidthMultiply) + (widget.width * widget.pivot.X);
	return sizeChanged;
}
bool UUIItem::CalculateVerticalAnchorAndSizeFromStretch()
{
	bool sizeChanged = false;
	const auto& parentWidget = ParentUIItem->widget;
	float height = parentWidget.height - widget.stretchTop - widget.stretchBottom;
	if (FMath::Abs(widget.height - height) > KINDA_SMALL_NUMBER)
	{
		widget.height = height;
		HeightChanged();
		sizeChanged = true;
	}

	float parentHeightMultiply = 0.5f;
	switch (widget.anchorVAlign)
	{
	case UIAnchorVerticalAlign::Bottom:
	{
		parentHeightMultiply = 0;
	}
	break;
	case UIAnchorVerticalAlign::Stretch:
	case UIAnchorVerticalAlign::Middle:
	{
		parentHeightMultiply = 0.5f;
	}
	break;
	case UIAnchorVerticalAlign::Top:
	{
		parentHeightMultiply = 1.0f;
	}
	break;
	}
	widget.anchorOffsetY = widget.stretchBottom - (parentWidget.height * parentHeightMultiply) + (widget.height * widget.pivot.Y);
	return sizeChanged;
}

void UUIItem::CheckRootUIItem()
{
	auto oldRootUIItem = RootUIItem;
	if (oldRootUIItem == this && oldRootUIItem != nullptr)
	{
#if WITH_EDITOR
		if (!this->GetWorld()->IsGameWorld())
		{
			ULGUIEditorManagerObject::RemoveRootUIItem(this);
		}
		else
#endif
		{
			ALGUIManagerActor::RemoveRootUIItem(this);
		}
	}

	UUIItem* topUIItem = this;
	UUIItem* tempRootUIItem = nullptr;
	while (topUIItem != nullptr && topUIItem->IsRegistered())
	{
		tempRootUIItem = topUIItem;
		topUIItem = Cast<UUIItem>(topUIItem->GetAttachParent());
	}
	RootUIItem = tempRootUIItem;

	if (RootUIItem == this && RootUIItem != nullptr)
	{
#if WITH_EDITOR
		if (!this->GetWorld()->IsGameWorld())
		{
			ULGUIEditorManagerObject::AddRootUIItem(this);
		}
		else
#endif
		{
			ALGUIManagerActor::AddRootUIItem(this);
		}
	}
}

void UUIItem::MarkUpdateLayout()
{
	if (this->RootUIItem.IsValid())
	{
		this->RootUIItem->bShouldUpdateRootUIItemLayout = true;
		this->RootUIItem->bNeedUpdateRootUIItem = true;
	}
}

void UUIItem::UpdateChildUIItemRecursive(UUIItem* target, bool parentLayoutChanged)
{
	const auto& childrenList = target->GetAttachUIChildren();
	for (auto uiChild : childrenList)
	{
		if (IsValid(uiChild))
		{
			//update layout
			auto layoutChanged = parentLayoutChanged;
			uiChild->UpdateLayout(layoutChanged, cacheForThisUpdate_ShouldUpdateLayout);
			uiChild->UpdateGeometry();

			UpdateChildUIItemRecursive(uiChild, layoutChanged);
		}
	}
}
void UUIItem::UpdateRootUIItem()
{
	if (bNeedUpdateRootUIItem)
	{
		bNeedUpdateRootUIItem = false;

		cacheForThisUpdate_ShouldUpdateLayout = bShouldUpdateRootUIItemLayout;
		bShouldUpdateRootUIItemLayout = false;

		//update layout
		bool parentLayoutChanged = false;
		this->UpdateLayout(parentLayoutChanged, cacheForThisUpdate_ShouldUpdateLayout);
		this->UpdateGeometry();

		UpdateChildUIItemRecursive(this, this->cacheForThisUpdate_LayoutChanged);
	}
}

void UUIItem::UpdateLayout(bool& parentLayoutChanged, bool shouldUpdateLayout)
{
	//update data change mark
	UpdateCachedData();

	//update layout
	if (parentLayoutChanged == false)
	{
		if (cacheForThisUpdate_LayoutChanged)
			parentLayoutChanged = true;
	}
	else
	{
		cacheForThisUpdate_LayoutChanged = true;
	}
	//if parent layout change or self layout change, then update layout
	if (cacheForThisUpdate_LayoutChanged && shouldUpdateLayout)
	{
		bCanSetAnchorFromTransform = false;
		CalculateTransformFromAnchor();
		CallUILifeCycleBehavioursDimensionsChanged(true, cacheForThisUpdate_SizeChanged);
		bCanSetAnchorFromTransform = true;
	}

	//Update canvas layout
	if (this->IsCanvasUIItem() && this->RenderCanvas.IsValid())
	{
		this->RenderCanvas->UpdateCanvasLayout(cacheForThisUpdate_ShouldUpdateLayout);
	}

	//data may change after layout calculation, so check it again
	UpdateCachedDataBeforeGeometry();
}

void UUIItem::UpdateGeometry()
{
	
}

FDelegateHandle UUIItem::RegisterUIHierarchyChanged(const FSimpleDelegate& InCallback)
{
	return UIHierarchyChangedDelegate.Add(InCallback);
}
void UUIItem::UnregisterUIHierarchyChanged(const FDelegateHandle& InHandle)
{
	UIHierarchyChangedDelegate.Remove(InHandle);
}

void UUIItem::CalculateTransformFromAnchor()
{
	if (!ParentUIItem.IsValid())return;
	const auto& parentWidget = ParentUIItem->widget;
	FVector resultLocation = this->GetRelativeLocation();
	switch (widget.anchorHAlign)
	{
	case UIAnchorHorizontalAlign::Left:
	{
		resultLocation.Y = parentWidget.width * (-parentWidget.pivot.X);
		resultLocation.Y += widget.anchorOffsetX;
	}
	break;
	case UIAnchorHorizontalAlign::Center:
	{
		resultLocation.Y = parentWidget.width * (0.5f - parentWidget.pivot.X);
		resultLocation.Y += widget.anchorOffsetX;
	}
	break;
	case UIAnchorHorizontalAlign::Right:
	{
		resultLocation.Y = parentWidget.width * (1 - parentWidget.pivot.X);
		resultLocation.Y += widget.anchorOffsetX;
	}
	break;
	case UIAnchorHorizontalAlign::Stretch:
	{
		float width = parentWidget.width - widget.stretchLeft - widget.stretchRight;
		if (FMath::Abs(widget.width - width) > KINDA_SMALL_NUMBER)
		{
			widget.width = width;
			WidthChanged();
			MarkLayoutDirty(true);
		}

		resultLocation.Y = -parentWidget.pivot.X * parentWidget.width;
		resultLocation.Y += widget.stretchLeft;
		resultLocation.Y += widget.pivot.X * widget.width;
	}
	break;
	}
	switch (widget.anchorVAlign)
	{
	case UIAnchorVerticalAlign::Top:
	{
		resultLocation.Z = parentWidget.height * (1 - parentWidget.pivot.Y);
		resultLocation.Z += widget.anchorOffsetY;
	}
	break;
	case UIAnchorVerticalAlign::Middle:
	{
		resultLocation.Z = parentWidget.height * (0.5f - parentWidget.pivot.Y);
		resultLocation.Z += widget.anchorOffsetY;
	}
	break;
	case UIAnchorVerticalAlign::Bottom:
	{
		resultLocation.Z = parentWidget.height * (-parentWidget.pivot.Y);
		resultLocation.Z += widget.anchorOffsetY;
	}
	break;
	case UIAnchorVerticalAlign::Stretch:
	{
		float height = parentWidget.height - widget.stretchTop - widget.stretchBottom;
		if (FMath::Abs(widget.height - height) > KINDA_SMALL_NUMBER)
		{
			widget.height = height;
			HeightChanged();
			MarkLayoutDirty(true);
		}

		resultLocation.Z = -parentWidget.pivot.Y * parentWidget.height;
		resultLocation.Z += widget.stretchBottom;
		resultLocation.Z += widget.pivot.Y * widget.height;
	}
	break;
	}
	if (!this->GetRelativeLocation().Equals(resultLocation))
	{
		GetRelativeLocation_DirectMutable() = resultLocation;
		UpdateComponentToWorld();
	}
#if WITH_EDITORONLY_DATA
	prevAnchorHAlign = widget.anchorHAlign;
	prevAnchorVAlign = widget.anchorVAlign;
#endif 
}
void UUIItem::UpdateCachedData()
{
	this->cacheForThisUpdate_LayoutChanged = bLayoutChanged;
	this->cacheForThisUpdate_SizeChanged = bSizeChanged;
	this->cacheForThisUpdate_FlattenHierarchyIndexChange = bFlattenHierarchyIndexChanged;
	bLayoutChanged = false;
	bSizeChanged = false;
	bFlattenHierarchyIndexChanged = false;
}
void UUIItem::UpdateCachedDataBeforeGeometry()
{
	if (bLayoutChanged)cacheForThisUpdate_LayoutChanged = true;
	if (bSizeChanged)cacheForThisUpdate_SizeChanged = true;
	if (bFlattenHierarchyIndexChanged)cacheForThisUpdate_FlattenHierarchyIndexChange = true;
}

void UUIItem::SetWidget(const FUIWidget& inWidget)
{
	SetPivot(inWidget.pivot);
	SetAnchorHAlign(inWidget.anchorHAlign);
	SetAnchorVAlign(inWidget.anchorVAlign);
	if (inWidget.anchorHAlign == UIAnchorHorizontalAlign::Stretch)
	{
		SetAnchorOffsetY(inWidget.anchorOffsetX);
		SetWidth(inWidget.width);
		SetStretchLeft(inWidget.stretchLeft);
		SetStretchRight(inWidget.stretchRight);
	}
	else
	{
		SetStretchLeft(inWidget.stretchLeft);
		SetStretchRight(inWidget.stretchRight);
		SetAnchorOffsetY(inWidget.anchorOffsetX);
		SetWidth(inWidget.width);
	}
	if (inWidget.anchorVAlign == UIAnchorVerticalAlign::Stretch)
	{
		SetAnchorOffsetZ(inWidget.anchorOffsetY);
		SetHeight(inWidget.height);
		SetStretchTop(inWidget.stretchTop);
		SetStretchBottom(inWidget.stretchBottom);
	}
	else
	{
		SetStretchTop(inWidget.stretchTop);
		SetStretchBottom(inWidget.stretchBottom);
		SetAnchorOffsetZ(inWidget.anchorOffsetY);
		SetHeight(inWidget.height);
	}
}

void UUIItem::SetWidth(float newWidth)
{
	if (FMath::Abs(widget.width - newWidth) > KINDA_SMALL_NUMBER)
	{
		widget.width = newWidth;
		WidthChanged();
		if (ParentUIItem.IsValid())
		{
			CalculateHorizontalStretchFromAnchorAndSize();
		}
		CalculateTransformFromAnchor();
		MarkLayoutDirty(true);
	}
}
void UUIItem::SetHeight(float newHeight)
{
	if (FMath::Abs(widget.height - newHeight) > KINDA_SMALL_NUMBER)
	{
		widget.height = newHeight;
		HeightChanged();
		if (ParentUIItem.IsValid())
		{
			CalculateVerticalStretchFromAnchorAndSize();
		}
		CalculateTransformFromAnchor();
		MarkLayoutDirty(true);
	}
}
void UUIItem::SetPivot(FVector2D pivot) 
{
	if (!widget.pivot.Equals(pivot))
	{
		//@todo: keep relative to parent
		widget.pivot = pivot;
		PivotChanged();
		MarkLayoutDirty(false);
	}
}

void UUIItem::WidthChanged()
{

}
void UUIItem::HeightChanged()
{

}
void UUIItem::PivotChanged()
{

}

void UUIItem::SetAnchorOffsetY(float newOffset) 
{
	if (FMath::Abs(widget.anchorOffsetX - newOffset) > KINDA_SMALL_NUMBER)
	{
		widget.anchorOffsetX = newOffset;
		if (ParentUIItem.IsValid())
		{
			CalculateHorizontalStretchFromAnchorAndSize();
		}
		CalculateTransformFromAnchor();
		MarkLayoutDirty(false);
	}
}
void UUIItem::SetAnchorOffsetZ(float newOffset) 
{
	if (FMath::Abs(widget.anchorOffsetY - newOffset) > KINDA_SMALL_NUMBER)
	{
		widget.anchorOffsetY = newOffset;
		if (ParentUIItem.IsValid())
		{
			CalculateVerticalStretchFromAnchorAndSize();
		}
		CalculateTransformFromAnchor();
		MarkLayoutDirty(false);
	}
}
void UUIItem::SetAnchorOffset(FVector2D newOffset)
{
	bool anyChange = false;
	if (FMath::Abs(widget.anchorOffsetX - newOffset.X) > KINDA_SMALL_NUMBER)
	{
		anyChange = true;
		widget.anchorOffsetX = newOffset.X;
		if (ParentUIItem.IsValid())
		{
			CalculateHorizontalStretchFromAnchorAndSize();
		}
	}
	if (FMath::Abs(widget.anchorOffsetY - newOffset.Y) > KINDA_SMALL_NUMBER)
	{
		anyChange = true;
		widget.anchorOffsetY = newOffset.Y;
		if (ParentUIItem.IsValid())
		{
			CalculateVerticalStretchFromAnchorAndSize();
		}
	}
	if (anyChange)
	{
		CalculateTransformFromAnchor();
		MarkLayoutDirty(false);
	}
}
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
void UUIItem::SetStretchLeft(float newLeft)
{
	if (FMath::Abs(widget.stretchLeft - newLeft) > KINDA_SMALL_NUMBER)
	{
		widget.stretchLeft = newLeft;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
		CalculateTransformFromAnchor();
	}
}
void UUIItem::SetStretchRight(float newRight)
{
	if (FMath::Abs(widget.stretchRight - newRight) > KINDA_SMALL_NUMBER)
	{
		widget.stretchRight = newRight;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
		CalculateTransformFromAnchor();
	}
}
void UUIItem::SetHorizontalStretch(FVector2D newStretch)
{
	if (FMath::Abs(widget.stretchLeft - newStretch.X) > KINDA_SMALL_NUMBER || FMath::Abs(widget.stretchRight - newStretch.Y) > KINDA_SMALL_NUMBER)
	{
		widget.stretchLeft = newStretch.X;
		widget.stretchRight = newStretch.Y;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
		CalculateTransformFromAnchor();
	}
}
void UUIItem::SetStretchTop(float newTop)
{
	if (FMath::Abs(widget.stretchTop - newTop) > KINDA_SMALL_NUMBER)
	{
		widget.stretchTop = newTop;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
		CalculateTransformFromAnchor();
	}
}
void UUIItem::SetStretchBottom(float newBottom)
{
	if (FMath::Abs(widget.stretchBottom - newBottom) > KINDA_SMALL_NUMBER)
	{
		widget.stretchBottom = newBottom;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
		CalculateTransformFromAnchor();
	}
}
void UUIItem::SetVerticalStretch(FVector2D newStretch)
{
	if (FMath::Abs(widget.stretchBottom - newStretch.X) > KINDA_SMALL_NUMBER || FMath::Abs(widget.stretchTop - newStretch.Y) > KINDA_SMALL_NUMBER)
	{
		widget.stretchBottom = newStretch.X;
		widget.stretchTop = newStretch.Y;
		if (ParentUIItem.IsValid())
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			MarkLayoutDirty(sizeChanged);
		}
		else
		{
			MarkLayoutDirty(false);
		}
		CalculateTransformFromAnchor();
	}
}

void UUIItem::SetAnchorHAlign(UIAnchorHorizontalAlign align)
{
	if (widget.anchorHAlign != align)
	{
		if (ParentUIItem.IsValid())
		{
			//first, convert from other anchor to left
			switch (widget.anchorHAlign)
			{
			case UIAnchorHorizontalAlign::Center:
			{
				float halfWidth = ParentUIItem->widget.width * 0.5f;
				widget.anchorOffsetX += halfWidth;
			}
			break;
			case UIAnchorHorizontalAlign::Right:
			{
				widget.anchorOffsetX += ParentUIItem->widget.width;
			}
			break;
			case UIAnchorHorizontalAlign::Stretch:
			{
				widget.anchorOffsetX = widget.stretchLeft + widget.width * widget.pivot.X;
			}
			break;
			}
			//second, convert from left anchor to other
			switch (align)
			{
			case UIAnchorHorizontalAlign::Center:
			{
				float halfWidth = ParentUIItem->widget.width * 0.5f;
				widget.anchorOffsetX -= halfWidth;
			}
			break;
			case UIAnchorHorizontalAlign::Right:
			{
				widget.anchorOffsetX -= ParentUIItem->widget.width;
			}
			break;
			case UIAnchorHorizontalAlign::Stretch:
			{
				widget.stretchLeft = widget.anchorOffsetX - widget.width * widget.pivot.X;
				widget.stretchRight = ParentUIItem->widget.width - widget.width - widget.stretchLeft;
			}
			break;
			}

			widget.anchorHAlign = align;
			CalculateTransformFromAnchor();
			MarkLayoutDirty(false);
		}
	}
#if WITH_EDITORONLY_DATA
	prevAnchorHAlign = align;
#endif
}
void UUIItem::SetAnchorVAlign(UIAnchorVerticalAlign align)
{
	if (widget.anchorVAlign != align)
	{
		if (ParentUIItem.IsValid())
		{
			//first, convert from other anchor to bottom
			switch (widget.anchorVAlign)
			{
			case UIAnchorVerticalAlign::Middle:
			{
				float halfHeight = ParentUIItem->widget.height * 0.5f;
				widget.anchorOffsetY += halfHeight;
			}
			break;
			case UIAnchorVerticalAlign::Top:
			{
				widget.anchorOffsetY += ParentUIItem->widget.height;
			}
			break;
			case UIAnchorVerticalAlign::Stretch:
			{
				widget.anchorOffsetY = widget.stretchBottom + widget.height * widget.pivot.Y;
			}
			break;
			}
			//second, convert from bottom to other
			switch (align)
			{
			case UIAnchorVerticalAlign::Middle:
			{
				float halfHeight = ParentUIItem->widget.height * 0.5f;
				widget.anchorOffsetY -= halfHeight;
			}
			break;
			case UIAnchorVerticalAlign::Top:
			{
				widget.anchorOffsetY -= ParentUIItem->widget.height;
			}
			break;
			case UIAnchorVerticalAlign::Stretch:
			{
				widget.stretchBottom = widget.anchorOffsetY - widget.height * widget.pivot.Y;
				widget.stretchTop = ParentUIItem->widget.height - widget.height - widget.stretchBottom;
			}
			break;
			}

			widget.anchorVAlign = align;
			CalculateTransformFromAnchor();
			MarkLayoutDirty(false);
		}
	}
#if WITH_EDITORONLY_DATA
	prevAnchorVAlign = align;
#endif
}

FVector2D UUIItem::GetLocalSpaceLeftBottomPoint()const
{
	FVector2D leftBottomPoint;
	leftBottomPoint.X = widget.width * -widget.pivot.X;
	leftBottomPoint.Y = widget.height * -widget.pivot.Y;
	return leftBottomPoint;
}
FVector2D UUIItem::GetLocalSpaceRightTopPoint()const
{
	FVector2D rightTopPoint;
	rightTopPoint.X = widget.width * (1.0f - widget.pivot.X);
	rightTopPoint.Y = widget.height * (1.0f - widget.pivot.Y);
	return rightTopPoint;
}
FVector2D UUIItem::GetLocalSpaceCenter()const
{
	return FVector2D(widget.width * (0.5f - widget.pivot.X), widget.height * (0.5f - widget.pivot.Y));
}

void UUIItem::GetLocalSpaceMinMaxPoint(FVector2D& min, FVector2D& max)const
{
	min = GetLocalSpaceLeftBottomPoint();
	max = GetLocalSpaceRightTopPoint();
}

float UUIItem::GetLocalSpaceLeft()const
{
	return widget.width * -widget.pivot.X;
}
float UUIItem::GetLocalSpaceRight()const
{
	return widget.width * (1.0f - widget.pivot.X);
}
float UUIItem::GetLocalSpaceBottom()const
{
	return widget.height * -widget.pivot.Y;
}
float UUIItem::GetLocalSpaceTop()const
{
	return widget.height * (1.0f - widget.pivot.Y);
}
UUIItem* UUIItem::GetParentAsUIItem()const
{
	if (!ParentUIItem.IsValid())
	{
		if (auto parent = GetAttachParent())
		{
			ParentUIItem = Cast<UUIItem>(parent);
		}
	}
	return ParentUIItem.Get();
}

void UUIItem::MarkLayoutDirty(bool sizeChange)
{
	bLayoutChanged = true;
	if (sizeChange)
	{
		bSizeChanged = true;
	}
	MarkCanvasUpdate();
	MarkUpdateLayout();
}

void UUIItem::MarkCanvasUpdate()
{
	if (RenderCanvas.IsValid())
	{
		RenderCanvas->MarkCanvasUpdate();
	}
	if (this->RootUIItem.IsValid())
	{
		this->RootUIItem->bNeedUpdateRootUIItem = true;
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
	if (!bRaycastTarget)return false;
	if (!GetIsUIActiveInHierarchy())return false;
	if (!RenderCanvas.IsValid())return false;
	if (!GetOwner())return false;
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
	bIsGroupAllowInteraction = CanvasGroup.IsValid() ? CanvasGroup->GetFinalInteractable() : true;
	CallUILifeCycleBehavioursInteractionStateChanged();
}
#pragma endregion UICanvasGroup

#pragma region UIActive

void UUIItem::OnChildActiveStateChanged(UUIItem* child)
{
	CallUILifeCycleBehavioursChildActiveInHierarchyStateChanged(child, child->GetIsUIActiveInHierarchy());
}

void UUIItem::SetUIActiveStateChange()
{
	auto thisUIActiveState = this->GetIsUIActiveInHierarchy();
	SetChildrenUIActiveChangeRecursive(thisUIActiveState);
}

void UUIItem::SetChildrenUIActiveChangeRecursive(bool InUpParentUIActive)
{
	for (auto uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{		//state is changed
			if (uiChild->bIsUIActive &&//when child is active, then parent's active state can affect child
				(uiChild->allUpParentUIActive != InUpParentUIActive)//state change
				)
			{
				uiChild->allUpParentUIActive = InUpParentUIActive;
				//apply for state change
				uiChild->ApplyUIActiveState();
				//affect children
				uiChild->SetChildrenUIActiveChangeRecursive(uiChild->GetIsUIActiveInHierarchy());
				//callback for parent
				this->OnChildActiveStateChanged(uiChild);
			}
			//state not changed
			else
			{
				uiChild->allUpParentUIActive = InUpParentUIActive;
				//affect children
				uiChild->SetChildrenUIActiveChangeRecursive(uiChild->GetIsUIActiveInHierarchy());
			}
		}
	}
}
void UUIItem::SetIsUIActive(bool active)
{
	if (bIsUIActive != active)
	{
		bIsUIActive = active;
		if (allUpParentUIActive)//state change only happens when up parent is active
		{
			ApplyUIActiveState();
			//affect children
			SetChildrenUIActiveChangeRecursive(bIsUIActive);
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

void UUIItem::ApplyUIActiveState()
{
#if WITH_EDITOR
	//modify inactive actor's name
	if (auto ownerActor = GetOwner())
	{
		auto actorLabel = ownerActor->GetActorLabel();
		FString prefix("//");
		if (GetIsUIActiveInHierarchy() && actorLabel.StartsWith(prefix))
		{
			actorLabel = actorLabel.Right(actorLabel.Len() - prefix.Len());
			ownerActor->SetActorLabel(actorLabel);
			ownerActor->SetIsTemporarilyHiddenInEditor(false);
		}
		else if (!GetIsUIActiveInHierarchy() && !actorLabel.StartsWith(prefix))
		{
			actorLabel = prefix.Append(actorLabel);
			ownerActor->SetActorLabel(actorLabel);
			ownerActor->SetIsTemporarilyHiddenInEditor(true);
		}
	}
#endif
	bLayoutChanged = true;
	//canvas update
	MarkCanvasUpdate();
	//layout update
	MarkUpdateLayout();
	//callback
	CallUILifeCycleBehavioursActiveInHierarchyStateChanged();
}

#pragma endregion UIActive




UUIItemEditorHelperComp::UUIItemEditorHelperComp()
{
	bSelectable = false;
	this->bIsEditorOnly = true;
}
#if WITH_EDITORONLY_DATA
FIntRect UUIItemEditorHelperComp::viewRect;
FViewMatrices UUIItemEditorHelperComp::viewMatrices;
#endif
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
			if (ULGUIEditorManagerObject::Instance != nullptr)
			{
				if (View->GetViewKey() == ULGUIEditorManagerObject::Instance->CurrentActiveViewportKey)
				{
					viewRect = View->UnconstrainedViewRect;
					viewMatrices = View->ViewMatrices;
				}
			}

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

	auto widget = Parent->GetWidget();
	auto origin = FVector(widget.width * (0.5f - widget.pivot.X), widget.height * (0.5f - widget.pivot.Y), 0);

	BoxElem->X = widget.width;
	BoxElem->Y = widget.height;
	BoxElem->Z = 0.0f;

	BoxElem->Center = origin;
}
FBoxSphereBounds UUIItemEditorHelperComp::CalcBounds(const FTransform& LocalToWorld) const
{
	if (!IsValid(Parent))return FBoxSphereBounds(EForceInit::ForceInit);
	auto widget = Parent->GetWidget();
	auto origin = FVector(widget.width * (0.5f - widget.pivot.X), widget.height * (0.5f - widget.pivot.Y), 0);
	return FBoxSphereBounds(origin, FVector(widget.width * 0.5f, widget.height * 0.5f, 1), (widget.width > widget.height ? widget.width : widget.height) * 0.5f).TransformBy(LocalToWorld);
}

//PRAGMA_ENABLE_OPTIMIZATION
