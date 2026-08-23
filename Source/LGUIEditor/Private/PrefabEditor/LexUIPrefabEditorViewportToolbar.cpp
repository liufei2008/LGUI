// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LexUIPrefabEditorViewportToolbar.h"
#include "LexUIPrefabEditorViewport.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Styling/AppStyle.h"
#include "ToolMenus.h"
#include "ToolMenu.h"
#include "ToolMenuEntry.h"
#include "ToolMenuSection.h"
#include "Widgets/Layout/SBorder.h"
#include "EditorViewportCommands.h"
#include "EditorViewportClient.h"
#include "ViewportToolbar/UnrealEdViewportToolbar.h"
#include "ViewportToolbar/UnrealEdViewportToolbarContext.h"

#define LOCTEXT_NAMESPACE "SLexUIPrefabEditorViewportToolbar"

namespace LexUI_Private
{
	// Builds a single Camera toggle button that switches between 3D (Perspective)
	// and 2D (Back / ortho back). It reuses the engine's Perspective command so the
	// Perspective shortcut (Alt+G) toggles between the two modes; the command action
	// is rebound in SLexUIPrefabEditorViewportToolbar::Construct to do the toggling.
	static FToolMenuEntry MakeCameraToggleEntry()
	{
		return FToolMenuEntry::InitDynamicEntry(
			"DynamicCameraToggle",
			FNewToolMenuSectionDelegate::CreateLambda(
				[](FToolMenuSection& InDynamicSection) -> void
				{
					TWeakPtr<SEditorViewport> WeakViewport;
					if (UUnrealEdViewportToolbarContext* const EditorViewportContext =
							InDynamicSection.FindContext<UUnrealEdViewportToolbarContext>())
					{
						WeakViewport = EditorViewportContext->Viewport;
					}

					// Button label: show "3D" while in Perspective, "2D" otherwise.
					const TAttribute<FText> Label = TAttribute<FText>::CreateLambda(
						[WeakViewport]()
						{
							if (TSharedPtr<SEditorViewport> Viewport = WeakViewport.Pin())
							{
								const bool bIsPerspective =
									Viewport->GetViewportClient()->ViewportType == LVT_Perspective;
								return bIsPerspective
									? LOCTEXT("CameraButton_3D", "3D")
									: LOCTEXT("CameraButton_2D", "2D");
							}
							return LOCTEXT("CameraSubmenuLabel", "Camera");
						}
					);

					FToolMenuEntry& Entry = InDynamicSection.AddEntry(
						FToolMenuEntry::InitToolBarButton(
							FEditorViewportCommands::Get().Perspective,
							Label,
							LOCTEXT("CameraToggleTooltip", "Toggle between 3D and 2D view"),
							FSlateIcon(FAppStyle::GetAppStyleSetName(), "ClassIcon.CameraComponent")
						)
					);
					Entry.UserInterfaceActionType = EUserInterfaceActionType::Button;
					Entry.ToolBarData.ResizeParams.ClippingPriority = 800;
				}
			)
		);
	}
}

///////////////////////////////////////////////////////////
// SLexUIPrefabEditorViewportToolbar

ICommonEditorViewportToolbarInfoProvider& SLexUIPrefabEditorViewportToolbar::GetInfoProvider() const
{
	return *InfoProviderWeakPtr.Pin().Get();
}

void SLexUIPrefabEditorViewportToolbar::Construct(const FArguments& InArgs, TSharedPtr<class ICommonEditorViewportToolbarInfoProvider> InInfoProvider)
{
	InfoProviderWeakPtr = InInfoProvider;

	// The base class SCommonEditorViewportToolbarBase::Construct() registers and populates the
	// globally-shared "UnrealEd.ViewportToolbar" tool menu, which adds the full set of buttons
	// (Transforms, Snapping, Camera, View Modes, Show, Performance/Scalability, Profile, Settings).
	// Since that menu is shared across editors we cannot trim it without affecting everyone, so
	// here we build a dedicated toolbar that only exposes the Camera and View Modes buttons.
	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	// Rebind the engine's Perspective command so its default shortcut (Alt+G) toggles
	// between 3D (Perspective) and 2D (Back), instead of only switching to Perspective.
	const FEditorViewportCommands& ViewportCommands = FEditorViewportCommands::Get();
	ViewportRef->GetCommandList()->MapAction(
		ViewportCommands.Perspective,
		FExecuteAction::CreateLambda([WeakViewport = TWeakPtr<SEditorViewport>(ViewportRef)]()
		{
			if (TSharedPtr<SEditorViewport> Viewport = WeakViewport.Pin())
			{
				if (auto ViewportClient = Viewport->GetViewportClient())
				{
					const ELevelViewportType NewViewportType = (ViewportClient->GetViewportType() == LVT_Perspective)
						? LVT_OrthoBack
						: LVT_Perspective;
					ViewportClient->SetViewportType(NewViewportType);
				}
			}
		})
	);

	static const FName LexUIViewportToolbarName = TEXT("LexUIPrefabEditor.ViewportToolbar");
	if (!UToolMenus::Get()->IsMenuRegistered(LexUIViewportToolbarName))
	{
		UToolMenu* const ViewportToolbarMenu = UToolMenus::Get()->RegisterMenu(
			LexUIViewportToolbarName, NAME_None, EMultiBoxType::SlimHorizontalToolBar
		);
		ViewportToolbarMenu->StyleName = TEXT("ViewportToolbar");

		FToolMenuSection& RightSection = ViewportToolbarMenu->AddSection("Right");
		RightSection.Alignment = EToolMenuSectionAlign::Last;
		{
			// Camera toggle button (custom: only 3D / 2D modes)
			RightSection.AddEntry(LexUI_Private::MakeCameraToggleEntry());

			// View Modes menu
			RightSection.AddEntry(UE::UnrealEd::CreateViewModesSubmenu());
		}
	}

	FToolMenuContext ViewportToolbarContext;
	{
		ViewportToolbarContext.AppendCommandList(ViewportRef->GetCommandList());

		UUnrealEdViewportToolbarContext* const ContextObject = UE::UnrealEd::CreateViewportToolbarDefaultContext(ViewportRef);
		ViewportToolbarContext.AddObject(ContextObject);
	}

	TSharedRef<SWidget> ToolMenuWidget = UToolMenus::Get()->GenerateWidget(LexUIViewportToolbarName, ViewportToolbarContext);

	ChildSlot
	[
		SNew(SBorder)
		.BorderImage(FAppStyle::GetBrush(TEXT("EditorViewportToolBar.Background")))
		.Cursor(EMouseCursor::Default)
		[
			ToolMenuWidget
		]
	];

	// Finish the SViewportToolBar base initialization (open-menu state, etc.)
	SViewportToolBar::Construct(SViewportToolBar::FArguments());
}

TSharedRef<SWidget> SLexUIPrefabEditorViewportToolbar::GenerateShowMenu() const
{
	GetInfoProvider().OnFloatingButtonClicked();

	TSharedRef<SEditorViewport> ViewportRef = GetInfoProvider().GetViewportWidget();

	const bool bInShouldCloseWindowAfterMenuSelection = true;
	FMenuBuilder ShowMenuBuilder(bInShouldCloseWindowAfterMenuSelection, ViewportRef->GetCommandList());

	return ShowMenuBuilder.MakeWidget();
}

#undef LOCTEXT_NAMESPACE
