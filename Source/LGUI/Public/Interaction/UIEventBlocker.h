// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Event/Interface/LexPointerEnterExitInterface.h"
#include "Event/Interface/LexPointerDownUpInterface.h"
#include "Event/Interface/LexPointerClickInterface.h"
#include "Event/Interface/LexPointerDragInterface.h"
#include "Event/Interface/LexPointerDropInterface.h"
#include "Event/Interface/LexPointerScrollInterface.h"
#include "Event/Interface/LexSelectDeselectInterface.h"
#include "Core/LexUIBehaviour.h"
#include "UIEventBlocker.generated.h"

//use this component to stop LexPointerEvent bubble up
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIEventBlocker : public ULexUIBehaviour
	, public ILexPointerEnterExitInterface
	, public ILexPointerDownUpInterface
	, public ILexPointerClickInterface
	, public ILexPointerDragInterface
	, public ILexPointerDropInterface
	, public ILexPointerScrollInterface
	, public ILexSelectDeselectInterface
{
	GENERATED_BODY()
protected:
	UPROPERTY(EditAnywhere, Category = "UIEventBlocker") bool AllowEventBubbleUp = false;
	virtual void OnPointerEnter_Implementation(ULexPointerEventData* EventData)override;
	virtual void OnPointerExit_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDown_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerUp_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerClick_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerBeginDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerEndDrag_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerDrop_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnPointerScroll_Implementation(ULexPointerEventData* EventData)override;
	virtual bool OnSelect_Implementation(ULexBaseEventData* EventData)override;
	virtual bool OnDeselect_Implementation(ULexBaseEventData* EventData)override;
};
