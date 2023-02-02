// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextGeometryCache.h"
#include "LGUIEmojiData_BaseObject.generated.h"

/** base class for UIText emoji render */
UCLASS(Abstract, BlueprintType)
class LGUI_API ULGUIEmojiData_BaseObject :public UObject
{
	GENERATED_BODY()
public:
	DECLARE_EVENT(ULGUIEmojiData_BaseObject, FLGUIEmojiDataRefreshEvent);
	/** Called when any data change, and need UIText to refresh. */
	FLGUIEmojiDataRefreshEvent OnDataChange;
	/** Create or update emoji object. */
	virtual void CreateOrUpdateEmojiObject(class UUIItem* parent, const TArray<FUIText_RichTextEmojiTag>& emojiTagArray, TArray<class UUIItem*>& inOutCreatedEmojiObjectArray, bool listEmojiObjectInOutliner) {};
};