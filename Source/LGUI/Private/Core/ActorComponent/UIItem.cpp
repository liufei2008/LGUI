// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIItem.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/ActorComponent/UIInteractionGroup.h"
#include "Core/LGUISettings.h"
#include "Core/LGUIBehaviour.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PhysicsEngine/BodySetup.h"
#include "Layout/LGUICanvasScaler.h"
#if WITH_EDITOR
#include "DrawDebugHelpers.h"
#include "EditorViewportClient.h"
#endif


UUIItem::UUIItem(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	SetMobility(EComponentMobility::Movable);
	SetUsingAbsoluteLocation(false);
	SetUsingAbsoluteRotation(false);
	SetUsingAbsoluteScale(false);
	SetVisibility(false);
	bWantsOnUpdateTransform = true;
	bCanSetAnchorFromTransform = false;//skip construction
	itemType = UIItemType::UIItem;

	bColorChanged = true;
	bLayoutChanged = true;
	bSizeChanged = true;

	isCanvasUIItem = false;

	traceChannel = GetDefault<ULGUISettings>()->defaultTraceChannel;
}

void UUIItem::BeginPlay()
{
	Super::BeginPlay();

	ParentUIItem = nullptr;
	GetParentAsUIItem();
	CheckRenderCanvas();

	bColorChanged = true;
	bLayoutChanged = true;
	bSizeChanged = true;
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

#pragma region LGUIBehaviour
void UUIItem::CallUIComponentsActiveInHierarchyStateChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIActiveInHierachy(IsUIActiveInHierarchy());
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIActiveInHierachy(IsUIActiveInHierarchy());
	}
}
void UUIItem::CallUIComponentsChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
	}
}
void UUIItem::CallUIComponentsChildActiveInHierarchyStateChanged(UUIItem* child, bool activeOrInactive)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildAcitveInHierarchy(child, activeOrInactive);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIChildAcitveInHierarchy(child, activeOrInactive);
	}
}
void UUIItem::CallUIComponentsDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIDimensionsChanged(positionChanged, sizeChanged);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIDimensionsChanged(positionChanged, sizeChanged);
	}

	//call parent
	if (ParentUIItem.IsValid())
	{
		ParentUIItem->CallUIComponentsChildDimensionsChanged(this, positionChanged, sizeChanged);
	}
}
void UUIItem::CallUIComponentsAttachmentChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIAttachmentChanged();
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIAttachmentChanged();
	}
}
void UUIItem::CallUIComponentsChildAttachmentChanged(UUIItem* child, bool attachOrDettach)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildAttachmentChanged(child, attachOrDettach);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIChildAttachmentChanged(child, attachOrDettach);
	}
}
void UUIItem::CallUIComponentsInteractionStateChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	auto interactable = this->IsGroupAllowInteraction();
	OnUIInteractionStateChanged(interactable);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIInteractionStateChanged(interactable);
	}
}
void UUIItem::CallUIComponentsChildHierarchyIndexChanged(UUIItem* child)
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIChildHierarchyIndexChanged(child);
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(LGUIBehaviourArray, false);
	}
#endif
	for (int i = 0; i < LGUIBehaviourArray.Num(); i++)
	{
		auto& CompItem = LGUIBehaviourArray[i];
		CompItem->OnUIChildHierarchyIndexChanged(child);
	}
}
#pragma endregion LGUIBehaviour


void UUIItem::OnChildHierarchyIndexChanged(UUIItem* child)
{
	CallUIComponentsChildHierarchyIndexChanged(child);
}

void UUIItem::CalculateFlattenHierarchyIndex_Recursive(int& parentFlattenHierarchyIndex)
{
	if (this->flattenHierarchyIndex != parentFlattenHierarchyIndex)
	{
		this->flattenHierarchyIndex = parentFlattenHierarchyIndex;
		if (this->isCanvasUIItem)
		{
			RenderCanvas->OnUIHierarchyIndexChanged();
		}
	}
	for (auto child : UIChildren)
	{
		if (IsValid(child))
		{
			parentFlattenHierarchyIndex++;
			child->CalculateFlattenHierarchyIndex_Recursive(parentFlattenHierarchyIndex);
		}
	}
}

