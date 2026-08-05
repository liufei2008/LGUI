// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexWidgetEditorHierarchyViewItem.h"

#include "ClassIconFinder.h"
#include "DetailLayoutBuilder.h"
#include "LexWidgetEditorHierarchyView.h"
#include "Widgets/Text/SInlineEditableTextBlock.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Styling/CoreStyle.h"
#include "LexUIPrefabEditor.h"
#include "ScopedTransaction.h"
#include "EditorFontGlyphs.h"
#include "Editor.h"
#include "LGUIEditorModule.h"
#include "LGUIEditorStyle.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexCanvas.h"
#include "Core/Components/LexVisual.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"

#define LOCTEXT_NAMESPACE "LexWidgetEditorHierarchyViewItem"

class FHierarchyLexWidgetDragDropOp : public FDecoratedDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FHierarchyLexWidgetDragDropOp, FDecoratedDragDropOp)

		virtual ~FHierarchyLexWidgetDragDropOp();

	virtual void OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent) override;

	struct FItem
	{
		/** The widget being dragged and dropped */
		ULexWidget* Widget = nullptr;

		/** The original parent of the widget. */
		ULexWidget* WidgetParent = nullptr;
	};

	TArray<FItem> DraggedWidgets;

	/** The widget being dragged and dropped */
	FScopedTransaction* Transaction;

	/** Constructs a new drag/drop operation */
	static TSharedRef<FHierarchyLexWidgetDragDropOp> New(const TArray<ULexWidget*>& InWidgets);
};

TSharedRef<FHierarchyLexWidgetDragDropOp> FHierarchyLexWidgetDragDropOp::New(const TArray<ULexWidget*>& InWidgets)
{
	check(InWidgets.Num() > 0);

	TSharedRef<FHierarchyLexWidgetDragDropOp> Operation = MakeShareable(new FHierarchyLexWidgetDragDropOp());

	// Set the display text and the transaction name based on whether we're dragging a single or multiple widgets
	if (InWidgets.Num() == 1)
	{
		Operation->CurrentHoverText = Operation->DefaultHoverText = FText::FromString(InWidgets[0]->GetDisplayName());
		Operation->Transaction = new FScopedTransaction(LOCTEXT("MoveWidget", "Change Hierarchy"));
	}
	else
	{
		Operation->CurrentHoverText = Operation->DefaultHoverText = LOCTEXT("DragMultipleWidgets", "Multiple Widgets");
		Operation->Transaction = new FScopedTransaction(LOCTEXT("MoveWidgets", "Change Hierarchy"));
	}

	// Add an FItem for each widget in the drag operation
	for (const auto& Widget : InWidgets)
	{
		FItem DraggedWidget;

		DraggedWidget.Widget = Widget;

		Widget->Modify();

		DraggedWidget.WidgetParent = Widget->GetParent();
		if (DraggedWidget.WidgetParent)
		{
			DraggedWidget.WidgetParent->Modify();
		}

		Operation->DraggedWidgets.Add(DraggedWidget);
	}

	Operation->Construct();

	return Operation;
}

FHierarchyLexWidgetDragDropOp::~FHierarchyLexWidgetDragDropOp()
{
	delete Transaction;
}

void FHierarchyLexWidgetDragDropOp::OnDrop(bool bDropWasHandled, const FPointerEvent& MouseEvent)
{
	if (!bDropWasHandled)
	{
		Transaction->Cancel();
	}
}



