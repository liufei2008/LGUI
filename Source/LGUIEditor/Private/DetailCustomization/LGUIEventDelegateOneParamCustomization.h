// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "Event/LGUIEventDelegate.h"
#include "LGUIEventDelegateCustomization.h"

#pragma once

#define LOCTEXT_NAMESPACE "LGUIEventDelegateOneParamCustomization"

/**
 * 
 */
class FLGUIEventDelegateOneParamCustomization : public FLGUIEventDelegateCustomization<FLGUIEventDelegate, FLGUIEventDelegateData>
{
private:
	FLGUIEventDelegateOneParamCustomization();
public:
	FLGUIEventDelegateOneParamCustomization(bool InCanChangeParameterType);
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
protected:
	virtual TArray<LGUIEventDelegateParameterType> GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)override;
	virtual void AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder)override;
	virtual TArray<LGUIEventDelegateParameterType> GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)override;
	virtual void SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIEventDelegateParameterType> ParameterTypeArray)override;
	virtual TSharedRef<IPropertyHandle> GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex)override { return TSharedRef<IPropertyHandle>((IPropertyHandle*)0); };
};

#undef LOCTEXT_NAMESPACE


#if 0
#define LOCTEXT_NAMESPACE "LGUIEventDelegateTwoParamCustomization"

/**
 *
 */
class FLGUIEventDelegateTwoParamCustomization : public FLGUIEventDelegateCustomization<FLGUIEventDelegate, FLGUIEventDelegateData>
{
private:
	FLGUIEventDelegateTwoParamCustomization();
public:
	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
protected:
	virtual TArray<LGUIEventDelegateParameterType> GetNativeParameterTypeArray(TSharedRef<IPropertyHandle> PropertyHandle)override;
	virtual void AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder)override;
	virtual TArray<LGUIEventDelegateParameterType> GetEventDataParameterTypeArray(TSharedRef<IPropertyHandle> EventDataItemHandle)override;
	virtual void SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, TArray<LGUIEventDelegateParameterType> ParameterTypeArray)override;
	virtual TSharedRef<IPropertyHandle> GetAdditionalParameterContainer(TSharedPtr<IPropertyHandle> EventDataItemHandle, int32 InAdditionalIndex)override;
};

#undef LOCTEXT_NAMESPACE
#endif