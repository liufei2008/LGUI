// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LexImageSequencePlayer.h"
#include "UISpriteSheetTexturePlayer.generated.h"

class ULexUISpriteData;

/** Play spritesheet texture, need UITexture component. */
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUISpriteSheetTexturePlayer : public ULexImageSequencePlayer
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
	UPROPERTY(Transient)
		TWeakObjectPtr<class ULexTexture> Texture;
	/** Sprite element count of horizontal direction in texture. */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		int WidthCount = 8;
	/** Sprite element count of vertical direction in texture. */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		int HeightCount = 8;

	float WidthUVInterval, HeightUVInterval;
	virtual bool CanPlay()override;
	virtual float GetDuration()const override;
	virtual void PrepareForPlay()override;
	virtual void OnUpdateAnimation(int FrameNumber)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		int GetWidthCount()const { return WidthCount; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		int GetHeightCount()const { return HeightCount; }
	/** Will take effect on next cycle. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetWidthCount(int value);
	/** Will take effect on next cycle. */
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetHeightCount(int value);
};
