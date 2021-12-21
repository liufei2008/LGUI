// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIPrefabOverrideParameterCustomization.h"
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
#include "SEnumCombobox.h"
#include "Serialization/BufferArchive.h"
#include "LGUIEditableTextPropertyHandle.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabOverrideParameterCustomization"

PRAGMA_DISABLE_OPTIMIZATION

#define LGUIActorSelfName "(ActorSelf)"

TArray<FString> FLGUIPrefabOverrideParameterCustomization::CopySourceData;

void FLGUIPrefabOverrideParameterCustomization::CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)
{
	PropertyUtilites = CustomizationUtils.GetPropertyUtilities();

	DataListHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameter, ParameterList))->AsArray();
	auto BasePropertyName = PropertyHandle->GetProperty()->GetFName();
	if (BasePropertyName == GET_MEMBER_NAME_CHECKED(ULGUIPrefabOverrideParameterObject, AutomaticParameter))
	{
		bIsAutomaticParameter = true;
	}

	auto IsTemplateHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameter, bIsTemplate));
	IsTemplateHandle->GetValue(bIsTemplate);

	if(bIsTemplate && !bIsAutomaticParameter)
	{
		ChildBuilder.AddCustomRow(LOCTEXT("Title", "Title"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Prefab Override Parameters")))
				.ToolTipText(PropertyHandle->GetToolTipText())
				//.Font(IDetailLayoutBuilder::GetDetailFont())
			]
			.ValueContent()
			[
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
						PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::OnClickListAdd))
					]
					+ SHorizontalBox::Slot()
					[
						PropertyCustomizationHelpers::MakeEmptyButton(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::OnClickListEmpty))
					]
				]
			]
		;
	}
	else if (bIsAutomaticParameter)
	{
		ChildBuilder.AddCustomRow(LOCTEXT("Title", "Title"))
			.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString(TEXT("Prefab Automatic Override Parameters")))
				.ToolTipText(PropertyHandle->GetToolTipText())
				//.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		;
	}

	uint32 ArrayCount;
	DataListHandle->GetNumElements(ArrayCount);
	for (int32 ItemIndex = 0; ItemIndex < (int32)ArrayCount; ItemIndex++)
	{
		auto DataHandle = DataListHandle->GetElement(ItemIndex);
		//HelperActor
		auto HelperActorHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, HelperActor));
		AActor* HelperActor = nullptr;
		UObject* HelperActorObject = nullptr;
		HelperActorHandle->GetValue(HelperActorObject);
		if (HelperActorObject != nullptr)
		{
			HelperActor = (AActor*)HelperActorObject;
		}

		//TargetObject
		auto TargetObjectHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, TargetObject));
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
				ComponentDisplayName = LGUIActorSelfName;
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

		//property
		auto PropertyNameHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, PropertyName));
		FName PropertyFName;
		PropertyNameHandle->GetValue(PropertyFName);
		FString PropertyName = PropertyFName.ToString();
		//parameterType
		auto PropertyParameterType = GetPropertyParameterType(DataHandle);

		bool bIsPropertyValid = false;//property valid?
		FProperty* FoundProperty = nullptr;

		if (TargetObject)
		{
			FoundProperty = FindFProperty<FProperty>(TargetObject->GetClass(), PropertyFName);
			if (FoundProperty)
			{
				if (ULGUIPrefabOverrideParameterHelper::IsStillSupported(FoundProperty, PropertyParameterType))
				{
					bIsPropertyValid = true;
				}
			}
		}

		if (!bIsPropertyValid)//not valid, show tip
		{
			if (PropertyName != "None Property" && !PropertyName.IsEmpty())
			{
				FString Prefix = "(NotValid)";
				PropertyName = Prefix.Append(PropertyName);
			}
		}
		if (PropertyName.IsEmpty())PropertyName = "None Property";

		auto DisplayNameHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, DisplayName));
		FString DisplayName;
		DisplayNameHandle->GetValue(DisplayName);
		if(bIsTemplate)
		{
			if (bIsAutomaticParameter)continue;//automatic parameter not visible in template
			//DisplayNameHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
			auto& ChildGroup = ChildBuilder.AddGroup(FName(*DisplayName), FText::FromString(DisplayName));
			ChildGroup.AddPropertyRow(DisplayNameHandle.ToSharedRef());
			ChildGroup.AddWidgetRow()
				.NameContent()
				[
					HelperActorHandle->CreatePropertyNameWidget()
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(SBox)
					.Padding(FMargin(2, 0))
					[
						SNew(SHorizontalBox)
						+ SHorizontalBox::Slot()
						[
							HelperActorHandle->CreatePropertyValueWidget()
						]
						+SHorizontalBox::Slot()
						[
							//Component
							SNew(SComboButton)
							.HasDownArrow(true)
							.IsEnabled(this, &FLGUIPrefabOverrideParameterCustomization::IsComponentSelectorMenuEnabled, DataHandle)
							.ToolTipText(LOCTEXT("Component", "Pick component"))
							.ButtonContent()
							[
								SNew(STextBlock)
								.Text(FText::FromString(ComponentDisplayName))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
							.MenuContent()
							[
								MakeComponentSelectorMenu(DataHandle)
							]
						]
					]
				]
				;
			ChildGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("Property")))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
				.ValueContent()
				[
					//Property
					SNew(SComboButton)
					.HasDownArrow(true)
					.IsEnabled(this, &FLGUIPrefabOverrideParameterCustomization::IsPropertySelectorMenuEnabled, DataHandle)
					.ToolTipText(LOCTEXT("Property", "Pick a property"))
					.ButtonContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString(PropertyName))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					.MenuContent()
					[
						MakePropertySelectorMenu(DataHandle)
					]
				]
				;

			//additional button
			int AdditionalButtonHeight = 20;
			ChildGroup.AddWidgetRow()
				.ValueContent()
				[
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
									.HeightOverride(AdditionalButtonHeight)
									.WidthOverride(30)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										.Text(LOCTEXT("C", "C"))
										.OnClicked(this, &FLGUIPrefabOverrideParameterCustomization::OnClickCopyPaste, true, DataHandle)
										.ToolTipText(LOCTEXT("Copy", "Copy"))
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
									.HeightOverride(AdditionalButtonHeight)
									.WidthOverride(30)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										.Text(LOCTEXT("P", "P"))
										.OnClicked(this, &FLGUIPrefabOverrideParameterCustomization::OnClickCopyPaste, false, DataHandle)
										.ToolTipText(LOCTEXT("Paste", "Paste copied data to this"))
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
									.HeightOverride(AdditionalButtonHeight)
									.WidthOverride(30)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										.Text(LOCTEXT("+", "+"))
										.OnClicked(this, &FLGUIPrefabOverrideParameterCustomization::OnClickAddRemove, true, DataHandle)
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
									.HeightOverride(AdditionalButtonHeight)
									.WidthOverride(30)
									[
										SNew(SButton)
										.HAlign(HAlign_Center)
										.VAlign(VAlign_Center)
										.Text(LOCTEXT("-", "-"))
										.OnClicked(this, &FLGUIPrefabOverrideParameterCustomization::OnClickAddRemove, false, DataHandle)
										.ToolTipText(LOCTEXT("Delete", "Delete this one"))
									]
								]
							]
						]
					]
				]
			;
		}
		else
		{
			if(bIsAutomaticParameter)//automatic parameter
			{
				TSharedRef<SWidget> ParameterWidget = SNew(SBox);
				if (IsValid(TargetObject) && FoundProperty != nullptr)
				{
					ParameterWidget = DrawPropertyParameter(DataHandle, PropertyParameterType, FoundProperty);
				}
				else
				{
					ParameterWidget =
						SNew(SBox)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString("(NotValid)"))
							.ToolTipText(FText::FromString(TEXT("This property is not valid, maybe TargetObject or Property is missing.")))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					;
				}
				ParameterWidget->SetToolTipText(LOCTEXT("Parameter", "Default value of the property"));
				ParameterWidget->SetEnabled(false);

				ChildBuilder.AddCustomRow(LOCTEXT("Property", "Property"))
					.NameContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString(DisplayName))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					.ValueContent()
					[
						//editable parameter
						ParameterWidget
					]
				;
			}
			else//common editable parameter
			{
				TSharedRef<SWidget> ParameterWidget = SNew(SBox);
				if (IsValid(TargetObject) && FoundProperty != nullptr)
				{
					ParameterWidget = DrawPropertyParameter(DataHandle, PropertyParameterType, FoundProperty);
				}
				else
				{
					ParameterWidget =
						SNew(SBox)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.Text(FText::FromString("(NotValid)"))
							.ToolTipText(FText::FromString(TEXT("This property is not valid, maybe TargetObject or Property is missing.")))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					;
				}
				ParameterWidget->SetToolTipText(LOCTEXT("Parameter", "Set parameter for the property"));

				ChildBuilder.AddCustomRow(LOCTEXT("Property", "Property"))
					.NameContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString(DisplayName))
						.Font(IDetailLayoutBuilder::GetDetailFont())
					]
					.ValueContent()
					[
						//editable parameter
						ParameterWidget
					]
				;
			}
		}
	}
}

