// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "TextGeometryCache.h"
#include "LGUIRichTextImageData_BaseObject.h"
#include "LGUIRichTextImageData.generated.h"

USTRUCT()
struct FLGUIRichTextImageItemData
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		TArray<class ULGUISpriteData_BaseObject*> frames;
	/** use this value as animation-fps, -1 means not override */
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float overrideAnimationFps = -1;
};
/** use sprite to render image for UIText */
UCLASS(NotBlueprintable, NotBlueprintType)
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
	virtual void CreateOrUpdateObject(class UUIItem* parent, const TArray<FUIText_RichTextImageTag>& imageTagArray, TArray<class UUIItem*>& inOutCreatedImageObjectArray, bool listImageObjectInEditorOutliner)override;
};