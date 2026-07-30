// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Event/InputModule/LexBaseInputModule.h"
#include "Event/LexPointerEventData.h"
#include "LexPointerInputModule.generated.h"

class ULexBaseRaycaster;
class ULexEventSystem;

UCLASS(Abstract)
class LGUI_API ULexPointerInputModule : public ULexBaseInputModule
{
	GENERATED_BODY()

public:
	static void ProcessPointerEvent(ULexEventSystem* EventSystem, ULexPointerEventData* pointerEventData, bool pointerHitAnything, const FLexUIHitResultContainer& hitResult, bool& OutIsHitSomething, FLexUIHitResult& OutHitResult);
protected:
	
	bool LineTrace(ULexPointerEventData* InPointerEventData, FLexUIHitResultContainer& OutLexHitResult);
	TArray<FLexUIHitResultContainer> MultiHitResult;//temp array for hit result
	static void ProcessPointerEnterExit(ULexEventSystem* EventSystem, ULexPointerEventData* pointerEventData, ULexWidget* OldObj, ULexWidget* NewObj);
	/** find a common root actor of two actors. return nullptr if no common root */
	static ULexWidget* FindCommonRoot(ULexWidget* A, ULexWidget* B);

	bool Navigate(ELexUINavigationDirection InDirection, ULexPointerEventData* InPointerEventData, FLexUIHitResultContainer& hitResult);
	void ProcessInputForNavigation();
	void ProcessInputForNavigation(ULexPointerEventData* InPointerEventData);
	void ClearEventByID(int pointerID);
	static bool CanHandleInterface(ULexWidget* targetComp, UClass* targetInterfaceClass);
	static ULexWidget* GetEventHandle(ULexWidget* targetComp, UClass* targetInterfaceClass);
	static void DeselectIfSelectionChanged(ULexEventSystem* eventSystem, ULexWidget* currentPressed, ULexBaseEventData* EventData);
public:
	virtual void ClearEvent()override;
};