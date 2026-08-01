// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditorDetails.h"
#include "ClassViewerFilter.h"
#include "ClassViewerModule.h"
#include "Modules/ModuleManager.h"
#include "UnrealEdGlobals.h"
#include "Editor/UnrealEdEngine.h"
#include "Framework/Application/SlateApplication.h"
#include "Misc/NotifyHook.h"
#include "LexUIPrefabEditor.h"
#include "DetailLayoutBuilder.h"
#include "LexWidgetDetailPropertyExtensionHandler.h"
#include "LexUIPrefabOverrideDataViewer.h"
#include "PrefabSystem/LexUIPrefab.h"
#include "LexUIEditorTools.h"
#include "Core/LexUIBehaviour.h"
#include "Core/LexUIManager.h"
#include "Core/Components/LexWidget.h"
#include "ScopedTransaction.h"
#include "SPositiveActionButton.h"
#include "Framework/Commands/GenericCommands.h"
#include "Styling/SlateIconFinder.h"
#include "Utils/LexUIUtils.h"
#include "SourceCodeNavigation.h"
#include "PrefabAnimation/LexUIDetailKeyframeHandler.h"
#include "AssetSelection.h"
#include "AssetRegistry/AssetData.h"
#include "DragAndDrop/AssetDragDropOp.h"
#include "DragAndDrop/DecoratedDragDropOp.h"
#include "Engine/Blueprint.h"
#include "Engine/Engine.h"
#include "PrefabSystem/LexUIPrefabHelperObject.h"
#include "Widgets/Input/SButton.h"
#include "Widgets/Input/SComboButton.h"
#include "Widgets/Layout/SBorder.h"
#include "Widgets/Views/SListView.h"
#include "Widgets/Views/STableRow.h"

#define LOCTEXT_NAMESPACE "LGUIPrefabEditorDetailTab"

using FLexWidgetComponentItem = TWeakObjectPtr<ULexUIBehaviour>;

class FLexWidgetComponentDragDropOp : public FDecoratedDragDropOp
{
public:
	DRAG_DROP_OPERATOR_TYPE(FLexWidgetComponentDragDropOp, FDecoratedDragDropOp)

	static TSharedRef<FLexWidgetComponentDragDropOp> New(const FLexWidgetComponentItem& InDraggedItem)
	{
		TSharedRef<FLexWidgetComponentDragDropOp> Operation = MakeShared<FLexWidgetComponentDragDropOp>();
		Operation->DraggedItem = InDraggedItem;
		Operation->SetToolTip(LOCTEXT("ReorderComponent", "Reorder component"), nullptr);
		Operation->SetupDefaults();
		Operation->Construct();
		return Operation;
	}

	FLexWidgetComponentItem DraggedItem;
};

class FLexWidgetComponentClassFilter : public IClassViewerFilter
{
public:
	virtual bool IsClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const UClass* InClass, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return IsComponentClassAllowed(InClass);
	}

	virtual bool IsUnloadedClassAllowed(const FClassViewerInitializationOptions& InInitOptions, const TSharedRef<const IUnloadedBlueprintData> InUnloadedClassData, TSharedRef<FClassViewerFilterFuncs> InFilterFuncs) override
	{
		return InUnloadedClassData->IsChildOf(ULexUIBehaviour::StaticClass())
			&& !InUnloadedClassData->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Hidden);
	}

	static bool IsComponentClassAllowed(const UClass* InClass)
	{
		if (InClass == nullptr)return false;
		if (!InClass->IsChildOf(ULexUIBehaviour::StaticClass()))return false;
		if (InClass->HasAnyClassFlags(CLASS_Abstract | CLASS_Deprecated | CLASS_NewerVersionExists | CLASS_Hidden | CLASS_Transient))return false;
		if (InClass->HasAnyClassFlags(CLASS_CompiledFromBlueprint) || !InClass->HasAnyClassFlags(CLASS_Native))//blueprint class
		{
			return true;
		}
		if (!InClass->HasMetaData("BlueprintSpawnableComponent"))return false;
		return true;
	}
};

class SLexWidgetComponentEditor : public SCompoundWidget
{
public:
	DECLARE_DELEGATE_RetVal(ULexWidget*, FOnGetWidgetContext);
	DECLARE_DELEGATE_RetVal(bool, FOnCanEdit);
	DECLARE_DELEGATE_OneParam(FOnComponentsSelectionChanged, const TArray<FLexWidgetComponentItem>&);

	SLATE_BEGIN_ARGS(SLexWidgetComponentEditor)
	{
	}
		SLATE_EVENT(FOnGetWidgetContext, GetWidgetContext)
		SLATE_EVENT(FOnCanEdit, CanEdit)
		SLATE_EVENT(FOnComponentsSelectionChanged, OnSelectionChanged)
	SLATE_END_ARGS()

