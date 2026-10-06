// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LexImageSequencePlayer.h"
#include "UISpriteSequencePlayer.generated.h"

class ULexUISpriteData_BaseObject;

/** Play Sprite sequence, need UISprite component. */
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUISpriteSequencePlayer : public ULexImageSequencePlayer
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
	UPROPERTY(Transient)
		TWeakObjectPtr<class ULexSpriteBase> Sprite;
	UPROPERTY(EditAnywhere, Category = "LexUI")
		TArray<TObjectPtr<ULexUISpriteData_BaseObject>> SpriteSequence;
	/** should also set size to Sprite-data? */
	UPROPERTY(EditAnywhere, Category = "LexUI")
		bool bSnapSpriteSize = true;

	virtual bool CanPlay()override;
	virtual float GetDuration()const override;
	virtual void PrepareForPlay()override;
	virtual void OnUpdateAnimation(int FrameNumber)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		const TArray<ULexUISpriteData_BaseObject*>& GetSpriteSequence()const { return SpriteSequence; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		bool GetSnapSpriteSize()const { return bSnapSpriteSize; }
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetSpriteSequence(TArray<ULexUISpriteData_BaseObject*> value);
	UFUNCTION(BlueprintCallable, Category = "LexUI")
		void SetSnapSpriteSize(bool value);
};
