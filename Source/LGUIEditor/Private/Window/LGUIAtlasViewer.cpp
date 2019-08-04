// Copyright 2019 LexLiu. All Rights Reserved.

#include "Window/LGUIAtlasViewer.h"
#include "Widgets/Docking/SDockTab.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUIAtlasData.h"
#include "LGUIEditorModule.h"

#define LOCTEXT_NAMESPACE "LGUIAtlasViewer"

void SLGUIAtlasViewer::Construct(const FArguments& Args, TSharedPtr<SDockTab> InOwnerTab)
{
	OwnerTab = InOwnerTab;
	InOwnerTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &SLGUIAtlasViewer::CloseTabCallback));
	if (ULGUIAtlasManager::Instance != nullptr)
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.bAllowSearch = false;
			DetailsViewArgs.bShowOptions = false;
			DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
			DetailsViewArgs.bAllowFavoriteSystem = false;
			DetailsViewArgs.bShowActorLabel = false;
			DetailsViewArgs.bHideSelectionTip = true;
		}
		TSharedPtr<IDetailsView> DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
		DescriptorDetailView->SetObject(ULGUIAtlasManager::Instance);

		ChildSlot
			[
				DescriptorDetailView.ToSharedRef()
			];
	}
	else
	{
		ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.AutoHeight()
			[
				SNew(STextBlock)
				.AutoWrapText(true)
				.Text(LOCTEXT("NoneAtlas", "None atlas texture here because none sprite have packed. Sprite will be packed when it get renderred"))
			]
		];
	}
}

TSharedRef<ITableRow> SLGUIAtlasViewer::CreateContentIconTitle(TSharedPtr<FLGUIPackedAtlasTextureViewerItem> ContentSource, const TSharedRef<STableViewBase>& OwnerTable)
{
	auto SpriteSlateBrush = TSharedPtr<FSlateBrush>(new FSlateBrush);
	SpriteSlateBrush->SetResourceObject(ContentSource->Texture);
	SpriteSlateBrushArray.Add(SpriteSlateBrush);
	return SNew(STableRow<TSharedPtr<FString>>, OwnerTable)
		[
			SNew(SVerticalBox)
			+SVerticalBox::Slot()
			.FillHeight(ThumbnailSize)
			.Padding(3)
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
					SNew(SImage)
					.Image(SpriteSlateBrush.Get())
				]
			]
			+ SVerticalBox::Slot()
			.AutoHeight()
			.Padding(FMargin(3))
			[
				SNew(SBox)
				.HeightOverride(20)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FString::Printf(TEXT("PackingTag:%s"), *(ContentSource->PackingTag.ToString()))))
				]
			]
		];
}
void SLGUIAtlasViewer::CloseTabCallback(TSharedRef<SDockTab> TabClosed)
{
	OwnerTab = nullptr;//need to clear pointer reference, otherwise the window cannot be opened. posiblly bacause ue think the window is still open
	SpriteSlateBrushArray.Empty();
}
void SLGUIAtlasViewer::OnMouseButtonDoubleClicked(TSharedPtr<FLGUIPackedAtlasTextureViewerItem> Item)
{
	FAssetEditorManager::Get().OpenEditorForAsset(Item->Texture, EToolkitMode::Standalone);
}
#undef LOCTEXT_NAMESPACE