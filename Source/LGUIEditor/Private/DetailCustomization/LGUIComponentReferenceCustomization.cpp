﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIComponentReferenceCustomization.h"
#include "EdGraphNode_Comment.h"
#include "GameFramework/Actor.h"
#include "Components/ActorComponent.h"
#include "LGUIComponentReference.h"
#include "DetailLayoutBuilder.h"
#include "IDetailChildrenBuilder.h"

#define LOCTEXT_NAMESPACE "LGUIComponentRefereceHelperCustomization"

TWeakObjectPtr<AActor> FLGUIComponentReferenceCustomization::CopiedHelperActor;
TWeakObjectPtr<UActorComponent> FLGUIComponentReferenceCustomization::CopiedTargetComp;
UClass* FLGUIComponentReferenceCustomization::CopiedHelperClass;

TSharedRef<IPropertyTypeCustomization> FLGUIComponentReferenceCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIComponentReferenceCustomization);
}
void FLGUIComponentReferenceCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyUtilites = CustomizationUtils.GetPropertyUtilities();
	bool bIsInWorld = false;
	FCommentNodeSet NodeSet;
	PropertyHandle->GetOuterObjects(NodeSet);
	for (UObject* obj : NodeSet)
	{
		bIsInWorld = obj->GetWorld() != nullptr;
		break;
	}

	UObject* HelperClassObj = nullptr;
	UClass* HelperClass = nullptr;
	TSharedPtr<IPropertyHandle> HelperClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, HelperClass));
	HelperClassHandle->GetValue(HelperClassObj);
	if (HelperClassObj)
	{
		HelperClass = Cast<UClass>(HelperClassObj);
	}

	UObject* TargetCompObj = nullptr;
	UActorComponent* TargetComp = nullptr;
	auto TargetCompHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, TargetComp));
	TargetCompHandle->GetValue(TargetCompObj);
	if (TargetCompObj)
	{
		TargetComp = Cast<UActorComponent>(TargetCompObj);
	}

	auto HelperComponentNameHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, HelperComponentName));

	TSharedPtr<IPropertyHandle> HelperActorHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, HelperActor));
	HelperActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]() {OnHelperActorValueChange(HelperActorHandle, TargetCompHandle, HelperClassHandle, HelperComponentNameHandle); }));
	UObject* HelperActorObj = nullptr;
	AActor* HelperActor = nullptr;
	HelperActorHandle->GetValue(HelperActorObj);

	//ChildBuilder.AddProperty(TargetCompHandle.ToSharedRef());
	//ChildBuilder.AddProperty(HelperActorHandle.ToSharedRef());
	TSharedPtr<SWidget> ContentWidget;
	if (bIsInWorld)
	{
		TArray<UActorComponent*> Components;
		if (HelperActorObj)
		{
			HelperActor = Cast<AActor>(HelperActorObj);
			if (HelperActor && HelperClass)
			{
				HelperActor->GetComponents(HelperClass, Components);
			}
		}
		if (!IsValid(TargetComp) && Components.Num() == 1)//if TargetComp not valid, but this type of component exist, then show a fix button to fix the reference.
		{
			ContentWidget = 
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.VAlign(EVerticalAlignment::VAlign_Center)
					.Padding(FMargin(4, 2))
					[
						SNew(STextBlock)
						.Text(LOCTEXT("Missing component reference!", "Missing component reference!"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
						.ColorAndOpacity(FSlateColor(FLinearColor(1.0f, 0.0f, 0.0f, 1.0f)))
					]
					+SHorizontalBox::Slot()
					.AutoWidth()
					[
						SNew(SButton)
						.Text(LOCTEXT("FixMissingComponent", "Fix"))
						.OnClicked(this, &FLGUIComponentReferenceCustomization::OnClickFixComponentReference, TargetCompHandle, Components[0])
					]
				]
			;
		}
		else
		{
			if (!IsValid(HelperClass))
			{
				ContentWidget =
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
				if (!IsValid(HelperActor))
				{
					ContentWidget = HelperActorHandle->CreatePropertyValueWidget();
				}
				else
				{
				
					if (Components.Num() == 0)
					{
						ContentWidget = SNew(SVerticalBox)
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							HelperActorHandle->CreatePropertyValueWidget()
						]
						+ SVerticalBox::Slot()
						.AutoHeight()
						[
							SNew(STextBlock)
							.ColorAndOpacity(FSlateColor(FLinearColor::Red))
							.AutoWrapText(true)
							.Text(FText::Format(LOCTEXT("ComponentOfTypeNotFound", "Component of type: {0} not found on target actor!"), FText::FromName(HelperClass->GetFName())))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						];
					}
					else if (Components.Num() == 1)
					{
						ContentWidget = HelperActorHandle->CreatePropertyValueWidget();
					}
					else
					{
						ContentWidget = 
						SNew(SBox)
						.WidthOverride(500)
						[
							SNew(SHorizontalBox)
							+ SHorizontalBox::Slot()
							.FillWidth(0.65f)
							[
								HelperActorHandle->CreatePropertyValueWidget()
							]
							+ SHorizontalBox::Slot()
							.FillWidth(0.35f)
							[
								SNew(SComboButton)
								.ToolTipText(FText::Format(LOCTEXT("TargetActorHaveMultipleComponent_YouMustSelectOne", "Target actor have multiple components of type: {0}, you must select one of them"), FText::FromName(HelperClass->GetFName())))
								.OnGetMenuContent(this, &FLGUIComponentReferenceCustomization::OnGetMenu, TargetCompHandle, Components)
								.ContentPadding(FMargin(0))
								.ButtonContent()
								[
									SNew(STextBlock)
									.Text(this, &FLGUIComponentReferenceCustomization::GetButtonText, TargetCompHandle, Components)
									.Font(IDetailLayoutBuilder::GetDetailFont())
								]
							]
						]
						;
					}
				}
			}
		}
	}
	else
	{
		ContentWidget = HelperClassHandle->CreatePropertyValueWidget();
	}

	ChildBuilder.AddCustomRow(LOCTEXT("ActorComponent", "ActorComponent"))
	.NameContent()
	[
		PropertyHandle->CreatePropertyNameWidget()
	]
	.ValueContent()
	.MinDesiredWidth(500)
	[
		ContentWidget.ToSharedRef()
	]
	.CopyAction(FUIAction
	(
		FExecuteAction::CreateSP(this, &FLGUIComponentReferenceCustomization::OnCopy, HelperActor, TargetComp, HelperClass),
		FCanExecuteAction::CreateLambda([=] {return bIsInWorld; })
	))
	.PasteAction(FUIAction
	(
		FExecuteAction::CreateSP(this, &FLGUIComponentReferenceCustomization::OnPaste, HelperActorHandle, TargetCompHandle, HelperClassHandle),
		FCanExecuteAction::CreateLambda([=] {return bIsInWorld; })
	))
	;
}
void FLGUIComponentReferenceCustomization::OnCopy(AActor* HelperActor, UActorComponent* TargetComp, UClass* HelperClass)
{
	CopiedHelperActor = HelperActor;
	CopiedTargetComp = TargetComp;
	CopiedHelperClass = HelperClass;
}
void FLGUIComponentReferenceCustomization::OnPaste(TSharedPtr<IPropertyHandle> HelperActorProperty, TSharedPtr<IPropertyHandle> TargetCompProperty, TSharedPtr<IPropertyHandle> HelperClassProperty)
{
	HelperActorProperty->SetValue((UObject*)CopiedHelperActor.Get());
	TargetCompProperty->SetValue((UObject*)CopiedTargetComp.Get());
	HelperClassProperty->SetValue((UObject*)CopiedHelperClass);
}
TSharedRef<SWidget> FLGUIComponentReferenceCustomization::OnGetMenu(TSharedPtr<IPropertyHandle> CompProperty, TArray<UActorComponent*> Components)
{
	FMenuBuilder MenuBuilder(true, nullptr);
	//MenuBuilder.BeginSection(FName(), LOCTEXT("Components", "Components"));
	{
		MenuBuilder.AddMenuEntry(
			FText::FromName(FName(NAME_None)),
			FText(LOCTEXT("Tip", "Clear component selection, will use first one.")),
			FSlateIcon(),
			FUIAction(FExecuteAction::CreateRaw(this, &FLGUIComponentReferenceCustomization::OnSelectComponent, CompProperty, (UActorComponent*)nullptr))
		);
		for (auto Comp : Components)
		{
			if (Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;
			MenuBuilder.AddMenuEntry(
				FText::FromString(Comp->GetName()),
				FText(),
				FSlateIcon(),
				FUIAction(FExecuteAction::CreateRaw(this, &FLGUIComponentReferenceCustomization::OnSelectComponent, CompProperty, Comp))
			);
		}
	}
	//MenuBuilder.EndSection();
	return MenuBuilder.MakeWidget();
}
void FLGUIComponentReferenceCustomization::OnSelectComponent(TSharedPtr<IPropertyHandle> CompProperty, UActorComponent* Comp)
{
	CompProperty->SetValue(Comp);
	PropertyUtilites->ForceRefresh();
}

FReply FLGUIComponentReferenceCustomization::OnClickFixComponentReference(TSharedPtr<IPropertyHandle> HelperCompHandle, UActorComponent* Target)
{
	HelperCompHandle->SetValue((UObject*)Target);
	PropertyUtilites->ForceRefresh();
	return FReply::Handled();
}

FText FLGUIComponentReferenceCustomization::GetButtonText(TSharedPtr<IPropertyHandle> TargetCompHandle, TArray<UActorComponent*> Components)const
{
	UObject* TargetCompObj = nullptr;
	UActorComponent* TargetComp = nullptr;
	TargetCompHandle->GetValue(TargetCompObj);
	if (TargetCompObj)
	{
		TargetComp = Cast<UActorComponent>(TargetCompObj);
	}

	if (IsValid(TargetComp))
	{
		return FText::FromName(TargetComp->GetFName());
	}
	else
	{
		return LOCTEXT("ComponentButtonNone", "None");
	}
}
void FLGUIComponentReferenceCustomization::OnHelperActorValueChange(TSharedPtr<IPropertyHandle> HelperActorHandle, TSharedPtr<IPropertyHandle> HelperCompHandle, TSharedPtr<IPropertyHandle> HelperClassHandle, TSharedPtr<IPropertyHandle> HelperComponentNameHandle)
{
	UObject* HelperActorObj = nullptr;
	HelperActorHandle->GetValue(HelperActorObj);
	UObject* HelperClassObj = nullptr;
	HelperClassHandle->GetValue(HelperClassObj);
	UClass* HelperClass = Cast<UClass>(HelperClassObj);
	AActor* HelperActor = Cast<AActor>(HelperActorObj);
	if (HelperActor)
	{
		if (HelperClass)
		{
			TArray<UActorComponent*> Components;
			HelperActor->GetComponents(HelperClass, Components);
			if (Components.Num() == 1)
			{
				HelperCompHandle->SetValue((UObject*)Components[0]);
				HelperComponentNameHandle->SetValue(Components[0]->GetFName());
			}
			else if (Components.Num() == 0)
			{
				HelperCompHandle->ResetToDefault();
				HelperActorHandle->ResetToDefault();
				HelperComponentNameHandle->ResetToDefault();
			}
			else//multiple component of the class
			{
				PropertyUtilites->ForceRefresh();//refresh to show helper
			}
		}
	}
	else
	{
		HelperCompHandle->ResetToDefault();
		HelperComponentNameHandle->ResetToDefault();
	}
}
#undef LOCTEXT_NAMESPACE