TOptional<EItemDropZone> ProcessHierarchyDragDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, bool bIsDrop, TSharedPtr<FLexUIPrefabEditor> Manager, ULexWidget* TargetItem, TOptional<int32> Index = TOptional<int32>())
{
	auto TargetTemplate = TargetItem;
	if (TargetTemplate && (DropZone == EItemDropZone::AboveItem || DropZone == EItemDropZone::BelowItem))
	{
		if (auto TargetParentTemplate = TargetTemplate->GetParent())
		{
			int32 InsertIndex = TargetTemplate->GetSiblingIndex();
			InsertIndex += (DropZone == EItemDropZone::AboveItem) ? 0 : 1;
			InsertIndex = FMath::Clamp(InsertIndex, 0, TargetParentTemplate->GetChildren().Num());

			TOptional<EItemDropZone> ParentZone = ProcessHierarchyDragDrop(DragDropEvent, EItemDropZone::OntoItem, bIsDrop, Manager, TargetParentTemplate, InsertIndex);
			if (ParentZone.IsSet())
			{
				return DropZone;
			}
			else
			{
				DropZone = EItemDropZone::OntoItem;
			}
		}
	}
	else
	{
		DropZone = EItemDropZone::OntoItem;
	}

	//drag/drop from content to create new widget
	TSharedPtr<FDragDropOperation> DragDropOp = DragDropEvent.GetOperation();
	if (DragDropOp.IsValid() && !DragDropOp->IsOfType<FHierarchyLexWidgetDragDropOp>())
	{
		if (bIsDrop)
		{
			if (DragDropOp->IsOfType<FAssetDragDropOp>() && Manager.IsValid())
			{
				Manager->TryHandleAssetDragDropOperation(DragDropEvent, TargetItem);
			}
		}
		return EItemDropZone::OntoItem;
	}

	TSharedPtr<FHierarchyLexWidgetDragDropOp> HierarchyDragDropOp = DragDropEvent.GetOperationAs<FHierarchyLexWidgetDragDropOp>();
	if (HierarchyDragDropOp.IsValid())
	{
		HierarchyDragDropOp->ResetToDefaultToolTip();

		// If the target item is valid we're dealing with a normal widget in the hierarchy, otherwise we should assume it's
		// the null case and we should be adding it as the root widget.
		if (TargetItem)
		{
			const bool bIsDraggedObject = HierarchyDragDropOp->DraggedWidgets.ContainsByPredicate([TargetItem](const FHierarchyLexWidgetDragDropOp::FItem& DraggedItem)
				{
					return DraggedItem.Widget == TargetItem;
				});
			const bool bIsChildOfDraggedObject = HierarchyDragDropOp->DraggedWidgets.ContainsByPredicate([TargetItem](const FHierarchyLexWidgetDragDropOp::FItem& DraggedItem)
				{
					return TargetItem->IsChildOf(DraggedItem.Widget);
				});

			if (bIsDraggedObject || bIsChildOfDraggedObject)
			{
				HierarchyDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
				return TOptional<EItemDropZone>();
			}

			auto* NewParent = TargetItem;

			if (bIsDrop)
			{
				if (Manager.IsValid())
				{
					Manager->GetPrefabHelperObject()->SetAnythingDirty();
				}
				NewParent->SetFlags(RF_Transactional);
				NewParent->Modify();

				for (const auto& DraggedWidget : HierarchyDragDropOp->DraggedWidgets)
				{
					auto TemplateWidget = DraggedWidget.Widget;
					TemplateWidget->SetFlags(RF_Transactional);
					TemplateWidget->Modify();

					if (Index.IsSet())
					{
						// If we're inserting at an index, and the widget we're moving is already
						// in the hierarchy before the point we're moving it to, we need to reduce the index
						// count by one, because the whole set is about to be shifted when it's removed.
						const bool bInsertInSameParent = TemplateWidget->GetParent() == NewParent;
						const bool bNeedToDropIndex = TemplateWidget->GetSiblingIndex() < Index.GetValue();

						if (bInsertInSameParent && bNeedToDropIndex)
						{
							Index = Index.GetValue() - 1;
						}
					}

					TemplateWidget->SetParent(nullptr, true);

					if (Index.IsSet())
					{
						TemplateWidget->SetParent(NewParent, true, Index.GetValue());
					}
					else
					{
						TemplateWidget->SetParent(NewParent, true);
					}
					FLexUIUtils::NotifyPropertyChanged(TemplateWidget, ULexWidget::GetPropertyName_SiblingIndex());
				}
			}

			HierarchyDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"));
			return EItemDropZone::OntoItem;
		}
		else
		{
			HierarchyDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
			HierarchyDragDropOp->CurrentHoverText = LOCTEXT("CantHaveChildren", "Widget can't have children.");
		}

		return TOptional<EItemDropZone>();
	}

	return TOptional<EItemDropZone>();
}