	void Construct(const FArguments& InArgs)
	{
		CommandList = MakeShareable(new FUICommandList);
		CommandList->MapAction(
			FGenericCommands::Get().Delete,
			FExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::HandleRemoveSelectedComponents),
			FCanExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::CanRemoveSelectedComponents)
		);
		CommandList->MapAction(
			FGenericCommands::Get().Copy,
			FExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::HandleCopySelectedComponent),
			FCanExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::CanCopySelectedComponent)
		);
		CommandList->MapAction(
			FGenericCommands::Get().Paste,
			FExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::HandlePasteComponent),
			FCanExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::CanPasteComponent)
		);
		
		GetWidgetContext = InArgs._GetWidgetContext;
		CanEdit = InArgs._CanEdit;
		OnSelectionChanged = InArgs._OnSelectionChanged;

		ToolbarWidget =
		SNew(SBox)
		.Padding(FMargin(4, 2, 4, 2))
		[
			SNew(SPositiveActionButton)
			.IsEnabled(this, &SLexWidgetComponentEditor::CanAddOrRemoveComponent)
			.Icon(FAppStyle::Get().GetBrush("Icons.Plus"))
			.Text(LOCTEXT("AddWidgetComponent", "Add Component"))
			.ToolTipText(LOCTEXT("AddWidgetComponentTooltip", "Add a LexUI component to the selected widget"))
			.OnGetMenuContent(this, &SLexWidgetComponentEditor::GenerateAddComponentMenu)
		]
		;

		ChildSlot
		[
			SNew(SOverlay)
			+ SOverlay::Slot()
			[
				SNew(SBorder)
				.Padding(FMargin(2))
				[
					SAssignNew(ComponentListView, SListView<FLexWidgetComponentItem>)
					.ListItemsSource(&ComponentItems)
					.SelectionMode(ESelectionMode::Single)
					.OnGenerateRow(this, &SLexWidgetComponentEditor::HandleGenerateRow)
					.OnSelectionChanged(this, &SLexWidgetComponentEditor::HandleSelectionChanged)
					.OnContextMenuOpening(this, &SLexWidgetComponentEditor::OnContextMenuOpening)
				]
			]
			+ SOverlay::Slot()
			.HAlign(HAlign_Center)
			.VAlign(VAlign_Center)
			[
				SNew(STextBlock)
				.Visibility(this, &SLexWidgetComponentEditor::GetEmptyStateVisibility)
				.Text(this, &SLexWidgetComponentEditor::GetEmptyStateText)
				.Font(IDetailLayoutBuilder::GetDetailFont())
			]
		];

		RefreshComponents();
	}

	virtual bool SupportsKeyboardFocus() const override
	{
		return true;
	}

	virtual FReply OnFocusReceived(const FGeometry& MyGeometry, const FFocusEvent& InFocusEvent) override
	{
		if (ComponentListView.IsValid())
		{
			FSlateApplication::Get().SetKeyboardFocus(ComponentListView, EFocusCause::SetDirectly);
			return FReply::Handled();
		}

		return SCompoundWidget::OnFocusReceived(MyGeometry, InFocusEvent);
	}

	virtual FReply OnKeyDown(const FGeometry& MyGeometry, const FKeyEvent& InKeyEvent) override
	{
		if (CommandList.IsValid() && CommandList->ProcessCommandBindings(InKeyEvent))
		{
			return FReply::Handled();
		}

		return SCompoundWidget::OnKeyDown(MyGeometry, InKeyEvent);
	}

	virtual void OnDragEnter(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		SCompoundWidget::OnDragEnter(MyGeometry, DragDropEvent);
	}

	virtual FReply OnDragOver(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		if (CanHandleAssetDrop(DragDropEvent))
		{
			if (TSharedPtr<FAssetDragDropOp> AssetDragOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>())
			{
				AssetDragOp->SetToolTip(LOCTEXT("DropToAddComponent", "Drop to add component"), AssetDragOp->CurrentIconBrush);
			}
			return FReply::Handled();
		}
		if (TSharedPtr<FAssetDragDropOp> AssetDragOp = DragDropEvent.GetOperationAs<FAssetDragDropOp>())
		{
			AssetDragOp->ResetToDefaultToolTip();
		}
		return SCompoundWidget::OnDragOver(MyGeometry, DragDropEvent);
	}

	virtual FReply OnDrop(const FGeometry& MyGeometry, const FDragDropEvent& DragDropEvent) override
	{
		if (HandleAddComponentsFromAssetDrop(DragDropEvent))
		{
			return FReply::Handled();
		}
		return SCompoundWidget::OnDrop(MyGeometry, DragDropEvent);
	}

	TSharedRef<SWidget> GetToolbarWidget() const
	{
		return ToolbarWidget.ToSharedRef();
	}
	void RefreshComponents()
	{
		ComponentItems.Reset();

		if (const ULexWidget* Widget = GetCurrentWidget())
		{
			for (ULexUIBehaviour* Component : Widget->GetAllComponents())
			{
				if (IsValid(Component))
				{
					ComponentItems.Add(Component);
				}
			}
		}

		if (ComponentListView.IsValid())
		{
			ComponentListView->RequestListRefresh();
		}
	}

	void ClearSelection()
	{
		if (ComponentListView.IsValid())
		{
			ComponentListView->ClearSelection();
		}
	}

	void SelectComponent(ULexUIBehaviour* InComponent)
	{
		if (!ComponentListView.IsValid() || !IsValid(InComponent))
		{
			return;
		}

		const FLexWidgetComponentItem Item = InComponent;
		ComponentListView->SetSelection(Item, ESelectInfo::Direct);
		ComponentListView->RequestScrollIntoView(Item);
	}

