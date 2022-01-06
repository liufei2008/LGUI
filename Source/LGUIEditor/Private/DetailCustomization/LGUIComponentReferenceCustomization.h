// Copyright 2019-2022 LexLiu. All Rights Reserved.
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
class FLGUIComponentReferenceCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override {};
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	TSharedPtr<IPropertyUtilities> PropertyUtilites;
	TSharedRef<SWidget> OnGetMenu(TSharedPtr<IPropertyHandle> CompProperty, TArray<UActorComponent*> Components);
	void OnSelectComponent(TSharedPtr<IPropertyHandle> CompProperty, UActorComponent* Comp);
	FText GetButtonText(TSharedPtr<IPropertyHandle> TargetCompHandle, TArray<UActorComponent*> Components)const;
	void OnCopy(AActor* TargetActor, UActorComponent* TargetComp, UClass* TargetClass);
	void OnPaste(TSharedPtr<IPropertyHandle> TargetActorProperty, TSharedPtr<IPropertyHandle> TargetCompProperty, TSharedPtr<IPropertyHandle> TargetClassProperty);
	void OnHelperActorValueChange(TSharedPtr<IPropertyHandle> HelperActorHandle, TSharedPtr<IPropertyHandle> HelperCompHandle, TSharedPtr<IPropertyHandle> HelperClassHandle, TSharedPtr<IPropertyHandle> HelperComponentNameHandle);
	FReply OnClickFixComponentReference(TSharedPtr<IPropertyHandle> HelperCompHandle, UActorComponent* Target);

	static TWeakObjectPtr<AActor> CopiedHelperActor;
	static TWeakObjectPtr<UActorComponent> CopiedTargetComp;
	static UClass* CopiedHelperClass;
};
