
#pragma once

#include "Widgets/SCompoundWidget.h"

class FLGUIPrefabEditor;

// LGUI编辑器动画列表
class SLGUIAnimationListUI : public SCompoundWidget
{
public:
	SLATE_BEGIN_ARGS( SLGUIAnimationListUI	) {}
	SLATE_END_ARGS()

	void Construct( const FArguments& InArgs, FLGUIPrefabEditor* InMainUI);
	
	
	virtual FReply OnKeyDown( const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent ) override;
	virtual void Tick(const FGeometry& AllottedGeometry, const double InCurrentTime, const float InDeltaTime) override;

	void UpdateAnimationList();

	// ??
	void RefreshAnimationListUI();

	void OnItemScrolledIntoView( TSharedPtr<struct FLGUIAnimationListItem> InListItem, const TSharedPtr<ITableRow>& InWidget ) const;

	FReply OnNewAnimationClicked();

	void OnSearchChanged( const FText& InSearchText );

	void OnSelectionChanged( TSharedPtr<FLGUIAnimationListItem> InSelectedItem, ESelectInfo::Type SelectionInfo );

	TSharedPtr<SWidget> OnContextMenuOpening() const;

	TSharedRef<ITableRow> OnGenerateWidgetForMovieScene( TSharedPtr<FLGUIAnimationListItem> InListItem, const TSharedRef< STableViewBase >& InOwnerTableView );

	void CreateCommandList();

	bool CanExecuteContextMenuAction() const;

	void OnDuplicateAnimation();

	void OnDeleteAnimation();

	void OnRenameAnimation();
	
private:
	typedef SListView<TSharedPtr<FLGUIAnimationListItem>> SLGUIAnimationListView;

	TSharedPtr<FUICommandList> CommandList;
	FLGUIPrefabEditor* MainUI;
	TSharedPtr<SLGUIAnimationListView> AnimationListView;

	// UI展示的Animation列表
	TArray< TSharedPtr<FLGUIAnimationListItem> > Animations;
	TSharedPtr<class SSearchBox> SearchBoxPtr;
	double NextUpdateUITime = 0;
};