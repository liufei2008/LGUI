// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LGUIProceduralRectData.h"
#include "LGUI.h"
#include "Utils/LGUIUtils.h"
#include "Rendering/Texture2DResource.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Engine/Texture2DDynamic.h"
#include "RenderingThread.h"

#define LOCTEXT_NAMESPACE "LGUIProceduralRectData"

#if WITH_EDITOR
void ULGUIProceduralRectData::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
}
void ULGUIProceduralRectData::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif
void ULGUIProceduralRectData::CreateTexture()
{
	auto TextureDynamic = NewObject<UTexture2DDynamic>(
		this,
		FName(*FString::Printf(TEXT("LGUIProceduralRectData_%d"), LGUIUtils::LGUITextureNameSuffix++))
	);
	TextureDynamic->Init(TextureSize, TextureSize, EPixelFormat::PF_R32_FLOAT, false);

	Texture = TextureDynamic;
	Texture->AddToRoot();
}
bool ULGUIProceduralRectData::ExpandTexture()
{
	uint32 NewTextureSize = TextureSize + TextureSize;
	if (NewTextureSize > GetMax2DTextureDimension())
	{
		auto WarningMsg = FText::Format(LOCTEXT("BufferTexture_Size_Error", "{0} Trying to expand buffer texture, result too large size that not supported! Maximun texture size is:{1}.")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
			, GetMax2DTextureDimension());
		UE_LOG(LGUI, Error, TEXT("%s"), *WarningMsg.ToString());
#if WITH_EDITOR
		LGUIUtils::EditorNotification(WarningMsg);
#endif
		return false;
	}
	auto OldTexture = Texture;
	auto OldTextureSize = TextureSize;
	TextureSize = NewTextureSize;
	CreateTexture();

	//copy existing data
	auto NewTexture = Texture;
	if (OldTexture->GetResource() != nullptr && NewTexture->GetResource() != nullptr)
	{
		ENQUEUE_RENDER_COMMAND(FLGUIProceduralRectUpdateAndCopyDataTexture)(
			[OldTexture, NewTexture, OldTextureSize](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(OldTextureSize, OldTextureSize, 0);
				CopyInfo.DestPosition = FIntVector(0, 0, 0);
				RHICmdList.CopyTexture(
					((FTexture2DDynamicResource*)OldTexture->GetResource())->GetTexture2DRHI(),
					((FTexture2DDynamicResource*)NewTexture->GetResource())->GetTexture2DRHI(),
					CopyInfo
				);
				RHICmdList.FlushResources();//Flush resource, or the texture will not show correct result
				OldTexture->RemoveFromRoot();//ready for gc
			});
	}
	else
	{
		OldTexture->RemoveFromRoot();//ready for gc
	}
	// right top quater as not using position
	for (int h = 0; h < OldTextureSize; h += 1)
	{
		for (int w = CurrentPosition.X; w + BlockPixelCount < TextureSize; w += BlockPixelCount)
		{
			NotUsingPositionArray.Add(FIntVector2(w, h));
		}
	}
	// set start position to left bottom quater
	CurrentPosition.X = 0;
	CurrentPosition.Y = OldTextureSize;

	OnDataTextureChange.Broadcast(Texture);

	return true;
}

void ULGUIProceduralRectData::Init(int InBlockSizeInByte)
{
	if (bIsInitialized)
	{
		return;
	}
	bIsInitialized = true;
	BlockSizeInByte = InBlockSizeInByte;
	BlockPixelCount = BlockSizeInByte / 4 + ((BlockSizeInByte % 4) > 0 ? 1 : 0);
	TextureSize = 32;//initial size is 256
	if (BlockPixelCount > TextureSize)
	{
		TextureSize += TextureSize;
	}
	CreateTexture();
}

FIntVector2 ULGUIProceduralRectData::RegisterBuffer()
{
	if (NotUsingPositionArray.Num() > 0)
	{
		auto Pos = NotUsingPositionArray[0];
		NotUsingPositionArray.RemoveSwap(Pos);
		return Pos;
	}
	auto PrevPos = CurrentPosition;
	CurrentPosition.X += BlockPixelCount;
	if (CurrentPosition.X >= TextureSize)
	{
		CurrentPosition.X = 0;
		CurrentPosition.Y += 1;
		PrevPos = CurrentPosition;
		CurrentPosition.X += BlockPixelCount;
	}
	if (CurrentPosition.Y >= TextureSize)//need to expand texture size
	{
		if (ExpandTexture())
		{
			auto Pos = NotUsingPositionArray[0];
			NotUsingPositionArray.RemoveSwap(Pos);
			return Pos;
		}
	}
	return PrevPos;
}
void ULGUIProceduralRectData::UnregisterBuffer(const FIntVector2& InPosition)
{
	NotUsingPositionArray.Add(InPosition);
}
void ULGUIProceduralRectData::UpdateBlock(const FIntVector2& InPosition, uint8* InData)
{
	if (Texture->GetResource())
	{
		auto TextureRes = (FTexture2DDynamicResource*)Texture->GetResource();
		ENQUEUE_RENDER_COMMAND(FLGUIProceduralRectData_UpdateBlock)(
			[TextureRes, InPosition, InData, BlockSizeInByte = this->BlockSizeInByte, BlockPixelCount = this->BlockPixelCount](FRHICommandListImmediate& RHICmdList)
			{
				RHICmdList.UpdateTexture2D(
					TextureRes->GetTexture2DRHI(),
					0,
					FUpdateTextureRegion2D(InPosition.X, InPosition.Y, 0, 0, BlockPixelCount, 1),
					BlockSizeInByte,
					InData
				);
				delete InData;
			});
	}
}

void ULGUIProceduralRectData::PostInitProperties()
{
	Super::PostInitProperties();
	CheckMaterials();
}

void ULGUIProceduralRectData::CheckMaterials()
{
	for (int i = 0; i < (int)ELGUICanvasClipType::Custom; i++)
	{
		if (DefaultMaterials[i] == nullptr)
		{
			FString matPath;
			switch (i)
			{
			default:
			case 0: matPath = TEXT("/LGUI/Materials/LGUI_ProceduralRect_NoClip"); break;
			case 1: matPath = TEXT("/LGUI/Materials/LGUI_ProceduralRect_RectClip"); break;
			case 2: matPath = TEXT("/LGUI/Materials/LGUI_ProceduralRect_TextureClip"); break;
			}
			auto mat = LoadObject<UMaterialInterface>(NULL, *matPath);
			if (mat == nullptr)
			{
				auto errMsg = FText::Format(LOCTEXT("MissingDefaultContent", "{0} Load material error! Missing some content of LGUI plugin, reinstall this plugin may fix the issue.")
					, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
				UE_LOG(LGUI, Error, TEXT("%s"), *errMsg.ToString());
#if WITH_EDITOR
				LGUIUtils::EditorNotification(errMsg, 10);
#endif
				continue;
			}
			DefaultMaterials[i] = mat;
			this->MarkPackageDirty();
		}
	}
}
UMaterialInterface* ULGUIProceduralRectData::GetMaterial(ELGUICanvasClipType clipType)
{
	return DefaultMaterials[(int)clipType];
}

#undef LOCTEXT_NAMESPACE
