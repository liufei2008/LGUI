// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Interaction/UIScrollView.h"
#include "LTweenManager.h"
#include "Core/Components/LexWidget.h"

UUIScrollViewHelper::UUIScrollViewHelper()
{
    bStartWithTickEnabled = false;
}

void UUIScrollViewHelper::Awake()
{
    Super::Awake();
    this->SetCanExecuteTick(false);
}
void UUIScrollViewHelper::OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
    Super::OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        TargetComp->bRangeCalculated = false;
        TargetComp->RecalculateRange();
    }
}
void UUIScrollViewHelper::OnChildDimensionsChanged(ULexWidget *Child, bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
    Super::OnChildDimensionsChanged(Child, PivotChanged, WidthChanged, HeightChanged);
    if (!TargetComp.IsValid())
    {
        this->DestroyComponent();
    }
    else
    {
        if (WidthChanged || HeightChanged)
        {
            TargetComp->bRangeCalculated = false;
            TargetComp->RecalculateRange();
        }
    }
}

void UUIScrollView::Awake()
{
    Super::Awake();
    bRangeCalculated = false;
    RecalculateRange();
    this->SetCanExecuteTick(true);
}

void UUIScrollView::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
    if (bCanUpdateAfterDrag)
        UpdateAfterDrag(DeltaTime);
}

#if WITH_EDITOR
void UUIScrollView::PostEditChangeProperty(FPropertyChangedEvent &PropertyChangedEvent)
{
    Super::PostEditChangeProperty(PropertyChangedEvent);
    bRangeCalculated = false;
    RecalculateRange();
    if (auto Property = PropertyChangedEvent.MemberProperty)
    {
        if (Property->GetFName() == GET_MEMBER_NAME_CHECKED(UUIScrollView, Progress))
        {
            ApplyContentPositionWithProgress();
        }
    }
}
#endif

void UUIScrollView::RecalculateRange()
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
            auto Position = Content->GetRelativeLocation();
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

void UUIScrollView::OnEnable()
{
    Super::OnEnable();
    bRangeCalculated = false;
    RecalculateRange();
}

void UUIScrollView::OnTransformChanged()
{
    Super::OnTransformChanged();
    bRangeCalculated = false;
    RecalculateRange();
}

void UUIScrollView::OnDimensionsChanged(bool PivotChanged, bool WidthChanged, bool HeightChanged)
{
    Super::OnDimensionsChanged(PivotChanged, WidthChanged, HeightChanged);
    bRangeCalculated = false;
    RecalculateRange();
}

bool UUIScrollView::CheckParameters()
{
    auto Widget = GetWidget();
    if (Content.IsValid() && ContentParent.IsValid() && Widget)
        return true;
    if (!Widget)return false;
    if (!Content.IsValid())return false;
    ContentParent = Content->GetParent();
    if (!ContentParent.IsValid())return false;
    //add helper comp to detect size change
    {
        auto HelperComp = ContentParent->AddComponent<UUIScrollViewHelper>();
        HelperComp->TargetComp = this;
    }
    return true;
}

bool UUIScrollView::CheckValidHit(ULexWidget *InHitComp)
{
    auto Widget = GetWidget();
    return (InHitComp->IsChildOf(Widget) || InHitComp == Widget); //make sure hit component is child of this or is this
}

bool UUIScrollView::OnPointerBeginDrag_Implementation(ULexPointerEventData *EventData)
{
    if (CheckParameters() && CheckValidHit(EventData->DragWidget))
    {
        PrevPointerPosition = EventData->PressWorldPoint;
        auto CurrentPointerPosition = EventData->GetWorldPointInPlane();
        const auto localMoveDelta = EventData->PressWorldToLocalTransform.TransformVector(CurrentPointerPosition - PrevPointerPosition);
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
        OnPointerDrag_Implementation(EventData);
    }
    else
    {
        bAllowHorizontalScroll = bAllowVerticalScroll = false;
    }
    return AllowEventBubbleUp;
}

