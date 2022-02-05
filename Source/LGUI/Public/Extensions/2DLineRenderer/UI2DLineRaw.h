// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "Core/Actor/UIBaseActor.h"
#include "UI2DLineRendererBase.h"
#include "UI2DLineRaw.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUI2DLineRaw : public UUI2DLineRendererBase
{
	GENERATED_BODY()

public:	
	UUI2DLineRaw(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)override;
#endif

	UPROPERTY(EditAnywhere, Category = LGUI)
		TArray<FVector2D> PointArray = { FVector2D(-100, 0), FVector2D(100, 0) };

	virtual void CalculatePoints()override {};
	virtual const TArray<FVector2D>& GetCalcaultedPointArray()override
	{
		return PointArray;
	}
public:
	UFUNCTION(BlueprintCallable, Category = LGUI)
		void SetPoints(const TArray<FVector2D>& InPoints);
};


UCLASS(ClassGroup = LGUI)
class LGUI_API AUI2DLineActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUI2DLineActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIElement; }
	FORCEINLINE UUI2DLineRaw* Get2DLineRaw()const { return UIElement; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUI2DLineRaw* UIElement;

};