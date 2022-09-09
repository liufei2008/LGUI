// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/UIPostProcessRenderableCustomization.h"
#include "LGUIEditorUtils.h"
#include "Core/ActorComponent/UIPostProcessRenderable.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"
#include "IDetailGroup.h"

#define LOCTEXT_NAMESPACE "UIPostProcessRenderableCustomization"
FUIPostProcessRenderableCustomization::FUIPostProcessRenderableCustomization()
{
}

FUIPostProcessRenderableCustomization::~FUIPostProcessRenderableCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUIPostProcessRenderableCustomization::MakeInstance()
{
	return MakeShareable(new FUIPostProcessRenderableCustomization);
}
void FUIPostProcessRenderableCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	TArray<TWeakObjectPtr<UObject>> targetObjects;
	DetailBuilder.GetObjectsBeingCustomized(targetObjects);
	TargetScriptArray.Empty();
	for (auto item : targetObjects)
	{
		if (auto validItem = Cast<UUIPostProcessRenderable>(item.Get()))
		{
			TargetScriptArray.Add(validItem);
		}
	}
	if (TargetScriptArray.Num() == 0)
	{
		UE_LOG(LGUIEditor, Log, TEXT("[UITextureCustomization]Get TargetScript is null"));
		return;
	}

	IDetailCategoryBuilder& LGUICategory = DetailBuilder.EditCategory("LGUI");
	TArray<FName> NeedToHidePropertyNames;
	auto MaskTextureHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIPostProcessRenderable, maskTexture));
	MaskTextureHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([&]() {
		DetailBuilder.ForceRefreshDetails();
		}));
	IDetailGroup& MaskTextureGroup = LGUICategory.AddGroup(FName("MaskTexture"), LOCTEXT("MaskTexture", "MaskTexture"));
	MaskTextureGroup.HeaderProperty(MaskTextureHandle);
	MaskTextureGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIPostProcessRenderable, MaskTextureType)));
	MaskTextureGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIPostProcessRenderable, MaskTextureSpriteInfo)));
	MaskTextureGroup.AddPropertyRow(DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIPostProcessRenderable, MaskTextureUVRect)));
}

#undef LOCTEXT_NAMESPACE