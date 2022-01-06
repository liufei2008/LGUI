﻿// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIEventDelegateCustomization.h"
#include "LGUIEditorStyle.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "LGUIEditorUtils.h"
#include "Widget/LGUIVectorInputBox.h"
#include "Widgets/Input/SRotatorInputBox.h"
#include "Widgets/Colors/SColorBlock.h"
#include "Widgets/Colors/SColorPicker.h"
#include "Widgets/SWidget.h"
#include "Math/UnitConversion.h"
#include "STextPropertyEditableTextBox.h"
#if ENGINE_MAJOR_VERSION < 5
#include "SEnumCombobox.h"
#else
#include "SEnumCombo.h"
#endif
#include "Serialization/BufferArchive.h"
#include "LGUIEditableTextPropertyHandle.h"

#define LOCTEXT_NAMESPACE "LGUIEventDelegateCustomization"

PRAGMA_DISABLE_OPTIMIZATION

#define LGUIEventActorSelfName "(ActorSelf)"

TArray<FString> FLGUIEventDelegateCustomization::CopySourceData;

void FLGUIEventDelegateCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyUtilites = CustomizationUtils.GetPropertyUtilities();

	EventParameterType = GetNativeParameterType(PropertyHandle);
	EventListHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegate, eventList))->AsArray();

	//add parameter type property
	bool bIsInWorld = false;
	TArray<UObject*> NodeSet;
	PropertyHandle->GetOuterObjects(NodeSet);
	for (UObject* obj : NodeSet)
	{
		bIsInWorld = obj->GetWorld() != nullptr;
		break;
	}
	if (!bIsInWorld)
	{
		if (CanChangeParameterType)
		{
			AddNativeParameterTypeProperty(PropertyHandle, ChildBuilder);
		}
		return;
	}

	auto nameStr = PropertyHandle->GetPropertyDisplayName().ToString();
	FString paramTypeString = ULGUIEventDelegateParameterHelper::ParameterTypeToName(EventParameterType, nullptr);
	nameStr = nameStr + "(" + paramTypeString + ")";
	auto titleWidget = 
		SNew(SHorizontalBox)
		+SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Left)
		.VAlign(EVerticalAlignment::VAlign_Center)
		.AutoWidth()
		[
			SNew(STextBlock)
			.Text(FText::FromString(nameStr))
			.ToolTipText(PropertyHandle->GetToolTipText())
			//.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		+SHorizontalBox::Slot()
		.HAlign(EHorizontalAlignment::HAlign_Right)
		[
			IsParameterTypeValid(EventParameterType) ?
			(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(2, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::OnClickListAdd))
					]
					+ SHorizontalBox::Slot()
					[
						PropertyCustomizationHelpers::MakeEmptyButton(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::OnClickListEmpty))
					]
				]
			)
			:
			(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(2, 0)
				[
					SNew(STextBlock)
					.AutoWrapText(true)
					.ColorAndOpacity(FSlateColor(FLinearColor::Red))
					.Text(FText::FromString("Parameter type is wrong!"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			)
		];

	auto verticalLayout = 
		SNew(SVerticalBox)
		+ SVerticalBox::Slot()
		.AutoHeight()
		[
			SNew(SBox)
			.Padding(FMargin(8, 0))
			.HeightOverride(30)
			[
				titleWidget
			]
		];
	uint32 arrayCount;
	EventListHandle->GetNumElements(arrayCount);
	for (int32 EventItemIndex = 0; EventItemIndex < (int32)arrayCount; EventItemIndex++)
	{
		auto ItemHandle = EventListHandle->GetElement(EventItemIndex);
		//HelperActor
		auto HelperActorHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperActor));
		AActor* HelperActor = nullptr;
		UObject* HelperActorObject = nullptr;
		HelperActorHandle->GetValue(HelperActorObject);
		if (HelperActorObject != nullptr)
		{
			HelperActor = (AActor*)HelperActorObject;
		}

		//TargetObject
		auto TargetObjectHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, TargetObject));
		UObject* TargetObject = nullptr;
		TargetObjectHandle->GetValue(TargetObject);

		if (!IsValid(HelperActor))
		{
			TargetObjectHandle->SetValue((UObject*)nullptr);
		}

		HelperActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
			
		FString ComponentDisplayName;
		if (IsValid(TargetObject))
		{
			if (TargetObject == HelperActor)
			{
				ComponentDisplayName = LGUIEventActorSelfName;
			}
			else
			{
				if (auto Comp = Cast<UActorComponent>(TargetObject))
				{
					ComponentDisplayName = TargetObject->GetName();
				}
				else
				{
					ComponentDisplayName = "(WrongType)";
				}
			}
		}
		else
		{
			ComponentDisplayName = "None";
		}

		//function
		auto FunctionNameHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, functionName));
		FName FunctionFName;
		FunctionNameHandle->GetValue(FunctionFName);
		FString FunctionName = FunctionFName.ToString();
		//parameterType
		auto FunctionParameterType = GetEventDataParameterType(ItemHandle);

		bool ComponentValid = false;//event target component valid?
		bool EventFunctionValid = false;//event target function valid?
		UFunction* EventFunction = nullptr;

		if (TargetObject)
		{
			EventFunction = TargetObject->FindFunction(FunctionFName);
			if (EventFunction)
			{
				if (ULGUIEventDelegateParameterHelper::IsStillSupported(EventFunction, FunctionParameterType))
				{
					EventFunctionValid = true;
				}
			}
		}

		if (!EventFunctionValid)//function not valid, show tip
		{
			if (FunctionName != "None Function" && !FunctionName.IsEmpty())
			{
				FString Prefix = "(NotValid)";
				FunctionName = Prefix.Append(FunctionName);
			}
		}
		if (FunctionName.IsEmpty())FunctionName = "None Function";


		//parameter
		TSharedRef<SWidget> ParameterWidget = SNew(SBox);
		if (IsValid(TargetObject) && IsValid(EventFunction))
		{
			bool bUseNativeParameter = false;
			auto UseNativeParameterHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UseNativeParameter));
			UseNativeParameterHandle->GetValue(bUseNativeParameter);
				
			if (EventParameterType != FunctionParameterType)//check "bUseNativeParameter" parameter
			{
				if (bUseNativeParameter)
				{
					bUseNativeParameter = false;
					UseNativeParameterHandle->SetValue(bUseNativeParameter);
				}
			}
			if ((EventParameterType == FunctionParameterType) && bUseNativeParameter)//support native parameter
			{
				//clear buffer and value
				ClearValueBuffer(ItemHandle);
				ClearReferenceValue(ItemHandle);
				//native parameter AnchorData
				ParameterWidget =
					SNew(SBox)
					.VAlign(EVerticalAlignment::VAlign_Center)
					[
						SNew(STextBlock)
						.Text(FText::FromString("(NativeParameter)"))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					;
			}
			else
			{
				ParameterWidget = DrawFunctionParameter(ItemHandle, FunctionParameterType, EventFunction);
			}
		}
		else
		{
			ParameterWidget = 
				SNew(SBox)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("(NotValid)"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				;
		}
		ParameterWidget->SetToolTipText(LOCTEXT("Parameter", "Set parameter for the function of this event"));
		EventParameterWidgetArray.Add(ParameterWidget);

		//additional button
		int additionalButtonHeight = 20;
		auto additionalButtons = 
		SNew(SBox)
		[
			//copy, paste, add, delete
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(HAlign_Right)
			[
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(SBox)
						.HeightOverride(additionalButtonHeight)
						.WidthOverride(30)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Text(LOCTEXT("C", "C"))
							.OnClicked(this, &FLGUIEventDelegateCustomization::OnClickCopyPaste, true, EventItemIndex)
							.ToolTipText(LOCTEXT("Copy", "Copy this function"))
						]
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
					SNew(SBox)
					.HeightOverride(additionalButtonHeight)
					.WidthOverride(30)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Text(LOCTEXT("P", "P"))
						.OnClicked(this, &FLGUIEventDelegateCustomization::OnClickCopyPaste, false, EventItemIndex)
						.ToolTipText(LOCTEXT("Paste", "Paste copied function to this function"))
					]
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
					SNew(SBox)
					.HeightOverride(additionalButtonHeight)
					.WidthOverride(30)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Text(LOCTEXT("+", "+"))
						.OnClicked(this, &FLGUIEventDelegateCustomization::OnClickAddRemove, true, EventItemIndex, (int32)arrayCount)
						.ToolTipText(LOCTEXT("Add", "Add new one"))
					]
					]
				]
				+ SHorizontalBox::Slot()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
					SNew(SBox)
					.HeightOverride(additionalButtonHeight)
					.WidthOverride(30)
					[
						SNew(SButton)
						.HAlign(HAlign_Center)
						.VAlign(VAlign_Center)
						.Text(LOCTEXT("-", "-"))
						.OnClicked(this, &FLGUIEventDelegateCustomization::OnClickAddRemove, false, EventItemIndex, (int32)arrayCount)
						.ToolTipText(LOCTEXT("Delete", "Delete this one"))
					]
					]
				]
			]
		];


		verticalLayout->AddSlot()
			.AutoHeight()
			[
				SNew(SBox)
				.Padding(FMargin(2, 0))
				[
					SNew(SOverlay)
					+ SOverlay::Slot()
					[
						SNew(SVerticalBox)
						+SVerticalBox::Slot()
						.AutoHeight()
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SBox)
							.WidthOverride(1000)
							.HeightOverride(this, &FLGUIEventDelegateCustomization::GetEventItemHeight, EventItemIndex)
							[
								SNew(SImage)
								.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.EventItem"))
								.ColorAndOpacity(FLinearColor(FColor(255, 255, 255, 255)))
							]
						]
					]
					+ SOverlay::Slot()
					[
						SNew(SBox)
						.Padding(FMargin(4, 4))
						[
							SNew(SVerticalBox)
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SBox)
								.Padding(FMargin(0, 0, 0, 2))
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot()
									[
										//HelperActor
										SNew(SBox)
										.Padding(FMargin(0, 0, 6, 0))
										[
											HelperActorHandle->CreatePropertyValueWidget()
										]
									]
									+SHorizontalBox::Slot()
									[
										SNew(SBox)
										.HeightOverride(26)
										[
											//Component
											SNew(SComboButton)
											.HasDownArrow(true)
											.IsEnabled(this, &FLGUIEventDelegateCustomization::IsComponentSelectorMenuEnabled, EventItemIndex)
											.ToolTipText(LOCTEXT("Component", "Pick component for this event"))
											.ButtonContent()
											[
												SNew(STextBlock)
												.Text(FText::FromString(ComponentDisplayName))
												.Font(IDetailLayoutBuilder::GetDetailFont())
											]
											.MenuContent()
											[
												MakeComponentSelectorMenu(EventItemIndex)
											]
										]
									]
								]
							]
							+SVerticalBox::Slot()
							.AutoHeight()
							[
								SNew(SBox)
								.Padding(FMargin(0, 0, 0, 2))
								[
									SNew(SHorizontalBox)
									+SHorizontalBox::Slot()
									[
										SNew(SBox)
										.Padding(FMargin(0, 0, 6, 0))
										[
											SNew(SBox)
											.HeightOverride(26)
											[
												//function
												SNew(SComboButton)
												.HasDownArrow(true)
												.IsEnabled(this, &FLGUIEventDelegateCustomization::IsFunctionSelectorMenuEnabled, EventItemIndex)
												.ToolTipText(LOCTEXT("Function", "Pick a function to execute of this event"))
												.ButtonContent()
												[
													SNew(STextBlock)
													.Text(FText::FromString(FunctionName))
													.Font(IDetailLayoutBuilder::GetDetailFont())
												]
												.MenuContent()
												[
													MakeFunctionSelectorMenu(EventItemIndex)
												]
											]
										]
									]
									+SHorizontalBox::Slot()
									[
										//parameter
										ParameterWidget
									]
								]
							]
							+SVerticalBox::Slot()
							[
								additionalButtons
							]
						]
					]
				]
			]
		;
	}
	ChildBuilder.AddCustomRow(LOCTEXT("EventDelegate", "EventDelegate"))
		.WholeRowContent()
		[
			SNew(SBox)
			.Padding(FMargin(-10, 0, -2, 0))
			[
				SNew(SOverlay)
				+ SOverlay::Slot()
				[
					SNew(SBox)
					.HAlign(EHorizontalAlignment::HAlign_Center)
					.VAlign(EVerticalAlignment::VAlign_Center)
					[
						SNew(SBox)
						.WidthOverride(1000)
						.HeightOverride(this, &FLGUIEventDelegateCustomization::GetEventTotalHeight)
						[
							SNew(SImage)
							.Image(FLGUIEditorStyle::Get().GetBrush("LGUIEditor.EventGroup"))
							.ColorAndOpacity(FLinearColor(FColor(255, 255, 255, 255)))
						]
					]
				]
				+ SOverlay::Slot()
				[
					verticalLayout
				]
			]
		]
	;

}

