// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByAspectRatio.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

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
#if WITH_EDITOR
bool UUISizeControlByAspectRatio::CanControlChildAnchor()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildAnchorOffsetX()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildAnchorOffsetY()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfAnchorOffsetX()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfAnchorOffsetY()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildWidth()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlChildHeight()
{
	return false;
}
bool UUISizeControlByAspectRatio::CanControlSelfHorizontalAnchor()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfVerticalAnchor()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfWidth()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::HeightControlWidth
		|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfHeight()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::WidthControlHeight
		|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfStrengthLeft()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfStrengthRight()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfStrengthTop()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
bool UUISizeControlByAspectRatio::CanControlSelfStrengthBottom()
{
	return (ControlMode == EUISizeControlByAspectRatioMode::FitInParent
		|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
		) && enable
		;
}
#endif
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