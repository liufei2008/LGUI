// Copyright 2019-2022 LexLiu. All Rights Reserved.

#include "DataFactory/LGUISpriteDataFactory.h"
#include "LGUIEditorModule.h"
#include "Core/LGUISettings.h"
#include "Core/LGUISpriteData.h"
#include "Framework/Notifications/NotificationManager.h"
#include "Widgets/Notifications/SNotificationList.h"
#include "Sound/SoundCue.h"

#define LOCTEXT_NAMESPACE "LGUISpriteDataFactory"


ULGUISpriteDataFactory::ULGUISpriteDataFactory()
{
	SupportedClass = ULGUISpriteData::StaticClass();
	bCreateNew = true;
	bEditAfterNew = true;
}
UObject* ULGUISpriteDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags, UObject* Context, FFeedbackContext* Warn)
{
	bool isDefaltTexture = false;
	if (SpriteTexture == nullptr)
	{
		SpriteTexture = LoadObject<UTexture2D>(NULL, TEXT("/LGUI/Textures/LGUIPreset_WhiteSolid"));
		isDefaltTexture = true;
	}
	// check size
	if (SpriteTexture && !isDefaltTexture)
	{
		int32 atlasPadding = 0;
		auto lguiSetting = GetDefault<ULGUISettings>()->defaultAtlasSetting.spaceBetweenSprites;
		if (SpriteTexture->GetSurfaceWidth() + atlasPadding * 2 > WARNING_ATLAS_SIZE || SpriteTexture->GetSurfaceHeight() + atlasPadding * 2 > WARNING_ATLAS_SIZE)
		{
			auto LogMsg = LOCTEXT("TextureSizeError", "Target texture width or height is too large! Consider use UITexture to render this texture.");
			UE_LOG(LGUIEditor, Error, TEXT("%s"), *(LogMsg.ToString()));
			FNotificationInfo Info(LogMsg);
			Info.Image = FEditorStyle::GetBrush(TEXT("LevelEditor.RecompileGameCode"));
			Info.FadeInDuration = 0.1f;
			Info.FadeOutDuration = 0.5f;
			Info.ExpireDuration = 8.0f;
			Info.bUseThrobber = false;
			Info.bUseSuccessFailIcons = true;
			Info.bUseLargeFont = true;
			Info.bFireAndForget = false;
			Info.bAllowThrottleWhenFrameRateIsLow = false;
			auto NotificationItem = FSlateNotificationManager::Get().AddNotification(Info);
			NotificationItem->SetCompletionState(SNotificationItem::CS_Success);
			NotificationItem->ExpireAndFadeout();

			auto CompileFailSound = LoadObject<USoundBase>(NULL, TEXT("/Engine/EditorSounds/Notifications/CompileFailed_Cue.CompileFailed_Cue"));
			GEditor->PlayEditorSound(CompileFailSound);

			return nullptr;
		}
		// Apply setting for sprite creation
		//SpriteTexture->MipGenSettings = TextureMipGenSettings::TMGS_NoMipmaps;
		ULGUISpriteData::CheckAndApplySpriteTextureSetting(SpriteTexture);
	}

	ULGUISpriteData* NewAsset = NewObject<ULGUISpriteData>(InParent, Class, Name, Flags | RF_Transactional);
	if (SpriteTexture)
	{
		NewAsset->spriteTexture = SpriteTexture;
		NewAsset->spriteInfo.width = SpriteTexture->GetSurfaceWidth();
		NewAsset->spriteInfo.height = SpriteTexture->GetSurfaceHeight();
	}
	return NewAsset;
}

#undef LOCTEXT_NAMESPACE
