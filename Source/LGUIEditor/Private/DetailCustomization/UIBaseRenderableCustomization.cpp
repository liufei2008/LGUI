// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "DetailCustomization/UIBaseRenderableCustomization.h"
#include "Core/ActorComponent/UIBaseRenderable.h"
#include "LGUIEditorModule.h"
#include "DetailLayoutBuilder.h"
#include "DetailCategoryBuilder.h"

#define LOCTEXT_NAMESPACE "UIBaseRenderableCustomization"
FUIBaseRenderableCustomization::FUIBaseRenderableCustomization()
{
}

FUIBaseRenderableCustomization::~FUIBaseRenderableCustomization()
{
	
}

TSharedRef<IDetailCustomization> FUIBaseRenderableCustomization::MakeInstance()
{
	return MakeShareable(new FUIBaseRenderableCustomization);
}
void FUIBaseRenderableCustomization::CustomizeDetails(IDetailLayoutBuilder& DetailBuilder)
{
	auto RaycastTypeHandle = DetailBuilder.GetProperty(GET_MEMBER_NAME_CHECKED(UUIBaseRenderable, RaycastType));
	RaycastTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=, &DetailBuilder] {
		DetailBuilder.ForceRefreshDetails();
		}));
}
#undef LOCTEXT_NAMESPACE