void FLGUIEventDelegateCustomization::OnSelectComponent(UActorComponent* Comp, int32 itemIndex)
{
	auto ItemHandle = EventListHandle->GetElement(itemIndex);
	auto TargetObjectHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, TargetObject));
	TargetObjectHandle->SetValue(Comp);

	auto HelperClassHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperClass));
	HelperClassHandle->SetValue(Comp->StaticClass());

	auto HelperComponentNameHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperComponentName));
	HelperComponentNameHandle->SetValue(Comp->GetFName());

	PropertyUtilites->ForceRefresh();
}
void FLGUIEventDelegateCustomization::OnSelectActorSelf(int32 itemIndex)
{
	auto ItemHandle = EventListHandle->GetElement(itemIndex);

	auto HelperActorHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperActor));
	UObject* HelperActorObject = nullptr;
	HelperActorHandle->GetValue(HelperActorObject);

	auto TargetObjectHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, TargetObject));
	TargetObjectHandle->SetValue(HelperActorObject);

	auto HelperClassHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperClass));
	HelperClassHandle->SetValue(AActor::StaticClass());

	PropertyUtilites->ForceRefresh();
}
void FLGUIEventDelegateCustomization::OnSelectFunction(FName FuncName, int32 itemIndex, LGUIEventDelegateParameterType ParamType, bool UseNativeParameter)
{
	auto ItemHandle = EventListHandle->GetElement(itemIndex);
	auto nameHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, functionName));
	nameHandle->SetValue(FuncName);
	SetEventDataParameterType(ItemHandle, ParamType);
	auto UseNativeParameterHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UseNativeParameter));
	UseNativeParameterHandle->SetValue(UseNativeParameter);
	PropertyUtilites->ForceRefresh();
}

