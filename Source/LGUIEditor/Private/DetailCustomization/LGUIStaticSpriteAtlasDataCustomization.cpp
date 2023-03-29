// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIStaticSpriteAtlasDataCustomization.h"
#include "Core/LGUIStaticSpriteAtlasData.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "LGUIStaticSpriteAtlasDataCustomization"

TSharedRef<IDetailCustomization> FLGUIStaticSpriteAtlasDataCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIStaticSpriteAtlasDataCustomization);
}

void FLGUIStaticSpriteAtlasDataCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptPtr = Cast<ULGUIStaticSpriteAtlasData>(targetObjects[0].Get());
	if (TargetScriptPtr == nullptr)
	{
		UE_LOG(LGUIEditor, Log, TEXT("Get TargetScript is null"));
		return;
	}
	IDetailCategoryBuilder& LguiCategory = DetailBuilder.EditCategory("LGUI");
	LguiCategory.AddProperty(GET_MEMBER_NAME_CHECKED(ULGUIStaticSpriteAtlasData, spriteArray));
	LguiCategory.AddCustomRow(LOCTEXT("PackPreviewButtonRow", "Pack Preview"))
		.ValueContent()
		[
			SNew(SButton)
			.Text(LOCTEXT("PackPreviewButton", "Pack Preview"))
			.HAlign(EHorizontalAlignment::HAlign_Center)
			.OnClicked_Lambda([=] {
				TargetScriptPtr->PackAtlas();
				TargetScriptPtr->MarkPackageDirty();
				return FReply::Handled();
			})
		];
}

#undef LOCTEXT_NAMESPACE