void FLGUIPrefabOverrideParameterCustomization::OnSelectComponent(UActorComponent* Comp, TSharedRef<IPropertyHandle> DataHandle)
{
	auto TargetObjectHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, TargetObject));
	TargetObjectHandle->SetValue(Comp);
	PropertyUtilites->ForceRefresh();
}
void FLGUIPrefabOverrideParameterCustomization::OnSelectActorSelf(TSharedRef<IPropertyHandle> DataHandle)
{
	auto HelperActorHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, HelperActor));
	UObject* HelperActorObject = nullptr;
	HelperActorHandle->GetValue(HelperActorObject);

	auto TargetObjectHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, TargetObject));
	TargetObjectHandle->SetValue(HelperActorObject);

	PropertyUtilites->ForceRefresh();
}
void FLGUIPrefabOverrideParameterCustomization::OnSelectProperty(FName PropertyName, ELGUIPrefabOverrideParameterType ParamType, bool UseNativeParameter, TSharedRef<IPropertyHandle> DataHandle)
{
	auto PropertyNameHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, PropertyName));
	PropertyNameHandle->SetValue(PropertyName);
	auto ParamTypeHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ParamType));
	ParamTypeHandle->SetValue((uint8)ParamType);

	PropertyUtilites->ForceRefresh();
}

