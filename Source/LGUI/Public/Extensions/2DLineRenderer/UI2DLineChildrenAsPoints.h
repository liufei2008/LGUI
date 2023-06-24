// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once

#include "Core/Actor/UIBaseActor.h"
#include "UI2DLineRendererBase.h"
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

	UPROPERTY(VisibleAnywhere, Transient, Category = LGUI)TArray<FVector2D> 
		CurrentPointArray;

	virtual void OnUIChildHierarchyIndexChanged(UUIItem* child)override;
	virtual void OnUIChildAttachmentChanged(UUIItem* child, bool attachOrDetach)override;
	virtual void OnUIChildDimensionsChanged(UUIItem* child, bool horizontalPositionChanged, bool verticalPositionChanged, bool widthChanged, bool heightChanged)override;
	UPROPERTY(Transient)TArray<TObjectPtr<UUIItem>> SortedItemArray;
	void RebuildChildrenList();

	virtual void CalculatePoints()override;
	virtual const TArray<FVector2D>& GetCalcaultedPointArray()override
	{
		return CurrentPointArray;
	}
public:
	void OnChildPositionChanged();
};


UCLASS(ClassGroup = LGUI)
class LGUI_API AUI2DLineChildrenAsPointsActor : public AUIBaseRenderableActor
{
	GENERATED_BODY()

public:
	AUI2DLineChildrenAsPointsActor();

	virtual UUIItem* GetUIItem()const override { return UIElement; }
	virtual class UUIBaseRenderable* GetUIRenderable()const override { return UIElement; }
	UFUNCTION(BlueprintCallable, Category = "LGUI")
		UUI2DLineChildrenAsPoints* Get2DLineChildrenAsPoints()const { return UIElement; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		TObjectPtr<class UUI2DLineChildrenAsPoints> UIElement;

};