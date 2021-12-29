// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByAspectRatio.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

void UUISizeControlByAspectRatio::SetControlMode(EUISizeControlByAspectRatioMode value)
{
	if (ControlMode != value)
	{
		ControlMode = value;
		MarkNeedRebuildLayout();
	}
}
void UUISizeControlByAspectRatio::SetAspectRatio(float value)
{
	if (AspectRatio != value)
	{
		AspectRatio = value;
		MarkNeedRebuildLayout();
	}
}

void UUISizeControlByAspectRatio::OnRebuildLayout()
{
	if (!CheckRootUIComponent())return;
	if (!enable)return;

	if (CheckRootUIComponent() && enable)
	{
		switch (ControlMode)
		{
		case EUISizeControlByAspectRatioMode::HeightControlWidth:
		{
			RootUIComp->SetWidth(RootUIComp->GetHeight() * AspectRatio);
		}
		break;
		case EUISizeControlByAspectRatioMode::WidthControlHeight:
		{
			RootUIComp->SetHeight(RootUIComp->GetWidth() / AspectRatio);
		}
		break;
		case EUISizeControlByAspectRatioMode::FitInParent:
		{
			if (auto parent = RootUIComp->GetParentUIItem())
			{
				RootUIComp->SetAnchorMin(FVector2D(0, 0));
				RootUIComp->SetAnchorMax(FVector2D(1, 1));

				auto parentWidth = parent->GetWidth();
				auto parentHeight = parent->GetHeight();
				auto parentAspectRatio = parentWidth / parentHeight;
				if (parentAspectRatio > AspectRatio)
				{
					auto SizeDelta = RootUIComp->GetSizeDelta();
					SizeDelta.X = -(parentWidth - parentHeight * AspectRatio);
					SizeDelta.Y = 0;
					RootUIComp->SetSizeDelta(SizeDelta);
				}
				else
				{
					auto SizeDelta = RootUIComp->GetSizeDelta();
					SizeDelta.Y = -(parentHeight - parentWidth / AspectRatio);
					SizeDelta.X = 0;
					RootUIComp->SetSizeDelta(SizeDelta);
				}
			}
		}
		break;
		case EUISizeControlByAspectRatioMode::EnvelopeParent:
		{
			if (auto parent = RootUIComp->GetParentUIItem())
			{
				RootUIComp->SetAnchorMin(FVector2D(0, 0));
				RootUIComp->SetAnchorMax(FVector2D(1, 1));

				auto parentWidth = parent->GetWidth();
				auto parentHeight = parent->GetHeight();
				auto parentAspectRatio = parentWidth / parentHeight;
				if (parentAspectRatio > AspectRatio)
				{
					auto SizeDelta = RootUIComp->GetSizeDelta();
					SizeDelta.Y = parentHeight - parentWidth / AspectRatio;
					SizeDelta.X = 0;
					RootUIComp->SetSizeDelta(SizeDelta);
				}
				else
				{
					auto SizeDelta = RootUIComp->GetSizeDelta();
					SizeDelta.X = parentWidth - parentHeight * AspectRatio;
					SizeDelta.Y = 0;
					RootUIComp->SetSizeDelta(SizeDelta);
				}
			}
		}
		break;
		}
	}
}

bool UUISizeControlByAspectRatio::CanControlChildAnchor_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildHorizontalAnchoredPosition_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildVerticalAnchoredPosition_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildWidth_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildHeight_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildAnchorLeft_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildAnchorRight_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildAnchorBottom_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildAnchorTop_Implementation()const
{
	return false;
}

bool UUISizeControlByAspectRatio::CanControlSelfAnchor_Implementation()const
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfHorizontalAnchoredPosition_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfVerticalAnchoredPosition_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfWidth_Implementation()const
{
	return (ControlMode == EUISizeControlByAspectRatioMode::HeightControlWidth
		|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfHeight_Implementation()const
{
	return (ControlMode == EUISizeControlByAspectRatioMode::WidthControlHeight
		|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfAnchorLeft_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfAnchorRight_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfAnchorBottom_Implementation()const
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfAnchorTop_Implementation()const
{
	return false;
}