ELGUIPrefabOverrideParameterType FLGUIPrefabOverrideParameterCustomization::GetPropertyParameterType(TSharedRef<IPropertyHandle> DataHandle)
{
	auto paramTypeHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ParamType));
	paramTypeHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::OnParameterTypeChange, DataHandle));
	uint8 ParameterTypeUint8;
	paramTypeHandle->GetValue(ParameterTypeUint8);
	ELGUIPrefabOverrideParameterType ParameterType = (ELGUIPrefabOverrideParameterType)ParameterTypeUint8;
	return ParameterType;
}

TSharedRef<SWidget> FLGUIPrefabOverrideParameterCustomization::MakeComponentSelectorMenu(TSharedRef<IPropertyHandle> DataHandle)
{
	auto HelperActorHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, HelperActor));
	UObject* HelperActorObject = nullptr;
	HelperActorHandle->GetValue(HelperActorObject);
	if (HelperActorObject == nullptr)
	{
		return SNew(SBox);
	}

	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

	auto HelperActor = (AActor*)HelperActorObject;
	MenuBuilder.AddMenuEntry(
		FUIAction(FExecuteAction::CreateRaw(this, &FLGUIPrefabOverrideParameterCustomization::OnSelectActorSelf, DataHandle)),
		SNew(SHorizontalBox)
		+ SHorizontalBox::Slot()
		[
			SNew(STextBlock)
			.Text(FText::FromString(LGUIActorSelfName))
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
	auto& Components = HelperActor->GetComponents();
	for (auto& Comp : Components)
	{
		if(Comp->HasAnyFlags(EObjectFlags::RF_Transient))continue;
		auto CompName = Comp->GetFName();
		auto CompTypeName = Comp->GetClass()->GetName();
		MenuBuilder.AddMenuEntry(
			FUIAction(FExecuteAction::CreateRaw(this, &FLGUIPrefabOverrideParameterCustomization::OnSelectComponent, Comp, DataHandle)),
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
TSharedRef<SWidget> FLGUIPrefabOverrideParameterCustomization::MakePropertySelectorMenu(TSharedRef<IPropertyHandle> DataHandle)
{
	auto TargetObjectHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, TargetObject));
	UObject* TargetObject = nullptr;
	TargetObjectHandle->GetValue(TargetObject);
	if (TargetObject == nullptr)
	{
		return SNew(SBox);
	}

	FMenuBuilder MenuBuilder(true, MakeShareable(new FUICommandList));

	for (TFieldIterator<FProperty>It(TargetObject->GetClass()); It; ++It)
	{
		auto Property = *It;
		ELGUIPrefabOverrideParameterType ParamType;
		if (ULGUIPrefabOverrideParameterHelper::IsSupportedProperty(Property, ParamType))//show only supported type
		{
			FString ParamTypeString = ULGUIPrefabOverrideParameterHelper::ParameterTypeToName(ParamType, Property);
			auto PropertySelectorName = FString::Printf(TEXT("%s(%s)"), *Property->GetName(), *ParamTypeString);
			MenuBuilder.AddMenuEntry(
				FUIAction(FExecuteAction::CreateRaw(this, &FLGUIPrefabOverrideParameterCustomization::OnSelectProperty, Property->GetFName(), ParamType, false, DataHandle)),
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(EHorizontalAlignment::HAlign_Left)
				[
					SNew(STextBlock)
					.Text(FText::FromString(PropertySelectorName))
					.Font(IDetailLayoutBuilder::GetDetailFont())
				]
			);				
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

TSharedRef<SWidget> FLGUIPrefabOverrideParameterCustomization::DrawPropertyParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, ELGUIPrefabOverrideParameterType InPropertyParameterType, FProperty* InProperty)
{
	auto ParamBufferHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ParamBuffer));

	switch (InPropertyParameterType)
	{
	case ELGUIPrefabOverrideParameterType::Bool:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 1);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, BoolValue));
		SET_VALUE_ON_BUFFER(bool);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::BoolValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Float:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 4);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, FloatValue));
		SET_VALUE_ON_BUFFER(float);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::FloatValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Double:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 8);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, DoubleValue));
		SET_VALUE_ON_BUFFER(double);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::DoubleValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Int8:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 1);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int8Value));
		SET_VALUE_ON_BUFFER(int8);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::Int8ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::UInt8:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 1);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt8Value));
		SET_VALUE_ON_BUFFER(uint8);
		if (auto enumValue = ULGUIPrefabOverrideParameterHelper::GetEnumParameter(InProperty))
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
						.CurrentValue(this, &FLGUIPrefabOverrideParameterCustomization::GetEnumValue, ValueHandle)
						.OnEnumSelectionChanged(this, &FLGUIPrefabOverrideParameterCustomization::EnumValueChange, ValueHandle, ParamBufferHandle)
					]
				]
			;
		}
		else
		{
			ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::UInt8ValueChange, ValueHandle, ParamBufferHandle));
			return ValueHandle->CreatePropertyValueWidget();
		}
	}
	break;
	case ELGUIPrefabOverrideParameterType::Int16:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 2);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int16Value));
		SET_VALUE_ON_BUFFER(int16);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::Int16ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::UInt16:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 2);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt16Value));
		SET_VALUE_ON_BUFFER(uint16);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::UInt16ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Int32:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 4);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int32Value));
		SET_VALUE_ON_BUFFER(int32);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::Int32ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::UInt32:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 4);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt32Value));
		SET_VALUE_ON_BUFFER(uint32);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::UInt32ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Int64:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 8);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int64Value));
		SET_VALUE_ON_BUFFER(int64);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::Int64ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::UInt64:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 8);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt64Value));
		SET_VALUE_ON_BUFFER(uint64);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::UInt64ValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Vector2:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 8);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Vector2Value));
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
				.AllowResponsiveLayout(true)
				.EnableX(true)
				.EnableY(true)
				.ShowX(true)
				.ShowY(true)
				.X(this, &FLGUIPrefabOverrideParameterCustomization::Vector2GetItemValue, 0, ValueHandle, ParamBufferHandle)
				.Y(this, &FLGUIPrefabOverrideParameterCustomization::Vector2GetItemValue, 1, ValueHandle, ParamBufferHandle)
				.OnXCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector2ItemValueChange, 0, ValueHandle, ParamBufferHandle)
				.OnYCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector2ItemValueChange, 1, ValueHandle, ParamBufferHandle)
			]
		;
	}
	break;
	case ELGUIPrefabOverrideParameterType::Vector3:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 12);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Vector3Value));
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
				.AllowResponsiveLayout(true)
				.EnableX(true)
				.EnableY(true)
				.EnableZ(true)
				.ShowX(true)
				.ShowY(true)
				.ShowZ(true)
				.X(this, &FLGUIPrefabOverrideParameterCustomization::Vector3GetItemValue, 0, ValueHandle, ParamBufferHandle)
				.Y(this, &FLGUIPrefabOverrideParameterCustomization::Vector3GetItemValue, 1, ValueHandle, ParamBufferHandle)
				.Z(this, &FLGUIPrefabOverrideParameterCustomization::Vector3GetItemValue, 2, ValueHandle, ParamBufferHandle)
				.OnXCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector3ItemValueChange, 0, ValueHandle, ParamBufferHandle)
				.OnYCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector3ItemValueChange, 1, ValueHandle, ParamBufferHandle)
				.OnZCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector3ItemValueChange, 2, ValueHandle, ParamBufferHandle)
			]
		;
	}
	break;
	case ELGUIPrefabOverrideParameterType::Vector4:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 16);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Vector4Value));
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
				.AllowResponsiveLayout(true)
				.EnableX(true)
				.EnableY(true)
				.EnableZ(true)
				.EnableW(true)
				.ShowX(true)
				.ShowY(true)
				.ShowZ(true)
				.ShowW(true)
				.X(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 0, ValueHandle, ParamBufferHandle)
				.Y(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 1, ValueHandle, ParamBufferHandle)
				.Z(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 2, ValueHandle, ParamBufferHandle)
				.W(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 3, ValueHandle, ParamBufferHandle)
				.OnXCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 0, ValueHandle, ParamBufferHandle)
				.OnYCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 1, ValueHandle, ParamBufferHandle)
				.OnZCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 2, ValueHandle, ParamBufferHandle)
				.OnWCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 3, ValueHandle, ParamBufferHandle)
			]
		;
	}
	break;
	case ELGUIPrefabOverrideParameterType::Color:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 4);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ColorValue));
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
				.Color(this, &FLGUIPrefabOverrideParameterCustomization::LinearColorGetValue, false, ValueHandle, ParamBufferHandle)
				.ShowBackgroundForAlpha(true)
				.IgnoreAlpha(false)
				.OnMouseButtonDown(this, &FLGUIPrefabOverrideParameterCustomization::OnMouseButtonDownColorBlock, false, ValueHandle, ParamBufferHandle)
				.Size(FVector2D(35.0f, 12.0f))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 2.0f)
			[
				// Displays the color without alpha
				SNew(SColorBlock)
				.Color(this, &FLGUIPrefabOverrideParameterCustomization::LinearColorGetValue, false, ValueHandle, ParamBufferHandle)
				.ShowBackgroundForAlpha(false)
				.IgnoreAlpha(true)
				.OnMouseButtonDown(this, &FLGUIPrefabOverrideParameterCustomization::OnMouseButtonDownColorBlock, false, ValueHandle, ParamBufferHandle)
				.Size(FVector2D(35.0f, 12.0f))
			];
		;
	}
	break;
	case ELGUIPrefabOverrideParameterType::LinearColor:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 16);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, LinearColorValue));
		SET_VALUE_ON_BUFFER(FLinearColor);
		return SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 2.0f)
			[
				// Displays the color with alpha unless it is ignored
				SAssignNew(ColorPickerParentWidget, SColorBlock)
				.Color(this, &FLGUIPrefabOverrideParameterCustomization::LinearColorGetValue, true, ValueHandle, ParamBufferHandle)
				.ShowBackgroundForAlpha(true)
				.IgnoreAlpha(false)
				.OnMouseButtonDown(this, &FLGUIPrefabOverrideParameterCustomization::OnMouseButtonDownColorBlock, true, ValueHandle, ParamBufferHandle)
				.Size(FVector2D(35.0f, 12.0f))
			]
			+ SHorizontalBox::Slot()
			.VAlign(VAlign_Center)
			.Padding(0.0f, 2.0f)
			[
				// Displays the color without alpha
				SNew(SColorBlock)
				.Color(this, &FLGUIPrefabOverrideParameterCustomization::LinearColorGetValue, true, ValueHandle, ParamBufferHandle)
				.ShowBackgroundForAlpha(false)
				.IgnoreAlpha(true)
				.OnMouseButtonDown(this, &FLGUIPrefabOverrideParameterCustomization::OnMouseButtonDownColorBlock, true, ValueHandle, ParamBufferHandle)
				.Size(FVector2D(35.0f, 12.0f))
			];
		;
	}
	break;
	case ELGUIPrefabOverrideParameterType::Quaternion:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 16);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, QuatValue));
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
				.AllowResponsiveLayout(true)
				.EnableX(true)
				.EnableY(true)
				.EnableZ(true)
				.EnableW(true)
				.ShowX(true)
				.ShowY(true)
				.ShowZ(true)
				.ShowW(true)
				.X(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 0, ValueHandle, ParamBufferHandle)
				.Y(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 1, ValueHandle, ParamBufferHandle)
				.Z(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 2, ValueHandle, ParamBufferHandle)
				.W(this, &FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue, 3, ValueHandle, ParamBufferHandle)
				.OnXCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 0, ValueHandle, ParamBufferHandle)
				.OnYCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 1, ValueHandle, ParamBufferHandle)
				.OnZCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 2, ValueHandle, ParamBufferHandle)
				.OnWCommitted(this, &FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange, 3, ValueHandle, ParamBufferHandle)
			]
		;
	}
	break;
	case ELGUIPrefabOverrideParameterType::String:
	{
		ClearReferenceValue(InDataContainerHandle);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, StringValue));
		SET_VALUE_ON_BUFFER(FString);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::StringValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	break;
	case ELGUIPrefabOverrideParameterType::Name:
	{
		ClearReferenceValue(InDataContainerHandle);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, NameValue));
		SET_VALUE_ON_BUFFER(FName);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::NameValueChange, ValueHandle, ParamBufferHandle));
		return ValueHandle->CreatePropertyValueWidget();
	}
	case ELGUIPrefabOverrideParameterType::Text:
	{
		ClearReferenceValue(InDataContainerHandle);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, TextValue));
		SET_VALUE_ON_BUFFER(FText);
		ValueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::TextValueChange, ValueHandle, ParamBufferHandle));
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
	case ELGUIPrefabOverrideParameterType::Object:
	case ELGUIPrefabOverrideParameterType::Actor:
	case ELGUIPrefabOverrideParameterType::Class:
	{
		return
			SNew(SBox)
			.MinDesiredWidth(500)
			[
				DrawPropertyReferenceParameter(InDataContainerHandle, InPropertyParameterType, InProperty)
			];
	}
	break;
	case ELGUIPrefabOverrideParameterType::Rotator:
	{
		ClearReferenceValue(InDataContainerHandle);
		SetBufferLength(ParamBufferHandle, 12);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, RotatorValue));
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
				.AllowResponsiveLayout(true)
				.Roll(this, &FLGUIPrefabOverrideParameterCustomization::RotatorGetItemValue, 0, ValueHandle, ParamBufferHandle)
				.Pitch(this, &FLGUIPrefabOverrideParameterCustomization::RotatorGetItemValue, 1, ValueHandle, ParamBufferHandle)
				.Yaw(this, &FLGUIPrefabOverrideParameterCustomization::RotatorGetItemValue, 2, ValueHandle, ParamBufferHandle)
				.OnRollCommitted(this, &FLGUIPrefabOverrideParameterCustomization::RotatorValueChange, 0, ValueHandle, ParamBufferHandle)
				.OnPitchCommitted(this, &FLGUIPrefabOverrideParameterCustomization::RotatorValueChange, 1, ValueHandle, ParamBufferHandle)
				.OnYawCommitted(this, &FLGUIPrefabOverrideParameterCustomization::RotatorValueChange, 2, ValueHandle, ParamBufferHandle)
				.TypeInterface(TypeInterface)
			]
		;
	}
	break;
	default:
	{
		ClearValueBuffer(InDataContainerHandle);
		ClearReferenceValue(InDataContainerHandle);
		return
			SNew(STextBlock)
			.Font(IDetailLayoutBuilder::GetDetailFont())
			.Text(FText::FromString("(Not handled)"));
	}
	}
}
//property's parameter editor
TSharedRef<SWidget> FLGUIPrefabOverrideParameterCustomization::DrawPropertyReferenceParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, ELGUIPrefabOverrideParameterType PropertyParameterType, FProperty* InProperty)
{
	auto ParamBufferHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ParamBuffer));

	TSharedPtr<SWidget> ParameterContent;
	switch (PropertyParameterType)
	{
	case ELGUIPrefabOverrideParameterType::Object:
	{
		ClearValueBuffer(InDataContainerHandle);
		return SNew(SObjectPropertyEntryBox)
			.IsEnabled(true)
			.AllowedClass(ULGUIPrefabOverrideParameterHelper::GetObjectParameterClass(InProperty))
			.PropertyHandle(InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ReferenceObject)))
			.AllowClear(true)
			.ToolTipText(LOCTEXT("UObjectTips", "UObject only referece asset, dont use for Actor"))
			.OnObjectChanged(this, &FLGUIPrefabOverrideParameterCustomization::ObjectValueChange, ParamBufferHandle, InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ReferenceObject)), true);
	}
	break;
	case ELGUIPrefabOverrideParameterType::Actor:
	{
		ClearValueBuffer(InDataContainerHandle);
		return SNew(SObjectPropertyEntryBox)
			.IsEnabled(true)
			.AllowedClass(ULGUIPrefabOverrideParameterHelper::GetObjectParameterClass(InProperty))
			.PropertyHandle(InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ReferenceObject)))
			.AllowClear(true)
			.OnObjectChanged(this, &FLGUIPrefabOverrideParameterCustomization::ObjectValueChange, ParamBufferHandle, InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ReferenceObject)), false);
	}
	break;
	case ELGUIPrefabOverrideParameterType::Class:
	{
		auto MetaClass = ULGUIPrefabOverrideParameterHelper::GetClassParameterClass(InProperty);
		auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ReferenceObject));
		ClearValueBuffer(InDataContainerHandle);
		return SNew(SClassPropertyEntryBox)
			.IsEnabled(true)
			.AllowAbstract(true)
			.AllowNone(true)
			.MetaClass(MetaClass)
			.SelectedClass(this, &FLGUIPrefabOverrideParameterCustomization::GetClassValue, ValueHandle)
			.OnSetClass(this, &FLGUIPrefabOverrideParameterCustomization::ClassValueChange, ValueHandle);
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