void FLGUIEventDelegateCustomization::SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, LGUIEventDelegateParameterType ParameterType)
{
	auto ParamTypeHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamType));
	ParamTypeHandle->SetValue((uint8)ParameterType);
}
LGUIEventDelegateParameterType FLGUIEventDelegateCustomization::GetNativeParameterType(TSharedRef<IPropertyHandle> PropertyHandle)
{
	auto NativeParameterTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegate, supportParameterType));
	NativeParameterTypeHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	uint8 supportParameterTypeUint8;
	NativeParameterTypeHandle->GetValue(supportParameterTypeUint8);
	LGUIEventDelegateParameterType eventParameterType = (LGUIEventDelegateParameterType)supportParameterTypeUint8;
	return eventParameterType;
}
void FLGUIEventDelegateCustomization::AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder)
{
	auto NativeParameterTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegate, supportParameterType));
	ChildBuilder.AddProperty(NativeParameterTypeHandle.ToSharedRef());
}
LGUIEventDelegateParameterType FLGUIEventDelegateCustomization::GetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle)
{
	auto paramTypeHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamType));
	paramTypeHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::OnParameterTypeChange, EventDataItemHandle));
	uint8 functionParameterTypeUint8;
	paramTypeHandle->GetValue(functionParameterTypeUint8);
	LGUIEventDelegateParameterType functionParameterType = (LGUIEventDelegateParameterType)functionParameterTypeUint8;
	return functionParameterType;
}

TSharedRef<SWidget> FLGUIEventDelegateCustomization::MakeComponentSelectorMenu(int32 itemIndex)
{
	auto ItemHandle = EventListHandle->GetElement(itemIndex);
	auto HelperActorHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperActor));
	UObject* HelperActorObject = nullptr;
	HelperActorHandle->GetValue(HelperActorObject);
	if (HelperActorObject == nullptr)
	{
		return SNew(SBox);
	}

	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

	auto HelperActor = (AActor*)HelperActorObject;
	MenuBuilder.AddMenuEntry(
		FUIAction(FExecuteAction::CreateRaw(this, &FLGUIEventDelegateCustomization::OnSelectActorSelf, itemIndex)),
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(LGUIEventActorSelfName))
			.Font(IDetailLayoutBuilder::GetDetailFont())
		]
		//+SHorizontalBox::Slot()
		//.HAlign(EHorizontalAlignment::HAlign_Right)
		//[
		//	SNew(STextBlock)
		//	.Text(FText::FromString(HelperActor->GetClass()->GetName()))
		//	.Font(IDetailLayoutBuilder::GetDetailFont())
		//]
	);
	auto Components = HelperActor->GetComponents();
	for (auto Comp : Components)
	{
		if(Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;
		auto CompName = Comp->GetFName();
		auto CompTypeName = Comp->GetClass()->GetName();
		MenuBuilder.AddMenuEntry(
			FUIAction(FExecuteAction::CreateRaw(this, &FLGUIEventDelegateCustomization::OnSelectComponent, Comp, itemIndex)),
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.HAlign(EHorizontalAlignment::HAlign_Left)
			[
				SNew(STextBlock)
				.Text(FText::FromString(CompName.ToString()))
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			//+ SHorizontalBox::Slot()
			//.HAlign(EHorizontalAlignment::HAlign_Right)
			//[
			//	SNew(STextBlock)
			//	.Text(FText::FromString(CompTypeName))
			//	.Font(IDetailLayoutBuilder::GetDetailFont())
			//]
		);
	}
	return MenuBuilder.MakeWidget();
}
TSharedRef<SWidget> FLGUIEventDelegateCustomization::MakeFunctionSelectorMenu(int32 itemIndex)
{
	auto ItemHandle = EventListHandle->GetElement(itemIndex);
	auto TargetObjectHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, TargetObject));
	UObject* TargetObject = nullptr;
	TargetObjectHandle->GetValue(TargetObject);
	if (TargetObject == nullptr)
	{
		return SNew(SBox);
	}

	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

	auto FunctionField = TFieldRange<UFunction>(TargetObject->GetClass());
	for (auto Func : FunctionField)
	{
		LGUIEventDelegateParameterType ParamType;
		if (ULGUIEventDelegateParameterHelper::IsSupportedFunction(Func, ParamType))//show only supported type
		{
			FString ParamTypeString = ULGUIEventDelegateParameterHelper::ParameterTypeToName(ParamType, Func);
			auto FunctionSelectorName = FString::Printf(TEXT("%s(%s)"), *Func->GetName(), *ParamTypeString);
			MenuBuilder.AddMenuEntry(
				FUIAction(FExecuteAction::CreateRaw(this, &FLGUIEventDelegateCustomization::OnSelectFunction, Func->GetFName(), itemIndex, ParamType, false)),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromString(FunctionSelectorName))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			);
			if (ParamType == EventParameterType)//if function support native parameter, then draw another button, and show as native parameter
			{
				FunctionSelectorName = FString::Printf(TEXT("%s(NativeParameter)"), *Func->GetName());
				MenuBuilder.AddMenuEntry(
					FUIAction(FExecuteAction::CreateRaw(this, &FLGUIEventDelegateCustomization::OnSelectFunction, Func->GetFName(), itemIndex, ParamType, true)),
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.HAlign(EHorizontalAlignment::HAlign_Left)
					[
						SNew(STextBlock)
						.Text(FText::FromString(FunctionSelectorName))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
				);
			}					
		}
	}
	return MenuBuilder.MakeWidget();
}


