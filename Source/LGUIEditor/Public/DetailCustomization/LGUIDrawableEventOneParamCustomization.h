// Copyright 2019 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "Event/LGUIDrawableEvent.h"
#include "LGUIDrawableEventCustomization.h"

#pragma once

#define LOCTEXT_NAMESPACE "LGUIDrawableEventOneParamCustomization"

/**
 * 
 */
class FLGUIDrawableEventOneParamCustomization : public FLGUIDrawableEventCustomization<FLGUIDrawableEvent, FLGUIDrawableEventData>
{
private:
	FLGUIDrawableEventOneParamCustomization();
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
protected:
	virtual TArray<LGUIDrawableEventParameterType> GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)override;
	virtual void AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailGroup& DetailGroup)override;
	virtual TArray<LGUIDrawableEventParameterType> GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)override;
	virtual void SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIDrawableEventParameterType> ParameterTypeArray)override;
	virtual TSharedRef<IPropertyHandle> GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex)override { return TSharedRef<IPropertyHandle>((IPropertyHandle*)0); };
};

#undef LOCTEXT_NAMESPACE


#if 0
#define LOCTEXT_NAMESPACE "LGUIDrawableEventTwoParamCustomization"

/**
 *
 */
class FLGUIDrawableEventTwoParamCustomization : public FLGUIDrawableEventCustomization<FLGUIDrawableEvent, FLGUIDrawableEventData>
{
private:
	FLGUIDrawableEventTwoParamCustomization();
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
protected:
	virtual TArray<LGUIDrawableEventParameterType> GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)override;
	virtual void AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailGroup& DetailGroup)override;
	virtual TArray<LGUIDrawableEventParameterType> GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)override;
	virtual void SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIDrawableEventParameterType> ParameterTypeArray)override;
	virtual TSharedRef<IPropertyHandle> GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex)override;
};

#undef LOCTEXT_NAMESPACE
#endif