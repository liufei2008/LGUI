// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LGUIBaseInputModule.h"
#include "LGUI_PointerInputModule.generated.h"

class ULGUIPointerEventData;
class ULGUIBaseRaycaster;

UCLASS(Abstract)
class LGUI_API ULGUI_PointerInputModule : public ULGUIBaseInputModule
{
	GENERATED_BODY()

protected:
	UPROPERTY(Transient)class ULGUIEventSystem* eventSystem = nullptr;
	bool CheckEventSystem();
	struct FHitResultContainerStruct
	{
		FHitResult hitResult;
		bool eventFireOnAll = false;

		FVector rayOrigin = FVector(0, 0, 0), rayDirection = FVector(1, 0, 0), rayEnd = FVector(1, 0, 0);

		ULGUIBaseRaycaster* raycaster = nullptr;
	};

	UPROPERTY(VisibleAnywhere, Category = LGUI)TMap<int, ULGUIPointerEventData*> pointerEventDataMap;
	ULGUIPointerEventData* GetPointerEventData(int pointerID, bool createIfNotExist = false);
	void ProcessPointerEvent(ULGUIPointerEventData* pointerEventData, bool pointerHitAnything, const FHitResultContainerStruct& hitResult, bool& outIsHitSomething, FHitResult& outHitResult);
	bool LineTrace(ULGUIPointerEventData* InPointerEventData, FHitResultContainerStruct& hitResult);
	TArray<FHitResultContainerStruct> multiHitResult;//temp array for hit result

	void ClearEventByID(int pointerID);
public:
	virtual void ClearEvent()override;
};