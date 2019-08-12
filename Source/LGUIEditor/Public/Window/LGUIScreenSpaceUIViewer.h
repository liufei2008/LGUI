// Copyright 2019 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Engine/TextureRenderTarget2D.h"
#pragma once

/**
 * 
 */
class SLGUIScreenSpaceUIViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIScreenSpaceUIViewer) {}
	
	SLATE_END_ARGS()
	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);
	static TWeakObjectPtr<UTextureRenderTarget2D> CurrentScreenSpaceUIRenderTarget;
	static TWeakObjectPtr<class UUIRoot> CurrentUIRoot;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)override;
private:
	TWeakPtr<SDockTab> OwnerTab;

	void CloseTabCallback(TSharedRef<SDockTab> TabClosed);

	TSharedPtr<SBox> RootImageBox;
	UMaterialInstanceDynamic* DynamicMaterial = nullptr;
	FOptionalSize GetMinDesiredHeight()const;
	FOptionalSize GetImageWidth()const;
	FOptionalSize GetImageHeight()const;
};
