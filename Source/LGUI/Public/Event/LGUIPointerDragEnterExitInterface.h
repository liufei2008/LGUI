// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIPointerEventData.h"
#include "LGUIPointerDragEnterExitInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragEnterExitInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when dragging a object and pointer enter or exit another object
class LGUI_API ILGUIPointerDragEnterExitInterface
{
	GENERATED_BODY()
public:
	//Called when dragging a object and pointer enter another object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragEnter(const FLGUIPointerEventData& eventData);
	//Called when dragging a object and pointer exit another object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragExit(const FLGUIPointerEventData& eventData);
};