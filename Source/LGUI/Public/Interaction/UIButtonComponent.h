// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Event/LGUIPointerClickInterface.h"
#include "UISelectableComponent.h"
#include "Event/LGUIDrawableEvent.h"
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
		FLGUIDrawableEvent OnClick = FLGUIDrawableEvent(LGUIDrawableEventParameterType::Empty);
	FSimpleMulticastDelegate OnClickCPP;
	virtual bool OnPointerClick_Implementation(ULGUIPointerEventData* eventData)override;
public:
	//Register click event
	void RegisterClickEvent(const FSimpleDelegate& InDelegate);
	//Unregister click event
	void UnregisterClickEvent(const FSimpleDelegate& InDelegate);

	UFUNCTION(BlueprintCallable, Category = "LGUI-Button")
		FLGUIDelegateHandleWrapper RegisterClickEvent(const FLGUIButtonDynamicDelegate& InDelegate);
	UFUNCTION(BlueprintCallable, Category = "LGUI-Button")
		void UnregisterClickEvent(const FLGUIDelegateHandleWrapper& InDelegateHandle);
};
