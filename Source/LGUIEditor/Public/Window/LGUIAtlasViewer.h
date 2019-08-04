// Copyright 2019 LexLiu. All Rights Reserved.

#include "Widgets/SCompoundWidget.h"
#include "Widgets/Docking/SDockTab.h"
#include "Core/LGUISpriteData.h"
#include "Widgets/Views/STileView.h"
#include "Widgets/Views/STableViewBase.h"
#pragma once


/**
* for check packed texture atlas
*/
class FLGUIPackedAtlasTextureViewerItem
{
public:
	FLGUIPackedAtlasTextureViewerItem(UTexture2D* InTexture, FName InPackingTag)
	{
		this->Texture = InTexture;
		this->PackingTag = InPackingTag;
	}

	UTexture2D* Texture;
	FName PackingTag;
};

/**
 * 
 */
class SLGUIAtlasViewer : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS(SLGUIAtlasViewer) {}
	
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs, TSharedPtr<SDockTab> InOwnerTab);

private:
	TSharedPtr<STileView<TSharedPtr<FLGUIPackedAtlasTextureViewerItem>>> ContentSourceTileView;
	TArray<TSharedPtr<FLGUIPackedAtlasTextureViewerItem>> Items;
	TSharedPtr<SDockTab> OwnerTab;
	TArray<TSharedPtr<FSlateBrush>> SpriteSlateBrushArray;

	TSharedRef<ITableRow> CreateContentIconTitle(TSharedPtr<FLGUIPackedAtlasTextureViewerItem> ContentSource, const TSharedRef<STableViewBase>& OwnerTable);
	//double click to opend texture
	void OnMouseButtonDoubleClicked(TSharedPtr<FLGUIPackedAtlasTextureViewerItem> Item);

	void CloseTabCallback(TSharedRef<SDockTab> TabClosed);
	const int ThumbnailSize = 256;
};