void FLGUIPrefabOverrideParameterCustomization::ObjectValueChange(const FAssetData& InObj, TSharedPtr<IPropertyHandle> BufferHandle, TSharedPtr<IPropertyHandle> ObjectReferenceHandle, bool ObjectOrActor)
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
const UClass* FLGUIPrefabOverrideParameterCustomization::GetClassValue(TSharedPtr<IPropertyHandle> ClassReferenceHandle)const
{
	UObject* referenceClassObject = nullptr;
	ClassReferenceHandle->GetValue(referenceClassObject);
	return (UClass*)referenceClassObject;
}
void FLGUIPrefabOverrideParameterCustomization::ClassValueChange(const UClass* InClass, TSharedPtr<IPropertyHandle> ClassReferenceHandle)
{
	ClassReferenceHandle->SetValue(InClass);
}
void FLGUIPrefabOverrideParameterCustomization::EnumValueChange(int32 InValue, ESelectInfo::Type SelectionType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
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

void FLGUIPrefabOverrideParameterCustomization::BoolValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(bool);
}
void FLGUIPrefabOverrideParameterCustomization::FloatValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(float);
}
void FLGUIPrefabOverrideParameterCustomization::DoubleValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(double);
}
void FLGUIPrefabOverrideParameterCustomization::Int8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int8);
}
void FLGUIPrefabOverrideParameterCustomization::UInt8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint8);
}
void FLGUIPrefabOverrideParameterCustomization::Int16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int16);
}
void FLGUIPrefabOverrideParameterCustomization::UInt16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint16);
}
void FLGUIPrefabOverrideParameterCustomization::Int32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int32);
}
void FLGUIPrefabOverrideParameterCustomization::UInt32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint32);
}
void FLGUIPrefabOverrideParameterCustomization::Int64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(int64);
}
void FLGUIPrefabOverrideParameterCustomization::UInt64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(uint64);
}
void FLGUIPrefabOverrideParameterCustomization::StringValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(FString);
}
void FLGUIPrefabOverrideParameterCustomization::NameValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(FName);
}
void FLGUIPrefabOverrideParameterCustomization::TextValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	SET_BUFFER_ON_VALUE(FText);
	//FText Value;
	//ValueHandle->GetValue(Value);
	//FBufferArchive ToBinary;
	//ToBinary << Value;
	//SetBufferValue(BufferHandle, ToBinary);
}
void FLGUIPrefabOverrideParameterCustomization::Vector2ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
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
TOptional<float> FLGUIPrefabOverrideParameterCustomization::Vector2GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
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
void FLGUIPrefabOverrideParameterCustomization::Vector3ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
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
TOptional<float> FLGUIPrefabOverrideParameterCustomization::Vector3GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
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
void FLGUIPrefabOverrideParameterCustomization::Vector4ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
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
TOptional<float> FLGUIPrefabOverrideParameterCustomization::Vector4GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
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
FLinearColor FLGUIPrefabOverrideParameterCustomization::LinearColorGetValue(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
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
void FLGUIPrefabOverrideParameterCustomization::LinearColorValueChange(FLinearColor NewValue, bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
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
FReply FLGUIPrefabOverrideParameterCustomization::OnMouseButtonDownColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	if (MouseEvent.GetEffectingButton() != EKeys::LeftMouseButton)
	{
		return FReply::Unhandled();
	}

	CreateColorPicker(bIsLinearColor, ValueHandle, BufferHandle);

	return FReply::Handled();
}
TOptional<float> FLGUIPrefabOverrideParameterCustomization::RotatorGetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const
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
void FLGUIPrefabOverrideParameterCustomization::RotatorValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
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
void FLGUIPrefabOverrideParameterCustomization::SetBufferValue(TSharedPtr<IPropertyHandle> BufferHandle, const TArray<uint8>& BufferArray)
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

void FLGUIPrefabOverrideParameterCustomization::SetBufferLength(TSharedPtr<IPropertyHandle> BufferHandle, int32 Count)
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

TArray<uint8> FLGUIPrefabOverrideParameterCustomization::GetBuffer(TSharedPtr<IPropertyHandle> BufferHandle)
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

TArray<uint8> FLGUIPrefabOverrideParameterCustomization::GetPropertyBuffer(TSharedPtr<IPropertyHandle> BufferHandle) const
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
int32 FLGUIPrefabOverrideParameterCustomization::GetEnumValue(TSharedPtr<IPropertyHandle> ValueHandle)const
{
	uint8 Value = 0;
	ValueHandle->GetValue(Value);
	return Value;
}

void FLGUIPrefabOverrideParameterCustomization::ClearValueBuffer(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	auto handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ParamBuffer))->AsArray();
	handle->EmptyArray();
}
void FLGUIPrefabOverrideParameterCustomization::ClearReferenceValue(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	ClearObjectValue(PropertyHandle);
}
void FLGUIPrefabOverrideParameterCustomization::ClearObjectValue(TSharedPtr<IPropertyHandle> PropertyHandle)
{
	auto handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ReferenceObject));
	handle->ResetToDefault();
}

