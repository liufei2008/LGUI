// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewComponent.h"
#include "LGUI.h"
#include "LTweenManager.h"
#include "Core/Actor/UIBaseActor.h"
#include "Utils/LGUIUtils.h"

void UUIScrollViewHelper::Awake()
{
    Super::Awake();
    this->SetCanExecuteUpdate(false);
}
void UUIScrollViewHelper::OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    Super::OnUIDimensionsChanged(horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        if (widthChanged || heightChanged)
        {
            TargetComp->bRangeCalculated = false;
            TargetComp->RecalculateRange();
        }
    }
}
void UUIScrollViewHelper::OnUIChildDimensionsChanged(UUIItem *child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    Super::OnUIChildDimensionsChanged(child, horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        if (widthChanged || heightChanged)
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
    if (auto Property = PropertyChangedEvent.MemberProperty)
    {
        if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UUIScrollViewComponent, Progress))
        {
            ApplyContentPositionWithProgress();
        }
    }
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

        if (KeepProgress)
        {
            ApplyContentPositionWithProgress();
        }
        else
        {
            auto Position = ContentUIItem->GetRelativeLocation();
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
void UUIScrollViewComponent::OnUIDimensionsChanged(bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)
{
    Super::OnUIDimensionsChanged(horizontalPositionChanged, verticalPositionChanged, widthChanged, heightChanged);
    if (widthChanged || heightChanged)
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

bool UUIScrollViewComponent::OnPointerBeginDrag_Implementation(ULGUIPointerEventData *eventData)
{
    if (CheckParameters() && CheckValidHit(eventData->dragComponent))
    {
        PrevPointerPosition = eventData->pressWorldPoint;
        auto CurrentPointerPosition = eventData->GetWorldPointInPlane();
        const auto localMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(CurrentPointerPosition - PrevPointerPosition);
        PrevPointerPosition = CurrentPointerPosition;
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
    auto Position = ContentUIItem->GetRelativeLocation();
    auto CurrentPointerPosition = eventData->GetWorldPointInPlane();
    auto localMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(CurrentPointerPosition - PrevPointerPosition);
    PrevPointerPosition = CurrentPointerPosition;
    if (bAllowHorizontalScroll)
    {
        auto predict = Position.Y + localMoveDelta.Y;
        if (predict < HorizontalRange.X || predict > HorizontalRange.Y && RestrictRectArea) //out-of-range, lower the sentitivity
        {
            Position.Y += localMoveDelta.Y * OutOfRangeDamper;
        }
        else
        {
            Position.Y = predict;
        }
        bCanUpdateAfterDrag = false;
        ContentUIItem->SetRelativeLocation(Position);
        UpdateProgress();
    }
    if (bAllowVerticalScroll)
    {
        auto predict = Position.Z + localMoveDelta.Z;
        if (predict < VerticalRange.X || predict > VerticalRange.Y && RestrictRectArea)
        {
            Position.Z += localMoveDelta.Z * OutOfRangeDamper;
        }
        else
        {
            Position.Z = predict;
        }
        bCanUpdateAfterDrag = false;
        ContentUIItem->SetRelativeLocation(Position);
        UpdateProgress();
    }
    return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerEndDrag_Implementation(ULGUIPointerEventData *eventData)
{
    auto Position = ContentUIItem->GetRelativeLocation();
    auto CurrentPointerPosition = eventData->GetWorldPointInPlane();
    const auto localMoveDelta = eventData->pressWorldToLocalTransform.TransformVector(CurrentPointerPosition - PrevPointerPosition);
    if (bAllowHorizontalScroll)
    {
        bCanUpdateAfterDrag = true;
        Velocity.X = localMoveDelta.Y / GetWorld()->DeltaTimeSeconds;
    }
    if (bAllowVerticalScroll)
    {
        bCanUpdateAfterDrag = true;
        Velocity.Y = localMoveDelta.Z / GetWorld()->DeltaTimeSeconds;
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

            auto Position = ContentUIItem->GetRelativeLocation();
            if (bAllowHorizontalScroll)
            {
                auto delta = eventData->scrollAxisValue.X * ScrollSensitivity;
                bCanUpdateAfterDrag = true;
                if (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y && RestrictRectArea)
                {
                    Position.Y += delta * OutOfRangeDamper;
                    Velocity.X = delta * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
                }
                else
                {
                    Position.Y += delta;
                    Velocity.X = delta / GetWorld()->DeltaTimeSeconds;
                }
                ContentUIItem->SetRelativeLocation(Position);
            }
            if (bAllowVerticalScroll)
            {
                auto delta = eventData->scrollAxisValue.Y * -ScrollSensitivity;
                bCanUpdateAfterDrag = true;
                if (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y && RestrictRectArea)
                {
                    Position.Z += delta * OutOfRangeDamper;
                    Velocity.Y = delta * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
                }
                else
                {
                    Position.Z += delta;
                    Velocity.Y = delta / GetWorld()->DeltaTimeSeconds;
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

void UUIScrollViewComponent::SetDecelerateRate(float value)
{
    if (DecelerateRate != value)
    {
        DecelerateRate = value;
        DecelerateRate = FMath::Max(0.0f, DecelerateRate);
    }
}

void UUIScrollViewComponent::SetRestrictRectArea(bool value)
{
    if (RestrictRectArea != value)
    {
        RestrictRectArea = value;
        if (RestrictRectArea)
        {
            bCanUpdateAfterDrag = true;
        }
    }
}

void UUIScrollViewComponent::SetOutOfRangeDamper(float value)
{
    if (OutOfRangeDamper != value)
    {
        OutOfRangeDamper = value;
        OutOfRangeDamper = FMath::Clamp(OutOfRangeDamper, 0.0f, 1.0f);
    }
}

void UUIScrollViewComponent::SetScrollDelta(FVector2D value)
{
    if (CheckParameters())
    {
        auto delta = value;
        auto Position = ContentUIItem->GetRelativeLocation();
        if (Horizontal)
		{
			bAllowHorizontalScroll = true;
			bCanUpdateAfterDrag = true;
			if (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y && RestrictRectArea)
			{
				Position.Y += delta.X * OutOfRangeDamper;
				Velocity.X = delta.X * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
			}
			else
			{
				Position.Y += delta.X;
				Velocity.X = delta.X / GetWorld()->DeltaTimeSeconds;
			}
			ContentUIItem->SetRelativeLocation(Position);
		}
		if (Vertical)
		{
			bAllowVerticalScroll = true;
			bCanUpdateAfterDrag = true;
			if (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y && RestrictRectArea)
			{
				Position.Z += delta.Y * OutOfRangeDamper;
				Velocity.Y = delta.Y * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
			}
			else
			{
				Position.Z += delta.Y;
				Velocity.Y = delta.Y / GetWorld()->DeltaTimeSeconds;
			}
			ContentUIItem->SetRelativeLocation(Position);
		}
    }
}
void UUIScrollViewComponent::SetScrollValue(FVector2D value)
{
    if (CheckParameters())
    {
        auto Position = ContentUIItem->GetRelativeLocation();
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
        auto Position = ContentUIItem->GetRelativeLocation();
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

void UUIScrollViewComponent::ScrollTo(AUIBaseActor* InChild, bool InEaseAnimation, float InAnimationDuration)
{
    if (!CheckParameters())return;
    auto CenterPos = InChild->GetUIItem()->GetLocalSpaceCenter();
    auto CenterPosWorld = InChild->GetUIItem()->GetComponentTransform().TransformPosition(FVector(0, CenterPos.X, CenterPos.Y));
    auto PosOffset = Content->GetUIItem()->GetComponentTransform().InverseTransformPosition(CenterPosWorld);
    auto TargetContentPos = FVector2D(-PosOffset.Y, -PosOffset.Z);
    TargetContentPos.X = FMath::Clamp(TargetContentPos.X, HorizontalRange.X, HorizontalRange.Y);
    TargetContentPos.Y = FMath::Clamp(TargetContentPos.Y, VerticalRange.X, VerticalRange.Y);
    if (InEaseAnimation)
    {
        ULTweenManager::To(this, FLTweenVector2DGetterFunction::CreateWeakLambda(this
            , [=] {
                auto ContentLocation = ContentUIItem->GetRelativeLocation();
                return FVector2D(ContentLocation.Y, ContentLocation.Z);
            })
            , FLTweenVector2DSetterFunction::CreateWeakLambda(this, [=](FVector2D value) {
                this->SetScrollValue(value);
                }), TargetContentPos, InAnimationDuration);
    }
    else
    {
        SetScrollValue(TargetContentPos);
    }
}

#define POSITION_THRESHOLD 0.001f
void UUIScrollViewComponent::UpdateAfterDrag(float deltaTime)
{
    auto Position = ContentUIItem->GetRelativeLocation();
    if (FMath::Abs(Velocity.X) > KINDA_SMALL_NUMBER || FMath::Abs(Velocity.Y) > KINDA_SMALL_NUMBER//speed larger than threshold
        || (bAllowHorizontalScroll && (Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y))//horizontal out of range
        || (bAllowVerticalScroll && (Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y)))//vertical out of range
    {
        bool canMove = false;
        const float dragForceMulitply = 500.0f;
        const float positionLerpTimeMultiply = 10.0f;
        if (bAllowHorizontalScroll)
        {
            if (Position.Y - HorizontalRange.X < 0 && RestrictRectArea)
            {
                if (Velocity.X < 0)
                {
                    float dragForce = (HorizontalRange.X - Position.Y) * dragForceMulitply;
                    Velocity.X += -FMath::Sign(Velocity.X) * dragForce * deltaTime;

                    Position.Y += Velocity.X * deltaTime;
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
            else if (Position.Y - HorizontalRange.Y > 0 && RestrictRectArea)
            {
                if (Velocity.X > 0) //move right, use opposite force
                {
                    float dragForce = (Position.Y - HorizontalRange.Y) * dragForceMulitply;
                    Velocity.X += -FMath::Sign(Velocity.X) * dragForce * deltaTime;

                    Position.Y += Velocity.X * deltaTime;
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
                float dragForce = dragForceMulitply * 0.1f;
                float VelocityLerpAlpha = FMath::Clamp(DecelerateRate * dragForce * deltaTime, 0.0f, 1.0f);
                Velocity.X = FMath::Lerp(Velocity.X, 0.0f, VelocityLerpAlpha);
                Position.Y += Velocity.X * deltaTime;
                canMove = true;
            }
        }
        if (bAllowVerticalScroll)
        {
            if (Position.Z - VerticalRange.X < 0 && RestrictRectArea)
            {
                if (Velocity.Y < 0)
                {
                    float dragForce = (VerticalRange.X - Position.Z) * dragForceMulitply;
                    Velocity.Y += -FMath::Sign(Velocity.Y) * dragForce * deltaTime;

                    Position.Z += Velocity.Y * deltaTime;
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
            else if (Position.Z - VerticalRange.Y > 0 && RestrictRectArea)
            {
                if (Velocity.Y > 0) //move up, use opposite force
                {
                    float dragForce = (Position.Z - VerticalRange.Y) * dragForceMulitply;
                    Velocity.Y += -FMath::Sign(Velocity.Y) * dragForce * deltaTime;

                    Position.Z += Velocity.Y * deltaTime;
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
                float dragForce = dragForceMulitply * 0.1f;
                float VelocityLerpAlpha = FMath::Clamp(DecelerateRate * dragForce * deltaTime, 0.0f, 1.0f);
                Velocity.Y = FMath::Lerp(Velocity.Y, 0.0f, VelocityLerpAlpha);
                Position.Z += Velocity.Y * deltaTime;
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

void UUIScrollViewComponent::ApplyContentPositionWithProgress()
{
    if (CheckParameters())
    {
        auto Position = ContentUIItem->GetRelativeLocation();
        if (Horizontal)
        {
            bCanUpdateAfterDrag = true;
            bAllowHorizontalScroll = true;

            Progress.X = FMath::Clamp(Progress.X, 0.0f, 1.0f);
            Position.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, 1.0f - Progress.X);
            ContentUIItem->SetRelativeLocation(Position);
        }
        if (Vertical)
        {
            bCanUpdateAfterDrag = true;
            bAllowVerticalScroll = true;

            Progress.Y = FMath::Clamp(Progress.Y, 0.0f, 1.0f);
            Position.Z = FMath::Lerp(VerticalRange.X, VerticalRange.Y, Progress.Y);
            ContentUIItem->SetRelativeLocation(Position);
        }
    }
}


void UUIScrollViewComponent::UpdateProgress(bool InFireEvent)
{
    if (!ContentUIItem.IsValid())
        return;
    auto relativeLocation = ContentUIItem->GetRelativeLocation();
    if (bAllowHorizontalScroll)
    {
        if (FMath::Abs(HorizontalRange.Y - HorizontalRange.X) > KINDA_SMALL_NUMBER)
        {
            Progress.X = 1.0f - (relativeLocation.Y - HorizontalRange.X) / (HorizontalRange.Y - HorizontalRange.X);
        }
    }
    if (bAllowVerticalScroll)
    {
        if (FMath::Abs(VerticalRange.Y - VerticalRange.X) > KINDA_SMALL_NUMBER)
        {
            Progress.Y = (relativeLocation.Z - VerticalRange.X) / (VerticalRange.Y - VerticalRange.X);
        }
    }
    if (InFireEvent)
    {
        OnScrollCPP.Broadcast(Progress);
        OnScroll.FireEvent(Progress);
    }
}

void UUIScrollViewComponent::CalculateHorizontalRange()
{
    if (ContentParentUIItem->GetWidth() > ContentUIItem->GetWidth())//content size smaller than parent
    {
        //parent
        HorizontalRange.X = -ContentParentUIItem->GetPivot().X * ContentParentUIItem->GetWidth();
        HorizontalRange.Y = (1.0f - ContentParentUIItem->GetPivot().X) * ContentParentUIItem->GetWidth();
        //self
        HorizontalRange.X += ContentUIItem->GetPivot().X * ContentUIItem->GetWidth();
        HorizontalRange.Y += (ContentUIItem->GetPivot().X - 1.0f) * ContentUIItem->GetWidth();

        if (KeepProgress)
        {
            if (!CanScrollInSmallSize)
            {
                //this can make content stay at Progress.X's position
                HorizontalRange.X = HorizontalRange.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y
                    , FlipDirectionInSmallSize ? 1.0f - Progress.X : Progress.X
                );
            }
        }
        else
        {
            HorizontalRange.Y -= ContentParentUIItem->GetWidth() - ContentUIItem->GetWidth();
        }
    }
    else//content size bigger than parent
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
    if (ContentParentUIItem->GetHeight() > ContentUIItem->GetHeight())//content size smaller than parent
    {
        //parent
        VerticalRange.X = -ContentParentUIItem->GetPivot().Y * ContentParentUIItem->GetHeight();
        VerticalRange.Y = (1.0f - ContentParentUIItem->GetPivot().Y) * ContentParentUIItem->GetHeight();
        //self
        VerticalRange.X += ContentUIItem->GetPivot().Y * ContentUIItem->GetHeight();
        VerticalRange.Y += (ContentUIItem->GetPivot().Y - 1.0f) * ContentUIItem->GetHeight();

        if (KeepProgress)
        {
            if (!CanScrollInSmallSize)
            {
                //this can make content stay at Progress.Y's position
                VerticalRange.X = VerticalRange.Y = FMath::Lerp(VerticalRange.X, VerticalRange.Y
                    , FlipDirectionInSmallSize ? Progress.Y : 1.0f - Progress.Y
                );
            }
        }
        else
        {
            VerticalRange.X += ContentParentUIItem->GetHeight() - ContentUIItem->GetHeight();
        }
    }
    else//content size bigger than parent
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
        InDelegate.ExecuteIfBound(InProgress);
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