#define SET_VALUE_ON_BUFFER(type)\
auto ParamBuffer = GetBuffer(ParamBufferHandle);\
FMemoryReader Reader(ParamBuffer);\
type Value;\
Reader << Value;\
ValueHandle->SetValue(Value);

TSharedRef<SWidget> FLGUIEventDelegateCustomization::DrawFunctionParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, LGUIEventDelegateParameterType InFunctionParameterType, UFunction* InFunction)
{
	auto ParamBufferHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamBuffer));
	if (InFunctionParameterType != LGUIEventDelegateParameterType::None)//None means not select function yet
	{
		switch (InFunctionParameterType)
		{
		default:
		case LGUIEventDelegateParameterType::Empty:
		{
			ClearValueBuffer(InDataContainerHandle);
			ClearReferenceValue(InDataContainerHandle);
			return
				SNew(SBox)
				.MinDesiredWidth(500)
				.VAlign(EVerticalAlignment::VAlign_Center)
				[
					SNew(STextBlock)
					.Text(FText::FromString("(No parameter)"))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			;
		}
		break;
		case LGUIEventDelegateParameterType::Bool:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 1);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, BoolValue));
			auto ParamBuffer = GetBuffer(ParamBufferHandle);
			bool Value = ParamBuffer[0] == 1;
			ValueHandle->SetValue(Value);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::BoolValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Float:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 4);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, FloatValue));
			SET_VALUE_ON_BUFFER(float);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::FloatValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Double:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 8);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, DoubleValue));
			SET_VALUE_ON_BUFFER(double);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::DoubleValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Int8:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 1);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int8Value));
			SET_VALUE_ON_BUFFER(int8);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::Int8ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::UInt8:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 1);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt8Value));
			SET_VALUE_ON_BUFFER(uint8);
			if (auto enumValue = ULGUIEventDelegateParameterHelper::GetEnumParameter(InFunction))
			{
				return
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					.VAlign(VAlign_Center)
					.FillWidth(1.0f)
					.Padding(0.0f, 2.0f)
					[
						SNew(SBox)
						.MinDesiredWidth(500)
						[
							SNew(SEnumComboBox, enumValue)
							.CurrentValue(this, &FLGUIEventDelegateCustomization::GetEnumValue, ValueHandle)
							.OnEnumSelectionChanged(this, &FLGUIEventDelegateCustomization::EnumValueChange, ValueHandle, ParamBufferHandle)
						]
					]
				;
			}
			else
			{
				ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::UInt8ValueChange, ValueHandle, ParamBufferHandle));
				return ValueHandle->CreatePropertyValueWidget();
			}
		}
		break;
		case LGUIEventDelegateParameterType::Int16:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 2);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int16Value));
			SET_VALUE_ON_BUFFER(int16);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::Int16ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::UInt16:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 2);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt16Value));
			SET_VALUE_ON_BUFFER(uint16);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::UInt16ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Int32:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 4);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int32Value));
			SET_VALUE_ON_BUFFER(int32);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::Int32ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::UInt32:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 4);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt32Value));
			SET_VALUE_ON_BUFFER(uint32);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::UInt32ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Int64:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 8);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int64Value));
			SET_VALUE_ON_BUFFER(int64);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::Int64ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::UInt64:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 8);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt64Value));
			SET_VALUE_ON_BUFFER(uint64);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::UInt64ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Vector2:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 8);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Vector2Value));
			SET_VALUE_ON_BUFFER(FVector2D);
			return SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				.Padding(0.0f, 2.0f)
				[
					SNew(SLGUIVectorInputBox)
					.AllowSpin(false)
					.bColorAxisLabels(true)
#if ENGINE_MAJOR_VERSION < 5
					.AllowResponsiveLayout(true)
#endif
					.EnableX(true)
					.EnableY(true)
					.ShowX(true)
					.ShowY(true)
					.X(this, &FLGUIEventDelegateCustomization::Vector2GetItemValue, 0, ValueHandle, ParamBufferHandle)
					.Y(this, &FLGUIEventDelegateCustomization::Vector2GetItemValue, 1, ValueHandle, ParamBufferHandle)
					.OnXCommitted(this, &FLGUIEventDelegateCustomization::Vector2ItemValueChange, 0, ValueHandle, ParamBufferHandle)
					.OnYCommitted(this, &FLGUIEventDelegateCustomization::Vector2ItemValueChange, 1, ValueHandle, ParamBufferHandle)
				]
			;
		}
		break;
		case LGUIEventDelegateParameterType::Vector3:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 12);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Vector3Value));
			SET_VALUE_ON_BUFFER(FVector);
			return SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				.Padding(0.0f, 2.0f)
				[
					SNew(SLGUIVectorInputBox)
					.AllowSpin(false)
					.bColorAxisLabels(true)
#if ENGINE_MAJOR_VERSION < 5
					.AllowResponsiveLayout(true)
#endif
					.EnableX(true)
					.EnableY(true)
					.EnableZ(true)
					.ShowX(true)
					.ShowY(true)
					.ShowZ(true)
					.X(this, &FLGUIEventDelegateCustomization::Vector3GetItemValue, 0, ValueHandle, ParamBufferHandle)
					.Y(this, &FLGUIEventDelegateCustomization::Vector3GetItemValue, 1, ValueHandle, ParamBufferHandle)
					.Z(this, &FLGUIEventDelegateCustomization::Vector3GetItemValue, 2, ValueHandle, ParamBufferHandle)
					.OnXCommitted(this, &FLGUIEventDelegateCustomization::Vector3ItemValueChange, 0, ValueHandle, ParamBufferHandle)
					.OnYCommitted(this, &FLGUIEventDelegateCustomization::Vector3ItemValueChange, 1, ValueHandle, ParamBufferHandle)
					.OnZCommitted(this, &FLGUIEventDelegateCustomization::Vector3ItemValueChange, 2, ValueHandle, ParamBufferHandle)
				]
			;
		}
		break;
		case LGUIEventDelegateParameterType::Vector4:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 16);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Vector4Value));
			SET_VALUE_ON_BUFFER(FVector4);
			return SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				.Padding(0.0f, 2.0f)
				[
					SNew(SLGUIVectorInputBox)
					.AllowSpin(false)
					.bColorAxisLabels(true)
#if ENGINE_MAJOR_VERSION < 5
					.AllowResponsiveLayout(true)
#endif
					.EnableX(true)
					.EnableY(true)
					.EnableZ(true)
					.EnableW(true)
					.ShowX(true)
					.ShowY(true)
					.ShowZ(true)
					.ShowW(true)
					.X(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 0, ValueHandle, ParamBufferHandle)
					.Y(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 1, ValueHandle, ParamBufferHandle)
					.Z(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 2, ValueHandle, ParamBufferHandle)
					.W(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 3, ValueHandle, ParamBufferHandle)
					.OnXCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 0, ValueHandle, ParamBufferHandle)
					.OnYCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 1, ValueHandle, ParamBufferHandle)
					.OnZCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 2, ValueHandle, ParamBufferHandle)
					.OnWCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 3, ValueHandle, ParamBufferHandle)
				]
			;
		}
		break;
		case LGUIEventDelegateParameterType::Color:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 4);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ColorValue));
			auto ParamBuffer = GetBuffer(ParamBufferHandle);
			FMemoryReader Reader(ParamBuffer);
			FColor Value;
			Reader << Value;
			ValueHandle->SetValueFromFormattedString(Value.ToString());
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 2.0f)
				[
					// Displays the color with alpha unless it is ignored
					SAssignNew(ColorPickerParentWidget, SColorBlock)
					.Color(this, &FLGUIEventDelegateCustomization::LinearColorGetValue, false, ValueHandle, ParamBufferHandle)
					.ShowBackgroundForAlpha(true)
#if ENGINE_MAJOR_VERSION < 5
					.IgnoreAlpha(false)
#else
					.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
#endif
					.OnMouseButtonDown(this, &FLGUIEventDelegateCustomization::OnMouseButtonDownColorBlock, false, ValueHandle, ParamBufferHandle)
					.Size(FVector2D(35.0f, 12.0f))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 2.0f)
				[
					// Displays the color without alpha
					SNew(SColorBlock)
					.Color(this, &FLGUIEventDelegateCustomization::LinearColorGetValue, false, ValueHandle, ParamBufferHandle)
					.ShowBackgroundForAlpha(false)
#if ENGINE_MAJOR_VERSION < 5
					.IgnoreAlpha(true)
#else
					.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Ignore)
