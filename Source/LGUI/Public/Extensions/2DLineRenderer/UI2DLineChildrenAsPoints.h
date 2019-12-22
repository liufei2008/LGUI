﻿// Copyright 2019 LexLiu. All Rights Reserved.

#pragma once

#include "Core/Actor/UIBaseActor.h"
#include "UI2DLineRendererBase.h"
#include "Core/UIComponentBase.h"
#include "UI2DLineChildrenAsPoints.generated.h"

//Collect U2DLineChildrenAsPointsChild, and use child's relative location as points to draw line
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUI2DLineChildrenAsPoints : public UUI2DLineRendererBase
{
	GENERATED_BODY()

public:	
	UUI2DLineChildrenAsPoints(const FObjectInitializer& ObjectInitializer);

protected:
	virtual void BeginPlay()override;
	virtual void OnRegister()override;

	UPROPERTY(EditAnywhere, Category = LGUI)bool UseWidthOrHeightAsRadius = true;
	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)TArray<FVector2D> CurrentPointArray;

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool positionChanged, bool sizeChanged)override;
	UPROPERTY(Transient)TArray<UUIItem*> SortedItemArray;
	void CalculatePoints();
	void RebuildChildrenList();

	virtual const TArray<FVector2D>& GetCalcaultedPointArray()override
	{
		return CurrentPointArray;
	}
public:
	void OnChildPositionChanged();
};


UCLASS()
class LGUI_API AUI2DLineChildrenAsPointsActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUI2DLineChildrenAsPointsActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UIElement; }
	FORCEINLINE UUI2DLineChildrenAsPoints* Get2DLineChildrenAsPoints()const { return UIElement; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUI2DLineChildrenAsPoints* UIElement;

};