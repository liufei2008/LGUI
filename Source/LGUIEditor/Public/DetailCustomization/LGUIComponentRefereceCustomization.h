// Copyright 2019-2021 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "IPropertyUtilities.h"
#include "Widgets/SWidget.h"
#pragma once

class UActorComponent;
class AActor;

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
