// Copyright 2019-2022 LexLiu. All Rights Reserved.
#include "CoreMinimal.h"
#include "IDetailCustomization.h"
#pragma once

/**
 * 
 */
class FLGUICanvasCustomization : public IDetailCustomization
{
public:
	FLGUICanvasCustomization();
	~FLGUICanvasCustomization();

	static TSharedRef<IDetailCustomization> MakeInstance();
	/** IDetailCustomization interface */
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;
private:
	TArray<TWeakObjectPtr<class ULGUICanvas>> TargetScriptArray;
	void ForceRefresh(class IDetailLayoutBuilder* DetailBuilder);
	FText GetSortOrderInfo(TWeakObjectPtr<class ULGUICanvas> InTargetScript)const;
	FText GetDrawcallInfo()const;
	FText GetDrawcallInfoTooltip()const;
	void OnCopySortOrder();
	void OnPasteSortOrder(TSharedRef<class IPropertyHandle> PropertyHandle);
	FReply OnClickFixClipTextureSetting(TSharedRef<IPropertyHandle> ClipTextureHandle);
	bool IsFixClipTextureEnabled(TSharedRef<IPropertyHandle> ClipTextureHandle)const;
};
