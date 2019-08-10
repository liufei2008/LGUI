﻿// Copyright 2019 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIItem.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Core/ActorComponent/UIPanel.h"
#include "Core/ActorComponent/UIInteractionGroup.h"
#include "Core/LGUISettings.h"
#include "Core/UIComponentBase.h"
#include "Core/Actor/LGUIManagerActor.h"
#include "PhysicsEngine/BodySetup.h"

DECLARE_CYCLE_STAT(TEXT("UIItem UpdateLayoutAndGeometry"), STAT_UIItemUpdateLayoutAndGeometry, STATGROUP_LGUI);

UUIItem::UUIItem()
{
	PrimaryComponentTick.bCanEverTick = false;
	itemType = UIItemType::UIItem;

	bWantsOnUpdateTransform = true;
	
	bVertexPositionChanged = true;
	bColorChanged = true;
	bDepthChanged = true;
	bTransformChanged = true;

	traceChannel = GetDefault<ULGUISettings>()->defaultTraceChannel;
}

void UUIItem::BeginPlay()
{
	Super::BeginPlay();

	if (auto parent = GetAttachParent())
	{
		cacheParentUIItem = Cast<UUIItem>(parent);
	}

	bVertexPositionChanged = true;
	bColorChanged = true;
	bDepthChanged = true;
	bTransformChanged = true;
	MarkPanelUpdate();
}

void UUIItem::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);
}

void UUIItem::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	Super::EndPlay(EndPlayReason);
}

#pragma region UIBaseComponent
void UUIItem::CallUIComponentsActiveInHierarchyStateChanged()
{
	if (this->GetOwner() == nullptr)return;
	if (this->GetWorld() == nullptr)return;
	OnUIActiveInHierachy(IsUIActiveInHierarchy());
#if WITH_EDITOR
	if (!this->GetWorld()->IsGameWorld())
	{
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
		CompItem->OnUIDimensionsChanged(positionChanged, sizeChanged);
	}

	//call parent
	if (cacheParentUIItem)
	{
		cacheParentUIItem->CallUIComponentsChildDimensionsChanged(this, positionChanged, sizeChanged);
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
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
		GetOwner()->GetComponents(UIBaseComponentArray, false);
	}
#endif
	for (int i = 0; i < UIBaseComponentArray.Num(); i++)
	{
		auto& CompItem = UIBaseComponentArray[i];
		CompItem->OnUIChildHierarchyIndexChanged(child);
	}
}
#pragma endregion UIBaseComponent


void UUIItem::OnChildHierarchyIndexChanged(UUIItem* child)
{
	CallUIComponentsChildHierarchyIndexChanged(child);
}
void UUIItem::SetHierarchyIndex(int32 InInt) 
{ 
	if (InInt != hierarchyIndex)
	{
		hierarchyIndex = InInt;
		if (cacheParentUIItem != nullptr)
		{
			cacheParentUIItem->OnChildHierarchyIndexChanged(this);
		}
	}
}

void UUIItem::MarkAllDirtyRecursive()
{
	bVertexPositionChanged = true;
	bColorChanged = true;
	bDepthChanged = true;
	bTransformChanged = true;

	auto& children = this->GetAttachChildren();
	for (auto child : children)
	{
		if (UUIItem* uiChild = Cast<UUIItem>(child))
		{
			uiChild->MarkAllDirtyRecursive();
		}
	}
}

