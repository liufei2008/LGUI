// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUIPointerEventData.h"
#include "LGUIPointerDragDropInterface.generated.h"


UINTERFACE(Blueprintable, MinimalAPI)
class ULGUIPointerDragDropInterface : public UInterface
{
	GENERATED_BODY()
};
//Called when dragging a object and drop on another object
class LGUI_API ILGUIPointerDragDropInterface
{
	GENERATED_BODY()
public:
	//Called when dragging a object and drop on another object
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = LGUI)
		bool OnPointerDragDrop(ULGUIPointerEventData* eventData);
};