DECLARE_CYCLE_STAT(TEXT("UIItem CalculateFlattenHierarchyIndex"), STAT_UIItemCalculateFlattenHierarchyIndex, STATGROUP_LGUI);
void UUIItem::RecalculateFlattenHierarchyIndex()
{
	SCOPE_CYCLE_COUNTER(STAT_UIItemCalculateFlattenHierarchyIndex);
	UUIItem* topUIItem = this;
	UUIItem* rootUIItem = nullptr;
	while (topUIItem != nullptr)
	{
		rootUIItem = topUIItem;
		topUIItem = Cast<UUIItem>(topUIItem->GetAttachParent());
	}
	int tempIndex = rootUIItem->flattenHierarchyIndex;
	rootUIItem->CalculateFlattenHierarchyIndex_Recursive(tempIndex);
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
			RecalculateFlattenHierarchyIndex();

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
	bColorChanged = true;
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
void UUIItem::PreEditChange(UProperty* PropertyAboutToChange)
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
			SetUIActive(!bIsUIActive);
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
				SetAnchorOffsetX(originValue);
			}
			else if (propetyName == GET_MEMBER_NAME_CHECKED(FUIWidget, anchorOffsetY))
			{
				auto originValue = widget.anchorOffsetY;
				widget.anchorOffsetY += 1;
				SetAnchorOffsetY(originValue);
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
	RenderCanvas = nullptr;//force check
	if (CheckRenderCanvas())
	{
		if (RenderCanvas->GetRootCanvas())
		{
			RenderCanvas->GetRootCanvas()->MarkCanvasUpdate();
			RenderCanvas->GetRootCanvas()->MarkCanvasUpdateLayout();
		}
		else
		{
			RenderCanvas->MarkCanvasUpdate();
			RenderCanvas->MarkCanvasUpdateLayout();
		}
	}
}
#endif
void UUIItem::OnAttachmentChanged()
{
	Super::OnAttachmentChanged();
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;
	UIHierarchyChanged();
	//callback. because hierarchy changed, then mark position and size as changed too
	MarkLayoutDirty(true);
	//callback
	CallUIComponentsAttachmentChanged();
}
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
				float anchorOffsetX = this->GetRelativeLocation().X + parentWidget.width * parentWidget.pivot.X;
				ApplyAnchorOffsetX(anchorOffsetX);
			}
			break;
			case UIAnchorHorizontalAlign::Center:
			{
				float anchorOffsetX = this->GetRelativeLocation().X + parentWidget.width * (parentWidget.pivot.X - 0.5f);
				ApplyAnchorOffsetX(anchorOffsetX);
			}
			break;
			case UIAnchorHorizontalAlign::Right:
			{
				float anchorOffsetX = this->GetRelativeLocation().X + parentWidget.width * (parentWidget.pivot.X - 1.0f);
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
				selfLeft = this->GetRelativeLocation().X + widget.width * -widget.pivot.X;
				selfRight = this->GetRelativeLocation().X + widget.width * (1.0f - widget.pivot.X);
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
				float anchorOffsetY = this->GetRelativeLocation().Y + parentWidget.height * (parentWidget.pivot.Y - 1.0f);
				ApplyAnchorOffsetY(anchorOffsetY);
			}
			break;
			case UIAnchorVerticalAlign::Middle:
			{
				float anchorOffsetY = this->GetRelativeLocation().Y + parentWidget.height * (parentWidget.pivot.Y - 0.5f);
				ApplyAnchorOffsetY(anchorOffsetY);
			}
			break;
			case UIAnchorVerticalAlign::Bottom:
			{
				float anchorOffsetY = this->GetRelativeLocation().Y + parentWidget.height * parentWidget.pivot.Y;
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
				selfBottom = this->GetRelativeLocation().Y + widget.height * -widget.pivot.Y;
				selfTop = this->GetRelativeLocation().Y + widget.height * (1.0f - widget.pivot.Y);
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
					childUIItem->CalculateAnchorFromTransform();//if not from PrefabSyste, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
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
					childUIItem->CalculateAnchorFromTransform();//if not from PrefabSyste, then calculate anchors on transform, so when use AttachComponent, the KeepRelative or KeepWorld will work. If from PrefabSystem, then anchor will automatically do the job
				}
			}
		}
		UIChildren.Add(childUIItem);
		SortCacheUIChildren();
		//flatten hierarchy index
		RecalculateFlattenHierarchyIndex();
		//interaction group
		childUIItem->allUpParentGroupAllowInteraction = this->IsGroupAllowInteraction();
		childUIItem->SetInteractionGroupStateChange();
		//active
		childUIItem->allUpParentUIActive = this->IsUIActiveInHierarchy();
		bool parentUIActive = childUIItem->IsUIActiveInHierarchy();
		childUIItem->SetChildUIActiveRecursive(parentUIActive);

		CallUIComponentsChildAttachmentChanged(childUIItem, true);
	}
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
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
		RecalculateFlattenHierarchyIndex();

		CallUIComponentsChildAttachmentChanged(childUIItem, false);
	}
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
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
			auto actorLabel = this->GetOwner()->GetActorLabel();
			if (actorLabel.StartsWith("//"))
			{
				actorLabel = actorLabel.Right(actorLabel.Len() - 2);
			}
			this->displayName = actorLabel;
		}
		else
