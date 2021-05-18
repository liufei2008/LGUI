// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once
#include "LGUIImageSequencePlayer.h"
#include "UISpriteSequencePlayer.generated.h"

class ULGUISpriteData;

/** Play sprite sequence, need UISprite component. */
UCLASS(ClassGroup = (LGUI), meta = (BlueprintSpawnableComponent))
class LGUI_API UUISpriteSequencePlayer : public ULGUIImageSequencePlayer
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif
#if WITH_EDITORONLY_DATA
	/** preview in editor. */
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (UIMin = "0.0", UIMax = "1.0"))
		float preview = 0;
#endif
	UPROPERTY(Transient)
		TWeakObjectPtr<class UUISpriteBase> sprite;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<ULGUISpriteData*> spriteSequence;

	virtual bool CanPlay()override;
	virtual float GetDuration()const override;
	virtual void PrepareForPlay()override;
	virtual void OnUpdateAnimation(int frameNumber)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TArray<ULGUISpriteData*> GetSpriteSequence()const { return spriteSequence; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSpriteSequence(TArray<ULGUISpriteData*> value);
};
