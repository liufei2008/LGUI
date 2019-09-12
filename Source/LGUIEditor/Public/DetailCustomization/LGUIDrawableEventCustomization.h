// Copyright 2019 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "LGUIEditorUtils.h"
#include "IDetailChildrenBuilder.h"
#include "IDetailPropertyRow.h"
#include "IDetailGroup.h"
#include "IPropertyUtilities.h"
#include "Utils/BitConverter.h"
#include "PropertyCustomizationHelpers.h"
#include "Widget/LGUIEnumComboBox.h"
#include "Window/LGUIEventComponentSelector.h"
#include "Window/LGUIEventFunctionSelector.h"

#pragma once

#define LOCTEXT_NAMESPACE "LGUIDrawableEventCustomization"

class ILGUIDrawableEventCustomizationInterface
{
public:
	virtual void OnSelectComponent(IPropertyHandleArray* InEventListHandle, FName CompName, int32 itemIndex) = 0;
	virtual void OnSelectActorSelf(IPropertyHandleArray* InEventListHandle, int32 itemIndex) = 0;
	virtual void OnSelectFunction(IPropertyHandleArray* InEventListHandle, FName FuncName, int32 EventArrayItemIndex, TArray<LGUIDrawableEventParameterType> ParamType, bool UseNativeParameter) = 0;
};

/**
 * 
 */