void SLexWidgetEditorHierarchyViewItem::Construct(const FArguments& InArgs, const TSharedRef<STableViewBase>& InOwnerTableView, TWeakObjectPtr<ULexWidget> InModel, TSharedPtr<SLexWidgetEditorHierarchyView> InHierarchyView, TSharedPtr<FLexUIPrefabEditor> InManager)
{
	Widget = InModel;
	MouseEnter = InArgs._MouseEnter;
	MouseExit = InArgs._MouseExit;
	HierarchyView = InHierarchyView;
	Manager = InManager;
	auto PrefabHelperObject = InManager.IsValid() ? InManager->GetPrefabHelperObject() : nullptr;

	STableRow::Construct(
		STableRow::FArguments()
		.OnCanAcceptDrop(this, &SLexWidgetEditorHierarchyViewItem::HandleCanAcceptDrop)
		.OnAcceptDrop(this, &SLexWidgetEditorHierarchyViewItem::HandleAcceptDrop)
		.OnDragDetected(this, &SLexWidgetEditorHierarchyViewItem::HandleDragDetected)
		.OnDragEnter(this, &SLexWidgetEditorHierarchyViewItem::HandleDragEnter)
		.OnDragLeave(this, &SLexWidgetEditorHierarchyViewItem::HandleDragLeave)
		.Padding(FMargin(0, 2))
		.Content()
		[
			SNew(SHorizontalBox)

			// Widget icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image_Lambda([=, this]()
				{
					if (Widget.IsValid())
					{
						if (Widget->GetVisual())
							return FSlateIconFinder::FindIconBrushForClass(Widget->GetVisual()->GetClass());
						return FSlateIconFinder::FindIconBrushForClass(ULexWidget::StaticClass());
					}
					return (const FSlateBrush*)nullptr;
				})
			]
			// Interaction icon
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image_Lambda([=, this]()
				{
					return FLGUIEditorModule::Get().GetInteractionIconBrush(Widget.Get());
				})
			]

			// Canvas
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0)
			[
				SNew(SBox)
				.Visibility_Lambda([=, this]()
				{
					if (Widget.IsValid() && Widget->IsCanvasWidget())
					{
						return EVisibility::Visible;
					}
					return EVisibility::Collapsed;
				})
				[
					SNew(SOverlay)
					+SOverlay::Slot()//canvas icon
					[
						SNew(SBox)
						.WidthOverride(16)
						.HeightOverride(16)
						.Padding(FMargin(0))
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(SImage)
							.Image(FLGUIEditorStyle::Get().GetBrush("CanvasMark"))
							.Visibility_Lambda([=, this]()
							{
								if (Widget->IsCanvasWidget())
								{
									return EVisibility::Visible;
								}
								return EVisibility::Hidden;
							})
							.ColorAndOpacity(FLinearColor(1.0f, 1.0f, 1.0f, 0.4f))
							.ToolTipText(LOCTEXT("CanvasMarkTip", "This is canvas widget. The number is the draw-call count of this canvas."))
						]
					]
					+SOverlay::Slot()//draw-call count
					[
						SNew(SBox)
						.WidthOverride(16)
						.HeightOverride(16)
						.Padding(FMargin(0))
						.HAlign(EHorizontalAlignment::HAlign_Left)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.ShadowColorAndOpacity(FLinearColor::Black)
							.ShadowOffset(FVector2D(1, 1))
							.Text_Lambda([=, this]()
							{
								int DrawCallCount = 0;
								if (Widget.IsValid() && Widget->IsCanvasWidget() && Widget->GetRenderCanvas())
								{
									 DrawCallCount = Widget->GetRenderCanvas()->GetDrawCallCount();
								}
								return FText::FromString(FString::Printf(TEXT("%d"), DrawCallCount));
							})
							.ColorAndOpacity(FSlateColor(FLinearColor(FColor::Green)))
							.Visibility_Lambda([=, this]()
							{
								if (Widget.IsValid() && Widget->IsCanvasWidget())
								{
									return EVisibility::Visible;
								}
								return EVisibility::Hidden;
							})
							.ToolTipText(LOCTEXT("DrawCallCountTip", "The number is the draw-call count generated by this LexCanvas."))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
				]
			]			

			// Name of the widget
			+SHorizontalBox::Slot()
			.FillWidth(1.0f)
			.Padding(2, 0, 0, 0)
			.VAlign(VAlign_Center)
			[
				SAssignNew(EditBox, SInlineEditableTextBlock)
				//.Font(this, &SHierarchyViewItem::GetItemFont)
				.Text(this, &SLexWidgetEditorHierarchyViewItem::GetItemText)
				.ToolTipText(this, &SLexWidgetEditorHierarchyViewItem::GetItemTooltipText)
				.ColorAndOpacity(this, &SLexWidgetEditorHierarchyViewItem::GetNameTextColorAndOpacity)
				.IsReadOnly(this, &SLexWidgetEditorHierarchyViewItem::IsReadOnly)
				.OnEnterEditingMode(this, &SLexWidgetEditorHierarchyViewItem::OnBeginNameTextEdit)
				.OnExitEditingMode(this, &SLexWidgetEditorHierarchyViewItem::OnEndNameTextEdit)
				.OnVerifyTextChanged(this, &SLexWidgetEditorHierarchyViewItem::OnVerifyNameTextChanged)
				.OnTextCommitted(this, &SLexWidgetEditorHierarchyViewItem::OnNameTextCommited)
				.IsSelected(this, &SLexWidgetEditorHierarchyViewItem::IsSelectedExclusively)				
			]

			// SubPrefab
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SOverlay)
				+SOverlay::Slot()
				[
					SNew(SImage)
					.Image_Lambda([=, this]()
					{
						if (PrefabHelperObject)
						{
							if (!PrefabHelperObject->IsWidgetBelongsToSubPrefab(Widget.Get()))//is sub prefab
							{
								if (PrefabHelperObject->IsWidgetBelongsToMissingSubPrefab(Widget.Get()))
								{
									return FLGUIEditorStyle::Get().GetBrush("PrefabMarkBroken");
								}
							}
							else
							{
								if (PrefabHelperObject->GetSubPrefabAsset(Widget.Get())->GetIsPrefabVariant())
								{
									return FLGUIEditorStyle::Get().GetBrush("PrefabVariantMarkWhite");
								}
							}
						}
						return FLGUIEditorStyle::Get().GetBrush("PrefabMarkWhite");
					})
					.ColorAndOpacity_Lambda([=, this]()
					{
						if (PrefabHelperObject)
						{
							if (PrefabHelperObject->IsWidgetBelongsToSubPrefab(Widget.Get()))//is sub prefab
							{
								return FSlateColor(PrefabHelperObject->GetSubPrefabData(Widget.Get()).EditorIdentifyColor);
							}
							else
							{
								if (PrefabHelperObject->IsWidgetBelongsToMissingSubPrefab(Widget.Get()))
								{
									return FSlateColor(FColor::White);
								}
							}
						}
						return FSlateColor(FColor::Green);
					})
					.Visibility_Lambda([=, this]()
					{
						if (PrefabHelperObject)
						{
							if (PrefabHelperObject->IsWidgetBelongsToSubPrefab(Widget.Get()))//is sub prefab
							{
								return EVisibility::Visible;
							}
							else
							{
								if (PrefabHelperObject->IsWidgetBelongsToMissingSubPrefab(Widget.Get()))
								{
									return EVisibility::Visible;
								}
								else
								{
									return EVisibility::Hidden;
								}
							}
						}
						return EVisibility::Hidden;
					})
					.ToolTipText_Lambda([=, this]()
					{
						if (PrefabHelperObject)
						{
							if (!PrefabHelperObject->IsWidgetBelongsToSubPrefab(Widget.Get()))//is sub prefab
							{
								if (PrefabHelperObject->IsWidgetBelongsToMissingSubPrefab(Widget.Get()))
								{
									return LOCTEXT("PrefabMarkBrokenTip", "This widget was part of another prefab, but the prefab asset is missing!");
								}
							}
						}
						return LOCTEXT("PrefabMarkWhiteTip", "This widget belongs to another prefab.");
					})
				]
			]

			// Visibility
			+SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			[
				SNew(SButton)
				.ContentPadding(FMargin(3, 1))
				.ButtonStyle(FAppStyle::Get(), "HoverHintOnly")
				.ForegroundColor(FCoreStyle::Get().GetSlateColor("Foreground"))
				.OnClicked(this, &SLexWidgetEditorHierarchyViewItem::OnToggleVisibility)
				.ToolTipText(LOCTEXT("WidgetVisibilityButtonToolTip", "Toggle Widget's Editor Visibility"))
				.HAlign(HAlign_Center)
				.VAlign(VAlign_Center)
				[
					SNew(STextBlock)
					.Font(FAppStyle::Get().GetFontStyle("FontAwesome.10"))
					.Text(this, &SLexWidgetEditorHierarchyViewItem::GetVisibilityBrushForWidget)
					.ColorAndOpacity(this, &SLexWidgetEditorHierarchyViewItem::GetVisibilityIconColorAndOpacity)
				]
			]
		],
		InOwnerTableView);
}

