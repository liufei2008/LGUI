// Copyright 2019 LexLiu. All Rights Reserved.

#include "Layout/UISizeControlByAspectRatio.h"
#include "LGUI.h"
#include "Core/ActorComponent/UIItem.h"

UUISizeControlByAspectRatio::UUISizeControlByAspectRatio()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUISizeControlByAspectRatio::OnRebuildLayout()
{
	if (CheckRootUIComponent())
	{
		switch (ControlMode)
		{
		case EUISizeControlByAspectRatioMode::HeightControlWidth:
		{
			auto height = RootUIComp->GetHeight();
			if (RootUIComp->GetAnchorHAlign() == UIAnchorHorizontalAlign::Stretch)
			{
				RootUIComp->SetAnchorHAlign(UIAnchorHorizontalAlign::Center);
			}
			RootUIComp->SetWidth(height * AspectRatio);
		}
		break;
		case EUISizeControlByAspectRatioMode::WidthControlHeight:
		{
			auto width = RootUIComp->GetWidth();
			if (RootUIComp->GetAnchorVAlign() == UIAnchorVerticalAlign::Stretch)
			{
				RootUIComp->SetAnchorVAlign(UIAnchorVerticalAlign::Middle);
			}
			RootUIComp->SetHeight(width / AspectRatio);
		}
		break;
		}
	}
}

bool UUISizeControlByAspectRatio::CanControlChildAnchor()
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
	return ControlMode == EUISizeControlByAspectRatioMode::HeightControlWidth;
}
bool UUISizeControlByAspectRatio::CanControlSelfVerticalAnchor()
{
	return ControlMode == EUISizeControlByAspectRatioMode::WidthControlHeight;
}
bool UUISizeControlByAspectRatio::CanControlSelfWidth()
{
	return ControlMode == EUISizeControlByAspectRatioMode::HeightControlWidth;
}
bool UUISizeControlByAspectRatio::CanControlSelfHeight()
{
	return ControlMode == EUISizeControlByAspectRatioMode::WidthControlHeight;
}

void UUISizeControlByAspectRatio::SetControlMode(EUISizeControlByAspectRatioMode value)
{
	if (ControlMode != value)
	{
		ControlMode = value;
		OnRebuildLayout();
	}
}
void UUISizeControlByAspectRatio::SetAspectRatio(float value)
{
	if (AspectRatio != value)
	{
		AspectRatio = value;
		OnRebuildLayout();
	}
}