#endif
		{
			ALGUIManagerActor::AddUIItem(this);
		}
	}

#if WITH_EDITOR
	//apply inactive actor's visibility state in editor scene outliner
	if (auto ownerActor = GetOwner())
	{
		if (!IsUIActiveInHierarchy())
		{
			ownerActor->SetIsTemporarilyHiddenInEditor(true);
		}
	}
#endif
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

void UUIItem::UIHierarchyChanged()
{
	auto oldRenderCanvas = RenderCanvas;
	RenderCanvas = nullptr;//force find new
	CheckRenderCanvas();
	if (isCanvasUIItem)RenderCanvas->OnUIHierarchyChanged();
	if (oldRenderCanvas != RenderCanvas)//if attach to new Canvas, need to remove from old and add to new
	{
		OnRenderCanvasChanged(oldRenderCanvas.Get(), RenderCanvas.Get());
	}

	for (auto uiItem : UIChildren)
	{
		if (IsValid(uiItem))
		{
			uiItem->UIHierarchyChanged();
		}
	}

	MarkLayoutDirty(true);

	ParentUIItem = nullptr;
	GetParentAsUIItem();
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
	if (IsValid(OldCanvas))
	{
		OldCanvas->MarkCanvasUpdate();
	}
	if (IsValid(NewCanvas))
	{
		isCanvasUIItem = (this->GetOwner() == NewCanvas->GetOwner());
		NewCanvas->MarkCanvasUpdate();
	}
	else
	{
		isCanvasUIItem = false;
	}
}