bool UUIScrollView::OnPointerDrag_Implementation(ULexPointerEventData *EventData)
{
    if (!Content.IsValid())
        return AllowEventBubbleUp;
    auto Position = Content->GetRelativeLocation();
    auto CurrentPointerPosition = EventData->GetWorldPointInPlane();
    auto localMoveDelta = EventData->PressWorldToLocalTransform.TransformVector(CurrentPointerPosition - PrevPointerPosition);
    PrevPointerPosition = CurrentPointerPosition;
    if (bAllowHorizontalScroll)
    {
        auto predict = Position.Y + localMoveDelta.Y;
        if ((predict < HorizontalRange.X || predict > HorizontalRange.Y) && RestrictRectArea) //out-of-range, lower the sentitivity
        {
            Position.Y += localMoveDelta.Y * OutOfRangeDamper;
        }
        else
        {
            Position.Y = predict;
        }
        bCanUpdateAfterDrag = false;
        Content->SetRelativeLocation(Position);
        UpdateProgress();
    }
    if (bAllowVerticalScroll)
    {
        auto predict = Position.Z + localMoveDelta.Z;
        if ((predict < VerticalRange.X || predict > VerticalRange.Y) && RestrictRectArea)
        {
            Position.Z += localMoveDelta.Z * OutOfRangeDamper;
        }
        else
        {
            Position.Z = predict;
        }
        bCanUpdateAfterDrag = false;
        Content->SetRelativeLocation(Position);
        UpdateProgress();
    }
    return AllowEventBubbleUp;
}

