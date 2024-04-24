// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "LGUITextData.h"
#include "LGUIRichTextImageData_BaseObject.h"
#include "LGUIRichTextImageData.generated.h"

USTRUCT(BlueprintType)
struct FLGUIRichTextImageItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<TObjectPtr<class ULGUISpriteData_BaseObject>> frames;
	/** use this value as animation-fps, -1 means not override */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float overrideAnimationFps = -1;
};
/** use sprite to render image for UIText */
UCLASS(NotBlueprintable, BlueprintType)
class LGUI_API ULGUIRichTextImageData :public ULGUIRichTextImageData_BaseObject
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TMap<FName, FLGUIRichTextImageItemData> imageMap;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float animationFps = 4;
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetImageMap(const TMap<FName, FLGUIRichTextImageItemData>& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetAnimationFps(float value);

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		const TMap<FName, FLGUIRichTextImageItemData>& GetImageMap()const { return imageMap; }
	/** Get this to directly modify the data. After modify is done, call BroadcastOnDataChange function to notify. */
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		TMap<FName, FLGUIRichTextImageItemData>& GetMutableImageMap() { return imageMap; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void BroadcastOnDataChange();
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		float GetAnimationFps()const { return animationFps; }

	virtual void CreateOrUpdateObject(class UUIItem* parent, const TArray<FUIText_RichTextImageTag>& imageTagArray, TArray<TObjectPtr<class UUIItem>>& inOutCreatedImageObjectArray, bool listImageObjectInEditorOutliner)override;
};