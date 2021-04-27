// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewComponent.h"
#include "LGUI.h"
#include "LTweenActor.h"
#include "Core/Actor/UIBaseActor.h"
#include "Utils/LGUIUtils.h"

void UUIScrollViewHelper::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        if (sizeChanged)
            TargetComp->RecalculateRange();
    }
}
void UUIScrollViewHelper::OnUIChildDimensionsChanged(UUIItem *child, bool positionChanged, bool sizeChanged)
{
    Super::OnUIChildDimensionsChanged(child, positionChanged, sizeChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        if (sizeChanged)
            TargetComp->RecalculateRange();
    }
}

void UUIScrollViewComponent::Awake()
{
    Super::Awake();
    RecalculateRange();
}

void UUIScrollViewComponent::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    if (CanUpdateAfterDrag)
        UpdateAfterDrag(DeltaTime);
}

#if WITH_EDITOR
void UUIScrollViewComponent::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    RecalculateRange();
}
#endif

void UUIScrollViewComponent::RecalculateRange()
{
    if (CheckParameters())
    {
        if (Horizontal)
        {
            this->CalculateHorizontalRange();
            AllowHorizontalScroll = true;
        }
        else
        {
            AllowHorizontalScroll = false;
        }
        if (Vertical)
        {
            this->CalculateVerticalRange();
            AllowVerticalScroll = true;
        }
        else
        {
            AllowVerticalScroll = false;
        }
        Position = ContentUIItem->GetRelativeLocation();
        if (Position.X < HorizontalRange.X || Position.X > HorizontalRange.Y || Position.Y < VerticalRange.X || Position.Y > VerticalRange.Y)
        {
            CanUpdateAfterDrag = true;
        }
        else
        {
            UpdateProgress(false);
        }
    }
}
void UUIScrollViewComponent::OnUIActiveInHierachy(bool ativeOrInactive)
{
    Super::OnUIActiveInHierachy(ativeOrInactive);
    if (ativeOrInactive)
    {
        RecalculateRange();
    }
}
void UUIScrollViewComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    if (sizeChanged)
        RecalculateRange();
}

bool UUIScrollViewComponent::CheckParameters()
{
    if (ContentUIItem.IsValid() && ContentParentUIItem.IsValid() && RootUIComp.IsValid())
        return true;
    if (!Content.IsValid())
        return false;
    if (Content->GetAttachParentActor() == nullptr)
        return false;
    auto contentParentActor = Cast<AUIBaseActor>(Content->GetAttachParentActor());
    if (contentParentActor == nullptr)
        return false;
    ContentUIItem = Content->GetUIItem();
    ContentParentUIItem = contentParentActor->GetUIItem();
    if (ContentParentUIItem != nullptr)
    {
        auto contentParentHelperComp = NewObject<UUIScrollViewHelper>(contentParentActor);
        contentParentHelperComp->TargetComp = this;
        contentParentHelperComp->RegisterComponent();
    }
    CheckRootUIComponent();
    if (ContentUIItem.IsValid() && ContentParentUIItem.IsValid() && RootUIComp.IsValid())
        return true;
    return false;
}

bool UUIScrollViewComponent::CheckValidHit(USceneComponent *InHitComp)
{
    return (InHitComp->IsAttachedTo(RootUIComp.Get()) || InHitComp == RootUIComp); //make sure hit component is child of this or is this
}