template<class Event, class EventData>
class FLGUIDrawableEventCustomization : public IPropertyTypeCustomization, public ILGUIDrawableEventCustomizationInterface
{
protected:
	TSharedPtr<IPropertyUtilities> PropertyUtilites;
	TSharedPtr<IPropertyHandleArray> EventListHandle;
	static TArray<FString> CopySourceData;
	TArray<LGUIDrawableEventParameterType> EventParameterTypeArray;
private:
	int32 ParameterCount = 1;
	bool IsParameterTypeArrayValid(const TArray<LGUIDrawableEventParameterType> InArray)
	{
		for (auto item : InArray)
		{
			if (item == LGUIDrawableEventParameterType::None)return false;
		}
		return true;
	}
public:
	FLGUIDrawableEventCustomization(int32 InParameterCount)
	{
		ParameterCount = InParameterCount;
	}
	~FLGUIDrawableEventCustomization()
	{
		SLGUIEventComponentSelector::TargetCustomization = nullptr;
		SLGUIEventFunctionSelector::TargetCustomization = nullptr;
	}
	/** IDetailCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};

	virtual TArray<LGUIDrawableEventParameterType> GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle) = 0;
	virtual void AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailGroup& DetailGroup) = 0;
	virtual TArray<LGUIDrawableEventParameterType> GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle) = 0;
	virtual TSharedRef<IPropertyHandle> GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex = 1/*start from 1, because 0 is default one parameter*/) = 0;

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)override
	{
		PropertyUtilites = CustomizationUtils.GetPropertyUtilities();

		EventParameterTypeArray = GetNativeParameterTypeArray(PropertyHandle);
		EventListHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(Event, eventList))->AsArray();
		uint32 arrayCount;
		EventListHandle->GetNumElements(arrayCount);
		auto nameStr = PropertyHandle->GetPropertyDisplayName().ToString();
		FString paramTypeString(TEXT(""));
		for (auto item : EventParameterTypeArray)
		{
			paramTypeString += ULGUIDrawableEventParameterHelper::ParameterTypeToName(item, nullptr);
			paramTypeString.AppendChar(',');
		}
		paramTypeString.RemoveAt(paramTypeString.Len() - 1);
		nameStr = nameStr + "(" + paramTypeString + ")";
		FString valueStr = FString::Printf(TEXT("Listener count %d"), arrayCount);
		IDetailGroup& eventGroup = ChildBuilder.AddGroup(FName("LGUIEvent"), FText::FromString(nameStr));
		eventGroup.HeaderRow()
		.NameContent()
		[
			SNew(STextBlock)
			.Text(FText::FromString(nameStr))
		]
		.ValueContent()
		.MinDesiredWidth(500)
		[
			IsParameterTypeArrayValid(EventParameterTypeArray) ?
			(
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(2, 0)
				[
					SNew(STextBlock)
					.Text(FText::FromString(valueStr))
				]
				+ SHorizontalBox::Slot()
				.AutoWidth()
				.HAlign(HAlign_Left)
				.VAlign(VAlign_Center)
				.Padding(2, 0)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						PropertyCustomizationHelpers::MakeAddButton(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::OnClickListAdd))
					]
					+ SHorizontalBox::Slot()
					[
						PropertyCustomizationHelpers::MakeEmptyButton(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::OnClickListEmpty))
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
				]
			)
		];

		//add parameter type property
		bool canChangeSupportParameterType;
		auto canChangeSupportParameterTypeHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(Event, canChangeSupportParameterType));
		canChangeSupportParameterTypeHandle->GetValue(canChangeSupportParameterType);
		if (canChangeSupportParameterType)
		{
			AddNativeParameterTypeProperty(PropertyHandle, eventGroup);
		}

		for (int32 i = 0; i < (int32)arrayCount; i++)
		{
			auto itemHandle = EventListHandle->GetElement(i);
			auto targetActorHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, targetActor));
			AActor* actor = nullptr;
			UObject* actorObject = nullptr;
			targetActorHandle->GetValue(actorObject);//get event target actor
			if (actorObject != nullptr)
			{
				actor = (AActor*)actorObject;
			}
			targetActorHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
			//eventGroup.AddPropertyRow(itemHandle);//show origin editor, for test
			//component
			auto componentHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, componentClass));
			UClass* componentClass = nullptr;
			UObject* componentClassObject = nullptr;
			FString componentDisplayName;
			componentHandle->GetValue(componentClassObject);
			if (componentClassObject != nullptr)
			{
				componentClass = (UClass*)componentClassObject;
				componentDisplayName = componentClass->GetName();
			}
			//function
			auto functionHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, functionName));
			FName functionFName;
			functionHandle->GetValue(functionFName);
			FString functionName = functionFName.ToString();
			//parameterType
			auto functionParameterTypeArray = GetEventDataParameterTypeArray(itemHandle);

			UObject* EventTarget = nullptr;//event target
			bool ComponentValid = false;//event target component valid?
			bool EventFunctionValid = false;//event target function valid?
			UFunction* eventFunction = nullptr;
			if (actor)//if event target actor is valid, check if Component valid
			{
				if (componentClass != nullptr)
				{
					if (componentClass != actor->GetClass())//if target is not actor self
					{
						TArray<UActorComponent*> Components;
						((AActor*)actor)->GetComponents(Components);
						int32 CompCount = Components.Num();

						for (int j = 0; j < CompCount; j++)
						{
							auto Comp = Components[j];
							if (componentClass == Comp->GetClass())//valid component
							{
								EventTarget = Comp;
								ComponentValid = true;
								j = CompCount;//break loop
							}
						}
					}
					else//if target is actor self, means event target is valid too, just mark ComponentValid = true
					{
						ComponentValid = true;
						EventTarget = actor;
					}
					//check if function valid
					if (EventTarget)
					{
						eventFunction = EventTarget->FindFunction(functionFName);
						if (eventFunction)
						{
							if (ULGUIDrawableEventParameterHelper::IsStillSupported(eventFunction, functionParameterTypeArray))
							{
								EventFunctionValid = true;
							}
						}
					}
				}
			}
			if (!ComponentValid)//component not valid
			{
				if (componentClass != nullptr)//componentClass is valid, means target actor lose the component
				{
					FString Prefix = "(Missing)";
					componentDisplayName = Prefix.Append(componentDisplayName);
				}
			}
			if (!EventFunctionValid)//function not valid, show tip
			{
				if (functionName != "No Function" && !functionName.IsEmpty())
				{
					FString Prefix = "(NotValid)";
					functionName = Prefix.Append(functionName);
				}
			}
			if (componentDisplayName.IsEmpty())componentDisplayName = "No Component";
			if (functionName.IsEmpty())functionName = "No Function";
			//actor
			eventGroup.AddPropertyRow(targetActorHandle.ToSharedRef());
			//component
			eventGroup.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Component"))
			]
			.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SButton)
				.OnClicked(this, &FLGUIDrawableEventCustomization::OnClickComponentNameButton, actor, i)
				[
					LGUIEditorUtils::GenerateArrowButtonContent(FText::FromString(componentDisplayName))
				]
			];

			//function
			eventGroup.AddWidgetRow()
			.NameContent()
			[
				SNew(STextBlock)
				.Text(FText::FromString("Function"))
			]
			.ValueContent()
			.MinDesiredWidth(500)
			[
				SNew(SButton)
				.OnClicked(this, &FLGUIDrawableEventCustomization::OnClickFunctionNameButton, actor, i)
				[
					LGUIEditorUtils::GenerateArrowButtonContent(FText::FromString(functionName))
				]
			];
			//parameter
			bool useNativeParameter = false;
			auto useNativeParameterHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UseNativeParameter));
			useNativeParameterHandle->GetValue(useNativeParameter);
			if ((EventParameterTypeArray == functionParameterTypeArray) && useNativeParameter)//support native parameter
			{
				//clear buffer and value
				ClearNumericValueBuffer(itemHandle);
				ClearReferenceValue(itemHandle);
				if (ParameterCount > 1)
				{
					for (int paramIndex = 1; paramIndex < ParameterCount; paramIndex++)
					{
						auto dataContainerHandle = GetAdditionalParameterContainer(itemHandle, i);
						ClearNumericValueBuffer(dataContainerHandle);
						ClearReferenceValue(dataContainerHandle);
					}
				}
				//add native parameter widget
				eventGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Parameter"))
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.Text(FText::FromString("(NativeParameter)"))
				];
			}
			else
			{
				DrawFunctionParameter(itemHandle, functionParameterTypeArray, 0, eventFunction, eventGroup);
				if (ParameterCount > 1)
				{
					for (int paramIndex = 1; paramIndex < ParameterCount; paramIndex++)
					{
						DrawFunctionParameter(GetAdditionalParameterContainer(itemHandle, i), functionParameterTypeArray, paramIndex, eventFunction, eventGroup);
					}
				}
			}

			//additional button
			eventGroup.AddWidgetRow()
			.ValueContent()
			.MinDesiredWidth(500)
			[
				//add, delete
				SNew(SHorizontalBox)
				+ SHorizontalBox::Slot()
				.HAlign(HAlign_Right)
				[
					SNew(SHorizontalBox)
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(30)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Text(LOCTEXT("C", "C"))
							.OnClicked(this, &FLGUIDrawableEventCustomization::OnClickCopyPaste, true, i)
							.ToolTipText(LOCTEXT("Copy", "Copy this function"))
						]
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(30)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Text(LOCTEXT("P", "P"))
							.OnClicked(this, &FLGUIDrawableEventCustomization::OnClickCopyPaste, false, i)
							.ToolTipText(LOCTEXT("Paste", "Paste copied function to this function"))
						]
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(30)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Text(LOCTEXT("+", "+"))
							.OnClicked(this, &FLGUIDrawableEventCustomization::OnClickAddRemove, true, i, (int32)arrayCount)
							.ToolTipText(LOCTEXT("Add", "Add new one"))
						]
					]
					+ SHorizontalBox::Slot()
					[
						SNew(SBox)
						.WidthOverride(30)
						[
							SNew(SButton)
							.HAlign(HAlign_Center)
							.VAlign(VAlign_Center)
							.Text(LOCTEXT("-", "-"))
							.OnClicked(this, &FLGUIDrawableEventCustomization::OnClickAddRemove, false, i, (int32)arrayCount)
							.ToolTipText(LOCTEXT("Delete", "Delete this one"))
						]
					]
				]
			];
		}
	}

	virtual void OnSelectComponent(IPropertyHandleArray* InEventListHandle, FName CompName, int32 itemIndex)override
	{
		auto itemHandle = InEventListHandle->GetElement(itemIndex);
		auto actorHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, targetActor));
		UObject* actorObject = nullptr;
		actorHandle->GetValue(actorObject);
		auto compClassHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, componentClass));
		auto& componentList = ((AActor*)actorObject)->GetComponents();
		for (auto comp : componentList)
		{
			if (comp->GetFName() == CompName)
			{
				compClassHandle->SetValue(comp->GetClass());
				break;
			}
		}
		PropertyUtilites->ForceRefresh();
	}
	virtual void OnSelectActorSelf(IPropertyHandleArray* InEventListHandle, int32 itemIndex)override
	{
		auto itemHandle = InEventListHandle->GetElement(itemIndex);

		auto actorHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, targetActor));
		UObject* actorObject = nullptr;
		actorHandle->GetValue(actorObject);
		auto componentClassHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, componentClass));
		componentClassHandle->SetValue(actorObject->GetClass());

		PropertyUtilites->ForceRefresh();
	}
	virtual void SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIDrawableEventParameterType> ParameterTypeArray) = 0;
	virtual void OnSelectFunction(IPropertyHandleArray* InEventListHandle, FName FuncName, int32 EventArrayItemIndex, TArray<LGUIDrawableEventParameterType> ParamTypeArray, bool UseNativeParameter)override
	{
		auto itemHandle = InEventListHandle->GetElement(EventArrayItemIndex);
		auto nameHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, functionName));
		nameHandle->SetValue(FuncName);
		SetEventDataParameterType(itemHandle, ParamTypeArray);
		auto useNativeParameterHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UseNativeParameter));
		useNativeParameterHandle->SetValue(UseNativeParameter);
		PropertyUtilites->ForceRefresh();
	}
