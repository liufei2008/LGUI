// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIDataAsTexture.h"
#include "LGUI.h"
#include "Utils/LexUIUtils.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "TextureResource.h"
#include "Engine/Texture2DDynamic.h"
#include "RenderingThread.h"

#define LOCTEXT_NAMESPACE "LWidgetDataAsTexture"

#if WITH_EDITOR
void ULexUIDataAsTexture::PreEditChange(FProperty* PropertyAboutToChange)
{
	Super::PreEditChange(PropertyAboutToChange);
}
void ULexUIDataAsTexture::PostEditChangeProperty(struct FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void ULexUIDataAsTexture::BeginDestroy()
{
	Super::BeginDestroy();
}
void ULexUIDataAsTexture::CreateTexture()
{
	static int TextureNameSuffix = 0;
	auto TextureDynamic = NewObject<UTexture2DDynamic>(
		this,
		FName(*FString::Printf(TEXT("LexUIDataAsTexture_%d"), TextureNameSuffix++))
	);
	TextureDynamic->LODGroup = TEXTUREGROUP_UI;
	EPixelFormat GraphicPixelFormat;
	switch (PixelFormat)
	{
	default:
	case ELexUIDataAsTexturePixelFormat::R8:
		TextureDynamic->CompressionSettings = TC_Grayscale;
		GraphicPixelFormat = PF_R8;
		break;
	case ELexUIDataAsTexturePixelFormat::R16:
		TextureDynamic->CompressionSettings = TC_HalfFloat;
		GraphicPixelFormat = PF_R16F;
		break;
	case ELexUIDataAsTexturePixelFormat::R32:
		TextureDynamic->CompressionSettings = TC_SingleFloat;
		GraphicPixelFormat = PF_R32_FLOAT;
		break;
	case ELexUIDataAsTexturePixelFormat::R8G8B8A8:
		TextureDynamic->CompressionSettings = TC_VectorDisplacementmap;
		GraphicPixelFormat = PF_R8G8B8A8;
		break;
	case ELexUIDataAsTexturePixelFormat::R16G16B16A16:
		TextureDynamic->CompressionSettings = TC_HDR;
		GraphicPixelFormat = PF_A16B16G16R16;
		break;
	case ELexUIDataAsTexturePixelFormat::R32G32B32A32:
		TextureDynamic->CompressionSettings = TC_HDR_F32;
		GraphicPixelFormat = PF_A32B32G32R32F;
		break;
	}
	TextureDynamic->SRGB = false;
	TextureDynamic->Init(TextureWidth, TextureHeight, GraphicPixelFormat, false);
	if (TextureDynamic->GetResource())
	{
		auto TextureRes = (FTexture2DDynamicResource*)TextureDynamic->GetResource();
		ENQUEUE_RENDER_COMMAND(FLexUIDataAsTexture_ZeroMemory)(
			[TextureRes, Width = TextureWidth, Height = TextureHeight, BytesPerPixel = BytesPerPixel](FRHICommandListImmediate& RHICmdList)
			{
				TArray<uint8> Data;
				Data.SetNumZeroed(Width * Height * BytesPerPixel);
				RHICmdList.UpdateTexture2D(
					TextureRes->GetTexture2DRHI(),
					0,
					FUpdateTextureRegion2D(0, 0, 0, 0, Width, Height),
					BytesPerPixel * Width,
					Data.GetData()
				);
			});
	}

	Texture = TextureDynamic;
}
bool ULexUIDataAsTexture::ExpandTextureWidth()
{
	uint32 NewTextureWidth = TextureWidth + TextureWidth;
	UE_LOG(LGUI, Log, TEXT("[%s].%d Will Expand Texture Width to %d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, NewTextureWidth)
	if (NewTextureWidth > GetMax2DTextureDimension())
	{
		auto WarningMsg = FText::Format(LOCTEXT("BufferTexture_Size_Error", "{0} Trying to expand buffer texture, result too large size that not supported! Maximum texture size is:{1}. This may happen when there are too many LexVisual!")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
			, GetMax2DTextureDimension());
		UE_LOG(LGUI, Error, TEXT("%s"), *WarningMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(WarningMsg, false);
#endif
		return false;
	}
	auto OldTexture = Texture;
	auto OldTextureWidth = TextureWidth;
	TextureWidth = NewTextureWidth;
	CreateTexture();

	//copy existing data
	auto NewTexture = Texture;
	if (OldTexture->GetResource() != nullptr && NewTexture->GetResource() != nullptr)
	{
		ENQUEUE_RENDER_COMMAND(FLFLexUIDataAsTexture_UpdateAndCopyDataTexture)(
			[OldTexture, NewTexture, Width = OldTextureWidth, Height = TextureHeight](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(Width, Height, 0);
				CopyInfo.DestPosition = FIntVector(0, 0, 0);
				RHICmdList.CopyTexture(
					((FTexture2DDynamicResource*)OldTexture->GetResource())->GetTexture2DRHI(),
					((FTexture2DDynamicResource*)NewTexture->GetResource())->GetTexture2DRHI(),
					CopyInfo
				);
				RHICmdList.FlushResources();//Flush resource, or the texture will not show correct result
			});
	}
	// set start position to bottom
	CurrentPosition.X += BlockPixelCount;
	CurrentPosition.Y = 0;

	OnDataTextureChange.Broadcast(Texture);

	return true;
}
bool ULexUIDataAsTexture::ExpandTextureHeight()
{
	uint32 NewTextureHeight = TextureHeight + TextureHeight;
	UE_LOG(LGUI, Log, TEXT("[%s].%d Will Expand Texture Height to %d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__, NewTextureHeight)
	if (NewTextureHeight > GetMax2DTextureDimension())
	{
		auto WarningMsg = FText::Format(LOCTEXT("BufferTexture_Size_Error", "{0} Trying to expand buffer texture, result too large size that not supported! Maximum texture size is:{1}. This may happen when there are too many LexVisual!")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
			, GetMax2DTextureDimension());
		UE_LOG(LGUI, Error, TEXT("%s"), *WarningMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(WarningMsg, false);
#endif
		return false;
	}
	auto OldTexture = Texture;
	auto OldTextureHeight = TextureHeight;
	TextureHeight = NewTextureHeight;
	CreateTexture();

	//copy existing data
	auto NewTexture = Texture;
	if (OldTexture->GetResource() != nullptr && NewTexture->GetResource() != nullptr)
	{
		ENQUEUE_RENDER_COMMAND(FLFLexUIDataAsTexture_UpdateAndCopyDataTexture)(
			[OldTexture, NewTexture, Width = TextureWidth, Height = OldTextureHeight](FRHICommandListImmediate& RHICmdList)
			{
				FRHICopyTextureInfo CopyInfo;
				CopyInfo.SourcePosition = FIntVector(0, 0, 0);
				CopyInfo.Size = FIntVector(Width, Height, 0);
				CopyInfo.DestPosition = FIntVector(0, 0, 0);
				RHICmdList.CopyTexture(
					((FTexture2DDynamicResource*)OldTexture->GetResource())->GetTexture2DRHI(),
					((FTexture2DDynamicResource*)NewTexture->GetResource())->GetTexture2DRHI(),
					CopyInfo
				);
				RHICmdList.FlushResources();//Flush resource, or the texture will not show correct result
			});
	}
	// set start position to bottom
	CurrentPosition.Y = OldTextureHeight;

	OnDataTextureChange.Broadcast(Texture);

	return true;
}

void ULexUIDataAsTexture::Init(int InBlockSizeInByte, ELexUIDataAsTexturePixelFormat InPixelFormat, int InInitialTextureHeight, int InMaxTextureSize)
{
	if (bIsInitialized)
	{
		return;
	}
	bIsInitialized = true;
	TextureMaxSize = FMath::RoundUpToPowerOfTwo(InMaxTextureSize);
	TextureMaxSize = FMath::Min(TextureMaxSize, (int32)GetMax2DTextureDimension());
	BlockSizeInByte = InBlockSizeInByte;
	PixelFormat = InPixelFormat;
	switch (PixelFormat)
	{
	case ELexUIDataAsTexturePixelFormat::R8:
		BytesPerPixel = 1;
		break;
	case ELexUIDataAsTexturePixelFormat::R16:
		BytesPerPixel = 2;
		break;
	case ELexUIDataAsTexturePixelFormat::R32:
		BytesPerPixel = 4;
		break;
	case ELexUIDataAsTexturePixelFormat::R8G8B8A8:
		BytesPerPixel = 4;
		break;
	case ELexUIDataAsTexturePixelFormat::R16G16B16A16:
		BytesPerPixel = 8;
		break;
	case ELexUIDataAsTexturePixelFormat::R32G32B32A32:
		BytesPerPixel = 16;
		break;
	}
	BlockPixelCount = BlockSizeInByte / BytesPerPixel + ((BlockSizeInByte % BytesPerPixel) > 0 ? 1 : 0);
	TextureWidth = FLexUIUtils::CeilPowerOfTwo(BlockPixelCount);
	TextureHeight = InInitialTextureHeight;
	CreateTexture();
}

int ULexUIDataAsTexture::RegisterBuffer()
{
	if (NotUsingPositionArray.Num() > 0)
	{
		auto Pos = NotUsingPositionArray[0];
		NotUsingPositionArray.RemoveSwap(Pos);
		return Pos.X * TextureHeight + Pos.Y;
	}
	if (CurrentPosition.X + BlockPixelCount >= TextureMaxSize)//position x exceed
	{
		auto WarningMsg = FText::Format(LOCTEXT("RegisterBuffer_Error", "{0} Trying to register buffer but exceed capacity! Buffer position:{1}, capacity:{2}! This may happen when there are too many LexVisual!")
			, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__))
			, FText::FromString(CurrentPosition.ToString())
			, TextureMaxSize);
		UE_LOG(LGUI, Error, TEXT("%s"), *WarningMsg.ToString());
#if WITH_EDITOR
		FLexUIUtils::EditorNotification(WarningMsg, false);
#endif
		return 0;
	}
	auto PrevPos = CurrentPosition;
	if (CurrentPosition.Y + 1 >= TextureMaxSize)//position y exceed, and x not exceed, then expand texture width
	{
		if (ExpandTextureWidth())
		{
			return PrevPos.X * TextureHeight + PrevPos.Y;
		}
		return 0;
	}
	CurrentPosition.Y += 1;
	if (CurrentPosition.Y >= TextureHeight)//need to expand texture height
	{
		if (ExpandTextureHeight())
		{
			return PrevPos.X * TextureHeight + PrevPos.Y;
		}
		return 0;
	}
	return PrevPos.X * TextureHeight + PrevPos.Y;
}
void ULexUIDataAsTexture::UnregisterBuffer(int InPosition)
{
	NotUsingPositionArray.Add(FIntVector2(InPosition / TextureHeight, InPosition % TextureHeight));
}
void ULexUIDataAsTexture::UpdateBlock(int InBufferPosition, TArray<uint8> InData)
{
	if (bBatchUpdateMode)
	{
		FPendingUpdateData Data;
		Data.PosX = InBufferPosition / TextureMaxSize;
		Data.PosY = InBufferPosition % TextureMaxSize;
		Data.Data = MoveTemp(InData);
		Data.DataPixelCount = this->BlockPixelCount;
		PendingUpdateDataArray.Add(MoveTemp(Data));
	}
	else
	{
		if (IsValid(Texture) && Texture->GetResource())
		{
			auto PosX = InBufferPosition / TextureMaxSize;
			auto PosY = InBufferPosition % TextureMaxSize;
			auto TextureRes = (FTexture2DDynamicResource*)Texture->GetResource();
			ENQUEUE_RENDER_COMMAND(FLexUIDataAsTexture_UpdateBlock)(
				[TextureRes, PosX, PosY, InData = MoveTemp(InData), BlockSizeInByte = this->BlockSizeInByte, BlockPixelCount = this->BlockPixelCount](FRHICommandListImmediate& RHICmdList)
				{
					RHICmdList.UpdateTexture2D(
						TextureRes->GetTexture2DRHI(),
						0,
						FUpdateTextureRegion2D(PosX, PosY, 0, 0, BlockPixelCount, 1),
						BlockSizeInByte,
						InData.GetData()
					);
				});
		}
	}
}

void ULexUIDataAsTexture::UpdateBlock(int InBufferPositionXOffset, int InBufferPosition, TArray<uint8> InData, int InDataPixelCount)
{
	if (bBatchUpdateMode)
	{
		FPendingUpdateData Data;
		Data.PosX = InBufferPositionXOffset + InBufferPosition / TextureMaxSize;
		Data.PosY = InBufferPosition % TextureMaxSize;
		Data.Data = MoveTemp(InData);
		Data.DataPixelCount = InDataPixelCount;
		PendingUpdateDataArray.Add(MoveTemp(Data));
	}
	else
	{
		if (IsValid(Texture) && Texture->GetResource())
		{
			auto PosX = InBufferPositionXOffset + InBufferPosition / TextureMaxSize;
			auto PosY = InBufferPosition % TextureMaxSize;
			auto TextureRes = (FTexture2DDynamicResource*)Texture->GetResource();
			ENQUEUE_RENDER_COMMAND(FLexUIDataAsTexture_UpdateBlock)(
				[TextureRes, PosX, PosY, InData = MoveTemp(InData), BlockSizeInByte = this->BlockSizeInByte, InDataPixelCount](FRHICommandListImmediate& RHICmdList)
				{
					RHICmdList.UpdateTexture2D(
						TextureRes->GetTexture2DRHI(),
						0,
						FUpdateTextureRegion2D(PosX, PosY, 0, 0, InDataPixelCount, 1),
						BlockSizeInByte,
						InData.GetData()
					);
				});
		}
	}
}

void ULexUIDataAsTexture::PrepareForBatchUpdate()
{
	check(!bBatchUpdateMode);
	bBatchUpdateMode = true;
}

void ULexUIDataAsTexture::Flush()
{
	check(bBatchUpdateMode);
	bBatchUpdateMode = false;
	if (PendingUpdateDataArray.Num() <= 0)return;
	if (IsValid(Texture) && Texture->GetResource())
	{
		auto TextureRes = (FTexture2DDynamicResource*)Texture->GetResource();
		ENQUEUE_RENDER_COMMAND(FLexUIDataAsTexture_FlushData)(
			[TextureRes, PendingUpdateDataArray = MoveTemp(PendingUpdateDataArray), BlockSizeInByte = this->BlockSizeInByte](FRHICommandListImmediate& RHICmdList)
			{
				for (auto& PendingUpdateData : PendingUpdateDataArray)
				{
					RHICmdList.UpdateTexture2D(
						TextureRes->GetTexture2DRHI(),
						0,
						FUpdateTextureRegion2D(PendingUpdateData.PosX, PendingUpdateData.PosY, 0, 0, PendingUpdateData.DataPixelCount, 1),
						BlockSizeInByte,
						PendingUpdateData.Data.GetData()
					);
				}
			});
	}
	else
	{
		PendingUpdateDataArray.Reset();
	}
}

void ULexUIDataAsTexture::PostInitProperties()
{
	Super::PostInitProperties();
}

#undef LOCTEXT_NAMESPACE