void SLexWidgetEditorHierarchyViewItem::OnMouseEnter(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	MouseEnter.ExecuteIfBound();
	STableRow::OnMouseEnter(MyGeometry, MouseEvent);
}
void SLexWidgetEditorHierarchyViewItem::OnMouseLeave(const FPointerEvent& MouseEvent)
{
	MouseExit.ExecuteIfBound();
	STableRow::OnMouseLeave(MouseEvent);
}
void SLexWidgetEditorHierarchyViewItem::RequestEditName()
{
	EditBox->EnterEditingMode();
}
bool SLexWidgetEditorHierarchyViewItem::CanRename()
{
	return true;
}

TOptional<EItemDropZone> SLexWidgetEditorHierarchyViewItem::HandleCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, TWeakObjectPtr<ULexWidget> TargetItem)
{
	TSharedPtr<FDragDropOperation> DragDropOp = DragDropEvent.GetOperation();
	if (DragDropOp.IsValid() && DragDropOp->IsOfType<FAssetDragDropOp>())
	{
		auto AssetDragDropOp = StaticCastSharedPtr<FAssetDragDropOp>(DragDropOp);
		if (AssetDragDropOp->GetAssets().Num() > 0)
		{
			auto EditingPrefab = Manager.Pin()->GetPrefabBeingEdited();
			TOptional<EItemDropZone> ValidDropZone;
			for (auto AssetData : AssetDragDropOp->GetAssets())
			{
				if (AssetData.AssetClassPath == ULexUIPrefab::StaticClass()->GetClassPathName())
				{
					if (AssetData.GetAsset()->GetPathName() == EditingPrefab->GetPathName())
					{
						AssetDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
						AssetDragDropOp->CurrentHoverText = LOCTEXT("CantDropPrefabToItself", "Can't drop prefab to itself.");
						return TOptional<EItemDropZone>();
					}
					if (Widget == Widget->GetRootWidgetInHierarchy())
					{
						AssetDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
						AssetDragDropOp->CurrentHoverText = LOCTEXT("CantDropPrefabToRootAgent", "Can't drop prefab to root agent.");
						return TOptional<EItemDropZone>();
					}
					AssetDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.OK"));
					AssetDragDropOp->CurrentHoverText = FText::GetEmpty();
					ValidDropZone = EItemDropZone::OntoItem;
				}
			}
			return ValidDropZone;
		}
	}

	if (DragDropOp.IsValid() && DragDropOp->IsOfType<FHierarchyLexWidgetDragDropOp>())
	{
		const bool bIsDrop = false;
		auto HierarchyDragDropOp = StaticCastSharedPtr<FHierarchyLexWidgetDragDropOp>(DragDropOp);
		if (HierarchyDragDropOp->DraggedWidgets.Num() > 0)
		{
			for (auto DraggedWidget : HierarchyDragDropOp->DraggedWidgets)
			{
				if (SupportDrop(DraggedWidget.Widget, Widget.Get(), DropZone))
				{
					return ProcessHierarchyDragDrop(DragDropEvent, DropZone, bIsDrop, Manager.Pin(), Widget.Get());
				}
				else
				{
					HierarchyDragDropOp->CurrentIconBrush = FAppStyle::GetBrush(TEXT("Graph.ConnectorFeedback.Error"));
					HierarchyDragDropOp->CurrentHoverText = LOCTEXT("CantDropWidgetHere", "Can't drop widget here.");
					return TOptional<EItemDropZone>();
				}
			}
			return TOptional<EItemDropZone>();
		}
	}
	return TOptional<EItemDropZone>();
}
FReply SLexWidgetEditorHierarchyViewItem::HandleAcceptDrop(FDragDropEvent const& DragDropEvent, EItemDropZone DropZone, TWeakObjectPtr<ULexWidget> TargetItem)
{
	const bool bIsDrop = true;
	TOptional<EItemDropZone> Zone = ProcessHierarchyDragDrop(DragDropEvent, DropZone, bIsDrop, Manager.Pin(), Widget.Get());
	if (Zone.IsSet())
	{
		HierarchyView.Pin()->RequestRefresh();
		return FReply::Handled();
	}
	else
		return FReply::Unhandled();
}
FReply SLexWidgetEditorHierarchyViewItem::HandleDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent)
{
	TArray<ULexWidget*> DraggedItems;

	// Dragging multiple items?
	if (auto Selection = ULexUISelection::GetInstance(Widget->GetWorld()))
	{
		if (Selection->GetSelectedWidgets().Num() > 1 && Selection->GetSelectedWidgets().Contains(Widget.Get()))
		{
			for (auto Selected : Selection->GetSelectedWidgets())
			{
				DraggedItems.Add(Selected.Get());
			}
		}
	}

	if (DraggedItems.Num() == 0)
	{
		DraggedItems.Add(Widget.Get());
	}

	if (DraggedItems.Num() > 0)
	{
		bool bAllCanDrag = true;
		if (auto PrefabHelperObject = ULexUIPrefabHelperObject::GetPrefabHelperObject_WhichManageThisWidget(Widget.Get()))
		{
			for (auto Item : DraggedItems)
			{
				if (PrefabHelperObject->IsWidgetBelongsToSubPrefab(Item) && !PrefabHelperObject->IsSubPrefabRootWidget(Item))
				{
					bAllCanDrag = false;
					break;
				}
			}
		}
		if (bAllCanDrag)
		{
			return FReply::Handled().BeginDragDrop(FHierarchyLexWidgetDragDropOp::New(DraggedItems));
		}
	}

	return FReply::Handled();
}
void SLexWidgetEditorHierarchyViewItem::HandleDragEnter(FDragDropEvent const& DragDropEvent)
{
	//UE_LOG(LogTemp, Log, TEXT("HandleDragEnter, %s"), *Model->GetName());
}
void SLexWidgetEditorHierarchyViewItem::HandleDragLeave(const FDragDropEvent& DragDropEvent)
{
	//UE_LOG(LogTemp, Log, TEXT("HandleDragLeave, %s"), *Model->GetName());
}

