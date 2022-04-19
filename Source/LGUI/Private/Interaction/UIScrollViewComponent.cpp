﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewComponent.h"
#include "LGUI.h"
#include "LTweenActor.h"
#include "Core/Actor/UIBaseActor.h"
#include "Utils/LGUIUtils.h"

void UUIScrollViewHelper::Awake()
{
    Super::Awake();
    this->SetCanExecuteUpdate(false);
}
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
        {
            TargetComp->bRangeCalculated = false;
            TargetComp->RecalculateRange();
        }
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
        {
            TargetComp->bRangeCalculated = false;
            TargetComp->RecalculateRange();
        }
    }
}

void UUIScrollViewComponent::Awake()
{
    Super::Awake();
    bRangeCalculated = false;
    RecalculateRange();
    this->SetCanExecuteUpdate(true);
}

void UUIScrollViewComponent::Update(float DeltaTime)
{
    Super::Update(DeltaTime);
    if (bCanUpdateAfterDrag)
        UpdateAfterDrag(DeltaTime);
}

#if WITH_EDITOR
void UUIScrollViewComponent::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    bRangeCalculated = false;
    RecalculateRange();
}
#endif

void UUIScrollViewComponent::RecalculateRange()
{
    if (bRangeCalculated)return;
    if (CheckParameters())
    {
        if (Horizontal)
        {
            this->CalculateHorizontalRange();
            bAllowHorizontalScroll = true;
        }
        else
        {
            bAllowHorizontalScroll = false;
        }
        if (Vertical)
        {
            this->CalculateVerticalRange();
            bAllowVerticalScroll = true;
        }
        else
        {
            bAllowVerticalScroll = false;
        }
        Position = ContentUIItem->GetRelativeLocation();
        if (
            (bAllowHorizontalScroll && (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y))
            || (bAllowVerticalScroll && (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y))
            )
        {
            bCanUpdateAfterDrag = true;
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
        bRangeCalculated = false;
        RecalculateRange();
    }
}
void UUIScrollViewComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
    Super::OnUIDimensionsChanged(positionChanged, sizeChanged);
    if (sizeChanged)
    {
        bRangeCalculated = false;
        RecalculateRange();
    }
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
        bAllowHorizontalScroll = false;
        bAllowVerticalScroll = false;
        if (OnlyOneDirection && Horizontal && Vertical)
        {
            if (FMath::Abs(localMoveDelta.Y) > FMath::Abs(localMoveDelta.Z))
            {
                bAllowHorizontalScroll = true;
            }
            else
            {
                bAllowVerticalScroll = true;
            }
        }
        else
        {
            if (Horizontal)
            {
                bAllowHorizontalScroll = true;
            }
            if (Vertical)
            {
                bAllowVerticalScroll = true;
            }
        }
        Position = ContentUIItem->GetRelativeLocation();
        bCanUpdateAfterDrag = false;
        OnPointerDrag_Implementation(eventData);
    }
    else
    {
        bAllowHorizontalScroll = bAllowVerticalScroll = false;
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
    localMoveDelta.X = 0;
    PrevWorldPoint = worldPoint;
    if (bAllowHorizontalScroll)
    {
        auto predict = position.Y + localMoveDelta.Y;
        if (predict < HorizontalRange.X || predict > HorizontalRange.Y) //out-of-range, lower the sentitivity
        {
            position.Y += localMoveDelta.Y * 0.5f;
        }
        else
        {
            position.Y = predict;
        }
        bCanUpdateAfterDrag = false;
        ContentUIItem->SetRelativeLocation(position);
        UpdateProgress();
    }
    if (bAllowVerticalScroll)
    {
        auto predict = position.Z + localMoveDelta.Z;
        if (predict < VerticalRange.X || predict > VerticalRange.Y)
        {
            position.Z += localMoveDelta.Z * 0.5f;
        }
        else
        {
            position.Z = predict;
        }
        bCanUpdateAfterDrag = false;
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
    if (bAllowHorizontalScroll)
    {
        bCanUpdateAfterDrag = true;
        Position = ContentUIItem->GetRelativeLocation();
        Velocity.X = localMoveDelta.Y;
    }
    if (bAllowVerticalScroll)
    {
        bCanUpdateAfterDrag = true;
        Position = ContentUIItem->GetRelativeLocation();
        Velocity.Y = localMoveDelta.Z;
    }
    return AllowEventBubbleUp;
}
bool UUIScrollViewComponent::OnPointerScroll_Implementation(ULGUIPointerEventData *eventData)
{
    if (CheckParameters() && CheckValidHit(eventData->enterComponent))
    {
        if (eventData->scrollAxisValue != FVector2D::ZeroVector)
        {
            bAllowHorizontalScroll = false;
            bAllowVerticalScroll = false;
            if (OnlyOneDirection && Horizontal && Vertical)
            {
                if (FMath::Abs(eventData->scrollAxisValue.X) > FMath::Abs(eventData->scrollAxisValue.Y))
                {
                    bAllowHorizontalScroll = true;
                }
                else
                {
                    bAllowVerticalScroll = true;
                }
            }
            else
            {
                if (Horizontal)
                {
                    bAllowHorizontalScroll = true;
                }
                if (Vertical)
                {
                    bAllowVerticalScroll = true;
                }
            }

            if (bAllowHorizontalScroll)
            {
                auto delta = eventData->scrollAxisValue.X * -ScrollSensitivity;
                bCanUpdateAfterDrag = true;
                if (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y)
                {
                    Position.Y += delta * 0.5f;
                    Velocity.X = delta * 0.5f;
                }
                else
                {
                    Position.Y += delta;
                    Velocity.X = delta;
                }
                ContentUIItem->SetRelativeLocation(Position);
            }
            if (bAllowVerticalScroll)
            {
                auto delta = eventData->scrollAxisValue.Y * -ScrollSensitivity;
                bCanUpdateAfterDrag = true;
                if (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y)
                {
                    Position.Z += delta * 0.5f;
                    Velocity.Y = delta * 0.5f;
                }
                else
                {
                    Position.Z += delta;
                    Velocity.Y = delta;
                }
                ContentUIItem->SetRelativeLocation(Position);
            }
        }
    }
    return AllowEventBubbleUp;
}

void UUIScrollViewComponent::SetVelocity(const FVector2D& value)
{
    if (CheckParameters())
    {
        Velocity = value;
        bCanUpdateAfterDrag = true;
    }
}

void UUIScrollViewComponent::SetScrollDelta(FVector2D value)
{
    if (CheckParameters())
    {
        auto delta = value;
		if (Horizontal)
		{
			bAllowHorizontalScroll = true;
			bCanUpdateAfterDrag = true;
			if (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y)
			{
				Position.Y += delta.X * 0.5f;
				Velocity.X = delta.X * 0.5f;
			}
			else
			{
				Position.Y += delta.X;
				Velocity.X = delta.X;
			}
			ContentUIItem->SetRelativeLocation(Position);
		}
		if (Vertical)
		{
			bAllowVerticalScroll = true;
			bCanUpdateAfterDrag = true;
			if (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y)
			{
				Position.Z += delta.Y * 0.5f;
				Velocity.Y = delta.Y * 0.5f;
			}
			else
			{
				Position.Z += delta.Y;
				Velocity.Y = delta.Y;
			}
			ContentUIItem->SetRelativeLocation(Position);
		}
    }
}
void UUIScrollViewComponent::SetScrollValue(FVector2D value)
{
    if (CheckParameters())
    {
		if (Horizontal)
		{
			bAllowHorizontalScroll = true;
			bCanUpdateAfterDrag = true;
			Position.Y = value.X;
			Velocity.X = 0;
			ContentUIItem->SetRelativeLocation(Position);
            UpdateProgress();
        }
		if (Vertical)
		{
			bAllowVerticalScroll = true;
			bCanUpdateAfterDrag = true;
			Position.Z = value.Y;
			Velocity.Y = 0;
			ContentUIItem->SetRelativeLocation(Position);
            UpdateProgress();
		}
    }
}

void UUIScrollViewComponent::SetScrollProgress(FVector2D value)
{
    if (CheckParameters())
    {
        if (Horizontal)
        {
            bCanUpdateAfterDrag = true;
            bAllowHorizontalScroll = true;

            RecalculateRange();
            value.X = FMath::Clamp(value.X, 0.0f, 1.0f);
            Position.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, value.X);
            ContentUIItem->SetRelativeLocation(Position);
            UpdateProgress();
        }
        if (Vertical)
        {
            bCanUpdateAfterDrag = true;
            bAllowVerticalScroll = true;

            RecalculateRange();
            value.Y = FMath::Clamp(value.Y, 0.0f, 1.0f);
            Position.Z = FMath::Lerp(VerticalRange.X, VerticalRange.Y, value.Y);
            ContentUIItem->SetRelativeLocation(Position);
            UpdateProgress();
        }
    }
}

