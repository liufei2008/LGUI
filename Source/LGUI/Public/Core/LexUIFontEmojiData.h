// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexUITextData.h"
#include "LexUIFontEmojiData.generated.h"

USTRUCT(BlueprintType)
struct LGUI_API FLexUIFontEmojiKey
{
	GENERATED_BODY()
#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, Category=LGUI)
	FString EmojiChar;
#endif
	UPROPERTY()
	uint32 EmojiCode = 0;
	UPROPERTY()
	uint16 VariantSelector = 0;

	FLexUIFontEmojiKey(){}
	FLexUIFontEmojiKey(uint32 InEmojiCode)
	{
		this->EmojiCode = InEmojiCode;
	}
	bool operator==(const FLexUIFontEmojiKey& other)const
	{
		return this->EmojiCode == other.EmojiCode;
	}
	friend FORCEINLINE uint32 GetTypeHash(const FLexUIFontEmojiKey& other)
	{
		return GetTypeHash(other.EmojiCode);
	}

#if WITH_EDITOR
	void ApplyEmoji();
#endif
};

USTRUCT(BlueprintType)
struct FLexUIFontEmojiDataItem
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TArray<TObjectPtr<class ULexUISpriteData_BaseObject>> Frames;
	/** use this value as animation-fps, -1 means not override */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float OverrideAnimationFps = -1;
};

UCLASS(NotBlueprintable, BlueprintType)
class LGUI_API ULexUIFontEmojiData :public UObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TMap<FLexUIFontEmojiKey, FLexUIFontEmojiDataItem> DataMap;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float AnimationFps = 4;
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	DECLARE_EVENT(ULexUIFontEmojiData, FLexUIFontEmojiDataRefreshEvent);
	/** Called when any data change, and need UIText to refresh. */
	FLexUIFontEmojiDataRefreshEvent OnDataChange;
	
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetDataMap(const TMap<FLexUIFontEmojiKey, FLexUIFontEmojiDataItem>& Value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetAnimationFps(float Value);

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		const TMap<FLexUIFontEmojiKey, FLexUIFontEmojiDataItem>& GetDataMap()const { return DataMap; }
	/** Get this to directly modify the data. After modify is done, call BroadcastOnDataChange function to notify. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		TMap<FLexUIFontEmojiKey, FLexUIFontEmojiDataItem>& GetMutableDataMap() { return DataMap; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void BroadcastOnDataChange();
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetAnimationFps()const { return AnimationFps; }

	void CreateOrUpdateObject(class ULexWidget* parent, const TArray<FLexUIText_Emoji>& emojiArray, TArray<TObjectPtr<class ULexWidget>>& inOutCreatedImageObjectArray);
	bool GetImageSize(const uint32& emojiCode, FIntVector2& outSize);
};