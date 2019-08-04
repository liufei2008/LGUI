// Copyright 2019 LexLiu. All Rights Reserved.

#include "LGUIEditorStyle.h"
#include "Slate/SlateGameResources.h"
#include "Interfaces/IPluginManager.h"

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

	Style->Set("ClassThumbnail.UIPanelActor", new IMAGE_BRUSH(TEXT("UIPanel_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISpriteActor", new IMAGE_BRUSH(TEXT("UISprite_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextActor", new IMAGE_BRUSH(TEXT("UIText_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIContainerActor", new IMAGE_BRUSH(TEXT("UIItem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISectorActor", new IMAGE_BRUSH(TEXT("UISector_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIRingActor", new IMAGE_BRUSH(TEXT("UIRing_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));

	Style->Set("ClassIcon.UIPanelActor", new IMAGE_BRUSH(TEXT("UIPanel_16x"), Icon16x16));
	Style->Set("ClassIcon.UISpriteActor", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextActor", new IMAGE_BRUSH(TEXT("UIText_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.UIContainerActor", new IMAGE_BRUSH(TEXT("UIItem_16x"), Icon16x16));
	Style->Set("ClassIcon.UISectorActor", new IMAGE_BRUSH(TEXT("UISector_16x"), Icon16x16));
	Style->Set("ClassIcon.UIRingActor", new IMAGE_BRUSH(TEXT("UIRing_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureActor", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));

	Style->Set("ClassThumbnail.UIPanel", new IMAGE_BRUSH(TEXT("UIPanel_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISpriteBase", new IMAGE_BRUSH(TEXT("UISprite_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIText", new IMAGE_BRUSH(TEXT("UIText_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITexture", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIItem", new IMAGE_BRUSH(TEXT("UIItem_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UISector", new IMAGE_BRUSH(TEXT("UISector_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UIRing", new IMAGE_BRUSH(TEXT("UIRing_40x"), Icon40x40));
	Style->Set("ClassThumbnail.UITextureBase", new IMAGE_BRUSH(TEXT("UITexture_40x"), Icon40x40));

	Style->Set("ClassIcon.UIPanel", new IMAGE_BRUSH(TEXT("UIPanel_16x"), Icon16x16));
	Style->Set("ClassIcon.UISpriteBase", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("ClassIcon.UIText", new IMAGE_BRUSH(TEXT("UIText_16x"), Icon16x16));
	Style->Set("ClassIcon.UITexture", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));
	Style->Set("ClassIcon.UIItem", new IMAGE_BRUSH(TEXT("UIItem_16x"), Icon16x16));
	Style->Set("ClassIcon.UISector", new IMAGE_BRUSH(TEXT("UISector_16x"), Icon16x16));
	Style->Set("ClassIcon.UIRing", new IMAGE_BRUSH(TEXT("UIRing_16x"), Icon16x16));
	Style->Set("ClassIcon.UITextureBase", new IMAGE_BRUSH(TEXT("UITexture_16x"), Icon16x16));

	Style->Set("ClassThumbnail.UIMeshCollider", new IMAGE_BRUSH(TEXT("MeshCollider_40x"), Icon40x40));
	Style->Set("ClassIcon.UIMeshCollider", new IMAGE_BRUSH(TEXT("MeshCollider_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LGUIEventSystemActor", new IMAGE_BRUSH(TEXT("EventSystem_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIEventSystemActor", new IMAGE_BRUSH(TEXT("EventSystem_16x"), Icon16x16));

	Style->Set("ClassThumbnail.LGUIPrefab", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIPrefab", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LGUIPrefabActor", new IMAGE_BRUSH(TEXT("Prefab_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIPrefabActor", new IMAGE_BRUSH(TEXT("Prefab_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LGUIFontData", new IMAGE_BRUSH(TEXT("Font_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUIFontData", new IMAGE_BRUSH(TEXT("Font_16x"), Icon16x16));
	Style->Set("ClassThumbnail.LGUISpriteData", new IMAGE_BRUSH(TEXT("Sprite_40x"), Icon40x40));
	Style->Set("ClassIcon.LGUISpriteData", new IMAGE_BRUSH(TEXT("Sprite_16x"), Icon16x16));

	Style->Set("LGUIEditor.SpriteDataAction", new IMAGE_BRUSH(TEXT("UISprite_16x"), Icon16x16));
	Style->Set("LGUIEditor.SpriteDataCreate", new IMAGE_BRUSH(TEXT("SpriteDataCreate_16x"), Icon16x16));
	Style->Set("LGUIEditor.SpriteDataSetting", new IMAGE_BRUSH(TEXT("SpriteDataSetting_16x"), Icon16x16));

	Style->Set("LGUIEditor.WhiteFrame", new BOX_BRUSH(TEXT("WhiteFrame_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("LGUIEditor.WhiteFrameHorizontal", new BOX_BRUSH(TEXT("WhiteFrameHorizontal_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("LGUIEditor.WhiteFrameVertical", new BOX_BRUSH(TEXT("WhiteFrameVertical_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("LGUIEditor.WhiteDot", new IMAGE_BRUSH(TEXT("WhiteDot_1x"), FVector2D(1, 1)));

	Style->Set("LGUIEditor.EditorTools", new IMAGE_BRUSH(TEXT("Button_Icon40"), FVector2D(40, 40)));

	FButtonStyle AnchorButton = FButtonStyle()
		.SetNormal(BOX_BRUSH(TEXT("WhiteFrame_1x"), FVector2D(16, 16), 4.0f / 16.0f))
		.SetHovered(BOX_BRUSH(TEXT("WhiteFrameHover_1x"), FVector2D(16, 16), 4.0f / 16.0f))
		.SetPressed(BOX_BRUSH(TEXT("WhiteFramePress_1x"), FVector2D(16, 16), 4.0f / 16.0f));
	Style->Set("AnchorButton", AnchorButton);

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