private:
	ULexWidget* GetCurrentWidget() const
	{
		return GetWidgetContext.IsBound() ? GetWidgetContext.Execute() : nullptr;
	}

	bool CanAddOrRemoveComponent() const
	{
		return IsValid(GetCurrentWidget()) && (!CanEdit.IsBound() || CanEdit.Execute());
	}

	bool CanRemoveSelectedComponents() const
	{
		if (!CanAddOrRemoveComponent() || !ComponentListView.IsValid())
		{
			return false;
		}

		TArray<FLexWidgetComponentItem> SelectedItems;
		ComponentListView->GetSelectedItems(SelectedItems);
		return SelectedItems.Num() > 0;
	}

	FReply HandleDragDetected(const FGeometry& MyGeometry, const FPointerEvent& MouseEvent, FLexWidgetComponentItem InItem)
	{
		if (!InItem.IsValid() || !CanAddOrRemoveComponent())
		{
			return FReply::Unhandled();
		}

		return FReply::Handled().BeginDragDrop(FLexWidgetComponentDragDropOp::New(InItem));
	}

	TOptional<EItemDropZone> HandleCanAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, FLexWidgetComponentItem TargetItem) const
	{
		if (!CanAddOrRemoveComponent() || !TargetItem.IsValid())
		{
			return TOptional<EItemDropZone>();
		}

		if (TSharedPtr<FLexWidgetComponentDragDropOp> DragOp = DragDropEvent.GetOperationAs<FLexWidgetComponentDragDropOp>())
		{
			if (!DragOp->DraggedItem.IsValid() || DragOp->DraggedItem == TargetItem)
			{
				return TOptional<EItemDropZone>();
			}
			return DropZone;
		}

		return TOptional<EItemDropZone>();
	}

	FReply HandleAcceptDrop(const FDragDropEvent& DragDropEvent, EItemDropZone DropZone, FLexWidgetComponentItem TargetItem)
	{
		ULexWidget* Widget = GetCurrentWidget();
		if (!CanAddOrRemoveComponent() || !IsValid(Widget) || !TargetItem.IsValid())
		{
			return FReply::Unhandled();
		}

		TSharedPtr<FLexWidgetComponentDragDropOp> DragOp = DragDropEvent.GetOperationAs<FLexWidgetComponentDragDropOp>();
		if (!DragOp.IsValid() || !DragOp->DraggedItem.IsValid() || DragOp->DraggedItem == TargetItem)
		{
			return FReply::Unhandled();
		}

		const int32 TargetIndex = ComponentItems.IndexOfByKey(TargetItem);
		if (TargetIndex == INDEX_NONE)
		{
			return FReply::Unhandled();
		}

		const int32 DesiredIndex = (DropZone == EItemDropZone::BelowItem) ? (TargetIndex + 1) : TargetIndex;
		if (MoveComponentToIndex(DragOp->DraggedItem.Get(), DesiredIndex))
		{
			return FReply::Handled();
		}

		return FReply::Unhandled();
	}

	bool CanHandleAssetDrop(const FDragDropEvent& DragDropEvent) const
	{
		if (!CanAddOrRemoveComponent())
		{
			return false;
		}

		TSharedPtr<FDragDropOperation> Operation = DragDropEvent.GetOperation();
		return Operation.IsValid() && Operation->IsOfType<FAssetDragDropOp>();
	}

	bool HandleAddComponentsFromAssetDrop(const FDragDropEvent& DragDropEvent)
	{
		if (!CanHandleAssetDrop(DragDropEvent))
		{
			return false;
		}

		ULexWidget* Widget = GetCurrentWidget();
		if (!IsValid(Widget))
		{
			return false;
		}

		TArray<FAssetData> DroppedAssetData = AssetUtil::ExtractAssetDataFromDrag(DragDropEvent.GetOperation());
		if (DroppedAssetData.Num() == 0)
		{
			return false;
		}

		TArray<UClass*> ComponentClassesToAdd;
		for (const FAssetData& AssetData : DroppedAssetData)
		{
			if (UClass* ComponentClass = ResolveComponentClassFromAsset(AssetData))
			{
				ComponentClassesToAdd.Add(ComponentClass);
			}
		}

		if (ComponentClassesToAdd.Num() == 0)
		{
			return false;
		}

		const FScopedTransaction Transaction(LOCTEXT("AddLexWidgetComponentByDragDrop_Transaction", "Add LexUI Component"));
		if (UObject* WidgetOuter = Widget->GetOuter())
		{
			WidgetOuter->SetFlags(RF_Transactional);
			WidgetOuter->Modify();
		}
		Widget->SetFlags(RF_Transactional);
		Widget->Modify();
		auto PrefabEditor = FLexUIPrefabEditor::GetEditorByWorld(Widget->GetWorld());
		if (PrefabEditor.IsValid())
		{
			PrefabEditor.Pin()->GetPrefabHelperObject()->SetAnythingDirty();
		}

		ULexUIBehaviour* LastAddedComponent = nullptr;
		for (UClass* ComponentClass : ComponentClassesToAdd)
		{
			ULexUIBehaviour* NewComponent = Widget->AddComponent(ComponentClass);
			if (!IsValid(NewComponent))
			{
				continue;
			}

			NewComponent->SetFlags(RF_Transactional);
			NewComponent->Modify();
			LastAddedComponent = NewComponent;
		}

		if (!IsValid(LastAddedComponent))
		{
			return false;
		}

		Widget->OnUnregister();
		Widget->OnRegister();
		FLexUIUtils::NotifyPropertyChanged(Widget, ULexWidget::GetPropertyName_Components());

		RefreshComponents();
		SelectComponent(LastAddedComponent);
		return true;
	}

	UClass* ResolveComponentClassFromAsset(const FAssetData& AssetData) const
	{
		UObject* Asset = AssetData.GetAsset();
		if (!IsValid(Asset))
		{
			return nullptr;
		}

		if (UBlueprint* Blueprint = Cast<UBlueprint>(Asset))
		{
			UClass* GeneratedClass = Blueprint->GeneratedClass;
			return FLexWidgetComponentClassFilter::IsComponentClassAllowed(GeneratedClass) ? GeneratedClass : nullptr;
		}

		if (UClass* ComponentClass = Cast<UClass>(Asset))
		{
			return FLexWidgetComponentClassFilter::IsComponentClassAllowed(ComponentClass) ? ComponentClass : nullptr;
		}

		return nullptr;
	}

	bool MoveComponentToIndex(ULexUIBehaviour* Component, int32 NewIndex)
	{
		ULexWidget* Widget = GetCurrentWidget();
		if (!CanAddOrRemoveComponent() || !IsValid(Widget) || !IsValid(Component) || Component->GetWidget() != Widget)
		{
			return false;
		}

		const int32 CurrentIndex = ComponentItems.IndexOfByKey(Component);
		if (CurrentIndex == INDEX_NONE)
		{
			return false;
		}

		const int32 ClampedIndex = FMath::Clamp(NewIndex, 0, ComponentItems.Num());
		if (CurrentIndex == ClampedIndex || (CurrentIndex == ComponentItems.Num() - 1 && ClampedIndex == ComponentItems.Num()))
		{
			return false;
		}

		const FScopedTransaction Transaction(LOCTEXT("ReorderLexWidgetComponent_Transaction", "Reorder LexUI Component"));
		if (UObject* WidgetOuter = Widget->GetOuter())
		{
			WidgetOuter->SetFlags(RF_Transactional);
			WidgetOuter->Modify();
		}
		Widget->SetFlags(RF_Transactional);
		Widget->Modify();
		auto PrefabEditor = FLexUIPrefabEditor::GetEditorByWorld(Widget->GetWorld());
		if (PrefabEditor.IsValid())
		{
			PrefabEditor.Pin()->GetPrefabHelperObject()->SetAnythingDirty();
		}

		Widget->MoveComponentToIndex(Component, ClampedIndex);
		FLexUIUtils::NotifyPropertyChanged(Widget, ULexWidget::GetPropertyName_Components());

		RefreshComponents();
		SelectComponent(Component);
		return true;
	}

	EVisibility GetEmptyStateVisibility() const
	{
		return ComponentItems.Num() == 0 ? EVisibility::Visible : EVisibility::Collapsed;
	}

	FText GetEmptyStateText() const
	{
		return IsValid(GetCurrentWidget())
			? LOCTEXT("NoWidgetComponents", "No components on this widget")
			: LOCTEXT("SelectWidgetForComponents", "Select a widget to edit components");
	}

	TSharedRef<ITableRow> HandleGenerateRow(FLexWidgetComponentItem InItem, const TSharedRef<STableViewBase>& OwnerTable)
	{
		return
		SNew(STableRow<FLexWidgetComponentItem>, OwnerTable)
		.Style(&FAppStyle::Get().GetWidgetStyle<FTableRowStyle>("SceneOutliner.TableViewRow"))
		.Padding(FMargin(0, 4, 0, 4))
		.ShowSelection(true)
		.OnDragDetected(this, &SLexWidgetComponentEditor::HandleDragDetected, InItem)
		.OnCanAcceptDrop(this, &SLexWidgetComponentEditor::HandleCanAcceptDrop)
		.OnAcceptDrop(this, &SLexWidgetComponentEditor::HandleAcceptDrop)
		[
			SNew(SHorizontalBox)
			+ SHorizontalBox::Slot()
			.AutoWidth()
			.VAlign(VAlign_Center)
			.Padding(2, 0)
			[
				SNew(SImage)
				.ColorAndOpacity(FSlateColor::UseForeground())
				.Image_Lambda([=, this]()
				{
					if (InItem.IsValid())
					{
						return FSlateIconFinder::FindIconBrushForClass(InItem->GetClass());
					}
					return (const FSlateBrush*)nullptr;
				})
			]
			+SHorizontalBox::Slot()
			[
				SNew(STextBlock)
				.Text(GetComponentText(InItem.Get()))
				.ToolTipText(GetComponentTooltipText(InItem.Get()))
			]
		];
	}

	void HandleSelectionChanged(FLexWidgetComponentItem InItem, ESelectInfo::Type SelectInfo)
	{
		if (SelectInfo != ESelectInfo::Direct && ComponentListView.IsValid())
		{
			FSlateApplication::Get().SetKeyboardFocus(ComponentListView, EFocusCause::SetDirectly);
		}

		BroadcastSelectionChanged();
	}

	TSharedPtr<SWidget> OnContextMenuOpening()
	{
		FMenuBuilder MenuBuilder(true, nullptr);
		
		MenuBuilder.BeginSection(NAME_None, LOCTEXT("EditComponent", "EDIT"));
		MenuBuilder.PushCommandList(CommandList.ToSharedRef());
		{
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Copy);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Paste);
			MenuBuilder.AddMenuEntry(FGenericCommands::Get().Delete);
		}
		MenuBuilder.PopCommandList();
		MenuBuilder.AddMenuEntry(
			LOCTEXT("PasteProperties", "Paste Properties"),
			LOCTEXT("PasteProperties_Tooltip", "Paste copied component properties onto the selected component"),
			FSlateIcon(),
			FUIAction(
				FExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::HandlePasteProperties),
				FCanExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::CanPasteProperties)
			)
		);
		MenuBuilder.EndSection();

		MenuBuilder.BeginSection(NAME_None, LOCTEXT("ComponentAsset", "ASSET"));
		{
			TArray<FLexWidgetComponentItem> SelectedItems;
			if (ComponentListView.IsValid())
			{
				ComponentListView->GetSelectedItems(SelectedItems);
			}
			ULexUIBehaviour* SelectedComp = (SelectedItems.Num() > 0 && SelectedItems[0].IsValid()) ? SelectedItems[0].Get() : nullptr;
			UClass* ComponentClass = IsValid(SelectedComp) ? SelectedComp->GetClass() : nullptr;
			bool bHasSelection = ComponentClass != nullptr;

			FText EditLabel = bHasSelection
				? FText::Format(LOCTEXT("EditComponentClass_Label", "Edit {0}"), FText::FromString(ComponentClass->GetName()))
				: LOCTEXT("EditComponentClass_LabelNoSel", "Edit");

			MenuBuilder.AddMenuEntry(
				EditLabel,
				LOCTEXT("EditComponentClass_Tooltip", "Open Blueprint editor or C++ source for this component class"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::HandleEditComponentClass),
					FCanExecuteAction::CreateLambda([bHasSelection]() { return bHasSelection; })
				)
			);

			MenuBuilder.AddMenuEntry(
				LOCTEXT("FindInContentBrowser_Label", "Find Class in Content Browser"),
				LOCTEXT("FindInContentBrowser_Tooltip", "Locate the Blueprint class in the Content Browser"),
				FSlateIcon(),
				FUIAction(
					FExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::HandleFindClassInContentBrowser),
					FCanExecuteAction::CreateSP(this, &SLexWidgetComponentEditor::CanFindClassInContentBrowser)
				)
			);
		}
		MenuBuilder.EndSection();
		
		return MenuBuilder.MakeWidget();
	}

	void BroadcastSelectionChanged()
	{
		if (!OnSelectionChanged.IsBound())
		{
			return;
		}

		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		OnSelectionChanged.Execute(SelectedItems);
	}

	TSharedRef<SWidget> GenerateAddComponentMenu()
	{
		FClassViewerInitializationOptions Options;
		Options.Mode = EClassViewerMode::ClassPicker;
		Options.DisplayMode = EClassViewerDisplayMode::TreeView;
		Options.NameTypeToDisplay = EClassViewerNameTypeToDisplay::Dynamic;
		Options.bShowObjectRootClass = false;
		Options.bShowNoneOption = false;
		Options.bExpandAllNodes = true;
		Options.bShowUnloadedBlueprints = true;

		Options.ClassFilters.Add(MakeShared<FLexWidgetComponentClassFilter>());

		FClassViewerModule& ClassViewerModule = FModuleManager::LoadModuleChecked<FClassViewerModule>("ClassViewer");
		return SNew(SBox)
			.WidthOverride(320.0f)
			.HeightOverride(400.0f)
			[
				ClassViewerModule.CreateClassViewer(Options, FOnClassPicked::CreateSP(this, &SLexWidgetComponentEditor::HandleComponentClassPicked))
			];
	}

	void HandleComponentClassPicked(UClass* InClass)
	{
		FSlateApplication::Get().DismissAllMenus();

		ULexWidget* Widget = GetCurrentWidget();
		if (!CanAddOrRemoveComponent() || !IsValid(Widget) || InClass == nullptr)
		{
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("AddLexWidgetComponent_Transaction", "Add LexUI Component"));
		if (UObject* WidgetOuter = Widget->GetOuter())
		{
			WidgetOuter->SetFlags(RF_Transactional);
			WidgetOuter->Modify();
		}
		Widget->SetFlags(RF_Transactional);
		Widget->Modify();

		ULexUIBehaviour* NewComponent = Widget->AddComponent(InClass);
		if (!IsValid(NewComponent))
		{
			return;
		}

		NewComponent->SetFlags(RF_Transactional);
		NewComponent->Modify();
		FLexUIUtils::NotifyPropertyChanged(Widget, ULexWidget::GetPropertyName_Components());

		RefreshComponents();
		SelectComponent(NewComponent);
	}

	void HandleEditComponentClass()
	{
		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		if (SelectedItems.Num() == 0 || !SelectedItems[0].IsValid())
		{
			return;
		}
		UClass* ComponentClass = SelectedItems[0]->GetClass();
		if (UBlueprint* Blueprint = Cast<UBlueprint>(ComponentClass->ClassGeneratedBy))
		{
			GEditor->EditObject(Blueprint);
		}
		else
		{
			FSourceCodeNavigation::NavigateToClass(ComponentClass);
		}
	}

	void HandleFindClassInContentBrowser()
	{
		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		if (SelectedItems.Num() == 0 || !SelectedItems[0].IsValid())
		{
			return;
		}
		UClass* ComponentClass = SelectedItems[0]->GetClass();
		if (UBlueprint* Blueprint = Cast<UBlueprint>(ComponentClass->ClassGeneratedBy))
		{
			TArray<UObject*> Objects;
			Objects.Add(Blueprint);
			GEditor->SyncBrowserToObjects(Objects);
		}
	}

	bool CanFindClassInContentBrowser() const
	{
		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		if (SelectedItems.Num() == 0 || !SelectedItems[0].IsValid())
		{
			return false;
		}
		UClass* ComponentClass = SelectedItems[0]->GetClass();
		return Cast<UBlueprint>(ComponentClass->ClassGeneratedBy) != nullptr;
	}

	void HandleRemoveSelectedComponents()
	{
		ULexWidget* Widget = GetCurrentWidget();
		if (!CanRemoveSelectedComponents() || !IsValid(Widget) || !ComponentListView.IsValid())
		{
			return;
		}

		TArray<FLexWidgetComponentItem> SelectedItems;
		ComponentListView->GetSelectedItems(SelectedItems);

		const FScopedTransaction Transaction(LOCTEXT("RemoveLexWidgetComponent_Transaction", "Remove LexUI Component"));
		if (UObject* WidgetOuter = Widget->GetOuter())
		{
			WidgetOuter->SetFlags(RF_Transactional);
			WidgetOuter->Modify();
		}
		Widget->SetFlags(RF_Transactional);
		Widget->Modify();

		for (const FLexWidgetComponentItem& Item : SelectedItems)
		{
			ULexUIBehaviour* Component = Item.Get();
			if (!IsValid(Component) || Component->GetWidget() != Widget)
			{
				continue;
			}

			Component->SetFlags(RF_Transactional);
			Component->Modify();
			Widget->RemoveComponent(Component);
			FLexUIUtils::NotifyPropertyChanged(Widget, ULexWidget::GetPropertyName_Components());
		}

		RefreshComponents();
		ClearSelection();
	}

	void HandleCopySelectedComponent()
	{
		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		if (SelectedItems.Num() > 0 && SelectedItems[0].IsValid())
		{
			ULexUIBehaviour* SelectedComponent = SelectedItems[0].Get();
			CopiedComponentClass = SelectedComponent->GetClass();

			// Clean up old template
			if (CopiedComponentTemplate)
			{
				CopiedComponentTemplate->MarkAsGarbage();
				CopiedComponentTemplate = nullptr;
			}

			// Create a transient template to hold the copied properties
			CopiedComponentTemplate = NewObject<ULexUIBehaviour>(
				GetTransientPackage(),
				CopiedComponentClass,
				NAME_None,
				RF_Transient | RF_ArchetypeObject | RF_Transactional
			);
			if (CopiedComponentTemplate)
			{
				UEngine::CopyPropertiesForUnrelatedObjects(SelectedComponent, CopiedComponentTemplate);
			}
		}
	}

	bool CanCopySelectedComponent() const
	{
		if (!ComponentListView.IsValid())
		{
			return false;
		}
		TArray<FLexWidgetComponentItem> SelectedItems;
		ComponentListView->GetSelectedItems(SelectedItems);
		return SelectedItems.Num() > 0 && SelectedItems[0].IsValid();
	}

	void HandlePasteComponent()
	{
		ULexWidget* Widget = GetCurrentWidget();
		if (!CanAddOrRemoveComponent() || !IsValid(Widget) || CopiedComponentClass == nullptr || CopiedComponentTemplate == nullptr)
		{
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("PasteLexWidgetComponent_Transaction", "Paste LexUI Component"));
		if (UObject* WidgetOuter = Widget->GetOuter())
		{
			WidgetOuter->SetFlags(RF_Transactional);
			WidgetOuter->Modify();
		}
		Widget->SetFlags(RF_Transactional);
		Widget->Modify();
		auto PrefabEditor = FLexUIPrefabEditor::GetEditorByWorld(Widget->GetWorld());
		if (PrefabEditor.IsValid())
		{
			PrefabEditor.Pin()->GetPrefabHelperObject()->SetAnythingDirty();
		}

		ULexUIBehaviour* NewComponent = Widget->AddComponent(CopiedComponentClass);
		if (!IsValid(NewComponent))
		{
			return;
		}

		NewComponent->SetFlags(RF_Transactional);
		NewComponent->Modify();

		// Copy properties from the template
		UEngine::CopyPropertiesForUnrelatedObjects(CopiedComponentTemplate, NewComponent);

		Widget->OnUnregister();
		Widget->OnRegister();
		FLexUIUtils::NotifyPropertyChanged(Widget, ULexWidget::GetPropertyName_Components());

		RefreshComponents();
		SelectComponent(NewComponent);
	}

	bool CanPasteComponent() const
	{
		return CanAddOrRemoveComponent() && CopiedComponentClass != nullptr && CopiedComponentTemplate != nullptr;
	}

	void HandlePasteProperties()
	{
		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		if (SelectedItems.Num() == 0 || !SelectedItems[0].IsValid() || CopiedComponentTemplate == nullptr)
		{
			return;
		}

		ULexUIBehaviour* TargetComponent = SelectedItems[0].Get();
		if (!CanAddOrRemoveComponent())
		{
			return;
		}

		const FScopedTransaction Transaction(LOCTEXT("PasteProperties_Transaction", "Paste Component Properties"));
		TargetComponent->SetFlags(RF_Transactional);
		TargetComponent->Modify();

		UEngine::CopyPropertiesForUnrelatedObjects(CopiedComponentTemplate, TargetComponent);

		if (ULexWidget* Widget = GetCurrentWidget())
		{
			FLexUIUtils::NotifyPropertyChanged(Widget, ULexWidget::GetPropertyName_Components());
		}
	}

	bool CanPasteProperties() const
	{
		if (CopiedComponentTemplate == nullptr || CopiedComponentClass == nullptr)
		{
			return false;
		}

		TArray<FLexWidgetComponentItem> SelectedItems;
		if (ComponentListView.IsValid())
		{
			ComponentListView->GetSelectedItems(SelectedItems);
		}
		if (SelectedItems.Num() == 0 || !SelectedItems[0].IsValid())
		{
			return false;
		}

		// Both must be ULexUIBehaviour subclasses - CopyPropertiesForUnrelatedObjects handles property matching
		UClass* TargetClass = SelectedItems[0]->GetClass();
		return TargetClass && TargetClass->IsChildOf(ULexUIBehaviour::StaticClass());
	}

	static FText GetComponentText(const ULexUIBehaviour* InComponent)
	{
		if (!IsValid(InComponent))
		{
			return LOCTEXT("InvalidLexWidgetComponent", "Invalid Component");
		}
		return InComponent->GetClass()->GetDisplayNameText();
	}
	static FText GetComponentTooltipText(const ULexUIBehaviour* InComponent)
	{
		if (!IsValid(InComponent))
		{
			return LOCTEXT("InvalidLexWidgetComponent", "Invalid Component");
		}

		return FText::FromString(InComponent->GetName());
	}

	FOnGetWidgetContext GetWidgetContext;
	FOnCanEdit CanEdit;
	FOnComponentsSelectionChanged OnSelectionChanged;
	TArray<FLexWidgetComponentItem> ComponentItems;
	TSharedPtr<SWidget> ToolbarWidget;
	TSharedPtr<SListView<FLexWidgetComponentItem>> ComponentListView;
	TSharedPtr<FUICommandList> CommandList;
	static UClass* CopiedComponentClass;
	static ULexUIBehaviour* CopiedComponentTemplate;
};

