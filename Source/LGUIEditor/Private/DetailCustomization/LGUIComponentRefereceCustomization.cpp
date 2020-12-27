// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIComponentRefereceCustomization.h"
#include "LGUIEditorUtils.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"

#define LOCTEXT_NAMESPACE "LGUIComponentRefereceHelperCustomization"

TWeakObjectPtr<AActor> FLGUIComponentRefereceCustomization::CopiedTargetActor;
FName FLGUIComponentRefereceCustomization::CopiedTargetComponentName;

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
			contentWidget =
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.ColorAndOpacity(FSlateColor(FLinearColor::Red))
					.AutoWrapText(true)
					.Font(IDetailLayoutBuilder::GetDetailFont())
					.Text(LOCTEXT("ComponnetCheckTip", "You must set your component class in variable declaration!"))
				];
		}
		else
		{
			if (!IsValid(targetActor))
			{
				contentWidget = targetActorHandle->CreatePropertyValueWidget();
			}
			else
			{
				TArray<UActorComponent*> components;
				targetActor->GetComponents(targetCompClass, components);
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
						.Font(IDetailLayoutBuilder::GetDetailFont())
					];
				}
				else if (components.Num() == 1)
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
							.Text(this, &FLGUIComponentRefereceCustomization::GetButtonText, components)
							.Font(IDetailLayoutBuilder::GetDetailFont())
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
	]
	.CopyAction(FUIAction
	(
		FExecuteAction::CreateSP(this, &FLGUIComponentRefereceCustomization::OnCopy, targetActor, targetCompClass, componentName),
		FCanExecuteAction::CreateLambda([=] {return isInWorld; })
	))
	.PasteAction(FUIAction
	(
		FExecuteAction::CreateSP(this, &FLGUIComponentRefereceCustomization::OnPaste, targetActorHandle, targetCompClassHandle, ComponentNameProperty),
		FCanExecuteAction::CreateLambda([=] {return isInWorld; })
	))
	;
}
void FLGUIComponentRefereceCustomization::OnCopy(AActor* actor, UClass* compClass, FName compName)
{
	CopiedTargetActor = actor;
	CopiedTargetComponentName = compName;
}
void FLGUIComponentRefereceCustomization::OnPaste(TSharedPtr<IPropertyHandle> actorProperty, TSharedPtr<IPropertyHandle> compClassProperty, TSharedPtr<IPropertyHandle> compNameProperty)
{
	actorProperty->SetValue((UObject*)CopiedTargetActor.Get());
	compNameProperty->SetValue(CopiedTargetComponentName);
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
			if (comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;
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
FText FLGUIComponentRefereceCustomization::GetButtonText(TArray<UActorComponent*> Components)const
{
	FName ComponentName;
	ComponentNameProperty->GetValue(ComponentName);

	if (ComponentName.IsValid() && !ComponentName.IsNone())
	{
		bool foundComponentByName = false;
		for (auto comp : Components)
		{
			if (comp->GetFName() == ComponentName)
			{
				foundComponentByName = true;
				break;
			}
		}
		if (foundComponentByName)
		{
			return FText::FromName(ComponentName);
		}
		else
		{
			return FText::FromString(FString::Printf(TEXT("%s(Missing)"), *ComponentName.ToString()));
		}
	}
	else
	{
		return FText::FromString(FString(TEXT("None")));
	}
}
#undef LOCTEXT_NAMESPACE