#if WITH_EDITOR
void UUIItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (PropertyChangedEvent.Property != nullptr)
	{
		auto propetyName = PropertyChangedEvent.Property->GetName();
		if (propetyName == TEXT("bIsUIActive"))
		{
			bIsUIActive = !bIsUIActive;//make it work
			SetUIActive(!bIsUIActive);
		}

		else if (propetyName == TEXT("hierarchyIndex"))
		{
			hierarchyIndex = hierarchyIndex + 1;//make it work
			SetHierarchyIndex(hierarchyIndex - 1);
		}

		else if (propetyName == TEXT("width"))
		{
			auto width = widget.width;
			widget.width += 1;//+1 just for make it work
			SetWidth(width);
		}
		else if (propetyName == TEXT("height"))
		{
			auto height = widget.height;
			widget.height += 1;//+1 just for make it work
			SetHeight(height);
		}
		else if (propetyName == TEXT("anchorOffsetX"))
		{
			auto originValue = widget.anchorOffsetX;
			widget.anchorOffsetX += 1;
			SetAnchorOffsetX(originValue);
		}
		else if (propetyName == TEXT("anchorOffsetY"))
		{
			auto originValue = widget.anchorOffsetY;
			widget.anchorOffsetY += 1;
			SetAnchorOffsetY(originValue);
		}
		else if (propetyName == TEXT("stretchLeft"))
		{
			auto originValue = widget.stretchLeft;
			widget.stretchLeft += 1;
			SetStretchLeft(originValue);
		}
		else if (propetyName == TEXT("stretchRight"))
		{
			auto originValue = widget.stretchRight;
			widget.stretchRight += 1;
			SetStretchRight(originValue);
		}
		else if (propetyName == TEXT("stretchTop"))
		{
			auto originValue = widget.stretchTop;
			widget.stretchTop += 1;
			SetStretchTop(originValue);
		}
		else if (propetyName == TEXT("stretchBottom"))
		{
			auto originValue = widget.stretchBottom;
			widget.stretchBottom += 1;
			SetStretchBottom(originValue);
		}

		else if (propetyName == TEXT("anchorHAlign"))
		{
			auto newAlign = widget.anchorHAlign;
			widget.anchorHAlign = prevAnchorHAlign;
			SetAnchorHAlign(newAlign);
		}
		else if (propetyName == TEXT("anchorVAlign"))
		{
			auto newAlign = widget.anchorVAlign;
			widget.anchorVAlign = prevAnchorVAlign;
			SetAnchorVAlign(newAlign);
			prevAnchorVAlign = newAlign;
		}

		EditorForceUpdateImmediately();
		UpdateBounds();
	}
}
void UUIItem::PostEditComponentMove(bool bFinished)
{
#if WITH_EDITORONLY_DATA
	//modify AnchorOffset for drag in editor
	if (widget.anchorHAlign != UIAnchorHorizontalAlign::None)
	{
		float anchorOffsetX = this->RelativeLocation.X - prevRelativeLocation.X + widget.anchorOffsetX;
		SetAnchorOffsetX(anchorOffsetX);
	}
	if (widget.anchorVAlign != UIAnchorVerticalAlign::None)
	{
		float anchorOffsetY = this->RelativeLocation.Y - prevRelativeLocation.Y + widget.anchorOffsetY;
		SetAnchorOffsetY(anchorOffsetY);
	}
#endif

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
	if (CheckRenderUIPanel())
	{
		if (RenderUIPanel->GetFirstUIPanel())
		{
			RenderUIPanel->GetFirstUIPanel()->MarkAllDirtyRecursive();
			RenderUIPanel->GetFirstUIPanel()->UpdatePanelGeometry();
		}
		else
		{
			RenderUIPanel->MarkAllDirtyRecursive();
			RenderUIPanel->UpdatePanelGeometry();
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
	CallUIComponentsDimensionsChanged(true, true);
	//callback
	CallUIComponentsAttachmentChanged();
}
void UUIItem::OnUpdateTransform(EUpdateTransformFlags UpdateTransformFlags, ETeleportType Teleport)
{
	Super::OnUpdateTransform(UpdateTransformFlags, Teleport);
	bTransformChanged = true;
	MarkPanelUpdate();
}
void UUIItem::OnChildAttached(USceneComponent* ChildComponent)
{
	Super::OnChildAttached(ChildComponent);
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;
	if (UUIItem* childUIItem = Cast<UUIItem>(ChildComponent))
	{
		MarkVertexPositionDirty();
		//interaction group
		childUIItem->allUpParentGroupAllowInteraction = this->IsGroupAllowInteraction();
		childUIItem->SetInteractionGroupStateChange();
		//active
		childUIItem->allUpParentUIActive = this->IsUIActiveInHierarchy();
		bool parentUIActive = childUIItem->IsUIActiveInHierarchy();
		childUIItem->SetChildUIActiveRecursive(parentUIActive);

		CallUIComponentsChildAttachmentChanged(childUIItem, true);
	}
	MarkPanelUpdate();
}
void UUIItem::OnChildDetached(USceneComponent* ChildComponent)
{
	Super::OnChildDetached(ChildComponent);
	if (this->IsPendingKillOrUnreachable())return;
	if (GetWorld() == nullptr)return;
	if (auto childUIItem = Cast<UUIItem>(ChildComponent))
	{
		MarkVertexPositionDirty();

		CallUIComponentsChildAttachmentChanged(childUIItem, false);
	}
	MarkPanelUpdate();
}
void UUIItem::OnRegister()
{
	Super::OnRegister();
	LGUIManager::AddUIItem(this);
#if WITH_EDITORONLY_DATA
	if (!this->GetWorld()->IsGameWorld())
	{
		if (!IsValid(HelperComp))
		{
			HelperComp = NewObject<UUIItemEditorHelperComp>(GetOwner());
			HelperComp->Parent = this;
			HelperComp->AttachToComponent(this, FAttachmentTransformRules::KeepRelativeTransform);
		}
	}
#endif
}
void UUIItem::OnUnregister()
{
	Super::OnUnregister();
	LGUIManager::RemoveUIItem(this);
}

void UUIItem::UIHierarchyChanged()
{
	auto oldRenderUIPanel = RenderUIPanel;
	RenderUIPanel = nullptr;//force find new
	CheckRenderUIPanel();
	if (oldRenderUIPanel != RenderUIPanel)//if attach to new UIPanel, need to remove from old and add to new
	{
		if (itemType == UIItemType::UIRenderable)
		{
			if (oldRenderUIPanel != nullptr)
			{
				oldRenderUIPanel->RemoveFromDrawcall((UUIRenderable*)this);
				oldRenderUIPanel->MarkNeedUpdate();
			}
			if (RenderUIPanel != nullptr)
			{
				RenderUIPanel->InsertIntoDrawcall((UUIRenderable*)this);
				RenderUIPanel->MarkNeedUpdate();
			}
		}
		else
		{
			if (oldRenderUIPanel != nullptr)
			{
				oldRenderUIPanel->MarkNeedUpdate();
			}
			if (RenderUIPanel != nullptr)
			{
				RenderUIPanel->MarkNeedUpdate();
			}
		}
	}
	MarkVertexPositionDirty();

	const auto& children = GetAttachChildren();
	for (auto item : children)
	{
		auto uiItem = Cast<UUIItem>(item);
		if(uiItem) uiItem->UIHierarchyChanged();
	}

	if (auto parent = GetAttachParent())
	{
		cacheParentUIItem = Cast<UUIItem>(parent);
		if (cacheParentUIItem != nullptr)
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

void UUIItem::CalculateHorizontalStretchFromAnchorAndSize()
{
	const auto& parentWidget = cacheParentUIItem->widget;
	widget.stretchLeft = (parentWidget.width - widget.width) * 0.5f + widget.anchorOffsetX;
	widget.stretchRight = parentWidget.width - widget.width - widget.stretchLeft;
}
void UUIItem::CalculateVerticalStretchFromAnchorAndSize()
{
	const auto& parentWidget = cacheParentUIItem->widget;
	widget.stretchBottom = (parentWidget.height - widget.height) * 0.5f + widget.anchorOffsetY;
	widget.stretchTop = parentWidget.height - widget.height - widget.stretchBottom;
}
bool UUIItem::CalculateHorizontalAnchorAndSizeFromStretch()
{
	bool sizeChanged = false;
	const auto& parentWidget = cacheParentUIItem->widget;
	float width = parentWidget.width - widget.stretchLeft - widget.stretchRight;
	if (IsFloatNotEqual(widget.width, width))
	{
		widget.width = width;
		WidthChanged();
		sizeChanged = true;
	}

	widget.anchorOffsetX = widget.stretchLeft - (parentWidget.width - widget.width) * 0.5f;
	return sizeChanged;
}
bool UUIItem::CalculateVerticalAnchorAndSizeFromStretch()
{
	bool sizeChanged = false;
	const auto& parentWidget = cacheParentUIItem->widget;
	float height = parentWidget.height - widget.stretchTop - widget.stretchBottom;
	if (IsFloatNotEqual(widget.height, height))
	{
		widget.height = height;
		HeightChanged();
		sizeChanged = true;
	}

	widget.anchorOffsetY = widget.stretchBottom - (parentWidget.height - widget.height) * 0.5f;
	return sizeChanged;
}

#pragma region VertexPositionChangeCallback
void UUIItem::RegisterVertexPositionChange(const FSimpleDelegate& InDelegate)
{
	vertexPositionChangeCallback.Add(InDelegate);
}
void UUIItem::UnregisterVertexPositionChange(const FSimpleDelegate& InDelegate)
{
	vertexPositionChangeCallback.Remove(InDelegate.GetHandle());
}
#pragma endregion VertexPositionChangeCallback

bool UUIItem::CheckRenderUIPanel()
{
	if (RenderUIPanel != nullptr)return true;
	LGUIUtils::FindRenderUIPanel(this, RenderUIPanel);
	if (RenderUIPanel != nullptr)return true;
	return false;
}

void UUIItem::UpdateLayoutAndGeometry(bool& parentLayoutChanged, bool& parentTransformChanged)
{
	SCOPE_CYCLE_COUNTER(STAT_UIItemUpdateLayoutAndGeometry);
	if (IsUIActiveInHierarchy() == false)return;
	UpdateCachedData();
	UpdateBasePrevData();
	//update layout
	if (parentLayoutChanged == false)
	{
		if (cacheForThisUpdate_VertexPositionChanged)
			parentLayoutChanged = true;
	}
	//if parent layout change or self layout change, then update layout
	if (parentLayoutChanged)
	{
		bool sizeChanged = CalculateLayoutRelatedParameters();
		CallUIComponentsDimensionsChanged(true, sizeChanged);
	}
	//prev update geometry
	if (cacheForThisUpdate_TransformChanged)
	{
		parentTransformChanged = true;
	}
	if (this->inheritAlpha)
	{
		if (cacheParentUIItem != nullptr)
		{
			auto tempAlpha = cacheParentUIItem->GetCalculatedParentAlpha() * (Color255To1_Table[cacheParentUIItem->widget.color.A]);
			this->SetCalculatedParentAlpha(tempAlpha);
		}
	}
	else
	{
		this->SetCalculatedParentAlpha(1.0f);
	}
	//update cache data
	UpdateCachedDataBeforeGeometry();
	//update geometry
	UpdateGeometry(parentTransformChanged);
	//post update geometry

	//callback
	vertexPositionChangeCallback.Broadcast();
}
void UUIItem::UpdateGeometry(const bool& parentTransformChanged)
{
	
}

bool UUIItem::CalculateLayoutRelatedParameters()
{
	if (cacheParentUIItem == nullptr)return false;
	bool sizeChanged = false;
	const auto& parentWidget = cacheParentUIItem->widget;
	FVector resultLocation = this->RelativeLocation;
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
		if (IsFloatNotEqual(widget.width, width))
		{
			widget.width = width;
			WidthChanged();
			MarkVertexPositionDirty();
			sizeChanged = true;
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
		if (IsFloatNotEqual(widget.height, height))
		{
			widget.height = height;
			HeightChanged();
			MarkVertexPositionDirty();
			sizeChanged = true;
		}

		resultLocation.Y = -parentWidget.pivot.Y * parentWidget.height;
		resultLocation.Y += widget.stretchBottom;
		resultLocation.Y += widget.pivot.Y * widget.height;
	}
	break;
	}
	if (IsVectorNotEqual(this->RelativeLocation, resultLocation))
	{
		this->SetRelativeLocation(resultLocation);
		RelativeLocation = resultLocation;
	}
#if WITH_EDITORONLY_DATA
	prevAnchorHAlign = widget.anchorHAlign;
	prevAnchorVAlign = widget.anchorVAlign;
	prevRelativeLocation = RelativeLocation;
#endif 
	return sizeChanged;
}
void UUIItem::UpdateCachedData()
{
	this->cacheForThisUpdate_DepthChanged = bDepthChanged;
	this->cacheForThisUpdate_VertexPositionChanged = bVertexPositionChanged;
	this->cacheForThisUpdate_ColorChanged = bColorChanged;
	this->cacheForThisUpdate_TransformChanged = bTransformChanged;
}
void UUIItem::UpdateCachedDataBeforeGeometry()
{
	if (bDepthChanged)cacheForThisUpdate_DepthChanged = true;
	if (bVertexPositionChanged)cacheForThisUpdate_VertexPositionChanged = true;
	if (bColorChanged)cacheForThisUpdate_ColorChanged = true;
	if (bTransformChanged)cacheForThisUpdate_TransformChanged = true;
}
void UUIItem::UpdateBasePrevData()
{
	bVertexPositionChanged = false;
	bColorChanged = false;
	bDepthChanged = false;
	bTransformChanged = false;
}

void UUIItem::SetDepth(int32 depth) {
	if (widget.depth != depth)
	{
		bDepthChanged = true;
		MarkPanelUpdate();
		widget.depth = depth;
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
	if (IsFloatNotEqual(widget.width, newWidth))
	{
		MarkVertexPositionDirty();
		widget.width = newWidth;
		WidthChanged();
		if (cacheParentUIItem)
		{
			CalculateHorizontalStretchFromAnchorAndSize();
		}
		CallUIComponentsDimensionsChanged(false, true);
	}
}
void UUIItem::SetHeight(float newHeight)
{
	if (IsFloatNotEqual(widget.height, newHeight))
	{
		MarkVertexPositionDirty();
		widget.height = newHeight;
		HeightChanged();
		if (cacheParentUIItem)
		{
			CalculateVerticalStretchFromAnchorAndSize();
		}
		CallUIComponentsDimensionsChanged(false, true);
	}
}
void UUIItem::WidthChanged()
{

}
void UUIItem::HeightChanged()
{

}
void UUIItem::SetAnchorOffsetX(float newOffset) 
{
	if (IsFloatNotEqual(widget.anchorOffsetX, newOffset))
	{
		widget.anchorOffsetX = newOffset;
		if (cacheParentUIItem)
		{
			const auto& parentWidget = cacheParentUIItem->widget;
			widget.stretchLeft = (parentWidget.width - widget.width) * 0.5f + widget.anchorOffsetX;
			widget.stretchRight = parentWidget.width - widget.width - widget.stretchLeft;
			
			MarkVertexPositionDirty();
			CallUIComponentsDimensionsChanged(true, false);
		}
	}
}
void UUIItem::SetAnchorOffsetY(float newOffset) 
{
	if (IsFloatNotEqual(widget.anchorOffsetY, newOffset))
	{
		widget.anchorOffsetY = newOffset;
		if (cacheParentUIItem)
		{
			const auto& parentWidget = cacheParentUIItem->widget;
			widget.stretchBottom = (parentWidget.height - widget.height) * 0.5f + widget.anchorOffsetY;
			widget.stretchTop = parentWidget.height - widget.height - widget.stretchBottom;
			
			MarkVertexPositionDirty();
			CallUIComponentsDimensionsChanged(true, false);
		}
	}
}
void UUIItem::SetAnchorOffset(FVector2D newOffset)
{
	bool anyChange = false;
	if (IsFloatNotEqual(widget.anchorOffsetX, newOffset.X))
	{
		widget.anchorOffsetX = newOffset.X;
		if (cacheParentUIItem)
		{
			anyChange = true;
			const auto& parentWidget = cacheParentUIItem->widget;
			widget.stretchLeft = (parentWidget.width - widget.width) * 0.5f + widget.anchorOffsetX;
			widget.stretchRight = parentWidget.width - widget.width - widget.stretchLeft;
		}
	}
	if (IsFloatNotEqual(widget.anchorOffsetY, newOffset.Y))
	{
		widget.anchorOffsetY = newOffset.Y;
		if (cacheParentUIItem)
		{
			anyChange = true;
			const auto& parentWidget = cacheParentUIItem->widget;
			widget.stretchBottom = (parentWidget.height - widget.height) * 0.5f + widget.anchorOffsetY;
			widget.stretchTop = parentWidget.height - widget.height - widget.stretchBottom;
		}
	}
	if (anyChange)
	{
		MarkVertexPositionDirty();
		CallUIComponentsDimensionsChanged(true, false);
	}
}
void UUIItem::SetUIRelativeLocation(FVector newLocation)
{
	if (IsVectorNotEqual(RelativeLocation, newLocation))
	{
		MarkVertexPositionDirty();
		this->SetRelativeLocation(newLocation);
		RelativeLocation = newLocation;

		if (cacheParentUIItem)
		{
			const auto& parentWidget = cacheParentUIItem->widget;
			if (widget.anchorHAlign != UIAnchorHorizontalAlign::None)
			{
				FVector resultLocation = this->RelativeLocation;
				switch (widget.anchorHAlign)
				{
				case UIAnchorHorizontalAlign::Left:
				{
					float anchorOffsetX = this->RelativeLocation.X + parentWidget.width * parentWidget.pivot.X;
					SetAnchorOffsetX(anchorOffsetX);
				}
				break;
				case UIAnchorHorizontalAlign::Center:
				{
					float anchorOffsetX = this->RelativeLocation.X + parentWidget.width * (parentWidget.pivot.X - 0.5f);
					SetAnchorOffsetX(anchorOffsetX);
				}
				break;
				case UIAnchorHorizontalAlign::Right:
				{
					float anchorOffsetX = this->RelativeLocation.X + parentWidget.width * (parentWidget.pivot.X - 1.0f);
					SetAnchorOffsetX(anchorOffsetX);
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
					selfLeft = newLocation.X + widget.width * -widget.pivot.X;
					selfRight = newLocation.X + widget.width * (1.0f - widget.pivot.X);
					//stretch
					SetHorizontalStretch(FVector2D(selfLeft - parentLeft, parentRight - selfRight));
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
					float anchorOffsetY = this->RelativeLocation.Y + parentWidget.height * (parentWidget.pivot.Y - 1.0f);
					SetAnchorOffsetY(anchorOffsetY);
				}
				break;
				case UIAnchorVerticalAlign::Middle:
				{
					float anchorOffsetY = this->RelativeLocation.Y + parentWidget.height * (parentWidget.pivot.Y - 0.5f);
					SetAnchorOffsetY(anchorOffsetY);
				}
				break;
				case UIAnchorVerticalAlign::Bottom:
				{
					float anchorOffsetY = this->RelativeLocation.Y + parentWidget.height * parentWidget.pivot.Y;
					SetAnchorOffsetY(anchorOffsetY);
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
					selfBottom = newLocation.Y + widget.height * -widget.pivot.Y;
					selfTop = newLocation.Y + widget.height * (1.0f - widget.pivot.Y);
					//stretch
					SetVerticalStretch(FVector2D(selfBottom - parentBottom, parentTop - selfTop));
				}
				break;
				}
			}
		}
	}
}
void UUIItem::SetStretchLeft(float newLeft)
{
	if (IsFloatNotEqual(widget.stretchLeft, newLeft))
	{
		MarkVertexPositionDirty();
		widget.stretchLeft = newLeft;
		if (cacheParentUIItem)
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			CallUIComponentsDimensionsChanged(true, sizeChanged);
		}
		else
		{
			CallUIComponentsDimensionsChanged(false, false);
		}
	}
}
void UUIItem::SetStretchRight(float newRight)
{
	if (IsFloatNotEqual(widget.stretchRight, newRight))
	{
		MarkVertexPositionDirty();
		widget.stretchRight = newRight;
		if (cacheParentUIItem)
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			CallUIComponentsDimensionsChanged(true, sizeChanged);
		}
		else
		{
			CallUIComponentsDimensionsChanged(false, false);
		}
	}
}
void UUIItem::SetHorizontalStretch(FVector2D newStretch)
{
	if (IsFloatNotEqual(widget.stretchLeft, newStretch.X) || IsFloatNotEqual(widget.stretchRight, newStretch.Y))
	{
		MarkVertexPositionDirty();
		widget.stretchLeft = newStretch.X;
		widget.stretchRight = newStretch.Y;
		if (cacheParentUIItem)
		{
			bool sizeChanged = CalculateHorizontalAnchorAndSizeFromStretch();
			CallUIComponentsDimensionsChanged(true, sizeChanged);
		}
		else
		{
			CallUIComponentsDimensionsChanged(false, false);
		}
	}
}
void UUIItem::SetStretchTop(float newTop)
{
	if (IsFloatNotEqual(widget.stretchTop, newTop))
	{
		MarkVertexPositionDirty();
		widget.stretchTop = newTop;
		if (cacheParentUIItem)
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			CallUIComponentsDimensionsChanged(true, sizeChanged);
		}
		else
		{
			CallUIComponentsDimensionsChanged(false, false);
		}
	}
}
void UUIItem::SetStretchBottom(float newBottom)
{
	if (IsFloatNotEqual(widget.stretchBottom, newBottom))
	{
		MarkVertexPositionDirty();
		widget.stretchBottom = newBottom;
		if (cacheParentUIItem)
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			CallUIComponentsDimensionsChanged(true, sizeChanged);
		}
		else
		{
			CallUIComponentsDimensionsChanged(false, false);
		}
	}
}
void UUIItem::SetVerticalStretch(FVector2D newStretch)
{
	if (IsFloatNotEqual(widget.stretchBottom, newStretch.X) || IsFloatNotEqual(widget.stretchTop, newStretch.Y))
	{
		MarkVertexPositionDirty();
		widget.stretchBottom = newStretch.X;
		widget.stretchTop = newStretch.Y;
		if (cacheParentUIItem)
		{
			bool sizeChanged = CalculateVerticalAnchorAndSizeFromStretch();
			CallUIComponentsDimensionsChanged(true, sizeChanged);
		}
		else
		{
			CallUIComponentsDimensionsChanged(false, false);
		}
	}
}