bool UUIScrollViewComponent::OnPointerDown_Implementation(ULGUIPointerEventData *eventData)
{
    PrevWorldPoint = eventData->pressWorldPoint;
    return AllowEventBubbleUp;
}
bool UUIScrollViewComponent::OnPointerUp_Implementation(ULGUIPointerEventData *eventData)
{
    return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData *eventData)
{
    if (CheckParameters() && CheckValidHit(eventData->dragComponent))
    {
        auto worldPoint = eventData->GetWorldPointInPlane();
        const auto localMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(worldPoint - PrevWorldPoint);
        PrevWorldPoint = worldPoint;
        if (OnlyOneDirection && Horizontal && Vertical)
        {
            if (FMath::Abs(localMoveDelta.X) > FMath::Abs(localMoveDelta.Y))
            {
                AllowHorizontalScroll = true;
            }
            else
            {
                AllowVerticalScroll = true;
            }
        }
        else
        {
            if (Horizontal)
            {
                AllowHorizontalScroll = true;
            }
            if (Vertical)
            {
                AllowVerticalScroll = true;
            }
        }
        Position = ContentUIItem->GetRelativeLocation();
        CanUpdateAfterDrag = false;
        OnPointerDrag_Implementation(eventData);
    }
    else
    {
        AllowHorizontalScroll = AllowVerticalScroll = false;
    }
    return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerDrag_Implementation(ULGUIPointerEventData *eventData)
{
    if (!ContentUIItem.IsValid())
        return AllowEventBubbleUp;
    auto position = ContentUIItem->GetRelativeLocation();
    auto worldPoint = eventData->GetWorldPointInPlane();
    auto localMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(worldPoint - PrevWorldPoint);
    localMoveDelta.Z = 0;
    PrevWorldPoint = worldPoint;
    if (AllowHorizontalScroll)
    {
        auto predict = position.X + localMoveDelta.X;
        if (predict < HorizontalRange.X || predict > HorizontalRange.Y) //out-of-range, lower the sentitivity
        {
            position.X += localMoveDelta.X * 0.5f;
        }
        else
        {
            position.X = predict;
        }
        CanUpdateAfterDrag = false;
        ContentUIItem->SetRelativeLocation(position);
        UpdateProgress();
    }
    if (AllowVerticalScroll)
    {
        auto predict = position.Y + localMoveDelta.Y;
        if (predict < VerticalRange.X || predict > VerticalRange.Y)
        {
            position.Y += localMoveDelta.Y * 0.5f;
        }
        else
        {
            position.Y = predict;
        }
        CanUpdateAfterDrag = false;
        ContentUIItem->SetRelativeLocation(position);
        UpdateProgress();
    }
    return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData *eventData)
{
    auto worldPoint = eventData->GetWorldPointInPlane();
    const auto localMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(worldPoint - PrevWorldPoint);
    PrevWorldPoint = worldPoint;
    if (AllowHorizontalScroll)
    {
        CanUpdateAfterDrag = true;
        Position = ContentUIItem->GetRelativeLocation();
        DragSpeed.X = localMoveDelta.X;
    }
    if (AllowVerticalScroll)
    {
        CanUpdateAfterDrag = true;
        Position = ContentUIItem->GetRelativeLocation();
        DragSpeed.Y = localMoveDelta.Y;
    }
    return AllowEventBubbleUp;
}
bool UUIScrollViewComponent::OnPointerScroll_Implementation(ULGUIPointerEventData *eventData)
{
    if (CheckParameters() && CheckValidHit(eventData->enterComponent))
    {
        if (eventData->scrollAxisValue != 0)
        {
            auto delta = eventData->scrollAxisValue * -ScrollSensitivity;
            if (Horizontal)
            {
                AllowHorizontalScroll = true;
                CanUpdateAfterDrag = true;
                if (Position.X < HorizontalRange.X || Position.X > HorizontalRange.Y)
                {
                    Position.X += delta * 0.5f;
                    DragSpeed.X = delta * 0.5f;
                }
                else
                {
                    Position.X += delta;
                    DragSpeed.X = delta;
                }
                ContentUIItem->SetRelativeLocation(Position);
            }
            if (Vertical)
            {
                AllowVerticalScroll = true;
                CanUpdateAfterDrag = true;
                if (Position.Y < VerticalRange.X || Position.Y > VerticalRange.Y)
                {
                    Position.Y += delta * 0.5f;
                    DragSpeed.Y = delta * 0.5f;
                }
                else
                {
                    Position.Y += delta;
                    DragSpeed.Y = delta;
                }
                ContentUIItem->SetRelativeLocation(Position);
            }
        }
    }
    return AllowEventBubbleUp;
}
#define POSITION_THRESHOLD 0.001f
void UUIScrollViewComponent::UpdateAfterDrag(float deltaTime)
{
    if (FMath::Abs(DragSpeed.X) > 0 || FMath::Abs(DragSpeed.Y) > 0                                       //speed large then threshold
        || (AllowHorizontalScroll && (Position.X < HorizontalRange.X || Position.X > HorizontalRange.Y)) //horizontal out of range
        || (AllowVerticalScroll && (Position.Y < VerticalRange.X || Position.Y > VerticalRange.Y)))      //vertical out of range
    {
        bool canMove = false;
        const float positionTimeMultiply = 50.0f;
        const float dragForceMulitply = 10.0f;
        const float positionLerpTimeMultiply = 10.0f;
        if (AllowHorizontalScroll)
        {
            if (Position.X - HorizontalRange.X < 0)
            {
                if (DragSpeed.X < 0)
                {
                    float dragForce = (HorizontalRange.X - Position.X) * dragForceMulitply;
                    DragSpeed.X += -FMath::Sign(DragSpeed.X) * dragForce * deltaTime;

                    Position.X += DragSpeed.X * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    DragSpeed.X = 0;
                    if (FMath::Abs(HorizontalRange.X - Position.X) < POSITION_THRESHOLD)
                    {
                        Position.X = HorizontalRange.X;
                    }
                    else
                    {
                        Position.X = FMath::Lerp(Position.X, HorizontalRange.X, positionLerpTimeMultiply * deltaTime);
                    }
                    canMove = true;
                }
            }
            else if (Position.X - HorizontalRange.Y > 0)
            {
                if (DragSpeed.X > 0) //move right, use opposite force
                {
                    float dragForce = (Position.X - HorizontalRange.Y) * dragForceMulitply;
                    DragSpeed.X += -FMath::Sign(DragSpeed.X) * dragForce * deltaTime;

                    Position.X += DragSpeed.X * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    DragSpeed.X = 0;
                    if (FMath::Abs(Position.X - HorizontalRange.Y) < POSITION_THRESHOLD)
                    {
                        Position.X = HorizontalRange.Y;
                    }
                    else
                    {
                        Position.X = FMath::Lerp(Position.X, HorizontalRange.Y, positionLerpTimeMultiply * deltaTime);
                    }
                    canMove = true;
                }
            }
            else
            {
                auto speedXDir = FMath::Sign(DragSpeed.X);
                float dragForce = 10;
                DragSpeed.X += -FMath::Sign(DragSpeed.X) * dragForce * deltaTime;
                if (FMath::Sign(DragSpeed.X) != speedXDir) //accelerate speed change speed direction, set speed to 0
                {
                    DragSpeed.X = 0;
                }
                Position.X += DragSpeed.X * deltaTime * positionTimeMultiply;
                canMove = true;
            }
        }
        if (AllowVerticalScroll)
        {
            if (Position.Y - VerticalRange.X < 0)
            {
                if (DragSpeed.Y < 0)
                {
                    float dragForce = (VerticalRange.X - Position.Y) * dragForceMulitply;
                    DragSpeed.Y += -FMath::Sign(DragSpeed.Y) * dragForce * deltaTime;

                    Position.Y += DragSpeed.Y * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    DragSpeed.Y = 0;
                    if (FMath::Abs(VerticalRange.X - Position.Y) < POSITION_THRESHOLD)
                    {
                        Position.Y = VerticalRange.X;
                    }
                    else
                    {
                        Position.Y = FMath::Lerp(Position.Y, VerticalRange.X, positionLerpTimeMultiply * deltaTime);
                    }
                    canMove = true;
                }
            }
            else if (Position.Y - VerticalRange.Y > 0)
            {
                if (DragSpeed.Y > 0) //move up, use opposite force
                {
                    float dragForce = (Position.Y - VerticalRange.Y) * dragForceMulitply;
                    DragSpeed.Y += -FMath::Sign(DragSpeed.Y) * dragForce * deltaTime;

                    Position.Y += DragSpeed.Y * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    DragSpeed.Y = 0;
                    if (FMath::Abs(Position.Y - VerticalRange.Y) < POSITION_THRESHOLD)
                    {
                        Position.Y = VerticalRange.Y;
                    }
                    else
                    {
                        Position.Y = FMath::Lerp(Position.Y, VerticalRange.Y, positionLerpTimeMultiply * deltaTime);
                    }
                    canMove = true;
                }
            }
            else
            {
                auto speedYDir = FMath::Sign(DragSpeed.Y);
                float dragForce = 10;
                DragSpeed.Y += -FMath::Sign(DragSpeed.Y) * dragForce * deltaTime;
                if (FMath::Sign(DragSpeed.Y) != speedYDir) //accelerate speed change speed direction, set speed to 0
                {
                    DragSpeed.Y = 0;
                }
                Position.Y += DragSpeed.Y * deltaTime * positionTimeMultiply;
                canMove = true;
            }
        }
        if (canMove)
        {
            UpdateProgress();
            ContentUIItem->SetRelativeLocation(Position);
        }
    }
    else
    {
        CanUpdateAfterDrag = false;
    }
}
void UUIScrollViewComponent::UpdateProgress(bool InFireEvent)
{
    if (!ContentUIItem.IsValid())
        return;
    auto relativeLocation = ContentUIItem->GetRelativeLocation();
    if (AllowHorizontalScroll)
    {
        Progress.X = (relativeLocation.X - HorizontalRange.X) / (HorizontalRange.Y - HorizontalRange.X);
    }
    if (AllowVerticalScroll)
    {
        Progress.Y = (relativeLocation.Y - VerticalRange.X) / (VerticalRange.Y - VerticalRange.X);
    }
    if (InFireEvent)
    {
        if (OnScrollCPP.IsBound())
            OnScrollCPP.Broadcast(Progress);
        OnScroll.FireEvent(Progress);
    }
}

void UUIScrollViewComponent::CalculateHorizontalRange()
{
    auto &parentWidget = ContentParentUIItem->GetWidget();
    auto &contentWidget = ContentUIItem->GetWidget();
    if (parentWidget.width > contentWidget.width)
    {
        if (CanScrollInSmallSize)
        {
            //parent
            HorizontalRange.X = -parentWidget.pivot.X * parentWidget.width;
            HorizontalRange.Y = (1.0f - parentWidget.pivot.X) * parentWidget.width;
            //self
            HorizontalRange.X += contentWidget.pivot.X * contentWidget.width;
            HorizontalRange.Y += (contentWidget.pivot.X - 1.0f) * contentWidget.width;
        }
        else
        {
			//parent
			HorizontalRange.X = -parentWidget.pivot.X * parentWidget.width;
			HorizontalRange.Y = (1.0f - parentWidget.pivot.X) * parentWidget.width - (parentWidget.width - contentWidget.width);
			//self
			HorizontalRange.X += contentWidget.pivot.X * contentWidget.width;
			HorizontalRange.Y += (contentWidget.pivot.X - 1.0f) * contentWidget.width;
        }
    }
    else
    {
        //self
        HorizontalRange.X = (contentWidget.pivot.X - 1.0f) * contentWidget.width;
        HorizontalRange.Y = contentWidget.pivot.X * contentWidget.width;
        //parent
        HorizontalRange.X += (1.0f - parentWidget.pivot.X) * parentWidget.width;
        HorizontalRange.Y += -parentWidget.pivot.X * parentWidget.width;
    }
}
void UUIScrollViewComponent::CalculateVerticalRange()
{
    auto &parentWidget = ContentParentUIItem->GetWidget();
    auto &contentWidget = ContentUIItem->GetWidget();
    if (parentWidget.height > contentWidget.height)
    {
        if (CanScrollInSmallSize)
        {
            //parent
            VerticalRange.X = -parentWidget.pivot.Y * parentWidget.height;
            VerticalRange.Y = (1.0f - parentWidget.pivot.Y) * parentWidget.height;
            //self
            VerticalRange.X += contentWidget.pivot.Y * contentWidget.height;
            VerticalRange.Y += (contentWidget.pivot.Y - 1.0f) * contentWidget.height;
        }
        else
        {
			//parent
			VerticalRange.X = -parentWidget.pivot.Y * parentWidget.height + (parentWidget.height - contentWidget.height);
			VerticalRange.Y = (1.0f - parentWidget.pivot.Y) * parentWidget.height;
			//self
			VerticalRange.X += contentWidget.pivot.Y * contentWidget.height;
			VerticalRange.Y += (contentWidget.pivot.Y - 1.0f) * contentWidget.height;
        }
    }
    else
    {
        //self
        VerticalRange.X = (contentWidget.pivot.Y - 1.0f) * contentWidget.height;
        VerticalRange.Y = contentWidget.pivot.Y * contentWidget.height;
        //parent
        VerticalRange.X += (1.0f - parentWidget.pivot.Y) * parentWidget.height;
        VerticalRange.Y += -parentWidget.pivot.Y * parentWidget.height;
    }
}
void UUIScrollViewComponent::RectRangeChanged()
{
    if (Horizontal)
        CalculateHorizontalRange();
    if (Vertical)
        CalculateVerticalRange();
}

FDelegateHandle UUIScrollViewComponent::RegisterScrollEvent(const FLGUIVector2Delegate &InDelegate)
{
    return OnScrollCPP.Add(InDelegate);
}
FDelegateHandle UUIScrollViewComponent::RegisterScrollEvent(const TFunction<void(FVector2D)> &InFunction)
{
    return OnScrollCPP.AddLambda(InFunction);
}
void UUIScrollViewComponent::UnregisterScrollEvent(const FDelegateHandle &InHandle)
{
    OnScrollCPP.Remove(InHandle);
}

FLGUIDelegateHandleWrapper UUIScrollViewComponent::RegisterScrollEvent(const FLGUIScrollViewDynamicDelegate &InDelegate)
{
    auto delegateHandle = OnScrollCPP.AddLambda([InDelegate](FVector2D InProgress) {
        if (InDelegate.IsBound())
            InDelegate.Execute(InProgress);
    });
    return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIScrollViewComponent::UnregisterScrollEvent(const FLGUIDelegateHandleWrapper &InDelegateHandle)
{
    OnScrollCPP.Remove(InDelegateHandle.DelegateHandle);
}

void UUIScrollViewComponent::SetHorizontal(bool value)
{
    if (Horizontal != value)
    {
        Horizontal = value;
        RecalculateRange();
    }
}
void UUIScrollViewComponent::SetVertical(bool value)
{
	if (Vertical != value)
	{
        Vertical = value;
		RecalculateRange();
	}
}
void UUIScrollViewComponent::SetOnlyOneDirection(bool value)
{
	if (OnlyOneDirection != value)
	{
        OnlyOneDirection = value;
	}
}
void UUIScrollViewComponent::SetScrollSensitivity(float value)
{
    if (ScrollSensitivity != value)
    {
        ScrollSensitivity = value;
    }
}
void UUIScrollViewComponent::SetCanScrollInSmallSize(bool value)
{
    if (CanScrollInSmallSize != value)
    {
        CanScrollInSmallSize = value;
        RecalculateRange();
    }
}
