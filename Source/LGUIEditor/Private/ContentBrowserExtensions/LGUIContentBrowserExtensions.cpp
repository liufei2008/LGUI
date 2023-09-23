// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "ContentBrowserExtensions/LGUIContentBrowserExtensions.h"
#include "EngineModule.h"
#include "Engine/EngineTypes.h"
#include "Core/LGUISpriteData.h"
#include "Engine/Texture2D.h"
#include "IContentBrowserSingleton.h"
#include "ContentBrowserModule.h"
#include "LGUIEditorStyle.h"
#include "DataFactory/LGUISpriteDataFactory.h"
#include "DataFactory/LGUIPrefabFactory.h"
#include "PrefabSystem/LGUIPrefab.h"

#define LOCTEXT_NAMESPACE "LGUI"

//////////////////////////////////////////////////////////////////////////

FContentBrowserMenuExtender_SelectedAssets ContentBrowserExtenderDelegate;
FDelegateHandle ContentBrowserExtenderDelegateHandle;

//////////////////////////////////////////////////////////////////////////
// FCreateSpriteFromTextureExtension

#include "IAssetTools.h"
#include "AssetToolsModule.h"


//////////////////////////////////////////////////////////////////////////
// FLGUIContentBrowserExtensions_Impl

class FLGUIContentBrowserExtensions_Impl
{
public:
	static void CreateSpriteActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<UTexture2D*> SelectedAssets)
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("SpriteActionsSubMenuLabel", "LGUISprite"),
			LOCTEXT("SpriteActionsSubMenuToolTip", "Sprite-related actions for this texture."),
			FNewMenuDelegate::CreateStatic(&FLGUIContentBrowserExtensions_Impl::PopulateSpriteActionsMenu, SelectedAssets),
			false,
			FSlateIcon(FLGUIEditorStyle::GetStyleSetName(), "LGUIEditor.SpriteDataAction")
		);
	}
	static void CreatePrefabActionsSubMenu(FMenuBuilder& MenuBuilder, TArray<ULGUIPrefab*> SelectedAssets)
	{
		MenuBuilder.AddSubMenu(
			LOCTEXT("PrefabActionsSubMenuLabel", "LGUIPrefab"),
			LOCTEXT("PrefabActionsSubMenuToolTip", "Prefab-related actions for this prefab."),
			FNewMenuDelegate::CreateStatic(&FLGUIContentBrowserExtensions_Impl::PopulatePrefabActionMenu, SelectedAssets),
			false,
			FSlateIcon(FLGUIEditorStyle::GetStyleSetName(), "LGUIEditor.PrefabDataAction")
		);
	}

	static void PopulateSpriteActionsMenu(FMenuBuilder& MenuBuilder, TArray<UTexture2D*> SelectedAssets)
	{
		// Create sprites
		struct LOCAL
		{
			static void CreateSpritesFromTextures(TArray<UTexture2D*> Textures)
			{
				FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
				FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

				TArray<UObject*> ObjectsToSync;

				for (auto Texture : Textures)
				{
					// Create the factory used to generate the sprite
					ULGUISpriteDataFactory* SpriteFactory = NewObject<ULGUISpriteDataFactory>();
					SpriteFactory->SpriteTexture = Texture;

					// Create the sprite
					FString Name;
					FString PackageName;

					// Get a unique name for the sprite
					const FString DefaultSuffix = TEXT("_Sprite");
					AssetToolsModule.Get().CreateUniqueAssetName(Texture->GetOutermost()->GetName(), DefaultSuffix, /*out*/ PackageName, /*out*/ Name);
					const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

					if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, PackagePath, ULGUISpriteData::StaticClass(), SpriteFactory))
					{
						ObjectsToSync.Add(NewAsset);
					}
				}

				if (ObjectsToSync.Num() > 0)
				{
					ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
				}
			}
			static void ConfigureTextureSettingsForSprites(TArray<UTexture2D*> InTextures)
			{
				// Change the compression settings and trigger a recompress
				for (auto Texture : InTextures)
				{
					ULGUISpriteData::CheckAndApplySpriteTextureSetting(Texture);
				}
			}
		};

		const FName LGUIStyleSetName = FLGUIEditorStyle::GetStyleSetName();
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CreateSprite", "Create Sprite"),
			LOCTEXT("CreateSprite_Tooltip", "Create sprites from selected textures"),
			FSlateIcon(LGUIStyleSetName, "LGUIEditor.SpriteDataCreate"),
			FUIAction(FExecuteAction::CreateStatic(&LOCAL::CreateSpritesFromTextures, SelectedAssets)),
			NAME_None,
			EUserInterfaceActionType::Button);

		MenuBuilder.AddMenuEntry(
			LOCTEXT("ConfigureTextureForSprites", "Apply Sprite Texture Settings"),
			LOCTEXT("ConfigureTextureForSprites_Tooltip", "Set texture for sprite"),
			FSlateIcon(LGUIStyleSetName, "LGUIEditor.SpriteDataSetting"),
			FUIAction(FExecuteAction::CreateStatic(&LOCAL::ConfigureTextureSettingsForSprites, SelectedAssets)),
			NAME_None,
			EUserInterfaceActionType::Button);
	}

	static void PopulatePrefabActionMenu(FMenuBuilder& MenuBuilder, TArray<ULGUIPrefab*> SelectedAssets)
	{
		struct LOCAL
		{
			static void CreatePrefabVariant(TArray<ULGUIPrefab*> Prefabs)
			{
				FAssetToolsModule& AssetToolsModule = FModuleManager::Get().LoadModuleChecked<FAssetToolsModule>("AssetTools");
				FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");

				TArray<UObject*> ObjectsToSync;

				for (auto Prefab : Prefabs)
				{
					// Create the factory used to generate the prefab
					auto PrefabFactory = NewObject<ULGUIPrefabFactory>();
					PrefabFactory->SourcePrefab = Prefab;

					// Create the prefab
					FString Name;
					FString PackageName;

					// Get a unique name for the prefab
					const FString DefaultSuffix = TEXT("_Variant");
					AssetToolsModule.Get().CreateUniqueAssetName(Prefab->GetOutermost()->GetName(), DefaultSuffix, /*out*/ PackageName, /*out*/ Name);
					const FString PackagePath = FPackageName::GetLongPackagePath(PackageName);

					if (UObject* NewAsset = AssetToolsModule.Get().CreateAsset(Name, PackagePath, ULGUIPrefab::StaticClass(), PrefabFactory))
					{
						ObjectsToSync.Add(NewAsset);
					}
				}

				if (ObjectsToSync.Num() > 0)
				{
					ContentBrowserModule.Get().SyncBrowserToAssets(ObjectsToSync);
				}
			}
		};

		const FName LGUIStyleSetName = FLGUIEditorStyle::GetStyleSetName();
		MenuBuilder.AddMenuEntry(
			LOCTEXT("CreatePrefabVariant", "Create PrefabVariant"),
			LOCTEXT("CreatePrefabVariant_Tooltip", "Create variant prefab using this prefab."),
			FSlateIcon(LGUIStyleSetName, "LGUIEditor.PrefabDataAction"),
			FUIAction(FExecuteAction::CreateStatic(&LOCAL::CreatePrefabVariant, SelectedAssets)),
			NAME_None,
			EUserInterfaceActionType::Button);
	}

	static TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
	{
		TSharedRef<FExtender> Extender(new FExtender());

		// Run thru the assets to determine if any meet our criteria
		TArray<UTexture2D*> Textures;
		TArray<ULGUIPrefab*> Prefabs;
		for (auto AssetIt = SelectedAssets.CreateConstIterator(); AssetIt; ++AssetIt)
		{
			const FAssetData& Asset = *AssetIt;
			auto AssetObject = Asset.GetAsset();
			if (auto Texture = Cast<UTexture2D>(AssetObject))
			{
				Textures.Add(Texture);
			}
			else if (auto Prefab = Cast<ULGUIPrefab>(AssetObject))
			{
				Prefabs.Add(Prefab);
			}
		}

		if (Textures.Num() > 0)
		{
			// Add the sprite actions sub-menu extender
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FLGUIContentBrowserExtensions_Impl::CreateSpriteActionsSubMenu, Textures));
		}
		if (Prefabs.Num() > 0)
		{
			Extender->AddMenuExtension(
				"GetAssetActions",
				EExtensionHook::After,
				nullptr,
				FMenuExtensionDelegate::CreateStatic(&FLGUIContentBrowserExtensions_Impl::CreatePrefabActionsSubMenu, Prefabs));
		}

		return Extender;
	}

	static TArray<FContentBrowserMenuExtender_SelectedAssets>& GetExtenderDelegates()
	{
		FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>(TEXT("ContentBrowser"));
		return ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	}
};

//////////////////////////////////////////////////////////////////////////
// FLGUIContentBrowserExtensions

void FLGUIContentBrowserExtensions::InstallHooks()
{
	ContentBrowserExtenderDelegate = FContentBrowserMenuExtender_SelectedAssets::CreateStatic(&FLGUIContentBrowserExtensions_Impl::OnExtendContentBrowserAssetSelectionMenu);

	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FLGUIContentBrowserExtensions_Impl::GetExtenderDelegates();
	CBMenuExtenderDelegates.Add(ContentBrowserExtenderDelegate);
	ContentBrowserExtenderDelegateHandle = CBMenuExtenderDelegates.Last().GetHandle();
}

void FLGUIContentBrowserExtensions::RemoveHooks()
{
	if (FModuleManager::Get().IsModuleLoaded("ContentBrowser"))
	{
		TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = FLGUIContentBrowserExtensions_Impl::GetExtenderDelegates();
		CBMenuExtenderDelegates.RemoveAll([](const FContentBrowserMenuExtender_SelectedAssets& Delegate) { return Delegate.GetHandle() == ContentBrowserExtenderDelegateHandle; });
	}
}

//////////////////////////////////////////////////////////////////////////

#undef LOCTEXT_NAMESPACE