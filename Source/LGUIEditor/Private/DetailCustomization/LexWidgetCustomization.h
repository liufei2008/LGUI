// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#include "Core/Components/LexLayout.h"
#include "Widget/AnchorPreviewWidget.h"
#pragma once

/**
 * 
 */
class FLexWidgetCustomization : public IDetailCustomization
{
public:
	FLexWidgetCustomization();
	~FLexWidgetCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class ULexWidget>> TargetScriptArray;
	static TArray<float> ValueRangeArray;

	FText GetAnchorsTooltipText()const;
	
	void ForceUpdateUI();

	bool OnCanCopyAnchor()const;
	bool OnCanPasteAnchor()const;
	void OnCopyAnchor();
	void OnPasteAnchor(IDetailLayoutBuilder* DetailBuilder);
	void OnCopyHierarchyIndex();
	void OnPasteHierarchyIndex(TSharedRef<IPropertyHandle> PropertyHandle);
	FReply OnClickIncreaseOrDecreaseSiblingIndex(bool IncreaseOrDecrease, TSharedRef<IPropertyHandle> HierarchyIndexHandle);
	EVisibility GetAnchorPresetButtonVisibility()const;

	FMargin AnchorOffset = FMargin(0);
	void OnPrePivotChange(TSharedPtr<IPropertyHandle> PivotPH);
	void OnPivotChanged(TSharedPtr<IPropertyHandle> PivotPH);

	TOptional<float> GetAnchorValue(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)const;
	TOptional<float> GetMinMaxSliderValue(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex, bool MinOrMax)const;
	void ApplyValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex, bool Commited);
	void OnAnchorValueChanged(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex);
	void OnAnchorValueCommitted(float Value, ETextCommit::Type commitType, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex);
	void OnAnchorValueSliderMovementBegin();
	void OnAnchorValueSliderMovementEnd(float Value, TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex);
	bool IsAnchorValueEnable(TSharedRef<IPropertyHandle> AnchorHandle, int AnchorValueIndex)const;
	bool IsAnchorEditable()const;
	TSharedPtr<IPropertyHandle> GetAnchorPropertyHandle(IDetailLayoutBuilder* DetailBuilder, TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int Index)const;
	FText GetAnchorLabelText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int LabelIndex)const;
	FText GetAnchorLabelTooltipText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle, int LabelTooltipIndex)const;
	void OnSelectAnchor(LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign HorizontalAlign, LGUIAnchorPreviewWidget::UIAnchorVerticalAlign VerticalAlign, IDetailLayoutBuilder* DetailBuilder);
	LGUIAnchorPreviewWidget::UIAnchorHorizontalAlign GetAnchorHAlign(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const;
	LGUIAnchorPreviewWidget::UIAnchorVerticalAlign GetAnchorVAlign(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const;
	FText GetHAlignText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const;
	FText GetVAlignText(TSharedRef<IPropertyHandle> AnchorMinHandle, TSharedRef<IPropertyHandle> AnchorMaxHandle)const;

	FLexLayoutControlAnchorData GetLayoutControlAnchorValue()const;
	enum class EAnchorControlledByLayoutType
	{
		HorizontalAnchor,
		HorizontalAnchoredPosition,
		HorizontalSizeDelta,
		VerticalAnchor,
		VerticalAnchoredPosition,
		VerticalSizeDelta,
	};
	bool IsAnchorControlledByMultipleLayout(TMap<EAnchorControlledByLayoutType, TArray<UObject*>>& Result)const;
	bool GetLayoutControlHorizontalAnchoredPosition()const;
	bool GetLayoutControlVerticalAnchoredPosition()const;
	bool GetLayoutControlHorizontalSizeDelta()const;
	bool GetLayoutControlVerticalSizeDelta()const;
};
