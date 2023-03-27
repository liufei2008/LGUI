// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Window/LGUIDynamicSpriteAtlasViewer.h"
#include "Widgets/Docking/SDockTab.h"
#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/LGUIDynamicSpriteAtlasData.h"
#include "LGUIEditorModule.h"
#include "ISinglePropertyView.h"

#define LOCTEXT_NAMESPACE "LGUIDynamicSpriteAtlasViewer"

void SLGUIDynamicSpriteAtlasViewer::Construct(const FArguments& Args, TSharedPtr<SDockTab> InOwnerTab)
{
	InOwnerTab->SetOnTabClosed(SDockTab::FOnTabClosedCallback::CreateSP(this, &SLGUIDynamicSpriteAtlasViewer::CloseTabCallback));
	if (ULGUIDynamicSpriteAtlasManager::Instance != nullptr)
	{
		FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
		FDetailsViewArgs DetailsViewArgs;
		{
			DetailsViewArgs.bAllowSearch = false;
			DetailsViewArgs.bShowOptions = false;
			DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
			DetailsViewArgs.bAllowFavoriteSystem = false;
			DetailsViewArgs.bHideSelectionTip = true;
		}
		//FSinglePropertyParams SinglePropertyParams;
		//TSharedPtr<ISinglePropertyView> Property = EditModule.CreateSingleProperty(ULGUIAtlasManager::Instance, TEXT("atlasMap"), SinglePropertyParams);
		//Property->SetObject(ULGUIAtlasManager::Instance);

		TSharedPtr<IDetailsView> DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
		DescriptorDetailView->SetObject(ULGUIDynamicSpriteAtlasManager::Instance);

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
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];
	}
}

void SLGUIDynamicSpriteAtlasViewer::CloseTabCallback(TSharedRef<SDockTab> TabClosed)
{
	
}
#undef LOCTEXT_NAMESPACE