void FLGUIPrefabOverrideParameterCustomization::OnParameterTypeChange(TSharedRef<IPropertyHandle> InDataContainerHandle)
{
	auto ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, BoolValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, FloatValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, DoubleValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int8Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt8Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int16Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt16Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int32Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt32Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Int64Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, UInt64Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Vector2Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Vector3Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, Vector4Value)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, QuatValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, ColorValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, LinearColorValue)); ValueHandle->ResetToDefault();
	ValueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, RotatorValue)); ValueHandle->ResetToDefault();
}

void FLGUIPrefabOverrideParameterCustomization::CreateColorPicker(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
{
	FLinearColor InitialColor = LinearColorGetValue(bIsLinearColor, ValueHandle, BufferHandle);

	FColorPickerArgs PickerArgs;
	{
		PickerArgs.bUseAlpha = true;
		PickerArgs.bOnlyRefreshOnMouseUp = false;
		PickerArgs.bOnlyRefreshOnOk = false;
		PickerArgs.sRGBOverride = bIsLinearColor;
		PickerArgs.DisplayGamma = TAttribute<float>::Create(TAttribute<float>::FGetter::CreateUObject(GEngine, &UEngine::GetDisplayGamma));
		PickerArgs.OnColorCommitted = FOnLinearColorValueChanged::CreateSP(this, &FLGUIPrefabOverrideParameterCustomization::LinearColorValueChange, bIsLinearColor, ValueHandle, BufferHandle);
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


FReply FLGUIPrefabOverrideParameterCustomization::OnClickAddRemove(bool AddOrRemove, TSharedRef<IPropertyHandle> DataHandle)
{
	int Index = DataHandle->GetIndexInArray();
	auto DataArrayHandle = DataHandle->GetParentHandle()->AsArray();
	uint32 Count;
	DataArrayHandle->GetNumElements(Count);
	if (AddOrRemove)
	{
		if (Count == 0)
		{
			DataArrayHandle->AddItem();
		}
		else
		{
			if (Index == Count - 1)//current is last, add to last
				DataArrayHandle->AddItem();
			else
				DataArrayHandle->Insert(Index + 1);
		}
	}
	else
	{
		if (Count != 0)
			DataArrayHandle->DeleteItem(Index);
	}
	PropertyUtilites->ForceRefresh();
	return FReply::Handled();
}
FReply FLGUIPrefabOverrideParameterCustomization::OnClickCopyPaste(bool CopyOrPaste, TSharedRef<IPropertyHandle> DataHandle)
{
	if (CopyOrPaste)
	{
		CopySourceData.Reset();
		DataHandle->GetPerObjectValues(CopySourceData);
	}
	else
	{
		DataHandle->SetPerObjectValues(CopySourceData);
		PropertyUtilites->ForceRefresh();
	}
	return FReply::Handled();
}

void FLGUIPrefabOverrideParameterCustomization::OnClickListAdd()
{
	DataListHandle->AddItem();
	PropertyUtilites->ForceRefresh();
}
void FLGUIPrefabOverrideParameterCustomization::OnClickListEmpty()
{
	DataListHandle->EmptyArray();
	PropertyUtilites->ForceRefresh();
}

PRAGMA_ENABLE_OPTIMIZATION
#undef LOCTEXT_NAMESPACE
