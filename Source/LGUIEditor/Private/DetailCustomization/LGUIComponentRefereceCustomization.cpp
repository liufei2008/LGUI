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
	PropertyUtilites = CustomizationUtils.GetPropertyUtilities();
	bool isInWorld = false;
	FCommentNodeSet nodeSet;
	PropertyHandle->GetOuterObjects(nodeSet);
	for (UObject* obj : nodeSet)
	{
		isInWorld = obj->GetWorld() != nullptr;
		break;
	}

	ComponentNameProperty = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, targetComonentName));
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
	FName componentName = NAME_None;
	ComponentNameProperty->GetValue(componentName);
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
				auto components = targetActor->GetComponentsByClass(targetCompClass);
				if (components.Num() == 0)
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
				else if (components.Num() == 1 && componentName.IsNone())
				{
					contentWidget = targetActorHandle->CreatePropertyValueWidget();
				}
				else
				{
					contentWidget = SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						targetActorHandle->CreatePropertyValueWidget()
					]
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SComboButton)
						.ToolTipText(FText::FromString(FString::Printf(TEXT("Target actor have multiple components of type:%s, you can select one from target actor"), *targetCompClass->GetName())))
						.OnGetMenuContent(this, &FLGUIComponentRefereceCustomization::OnGetMenu, components)
						.ContentPadding(FMargin(0))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(this, &FLGUIComponentRefereceCustomization::GetButtonText)
						]
					]
					;
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
TSharedRef<SWidget> FLGUIComponentRefereceCustomization::OnGetMenu(TArray<UActorComponent*> Components)
{
	FMenuBuilder MenuBuilder(true, nullptr);
	//MenuBuilder.BeginSection(FName(), LOCTEXT("Components", "Components"));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromName(FName(NAME_None)),
			FText(LOCTEXT("Tip", "Clear component selection, will use first one.")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FLGUIComponentRefereceCustomization::OnSelectComponent, FName(NAME_None)))
		);
		for (auto comp : Components)
		{
			MenuBuilder.AddMenuEntry(
				FText::FromString(comp->GetName()),
				FText(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FLGUIComponentRefereceCustomization::OnSelectComponent, comp->GetFName()))
			);
		}
	}
	//MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}
void FLGUIComponentRefereceCustomization::OnSelectComponent(FName CompName)
{
	ComponentNameProperty->SetValue(CompName);
	PropertyUtilites->ForceRefresh();
}
FText FLGUIComponentRefereceCustomization::GetButtonText()const
{
	FName ComponentName;
	ComponentNameProperty->GetValue(ComponentName);
	return FText::FromName(ComponentName);
}
#undef LOCTEXT_NAMESPACE