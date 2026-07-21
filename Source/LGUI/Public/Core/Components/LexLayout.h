// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "CoreMinimal.h"
#include "LexWidget.h"
#include "LexWidgetSubObjectBehaviour.h"
#include "LexLayout.generated.h"


struct FLexLayoutControlAnchorData
{
	bool bCanControlHorizontalPosition = false;
	bool bCanControlVerticalPosition = false;
	bool bCanControlHorizontalSize = false;
	bool bCanControlVerticalSize = false;

	bool HaveRepeatedControl(const FLexLayoutControlAnchorData& Other)const
	{
		if (
			(bCanControlHorizontalPosition && Other.bCanControlHorizontalPosition)
			|| (bCanControlVerticalPosition && Other.bCanControlVerticalPosition)
			|| (bCanControlHorizontalSize && Other.bCanControlHorizontalSize)
			|| (bCanControlVerticalSize && Other.bCanControlVerticalSize)
			)
		{
			return true;
		}
		return false;
	}
	void Or(const FLexLayoutControlAnchorData& Other)
	{
		bCanControlHorizontalPosition |= Other.bCanControlHorizontalPosition;
		bCanControlVerticalPosition |= Other.bCanControlVerticalPosition;
		bCanControlHorizontalSize |= Other.bCanControlHorizontalSize;
		bCanControlVerticalSize |= Other.bCanControlVerticalSize;
	}
	bool AnyControl()const
	{
		return bCanControlHorizontalPosition || bCanControlVerticalPosition
		|| bCanControlHorizontalSize || bCanControlVerticalSize;
	}
	bool Conflict(const FLexLayoutControlAnchorData& Other)const
	{
		if (bCanControlHorizontalPosition && bCanControlHorizontalPosition == Other.bCanControlHorizontalPosition)
			return true;
		if (bCanControlVerticalPosition && bCanControlVerticalPosition == Other.bCanControlVerticalPosition)
			return true;
		if (bCanControlHorizontalSize && bCanControlHorizontalSize == Other.bCanControlHorizontalSize)
			return true;
		if (bCanControlVerticalSize && bCanControlVerticalSize == Other.bCanControlVerticalSize)
			return true;
		return false;
	}
};

UCLASS(Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexLayout : public ULexWidgetSubObjectBehaviour
{
	GENERATED_BODY()
protected:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
public:
	virtual void OnTransformChanged() {}
	virtual void OnDimensionChanged(bool InPivotChange, bool InWidthChange, bool InHeightChange) {}
	
	virtual FLexLayoutControlAnchorData GetLayoutControlAnchor(const ULexWidget* Widget)const PURE_VIRTUAL(ULexLayout::GetLayoutControlAnchor, return FLexLayoutControlAnchorData(););

	virtual FVector2f GetLayoutPreferredSize()PURE_VIRTUAL(ULexLayout::GetLayoutProperties, return FVector2f::ZeroVector;);
	virtual void MarkLayoutDirty();
protected:
	bool bIsLayoutDirty = false;
};

USTRUCT(BlueprintType)
struct FLayoutAnimationSnapshotData
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere)
	TObjectPtr<ULexWidget> Widget = nullptr;
	UPROPERTY(EditAnywhere)
	FVector2D Position;
	UPROPERTY(EditAnywhere)
	FVector2D Size;
};

UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexLayoutAnimation : public UObject
{
	GENERATED_BODY()
public:
	ULexLayoutAnimation();

	virtual void OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, TArray<TWeakObjectPtr<ULTweener>>& ResultTweenerArray);

	UFUNCTION(BlueprintCallable, Category = LGUI)
	ULexLayoutContainer* GetLayoutContainer()const;
private:
	UPROPERTY(Transient, BlueprintReadOnly, Category = LGUI, Getter=GetLayoutContainer, meta = (AllowPrivateAccess = true), DisplayName=LayoutContainer)
	mutable TObjectPtr<ULexLayoutContainer> OwnerLayoutContainer;
	bool bCanExecuteBlueprintEvent;
protected:
	UFUNCTION(BlueprintImplementableEvent, meta = (DisplayName = "OnApplyLayoutResults"), Category = "LayoutContainer")
	void ReceiveOnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, TArray<ULTweener*>& ResultTweenerArray);
};