FText SLexWidgetEditorHierarchyViewItem::GetItemText() const
{
	return Widget.IsValid() ? FText::FromString(Widget->GetDisplayName()) : FText::GetEmpty();
}

FText SLexWidgetEditorHierarchyViewItem::GetItemTooltipText() const
{
	return Widget.IsValid() ? FText::Format(LOCTEXT("ItemTooltipFormat", "Path name: {0}"), FText::FromString(Widget->GetPathName()))
		: FText::GetEmpty();
}

FSlateColor SLexWidgetEditorHierarchyViewItem::GetNameTextColorAndOpacity() const
{
	if (Widget.IsValid())
	{
		if (auto PrefabHelperObject = ULexUIPrefabHelperObject::GetPrefabHelperObject_WhichManageThisWidget(Widget.Get()))
		{
			if (PrefabHelperObject->IsWidgetBelongsToSubPrefab(Widget.Get()))//is sub prefab
			{
				if (Widget->GetWidgetActiveInHierarchy())
				{
					return FLinearColor(FColor(124,171,240, 255));
				}
				return FLinearColor(FColor(124,171,240, 128));
			}
			else
			{
				if (PrefabHelperObject->IsWidgetBelongsToMissingSubPrefab(Widget.Get()))
				{
					if (Widget->GetWidgetActiveInHierarchy())
					{
						return FSlateColor(FColor::Red);
					}
					return FSlateColor(FColor(255, 0, 0, 128));
				}
			}
		}
		if (Widget->GetWidgetActiveInHierarchy())
		{
			return FSlateColor(FColor(192,192,192,255));
		}
		return FSlateColor(FColor(192,192,192,128));
	}
	return FSlateColor(FColor(192,192,192,128));
}

