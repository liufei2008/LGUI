// Copyright 2019-2020 LexLiu. All Rights Reserved.

#include "DetailCustomization/LGUIDrawableEventOneParamCustomization.h"

FLGUIDrawableEventOneParamCustomization::FLGUIDrawableEventOneParamCustomization():FLGUIDrawableEventCustomization(1)
{

}
TSharedRef<IPropertyTypeCustomization> FLGUIDrawableEventOneParamCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIDrawableEventOneParamCustomization());
}
TArray<LGUIDrawableEventParameterType> FLGUIDrawableEventOneParamCustomization::GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)
{
	auto supportedParameterHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEvent, supportParameterType));
	supportedParameterHandle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	uint8 supportParameterTypeUint8;
	supportedParameterHandle->GetValue(supportParameterTypeUint8);
	LGUIDrawableEventParameterType eventParameterType = (LGUIDrawableEventParameterType)supportParameterTypeUint8;
	return { eventParameterType };
}
void FLGUIDrawableEventOneParamCustomization::AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder)
{
	bool isInWorld = false;
	FCommentNodeSet nodeSet;
	PropertyHandle->GetOuterObjects(nodeSet);
	for (UObject* obj : nodeSet)
	{
		isInWorld = obj->GetWorld() != nullptr;
		break;
	}
	if (!isInWorld)
	{
		auto supportedParameterHandle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEvent, supportParameterType));
		ChildBuilder.AddProperty(supportedParameterHandle.ToSharedRef());
	}
}
TArray<LGUIDrawableEventParameterType> FLGUIDrawableEventOneParamCustomization::GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)
{
	auto paramTypeHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventData, ParamType));
	paramTypeHandle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventOneParamCustomization::OnParameterTypeChange, EventDataItemHandle));
	uint8 functionParameterTypeUint8;
	paramTypeHandle->GetValue(functionParameterTypeUint8);
	LGUIDrawableEventParameterType functionParameterType = (LGUIDrawableEventParameterType)functionParameterTypeUint8;
	return { functionParameterType };
}
void FLGUIDrawableEventOneParamCustomization::SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIDrawableEventParameterType> ParameterTypeArray)
{
	auto paramTypeHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventData, ParamType));
	if (ParameterTypeArray.Num() == 0)
	{
		paramTypeHandle->SetValue((uint8)(LGUIDrawableEventParameterType::Empty));
	}
	else
	{
		paramTypeHandle->SetValue((uint8)(ParameterTypeArray[0]));
	}
}



#if 0
FLGUIDrawableEventTwoParamCustomization::FLGUIDrawableEventTwoParamCustomization() :FLGUIDrawableEventCustomization(2)
{

}
TSharedRef<IPropertyTypeCustomization> FLGUIDrawableEventTwoParamCustomization::MakeInstance()
{
	return MakeShareable(new FLGUIDrawableEventTwoParamCustomization());
}
TArray<LGUIDrawableEventParameterType> FLGUIDrawableEventTwoParamCustomization::GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)
{
	auto supportedParameter1Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventTwoParam, supportParameterType1));
	auto supportedParameter2Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventTwoParam, supportParameterType2));
	supportedParameter1Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	supportedParameter2Handle->SetOnPropertyValueChanged(FSimpleDelegate::CreateLambda([=] { PropertyUtilites->ForceRefresh(); }));
	uint8 supportParameterType1Uint8;
	uint8 supportParameterType2Uint8;
	supportedParameter1Handle->GetValue(supportParameterType1Uint8);
	supportedParameter2Handle->GetValue(supportParameterType2Uint8);
	return { (LGUIDrawableEventParameterType)supportParameterType1Uint8, (LGUIDrawableEventParameterType)supportParameterType2Uint8 };
}
void FLGUIDrawableEventTwoParamCustomization::AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailGroup& DetailGroup)
{
	auto supportedParameter1Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventTwoParam, supportParameterType1));
	auto supportedParameter2Handle = PropertyHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventTwoParam, supportParameterType2));
	DetailGroup.AddPropertyRow(supportedParameter1Handle.ToSharedRef());
	DetailGroup.AddPropertyRow(supportedParameter2Handle.ToSharedRef());
}
TArray<LGUIDrawableEventParameterType> FLGUIDrawableEventTwoParamCustomization::GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)
{
	auto paramType1Handle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventDataTwoParam, ParamType));
	auto eventDataParam2ItemHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventDataTwoParam, param2DataContainer));
	auto paramType2Handle = eventDataParam2ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventData_DataContainer, ParamType));
	paramType1Handle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventTwoParamCustomization::OnParameterTypeChange, EventDataItemHandle));
	paramType2Handle->SetOnPropertyValueChanged(
		FSimpleDelegate::CreateSP(this, &FLGUIDrawableEventTwoParamCustomization::OnParameterTypeChange, eventDataParam2ItemHandle.ToSharedRef()));
	uint8 functionParameterType1Uint8;
	uint8 functionParameterType2Uint8;
	paramType1Handle->GetValue(functionParameterType1Uint8);
	paramType2Handle->GetValue(functionParameterType2Uint8);
	return { (LGUIDrawableEventParameterType)functionParameterType1Uint8, (LGUIDrawableEventParameterType)functionParameterType2Uint8 };
}
void FLGUIDrawableEventTwoParamCustomization::SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIDrawableEventParameterType> ParameterTypeArray)
{
	auto paramType1Handle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventDataTwoParam, ParamType));
	auto eventDataParam2ItemHandle = EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventDataTwoParam, param2DataContainer));
	auto paramType2Handle = eventDataParam2ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventData_DataContainer, ParamType));
	if (ParameterTypeArray.Num() == 0)
	{
		paramType1Handle->SetValue((uint8)(LGUIDrawableEventParameterType::Empty));
		paramType2Handle->SetValue((uint8)(LGUIDrawableEventParameterType::Empty));
	}
	else
	{
		paramType1Handle->SetValue((uint8)(ParameterTypeArray[0]));
		paramType2Handle->SetValue((uint8)(ParameterTypeArray[1]));
	}
}
TSharedRef<IPropertyHandle> FLGUIDrawableEventTwoParamCustomization::GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex)
{
	return EventDataItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIDrawableEventDataTwoParam, param2DataContainer)).ToSharedRef();
}
#endif