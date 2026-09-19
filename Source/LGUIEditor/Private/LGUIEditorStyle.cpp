// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "LGUIEditorStyle.h"
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

FString UMG_Brush_RootToContentDir(const FString& RelativePath, const TCHAR* Extension)
{
	return (FPaths::EngineContentDir() / TEXT("Editor/Slate/UMG") / RelativePath) + Extension;
}

#define IMAGE_BRUSH( RelativePath, ... ) FSlateImageBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BOX_BRUSH( RelativePath, ... ) FSlateBoxBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define BORDER_BRUSH( RelativePath, ... ) FSlateBorderBrush( Style->RootToContentDir( RelativePath, TEXT(".png") ), __VA_ARGS__ )
#define UMG_ICON( RelativePath, ... ) FSlateVectorImageBrush( UMG_Brush_RootToContentDir( RelativePath, TEXT(".svg")), __VA_ARGS__ )
#define TTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".ttf") ), __VA_ARGS__ )
#define OTF_FONT( RelativePath, ... ) FSlateFontInfo( Style->RootToContentDir( RelativePath, TEXT(".otf") ), __VA_ARGS__ )

const FVector2D Icon16x16(16.0f, 16.0f);
const FVector2D Icon20x20(20.0f, 20.0f);
const FVector2D Icon40x40(40.0f, 40.0f);