FSlateColor SLexWidgetEditorHierarchyViewItem::GetVisibilityIconColorAndOpacity() const
{
	auto NameTextColorAndOpacity = GetNameTextColorAndOpacity();
	auto Alpha = NameTextColorAndOpacity.GetSpecifiedColor().A * 255;
	NameTextColorAndOpacity = FSlateColor(FColor(255,255,255,(uint8)Alpha));
	return NameTextColorAndOpacity;
}

bool SLexWidgetEditorHierarchyViewItem::IsReadOnly() const
{
	return false;
}
void SLexWidgetEditorHierarchyViewItem::OnBeginNameTextEdit()
{
	InitialText = FText::FromString(Widget->GetDisplayName());
}
void SLexWidgetEditorHierarchyViewItem::OnEndNameTextEdit()
{

}
bool SLexWidgetEditorHierarchyViewItem::OnVerifyNameTextChanged(const FText& InText, FText& OutErrorMessage)
{
	return true;
}
void SLexWidgetEditorHierarchyViewItem::OnNameTextCommited(const FText& InText, ETextCommit::Type CommitInfo)
{
	// The model can return nice names "Border_53" becomes [Border] in some cases
	// This check makes sure we don't rename the object internally to that nice name.
	// Most common case would be the user enters edit mode by accident then just moves focus away.
	if (InitialText.EqualToCaseIgnored(InText))
	{
		return;
	}

	GEditor->BeginTransaction(LOCTEXT("ChangeWidgetName_Transaction", "Change Name"));
	Widget->Modify();
	FLexUIUtils::ChangePropertyWithNotify(Widget.Get(), ULexWidget::GetPropertyName_DisplayName(), [=, this]()
	{
		Widget->SetDisplayName(InText.ToString());
	});
	GEditor->EndTransaction();

	HierarchyView.Pin()->RequestRefresh();
}
FReply SLexWidgetEditorHierarchyViewItem::OnToggleVisibility()
{
	GEditor->BeginTransaction(LOCTEXT("ToggleWidgetVisibility_Transaction", "Toggle Visibility"));
	Widget->Modify();
	FLexUIUtils::ChangePropertyWithNotify(Widget.Get(), ULexWidget::GetPropertyName_WidgetActive(), [=, this]()
	{
		Widget->SetWidgetActive(!Widget->GetWidgetActive());
	});
	GEditor->EndTransaction();

	return FReply::Handled();
}
FText SLexWidgetEditorHierarchyViewItem::GetVisibilityBrushForWidget() const
{
	return Widget.IsValid() && Widget->GetWidgetActive() ? FEditorFontGlyphs::Eye : FEditorFontGlyphs::Eye_Slash;
}

bool SLexWidgetEditorHierarchyViewItem::SupportDrop(ULexWidget* Dragging, ULexWidget* Current, EItemDropZone DropZone)
{
	if (Current == Current->GetRootWidgetInHierarchy())
	{
		if (ULexUIPrefabHelperObject::GetPrefabHelperObject_WhichManageThisWidget(Current))//contains prefab-helper-object, means it is a prefab editor mode
		{
			return false;//editor world's root widget can't be dropped
		}
		if (DropZone == EItemDropZone::OntoItem)
		{
			return true;
		}
		return false;
	}
	if (Current->IsChildOf(Dragging))
	{
		return false;
	}
	return true;
}

#undef LOCTEXT_NAMESPACE
