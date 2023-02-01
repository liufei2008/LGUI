// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextGeometryCache.h"
#include "LGUIEmojiData.generated.h"

USTRUCT()
struct FLGUIEmojiItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<class ULGUISpriteData_BaseObject*> frames;
	/** use this value as animation-fps, -1 means not override */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float overrideAnimationFps = -1;
};
/** emoji sprite data for UIText to render */
UCLASS(NotBlueprintable, NotBlueprintType)
class LGUI_API ULGUIEmojiData :public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TMap<FName, FLGUIEmojiItemData> emojiMap;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float animationFps = 4;
protected:
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
public:
	DECLARE_EVENT(ULGUIEmojiData, FLGUIEmojiDataRefreshEvent);

	FLGUIEmojiDataRefreshEvent OnDataChange;
	const TMap<FName, FLGUIEmojiItemData>& GetEmojiMap()const { return emojiMap; }
	float GetAnimationFps()const { return animationFps; }
};