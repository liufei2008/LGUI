// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIEditorStyle.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleRegistry.h"

TSharedPtr< FSlateStyleSet > FLGUIEditorStyle::StyleInstance = NULL;

void FLGUIEditorStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FLGUIEditorStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FLGUIEditorStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("LGUIEditorStyle"));
	return StyleSetName;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FLGUIEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LGUIEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LGUI")->GetBaseDir() / TEXT("Resources/Icons"));;

	Style->Set("ClassThumbnail.UIBaseActor", new IMAGE_BRUSH(TEXT("UIItem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISpriteActor", new IMAGE_BRUSH(TEXT("UISprite_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextActor", new IMAGE_BRUSH(TEXT("UIText_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIContainerActor", new IMAGE_BRUSH(TEXT("UIItem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPolygonActor", new IMAGE_BRUSH(TEXT("UIPolygon_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPolygonLineActor", new IMAGE_BRUSH(TEXT("UIPolygonLine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineActor", new IMAGE_BRUSH(TEXT("UILine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineChildrenAsPointsActor", new IMAGE_BRUSH(TEXT("UILineChildrenAsPoints_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIRingActor", new IMAGE_BRUSH(TEXT("UIRing_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIBasePostProcessActor", new IMAGE_BRUSH(TEXT("UIPostProcess_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIProceduralRectActor", new IMAGE_BRUSH(TEXT("UIProceduralRect_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LGUIWidgetActor", new IMAGE_BRUSH(TEXT("LGUIWidget_40x"), Icon40x40));

	Style->Set("ClassIcon.UIBaseActor", new IMAGE_BRUSH(TEXT("UIItem_16x"), Icon16x16));
	Style->Set("ClassIcon.UISpriteActor", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextActor", new IMAGE_BRUSH(TEXT("UIText_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.UIContainerActor", new IMAGE_BRUSH(TEXT("UIItem_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPolygonActor", new IMAGE_BRUSH(TEXT("UIPolygon_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPolygonLineActor", new IMAGE_BRUSH(TEXT("UIPolygonLine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineActor", new IMAGE_BRUSH(TEXT("UILine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineChildrenAsPointsActor", new IMAGE_BRUSH(TEXT("UILineChildrenAsPoints_16x"), Icon16x16));
	Style->Set("ClassIcon.UIRingActor", new IMAGE_BRUSH(TEXT("UIRing_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.UIBasePostProcessActor", new IMAGE_BRUSH(TEXT("UIPostProcess_16x"), Icon16x16));
	Style->Set("ClassIcon.UIProceduralRectActor", new IMAGE_BRUSH(TEXT("UIProceduralRect_16x"), Icon16x16));
	Style->Set("ClassIcon.LGUIWidgetActor", new IMAGE_BRUSH(TEXT("LGUIWidget_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LGUICanvas", new IMAGE_BRUSH(TEXT("LGUICanvas_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LGUICanvasScaler", new IMAGE_BRUSH(TEXT("CanvasScaler_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISpriteBase", new IMAGE_BRUSH(TEXT("UISprite_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIText", new IMAGE_BRUSH(TEXT("UIText_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITexture", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIItem", new IMAGE_BRUSH(TEXT("UIItem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPolygon", new IMAGE_BRUSH(TEXT("UIPolygon_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPolygonLine", new IMAGE_BRUSH(TEXT("UIPolygonLine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineRaw", new IMAGE_BRUSH(TEXT("UILine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineRendererBase", new IMAGE_BRUSH(TEXT("UILine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineChildrenAsPoints", new IMAGE_BRUSH(TEXT("UILineChildrenAsPoints_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIRing", new IMAGE_BRUSH(TEXT("UIRing_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureBase", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPostProcessRenderable", new IMAGE_BRUSH(TEXT("UIPostProcess_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LGUILifeCycleUIBehaviour", new IMAGE_BRUSH(TEXT("LGUILifeCycleBehaviour_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LGUILifeCycleBehaviour", new IMAGE_BRUSH(TEXT("LGUILifeCycleBehaviour_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UICanvasGroup", new IMAGE_BRUSH(TEXT("UICanvasGroup_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIProceduralRect", new IMAGE_BRUSH(TEXT("UIProceduralRect_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LGUIWidget", new IMAGE_BRUSH(TEXT("LGUIWidget_40x"), Icon40x40));

	Style->Set("ClassIcon.LGUICanvas", new IMAGE_BRUSH(TEXT("LGUICanvas_16x"), Icon16x16));
	Style->Set("ClassIcon.LGUICanvasScaler", new IMAGE_BRUSH(TEXT("CanvasScaler_16x"), Icon16x16));
	Style->Set("ClassIcon.UISpriteBase", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("ClassIcon.UIText", new IMAGE_BRUSH(TEXT("UIText_16x"), Icon16x16));
	Style->Set("ClassIcon.UITexture", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.UIItem", new IMAGE_BRUSH(TEXT("UIItem_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPolygon", new IMAGE_BRUSH(TEXT("UIPolygon_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPolygonLine", new IMAGE_BRUSH(TEXT("UIPolygonLine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineRaw", new IMAGE_BRUSH(TEXT("UILine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineRendererBase", new IMAGE_BRUSH(TEXT("UILine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineChildrenAsPoints", new IMAGE_BRUSH(TEXT("UILineChildrenAsPoints_16x"), Icon16x16));
	Style->Set("ClassIcon.UIRing", new IMAGE_BRUSH(TEXT("UIRing_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureBase", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPostProcessRenderable", new IMAGE_BRUSH(TEXT("UIPostProcess_16x"), Icon16x16));
	Style->Set("ClassIcon.LGUILifeCycleUIBehaviour", new IMAGE_BRUSH(TEXT("LGUILifeCycleBehaviour_16x"), Icon16x16));
	Style->Set("ClassIcon.LGUILifeCycleBehaviour", new IMAGE_BRUSH(TEXT("LGUILifeCycleBehaviour_16x"), Icon16x16));
	Style->Set("ClassIcon.UICanvasGroup", new IMAGE_BRUSH(TEXT("UICanvasGroup_16x"), Icon16x16));
	Style->Set("ClassIcon.UIProceduralRect", new IMAGE_BRUSH(TEXT("UIProceduralRect_16x"), Icon16x16));
	Style->Set("ClassIcon.LGUIWidget", new IMAGE_BRUSH(TEXT("LGUIWidget_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LGUIEventSystemActor", new IMAGE_BRUSH(TEXT("EventSystem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LGUIEventSystem", new IMAGE_BRUSH(TEXT("EventSystem_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIEventSystemActor", new IMAGE_BRUSH(TEXT("EventSystem_16x"), Icon16x16));
	Style->Set("ClassIcon.LGUIEventSystem", new IMAGE_BRUSH(TEXT("EventSystem_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LGUIPrefab", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIPrefab", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LGUIPrefabActor", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIPrefabActor", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LGUISpriteData", new IMAGE_BRUSH(TEXT("Sprite_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUISpriteData", new IMAGE_BRUSH(TEXT("Sprite_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LGUIBaseInputModule", new IMAGE_BRUSH(TEXT("InputModule_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIBaseInputModule", new IMAGE_BRUSH(TEXT("InputModule_16x"), Icon16x16));

	Style->Set("LGUIEditor.SpriteDataAction", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("LGUIEditor.SpriteDataCreate", new IMAGE_BRUSH(TEXT("SpriteDataCreate_16x"), Icon16x16));
	Style->Set("LGUIEditor.SpriteDataSetting", new IMAGE_BRUSH(TEXT("SpriteDataSetting_16x"), Icon16x16));
	Style->Set("LGUIEditor.PrefabDataAction", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));

	Style->Set("LGUIEditor.WhiteFrame", new BOX_BRUSH(TEXT("WhiteFrame_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("LGUIEditor.WhiteFrameHorizontal", new BOX_BRUSH(TEXT("WhiteFrameHorizontal_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("LGUIEditor.WhiteFrameVertical", new BOX_BRUSH(TEXT("WhiteFrameVertical_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("LGUIEditor.WhiteDot", new IMAGE_BRUSH(TEXT("WhiteDot_1x"), FVector2D(1, 1)));
	Style->Set("LGUIEditor.AnchorData_Dot", new IMAGE_BRUSH(TEXT("AnchorData_Dot"), FVector2D(3, 3)));

	Style->Set("LGUIEditor.EventGroup", new BOX_BRUSH(TEXT("EventGroup"), FMargin(15.0f / 30.0f, 34.0f / 40.0f, 15.0f / 30.0f, 6.0f / 40.0f)));
	Style->Set("LGUIEditor.EventItem", new BOX_BRUSH(TEXT("EventItem"), FVector2D(26, 26), 12.0f / 26.0f));

	Style->Set("LGUIEditor.EditorTools", new IMAGE_BRUSH(TEXT("Button_Icon40"), FVector2D(40, 40)));

	FButtonStyle AnchorButton = FButtonStyle()
		.SetNormal(BOX_BRUSH(TEXT("AnchorData_Button_Normal"), FVector2D(16, 16), 4.0f / 16.0f))
		.SetDisabled(BOX_BRUSH(TEXT("AnchorData_Button_Normal"), FVector2D(16, 16), 4.0f / 16.0f))
		.SetHovered(BOX_BRUSH(TEXT("WhiteFrameHover_1x"), FVector2D(16, 16), 4.0f / 16.0f))
		.SetPressed(BOX_BRUSH(TEXT("WhiteFramePress_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("AnchorButton", AnchorButton);

	FButtonStyle EmptyButton = FButtonStyle()
		.SetNormal(FSlateColorBrush(FColor(0, 39, 131, 0)))
		.SetHovered(FSlateColorBrush(FColor(0, 39, 131, 64)))
		.SetPressed(FSlateColorBrush(FColor(0, 39, 131, 128)));
	Style->Set("EmptyButton", EmptyButton);
	Style->Set("PrefabMarkWhite", new IMAGE_BRUSH("PrefabMarkWhite_16x", Icon16x16));
	Style->Set("PrefabPlusMarkWhite", new IMAGE_BRUSH("PrefabPlusMarkWhite_16x", Icon16x16));
	Style->Set("PrefabVariantMarkWhite", new IMAGE_BRUSH("PrefabVariantMarkWhite_16x", Icon16x16));
	Style->Set("PrefabMarkBroken", new IMAGE_BRUSH("PrefabMarkBroken_16x", Icon16x16));
	Style->Set("CanvasMark", new IMAGE_BRUSH("CanvasMark_16x", Icon16x16));

	return Style;
}

#undef IMAGE_BRUSH
#undef BOX_BRUSH
#undef BORDER_BRUSH
#undef TTF_FONT
#undef OTF_FONT

void FLGUIEditorStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FLGUIEditorStyle::Get()
{
	return *StyleInstance;
}
