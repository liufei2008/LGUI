// Copyright 2019 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIComponentRefereceCustomization.h"
#include "LGUIEditorUtils.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "LGUIComponentRefereceHelperCustomization"

TSharedRef<IPropertyTypeCustomization> FLGUIComponentRefereceCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIComponentRefereceCustomization);
}
void FLGUIComponentRefereceCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	TSharedPtr<IPropertyUtilities> PropertyUtilites = CustomizationUtils.GetPropertyUtilities();
	bool isInWorld = false;
	FCommentNodeSet nodeSet;
	PropertyHandle->GetOuterObjects(nodeSet);
	for (UObject* obj : nodeSet)
	{
		isInWorld = obj->GetWorld() != nullptr;
		break;
	}

	TSharedPtr<IPropertyHandle> targetActorHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, targetActor));
	targetActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] {
		PropertyUtilites->ForceRefresh();
	}));
	TSharedPtr<IPropertyHandle> targetCompClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, targetComponentClass));
	UObject* targetActorObj = nullptr;
	AActor* targetActor = nullptr;
	targetActorHandle->GetValue(targetActorObj);
	if (targetActorObj)
	{
		targetActor = (AActor*)targetActorObj;
	}
	UObject* targetCompClassObj = nullptr;
	UClass* targetCompClass = nullptr;
	targetCompClassHandle->GetValue(targetCompClassObj);
	if (targetCompClassObj)
	{
		targetCompClass = (UClass*)targetCompClassObj;
	}
	TSharedPtr<SWidget> contentWidget;
	if (isInWorld)
	{
		if (!IsValid(targetCompClass))
		{
			contentWidget = SNew(STextBlock)
				.ColorAndOpacity(FSlateColor(FLinearColor::Red))
				.AutoWrapText(true)
				.Text(LOCTEXT("ComponnetCheckTip", "You must assign your component class!"));
		}
		else
		{
			if (!IsValid(targetActor))
			{
				contentWidget = targetActorHandle->CreatePropertyValueWidget();
			}
			else
			{
				if (auto comp = targetActor->FindComponentByClass(targetCompClass))
				{
					contentWidget = targetActorHandle->CreatePropertyValueWidget();
				}
				else
				{
					contentWidget = SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							targetActorHandle->CreatePropertyValueWidget()
						]
					+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.ColorAndOpacity(FSlateColor(FLinearColor::Red))
						.AutoWrapText(true)
						.Text(LOCTEXT("ComponentNotFoundTip", "Component not found on target actor!"))
						];
				}
			}
		}
	}
	else
	{
		contentWidget = targetCompClassHandle->CreatePropertyValueWidget();
	}

	ChildBuilder.AddCustomRow(FText::FromString("ActorComponent"))
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(500)
	[
		contentWidget.ToSharedRef()
	];
}
#undef LOCTEXT_NAMESPACE