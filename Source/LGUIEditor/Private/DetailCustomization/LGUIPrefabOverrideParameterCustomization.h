// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PrefabSystem/LGUIPrefabOverrideParameter.h"
#pragma once


/**
 * 
 */
class FLGUIPrefabOverrideParameterCustomization : public IPropertyTypeCustomization
{
protected:
	TSharedPtr<IPropertyUtilities> PropertyUtilites;
	TSharedPtr<IPropertyHandleArray> DataListHandle;
	TSharedPtr<SWidget> ColorPickerParentWidget;
	static TArray<FString> CopySourceData;
	static bool bShowRawData;
private:
	bool bIsTemplate = true;
	bool bIsAutomaticParameter = false;
public:
	FLGUIPrefabOverrideParameterCustomization()
	{
		
	}
	~FLGUIPrefabOverrideParameterCustomization()
	{
		
	}
	static TSharedRef<IPropertyTypeCustomization> MakeInstance()
	{
		return MakeShareable(new FLGUIPrefabOverrideParameterCustomization());
	}
	/** IDetailCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	ELGUIPrefabOverrideParameterType GetPropertyParameterType(TSharedRef<IPropertyHandle> DataHandle);
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils)override;
private:
	void OnSelectComponent(UActorComponent* Comp, TSharedRef<IPropertyHandle> DataHandle);
	void OnSelectActorSelf(TSharedRef<IPropertyHandle> DataHandle);
	void OnSelectProperty(FName FuncName, ELGUIPrefabOverrideParameterType ParamType, bool UseNativeParameter, TSharedRef<IPropertyHandle> DataHandle);
	bool IsComponentSelectorMenuEnabled(TSharedRef<IPropertyHandle> DataHandle)const
	{
		auto HelperActorHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, HelperActor));
		UObject* HelperActorObject = nullptr;
		HelperActorHandle->GetValue(HelperActorObject);
		return IsValid(HelperActorObject);
	}
	bool IsPropertySelectorMenuEnabled(TSharedRef<IPropertyHandle> DataHandle)const
	{
		auto HelperActorHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, HelperActor));
		UObject* HelperActorObject = nullptr;
		HelperActorHandle->GetValue(HelperActorObject);

		auto TargetObjectHandle = DataHandle->GetChildHandle(GET_MEMBER_NAME_CHECKED(FLGUIPrefabOverrideParameterData, TargetObject));
		UObject* TargetObject = nullptr;
		TargetObjectHandle->GetValue(TargetObject);

		return IsValid(HelperActorObject) && IsValid(TargetObject);
	}
	TSharedRef<SWidget> MakeComponentSelectorMenu(TSharedRef<IPropertyHandle> DataHandle);
	TSharedRef<SWidget> MakePropertySelectorMenu(TSharedRef<IPropertyHandle> DataHandle);

	TSharedRef<SWidget> DrawPropertyParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, ELGUIPrefabOverrideParameterType InPropertyParameterType, FProperty* InProperty);
	//property's parameter editor
	TSharedRef<SWidget> DrawPropertyReferenceParameter(TSharedRef<IPropertyHandle> InDataContainerHandle, ELGUIPrefabOverrideParameterType PropertyParameterType, FProperty* InProperty);

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
	void ClearValueBuffer(TSharedPtr<IPropertyHandle> PropertyHandle);
	void ClearReferenceValue(TSharedPtr<IPropertyHandle> PropertyHandle);
	void ClearObjectValue(TSharedPtr<IPropertyHandle> PropertyHandle);
	void OnParameterTypeChange(TSharedRef<IPropertyHandle> InDataContainerHandle);
	void CreateColorPicker(bool bIsLinearColor, TSharedPtr<IPropertyHandle> ValueHandle, TSharedPtr<IPropertyHandle> BufferHandle);


	FReply OnClickAddRemove(bool AddOrRemove, TSharedRef<IPropertyHandle> DataHandle);
	FReply OnClickCopyPaste(bool CopyOrPaste, TSharedRef<IPropertyHandle> DataHandle);
	void OnClickListAdd();
	void OnClickListEmpty();
};

