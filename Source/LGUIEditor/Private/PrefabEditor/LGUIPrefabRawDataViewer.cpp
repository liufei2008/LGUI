// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "LGUIPrefabRawDataViewer.h"
#include "PrefabSystem/LGUIPrefab.h"
#include "LGUIPrefabEditor.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabRawDataViewer"

void SLGUIPrefabRawDataViewer::Construct(const FArguments& InArgs, TSharedPtr<FLGUIPrefabEditor> InPrefabEditorPtr, UObject* InObject)
{
	PrefabEditorPtr = InPrefabEditorPtr;
	FPropertyEditorModule& EditModule = FModuleManager::Get().GetModuleChecked<FPropertyEditorModule>("PropertyEditor");
	FDetailsViewArgs DetailsViewArgs;
	{
		DetailsViewArgs.bAllowSearch = false;
		DetailsViewArgs.bShowOptions = false;
		DetailsViewArgs.bAllowMultipleTopLevelObjects = false;
		DetailsViewArgs.bAllowFavoriteSystem = false;
		DetailsViewArgs.bHideSelectionTip = true;
	}
	DescriptorDetailView = EditModule.CreateDetailView(DetailsViewArgs);
	DescriptorDetailView->SetObject(InObject);
	ChildSlot
		[
			DescriptorDetailView.ToSharedRef()
		];
}

#undef LOCTEXT_NAMESPACE
