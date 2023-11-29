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

}
#undef LOCTEXT_NAMESPACE