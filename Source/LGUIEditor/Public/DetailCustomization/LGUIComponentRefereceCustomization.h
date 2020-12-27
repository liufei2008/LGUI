// Copyright 2019-2020 LexLiu. All Rights Reserved.
#include "IDetailCustomization.h"
#include "PropertyCustomizationHelpers.h"
#pragma once


/**
 * 
 */
class FLGUIComponentRefereceCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TSharedPtr<IPropertyUtilities> PropertyUtilites;
	TSharedPtr<IPropertyHandle> ComponentNameProperty;
	TSharedRef<SWidget> OnGetMenu(TArray<UActorComponent*> Components);
	void OnSelectComponent(FName CompName);
	FText GetButtonText(TArray<UActorComponent*> Components)const;
	void OnCopy(AActor* actor, UClass* compClass, FName compName);
	void OnPaste(TSharedPtr<IPropertyHandle> actorProperty, TSharedPtr<IPropertyHandle> compClassProperty, TSharedPtr<IPropertyHandle> compNameProperty);

	static TWeakObjectPtr<AActor> CopiedTargetActor;
	static FName CopiedTargetComponentName;
};