#endif
					.OnMouseButtonDown(this, &FLGUIEventDelegateCustomization::OnMouseButtonDownColorBlock, false, ValueHandle, ParamBufferHandle)
					.Size(FVector2D(35.0f, 12.0f))
				];
			;
		}
		break;
		case LGUIEventDelegateParameterType::LinearColor:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 16);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, LinearColorValue));
			SET_VALUE_ON_BUFFER(FLinearColor);
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 2.0f)
				[
					// Displays the color with alpha unless it is ignored
					SAssignNew(ColorPickerParentWidget, SColorBlock)
					.Color(this, &FLGUIEventDelegateCustomization::LinearColorGetValue, true, ValueHandle, ParamBufferHandle)
					.ShowBackgroundForAlpha(true)
#if ENGINE_MAJOR_VERSION < 5
					.IgnoreAlpha(false)
#else
					.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Separate)
#endif
					.OnMouseButtonDown(this, &FLGUIEventDelegateCustomization::OnMouseButtonDownColorBlock, true, ValueHandle, ParamBufferHandle)
					.Size(FVector2D(35.0f, 12.0f))
				]
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.Padding(0.0f, 2.0f)
				[
					// Displays the color without alpha
					SNew(SColorBlock)
					.Color(this, &FLGUIEventDelegateCustomization::LinearColorGetValue, true, ValueHandle, ParamBufferHandle)
					.ShowBackgroundForAlpha(false)
#if ENGINE_MAJOR_VERSION < 5
					.IgnoreAlpha(true)
#else
					.AlphaDisplayMode(EColorBlockAlphaDisplayMode::Ignore)
#endif
					.OnMouseButtonDown(this, &FLGUIEventDelegateCustomization::OnMouseButtonDownColorBlock, true, ValueHandle, ParamBufferHandle)
					.Size(FVector2D(35.0f, 12.0f))
				];
			;
		}
		break;
		case LGUIEventDelegateParameterType::Quaternion:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 16);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, QuatValue));
			SET_VALUE_ON_BUFFER(FQuat);
			return SNew(SHorizontalBox)
				+SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				.Padding(0.0f, 2.0f)
				[
					SNew(SLGUIVectorInputBox)
					.AllowSpin(false)
					.bColorAxisLabels(true)
#if ENGINE_MAJOR_VERSION < 5
					.AllowResponsiveLayout(true)
#endif
					.EnableX(true)
					.EnableY(true)
					.EnableZ(true)
					.EnableW(true)
					.ShowX(true)
					.ShowY(true)
					.ShowZ(true)
					.ShowW(true)
					.X(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 0, ValueHandle, ParamBufferHandle)
					.Y(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 1, ValueHandle, ParamBufferHandle)
					.Z(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 2, ValueHandle, ParamBufferHandle)
					.W(this, &FLGUIEventDelegateCustomization::Vector4GetItemValue, 3, ValueHandle, ParamBufferHandle)
					.OnXCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 0, ValueHandle, ParamBufferHandle)
					.OnYCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 1, ValueHandle, ParamBufferHandle)
					.OnZCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 2, ValueHandle, ParamBufferHandle)
					.OnWCommitted(this, &FLGUIEventDelegateCustomization::Vector4ItemValueChange, 3, ValueHandle, ParamBufferHandle)
				]
			;
		}
		break;
		case LGUIEventDelegateParameterType::String:
		{
			ClearReferenceValue(InDataContainerHandle);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, StringValue));
			SET_VALUE_ON_BUFFER(FString);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::StringValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		break;
		case LGUIEventDelegateParameterType::Name:
		{
			ClearReferenceValue(InDataContainerHandle);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, NameValue));
			SET_VALUE_ON_BUFFER(FName);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::NameValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
		case LGUIEventDelegateParameterType::Text:
		{
			ClearReferenceValue(InDataContainerHandle);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, TextValue));
			SET_VALUE_ON_BUFFER(FText);
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateCustomization::TextValueChange, ValueHandle, ParamBufferHandle));
			TSharedRef<IEditableTextProperty> EditableTextProperty = MakeShareable(new FLGUIEditableTextPropertyHandle(ValueHandle.ToSharedRef(), PropertyUtilites));
			const bool bIsMultiLine = EditableTextProperty->IsMultiLineText();
			return 
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				.Padding(0.0f, 2.0f)
				[
					SNew(SBox)
					.MinDesiredWidth(bIsMultiLine ? 250.f : 125.f)
					.MaxDesiredWidth(600)
					[
						SNew(STextPropertyEditableTextBox, EditableTextProperty)
						.Font(FEditorStyle::GetFontStyle("PropertyWindow.NormalFont"))
						.AutoWrapText(true)
					]
				]
				;
		}
		case LGUIEventDelegateParameterType::PointerEvent:
		{
			ClearValueBuffer(InDataContainerHandle);
			ClearReferenceValue(InDataContainerHandle);
			return
				SNew(SBox)
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("(PointerEventData not editable! You can only pass native parameter!)")))
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(FColor(255, 0, 0, 255)))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				];
		}
		break;
		case LGUIEventDelegateParameterType::Object:
		case LGUIEventDelegateParameterType::Actor:
		case LGUIEventDelegateParameterType::Class:
		{
			return
				SNew(SBox)
				.MinDesiredWidth(500)
				[
					DrawFunctionReferenceParameter(InDataContainerHandle, InFunctionParameterType, InFunction)
				];
		}
		break;
		case LGUIEventDelegateParameterType::Rotator:
		{
			ClearReferenceValue(InDataContainerHandle);
			SetBufferLength(ParamBufferHandle, 12);
			auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, RotatorValue));
			SET_VALUE_ON_BUFFER(FRotator);
			TSharedPtr<INumericTypeInterface<float>> TypeInterface;
			if (FUnitConversion::Settings().ShouldDisplayUnits())
			{
				TypeInterface = MakeShareable(new TNumericUnitTypeInterface<float>(EUnit::Degrees));
			}
			return SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.VAlign(VAlign_Center)
				.FillWidth(1.0f)
				.Padding(0.0f, 2.0f)
				[
					SNew(SRotatorInputBox)
					.AllowSpin(false)
					.bColorAxisLabels(true)
#if ENGINE_MAJOR_VERSION < 5
					.AllowResponsiveLayout(true)
#endif
					.Roll(this, &FLGUIEventDelegateCustomization::RotatorGetItemValue, 0, ValueHandle, ParamBufferHandle)
					.Pitch(this, &FLGUIEventDelegateCustomization::RotatorGetItemValue, 1, ValueHandle, ParamBufferHandle)
					.Yaw(this, &FLGUIEventDelegateCustomization::RotatorGetItemValue, 2, ValueHandle, ParamBufferHandle)
					.OnRollCommitted(this, &FLGUIEventDelegateCustomization::RotatorValueChange, 0, ValueHandle, ParamBufferHandle)
					.OnPitchCommitted(this, &FLGUIEventDelegateCustomization::RotatorValueChange, 1, ValueHandle, ParamBufferHandle)
					.OnYawCommitted(this, &FLGUIEventDelegateCustomization::RotatorValueChange, 2, ValueHandle, ParamBufferHandle)
					.TypeInterface(TypeInterface)
				]
			;
		}
		break;
		}
	}
	else
	{
		ClearValueBuffer(InDataContainerHandle);
		ClearReferenceValue(InDataContainerHandle);
		return
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(FText::FromString("(Not handled)"));
	}
}
//function's parameter editor
TSharedRef<SWidget> FLGUIEventDelegateCustomization::DrawFunctionReferenceParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, LGUIEventDelegateParameterType FunctionParameterType, UFunction* InFunction)
{
	auto ParamBufferHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamBuffer));

	TSharedPtr<SWidget> ParameterContent;
	switch (FunctionParameterType)
	{
	case LGUIEventDelegateParameterType::Object:
	{
		ClearValueBuffer(InDataContainerHandle);
		return SNew(SObjectPropertyEntryBox)
			.IsEnabled(true)
			.AllowedClass(ULGUIEventDelegateParameterHelper::GetObjectParameterClass(InFunction))
			.PropertyHandle(InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ReferenceObject)))
			.AllowClear(true)
			.ToolTipText(LOCTEXT("UObjectTips", "UObject only referece asset, dont use for HelperActor"))
			.OnObjectChanged(this, &FLGUIEventDelegateCustomization::ObjectValueChange, ParamBufferHandle, InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ReferenceObject)), true);
	}
	break;
	case LGUIEventDelegateParameterType::Actor:
	{
		ClearValueBuffer(InDataContainerHandle);
		return SNew(SObjectPropertyEntryBox)
			.IsEnabled(true)
			.AllowedClass(ULGUIEventDelegateParameterHelper::GetObjectParameterClass(InFunction))
			.PropertyHandle(InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ReferenceObject)))
			.AllowClear(true)
			.OnObjectChanged(this, &FLGUIEventDelegateCustomization::ObjectValueChange, ParamBufferHandle, InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ReferenceObject)), false);
	}
	break;
	case LGUIEventDelegateParameterType::Class:
	{
		auto MetaClass = ULGUIEventDelegateParameterHelper::GetClassParameterClass(InFunction);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ReferenceObject));
		ClearValueBuffer(InDataContainerHandle);
		return SNew(SClassPropertyEntryBox)
			.IsEnabled(true)
			.AllowAbstract(true)
			.AllowNone(true)
			.MetaClass(MetaClass)
			.SelectedClass(this, &FLGUIEventDelegateCustomization::GetClassValue, ValueHandle)
			.OnSetClass(this, &FLGUIEventDelegateCustomization::ClassValueChange, ValueHandle);
	}
	break;
	default:
		break;
	}
	return 
		SNew(STextBlock)
		.Font(IDetailLayoutBuilder::GetDetailFont())
		.Text(FText::FromString("(Not handled)"));
}