void UUIItem::SetPivot(FVector2D pivot) {
	if (IsVector2NotEqual(widget.pivot, pivot))
	{
		MarkVertexPositionDirty();
		widget.pivot = pivot;
	}
}
void UUIItem::SetAnchorHAlign(UIAnchorHorizontalAlign align, bool keepRelativeLocation)
{
	if (widget.anchorHAlign != align)
	{
		if (cacheParentUIItem)
		{
			if (keepRelativeLocation)
			{
				//calculate to keep relative position
				//anchor convert from other to center
				float halfWidth = cacheParentUIItem->widget.width * 0.5f;
				switch (widget.anchorHAlign)
				{
				case UIAnchorHorizontalAlign::Left:
				{
					widget.anchorOffsetX -= halfWidth;
				}
				break;
				case UIAnchorHorizontalAlign::Right:
				{
					widget.anchorOffsetX += halfWidth;
				}
				break;
				}
				//anchor convert from center to other
				switch (align)
				{
				case UIAnchorHorizontalAlign::Left:
				{
					widget.anchorOffsetX += halfWidth;
				}
				break;
				case UIAnchorHorizontalAlign::Right:
				{
					widget.anchorOffsetX -= halfWidth;
				}
				break;
				}
			}

			MarkVertexPositionDirty();
		}
		widget.anchorHAlign = align;
	}
#if WITH_EDITORONLY_DATA
	prevAnchorHAlign = align;
#endif
}
void UUIItem::SetAnchorVAlign(UIAnchorVerticalAlign align, bool keepRelativeLocation)
{
	if (widget.anchorVAlign != align)
	{
		if (cacheParentUIItem)
		{
			if (keepRelativeLocation)
			{
				//calculate to keep relative position
				//anchor convert from other to center
				float halfHeight = cacheParentUIItem->widget.height * 0.5f;
				switch (widget.anchorVAlign)
				{
				case UIAnchorVerticalAlign::Bottom:
				{
					widget.anchorOffsetY -= halfHeight;
				}
				break;
				case UIAnchorVerticalAlign::Top:
				{
					widget.anchorOffsetY += halfHeight;
				}
				break;
				}
				//anchor convert from center to other
				switch (align)
				{
				case UIAnchorVerticalAlign::Bottom:
				{
					widget.anchorOffsetY += halfHeight;
				}
				break;
				case UIAnchorVerticalAlign::Top:
				{
					widget.anchorOffsetY -= halfHeight;
				}
				break;
				}
			}

			MarkVertexPositionDirty();
		}
		widget.anchorVAlign = align;
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
	if (cacheParentUIItem == nullptr)
	{
		if (auto parent = GetAttachParent())
		{
			cacheParentUIItem = Cast<UUIItem>(parent);
		}
	}
	return cacheParentUIItem;
}

void UUIItem::MarkVertexPositionDirty() 
{ 
	bVertexPositionChanged = true; 
	MarkPanelUpdate();
}
void UUIItem::MarkColorDirty() 
{ 
	bColorChanged = true;
	MarkPanelUpdate();
}
void UUIItem::MarkPanelUpdate()
{
	if (CheckRenderUIPanel()) RenderUIPanel->MarkNeedUpdate();
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
	if (RenderUIPanel == nullptr)return false;
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


FColor UUIItem::GetFinalColor()const
{
	return FColor(widget.color.R, widget.color.G, widget.color.B, inheritAlpha ? widget.color.A * calculatedParentAlpha : widget.color.A);
}

uint8 UUIItem::GetFinalAlpha()const
{
	return inheritAlpha ? (uint8)(widget.color.A * calculatedParentAlpha) : widget.color.A;
}

#pragma region InteractionGroup
bool UUIItem::IsGroupAllowInteraction()
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
	if (auto ownerActor = GetOwner())
	{
		TArray<AActor*> ChildrenActor;
		ownerActor->GetAttachedActors(ChildrenActor);
		for (auto childActor : ChildrenActor)
		{
			if (auto childUIItem = Cast<UUIItem>(childActor->GetRootComponent()))
			{
				childUIItem->allUpParentGroupAllowInteraction = InParentInteractable;
				childUIItem->SetInteractionGroupStateChange();
			}
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
	auto& children = this->GetAttachChildren();
	for (auto child : children)
	{
		if (UUIItem* uiChild = Cast<UUIItem>(child))
		{
			//state is changed
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
			if (cacheParentUIItem != nullptr)cacheParentUIItem->OnChildActiveStateChanged(this);
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
		auto displayName = ownerActor->GetActorLabel();
		FString prefix("//");
		if (IsUIActiveInHierarchy() && displayName.StartsWith(prefix))
		{
			displayName = displayName.Right(displayName.Len() - prefix.Len());
			ownerActor->SetActorLabel(displayName);
		}
		else if (!IsUIActiveInHierarchy() && !displayName.StartsWith(prefix))
		{
			displayName = prefix.Append(displayName);
			ownerActor->SetActorLabel(displayName);
		}
	}
#endif
	bVertexPositionChanged = true;
	bColorChanged = true;
	bDepthChanged = true;
	bTransformChanged = true;
	//panel update
	MarkPanelUpdate();
	//callback
	CallUIComponentsActiveInHierarchyStateChanged();
}

#pragma endregion UIActive

bool UUIItem::IsFloatNotEqual(float a, float b)
{
	return FMath::Abs(a - b) > KINDA_SMALL_NUMBER;
}
bool UUIItem::IsVectorNotEqual(FVector a, FVector b)
{
	return FMath::Abs(a.X - b.X) > KINDA_SMALL_NUMBER
		|| FMath::Abs(a.Y - b.Y) > KINDA_SMALL_NUMBER
		|| FMath::Abs(a.Z - b.Z) > KINDA_SMALL_NUMBER
		;
}
bool UUIItem::IsVector2NotEqual(FVector2D a, FVector2D b)
{
	return FMath::Abs(a.X - b.X) > KINDA_SMALL_NUMBER
		|| FMath::Abs(a.Y - b.Y) > KINDA_SMALL_NUMBER
		;
}

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
	bSelectable = true;
}
#if WITH_EDITOR
FPrimitiveSceneProxy* UUIItemEditorHelperComp::CreateSceneProxy()
{
	return nullptr;
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
			const auto& widget = Component->GetWidget();
			auto worldTransform = Component->GetComponentTransform();
			FVector relativeOffset(0, 0, 0);
			relativeOffset.X = (0.5f - widget.pivot.X) * widget.width;
			relativeOffset.Y = (0.5f - widget.pivot.Y) * widget.height;
			auto worldLocation = worldTransform.TransformPosition(relativeOffset);
			//calculate world location
			auto parentUIItem = Component->GetParentAsUIItem();
			if (parentUIItem != nullptr)
			{
				FVector relativeLocation = Component->RelativeLocation;
				const auto& parentWidget = parentUIItem->GetWidget();
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
				FTransform::Multiply(&calculatedWorldTf, &relativeTf, &(parentUIItem->GetComponentTransform()));
				worldLocation = calculatedWorldTf.TransformPosition(relativeOffset);
			}

			auto extends = FVector(widget.width, widget.height, 0) * 0.5f;
			for (int32 ViewIndex = 0; ViewIndex < Views.Num(); ViewIndex++)
			{
				auto& View = Views[ViewIndex];
				if (VisibilityMap & (1 << ViewIndex))
				{
					FColor DrawColor = FColor(128, 128, 128);//gray means normal object
					//if (IsSelected())//select self
					{
						DrawColor = FColor(0, 255, 0);//green means selected object
						extends += FVector(0, 0, 1);

						FPrimitiveDrawInterface* PDI = Collector.GetPDI(ViewIndex);
						//DrawOrientedWireBox(PDI, worldLocation, worldTransform.GetScaledAxis(EAxis::X), worldTransform.GetScaledAxis(EAxis::Y), worldTransform.GetScaledAxis(EAxis::Z), extends, DrawColor, SDPG_World);
						DrawDebugSolidBox(Component->GetWorld(), worldLocation, extends, worldTransform.GetRotation(), DrawColor);
						//DrawBox(PDI, worldTransform.ToMatrixWithScale(), extends, GEngine->ConstraintLimitMaterialPrismatic->GetRenderProxy(false), SDPG_Foreground);
					}
				}
			}
		}

		virtual FPrimitiveViewRelevance GetViewRelevance(const FSceneView* View) const override
		{
			FPrimitiveViewRelevance Result;
			bool needToShow = IsShown(View)
				&& Component->IsUIActiveInHierarchy()
				&& Component->GetWorld() != nullptr
				&& Component->GetWorld()->IsGameWorld();
			Result.bDrawRelevance = needToShow;
			Result.bDynamicRelevance = true;
			Result.bShadowRelevance = IsShadowCast(View);
			Result.bEditorPrimitiveRelevance = UseEditorCompositing(View);
			return Result;
		}
		virtual uint32 GetMemoryFootprint(void) const override { return(sizeof(*this) + GetAllocatedSize()); }
		uint32 GetAllocatedSize(void) const { return(FPrimitiveSceneProxy::GetAllocatedSize()); }
	private:
		UUIItem* Component;
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
	if (BodySetup == nullptr)
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
	auto widget = Parent->GetWidget();
	auto origin = FVector(widget.width * (0.5f - widget.pivot.X), widget.height * (0.5f - widget.pivot.Y), 0);
	return FBoxSphereBounds(origin, FVector(widget.width * 0.5f, widget.height * 0.5f, 1), (widget.width > widget.height ? widget.width : widget.height) * 0.5f).TransformBy(LocalToWorld);
}