#define POSITION_THRESHOLD 0.001f
void UUIScrollViewComponent::UpdateAfterDrag(float deltaTime)
{
    if (FMath::Abs(Velocity.X) > 0 || FMath::Abs(Velocity.Y) > 0                                       //speed large then threshold
        || (bAllowHorizontalScroll && (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y)) //horizontal out of range
        || (bAllowVerticalScroll && (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y)))      //vertical out of range
    {
        bool canMove = false;
        const float positionTimeMultiply = 50.0f;
        const float dragForceMulitply = 10.0f;
        const float positionLerpTimeMultiply = 10.0f;
        if (bAllowHorizontalScroll)
        {
            if (Position.Y - HorizontalRange.X < 0)
            {
                if (Velocity.X < 0)
                {
                    float dragForce = (HorizontalRange.X - Position.Y) * dragForceMulitply;
                    Velocity.X += -FMath::Sign(Velocity.X) * dragForce * deltaTime;

                    Position.Y += Velocity.X * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    Velocity.X = 0;
                    if (FMath::Abs(HorizontalRange.X - Position.Y) < POSITION_THRESHOLD)
                    {
                        Position.Y = HorizontalRange.X;
                    }
                    else
                    {
                        auto lerpAlpha = FMath::Clamp(positionLerpTimeMultiply * deltaTime, 0.0f, 1.0f);
                        Position.Y = FMath::Lerp(Position.Y, HorizontalRange.X, lerpAlpha);
                    }
                    canMove = true;
                }
            }
            else if (Position.Y - HorizontalRange.Y > 0)
            {
                if (Velocity.X > 0) //move right, use opposite force
                {
                    float dragForce = (Position.Y - HorizontalRange.Y) * dragForceMulitply;
                    Velocity.X += -FMath::Sign(Velocity.X) * dragForce * deltaTime;

                    Position.Y += Velocity.X * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    Velocity.X = 0;
                    if (FMath::Abs(Position.Y - HorizontalRange.Y) < POSITION_THRESHOLD)
                    {
                        Position.Y = HorizontalRange.Y;
                    }
                    else
                    {
                        auto lerpAlpha = FMath::Clamp(positionLerpTimeMultiply * deltaTime, 0.0f, 1.0f);
                        Position.Y = FMath::Lerp(Position.Y, HorizontalRange.Y, lerpAlpha);
                    }
                    canMove = true;
                }
            }
            else
            {
                auto speedXDir = FMath::Sign(Velocity.X);
                float dragForce = 10;
                Velocity.X += -FMath::Sign(Velocity.X) * dragForce * deltaTime;
                if (FMath::Sign(Velocity.X) != speedXDir) //accelerate speed change speed direction, set speed to 0
                {
                    Velocity.X = 0;
                }
                Position.Y += Velocity.X * deltaTime * positionTimeMultiply;
                canMove = true;
            }
        }
        if (bAllowVerticalScroll)
        {
            if (Position.Z - VerticalRange.X < 0)
            {
                if (Velocity.Y < 0)
                {
                    float dragForce = (VerticalRange.X - Position.Z) * dragForceMulitply;
                    Velocity.Y += -FMath::Sign(Velocity.Y) * dragForce * deltaTime;

                    Position.Z += Velocity.Y * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    Velocity.Y = 0;
                    if (FMath::Abs(VerticalRange.X - Position.Z) < POSITION_THRESHOLD)
                    {
                        Position.Z = VerticalRange.X;
                    }
                    else
                    {
                        auto lerpAlpha = FMath::Clamp(positionLerpTimeMultiply * deltaTime, 0.0f, 1.0f);
                        Position.Z = FMath::Lerp(Position.Z, VerticalRange.X, lerpAlpha);
                    }
                    canMove = true;
                }
            }
            else if (Position.Z - VerticalRange.Y > 0)
            {
                if (Velocity.Y > 0) //move up, use opposite force
                {
                    float dragForce = (Position.Z - VerticalRange.Y) * dragForceMulitply;
                    Velocity.Y += -FMath::Sign(Velocity.Y) * dragForce * deltaTime;

                    Position.Z += Velocity.Y * deltaTime * positionTimeMultiply;
                    canMove = true;
                }
                else
                {
                    Velocity.Y = 0;
                    if (FMath::Abs(Position.Z - VerticalRange.Y) < POSITION_THRESHOLD)
                    {
                        Position.Z = VerticalRange.Y;
                    }
                    else
                    {
                        auto lerpAlpha = FMath::Clamp(positionLerpTimeMultiply * deltaTime, 0.0f, 1.0f);
                        Position.Z = FMath::Lerp(Position.Z, VerticalRange.Y, lerpAlpha);
                    }
                    canMove = true;
                }
            }
            else
            {
                auto speedYDir = FMath::Sign(Velocity.Y);
                float dragForce = 10;
                Velocity.Y += -FMath::Sign(Velocity.Y) * dragForce * deltaTime;
                if (FMath::Sign(Velocity.Y) != speedYDir) //accelerate speed change speed direction, set speed to 0
                {
                    Velocity.Y = 0;
                }
                Position.Z += Velocity.Y * deltaTime * positionTimeMultiply;
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
        bCanUpdateAfterDrag = false;
    }
}
void UUIScrollViewComponent::UpdateProgress(bool InFireEvent)
{
    if (!ContentUIItem.IsValid())
        return;
    auto relativeLocation = ContentUIItem->GetRelativeLocation();
    if (bAllowHorizontalScroll)
    {
        Progress.X = (relativeLocation.Y - HorizontalRange.X) / (HorizontalRange.Y - HorizontalRange.X);
    }
    if (bAllowVerticalScroll)
    {
        Progress.Y = (relativeLocation.Z - VerticalRange.X) / (VerticalRange.Y - VerticalRange.X);
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
    if (ContentParentUIItem->GetWidth() > ContentUIItem->GetWidth())
    {
        if (CanScrollInSmallSize)
        {
            //parent
            HorizontalRange.X = -ContentParentUIItem->GetPivot().X * ContentParentUIItem->GetWidth();
            HorizontalRange.Y = (1.0f - ContentParentUIItem->GetPivot().X) * ContentParentUIItem->GetWidth();
            //self
            HorizontalRange.X += ContentUIItem->GetPivot().X * ContentUIItem->GetWidth();
            HorizontalRange.Y += (ContentUIItem->GetPivot().X - 1.0f) * ContentUIItem->GetWidth();
        }
        else
        {
			//parent
			HorizontalRange.X = -ContentParentUIItem->GetPivot().X * ContentParentUIItem->GetWidth();
			HorizontalRange.Y = (1.0f - ContentParentUIItem->GetPivot().X) * ContentParentUIItem->GetWidth() - (ContentParentUIItem->GetWidth() - ContentUIItem->GetWidth());
			//self
			HorizontalRange.X += ContentUIItem->GetPivot().X * ContentUIItem->GetWidth();
			HorizontalRange.Y += (ContentUIItem->GetPivot().X - 1.0f) * ContentUIItem->GetWidth();
        }
    }
    else
    {
        //self
        HorizontalRange.X = (ContentUIItem->GetPivot().X - 1.0f) * ContentUIItem->GetWidth();
        HorizontalRange.Y = ContentUIItem->GetPivot().X * ContentUIItem->GetWidth();
        //parent
        HorizontalRange.X += (1.0f - ContentParentUIItem->GetPivot().X) * ContentParentUIItem->GetWidth();
        HorizontalRange.Y += -ContentParentUIItem->GetPivot().X * ContentParentUIItem->GetWidth();
    }
}
void UUIScrollViewComponent::CalculateVerticalRange()
{
    auto &parentAnchorData = ContentParentUIItem->GetAnchorData();
    auto &contentAnchorData = ContentUIItem->GetAnchorData();
    if (ContentParentUIItem->GetHeight() > ContentUIItem->GetHeight())
    {
        if (CanScrollInSmallSize)
        {
            //parent
            VerticalRange.X = -ContentParentUIItem->GetPivot().Y * ContentParentUIItem->GetHeight();
            VerticalRange.Y = (1.0f - ContentParentUIItem->GetPivot().Y) * ContentParentUIItem->GetHeight();
            //self
            VerticalRange.X += ContentUIItem->GetPivot().Y * ContentUIItem->GetHeight();
            VerticalRange.Y += (ContentUIItem->GetPivot().Y - 1.0f) * ContentUIItem->GetHeight();
        }
        else
        {
			//parent
			VerticalRange.X = -ContentParentUIItem->GetPivot().Y * ContentParentUIItem->GetHeight() + (ContentParentUIItem->GetHeight() - ContentUIItem->GetHeight());
			VerticalRange.Y = (1.0f - ContentParentUIItem->GetPivot().Y) * ContentParentUIItem->GetHeight();
			//self
			VerticalRange.X += ContentUIItem->GetPivot().Y * ContentUIItem->GetHeight();
			VerticalRange.Y += (ContentUIItem->GetPivot().Y - 1.0f) * ContentUIItem->GetHeight();
        }
    }
    else
    {
        //self
        VerticalRange.X = (ContentUIItem->GetPivot().Y - 1.0f) * ContentUIItem->GetHeight();
        VerticalRange.Y = ContentUIItem->GetPivot().Y * ContentUIItem->GetHeight();
        //parent
        VerticalRange.X += (1.0f - ContentParentUIItem->GetPivot().Y) * ContentParentUIItem->GetHeight();
        VerticalRange.Y += -ContentParentUIItem->GetPivot().Y * ContentParentUIItem->GetHeight();
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
        bRangeCalculated = false;
        RecalculateRange();
    }
}
void UUIScrollViewComponent::SetVertical(bool value)
{
	if (Vertical != value)
	{
        Vertical = value;
        bRangeCalculated = false;
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
        bRangeCalculated = false;
        RecalculateRange();
    }
}