bool UUIScrollView::OnPointerEndDrag_Implementation(ULexPointerEventData *EventData)
{
    auto Position = Content->GetRelativeLocation();
    auto CurrentPointerPosition = EventData->GetWorldPointInPlane();
    const auto localMoveDelta = EventData->PressWorldToLocalTransform.TransformVector(CurrentPointerPosition - PrevPointerPosition);
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
bool UUIScrollView::OnPointerScroll_Implementation(ULexPointerEventData *EventData)
{
    if (CheckParameters() && CheckValidHit(EventData->EnterWidget))
    {
        if (EventData->ScrollAxisValue != FVector2D::ZeroVector)
        {
            bAllowHorizontalScroll = false;
            bAllowVerticalScroll = false;
            if (OnlyOneDirection && Horizontal && Vertical)
            {
                if (FMath::Abs(EventData->ScrollAxisValue.X) > FMath::Abs(EventData->ScrollAxisValue.Y))
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

            auto Position = Content->GetRelativeLocation();
            if (bAllowHorizontalScroll)
            {
                auto delta = EventData->ScrollAxisValue.X * ScrollSensitivity;
                bCanUpdateAfterDrag = true;
                if ((Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y) && RestrictRectArea)
                {
                    Position.Y += delta * OutOfRangeDamper;
                    Velocity.X = delta * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
                }
                else
                {
                    Position.Y += delta;
                    Velocity.X = delta / GetWorld()->DeltaTimeSeconds;
                }
                Content->SetRelativeLocation(Position);
            }
            if (bAllowVerticalScroll)
            {
                auto delta = EventData->ScrollAxisValue.Y * -ScrollSensitivity;
                bCanUpdateAfterDrag = true;
                if ((Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y) && RestrictRectArea)
                {
                    Position.Z += delta * OutOfRangeDamper;
                    Velocity.Y = delta * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
                }
                else
                {
                    Position.Z += delta;
                    Velocity.Y = delta / GetWorld()->DeltaTimeSeconds;
                }
                Content->SetRelativeLocation(Position);
            }
        }
    }
    return AllowEventBubbleUp;
}

void UUIScrollView::SetVelocity(const FVector2D& value)
{
    if (CheckParameters())
    {
        Velocity = value;
        bCanUpdateAfterDrag = true;
    }
}

void UUIScrollView::SetDecelerateRate(float value)
{
    if (DecelerateRate != value)
    {
        DecelerateRate = value;
        DecelerateRate = FMath::Max(0.0f, DecelerateRate);
    }
}

void UUIScrollView::SetRestrictRectArea(bool value)
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

void UUIScrollView::SetOutOfRangeDamper(float value)
{
    if (OutOfRangeDamper != value)
    {
        OutOfRangeDamper = value;
        OutOfRangeDamper = FMath::Clamp(OutOfRangeDamper, 0.0f, 1.0f);
    }
}

void UUIScrollView::SetScrollDelta(FVector2D value)
{
    if (CheckParameters())
    {
        auto delta = value;
        auto Position = Content->GetRelativeLocation();
        if (Horizontal)
		{
			bAllowHorizontalScroll = true;
			bCanUpdateAfterDrag = true;
			if ((Position.Y < HorizontalRange.X || Position.Y > HorizontalRange.Y) && RestrictRectArea)
			{
				Position.Y += delta.X * OutOfRangeDamper;
				Velocity.X = delta.X * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
			}
			else
			{
				Position.Y += delta.X;
				Velocity.X = delta.X / GetWorld()->DeltaTimeSeconds;
			}
			Content->SetRelativeLocation(Position);
		}
		if (Vertical)
		{
			bAllowVerticalScroll = true;
			bCanUpdateAfterDrag = true;
			if ((Position.Z < VerticalRange.X || Position.Z > VerticalRange.Y) && RestrictRectArea)
			{
				Position.Z += delta.Y * OutOfRangeDamper;
				Velocity.Y = delta.Y * OutOfRangeDamper / GetWorld()->DeltaTimeSeconds;
			}
			else
			{
				Position.Z += delta.Y;
				Velocity.Y = delta.Y / GetWorld()->DeltaTimeSeconds;
			}
			Content->SetRelativeLocation(Position);
		}
    }
}
void UUIScrollView::SetScrollValue(FVector2D value)
{
    if (CheckParameters())
    {
        auto Position = Content->GetRelativeLocation();
        if (Horizontal)
		{
			bAllowHorizontalScroll = true;
			bCanUpdateAfterDrag = true;
			Position.Y = value.X;
			Velocity.X = 0;
			Content->SetRelativeLocation(Position);
            UpdateProgress();
        }
		if (Vertical)
		{
			bAllowVerticalScroll = true;
			bCanUpdateAfterDrag = true;
			Position.Z = value.Y;
			Velocity.Y = 0;
			Content->SetRelativeLocation(Position);
            UpdateProgress();
		}
    }
}

void UUIScrollView::SetScrollProgress(FVector2D value)
{
    if (CheckParameters())
    {
        auto Position = Content->GetRelativeLocation();
        if (Horizontal)
        {
            bCanUpdateAfterDrag = true;
            bAllowHorizontalScroll = true;

            RecalculateRange();
            value.X = FMath::Clamp(value.X, 0.0f, 1.0f);
            Position.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, value.X);
            Content->SetRelativeLocation(Position);
            UpdateProgress();
        }
        if (Vertical)
        {
            bCanUpdateAfterDrag = true;
            bAllowVerticalScroll = true;

            RecalculateRange();
            value.Y = FMath::Clamp(value.Y, 0.0f, 1.0f);
            Position.Z = FMath::Lerp(VerticalRange.X, VerticalRange.Y, value.Y);
            Content->SetRelativeLocation(Position);
            UpdateProgress();
        }
    }
}

void UUIScrollView::ScrollTo(ULexWidget* InChild, bool InEaseAnimation, float InAnimationDuration)
{
    if (!CheckParameters())return;
    auto CenterPos = InChild->GetLocalSpaceCenter();
    auto CenterPosWorld = InChild->GetWorldTransform().TransformPosition(FVector(0, CenterPos.X, CenterPos.Y));
    auto PosOffset = Content->GetWorldTransform().InverseTransformPosition(CenterPosWorld);
    auto TargetContentPos = FVector2D(-PosOffset.Y, -PosOffset.Z);
    TargetContentPos.X = FMath::Clamp(TargetContentPos.X, HorizontalRange.X, HorizontalRange.Y);
    TargetContentPos.Y = FMath::Clamp(TargetContentPos.Y, VerticalRange.X, VerticalRange.Y);
    if (InEaseAnimation)
    {
        auto Tweener = ULTweenManager::To(this, FLTweenVector2DGetterFunction::CreateWeakLambda(this
            , [this] {
                auto ContentLocation = Content->GetRelativeLocation();
                return FVector2D(ContentLocation.Y, ContentLocation.Z);
            })
            , FLTweenVector2DSetterFunction::CreateWeakLambda(this, [this](FVector2D value) {
                this->SetScrollValue(value);
                }), TargetContentPos, InAnimationDuration);
        if (Tweener)
        {
            ULexWidget::SetWidgetTweenerAffectByGamePauseAndTimeDilation(GetWidget(), Tweener);
        }
    }
    else
    {
        SetScrollValue(TargetContentPos);
    }
}

#define POSITION_THRESHOLD 0.001f
void UUIScrollView::UpdateAfterDrag(float deltaTime)
{
    auto Position = Content->GetRelativeLocation();
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
            Content->SetRelativeLocation(Position);
        }
    }
    else
    {
        bCanUpdateAfterDrag = false;
    }
}

