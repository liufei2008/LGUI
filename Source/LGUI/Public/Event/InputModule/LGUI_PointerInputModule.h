// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "LGUI_PointerInputModule.generated.h"

class ULGUIPointerEventData;
class ULGUIBaseRaycaster;

//only one InputModule is valid in the same time. so if multiple InputModule is activated, then the latest one is valid.
UCLASS(Abstract)
class LGUI_API ULGUI_PointerInputModule : public ULGUIBaseInputModule
{
	GENERATED_BODY()

protected:

	struct FHitResultContainerStruct
	{
		FHitResult hitResult;
		bool eventFireOnAll = false;

		FVector rayOrigin = FVector(0, 0, 0), rayDirection = FVector(1, 0, 0), rayEnd = FVector(1, 0, 0);

		ULGUIBaseRaycaster* raycaster = nullptr;
	};

	UPROPERTY(VisibleAnywhere, Category = LGUI)TMap<int, ULGUIPointerEventData*> pointerEventDataMap;
	ULGUIPointerEventData* GetPointerEventData(int pointerID, bool createIfNotExist = false);
	void ProcessPointerEvent(ULGUIPointerEventData* pointerEventData, bool pointerHitAnything, const FHitResultContainerStruct& hitResult, bool& isHitSomething);
	bool LineTrace(FHitResultContainerStruct& hitResult);
	TArray<FHitResultContainerStruct> multiHitResult;//temp array for hit result

	void ClearEventByID(int pointerID);
public:
	virtual void ClearEvent()override;
};