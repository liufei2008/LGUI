// Copyright 2019-Present LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IPropertyTypeCustomization.h"
#include "PropertyCustomizationHelpers.h"
#include "IPropertyUtilities.h"
#include "Widgets/SWidget.h"
#pragma once

class UActorComponent;
class AActor;
struct FLGUIComponentReference;

/**
 * 
 */
class FLGUIComponentReferenceCustomization : public IPropertyTypeCustomization
{
public:

	static TSharedRef<IPropertyTypeCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeHeader(TSharedRef<IPropertyHandle> PropertyHandle, FDetailWidgetRow& HeaderRow, IPropertyTypeCustomizationUtils& CustomizationUtils) override;
	virtual void CustomizeChildren(TSharedRef<IPropertyHandle> PropertyHandle, IDetailChildrenBuilder& ChildBuilder, IPropertyTypeCustomizationUtils& CustomizationUtils) override;

private:
	void BuildClassFilters();

	TSharedPtr<IPropertyUtilities> PropertyUtilites;
	TSharedRef<SWidget> OnGetMenu(TSharedPtr<IPropertyHandle> CompProperty, TSharedPtr<IPropertyHandle> CompNameProperty, TArray<UActorComponent*> Components);
	void OnSelectComponent(TSharedPtr<IPropertyHandle> CompProperty, TSharedPtr<IPropertyHandle> CompNameProperty, UActorComponent* Comp);
	FText GetButtonText(TSharedPtr<IPropertyHandle> TargetCompHandle, TArray<UActorComponent*> Components)const;
	void OnCopy();
	void OnPaste();
	void OnHelperActorValueChange();
	void RegenerateContentWidget();
	void OnResetToDefaultClicked();

	TSharedPtr<IPropertyHandle> PropertyHandle;
	/** Classes that can be used with this property */
	TArray<const UClass*> AllowedComponentClassFilters;
	/** Classes that can NOT be used with this property */
	TArray<const UClass*> DisallowedComponentClassFilters;
	TSharedPtr<SBox> ContentWidgetBox;
	bool bIsInWorld = false;
	TArray<FLGUIComponentReference*> ComponentReferenceInstances;
	static TWeakObjectPtr<AActor> CopiedHelperActor;
	static TWeakObjectPtr<UActorComponent> CopiedTargetComp;
	static UClass* CopiedHelperClass;
};
