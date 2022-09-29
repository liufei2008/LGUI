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

UENUM(BlueprintType)
enum class EUIRecyclableScrollViewCellTemplateType :uint8
{
	Actor, Prefab,
};

/**
 * RecyclableScrollView can reuse cell's ui element.
 * Assign your own DataSource object (IUIRecyclableScrollViewDataSource) and create recyclable scroll view on that data.
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
	virtual bool CanEditChange(const FProperty* InProperty)const override;
#endif
protected:
	/** Use a Actor which implement UIRecyclableScrollViewDataSource interface. */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView", meta = (AllowedClasses = "UIRecyclableScrollViewDataSource", DisplayThumbnail = "false"))
		UObject* DataSource;
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView")
		EUIRecyclableScrollViewCellTemplateType CellTemplateType = EUIRecyclableScrollViewCellTemplateType::Actor;
	/**
	 * CellTemplate must have a ActorComponent which implement UIRecyclableScrollViewCell interface.
	 * Only valid if CellTemplateType is Actor.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView")
		AUIBaseActor* CellTemplate;
	/**
	 * CellTemplatePrefab's root actor must have a ActorComponent which implement UIRecyclableScrollViewCell interface.
	 * Only valid if CellTemplateType is Prefab.
	 */
	UPROPERTY(EditAnywhere, Category = "LGUI-RecyclableScrollView")
		class ULGUIPrefab* CellTemplatePrefab;
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
	/**
	 * Get all created cell object array. Note this just directly return cell list, which is not in user-friendly order (first one may not at the left-top position).
	 * Use "GetUserFriendlyCacheCellList" can get the cell list in good order.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		const TArray<FUIRecyclableScrollViewCellContainer>& GetCacheCellList()const { return CacheCellList; }
	/** Get all created cell object array, with user-friendly order (left-top is the first one, and right-bottom is last). */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void GetUserFriendlyCacheCellList(TArray<FUIRecyclableScrollViewCellContainer>& OutResult)const;
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		const FMargin& GetPadding()const { return Padding; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		const FVector2D& GetSpace()const { return Space; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		EUIRecyclableScrollViewCellTemplateType GetCellTemplateType()const { return CellTemplateType; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		AUIBaseActor* GetCellTemplate()const { return CellTemplate; }
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		class ULGUIPrefab* GetCellTemplatePrefab()const { return CellTemplatePrefab; }

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
	/**
	 * CellTemplate must have a ActorComponent which implement UIRecyclableScrollViewCell interface.
	 * This function only set the parameter. If you want to refresh the display UI list, just call UpdateWithDataSource.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetCellTemplate(AUIBaseActor* value);
	/**
	 * CellTemplatePrefab's root actor must have a ActorComponent which implement UIRecyclableScrollViewCell interface.
	 * This function only set the parameter. If you want to refresh the display UI list, just call UpdateWithDataSource.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void SetCellTemplatePrefab(class ULGUIPrefab* value);

	/** Recreate cell list. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void RecreateList() { InitializeOnDataSource(); }
	UE_DEPRECATED(4.26, "Use RecreateList instead.")
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView", meta = (DeprecatedFunction, DeprecationMessage = "Use RecreateList instead."))
		void UpdateWithDataSource() { RecreateList(); }
	/** Update list cell's date, this will not change current layout, only set data. */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		void UpdateCellData();
	/**
	 * RecyclableScrollView will create a cache list to store cell object, use data-index to get the cell that represent the data.
	 * @param Index		data index
	 * @param OutResult			The cell object which represent the data. could be null if there is no cell represent the data (not in render range)
	 * @return true if have valid result, false otherwise
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-RecyclableScrollView")
		bool GetCellItemByDataIndex(int Index, FUIRecyclableScrollViewCellContainer& OutResult)const;

	/**
	 * Try to scroll the scrollview so the cell item with InDataIndex can sit at center. Will clamp it in valid range.
	 * @param InDataIndex Cell data index.
	 * @param InEaseAnimation true-use tween animation to make smooth scroll, false-immediate set.
	 * @param InAnimationDuration Animation duration if InEaseAnimation = true.
	 */
	UFUNCTION(BlueprintCallable, Category = "LGUI-ScrollViewWithScrollbar")
		void ScrollToByDataIndex(int InDataIndex, bool InEaseAnimation = true, float InAnimationDuration = 0.5f);
private:
	UPROPERTY(VisibleAnywhere, Transient, Category = "LGUI-RecyclableScrollView", AdvancedDisplay)
		TArray<FUIRecyclableScrollViewCellContainer> CacheCellList;

	void InitializeOnDataSource();
	EUIRecyclableScrollViewCellTemplateType WorkingCellTemplateType = EUIRecyclableScrollViewCellTemplateType::Actor;
	TWeakObjectPtr<AUIBaseActor> WorkingCellTemplate = nullptr;//current using cell template, could be CellTemplate or CellTemplatePrefab's instance, tell by 'WorkingCellTemplateType'
	FVector2D WorkingCellTemplateSize = FVector2D::ZeroVector;
	FVector2D RangeArea = FVector2D::ZeroVector;//min max range point in parent location
	FDelegateHandle OnScrollEventDelegateHandle;
	void OnScrollCallback(FVector2D value);
	FVector2D PrevContentPosition = FVector2D::ZeroVector;
	int DataItemCount = 0;
	int MinCellIndexInCacheCellList = 0;
	int MaxCellIndexInCacheCellList = 0;
	float MinCellPosition = 0;//horizontal left cell position.y, or vertical top cell position.z
	int MinCellDataIndex = 0;//horizontal left-top cell data index, or vertical left-top cell data index.
	void IncreaseMinMaxCellIndexInCacheCellList(int Count);
	void DecreaseMinMaxCellIndexInCacheCellList(int Count);
};
