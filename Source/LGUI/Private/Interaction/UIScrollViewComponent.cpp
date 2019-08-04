// Copyright 2019 LexLiu. All Rights Reserved.

#include "Interaction/UIScrollViewComponent.h"
#include "LGUI.h"
#include "LTweenActor.h"
#include "Core/Actor/UIBaseActor.h"


void UUIScrollViewHelper::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if (TargetComp == nullptr)
	{
		this->DestroyComponent();
	}
	else
	{
		if (sizeChanged)
			TargetComp->RecalculateRange();
	}
}
void UUIScrollViewHelper::OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)
{
	if (TargetComp == nullptr)
	{
		this->DestroyComponent();
	}
	else
	{
		if (sizeChanged)
			TargetComp->RecalculateRange();
	}
}


UUIScrollViewComponent::UUIScrollViewComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UUIScrollViewComponent::BeginPlay()
{
	Super::BeginPlay();
	RecalculateRange();
}

void UUIScrollViewComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
	if (CanUpdateAfterDrag)UpdateAfterDrag(DeltaTime);
}

#if WITH_EDITOR
void UUIScrollViewComponent::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (GetOwner())
	{
		ContentUIItem = Cast<UUIItem>(GetOwner()->GetRootComponent());
	}
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
		if (Vertical)
		{
			this->CalculateVerticalRange();
			AllowVerticalScroll = true;
		}
		Position = ContentUIItem->RelativeLocation;
		if (Position.X < HorizontalRange.X || Position.X > HorizontalRange.Y
			|| Position.Y < VerticalRange.X || Position.Y > VerticalRange.Y)
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
	if (ativeOrInactive)
	{
		RecalculateRange();
	}
}
void UUIScrollViewComponent::OnUIDimensionsChanged(bool positionChanged, bool sizeChanged)
{
	if(sizeChanged)
		RecalculateRange();
}

bool UUIScrollViewComponent::CheckParameters()
{
	if (ContentUIItem != nullptr && ContentParentUIItem != nullptr && RootUIComp != nullptr)return true;
	if (Content == nullptr)return false;
	if (Content->GetAttachParentActor() == nullptr)return false;
	auto contentParentActor = Cast<AUIBaseActor>(Content->GetAttachParentActor());
	if (contentParentActor == nullptr)return false;
	ContentUIItem = Content->GetUIItem();
	ContentParentUIItem = contentParentActor->GetUIItem();
	if (ContentParentUIItem != nullptr)
	{
		auto contentParentHelperComp = NewObject<UUIScrollViewHelper>(contentParentActor);
		contentParentHelperComp->TargetComp = this;
		contentParentHelperComp->RegisterComponent();
	}
	CheckRootUIComponent();
	if (ContentUIItem != nullptr && ContentParentUIItem != nullptr && RootUIComp != nullptr)return true;
	return false;
}

bool UUIScrollViewComponent::CheckValidHit(UPrimitiveComponent* InHitComp)
{
	return (InHitComp->IsAttachedTo(RootUIComp) || InHitComp == RootUIComp);//make sure hit component is child of this or is this
}

