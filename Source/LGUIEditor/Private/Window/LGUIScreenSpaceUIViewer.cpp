// Copyright 2019 LexLiu. All Rights Reserved.

#include "Window/LGUIScreenSpaceUIViewer.h"
#include "Widgets/Docking/SDockTab.h"
#include "SlateMaterialBrush.h"
#include "Styling/SlateBrush.h"
#include "LGUIEditorModule.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"

#define LOCTEXT_NAMESPACE "LGUIAtlasViewer"

TWeakObjectPtr<UTextureRenderTarget2D> SLGUIScreenSpaceUIViewer::CurrentScreenSpaceUIRenderTarget = nullptr;
TWeakObjectPtr<class UUIRoot> SLGUIScreenSpaceUIViewer::CurrentUIRoot = nullptr;

void SLGUIScreenSpaceUIViewer::Construct(const FArguments& Args, TSharedPtr<SDockTab> InOwnerTab)
{
	this->SetCanTick(true);
	OwnerTab = InOwnerTab;
	InOwnerTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &SLGUIScreenSpaceUIViewer::CloseTabCallback));
	if (!CurrentScreenSpaceUIRenderTarget.IsValid())
	{
		ChildSlot
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("InValidTextureTarget", "This window should open from UIRoot component"))
			]
		;
		return;
	}

	FString matPath = TEXT("/LGUI/OverlayBySceneCaptureMaterial");
	auto mat = LoadObject<UMaterialInterface>(CurrentUIRoot.Get(), *matPath);
	if (mat == nullptr)
	{
		ChildSlot
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("MissingMaterial", "Load material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issure."))
			]
		;
		return;
	}

	auto MaterialBrush = new FSlateMaterialBrush(*mat, FVector2D(1920, 1080));
	UObject* Resource = MaterialBrush->GetResourceObject();

	DynamicMaterial = Cast<UMaterialInstanceDynamic>(Resource);
	if (DynamicMaterial == nullptr)
	{
		DynamicMaterial = UMaterialInstanceDynamic::Create(mat, CurrentUIRoot.Get());
		MaterialBrush->SetResourceObject(DynamicMaterial);
	}
	DynamicMaterial->SetTextureParameterValue("MainTexture", CurrentScreenSpaceUIRenderTarget.Get());

	ChildSlot
	[
		SAssignNew(RootImageBox, SBox)
		.HeightOverride(this, &SLGUIScreenSpaceUIViewer::GetMinDesiredHeight)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.AutoHeight()
			.VAlign(EVerticalAlignment::VAlign_Center)
			.HAlign(EHorizontalAlignment::HAlign_Center)
			[
				SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(EHorizontalAlignment::HAlign_Center)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SOverlay)
					+SOverlay::Slot()
					[
						SNew(SImage)
						.Image(FEditorStyle::GetBrush("Checkerboard"))
						.ColorAndOpacity(FSlateColor(FLinearColor(0.15f, 0.15f, 0.15f)))
					]
					+SOverlay::Slot()
					[
						SNew(SBox)
						.WidthOverride(this, &SLGUIScreenSpaceUIViewer::GetImageWidth)
						.HeightOverride(this, &SLGUIScreenSpaceUIViewer::GetImageHeight)
						[
							SNew(SImage)
							.Image(MaterialBrush)
						]
					]
				]
			]
		]
	]
	;
}
void SLGUIScreenSpaceUIViewer::Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime)
{
	if (!CurrentScreenSpaceUIRenderTarget.IsValid() || !CurrentUIRoot.IsValid())
	{
		OwnerTab.Pin()->RequestCloseTab();
		this->SetCanTick(false);
	}
}

void SLGUIScreenSpaceUIViewer::CloseTabCallback(TSharedRef<SDockTab> TabClosed)
{
	
}

FOptionalSize SLGUIScreenSpaceUIViewer::GetMinDesiredHeight()const
{
	return OwnerTab.Pin()->GetCachedGeometry().GetLocalSize().Y;
}
FOptionalSize SLGUIScreenSpaceUIViewer::GetImageWidth()const
{
	if (!CurrentScreenSpaceUIRenderTarget.IsValid())return 0;
	float imageAspect = (float)(CurrentScreenSpaceUIRenderTarget->SizeX) / CurrentScreenSpaceUIRenderTarget->SizeY;
	auto imageBoxSize = RootImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return imageBoxSize.X;
	}
	else
	{
		return imageBoxSize.Y * imageAspect;
	}
}
FOptionalSize SLGUIScreenSpaceUIViewer::GetImageHeight()const
{
	if (!CurrentScreenSpaceUIRenderTarget.IsValid())return 0;
	float imageAspect = (float)(CurrentScreenSpaceUIRenderTarget->SizeX) / CurrentScreenSpaceUIRenderTarget->SizeY;
	auto imageBoxSize = RootImageBox->GetCachedGeometry().GetLocalSize();
	float imageBoxAspect = (float)(imageBoxSize.X / imageBoxSize.Y);
	if (imageAspect > imageBoxAspect)
	{
		return imageBoxSize.X / imageAspect;
	}
	else
	{
		return imageBoxSize.Y;
	}
}
#undef LOCTEXT_NAMESPACE