UCLASS(BlueprintType)
class LGUI_API ULexLayoutAnimation_CommonTween : public ULexLayoutAnimation
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	float Duration = 0.3f;
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	ELTweenEase Ease = ELTweenEase::OutCubic;
	UPROPERTY(EditAnywhere, Category = "LayoutContainer", meta = (EditCondition = "Ease==ELTweenEase::CurveFloat"))
	FRuntimeFloatCurve EaseCurve;
public:
	virtual void OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, TArray<TWeakObjectPtr<ULTweener>>& ResultTweenerArray) override;
};

UCLASS(BlueprintType)
class LGUI_API ULexLayoutAnimation_SlideIn : public ULexLayoutAnimation
{
	GENERATED_BODY()
private:
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	float Duration = 0.3f;
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	ELTweenEase Ease = ELTweenEase::OutCubic;
	UPROPERTY(EditAnywhere, Category = "LayoutContainer", meta = (EditCondition = "Ease==ELTweenEase::CurveFloat"))
	FRuntimeFloatCurve EaseCurve;
	
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	float OpacityOffset = 0;
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	FVector2D PositionOffset = FVector2D(100, 0);
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	FVector2D SizeOffset = FVector2D(0, 0);
public:
	virtual void OnApplyLayoutResults(const TArray<FLayoutAnimationSnapshotData>& SnapshotDataArray, TArray<TWeakObjectPtr<ULTweener>>& ResultTweenerArray) override;
};

/**
 * LayoutContainer can handle children position
 */
UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexLayoutContainer : public ULexLayout
{
	GENERATED_BODY()
public:
	ULexLayoutContainer();
protected:
	UPROPERTY(EditAnywhere, Category = "LayoutContainer")
	bool bUseAnimation = false;
	UPROPERTY(EditAnywhere, Instanced, Category = "LayoutContainer", meta = (EditCondition = "bUseAnimation"))
	TObjectPtr<ULexLayoutAnimation> AnimationHandler;

	//position and size snapshot before layout calculation
	TArray<FLayoutAnimationSnapshotData> LayoutAnimSnapshotDataArray;
	TArray<TWeakObjectPtr<ULTweener>> LayoutAnimTweenerArray;

	void RefreshChildren();
	UPROPERTY(Transient)TArray<ULexWidget*> Children;
public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	virtual void PostReinitProperties()override;

	virtual void OnRegister() override;

	virtual void SnapshotLayout();
	virtual void ApplyLayoutResult();
	
	//called by LexWidget during layout processing
	virtual void CalculateLayout(){}

	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	bool GetUseAnimation()const{return bUseAnimation;}
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer")
	void SetUseAnimation(bool Value){bUseAnimation = Value;}
	
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer", meta=(DeterminesOutputType="LayoutClass"))
	void SetLayoutAnimation(ULexLayoutAnimation* Value);
	UFUNCTION(BlueprintCallable, Category = "LayoutContainer", meta=(DeterminesOutputType="LayoutClass"))
	ULexLayoutAnimation* CreateNewLayoutAnimation(TSubclassOf<ULexLayoutAnimation> Class);
	template<class T>
	T* CreateNewLayoutAnimation()
	{
		static_assert(TPointerIsConvertibleFromTo<T, const ULexLayoutAnimation>::Value, "'T' template parameter to CreateNewLayoutAnimation must be derived from ULexLayoutAnimation");
		return (T*)CreateNewLayoutAnimation(T::StaticClass());
	}
};

/**
 * LayoutSelf can handle self size.
 * This base class just provide IgnoreLayout.
 */
UCLASS(BlueprintType, Abstract, DefaultToInstanced, EditInlineNew)
class LGUI_API ULexLayoutSelf : public ULexLayout
{
	GENERATED_BODY()

public:
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
	virtual void PostReinitProperties()override;

	//called by LexWidget during layout processing
	virtual void CalculateSize(){}
	
	virtual FVector2f GetLayoutFinalSize();
	
	virtual FLexLayoutControlAnchorData GetLayoutControlAnchor(const ULexWidget* Widget) const override{return FLexLayoutControlAnchorData();}
};