protected:
	void OnClickListAdd()
	{
		EventListHandle->AddItem();
		PropertyUtilites->ForceRefresh();
	}
	void OnClickListEmpty()
	{
		EventListHandle->EmptyArray();
		PropertyUtilites->ForceRefresh();
	}
	FReply OnClickAddRemove(bool AddOrRemove, int32 Index, int32 Count)
	{
		if (AddOrRemove)
		{
			if (Count == 0)
			{
				EventListHandle->AddItem();
			}
			else
			{
				if (Index == Count - 1)//current is last, add to last
					EventListHandle->AddItem();
				else
					EventListHandle->Insert(Index + 1);
			}
		}
		else
		{
			if (Count != 0)
				EventListHandle->DeleteItem(Index);
		}
		PropertyUtilites->ForceRefresh();
		return FReply::Handled();
	}
	FReply OnClickCopyPaste(bool CopyOrPaste, int32 Index)
	{
		auto eventDataHandle = EventListHandle->GetElement(Index);
		if (CopyOrPaste)
		{
			CopySourceData.Reset();
			eventDataHandle->GetPerObjectValues(CopySourceData);
		}
		else
		{
			eventDataHandle->SetPerObjectValues(CopySourceData);
		}
		PropertyUtilites->ForceRefresh();
		return FReply::Handled();
	}
	FReply OnClickComponentNameButton(AActor* TargetActor, int32 itemIndex)
	{
		SLGUIEventComponentSelector::TargetCustomization = this;
		SLGUIEventComponentSelector::TargetActor = TargetActor;
		SLGUIEventComponentSelector::TargetItemIndex = itemIndex;
		SLGUIEventComponentSelector::EventListHandle = EventListHandle.Get();
		FGlobalTabmanager::Get()->InvokeTab(FLGUIEditorModule::LGUIEventComponentSelectorName);
		return FReply::Handled();
	}

	FReply OnClickFunctionNameButton(AActor* TargetActor, int32 itemIndex)
	{
		auto itemHandle = EventListHandle->GetElement(itemIndex);
		auto compClassHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, componentClass));
		UObject* componentClassObject = nullptr;
		compClassHandle->GetValue(componentClassObject);
		if (componentClassObject == nullptr)
		{
			UE_LOG(LGUIEditor, Error, TEXT("Target component is null!"));
			return FReply::Handled();
		}

		auto actorHandle = itemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, targetActor));
		UObject* actorObject = nullptr;
		actorHandle->GetValue(actorObject);
		if (actorObject == nullptr)
		{
			UE_LOG(LGUIEditor, Error, TEXT("Target actor is null!"));
			return FReply::Handled();
		}

		UClass* componentClass = (UClass*)componentClassObject;

		bool GetObject = false;
		if (actorObject->GetClass() == componentClass)//actor self
		{
			SLGUIEventFunctionSelector::TargetObject = TargetActor;
			GetObject = true;
		}
		else
		{
			auto& Components = TargetActor->GetComponents();
			for (auto Comp : Components)//fint component by class Component
			{
				if (Comp->GetClass() == componentClass)
				{
					SLGUIEventFunctionSelector::TargetObject = Comp;
					GetObject = true;
					break;
				}
			}
		}
		if (GetObject)
		{
			SLGUIEventFunctionSelector::TargetCustomization = this;
			SLGUIEventFunctionSelector::NativeSupportParameter = EventParameterTypeArray;
			SLGUIEventFunctionSelector::TargetItemIndex = itemIndex;
			SLGUIEventFunctionSelector::EventListHandle = EventListHandle.Get();
			FGlobalTabmanager::Get()->InvokeTab(FLGUIEditorModule::LGUIEventFunctionSelectorName);
		}
		else
		{
			UE_LOG(LGUIEditor, Error, TEXT("Target component is missing!"));
		}

		return FReply::Handled();
	}
	
	void DrawFunctionParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, TArray<LGUIDrawableEventParameterType> InFunctionParameterTypeArray, int32 InParameterIndex, UFunction* InFunction, IDetailGroup& InDetailGroup)
	{
		auto paramBufferHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ParamBuffer));
		auto functionParameterType = InFunctionParameterTypeArray[InParameterIndex];
		if (functionParameterType != LGUIDrawableEventParameterType::None)//None means not select function yet
		{
			switch (functionParameterType)
			{
			case LGUIDrawableEventParameterType::Empty:
			{
				ClearNumericValueBuffer(InDataContainerHandle);
				ClearReferenceValue(InDataContainerHandle);
				InDetailGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Parameter"))
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.Text(FText::FromString("(No parameter)"))
				];
			}
			break;
			case LGUIDrawableEventParameterType::Bool:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 1);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, BoolValue));
				valueHandle->SetValue(BitConverter::ToBoolean(GetBuffer(paramBufferHandle, 1)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::BoolValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Float:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 4);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, FloatValue));
				valueHandle->SetValue(BitConverter::ToFloat(GetBuffer(paramBufferHandle, 4)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::FloatValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Double:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 8);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, DoubleValue));
				valueHandle->SetValue(BitConverter::ToDouble(GetBuffer(paramBufferHandle, 8)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::DoubleValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Int8:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 1);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int8Value));
				valueHandle->SetValue(BitConverter::ToInt8(GetBuffer(paramBufferHandle, 1)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Int8ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::UInt8:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 1);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt8Value));
				valueHandle->SetValue(BitConverter::ToUInt8(GetBuffer(paramBufferHandle, 1)));
				if (auto enumValue = ULGUIDrawableEventParameterHelper::GetEnumParameter(InFunction))
				{
					InDetailGroup.AddWidgetRow()
					.NameContent()
					[
						SNew(STextBlock)
						.Text(FText::FromString("Parameter"))
					]
					.ValueContent()
					.MinDesiredWidth(500)
					[
						SNew(SEnumCombobox, enumValue)
						.CurrentValue(this, &FLGUIDrawableEventCustomization::GetEnumValue, valueHandle)
						.OnEnumSelectionChanged(this, &FLGUIDrawableEventCustomization::EnumValueChange, valueHandle, paramBufferHandle)
					];
				}
				else
				{
					valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::UInt8ValueChange, valueHandle, paramBufferHandle));
					InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
				}
			}
			break;
			case LGUIDrawableEventParameterType::Int16:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 2);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int16Value));
				valueHandle->SetValue(BitConverter::ToInt16(GetBuffer(paramBufferHandle, 2)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Int16ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::UInt16:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 2);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt16Value));
				valueHandle->SetValue(BitConverter::ToUInt16(GetBuffer(paramBufferHandle, 2)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::UInt16ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Int32:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 4);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int32Value));
				valueHandle->SetValue(BitConverter::ToInt32(GetBuffer(paramBufferHandle, 4)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Int32ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::UInt32:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 4);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt32Value));
				valueHandle->SetValue(BitConverter::ToUInt32(GetBuffer(paramBufferHandle, 4)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::UInt32ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Int64:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 8);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int64Value));
				valueHandle->SetValue(BitConverter::ToInt64(GetBuffer(paramBufferHandle, 8)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Int64ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::UInt64:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 8);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt64Value));
				valueHandle->SetValue(BitConverter::ToUInt64(GetBuffer(paramBufferHandle, 8)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::UInt64ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			case LGUIDrawableEventParameterType::Vector2:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 8);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Vector2Value));
				valueHandle->SetValue(BitConverter::ToVector2(GetBuffer(paramBufferHandle, 8)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Vector2ValueChange, valueHandle, paramBufferHandle));
				valueHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Vector2ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Vector3:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 12);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Vector3Value));
				valueHandle->SetValue(BitConverter::ToVector3(GetBuffer(paramBufferHandle, 12)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Vector3ValueChange, valueHandle, paramBufferHandle));
				valueHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Vector3ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Vector4:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 16);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Vector4Value));
				valueHandle->SetValue(BitConverter::ToVector4(GetBuffer(paramBufferHandle, 16)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Vector4ValueChange, valueHandle, paramBufferHandle));
				valueHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::Vector4ValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Color:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 4);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ColorValue));
				valueHandle->SetValue(BitConverter::ToInt32(GetBuffer(paramBufferHandle, 4)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::ColorValueChange, valueHandle, paramBufferHandle));
				valueHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::ColorValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::LinearColor:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 16);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, LinearColorValue));
				valueHandle->SetValue(BitConverter::ToLinearColor(GetBuffer(paramBufferHandle, 16)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::LinearColorValueChange, valueHandle, paramBufferHandle));
				valueHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::LinearColorValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::Quaternion:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 16);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, QuatValue));
				valueHandle->SetValue(BitConverter::ToQuat(GetBuffer(paramBufferHandle, 16)));
				InDetailGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Parameter"))
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("(Quaternion not editable! You can only pass native parameter! Consider use Vector as euler angle or Rotator, and convert to Quaterion before calculation)")))
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(FColor(255, 0, 0, 255)))
				];
			}
			break;
			case LGUIDrawableEventParameterType::String:
			{
				ClearNumericValueBuffer(InDataContainerHandle);
				ClearObjectValue(InDataContainerHandle);
				ClearActorValue(InDataContainerHandle);
				ClearClassValue(InDataContainerHandle);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ReferenceString));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			case LGUIDrawableEventParameterType::PointerEvent:
			{
				ClearNumericValueBuffer(InDataContainerHandle);
				ClearReferenceValue(InDataContainerHandle);
				InDetailGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Parameter"))
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					SNew(STextBlock)
					.Text(FText::FromString(TEXT("(PointerEventData not editable! You can only pass native parameter!)")))
					.WrappingPolicy(ETextWrappingPolicy::AllowPerCharacterWrapping)
					.AutoWrapText(true)
					.ColorAndOpacity(FLinearColor(FColor(255, 0, 0, 255)))
				];
			}
			break;
			case LGUIDrawableEventParameterType::Object:
			case LGUIDrawableEventParameterType::Actor:
			case LGUIDrawableEventParameterType::Class:
			{
				InDetailGroup.AddWidgetRow()
				.NameContent()
				[
					SNew(STextBlock)
					.Text(FText::FromString("Parameter"))
				]
				.ValueContent()
				.MinDesiredWidth(500)
				[
					DrawFunctionReferenceParameter(InDataContainerHandle, functionParameterType, InFunction)
				];
			}
			break;
			case LGUIDrawableEventParameterType::Rotator:
			{
				ClearReferenceValue(InDataContainerHandle);
				SetBufferLength(paramBufferHandle, 12);
				auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, RotatorValue));
				valueHandle->SetValue(BitConverter::ToRotator(GetBuffer(paramBufferHandle, 12)));
				valueHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::RotatorValueChange, valueHandle, paramBufferHandle));
				valueHandle->SetOnChildPropertyValueChanged(FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventCustomization::RotatorValueChange, valueHandle, paramBufferHandle));
				InDetailGroup.AddPropertyRow(valueHandle.ToSharedRef()).DisplayName(FText::FromString("Parameter"));
			}
			break;
			default:
				break;
			}
		}
		else
		{
			ClearNumericValueBuffer(InDataContainerHandle);
			ClearReferenceValue(InDataContainerHandle);
			SNew(STextBlock)
				.Text(FText::FromString("(Not handled)"));
		}
	}
	//function's parameter editor
	TSharedRef<SWidget> DrawFunctionReferenceParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, LGUIDrawableEventParameterType FunctionParameterType, UFunction* InFunction)
	{
		auto paramBufferHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ParamBuffer));

		TSharedPtr<SWidget> ParameterContent;
		switch (FunctionParameterType)
		{
			//case LGUIDrawableEventParameterType::Name:
			//	break;
		case LGUIDrawableEventParameterType::Object:
		{
			ClearNumericValueBuffer(InDataContainerHandle);
			ClearStringValue(InDataContainerHandle);
			ClearActorValue(InDataContainerHandle);
			ClearClassValue(InDataContainerHandle);
			return SNew(SObjectPropertyEntryBox)
				.IsEnabled(true)
				.AllowedClass(ULGUIDrawableEventParameterHelper::GetObjectParameterClass(InFunction))
				.PropertyHandle(InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ReferenceObject)))
				.AllowClear(true)
				.ToolTipText(LOCTEXT("UObjectTips", "UObject only referece asset, dont use for actor"))
				.OnObjectChanged(this, &FLGUIDrawableEventCustomization::ObjectValueChange, paramBufferHandle, InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ReferenceObject)), true);
		}
		break;
		case LGUIDrawableEventParameterType::Actor:
		{
			ClearNumericValueBuffer(InDataContainerHandle);
			ClearClassValue(InDataContainerHandle);
			ClearStringValue(InDataContainerHandle);
			ClearObjectValue(InDataContainerHandle);
			return SNew(SObjectPropertyEntryBox)
				.IsEnabled(true)
				.AllowedClass(ULGUIDrawableEventParameterHelper::GetObjectParameterClass(InFunction))
				.PropertyHandle(InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ReferenceActor)))
				.AllowClear(true)
				.OnObjectChanged(this, &FLGUIDrawableEventCustomization::ObjectValueChange, paramBufferHandle, InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ReferenceActor)), false);
		}
		break;
		case LGUIDrawableEventParameterType::Class:
		{
			auto metaClass = ULGUIDrawableEventParameterHelper::GetClassParameterClass(InFunction);
			auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ReferenceClass));
			ClearNumericValueBuffer(InDataContainerHandle);
			ClearStringValue(InDataContainerHandle);
			ClearObjectValue(InDataContainerHandle);
			ClearActorValue(InDataContainerHandle);
			return SNew(SClassPropertyEntryBox)
				.IsEnabled(true)
				.AllowAbstract(true)
				.AllowNone(true)
				.MetaClass(metaClass)
				.SelectedClass(this, &FLGUIDrawableEventCustomization::GetClassValue, valueHandle)
				.OnSetClass(this, &FLGUIDrawableEventCustomization::ClassValueChange, valueHandle);
		}
		break;
		default:
			break;
		}
		return SNew(STextBlock)
			.Text(FText::FromString("(Not handled)"));
	}
	void BoolValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		bool Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void ClearArrayBuffer(TSharedPtr<IPropertyHandle> BufferHandle)
	{
		auto BufferArrayHandle = BufferHandle->AsArray();
		BufferArrayHandle->EmptyArray();//clear buffer
	}

	void ObjectValueChange(const FAssetData& InObj, TSharedPtr<IPropertyHandle> BufferHandle, TSharedPtr<IPropertyHandle> ObjectReferenceHandle, bool ObjectOrActor)
	{
		if (ObjectOrActor)
		{
			//ObjectReferenct is not for actor reference
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
	const UClass* GetClassValue(TSharedPtr<IPropertyHandle> ClassReferenceHandle)const
	{
		UObject* referenceClassObject = nullptr;
		ClassReferenceHandle->GetValue(referenceClassObject);
		return (UClass*)referenceClassObject;
	}
	void ClassValueChange(const UClass* InClass, TSharedPtr<IPropertyHandle> ClassReferenceHandle)
	{
		ClassReferenceHandle->SetValue(InClass);
	}
	void EnumValueChange(int32 InValue, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		uint8 Value = (uint8)InValue;
		ValueHandle->SetValue(Value);
		UInt8ValueChange(ValueHandle, BufferHandle);
	}
	void FloatValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		float Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void DoubleValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		double Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Int8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		int8 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void UInt8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		uint8 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Int16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		int16 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void UInt16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		uint16 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Int32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		int32 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void UInt32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		uint32 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Int64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		int64 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void UInt64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		uint64 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Vector2ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		FVector2D Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Vector3ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		FVector Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void Vector4ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		FVector4 Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void ColorValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		FColor Value;
		FString FormatedString;
		ValueHandle->GetValueAsFormattedString(FormatedString);
		Value.InitFromString(FormatedString);
		//format string use bgra, color use rgba, so switch it
		auto r = Value.R;
		auto b = Value.B;
		Value.R = b;
		Value.B = r;
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void LinearColorValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		FLinearColor Value;
		FString FormatedString;
		ValueHandle->GetValueAsFormattedString(FormatedString);
		Value.InitFromString(FormatedString);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void RotatorValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)
	{
		FRotator Value;
		ValueHandle->GetValue(Value);
		SetBufferValue(BufferHandle, BitConverter::GetBytes(Value));
	}
	void SetBufferValue(TSharedPtr<IPropertyHandle> BufferHandle, TArray<uint8> BufferArray)
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
			}
			PropertyUtilites->ForceRefresh();
		}
		else
		{
			FString printString = TEXT("SetBufferValue");
			for (int i = 0; i < bufferCount; i++)
			{
				auto bufferHandle = BufferArrayHandle->GetElement(i);
				auto buffer = BufferArray[i];
				printString.AppendInt(buffer);
				bufferHandle->SetValue(buffer);
			}
		}
	}

	void SetBufferLength(TSharedPtr<IPropertyHandle> BufferHandle, int32 Count)
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

	TArray<uint8> GetBuffer(TSharedPtr<IPropertyHandle> BufferHandle, int32 Count)
	{
		auto BufferArrayHandle = BufferHandle->AsArray();
		TArray<uint8> resultBuffer;
		resultBuffer.Reserve(Count);
		uint32 bufferHandleCount;
		BufferArrayHandle->GetNumElements(bufferHandleCount);
		if (Count != bufferHandleCount)
		{
			resultBuffer.Init(0, Count);
		}
		else
		{
			for (int i = 0; i < Count; i++)
			{
				auto elementHandle = BufferArrayHandle->GetElement(i);
				uint8 value;
				elementHandle->GetValue(value);
				resultBuffer.Add(value);
			}
		}
		return resultBuffer;
	}

	TArray<uint8> GetPropertyBuffer(TSharedPtr<IPropertyHandle> BufferHandle) const
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
	int32 GetEnumValue(TSharedPtr<IPropertyHandle> ValueHandle)const
	{
		uint8 Value = 0;
		ValueHandle->GetValue(Value);
		return Value;
	}

	void ClearNumericValueBuffer(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		auto handle = PropertyHandle->GetChildHandle("ParamBuffer")->AsArray();
		handle->EmptyArray();
	}
	void ClearReferenceValue(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		ClearObjectValue(PropertyHandle);
		ClearActorValue(PropertyHandle);
		ClearClassValue(PropertyHandle);
		ClearStringValue(PropertyHandle);
	}
	void ClearStringValue(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		auto handle = PropertyHandle->GetChildHandle("ReferenceString");
		handle->SetValue(FString(""));
	}
	void ClearObjectValue(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		auto handle = PropertyHandle->GetChildHandle("ReferenceObject");
		UObject* EmptyObj = nullptr;
		handle->SetValue(EmptyObj);
	}
	void ClearActorValue(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		auto handle = PropertyHandle->GetChildHandle("ReferenceActor");
		UObject* EmptyObj = nullptr;
		handle->SetValue(EmptyObj);
	}
	void ClearClassValue(TSharedPtr<IPropertyHandle> PropertyHandle)
	{
		auto handle = PropertyHandle->GetChildHandle("ReferenceClass");
		UObject* EmptyObj = nullptr;
		handle->SetValue(EmptyObj);
	}

	void OnParameterTypeChange(TSharedRef<IPropertyHandle> InDataContainerHandle)
	{
		auto valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, BoolValue)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, FloatValue)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, DoubleValue)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int8Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt8Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int16Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt16Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int32Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt32Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Int64Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, UInt64Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Vector2Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Vector3Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, Vector4Value)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, QuatValue)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, ColorValue)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, LinearColorValue)); valueHandle->ResetToDefault();
		valueHandle = InDataContainerHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(EventData, RotatorValue)); valueHandle->ResetToDefault();
	}
};

#undef LOCTEXT_NAMESPACE