void UUIScrollView::ApplyContentPositionWithProgress()
{
    if (CheckParameters())
    {
        auto Position = Content->GetRelativeLocation();
        if (Horizontal)
        {
            bCanUpdateAfterDrag = true;
            bAllowHorizontalScroll = true;

            Progress.X = FMath::Clamp(Progress.X, 0.0f, 1.0f);
            Position.Y = FMath::Lerp(HorizontalRange.X, HorizontalRange.Y, 1.0f - Progress.X);
            Content->SetRelativeLocation(Position);
        }
        if (Vertical)
        {
            bCanUpdateAfterDrag = true;
            bAllowVerticalScroll = true;

            Progress.Y = FMath::Clamp(Progress.Y, 0.0f, 1.0f);
            Position.Z = FMath::Lerp(VerticalRange.X, VerticalRange.Y, Progress.Y);
            Content->SetRelativeLocation(Position);
        }
    }
}


void UUIScrollView::UpdateProgress(bool InFireEvent)
{
    if (!Content.IsValid())
        return;
    auto relativeLocation = Content->GetRelativeLocation();
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
        OnValueChangedCPP.Broadcast(Progress);
        OnValueChanged.Broadcast(Progress);
        OnValueChangedED.FireEvent(Progress);
    }
}

void UUIScrollView::CalculateHorizontalRange()
{
    if (ContentParent->GetWidth() > Content->GetWidth())//content size smaller than parent
    {
        //parent
        HorizontalRange.X = -ContentParent->GetPivot().X * ContentParent->GetWidth();
        HorizontalRange.Y = (1.0f - ContentParent->GetPivot().X) * ContentParent->GetWidth();
        //self
        HorizontalRange.X += Content->GetPivot().X * Content->GetWidth();
        HorizontalRange.Y += (Content->GetPivot().X - 1.0f) * Content->GetWidth();

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
            HorizontalRange.Y -= ContentParent->GetWidth() - Content->GetWidth();
        }
    }
    else//content size bigger than parent
    {
        //self
        HorizontalRange.X = (Content->GetPivot().X - 1.0f) * Content->GetWidth();
        HorizontalRange.Y = Content->GetPivot().X * Content->GetWidth();
        //parent
        HorizontalRange.X += (1.0f - ContentParent->GetPivot().X) * ContentParent->GetWidth();
        HorizontalRange.Y += -ContentParent->GetPivot().X * ContentParent->GetWidth();
    }
}
void UUIScrollView::CalculateVerticalRange()
{
    if (ContentParent->GetHeight() > Content->GetHeight())//content size smaller than parent
    {
        //parent
        VerticalRange.X = -ContentParent->GetPivot().Y * ContentParent->GetHeight();
        VerticalRange.Y = (1.0f - ContentParent->GetPivot().Y) * ContentParent->GetHeight();
        //self
        VerticalRange.X += Content->GetPivot().Y * Content->GetHeight();
        VerticalRange.Y += (Content->GetPivot().Y - 1.0f) * Content->GetHeight();

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
            VerticalRange.X += ContentParent->GetHeight() - Content->GetHeight();
        }
    }
    else//content size bigger than parent
    {
        //self
        VerticalRange.X = (Content->GetPivot().Y - 1.0f) * Content->GetHeight();
        VerticalRange.Y = Content->GetPivot().Y * Content->GetHeight();
        //parent
        VerticalRange.X += (1.0f - ContentParent->GetPivot().Y) * ContentParent->GetHeight();
        VerticalRange.Y += -ContentParent->GetPivot().Y * ContentParent->GetHeight();
    }
}
void UUIScrollView::RectRangeChanged()
{
    if (Horizontal)
        CalculateHorizontalRange();
    if (Vertical)
        CalculateVerticalRange();
}

void UUIScrollView::SetHorizontal(bool value)
{
    if (Horizontal != value)
    {
        Horizontal = value;
        bRangeCalculated = false;
        RecalculateRange();
    }
}
void UUIScrollView::SetVertical(bool value)
{
	if (Vertical != value)
	{
        Vertical = value;
        bRangeCalculated = false;
        RecalculateRange();
	}
}
void UUIScrollView::SetOnlyOneDirection(bool value)
{
	if (OnlyOneDirection != value)
	{
        OnlyOneDirection = value;
	}
}
void UUIScrollView::SetScrollSensitivity(float value)
{
    if (ScrollSensitivity != value)
    {
        ScrollSensitivity = value;
    }
}
void UUIScrollView::SetCanScrollInSmallSize(bool value)
{
    if (CanScrollInSmallSize != value)
    {
        CanScrollInSmallSize = value;
        bRangeCalculated = false;
        RecalculateRange();
    }
}