void FLGUIEventDelegateCustomization::ObjectValueChange(const FAssetData& InObj, TSharedPtr<IPropertyHandle> BufferHandle, TSharedPtr<IPropertyHandle> ObjectReferenceHandle, bool ObjectOrActor)
{
	if (ObjectOrActor)
	{
		//ObjectReferenct is not for HelperActor reference
		if (InObj.GetClass()->IsChildOf(AActor::StaticClass()))
		{
			UE_LOG(LGUIEditor, Error, TEXT("Please use Actor type for referece Actor, UObject is for asset object referece"));
			AActor* NullActor = nullptr;
			ObjectReferenceHandle->SetValue(NullActor);
		}
		else
		{
			ObjectReferenceHandle->SetValue(InObj);
		}
	}
	else
	{
		ObjectReferenceHandle->SetValue(InObj);
	}
}
const UClass* FLGUIEventDelegateCustomization::GetClassValue(TSharedPtr<IPropertyHandle> ClassReferenceHandle)const
{
	UObject* referenceClassObject = nullptr;
	ClassReferenceHandle->GetValue(referenceClassObject);
	return (UClass*)referenceClassObject;
}
void FLGUIEventDelegateCustomization::ClassValueChange(const UClass* InClass, TSharedPtr<IPropertyHandle> ClassReferenceHandle)
{
	ClassReferenceHandle->SetValue(InClass);
}
void FLGUIEventDelegateCustomization::EnumValueChange(int32 InValue, ESelectInfo::Type SelectionType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	uint8 Value = (uint8)InValue;
	ValueHandle->SetValue(Value);
	UInt8ValueChange(ValueHandle, BufferHandle);
}

