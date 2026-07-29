// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerClickInterface.h"
#include "UISelectable.h"
#include "Event/LexUIEventDelegate.h"
#include "UIButton.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FUIButtonClickedEvent);

UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIButton : public UUISelectable, public ILexPointerClickInterface
{
	GENERATED_BODY()
protected:

	UPROPERTY(EditAnywhere, Category = "LGUI-Button", DisplayName="OnClick")
	FLexUIEventDelegate OnClickED = FLexUIEventDelegate(ELexUIEventDelegateParameterType::Empty);
	FSimpleMulticastDelegate OnClickCPP;
	UPROPERTY(BlueprintAssignable, Category = "LGUI-Toggle")
	FUIButtonClickedEvent OnClick;
	virtual bool OnPointerClick_Implementation(ULexPointerEventData* EventData)override;
public:
	FSimpleMulticastDelegate& GetOnClickEvent(){return OnClickCPP;}
};