bool UUIScrollViewComponent::OnPointerDown_Implementation(const FLGUIPointerEventData& eventData)
{
	PrevWorldPoint = eventData.pressWorldPoint;
	return AllowEventBubbleUp;
}
bool UUIScrollViewComponent::OnPointerUp_Implementation(const FLGUIPointerEventData& eventData)
{
	return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerBeginDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	if (CheckParameters() && CheckValidHit(eventData.hitComponent))
	{
		auto worldPoint = eventData.GetWorldPointInPlane();
		const auto localMoveDelta = eventData.pressWorldToLocalTransform.TransformVector(worldPoint - PrevWorldPoint);
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
		Position = ContentUIItem->RelativeLocation;
		CanUpdateAfterDrag = false;
		OnPointerDrag_Implementation(eventData);
	}
	else
	{
		AllowHorizontalScroll = AllowVerticalScroll = false;
	}
	return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	if (ContentUIItem == nullptr)return AllowEventBubbleUp;
	auto position = ContentUIItem->RelativeLocation;
	auto worldPoint = eventData.GetWorldPointInPlane();
	auto localMoveDelta = eventData.pressWorldToLocalTransform.TransformVector(worldPoint - PrevWorldPoint);
	localMoveDelta.Z = 0;
	PrevWorldPoint = worldPoint;
	if (AllowHorizontalScroll)
	{
		auto predict = position.X + localMoveDelta.X;
		if (predict < HorizontalRange.X || predict > HorizontalRange.Y)//out-of-range, lower the sentitivity
		{
			position.X += localMoveDelta.X * 0.5f;
		}
		else
		{
			position.X = predict;
		}
		CanUpdateAfterDrag = false;
		ContentUIItem->SetUIRelativeLocation(position);
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
		ContentUIItem->SetUIRelativeLocation(position);
		UpdateProgress();
	}
	return AllowEventBubbleUp;
}

bool UUIScrollViewComponent::OnPointerEndDrag_Implementation(const FLGUIPointerEventData& eventData)
{
	auto worldPoint = eventData.GetWorldPointInPlane();
	const auto localMoveDelta = eventData.pressWorldToLocalTransform.TransformVector(worldPoint - PrevWorldPoint);
	PrevWorldPoint = worldPoint;
	if (AllowHorizontalScroll)
	{
		CanUpdateAfterDrag = true;
		Position = ContentUIItem->RelativeLocation;
		DragSpeed.X = localMoveDelta.X;
	}
	if (AllowVerticalScroll)
	{
		CanUpdateAfterDrag = true;
		Position = ContentUIItem->RelativeLocation;
		DragSpeed.Y = localMoveDelta.Y;
	}
	return AllowEventBubbleUp;
}
bool UUIScrollViewComponent::OnPointerScroll_Implementation(const FLGUIPointerEventData& eventData)
{
	if (CheckParameters() && CheckValidHit(eventData.hitComponent))
	{
		auto delta = eventData.scrollAxisValue * -ScrollSensitivity;
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
			ContentUIItem->SetUIRelativeLocation(Position);
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
			ContentUIItem->SetUIRelativeLocation(Position);
		}
	}
	return AllowEventBubbleUp;
}

void UUIScrollViewComponent::UpdateAfterDrag(float deltaTime)
{
	if (FMath::Abs(DragSpeed.X) > 0 || FMath::Abs(DragSpeed.Y) > 0//speed large then threshold
		|| (AllowHorizontalScroll && (Position.X < HorizontalRange.X || Position.X > HorizontalRange.Y))//horizontal out of range
		|| (AllowVerticalScroll && (Position.Y < VerticalRange.X || Position.Y > VerticalRange.Y)))//vertical out of range
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
					if (HorizontalRange.X - Position.X < KINDA_SMALL_NUMBER)
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
				if (DragSpeed.X > 0)//move right, use opposite force
				{
					float dragForce = (Position.X - HorizontalRange.Y) * dragForceMulitply;
					DragSpeed.X += -FMath::Sign(DragSpeed.X) * dragForce * deltaTime;

					Position.X += DragSpeed.X * deltaTime * positionTimeMultiply;
					canMove = true;
				}
				else
				{
					DragSpeed.X = 0;
					if (Position.X - HorizontalRange.Y < KINDA_SMALL_NUMBER)
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
				if (FMath::Sign(DragSpeed.X) != speedXDir)//accelerate speed change speed direction, set speed to 0
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
					if (VerticalRange.X - Position.Y < KINDA_SMALL_NUMBER)
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
				if (DragSpeed.Y > 0)//move up, use opposite force
				{
					float dragForce = (Position.Y - VerticalRange.Y) * dragForceMulitply;
					DragSpeed.Y += -FMath::Sign(DragSpeed.Y) * dragForce * deltaTime;

					Position.Y += DragSpeed.Y * deltaTime * positionTimeMultiply;
					canMove = true;
				}
				else
				{
					DragSpeed.Y = 0;
					if (Position.Y - VerticalRange.Y < KINDA_SMALL_NUMBER)
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
				if (FMath::Sign(DragSpeed.Y) != speedYDir)//accelerate speed change speed direction, set speed to 0
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
			ContentUIItem->SetUIRelativeLocation(Position);
		}
	}
	else
	{
		CanUpdateAfterDrag = false;
	}
}
void UUIScrollViewComponent::UpdateProgress(bool InFireEvent)
{
	if (ContentUIItem == nullptr)return;
	auto& relativeLocation = ContentUIItem->RelativeLocation;
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
		if (OnScrollCPP.IsBound()) OnScrollCPP.Broadcast(Progress);
		OnScroll.FireEvent(Progress);
	}
}