#define SET_BUFFER_ON_VALUE(type)\
type Value;\
ValueHandle->GetValue(Value);\
FBufferArchive ToBinary;\
ToBinary << Value;\
SetBufferValue(BufferHandle, ToBinary);

void FLGUIEventDelegateCustomization::BoolValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	bool Value; 
	ValueHandle->GetValue(Value); 
	TArray<uint8> Buffer;
	Buffer.Add(Value ? 1 : 0);
	SetBufferValue(BufferHandle, Buffer);
}
void FLGUIEventDelegateCustomization::FloatValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(float);
}
void FLGUIEventDelegateCustomization::DoubleValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(double);
}
void FLGUIEventDelegateCustomization::Int8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int8);
}
void FLGUIEventDelegateCustomization::UInt8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint8);
}
void FLGUIEventDelegateCustomization::Int16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int16);
}
void FLGUIEventDelegateCustomization::UInt16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint16);
}
void FLGUIEventDelegateCustomization::Int32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int32);
}
void FLGUIEventDelegateCustomization::UInt32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint32);
}
void FLGUIEventDelegateCustomization::Int64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int64);
}
void FLGUIEventDelegateCustomization::UInt64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint64);
}
void FLGUIEventDelegateCustomization::StringValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(FString);
}
void FLGUIEventDelegateCustomization::NameValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(FName);
}
void FLGUIEventDelegateCustomization::TextValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(FText);
}
void FLGUIEventDelegateCustomization::Vector2ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	FVector2D Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	case 0:	Value.X = NewValue; break;
	case 1:	Value.Y = NewValue; break;
	}
	ValueHandle->SetValue(Value);
	FBufferArchive ToBinary;
	ToBinary << Value;
	SetBufferValue(BufferHandle, ToBinary);
}
TOptional<float> FLGUIEventDelegateCustomization::Vector2GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
{
	FVector2D Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	default:
	case 0: return	Value.X;
	case 1: return	Value.Y;
	}
}
void FLGUIEventDelegateCustomization::Vector3ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	FVector Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	case 0:	Value.X = NewValue; break;
	case 1:	Value.Y = NewValue; break;
	case 2:	Value.Z = NewValue; break;
	}
	ValueHandle->SetValue(Value);
	FBufferArchive ToBinary;
	ToBinary << Value;
	SetBufferValue(BufferHandle, ToBinary);
}
TOptional<float> FLGUIEventDelegateCustomization::Vector3GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
{
	FVector Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	default:
	case 0: return	Value.X;
	case 1: return	Value.Y;
	case 2: return	Value.Z;
	}
}
void FLGUIEventDelegateCustomization::Vector4ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	FVector4 Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	case 0:	Value.X = NewValue; break;
	case 1:	Value.Y = NewValue; break;
	case 2:	Value.Z = NewValue; break;
	case 3:	Value.W = NewValue; break;
	}
	ValueHandle->SetValue(Value);
	FBufferArchive ToBinary;
	ToBinary << Value;
	SetBufferValue(BufferHandle, ToBinary);
}
TOptional<float> FLGUIEventDelegateCustomization::Vector4GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
{
	FVector4 Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	default:
	case 0: return	Value.X;
	case 1: return	Value.Y;
	case 2: return	Value.Z;
	case 3: return	Value.W;
	}
}
FLinearColor FLGUIEventDelegateCustomization::LinearColorGetValue(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
{
	if (bIsLinearColor)
	{
		FLinearColor Value;
		FString FormatedString;
		ValueHandle->GetValueAsFormattedString(FormatedString);
		Value.InitFromString(FormatedString);
		return Value;
	}
	else
	{
		FColor Value;
		FString FormatedString;
		ValueHandle->GetValueAsFormattedString(FormatedString);
		Value.InitFromString(FormatedString);
		return FLinearColor(Value.R / 255.0f, Value.G / 255.0f, Value.B / 255.0f, Value.A / 255.0f);
	}
}
void FLGUIEventDelegateCustomization::LinearColorValueChange(FLinearColor NewValue, bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	if (bIsLinearColor)
	{
		FString FormatedString = NewValue.ToString();
		ValueHandle->SetValueFromFormattedString(FormatedString);
		FBufferArchive ToBinary;
		ToBinary << NewValue;
		SetBufferValue(BufferHandle, ToBinary);
	}
	else
	{
		FColor ColorValue = NewValue.ToFColor(false);
		FString FormatedString = ColorValue.ToString();
		ValueHandle->SetValueFromFormattedString(FormatedString);
		FBufferArchive ToBinary;
		ToBinary << ColorValue;
		SetBufferValue(BufferHandle, ToBinary);
	}
}
FReply FLGUIEventDelegateCustomization::OnMouseButtonDownColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	CreateColorPicker(bIsLinearColor, ValueHandle, BufferHandle);

	return FReply::Handled();
}
TOptional<float> FLGUIEventDelegateCustomization::RotatorGetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
{
	FRotator Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	default:
	case 0: return	Value.Roll;
	case 1: return	Value.Pitch;
	case 2: return	Value.Yaw;
	}
}
void FLGUIEventDelegateCustomization::RotatorValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	FRotator Value;
	ValueHandle->GetValue(Value);
	switch (AxisType)
	{
	case 0:	Value.Roll = NewValue; break;
	case 1:	Value.Pitch = NewValue; break;
	case 2:	Value.Yaw = NewValue; break;
	}
	ValueHandle->SetValue(Value);
	FBufferArchive ToBinary;
	ToBinary << Value;
	SetBufferValue(BufferHandle, ToBinary);
}
void FLGUIEventDelegateCustomization::SetBufferValue(TSharedPtr<IPropertyHandle> BufferHandle, const TArray<uint8>& BufferArray)
{
	auto BufferArrayHandle = BufferHandle->AsArray();
	auto bufferCount = BufferArray.Num();
	uint32 bufferHandleCount;
	BufferArrayHandle->GetNumElements(bufferHandleCount);
	if (bufferCount != (int32)bufferHandleCount)
	{
		BufferArrayHandle->EmptyArray();
		for (int i = 0; i < bufferCount; i++)
		{
			BufferArrayHandle->AddItem();

			auto bufferHandle = BufferArrayHandle->GetElement(i);
			auto buffer = BufferArray[i];
			bufferHandle->SetValue(buffer);
		}
	}
	else
	{
		for (int i = 0; i < bufferCount; i++)
		{
			auto bufferHandle = BufferArrayHandle->GetElement(i);
			auto buffer = BufferArray[i];
			bufferHandle->SetValue(buffer);
		}
	}
}