TSharedRef< FSlateStyleSet > FLGUIEditorStyle::Create()
{
	TSharedRef< FSlateStyleSet > Style = MakeShareable(new FSlateStyleSet("LGUIEditorStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("LGUI")->GetBaseDir() / TEXT("Resources/Icons"));

	Style->Set("ClassThumbnail.LexWidgetActor", new IMAGE_BRUSH(TEXT("LexWidget_40x"), Icon40x40));

	Style->Set("ClassIcon.LexWidgetActor", new IMAGE_BRUSH(TEXT("LexWidget_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LexCanvas", new IMAGE_BRUSH(TEXT("LexCanvas_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISpriteBase", new IMAGE_BRUSH(TEXT("UISprite_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexText", new IMAGE_BRUSH(TEXT("LexText_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITexture", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexWidget", new IMAGE_BRUSH(TEXT("LexWidget_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPolygon", new IMAGE_BRUSH(TEXT("UIPolygon_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIPolygonLine", new IMAGE_BRUSH(TEXT("UIPolygonLine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineRaw", new IMAGE_BRUSH(TEXT("UILine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineRendererBase", new IMAGE_BRUSH(TEXT("UILine_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UI2DLineChildrenAsPoints", new IMAGE_BRUSH(TEXT("UILineChildrenAsPoints_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIRing", new IMAGE_BRUSH(TEXT("UIRing_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureBase", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexVisualBackBufferReader", new IMAGE_BRUSH(TEXT("UIPostProcess_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexUIBehaviour", new IMAGE_BRUSH(TEXT("LexUIBehaviour_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexUIMLBehaviour", new IMAGE_BRUSH(TEXT("Xaml_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexRectBlock", new IMAGE_BRUSH(TEXT("LexRectBlock_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIWidget", new IMAGE_BRUSH(TEXT("UIWidget_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIRenderTarget", new IMAGE_BRUSH(TEXT("UIRenderTarget_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UICustomMesh", new IMAGE_BRUSH(TEXT("UICustomMesh_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexUICustomMesh", new IMAGE_BRUSH(TEXT("UICustomMesh_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexUIFontData_BaseObject", new IMAGE_BRUSH(TEXT("Font_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexUIFontEmojiData", new IMAGE_BRUSH(TEXT("FontEmoji_40x"), Icon40x40));

	Style->Set("ClassIcon.LexCanvas", new IMAGE_BRUSH(TEXT("LexCanvas_16x"), Icon16x16));
	Style->Set("ClassIcon.UISpriteBase", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("ClassIcon.LexText", new IMAGE_BRUSH(TEXT("LexText_16x"), Icon16x16));
	Style->Set("ClassIcon.UITexture", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.LexImage", new IMAGE_BRUSH(TEXT("Image_16x"), Icon16x16));
	Style->Set("ClassIcon.LexWidget", new IMAGE_BRUSH(TEXT("LexWidget_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPolygon", new IMAGE_BRUSH(TEXT("UIPolygon_16x"), Icon16x16));
	Style->Set("ClassIcon.UIPolygonLine", new IMAGE_BRUSH(TEXT("UIPolygonLine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineRaw", new IMAGE_BRUSH(TEXT("UILine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineRendererBase", new IMAGE_BRUSH(TEXT("UILine_16x"), Icon16x16));
	Style->Set("ClassIcon.UI2DLineChildrenAsPoints", new IMAGE_BRUSH(TEXT("UILineChildrenAsPoints_16x"), Icon16x16));
	Style->Set("ClassIcon.UIRing", new IMAGE_BRUSH(TEXT("UIRing_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureBase", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.LexVisualBackBufferReader", new IMAGE_BRUSH(TEXT("UIPostProcess_16x"), Icon16x16));
	Style->Set("ClassIcon.LexUIBehaviour", new IMAGE_BRUSH(TEXT("LexUIBehaviour_16x"), Icon16x16));
	Style->Set("ClassIcon.LexUIMLBehaviour", new IMAGE_BRUSH(TEXT("Xaml_16x"), Icon16x16));
	Style->Set("ClassIcon.LexRectBlock", new IMAGE_BRUSH(TEXT("LexRectBlock_16x"), Icon16x16));
	Style->Set("ClassIcon.UIWidget", new IMAGE_BRUSH(TEXT("UIWidget_16x"), Icon16x16));
	Style->Set("ClassIcon.UIRenderTarget", new IMAGE_BRUSH(TEXT("UIRenderTarget_16x"), Icon16x16));
	Style->Set("ClassIcon.UICustomMesh", new IMAGE_BRUSH(TEXT("UICustomMesh_16x"), Icon16x16));
	Style->Set("ClassIcon.LexUICustomMesh", new IMAGE_BRUSH(TEXT("UICustomMesh_16x"), Icon16x16));
	Style->Set("ClassIcon.LexUIFontData_BaseObject", new IMAGE_BRUSH(TEXT("Font_16x"), Icon16x16));
	Style->Set("ClassIcon.LexUIFontEmojiData", new IMAGE_BRUSH(TEXT("FontEmoji_16x"), Icon16x16));
	Style->Set("ClassIcon.UIButton", new UMG_ICON(TEXT("Button"), Icon16x16));
	Style->Set("ClassIcon.UITextInput", new UMG_ICON(TEXT("TextBox"), Icon16x16));
	Style->Set("ClassIcon.UIToggle", new UMG_ICON(TEXT("CheckBox"), Icon16x16));
	Style->Set("ClassIcon.UISlider", new UMG_ICON(TEXT("Slider"), Icon16x16));
	Style->Set("ClassIcon.UIScrollbar", new UMG_ICON(TEXT("ProgressBar"), Icon16x16));
	Style->Set("ClassIcon.UIDropdown", new UMG_ICON(TEXT("ComboBox"), Icon16x16));
	Style->Set("ClassIcon.UIScrollView", new UMG_ICON(TEXT("ScrollBox"), Icon16x16));

	Style->Set("ClassThumbnail.LexUIEventSystemActor", new IMAGE_BRUSH(TEXT("EventSystem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.LexUIEventSystem", new IMAGE_BRUSH(TEXT("EventSystem_40x"), Icon40x40));
	Style->Set("ClassIcon.LexUIEventSystemActor", new IMAGE_BRUSH(TEXT("EventSystem_16x"), Icon16x16));
	Style->Set("ClassIcon.LexUIEventSystem", new IMAGE_BRUSH(TEXT("EventSystem_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LexUIPrefab", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LexUIPrefab", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LexUIPrefabActor", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LexUIPrefabActor", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LexUISpriteData", new IMAGE_BRUSH(TEXT("UISprite_40x"), Icon40x40));
	Style->Set("ClassIcon.LexUISpriteData", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LexUIStaticSpriteAtlasData", new IMAGE_BRUSH(TEXT("SpriteAtlas_40x"), Icon40x40));
	Style->Set("ClassIcon.LexUIStaticSpriteAtlasData", new IMAGE_BRUSH(TEXT("SpriteAtlas_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LexUIBaseInputModule", new IMAGE_BRUSH(TEXT("InputModule_40x"), Icon40x40));
	Style->Set("ClassIcon.LexUIBaseInputModule", new IMAGE_BRUSH(TEXT("InputModule_16x"), Icon16x16));

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

	Style->Set("WidgetSize_ExpandToParent", new IMAGE_BRUSH("WidgetSize_ExpandToParent", Icon16x16));
	Style->Set("WidgetSize_ShrinkToChildren", new IMAGE_BRUSH("WidgetSize_ShrinkToChildren", Icon16x16));
	Style->Set("WidgetSize_ExpandToParent_V", new IMAGE_BRUSH("WidgetSize_ExpandToParent_V", Icon16x16));
	Style->Set("WidgetSize_ShrinkToChildren_V", new IMAGE_BRUSH("WidgetSize_ShrinkToChildren_V", Icon16x16));
	Style->Set("WidgetSize_Off", new IMAGE_BRUSH("WidgetSize_Off", Icon16x16));
	Style->Set("LayoutDirection_Horizontal", new IMAGE_BRUSH("LayoutDirection_Horizontal", Icon16x16));
	Style->Set("LayoutDirection_HorizontalReverse", new IMAGE_BRUSH("LayoutDirection_HorizontalReverse", Icon16x16));
	Style->Set("LayoutDirection_Vertical", new IMAGE_BRUSH("LayoutDirection_Vertical", Icon16x16));
	Style->Set("LayoutDirection_VerticalReverse", new IMAGE_BRUSH("LayoutDirection_VerticalReverse", Icon16x16));
	
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
