// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "LGUIPrefabRawDataViewer.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabEditor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabRawDataViewer"

void SLGUIPrefabRawDataViewer::Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr, ULGUIPrefab* InPrefab)
{
	PrefabEditorPtr = InPrefabEditorPtr;
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
	DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
	DescriptorDetailView->SetObject(InPrefab);
	ChildSlot
		[
			DescriptorDetailView.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE
