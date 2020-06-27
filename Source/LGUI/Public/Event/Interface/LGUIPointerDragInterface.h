// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/LGUIPointerEventData.h"
#include "LGUIPointerDragInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when drag a object
class LGUI_API ILGUIPointerDragInterface
{
	GENERATED_BODY()
public:
	//Called when drag a object begin
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerBeginDrag(ULGUIPointerEventData* eventData);
	//Called when drag a object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDrag(ULGUIPointerEventData* eventData);
	//Called when drag a object end
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerEndDrag(ULGUIPointerEventData* eventData);
};