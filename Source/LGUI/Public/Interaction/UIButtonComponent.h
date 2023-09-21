// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LGUIPointerClickInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIEventDelegate.h"
#include "Event/LGUIDelegateDeclaration.h"
#include "Event/LGUIDelegateHandleWrapper.h"
#include "UIButtonComponent.generated.h"

DECLARE_DYNAMIC_DELEGATE(FLGUIButtonDynamicDelegate);

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIButtonComponent : public UUISelectableComponent, public ILGUIPointerClickInterface
{
	GENERATED_BODY()
protected:

	UPROPERTY(EditAnywhere, Category = "LGUI-Button")
		FLGUIEventDelegate OnClick = FLGUIEventDelegate(ELGUIEventDelegateParameterType::Empty);
	FSimpleMulticastDelegate OnClickCPP;
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData)override;
public:
	/** Register click event */
	FDelegateHandle RegisterClickEvent(const FSimpleDelegate& InDelegate);
	/** Register click event */
	FDelegateHandle RegisterClickEvent(const TFunction<void()>& InFunction);
	/** Unregister click event */
	void UnregisterClickEvent(const FDelegateHandle& InHandle);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Button")
		FLGUIDelegateHandleWrapper RegisterClickEvent(const FLGUIButtonDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Button")
		void UnregisterClickEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};