UClass* SLexWidgetComponentEditor::CopiedComponentClass = nullptr;
ULexUIBehaviour* SLexWidgetComponentEditor::CopiedComponentTemplate = nullptr;

void SLexUIPrefabEditorDetails::Construct(const FArguments& Args, UWorld* InWorld)
{
	World = InWorld;
	PrefabEditorPtr = FLexUIPrefabEditor::GetEditorByWorld(World.Get());
	if (PrefabEditorPtr.IsValid())
	{
		//if open in PrefabEditor then sync selection by PrefabEditor
		PrefabEditorPtr.Pin()->OnSelectionChanged.AddRaw(this, &SLexUIPrefabEditorDetails::OnEditorSelectionChanged);
	}
	else
	{
		//if open in WidgetInspector then sync selection by LexUISelection
		ULexUISelection::GetInstance(World.Get())->OnSelectionChanged.AddRaw(this, &SLexUIPrefabEditorDetails::OnEditorSelectionChanged);
	}

    FPropertyEditorModule& PropPlugin = FModuleManager::LoadModuleChecked<FPropertyEditorModule>("PropertyEditor");
    FDetailsViewArgs DetailsViewArgs;
    DetailsViewArgs.bUpdatesFromSelection = true;
    DetailsViewArgs.bLockable = true;
    DetailsViewArgs.NotifyHook = GUnrealEd;
    DetailsViewArgs.ViewIdentifier = FName(TEXT("LexUIPrefabEditor"));
    DetailsViewArgs.bCustomNameAreaLocation = true;
    DetailsViewArgs.bCustomFilterAreaLocation = false;
    DetailsViewArgs.DefaultsOnlyVisibility = EEditDefaultsOnlyNodeVisibility::Hide;
    DetailsViewArgs.NameAreaSettings = FDetailsViewArgs::ComponentsAndActorsUseNameArea;
    DetailsViewArgs.bShowOptions = true;
	DetailsViewArgs.bAllowSearch = true;
	DetailsViewArgs.bShowObjectLabel = true;
    //DetailsViewArgs.HostCommandList = InCommandList;

    DetailsView = PropPlugin.CreateDetailView(DetailsViewArgs);
    DetailsView->SetIsPropertyReadOnlyDelegate(FIsPropertyReadOnly::CreateSP(this, &SLexUIPrefabEditorDetails::IsPropertyReadOnly));

	if (PrefabEditorPtr.IsValid())
	{
		TSharedRef<FLexUIDetailKeyframeHandler> KeyframeHandler = MakeShareable(new FLexUIDetailKeyframeHandler(PrefabEditorPtr.Pin()));
		DetailsView->SetKeyframeHandler(KeyframeHandler);
	}

	TSharedRef<FLexWidgetDetailPropertyExtensionHandler> BindingHandler = MakeShareable(new FLexWidgetDetailPropertyExtensionHandler(World.Get()));
	DetailsView->SetExtensionHandler(BindingHandler);

	ComponentEditor = SNew(SLexWidgetComponentEditor)
		.GetWidgetContext(this, &SLexUIPrefabEditorDetails::GetSelectedWidgetContext)
		.CanEdit(this, &SLexUIPrefabEditorDetails::IsEditorAllowEditing)
		.OnSelectionChanged(this, &SLexUIPrefabEditorDetails::OnComponentSelectionChanged);

	ChildSlot
		[
			SNew(SVerticalBox)
			+ SVerticalBox::Slot()
			.Padding(FMargin(2, 2))
			.AutoHeight()
			[
				ComponentEditor->GetToolbarWidget()
			]
			+ SVerticalBox::Slot()
			.Padding(FMargin(2, 2))
			.AutoHeight()
			[
				SNew(SBox)
				.Visibility(this, &SLexUIPrefabEditorDetails::GetPrefabButtonVisibility)
				.IsEnabled(this, &SLexUIPrefabEditorDetails::IsPrefabButtonEnable)
				.HeightOverride(this, &SLexUIPrefabEditorDetails::GetPrefabButtonHeight)
				[
					SNew(SHorizontalBox)
					+SHorizontalBox::Slot()
					.AutoWidth()
					.Padding(FMargin(4, 0))
					[
						SNew(SBox)
						.HAlign(EHorizontalAlignment::HAlign_Center)
						.VAlign(EVerticalAlignment::VAlign_Center)
						[
							SNew(STextBlock)
							.Text(LOCTEXT("PrefabFunctions", "Prefab"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(FMargin(2, 0))
					[
						SNew(SButton)
						.OnClicked_Lambda([=, this]() {
							if (PrefabEditorPtr.IsValid())
							{
								PrefabEditorPtr.Pin()->OpenSubPrefab(CachedWidget.Get());
							}
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("OpenPrefab", "Open"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.2f)
					.Padding(FMargin(2, 0))
					[
						SNew(SButton)
						.OnClicked_Lambda([=, this]() {
							if (PrefabEditorPtr.IsValid())
							{
								PrefabEditorPtr.Pin()->SelectSubPrefab(CachedWidget.Get());
							}
							return FReply::Handled();
						})
						[
							SNew(SBox)
							.HAlign(EHorizontalAlignment::HAlign_Center)
							.VAlign(EVerticalAlignment::VAlign_Center)
							[
								SNew(STextBlock)
								.Text(LOCTEXT("SelectPrefab", "Select"))
								.Font(IDetailLayoutBuilder::GetDetailFont())
							]
						]
					]
					+SHorizontalBox::Slot()
					.FillWidth(0.5f)
					.Padding(FMargin(2, 0))
					[
						SNew(SComboButton)
						.HasDownArrow(true)
						.ToolTipText(LOCTEXT("PrefabOverride", "Edit override parameters for this prefab"))
						.ButtonContent()
						[
							SNew(STextBlock)
							.Text(LOCTEXT("OverrideButton", "Prefab Override Properties"))
							.Font(IDetailLayoutBuilder::GetDetailFont())
						]
						.MenuContent()
						[
							SNew(SBox)
							.Padding(FMargin(4, 4))
							[
								SNew(SHorizontalBox)
								+SHorizontalBox::Slot()
								.AutoWidth()
								[
									SNew(SVerticalBox)
									+SVerticalBox::Slot()
									.AutoHeight()
									[
										SNew(SHorizontalBox)
										+SHorizontalBox::Slot()
										.AutoWidth()
										[
											SAssignNew(PrefabOverrideDataViewer, SLexUIPrefabOverrideDataViewer, [=, this]()
											{
												return CachedWidget.Get();
											})
											.AfterRevertPrefab_Lambda([=, this](ULexUIPrefab* PrefabAsset) {
												})
											.AfterApplyPrefab_Lambda([=, this](ULexUIPrefab* PrefabAsset){
												FLexUIEditorTools::RefreshLoadedPrefab();
												FLexUIEditorTools::RefreshOnSubPrefabChange(PrefabAsset);
												FLexUIEditorTools::RefreshOpenedPrefabEditor(PrefabAsset);
												})
										]
									]
								]
							]
						]
					]
				]
			]
			+ SVerticalBox::Slot()
			[
				SNew(SSplitter)
				.Orientation(EOrientation::Orient_Vertical)
				+ SSplitter::Slot()
				.Resizable(true)
				.SizeRule(SSplitter::ESizeRule::FractionOfParent)
				.Value(0.2f)
				[
					SNew(SBox)
					.MinDesiredHeight(200)
					.Padding(FMargin(0, 2))
					[
						SNew(SBorder)
						.BorderImage(FAppStyle::Get().GetBrush("SCSEditor.Background"))
						.Padding(4.f)
						.AddMetaData<FTagMetaData>(FTagMetaData(TEXT("ComponentsPanel")))
						[
							ComponentEditor.ToSharedRef()
						]
					]
				]
				+ SSplitter::Slot()
				[
					SNew(SVerticalBox)
					+SVerticalBox::Slot()
					.Padding(FMargin(0, 2))
					[
						DetailsView.ToSharedRef()
					]
				]
			]
		];
}

SLexUIPrefabEditorDetails::~SLexUIPrefabEditorDetails()
{
	if (auto Selection = ULexUISelection::GetInstance(World.Get()))
	{
		Selection->OnSelectionChanged.RemoveAll(this);
	}
}

bool SLexUIPrefabEditorDetails::IsPrefabButtonEnable()const
{
	if (PrefabEditorPtr.IsValid() && CachedWidget.IsValid())
	{
		return PrefabEditorPtr.Pin()->WidgetIsSubPrefabRoot(CachedWidget.Get());
	}
	return false;
}

FOptionalSize SLexUIPrefabEditorDetails::GetPrefabButtonHeight()const
{
	return IsPrefabButtonEnable() ? 26 : 0;
}

EVisibility SLexUIPrefabEditorDetails::GetPrefabButtonVisibility()const
{
	return IsPrefabButtonEnable() ? EVisibility::Visible : EVisibility::Hidden;
}

bool SLexUIPrefabEditorDetails::IsEditorAllowEditing()const
{
	if (PrefabEditorPtr.IsValid() && CachedWidget.IsValid())
	{
		return !PrefabEditorPtr.Pin()->WidgetBelongsToSubPrefab(CachedWidget.Get());
	}
	return true;
}

ULexWidget* SLexUIPrefabEditorDetails::GetSelectedWidgetContext() const
{
	if (auto Selection = ULexUISelection::GetInstance(World.Get()))
	{
		auto SelectedWidgets = Selection->GetSelectedWidgets();
		if (SelectedWidgets.Num() > 0 && SelectedWidgets[0].IsValid())
		{
			return SelectedWidgets[0].Get();
		}
	}
	return nullptr;
}

void SLexUIPrefabEditorDetails::OnEditorSelectionChanged()
{
	if (bIsSelectFromComponentList)return;
	bIsSelectFromLexUIEditor = true;
	auto Selection = ULexUISelection::GetInstance(World.Get());
	auto SelectedWidgets = Selection->GetSelectedWidgets();
	auto SelectedComponents = Selection->GetSelectedComponents();
	if (SelectedWidgets.Num() > 0)
	{
		if (auto Widget = SelectedWidgets[0].Get())
		{
			if (Widget->GetWorld() != World.Get())
			{
				bIsSelectFromLexUIEditor = false;
				return;
			}

			CachedWidget = Widget;
			PrefabOverrideDataViewer->RefreshDataContent();
			if (ComponentEditor)
			{
				ComponentEditor->RefreshComponents();
				ComponentEditor->ClearSelection();
			}
		}

		TArray<UObject*> SelectedObjectList;
		for (int32 i = 0; i < SelectedWidgets.Num(); i++)
		{
			auto Widget = SelectedWidgets[i];
			if (Widget.IsValid())
			{
				if (Widget->GetWorld() != World.Get())
				{
					continue;
				}

				Widget->SetFlags(RF_Transactional);
				ForEachObjectWithOuter(Widget.Get(), [=](UObject* Object) {
					Object->SetFlags(RF_Transactional);
				});
				SelectedObjectList.Add(Widget.Get());
			}
		}

		if (DetailsView)
		{
			DetailsView->SetObjects(SelectedObjectList, true);
		}
		if (SelectedObjectList.Num() == 0)
		{
			CachedWidget = nullptr;
			PrefabOverrideDataViewer->RefreshDataContent();
			if (ComponentEditor)
			{
				ComponentEditor->RefreshComponents();
				ComponentEditor->ClearSelection();
			}
		}
		else
		{
			if (SelectedComponents.Num() > 0)
			{
				if (ComponentEditor)
				{
					ComponentEditor->SelectComponent(SelectedComponents[0].Get());
				}
			}
		}
	}
	else
	{
		TArray<UObject*> SelectedObjectList;
		if (DetailsView)
		{
			DetailsView->SetObjects(SelectedObjectList, true);
		}
		CachedWidget = nullptr;
		PrefabOverrideDataViewer->RefreshDataContent();
		if (ComponentEditor)
		{
			ComponentEditor->RefreshComponents();
			ComponentEditor->ClearSelection();
		}
	}
	bIsSelectFromLexUIEditor = false;
}

void SLexUIPrefabEditorDetails::OnComponentSelectionChanged(const TArray<TWeakObjectPtr<ULexUIBehaviour>>& SelectedComponents)
{
	bIsSelectFromComponentList = true;

	if (!GIsTransacting)
	{
		GEditor->BeginTransaction(LOCTEXT("SelectionChanged_Transaction", "Select Component"));
	}
	ULexUISelection::GetInstance(World.Get())->Modify();

	TArray<UObject*> SelectedObjects;
	TArray<ULexUIBehaviour*> ValidSelectedComponents;
	for (const TWeakObjectPtr<ULexUIBehaviour>& SelectedComponent : SelectedComponents)
	{
		if (ULexUIBehaviour* Component = SelectedComponent.Get())
		{
			Component->SetFlags(RF_Transactional);
			ForEachObjectWithOuter(Component, [=](UObject* Object) {
				Object->SetFlags(RF_Transactional);
			});
			SelectedObjects.Add(Component);
			ValidSelectedComponents.Add(Component);
		}
	}

	if (DetailsView.IsValid())
	{
		if (SelectedObjects.Num() > 0)
		{
			DetailsView->SetObjects(SelectedObjects);
		}
		else if (PrefabEditorPtr.IsValid())
		{
			TArray<UObject*> SelectedWidgetObjects;
			auto RootWidget = PrefabEditorPtr.Pin()->GetRootAgentWidget();
			for (const TWeakObjectPtr<ULexWidget>& Widget : PrefabEditorPtr.Pin()->GetSelectedWidgets())
			{
				if (Widget.IsValid() && (Widget->IsChildOf(RootWidget) || Widget.Get() == RootWidget))
				{
					SelectedWidgetObjects.Add(Widget.Get());
				}
			}
			DetailsView->SetObjects(SelectedWidgetObjects, true);
		}
	}

	if (!bIsSelectFromLexUIEditor)
	{
		auto Selection = ULexUISelection::GetInstance(World.Get());
		Selection->ClearComponentSelection();
		for (ULexUIBehaviour* Component : ValidSelectedComponents)
		{
			Selection->SelectComponent(Component);
		}
	}
	if (!GIsTransacting)
	{
		GEditor->EndTransaction();
	}

	bIsSelectFromComponentList = false;
}

bool SLexUIPrefabEditorDetails::IsPropertyReadOnly(const FPropertyAndParent& InPropertyAndParent)
{
	return false;
}

#undef LOCTEXT_NAMESPACE