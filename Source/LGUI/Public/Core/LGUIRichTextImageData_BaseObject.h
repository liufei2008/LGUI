// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextGeometryCache.h"
#include "LGUIRichTextImageData_BaseObject.generated.h"

/** base class for UIText image render */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULGUIRichTextImageData_BaseObject :public UObject
{
	GENERATED_BODY()
public:
	DECLARE_EVENT(ULGUIRichTextImageData_BaseObject, FLGUIRichTextImageDataRefreshEvent);
	/** Called when any data change, and need UIText to refresh. */
	FLGUIRichTextImageDataRefreshEvent OnDataChange;
	/** Create or update image object. */
	virtual void CreateOrUpdateObject(class UUIItem* parent, const TArray<FUIText_RichTextImageTag>& imageTagArray, TArray<class UUIItem*>& inOutCreatedImageObjectArray, bool listImageObjectInOutliner) {};
};