void FLGUIEventDelegateCustomization::SetBufferLength(TSharedPtr<IPropertyHandle> BufferHandle, int32 Count)
{
	auto BufferArrayHandle = BufferHandle->AsArray();
	uint32 bufferHandleCount;
	BufferArrayHandle->GetNumElements(bufferHandleCount);
	if (Count != (int32)bufferHandleCount)
	{
		BufferArrayHandle->EmptyArray();
		for (int i = 0; i < Count; i++)
		{
			BufferArrayHandle->AddItem();
		}
	}
}

TArray<uint8> FLGUIEventDelegateCustomization::GetBuffer(TSharedPtr<IPropertyHandle> BufferHandle)
{
	auto BufferArrayHandle = BufferHandle->AsArray();
	uint32 bufferHandleCount;
	BufferArrayHandle->GetNumElements(bufferHandleCount);
	TArray<uint8> resultBuffer;
	resultBuffer.Reserve(bufferHandleCount);
	for (uint32 i = 0; i < bufferHandleCount; i++)
	{
		auto elementHandle = BufferArrayHandle->GetElement(i);
		uint8 value;
		elementHandle->GetValue(value);
		resultBuffer.Add(value);
	}
	return resultBuffer;
}

TArray<uint8> FLGUIEventDelegateCustomization::GetPropertyBuffer(TSharedPtr<IPropertyHandle> BufferHandle) const
{
	auto paramBufferArrayHandle = BufferHandle->AsArray();
	uint32 bufferCount;
	paramBufferArrayHandle->GetNumElements(bufferCount);
	TArray<uint8> paramBuffer;
	for (uint32 i = 0; i < bufferCount; i++)
	{
		auto bufferHandle = paramBufferArrayHandle->GetElement(i);
		uint8 buffer;
		bufferHandle->GetValue(buffer);
		paramBuffer.Add(buffer);
	}
	return paramBuffer;
}
int32 FLGUIEventDelegateCustomization::GetEnumValue(TSharedPtr<IPropertyHandle> ValueHandle)const
{
	uint8 Value = 0;
	ValueHandle->GetValue(Value);
	return Value;
}
FText FLGUIEventDelegateCustomization::GetTextValue(TSharedPtr<IPropertyHandle> ValueHandle)const
{
	FText Value;
	ValueHandle->GetValue(Value);
	return Value;
}
void FLGUIEventDelegateCustomization::SetTextValue(const FText& InText, ETextCommit::Type InCommitType, TSharedPtr<IPropertyHandle> ValueHandle)
{
	ValueHandle->SetValue(InText);
}

void FLGUIEventDelegateCustomization::ClearValueBuffer(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	auto handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamBuffer))->AsArray();
	uint32 NumElements = 0;
	if (handle->GetNumElements(NumElements) == FPropertyAccess::Result::Success && NumElements > 0)
	{
		handle->EmptyArray();
	}
}
void FLGUIEventDelegateCustomization::ClearReferenceValue(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	ClearObjectValue(PropertyHandle);
}
void FLGUIEventDelegateCustomization::ClearObjectValue(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	auto handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ReferenceObject));
	UObject* Obj = nullptr;
	if (handle->GetValue(Obj) == FPropertyAccess::Result::Success && Obj != nullptr)
	{
		handle->ResetToDefault();
	}
}

void FLGUIEventDelegateCustomization::OnParameterTypeChange(TSharedRef<IPropertyHandle> InDataContainerHandle)
{
	auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, BoolValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, FloatValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, DoubleValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int8Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt8Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int16Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt16Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int32Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt32Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Int64Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, UInt64Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Vector2Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Vector3Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, Vector4Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, QuatValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ColorValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, LinearColorValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, RotatorValue)); ValueHandle->ResetToDefault();
}



void FLGUIEventDelegateCustomization::CreateColorPicker(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	FLinearColor InitialColor = LinearColorGetValue(bIsLinearColor, ValueHandle, BufferHandle);

	FColorPickerArgs PickerArgs;
	{
		PickerArgs.bUseAlpha = true;
		PickerArgs.bOnlyRefreshOnMouseUp = false;
		PickerArgs.bOnlyRefreshOnOk = false;
		PickerArgs.sRGBOverride = bIsLinearColor;
		PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
		PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &FLGUIEventDelegateCustomization::LinearColorValueChange, bIsLinearColor, ValueHandle, BufferHandle);
		//PickerArgs.OnColorPickerCancelled = FOnColorPickerCancelled::CreateSP(this, &FColorStructCustomization::OnColorPickerCancelled);
		//PickerArgs.OnInteractivePickBegin = FSimpleDelegate::CreateSP(this, &FColorStructCustomization::OnColorPickerInteractiveBegin);
		//PickerArgs.OnInteractivePickEnd = FSimpleDelegate::CreateSP(this, &FColorStructCustomization::OnColorPickerInteractiveEnd);
		PickerArgs.InitialColorOverride = InitialColor;
		PickerArgs.ParentWidget = ColorPickerParentWidget;
		PickerArgs.OptionalOwningDetailsView = ColorPickerParentWidget;
		FWidgetPath ParentWidgetPath;
		if (FSlateApplication::Get().FindPathToWidget(ColorPickerParentWidget.ToSharedRef(), ParentWidgetPath))
		{
			PickerArgs.bOpenAsMenu = FSlateApplication::Get().FindMenuInWidgetPath(ParentWidgetPath).IsValid();
		}
	}

	OpenColorPicker(PickerArgs);
}

PRAGMA_ENABLE_OPTIMIZATION
#undef LOCTEXT_NAMESPACE
