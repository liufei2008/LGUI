// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UISpriteBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "LTweener.h"
#include "UIPolygon.generated.h"


UENUM(BlueprintType, Category = LGUI)
enum class UIPolygonUVType :uint8
{
	//Use full rect uv
	SpriteRect,
	//Use left center as polygon's center, and right center as polygon's ring uv
	HeightCenter,
	//Use left center as polygon's center, right bottom as polygon ring's start, and right top as polygon ring's end
	StretchSpriteHeight,
};
/**
 * render a solid polygon shape
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIPolygon : public UUISpriteBase
{
	GENERATED_BODY()

public:	
	UUIPolygon(const FObjectInitializer& ObjectInitializer);

protected:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool FullCycle = true;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		float StartAngle = 0.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta = (EditCondition = "!FullCycle"))
		float EndAngle = 90.0f;
	//Sides of polygon
	UPROPERTY(EditAnywhere, Category = "LGUI")
		int Sides = 3;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UIPolygonUVType UVType = UIPolygonUVType::SpriteRect;
	UPROPERTY(EditAnywhere, Category = "LGUI", meta=(UIMin="0.0", UIMax="1.0"))
		TArray<float> VertexOffsetArray;
	
	virtual void OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetStartAngle()const { return StartAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") float GetEndAngle()const { return EndAngle; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") int GetSides()const { return Sides; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") UIPolygonUVType GetUVType()const { return UVType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI") const TArray<float>& GetVertexOffsetArray()const { return VertexOffsetArray; }
	//Return direct mutable array for edit and change. Call MarkVertexPositionDirty() function after change.
	TArray<float>& GetVertexOffsetArray_Direct() { return VertexOffsetArray; }

	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetStartAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetEndAngle(float value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetSides(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetUVType(UIPolygonUVType value);
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		void SetVertexOffsetArray(const TArray<float>& value);

	UFUNCTION(BlueprintCallable, Category = "LTweenLGUI")
		ULTweener* StartAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
	UFUNCTION(BlueprintCallable, Category = "LTweenLGUI")
		ULTweener* EndAngleTo(float endValue, float duration = 0.5f, float delay = 0.0f, ELTweenEase easeType = ELTweenEase::OutCubic);
};

/**
 * render a solid polygon shape
 */
UCLASS(ClassGroup = LGUI)
class LGUI_API AUIPolygonActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUIPolygonActor();

	virtual UUIItem* GetUIItem()const override { return UIPolygon; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIPolygon; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUIPolygon* GetUIPolygon()const { return UIPolygon; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUIPolygon> UIPolygon;

};