void UUIScrollViewComponent::CalculateHorizontalRange()
{
	auto& parentWidget = ContentParentUIItem->GetWidget();
	auto& thisWidget = ContentUIItem->GetWidget();
	if (parentWidget.width > thisWidget.width)
	{
		HorizontalRange.X = HorizontalRange.Y = ContentUIItem->RelativeLocation.X;
		//parent
		HorizontalRange.X = -parentWidget.pivot.X * parentWidget.width;
		HorizontalRange.Y = (1.0f - parentWidget.pivot.X) * parentWidget.width;
		//selft
		HorizontalRange.X += thisWidget.pivot.X * parentWidget.width;
		HorizontalRange.Y += (thisWidget.pivot.X - 1) * parentWidget.width;
	}
	else
	{
		//self
		HorizontalRange.X = (thisWidget.pivot.X - 1) * thisWidget.width;
		HorizontalRange.Y = thisWidget.pivot.X * thisWidget.width;
		//parent
		HorizontalRange.X += (1 - parentWidget.pivot.X) * parentWidget.width;
		HorizontalRange.Y += -parentWidget.pivot.X * parentWidget.width;
	}
}
void UUIScrollViewComponent::CalculateVerticalRange()
{
	auto& parentWidget = ContentParentUIItem->GetWidget();
	auto& thisWidget = ContentUIItem->GetWidget();
	if (parentWidget.height > thisWidget.height)
	{
		//parent
		VerticalRange.X = -parentWidget.pivot.Y * parentWidget.height;
		VerticalRange.Y = (1.0f - parentWidget.pivot.Y) * parentWidget.height;
		//self
		VerticalRange.X += thisWidget.pivot.Y * parentWidget.height;
		VerticalRange.Y += (thisWidget.pivot.Y - 1) * parentWidget.height;
	}
	else
	{
		//self
		VerticalRange.X = (thisWidget.pivot.Y - 1) * thisWidget.height;
		VerticalRange.Y = thisWidget.pivot.Y * thisWidget.height;
		//parent
		VerticalRange.X += (1 - parentWidget.pivot.Y) * parentWidget.height;
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


void UUIScrollViewComponent::RegisterScrollEvent(const FLGUIVector2Delegate& InDelegate)
{
	OnScrollCPP.Add(InDelegate);
}
void UUIScrollViewComponent::UnregisterScrollEvent(const FLGUIVector2Delegate& InDelegate)
{
	OnScrollCPP.Remove(InDelegate.GetHandle());
}

FLGUIDelegateHandleWrapper UUIScrollViewComponent::RegisterScrollEvent(const FLGUIScrollViewDynamicDelegate& InDelegate)
{
	auto delegateHandle = OnScrollCPP.AddLambda([InDelegate](FVector2D Progress) {
		if (InDelegate.IsBound())InDelegate.Execute(Progress);
	});
	return FLGUIDelegateHandleWrapper(delegateHandle);
}
void UUIScrollViewComponent::UnregisterScrollEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle)
{
	OnScrollCPP.Remove(InDelegateHandle.DelegateHandle);
}
