// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIEditHelperButtonCustomization.h"
#include "LGUIEditorUtils.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "LGUIEditHelperButtonCustomization"

TSharedRef<IPropertyTypeCustomization> FLGUIEditHelperButtonCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIEditHelperButtonCustomization);
}
void FLGUIEditHelperButtonCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	auto propertyDisplayName = PropertyHandle->GetPropertyDisplayName();
	int32 clickCount = 0;
	TSharedPtr<IPropertyHandle> clickCountHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEditHelperButton, clickCount));
	clickCountHandle->GetValue(clickCount);
	TSharedPtr<IPropertyUtilities> PropertyUtilites = CustomizationUtils.GetPropertyUtilities();
	ChildBuilder.AddCustomRow(FText::FromString("Button"))
	.WholeRowContent()
	[
		SNew(SButton)
		.HAlign(EHorizontalAlignment::HAlign_Center)
		.Text(propertyDisplayName)
		.OnClicked_Lambda([=]{
			int32 newClickCount = clickCount+1;
			clickCountHandle->SetValue(newClickCount);
			PropertyUtilites->ForceRefresh();
			return FReply::Handled();
		})
	];
}
#undef LOCTEXT_NAMESPACE