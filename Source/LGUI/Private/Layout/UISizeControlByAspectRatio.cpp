// Copyright 2019-2022 LexLiu. All Rights Reserved.

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
	if (!GetEnable())return;

	if (CheckRootUIComponent() && GetEnable())
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

bool UUISizeControlByAspectRatio::GetCanLayoutControlAnchor_Implementation(class UUIItem* InUIItem, FLGUICanLayoutControlAnchor& OutResult)const
{
	if (this->GetRootUIComponent() == InUIItem)
	{
		OutResult.bCanControlHorizontalAnchor = OutResult.bCanControlVerticalAnchor =
			(ControlMode == EUISizeControlByAspectRatioMode::FitInParent
				|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
				) && this->GetEnable();
		OutResult.bCanControlHorizontalSizeDelta =
			(ControlMode == EUISizeControlByAspectRatioMode::HeightControlWidth
				|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
				|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
				) && this->GetEnable();
		OutResult.bCanControlVerticalSizeDelta =
			(ControlMode == EUISizeControlByAspectRatioMode::WidthControlHeight
				|| ControlMode == EUISizeControlByAspectRatioMode::FitInParent
				|| ControlMode == EUISizeControlByAspectRatioMode::EnvelopeParent
				) && this->GetEnable();
		return true;
	}
	else
	{
		return false;
	}
}
