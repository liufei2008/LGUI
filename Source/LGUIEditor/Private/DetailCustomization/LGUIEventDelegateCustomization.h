// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#pragma once


/**
 * 
 */
class FLGUIEventDelegateCustomization : public IPropertyTypeCustomization
{
protected:
	TSharedPtr<IPropertyUtilities> PropertyUtilites;
	TSharedPtr<IPropertyHandleArray> EventListHandle;
	static TArray<FString> CopySourceData;
	LGUIEventDelegateParameterType EventParameterType;
	TSharedPtr<SWidget> ColorPickerParentWidget;
	TArray<TSharedRef<SWidget>> EventParameterWidgetArray;
private:
	bool CanChangeParameterType = true;
	bool IsParameterTypeValid(LGUIEventDelegateParameterType InParamType)
	{
		return InParamType != LGUIEventDelegateParameterType::None;
	}
public:
	FLGUIEventDelegateCustomization(bool InCanChangeParameterType)
	{
		CanChangeParameterType = InCanChangeParameterType;
	}
	~FLGUIEventDelegateCustomization()
	{
		
	}
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FLGUIEventDelegateCustomization(true));
	}
	/** IDetailCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};

	LGUIEventDelegateParameterType GetNativeParameterType(TSharedRef<IPropertyHandle> PropertyHandle);
	void AddNativeParameterTypeProperty(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder);
	LGUIEventDelegateParameterType GetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle);

	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)override;

	void SetEventDataParameterType(TSharedRef<IPropertyHandle> EventDataItemHandle, LGUIEventDelegateParameterType ParameterType);
private:
	FOptionalSize GetEventItemHeight(int itemIndex)const
	{
		return EventParameterWidgetArray[itemIndex]->GetCachedGeometry().Size.Y + 60;//60 is other's size
	}
	FOptionalSize GetEventTotalHeight()const
	{
		float result = EventParameterWidgetArray.Num() > 0 ? 32 : 40;//32 & 40 is the header and tail size
		for (int i = 0; i < EventParameterWidgetArray.Num(); i++)
		{
			result += GetEventItemHeight(i).Get();
		}
		return result;
	}
	void OnSelectComponent(UActorComponent* Comp, int32 itemIndex);
	void OnSelectActorSelf(int32 itemIndex);
	void OnSelectFunction(FName FuncName, int32 itemIndex, LGUIEventDelegateParameterType ParamType, bool UseNativeParameter);
	bool IsComponentSelectorMenuEnabled(int32 itemIndex)const
	{
		auto ItemHandle = EventListHandle->GetElement(itemIndex);
		auto HelperActorHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperActor));
		UObject* HelperActorObject = nullptr;
		HelperActorHandle->GetValue(HelperActorObject);
		return IsValid(HelperActorObject);
	}
	bool IsFunctionSelectorMenuEnabled(int32 itemIndex)const
	{
		auto ItemHandle = EventListHandle->GetElement(itemIndex);
		auto HelperActorHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, HelperActor));
		UObject* HelperActorObject = nullptr;
		HelperActorHandle->GetValue(HelperActorObject);

		auto TargetObjectHandle = ItemHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIEventDelegateData, TargetObject));
		UObject* TargetObject = nullptr;
		TargetObjectHandle->GetValue(TargetObject);

		return IsValid(HelperActorObject) && IsValid(TargetObject);
	}
	TSharedRef<SWidget> MakeComponentSelectorMenu(int32 itemIndex);
	TSharedRef<SWidget> MakeFunctionSelectorMenu(int32 itemIndex);
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
			PropertyUtilites->ForceRefresh();
		}
		return FReply::Handled();
	}

	TSharedRef<SWidget> DrawFunctionParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, LGUIEventDelegateParameterType InFunctionParameterType, UFunction* InFunction);
	//function's parameter editor
	TSharedRef<SWidget> DrawFunctionReferenceParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, LGUIEventDelegateParameterType FunctionParameterType, UFunction* InFunction);

	void ObjectValueChange(const FAssetData& InObj, TSharedPtr<IPropertyHandle> BufferHandle, TSharedPtr<IPropertyHandle> ObjectReferenceHandle, bool ObjectOrActor);
	const UClass* GetClassValue(TSharedPtr<IPropertyHandle> ClassReferenceHandle)const;
	void ClassValueChange(const UClass* InClass, TSharedPtr<IPropertyHandle> ClassReferenceHandle);
	void EnumValueChange(int32 InValue, ESelectInfo::Type SelectionType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void BoolValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void FloatValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void DoubleValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void Int8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void UInt8ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void Int16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void UInt16ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void Int32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void UInt32ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void Int64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void UInt64ValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void StringValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void NameValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void TextValueChange(TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void Vector2ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	TOptional<float> Vector2GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const;
	void Vector3ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	TOptional<float> Vector3GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const;
	void Vector4ItemValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	TOptional<float> Vector4GetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const;
	FLinearColor LinearColorGetValue(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const;
	void LinearColorValueChange(FLinearColor NewValue, bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	FReply OnMouseButtonDownColorBlock(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	TOptional<float> RotatorGetItemValue(int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle)const;
	void RotatorValueChange(float NewValue, ETextCommit::Type CommitInfo, int AxisType, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
	void SetBufferValue(TSharedPtr<IPropertyHandle> BufferHandle, const TArray<uint8>& BufferArray);
	void SetBufferLength(TSharedPtr<IPropertyHandle> BufferHandle, int32 Count);
	TArray<uint8> GetBuffer(TSharedPtr<IPropertyHandle> BufferHandle);
	TArray<uint8> GetPropertyBuffer(TSharedPtr<IPropertyHandle> BufferHandle) const;
	int32 GetEnumValue(TSharedPtr<IPropertyHandle> ValueHandle)const;
	FText GetTextValue(TSharedPtr<IPropertyHandle> ValueHandle)const;
	void SetTextValue(const FText& InText, ETextCommit::Type InCommitType, TSharedPtr<IPropertyHandle> ValueHandle);
	void ClearValueBuffer(TSharedPtr<IPropertyHandle> PropertyHandle);
	void ClearReferenceValue(TSharedPtr<IPropertyHandle> PropertyHandle);
	void ClearObjectValue(TSharedPtr<IPropertyHandle> PropertyHandle);
	void OnParameterTypeChange(TSharedRef<IPropertyHandle> InDataContainerHandle);
	void CreateColorPicker(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);
};

