// Copyright 2019-2022 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Interface.h"
#include "UIScrollViewWithScrollbarComponent.h"
#include "UIRecyclableScrollViewComponent.generated.h"

class AUIBaseActor;


UINTERFACE(Blueprintable, MinimalAPI)
class UUIRecyclableScrollViewCell : public UInterface
{
	GENERATED_BODY()
};

class LGUI_API IUIRecyclableScrollViewCell
{
	GENERATED_BODY()
};


UINTERFACE(Blueprintable, MinimalAPI)
class UUIRecyclableScrollViewDataSource : public UInterface
{
	GENERATED_BODY()
};

class LGUI_API IUIRecyclableScrollViewDataSource
{
	GENERATED_BODY()
public:
	/**
	 * @return data item count
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		int GetItemCount();
	/**
	 * Init cell when it is created. Only called one time when the cell is created.
	 * @param	Component		ActorComponent which implement UIRecyclableScrollViewCell interface. Cast this component to your own type and do the init process.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void InitOnCreate(UActorComponent* Component);

	// Called before calling any "SetCell" function for all children
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void BeforeSetCell();
	/**
	 * @param	Component		ActorComponent which implement UIRecyclableScrollViewCell interface. Cast this component to your own type and set cell UI's data.
	 * @param	Index			Cell's data index.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetCell(UActorComponent* Component, int Index);
	// Called after calling "SetCell" function for all children
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void AfterSetCell();
};

USTRUCT(BlueprintType)
struct FUIRecyclableScrollViewCellContainer
{
	GENERATED_BODY()
public:
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UActorComponent* CellComponent = nullptr;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		UUIItem* UIItem = nullptr;
};

/**
 * RecyclableScrollView can reuse cell's ui element.
 * Assign your own
 */
UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUIRecyclableScrollViewComponent : public UUIScrollViewWithScrollbarComponent
{
	GENERATED_BODY()

protected:
	virtual void Awake() override;
	virtual void Start() override;
	virtual void Update(float DeltaTime) override;
	virtual void OnDestroy()override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	/** Use a Actor which implement UIRecyclableScrollViewDataSource interface. */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView", meta = (AllowedClasses = "UIRecyclableScrollViewDataSource", DisplayThumbnail = "false"))
		UObject* DataSource;
	/** CellTemplate must have a ActorComponent which implement UIRecyclableScrollViewCell interface. */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView")
		TWeakObjectPtr<AUIBaseActor> CellTemplate;
	/** When use horizontal scroll, this can set the row count in every cell. */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView", meta = (ClampMin = "1", EditCondition = "Horizontal"))
		uint16 Rows = 1;
	/** When use vertical scroll, this can set the column count in every cell. */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView", meta = (ClampMin = "1", EditCondition = "Vertical"))
		uint16 Columns = 1;
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView")
		FMargin Padding = FMargin(0);
	/** Space between cells */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView")
		FVector2D Space = FVector2D::ZeroVector;
public:
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		TScriptInterface<IUIRecyclableScrollViewDataSource> GetDataSource()const { return DataSource; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		int GetRows()const { return Rows; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		int GetColumns()const { return Columns; }
	/** Get all created cell object array. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		const TArray<FUIRecyclableScrollViewCellContainer>& GetCacheCellList()const { return CacheCellList; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		const FMargin& GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		const FVector2D& GetSpace()const { return Space; }

	/**
	 * Delete all created cell objects.
	 * Call "UpdateWithDataSource" to recreate cells.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void ClearAllCells();

	/** Set DataSource object, will automatically recreate cells. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetDataSource(TScriptInterface<IUIRecyclableScrollViewDataSource> InDataSource);
	/** Set horizontal row count, will automatically recreate cells if current is horizontal scroll. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetRows(int value);
	/** Set vertical column count, will automatically recreate cells if current is vertical scroll. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetColumns(int value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetPadding(const FMargin& value);
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetSpace(const FVector2D& value);

	/** Recreate cells. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void UpdateWithDataSource() { InitializeOnDataSource(); }

	/**
	 * RecyclableScrollView will create a cache list to store cell object, use data-index to get the cell that represent the data.
	 * @param Index		data index
	 * @return			The cell object which represent the data. could be null if there is no cell represent the data (not in render range)
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		FUIRecyclableScrollViewCellContainer GetCellItemByDataIndex(int Index)const;
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI-RecyclableScrollView", AdvancedDisplay)
		TArray<FUIRecyclableScrollViewCellContainer> CacheCellList;

	void InitializeOnDataSource();
	FVector2D RangeArea = FVector2D::ZeroVector;//min max range point in parent location
	FDelegateHandle OnScrollEventDelegateHandle;
	void OnScrollCallback(FVector2D value);
	FVector2D PrevContentPosition = FVector2D::ZeroVector;
	int DataItemCount = 0;
	int MinCellIndexInCacheCellList = 0;
	int MaxCellIndexInCacheCellList = 0;
	float MinCellPosition = 0;//horizontal left cell position.y, or vertical top cell position.z
	int MinCellIndexInData = 0;//horizontal left-top cell data index, or vertical left-top cell data index.
	int MaxCellIndexInData = 0;//horizontal right-bottom cell data index, or vertical right-bottom cell data index.
	void IncreaseMinMaxCellIndexInCacheCellList(int Count);
	void DecreaseMinMaxCellIndexInCacheCellList(int Count);
};
