// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LexUITextData.h"
#include "LexUIRichTextImageData_BaseObject.h"
#include "LexUIRichTextImageData.generated.h"

USTRUCT(BlueprintType)
struct FLexUIRichTextImageItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TArray<TObjectPtr<class ULexUISpriteData_BaseObject>> Frames;
	/** use this value as animation-fps, -1 means not override */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float OverrideAnimationFps = -1;
};
/** use Sprite to render image for UIText */
UCLASS(NotBlueprintable, BlueprintType)
class LGUI_API ULexUIRichTextImageData :public ULexUIRichTextImageData_BaseObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TMap<FName, FLexUIRichTextImageItemData> ImageMap;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		float AnimationFps = 4;
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetImageMap(const TMap<FName, FLexUIRichTextImageItemData>& value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetAnimationFps(float value);

	UFUNCTION(BlueprintCallable, Category = "LexUI")
		const TMap<FName, FLexUIRichTextImageItemData>& GetImageMap()const { return ImageMap; }
	/** Get this to directly modify the data. After modify is done, call BroadcastOnDataChange function to notify. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		TMap<FName, FLexUIRichTextImageItemData>& GetMutableImageMap() { return ImageMap; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void BroadcastOnDataChange();
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		float GetAnimationFps()const { return AnimationFps; }

	virtual void CreateOrUpdateObject(class ULexWidget* parent, const TArray<FLexUIText_RichTextImageTag>& imageTagArray, TArray<TObjectPtr<class ULexWidget>>& inOutCreatedImageObjectArray)override;
	virtual bool GetImageSize(const FName& imageTag, FIntVector2& outSize)override;
};