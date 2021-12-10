// Copyright 2019-2021 LexLiu. All Rights Reserved.

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
UClass* FLGUIComponentReferenceCustomization::CopiedTargetClass;

TSharedRef<IPropertyTypeCustomization> FLGUIComponentReferenceCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIComponentReferenceCustomization);
}
void FLGUIComponentReferenceCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyUtilites = CustomizationUtils.GetPropertyUtilities();
	bool bIsInWorld = false;
	FCommentNodeSet nodeSet;
	PropertyHandle->GetOuterObjects(nodeSet);
	for (UObject* obj : nodeSet)
	{
		bIsInWorld = obj->GetWorld() != nullptr;
		break;
	}

	UObject* TargetClassObj = nullptr;
	UClass* TargetClass = nullptr;
	TSharedPtr<IPropertyHandle> TargetClassHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, TargetClass));
	TargetClassHandle->GetValue(TargetClassObj);
	if (TargetClassObj)
	{
		TargetClass = Cast<UClass>(TargetClassObj);
	}

	UObject* TargetCompObj = nullptr;
	UActorComponent* TargetComp = nullptr;
	auto TargetCompHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, TargetComp));
	TargetCompHandle->GetValue(TargetCompObj);
	if (TargetCompObj)
	{
		TargetComp = Cast<UActorComponent>(TargetCompObj);
	}

	TSharedPtr<IPropertyHandle> HelperActorHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIComponentReference, HelperActor));
	HelperActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=]() {OnHelperActorValueChange(HelperActorHandle, TargetCompHandle, TargetClassHandle); }));
	UObject* HelperActorObj = nullptr;
	AActor* HelperActor = nullptr;
	HelperActorHandle->GetValue(HelperActorObj);
	TArray<UActorComponent*> Components;
	if (HelperActorObj)
	{
		HelperActor = Cast<AActor>(HelperActorObj);
		if (HelperActor)
		{
			HelperActor->GetComponents(TargetClass, Components);
		}
	}
	if (!IsValid(TargetComp) && Components.Num() == 1)//if TargetComp not valid, but this type of component exist, then clear HelperActor, because we need to reassign it
	{
		HelperActorHandle->SetValue((UObject*)nullptr);
	}

	TSharedPtr<SWidget> ContentWidget;
	if (bIsInWorld)
	{
		if (!IsValid(TargetClass))
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
						.Text(FText::FromString(FString::Printf(TEXT("Component of type: %s not found on target actor!"), *TargetClass->GetName())))
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
							.ToolTipText(FText::FromString(FString::Printf(TEXT("Target actor have multiple components of type:%s, you must select one of them"), *TargetClass->GetName())))
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
	else
	{
		ContentWidget = TargetClassHandle->CreatePropertyValueWidget();
	}

	ChildBuilder.AddCustomRow(FText::FromString("ActorComponent"))
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
		FExecuteAction::CreateSP(this, &FLGUIComponentReferenceCustomization::OnCopy, HelperActor, TargetComp, TargetClass),
		FCanExecuteAction::CreateLambda([=] {return bIsInWorld; })
	))
	.PasteAction(FUIAction
	(
		FExecuteAction::CreateSP(this, &FLGUIComponentReferenceCustomization::OnPaste, HelperActorHandle, TargetCompHandle, TargetClassHandle),
		FCanExecuteAction::CreateLambda([=] {return bIsInWorld; })
	))
	;
}
void FLGUIComponentReferenceCustomization::OnCopy(AActor* HelperActor, UActorComponent* TargetComp, UClass* TargetClass)
{
	CopiedHelperActor = HelperActor;
	CopiedTargetComp = TargetComp;
	CopiedTargetClass = TargetClass;
}
void FLGUIComponentReferenceCustomization::OnPaste(TSharedPtr<IPropertyHandle> HelperActorProperty, TSharedPtr<IPropertyHandle> TargetCompProperty, TSharedPtr<IPropertyHandle> TargetClassProperty)
{
	HelperActorProperty->SetValue((UObject*)CopiedHelperActor.Get());
	TargetCompProperty->SetValue((UObject*)CopiedTargetComp.Get());
	TargetClassProperty->SetValue((UObject*)CopiedTargetClass);
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
		return FText::FromString(FString(TEXT("None")));
	}
}
void FLGUIComponentReferenceCustomization::OnHelperActorValueChange(TSharedPtr<IPropertyHandle> HelperActorHandle, TSharedPtr<IPropertyHandle> TargetCompHandle, TSharedPtr<IPropertyHandle> TargetClassHandle)
{
	UObject* HelperActorObj = nullptr;
	HelperActorHandle->GetValue(HelperActorObj);
	UObject* TargetClassObj = nullptr;
	TargetClassHandle->GetValue(TargetClassObj);
	UClass* TargetClass = Cast<UClass>(TargetClassObj);
	AActor* HelperActor = Cast<AActor>(HelperActorObj);
	if (HelperActor)
	{
		if (TargetClass)
		{
			TArray<UActorComponent*> Components;
			HelperActor->GetComponents(TargetClass, Components);
			if (Components.Num() == 1)
			{
				TargetCompHandle->SetValue(Components[0]);
			}
			else if (Components.Num() == 0)
			{
				TargetCompHandle->SetValue((UObject*)nullptr);
				HelperActorHandle->SetValue((UObject*)nullptr);
			}
			else//multiple component of the class
			{
				PropertyUtilites->ForceRefresh();//refresh to show helper
			}
		}
	}
	else
	{
		TargetCompHandle->SetValue((UObject*)nullptr);
	}
}
#undef LOCTEXT_NAMESPACE