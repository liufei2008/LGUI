// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIEventDelegateOneParamCustomization.h"

FLGUIEventDelegateOneParamCustomization::FLGUIEventDelegateOneParamCustomization():FLGUIEventDelegateCustomization(1, true)
{

}
FLGUIEventDelegateOneParamCustomization::FLGUIEventDelegateOneParamCustomization(bool InCanChangeParameterType) : FLGUIEventDelegateCustomization(1, InCanChangeParameterType)
{

}
TSharedRef<IPropertyTypeCustomization> FLGUIEventDelegateOneParamCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIEventDelegateOneParamCustomization());
}
TArray<LGUIEventDelegateParameterType> FLGUIEventDelegateOneParamCustomization::GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)
{
	auto supportedParameterHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegate, supportParameterType));
	supportedParameterHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	uint8 supportParameterTypeUint8;
	supportedParameterHandle->GetValue(supportParameterTypeUint8);
	LGUIEventDelegateParameterType eventParameterType = (LGUIEventDelegateParameterType)supportParameterTypeUint8;
	return { eventParameterType };
}
void FLGUIEventDelegateOneParamCustomization::AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder)
{
	auto supportedParameterHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegate, supportParameterType));
	ChildBuilder.AddProperty(supportedParameterHandle.ToSharedRef());
}
TArray<LGUIEventDelegateParameterType> FLGUIEventDelegateOneParamCustomization::GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)
{
	auto paramTypeHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamType));
	paramTypeHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateOneParamCustomization::OnParameterTypeChange, EventDataItemHandle));
	uint8 functionParameterTypeUint8;
	paramTypeHandle->GetValue(functionParameterTypeUint8);
	LGUIEventDelegateParameterType functionParameterType = (LGUIEventDelegateParameterType)functionParameterTypeUint8;
	return { functionParameterType };
}
void FLGUIEventDelegateOneParamCustomization::SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIEventDelegateParameterType> ParameterTypeArray)
{
	auto paramTypeHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, ParamType));
	if (ParameterTypeArray.Num() == 0)
	{
		paramTypeHandle->SetValue((uint8)(LGUIEventDelegateParameterType::Empty));
	}
	else
	{
		paramTypeHandle->SetValue((uint8)(ParameterTypeArray[0]));
	}
}



#if 0
FLGUIEventDelegateTwoParamCustomization::FLGUIEventDelegateTwoParamCustomization() :FLGUIEventDelegateCustomization(2)
{

}
TSharedRef<IPropertyTypeCustomization> FLGUIEventDelegateTwoParamCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIEventDelegateTwoParamCustomization());
}
TArray<LGUIEventDelegateParameterType> FLGUIEventDelegateTwoParamCustomization::GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)
{
	auto supportedParameter1Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateTwoParam, supportParameterType1));
	auto supportedParameter2Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateTwoParam, supportParameterType2));
	supportedParameter1Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	supportedParameter2Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	uint8 supportParameterType1Uint8;
	uint8 supportParameterType2Uint8;
	supportedParameter1Handle->GetValue(supportParameterType1Uint8);
	supportedParameter2Handle->GetValue(supportParameterType2Uint8);
	return { (LGUIEventDelegateParameterType)supportParameterType1Uint8, (LGUIEventDelegateParameterType)supportParameterType2Uint8 };
}
void FLGUIEventDelegateTwoParamCustomization::AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailGroup& DetailGroup)
{
	auto supportedParameter1Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateTwoParam, supportParameterType1));
	auto supportedParameter2Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateTwoParam, supportParameterType2));
	DetailGroup.AddPropertyRow(supportedParameter1Handle.ToSharedRef());
	DetailGroup.AddPropertyRow(supportedParameter2Handle.ToSharedRef());
}
TArray<LGUIEventDelegateParameterType> FLGUIEventDelegateTwoParamCustomization::GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)
{
	auto paramType1Handle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateDataTwoParam, ParamType));
	auto eventDataParam2ItemHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateDataTwoParam, param2DataContainer));
	auto paramType2Handle = eventDataParam2ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData_DataContainer, ParamType));
	paramType1Handle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateTwoParamCustomization::OnParameterTypeChange, EventDataItemHandle));
	paramType2Handle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIEventDelegateTwoParamCustomization::OnParameterTypeChange, eventDataParam2ItemHandle.ToSharedRef()));
	uint8 functionParameterType1Uint8;
	uint8 functionParameterType2Uint8;
	paramType1Handle->GetValue(functionParameterType1Uint8);
	paramType2Handle->GetValue(functionParameterType2Uint8);
	return { (LGUIEventDelegateParameterType)functionParameterType1Uint8, (LGUIEventDelegateParameterType)functionParameterType2Uint8 };
}
void FLGUIEventDelegateTwoParamCustomization::SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIEventDelegateParameterType> ParameterTypeArray)
{
	auto paramType1Handle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateDataTwoParam, ParamType));
	auto eventDataParam2ItemHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateDataTwoParam, param2DataContainer));
	auto paramType2Handle = eventDataParam2ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData_DataContainer, ParamType));
	if (ParameterTypeArray.Num() == 0)
	{
		paramType1Handle->SetValue((uint8)(LGUIEventDelegateParameterType::Empty));
		paramType2Handle->SetValue((uint8)(LGUIEventDelegateParameterType::Empty));
	}
	else
	{
		paramType1Handle->SetValue((uint8)(ParameterTypeArray[0]));
		paramType2Handle->SetValue((uint8)(ParameterTypeArray[1]));
	}
}
TSharedRef<IPropertyHandle> FLGUIEventDelegateTwoParamCustomization::GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex)
{
	return EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateDataTwoParam, param2DataContainer)).ToSharedRef();
}
#endif