void UUIItem::CalculateHorizontalStretchFromAnchorAndSize()
{
	float parentWidthMultiply = 0.0f;
	switch (widget.anchorHAlign)
	{
	case UIAnchorHorizontalAlign::Left:
	{
		parentWidthMultiply = 0.0f;
	}
	break;
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

	widget.stretchLeft = widget.anchorOffsetX + ParentUIItem->widget.width * parentWidthMultiply - widget.width * widget.pivot.X;
	widget.stretchRight = ParentUIItem->widget.width - widget.width - widget.stretchLeft;
}
void UUIItem::CalculateVerticalStretchFromAnchorAndSize()
{
	float parentHeightMultiply = 0.0f;
	switch (widget.anchorVAlign)
	{
	case UIAnchorVerticalAlign::Bottom:
	{
		parentHeightMultiply = 0.0f;
	}
	break;
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

	float parentWidthMultiply = 0.0f;
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

	float parentHeightMultiply = 0.0f;
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

#pragma region VertexPositionChangeCallback
void UUIItem::RegisterLayoutChange(const FSimpleDelegate& InDelegate)
{
	layoutChangeCallback.Add(InDelegate);
}
void UUIItem::UnregisterLayoutChange(const FSimpleDelegate& InDelegate)
{
	layoutChangeCallback.Remove(InDelegate.GetHandle());
}
#pragma endregion VertexPositionChangeCallback

bool UUIItem::CheckRenderCanvas()const
{
	if (RenderCanvas.IsValid())return true;
	RenderCanvas = LGUIUtils::GetComponentInParent<ULGUICanvas>(this->GetOwner(), false);
	if (RenderCanvas.IsValid())
	{
		isCanvasUIItem = (this->GetOwner() == RenderCanvas->GetOwner());
		return true;
	}
	isCanvasUIItem = false;
	return false;
}

DECLARE_CYCLE_STAT(TEXT("UIItem UpdateLayoutAndGeometry"), STAT_UIItemUpdateLayoutAndGeometry, STATGROUP_LGUI);
void UUIItem::UpdateLayoutAndGeometry(bool& parentLayoutChanged, bool shouldUpdateLayout)
{
	SCOPE_CYCLE_COUNTER(STAT_UIItemUpdateLayoutAndGeometry);
	UpdateCachedData();
	UpdateBasePrevData();
	//update layout
	if (parentLayoutChanged == false)
	{
		if (cacheForThisUpdate_LayoutChanged)
			parentLayoutChanged = true;
	}
	//if parent layout change or self layout change, then update layout
	if (parentLayoutChanged && shouldUpdateLayout)
	{
		bCanSetAnchorFromTransform = false;
		CalculateTransformFromAnchor();
		CallUIComponentsDimensionsChanged(true, cacheForThisUpdate_SizeChanged);
		bCanSetAnchorFromTransform = true;
	}

	//update cache data
	UpdateCachedDataBeforeGeometry();
	//alpha
	if (this->inheritAlpha)
	{
		if (ParentUIItem.IsValid())
		{
			auto tempAlpha = ParentUIItem->GetCalculatedParentAlpha() * (Color255To1_Table[ParentUIItem->widget.color.A]);
			this->SetCalculatedParentAlpha(tempAlpha);
		}
	}
	else
	{
		this->SetCalculatedParentAlpha(1.0f);
	}
	//update geometry
	UpdateGeometry(parentLayoutChanged);
	//post update geometry

	//callback
	if (parentLayoutChanged && layoutChangeCallback.IsBound())
	{
		layoutChangeCallback.Broadcast();
	}
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
		resultLocation.X = parentWidget.width * (-parentWidget.pivot.X);
		resultLocation.X += widget.anchorOffsetX;
	}
	break;
	case UIAnchorHorizontalAlign::Center:
	{
		resultLocation.X = parentWidget.width * (0.5f - parentWidget.pivot.X);
		resultLocation.X += widget.anchorOffsetX;
	}
	break;
	case UIAnchorHorizontalAlign::Right:
	{
		resultLocation.X = parentWidget.width * (1 - parentWidget.pivot.X);
		resultLocation.X += widget.anchorOffsetX;
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

		resultLocation.X = -parentWidget.pivot.X * parentWidget.width;
		resultLocation.X += widget.stretchLeft;
		resultLocation.X += widget.pivot.X * widget.width;
	}
	break;
	}
	switch (widget.anchorVAlign)
	{
	case UIAnchorVerticalAlign::Top:
	{
		resultLocation.Y = parentWidget.height * (1 - parentWidget.pivot.Y);
		resultLocation.Y += widget.anchorOffsetY;
	}
	break;
	case UIAnchorVerticalAlign::Middle:
	{
		resultLocation.Y = parentWidget.height * (0.5f - parentWidget.pivot.Y);
		resultLocation.Y += widget.anchorOffsetY;
	}
	break;
	case UIAnchorVerticalAlign::Bottom:
	{
		resultLocation.Y = parentWidget.height * (-parentWidget.pivot.Y);
		resultLocation.Y += widget.anchorOffsetY;
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

		resultLocation.Y = -parentWidget.pivot.Y * parentWidget.height;
		resultLocation.Y += widget.stretchBottom;
		resultLocation.Y += widget.pivot.Y * widget.height;
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
	this->cacheForThisUpdate_ColorChanged = bColorChanged;
	this->cacheForThisUpdate_LayoutChanged = bLayoutChanged;
	this->cacheForThisUpdate_SizeChanged = bSizeChanged;
}
void UUIItem::UpdateCachedDataBeforeGeometry()
{
	if (bColorChanged)cacheForThisUpdate_ColorChanged = true;
	if (bLayoutChanged)cacheForThisUpdate_LayoutChanged = true;
	if (bSizeChanged)cacheForThisUpdate_SizeChanged = true;
}
void UUIItem::UpdateBasePrevData()
{
	bColorChanged = false;
	bLayoutChanged = false;
	bSizeChanged = false;
}

void UUIItem::SetWidget(const FUIWidget& inWidget)
{
	SetDepth(inWidget.depth);
	SetColor(inWidget.color);
	SetPivot(inWidget.pivot);
	SetAnchorHAlign(inWidget.anchorHAlign);
	SetAnchorVAlign(inWidget.anchorVAlign);
	if (inWidget.anchorHAlign == UIAnchorHorizontalAlign::Stretch)
	{
		SetAnchorOffsetX(inWidget.anchorOffsetX);
		SetWidth(inWidget.width);
		SetStretchLeft(inWidget.stretchLeft);
		SetStretchRight(inWidget.stretchRight);
	}
	else
	{
		SetStretchLeft(inWidget.stretchLeft);
		SetStretchRight(inWidget.stretchRight);
		SetAnchorOffsetX(inWidget.anchorOffsetX);
		SetWidth(inWidget.width);
	}
	if (inWidget.anchorVAlign == UIAnchorVerticalAlign::Stretch)
	{
		SetAnchorOffsetY(inWidget.anchorOffsetY);
		SetHeight(inWidget.height);
		SetStretchTop(inWidget.stretchTop);
		SetStretchBottom(inWidget.stretchBottom);
	}
	else
	{
		SetStretchTop(inWidget.stretchTop);
		SetStretchBottom(inWidget.stretchBottom);
		SetAnchorOffsetY(inWidget.anchorOffsetY);
		SetHeight(inWidget.height);
	}
}

void UUIItem::SetDepth(int32 depth, bool propagateToChildren) {
	if (widget.depth != depth)
	{
		int32 diff = depth - widget.depth;
		widget.depth = depth;
		if (propagateToChildren)
		{
			auto children = this->UIChildren;
			for (int i = 0; i < children.Num(); i++)
			{
				auto child = children[i];
				if (IsValid(child))
				{
					child->SetDepth(child->widget.depth + diff, propagateToChildren);
				}
			}
		}
		DepthChanged();
		if (CheckRenderCanvas())
		{
			RenderCanvas->MarkCanvasUpdate();
			RenderCanvas->MarkSortRenderPriority();
		}
	}
}
void UUIItem::SetColor(FColor color) {
	if (widget.color != color)
	{
		MarkColorDirty();
		widget.color = color;
	}
}
void UUIItem::SetAlpha(float newAlpha) {
	newAlpha = newAlpha > 1.0f ? 1.0f : newAlpha;
	newAlpha = newAlpha < 0.0f ? 0.0f : newAlpha;
	auto uintAlpha = (uint8)(newAlpha * 255);
	if (widget.color.A != uintAlpha)
	{
		MarkColorDirty();
		widget.color.A = uintAlpha;
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
void UUIItem::DepthChanged()
{

}
void UUIItem::SetAnchorOffsetX(float newOffset) 
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
void UUIItem::SetAnchorOffsetY(float newOffset) 
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
void UUIItem::SetUIRelativeLocation(FVector newLocation)
{
	Super::SetRelativeLocation(newLocation);
}
void UUIItem::SetUIRelativeLocationAndRotation(const FVector& newLocation, const FRotator& newRotation)
{
	Super::SetRelativeLocationAndRotation(newLocation, newRotation);
}
void UUIItem::SetUIRelativeLocationAndRotationQuat(const FVector& newLocation, const FQuat& newRotation)
{
	Super::SetRelativeLocationAndRotation(newLocation, newRotation);
}
void UUIItem::SetUIRelativeRotation(const FRotator& newRotation)
{
	Super::SetRelativeLocationAndRotation(GetRelativeLocation(), newRotation);
}
void UUIItem::SetUIRelativeRotationQuat(const FQuat& newRotation)
{
	Super::SetRelativeLocationAndRotation(GetRelativeLocation(), newRotation);
}
void UUIItem::SetUIParent(UUIItem* inParent, bool keepWorldTransform)
{
	Super::AttachToComponent(inParent, keepWorldTransform ? FAttachmentTransformRules::KeepWorldTransform : FAttachmentTransformRules::KeepRelativeTransform);
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
void UUIItem::SetCalculatedParentAlpha(float alpha)
{
	if (FMath::Abs(calculatedParentAlpha - alpha) > SMALL_NUMBER)
	{
		MarkColorDirty();
		calculatedParentAlpha = alpha;
	}
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

#ifdef LGUI_DRAWCALLMODE_AUTO
void UUIItem::GetLocalSpaceMinMaxPoint_ForAutoManageDepth(FVector2D& min, FVector2D& max)const
{
	min = GetLocalSpaceLeftBottomPoint();
	max = GetLocalSpaceRightTopPoint();
}
#endif

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
	if (CheckRenderCanvas())
	{
		RenderCanvas->MarkCanvasUpdate();
		RenderCanvas->MarkCanvasUpdateLayout();
	}
}
void UUIItem::MarkColorDirty() 
{ 
	bColorChanged = true;
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
}
void UUIItem::MarkCanvasUpdate()
{
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
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
	if (!IsUIActiveInHierarchy())return false;
	if (!RenderCanvas.IsValid())return false;
	auto inverseTf = GetComponentTransform().Inverse();
	auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
	auto localSpaceRayEnd = inverseTf.TransformPosition(End);

	//start and end point must be different side of z plane
	if (FMath::Sign(localSpaceRayOrigin.Z) != FMath::Sign(localSpaceRayEnd.Z))
	{
		auto result = FMath::LinePlaneIntersection(localSpaceRayOrigin, localSpaceRayEnd, FVector::ZeroVector, FVector(0, 0, 1));
		//hit point inside rect area
		if (result.X > GetLocalSpaceLeft() && result.X < GetLocalSpaceRight() && result.Y > GetLocalSpaceBottom() && result.Y < GetLocalSpaceTop())
		{
			OutHit.TraceStart = Start;
			OutHit.TraceEnd = End;
			OutHit.Actor = GetOwner();
			OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
			OutHit.Location = GetComponentTransform().TransformPosition(result);
			OutHit.Normal = GetComponentTransform().TransformVector(FVector(0, 0, 1));
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
	if (RenderCanvas.IsValid())
	{
		return RenderCanvas.Get();
	}
	return nullptr;
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

FColor UUIItem::GetFinalColor()const
{
	return FColor(widget.color.R, widget.color.G, widget.color.B, inheritAlpha ? widget.color.A * calculatedParentAlpha : widget.color.A);
}

uint8 UUIItem::GetFinalAlpha()const
{
	return inheritAlpha ? (uint8)(widget.color.A * calculatedParentAlpha) : widget.color.A;
}

float UUIItem::GetFinalAlpha01()const
{
	return Color255To1_Table[GetFinalAlpha()];
}

#pragma region InteractionGroup
bool UUIItem::IsGroupAllowInteraction()const
{
	bool thisGroupsAllowInteraction = true;
	if (auto interactionGroup = GetOwner()->FindComponentByClass<UUIInteractionGroup>())
	{
		if (interactionGroup->GetIgnoreParentGroup())
		{
			thisGroupsAllowInteraction = interactionGroup->GetInteractable();
		}
		else
		{
			if (allUpParentGroupAllowInteraction)
			{
				thisGroupsAllowInteraction = interactionGroup->GetInteractable();
			}
			else
			{
				thisGroupsAllowInteraction = false;
			}
		}
	}
	else
	{
		thisGroupsAllowInteraction = allUpParentGroupAllowInteraction;
	}
	return thisGroupsAllowInteraction;
}
void UUIItem::SetChildInteractionGroupStateChangeRecursive(bool InParentInteractable)
{
	for (auto uiChild : UIChildren)
	{
		if (IsValid(uiChild))
		{
			uiChild->allUpParentGroupAllowInteraction = InParentInteractable;
			uiChild->SetInteractionGroupStateChange();
		}
	}
}

void UUIItem::SetInteractionGroupStateChange(bool InInteractable, bool InIgnoreParentGroup)
{
	bool thisGroupsAllowInteraction = true;
	if (InIgnoreParentGroup)
	{
		thisGroupsAllowInteraction = InInteractable;
	}
	else
	{
		if (allUpParentGroupAllowInteraction)
		{
			thisGroupsAllowInteraction = InInteractable;
		}
		else
		{
			thisGroupsAllowInteraction = false;
		}
	}
	SetChildInteractionGroupStateChangeRecursive(thisGroupsAllowInteraction);
	CallUIComponentsInteractionStateChanged();
}
void UUIItem::SetInteractionGroupStateChange()
{
	auto thisGroupsAllowInteraction = IsGroupAllowInteraction();
	SetChildInteractionGroupStateChangeRecursive(thisGroupsAllowInteraction);
	CallUIComponentsInteractionStateChanged();
}
#pragma endregion InteractionGroup

#pragma region UIActive

void UUIItem::OnChildActiveStateChanged(UUIItem* child)
{
	CallUIComponentsChildActiveInHierarchyStateChanged(child, child->IsUIActiveInHierarchy());
}

void UUIItem::SetChildUIActiveRecursive(bool InUpParentUIActive)
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
				uiChild->SetChildUIActiveRecursive(uiChild->IsUIActiveInHierarchy());
				//callback for parent
				this->OnChildActiveStateChanged(uiChild);
			}
			//state not changed
			else
			{
				uiChild->allUpParentUIActive = InUpParentUIActive;
				//affect children
				uiChild->SetChildUIActiveRecursive(uiChild->IsUIActiveInHierarchy());
			}
		}
	}
}
void UUIItem::SetUIActive(bool active)
{
	if (bIsUIActive != active)
	{
		bIsUIActive = active;
		if (allUpParentUIActive)//state change only happens when up parent is active
		{
			ApplyUIActiveState();
			//affect children
			SetChildUIActiveRecursive(bIsUIActive);
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
		if (IsUIActiveInHierarchy() && actorLabel.StartsWith(prefix))
		{
			actorLabel = actorLabel.Right(actorLabel.Len() - prefix.Len());
			ownerActor->SetActorLabel(actorLabel);
			ownerActor->SetIsTemporarilyHiddenInEditor(false);
		}
		else if (!IsUIActiveInHierarchy() && !actorLabel.StartsWith(prefix))
		{
			actorLabel = prefix.Append(actorLabel);
			ownerActor->SetActorLabel(actorLabel);
			ownerActor->SetIsTemporarilyHiddenInEditor(true);
		}
	}
#endif
	if (isCanvasUIItem)RenderCanvas->OnUIActiveStateChanged(IsUIActiveInHierarchy());
	bColorChanged = true;
	bLayoutChanged = true;
	//canvas update
	if (CheckRenderCanvas()) RenderCanvas->MarkCanvasUpdate();
	//callback
	CallUIComponentsActiveInHierarchyStateChanged();
}

#pragma endregion UIActive

float UUIItem::Color255To1_Table[256] =
{
	0,0.003921569,0.007843138,0.01176471,0.01568628,0.01960784,0.02352941,0.02745098,0.03137255,0.03529412,0.03921569,0.04313726,0.04705882,0.05098039
	,0.05490196,0.05882353,0.0627451,0.06666667,0.07058824,0.07450981,0.07843138,0.08235294,0.08627451,0.09019608,0.09411765,0.09803922,0.1019608,0.1058824
	,0.1098039,0.1137255,0.1176471,0.1215686,0.1254902,0.1294118,0.1333333,0.1372549,0.1411765,0.145098,0.1490196,0.1529412,0.1568628,0.1607843,0.1647059,0.1686275
	,0.172549,0.1764706,0.1803922,0.1843137,0.1882353,0.1921569,0.1960784,0.2,0.2039216,0.2078431,0.2117647,0.2156863,0.2196078,0.2235294,0.227451,0.2313726,0.2352941
	,0.2392157,0.2431373,0.2470588,0.2509804,0.254902,0.2588235,0.2627451,0.2666667,0.2705882,0.2745098,0.2784314,0.282353,0.2862745,0.2901961,0.2941177,0.2980392,0.3019608
	,0.3058824,0.3098039,0.3137255,0.3176471,0.3215686,0.3254902,0.3294118,0.3333333,0.3372549,0.3411765,0.345098,0.3490196,0.3529412,0.3568628,0.3607843,0.3647059,0.3686275
	,0.372549,0.3764706,0.3803922,0.3843137,0.3882353,0.3921569,0.3960784,0.4,0.4039216,0.4078431,0.4117647,0.4156863,0.4196078,0.4235294,0.427451,0.4313726,0.4352941,0.4392157
	,0.4431373,0.4470588,0.4509804,0.454902,0.4588235,0.4627451,0.4666667,0.4705882,0.4745098,0.4784314,0.4823529,0.4862745,0.4901961,0.4941176,0.4980392,0.5019608,0.5058824,0.509804
	,0.5137255,0.5176471,0.5215687,0.5254902,0.5294118,0.5333334,0.5372549,0.5411765,0.5450981,0.5490196,0.5529412,0.5568628,0.5607843,0.5647059,0.5686275,0.572549,0.5764706,0.5803922
	,0.5843138,0.5882353,0.5921569,0.5960785,0.6,0.6039216,0.6078432,0.6117647,0.6156863,0.6196079,0.6235294,0.627451,0.6313726,0.6352941,0.6392157,0.6431373,0.6470588,0.6509804,0.654902
	,0.6588235,0.6627451,0.6666667,0.6705883,0.6745098,0.6784314,0.682353,0.6862745,0.6901961,0.6941177,0.6980392,0.7019608,0.7058824,0.7098039,0.7137255,0.7176471,0.7215686,0.7254902,0.7294118
	,0.7333333,0.7372549,0.7411765,0.7450981,0.7490196,0.7529412,0.7568628,0.7607843,0.7647059,0.7686275,0.772549,0.7764706,0.7803922,0.7843137,0.7882353,0.7921569,0.7960784,0.8,0.8039216,0.8078431
	,0.8117647,0.8156863,0.8196079,0.8235294,0.827451,0.8313726,0.8352941,0.8392157,0.8431373,0.8470588,0.8509804,0.854902,0.8588235,0.8627451,0.8666667,0.8705882,0.8745098,0.8784314,0.8823529,0.8862745
	,0.8901961,0.8941177,0.8980392,0.9019608,0.9058824,0.9098039,0.9137255,0.9176471,0.9215686,0.9254902,0.9294118,0.9333333,0.9372549,0.9411765,0.945098,0.9490196,0.9529412,0.9568627,0.9607843,0.9647059
	,0.9686275,0.972549,0.9764706,0.9803922,0.9843137,0.9882353,0.9921569,0.9960784,1
};




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
			if (!Component.IsValid())return;
			const auto& widget = Component->GetWidget();
			auto worldTransform = Component->GetComponentTransform();
			FVector relativeOffset(0, 0, 0);
			relativeOffset.X = (0.5f - widget.pivot.X) * widget.width;
			relativeOffset.Y = (0.5f - widget.pivot.Y) * widget.height;
			auto worldLocation = worldTransform.TransformPosition(relativeOffset);
			//calculate world location
			if (Component->GetParentAsUIItem() != nullptr)
			{
				FVector relativeLocation = Component->GetRelativeLocation();
				const auto& parentWidget = Component->GetParentAsUIItem()->GetWidget();
				switch (widget.anchorHAlign)
				{
				case UIAnchorHorizontalAlign::Left:
				{
					relativeLocation.X = parentWidget.width * (-parentWidget.pivot.X);
					relativeLocation.X += widget.anchorOffsetX;
				}
				break;
				case UIAnchorHorizontalAlign::Center:
				{
					relativeLocation.X = parentWidget.width * (0.5f - parentWidget.pivot.X);
					relativeLocation.X += widget.anchorOffsetX;
				}
				break;
				case UIAnchorHorizontalAlign::Right:
				{
					relativeLocation.X = parentWidget.width * (1 - parentWidget.pivot.X);
					relativeLocation.X += widget.anchorOffsetX;
				}
				break;
				case UIAnchorHorizontalAlign::Stretch:
				{
					relativeLocation.X = -parentWidget.pivot.X * parentWidget.width;
					relativeLocation.X += widget.stretchLeft;
					relativeLocation.X += widget.pivot.X * widget.width;
				}
				break;
				}
				switch (widget.anchorVAlign)
				{
				case UIAnchorVerticalAlign::Top:
				{
					relativeLocation.Y = parentWidget.height * (1 - parentWidget.pivot.Y);
					relativeLocation.Y += widget.anchorOffsetY;
				}
				break;
				case UIAnchorVerticalAlign::Middle:
				{
					relativeLocation.Y = parentWidget.height * (0.5f - parentWidget.pivot.Y);
					relativeLocation.Y += widget.anchorOffsetY;
				}
				break;
				case UIAnchorVerticalAlign::Bottom:
				{
					relativeLocation.Y = parentWidget.height * (-parentWidget.pivot.Y);
					relativeLocation.Y += widget.anchorOffsetY;
				}
				break;
				case UIAnchorVerticalAlign::Stretch:
				{
					relativeLocation.Y = -parentWidget.pivot.Y * parentWidget.height;
					relativeLocation.Y += widget.stretchBottom;
					relativeLocation.Y += widget.pivot.Y * widget.height;
				}
				break;
				}
				auto relativeTf = Component->GetRelativeTransform();
				relativeTf.SetLocation(relativeLocation);
				FTransform calculatedWorldTf;
				FTransform::Multiply(&calculatedWorldTf, &relativeTf, &(Component->GetParentAsUIItem()->GetComponentTransform()));
				worldLocation = calculatedWorldTf.TransformPosition(relativeOffset);
			}

			auto extends = FVector(widget.width, widget.height, 0) * 0.5f;
			auto scaleZ = worldTransform.GetScale3D().Z;
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				auto& View = Views[ViewIndex];
				if (VisibilityMap & (1 << ViewIndex))
				{
					bool canDraw = false;
					FLinearColor DrawColor = FColor(128, 128, 128, 128);//gray means normal object
					if (ULGUIEditorManagerObject::IsSelected(Component->GetOwner()))//select self
					{
						DrawColor = FColor(0, 255, 0, 255);//green means selected object
						extends += FVector(0, 0, scaleZ);
						canDraw = true;
					}
					else
					{
						//parent selected
						if (IsValid(Component->GetParentAsUIItem()))
						{
							if (ULGUIEditorManagerObject::IsSelected(Component->GetParentAsUIItem()->GetOwner()))
							{
								canDraw = true;
							}
						}
						//child selected
						const auto childrenCompArray = Component->GetAttachUIChildren();
						for (auto uiComp : childrenCompArray)
						{
							if (IsValid(uiComp) && IsValid(uiComp->GetOwner()) && ULGUIEditorManagerObject::IsSelected(uiComp->GetOwner()))
							{
								canDraw = true;
								break;
							}
						}
						//other object of same hierarchy is selected
						if (IsValid(Component->GetParentAsUIItem()))
						{
							const auto& sameLevelCompArray = Component->GetParentAsUIItem()->GetAttachUIChildren();
							for (auto uiComp : sameLevelCompArray)
							{
								if (IsValid(uiComp) && IsValid(uiComp->GetOwner()) && ULGUIEditorManagerObject::IsSelected(uiComp->GetOwner()))
								{
									canDraw = true;
									break;
								}
							}
						}
					}
					//canvas scaler
					if (!canDraw)
					{
						if (Component->IsCanvasUIItem())
						{
							if (auto canvasScaler = Component->GetOwner()->FindComponentByClass<ULGUICanvasScaler>())
							{
								if (ULGUIEditorManagerObject::AnySelectedIsChildOf(Component->GetOwner()))
								{
									canDraw = true;
									DrawColor = FColor(255, 227, 124);
								}
							}
						}
					}

					if (canDraw)
					{
						FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
						DrawOrientedWireBox(PDI, worldLocation, worldTransform.GetScaledAxis(EAxis::X), worldTransform.GetScaledAxis(EAxis::Y), worldTransform.GetScaledAxis(EAxis::Z), extends, DrawColor, SDPG_Foreground);
					}
				}
			}
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
		BodySetup = NewObject<UBodySetup>(this);
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
