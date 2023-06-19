// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIProceduralRect.h"
#include "LGUI.h"
#include "Core/UIGeometry.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Core/LGUISpriteInfo.h"
#include "Core/LGUIProceduralRectData.h"
#include "Materials/MaterialInterface.h"
#include "Materials/MaterialInstanceDynamic.h"
#include "Core/UIDrawcall.h"
#include "Core/ActorComponent/UITextureBase.h"

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_DISABLE_OPTIMIZATION
#endif

#define LOCTEXT_NAMESPACE "UIProceduralRect"

FName UUIProceduralRect::DataTextureParameterName = TEXT("DataTexture");

UUIProceduralRect::UUIProceduralRect(const FObjectInitializer& ObjectInitializer):Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UUIProceduralRect::BeginPlay()
{
	Super::BeginPlay();
}

void UUIProceduralRect::OnRegister()
{
	Super::OnRegister();
	if (ProceduralRectData == nullptr)
	{
		ProceduralRectData = LoadObject<ULGUIProceduralRectData>(NULL, TEXT("/LGUI/DefaultProceduralRectData"));
		check(ProceduralRectData != nullptr);
	}
	ProceduralRectData->Init(FUIProceduralRectBlockData::DataCountInBytes());
	DataStartPosition = ProceduralRectData->RegisterBuffer();
	OnDataTextureChangedDelegateHandle = ProceduralRectData->OnDataTextureChange.AddUObject(this, &UUIProceduralRect::OnDataTextureChanged);
}
void UUIProceduralRect::OnUnregister()
{
	Super::OnUnregister();
	ProceduralRectData->UnregisterBuffer(DataStartPosition);
	if (OnDataTextureChangedDelegateHandle.IsValid())
	{
		ProceduralRectData->OnDataTextureChange.Remove(OnDataTextureChangedDelegateHandle);
		OnDataTextureChangedDelegateHandle.Reset();
	}
}

#if WITH_EDITOR
void UUIProceduralRect::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
void UUIProceduralRect::EditorForceUpdate()
{
	Super::EditorForceUpdate();
}
#endif

UTexture* UUIProceduralRect::GetTextureToCreateGeometry()
{
	CheckAdditionalShaderChannels();
	if (!IsValid(BlockData.Texture))
	{
		BlockData.Texture = UUITextureBase::GetDefaultWhiteTexture();
	}
	return BlockData.Texture;
}
UMaterialInterface* UUIProceduralRect::GetMaterialToCreateGeometry()
{
	if (auto Result = Super::GetMaterialToCreateGeometry())
	{
		return Result;
	}
	else
	{
		check(ProceduralRectData);
		CheckAdditionalShaderChannels();
		auto CanvasClipType = ELGUICanvasClipType::None;
		if (this->GetRenderCanvas() != nullptr)
		{
			CanvasClipType = this->GetRenderCanvas()->GetActualClipType();
			if (CanvasClipType == ELGUICanvasClipType::Custom)
			{
				CanvasClipType = ELGUICanvasClipType::None;
			}
		}
		return ProceduralRectData->GetMaterial(CanvasClipType);
	}
}
void UUIProceduralRect::UpdateMaterialClipType()
{
	geometry->material = GetMaterialToCreateGeometry();
	if (drawcall.IsValid())
	{
		drawcall->bMaterialChanged = true;
		drawcall->bMaterialNeedToReassign = true;
		drawcall->bNeedToUpdateVertex = true;
	}
}
void UUIProceduralRect::OnMaterialInstanceDynamicCreated(class UMaterialInstanceDynamic* mat) 
{
	mat->SetTextureParameterValue(DataTextureParameterName, ProceduralRectData->GetDataTexture());
}

void UUIProceduralRect::CheckAdditionalShaderChannels()
{
	if (RenderCanvas.IsValid())
	{
		auto flags =
			//UV1: data position 2d
			//UV2.x: bIsOuterShadow
			(1 << (int)ELGUICanvasAdditionalChannelType::UV1)
			| (1 << (int)ELGUICanvasAdditionalChannelType::UV2)
			;
#if WITH_EDITOR
		auto originFlags = RenderCanvas->GetActualAdditionalShaderChannelFlags();
		if (flags != 0 && (originFlags & flags) == 0)
		{
			auto MsgText = FText::Format(LOCTEXT("UIProceduralRectShaderChannels"
				, "{0} Automatically change 'AdditionalShaderChannels' property for LGUICanvas because UIProceduralRect need it")
				, FText::FromString(FString::Printf(TEXT("[%s].%d"), ANSI_TO_TCHAR(__FUNCTION__), __LINE__)));
			//LGUIUtils::EditorNotification(MsgText);
			UE_LOG(LGUI, Log, TEXT("%s"), *MsgText.ToString());
		}
#endif
		RenderCanvas->SetActualRequireAdditionalShaderChannels(flags);
	}
}

void UUIProceduralRect::OnDataTextureChanged(class UTexture2D* Texture)
{
	geometry->texture = GetTextureToCreateGeometry();
	if (drawcall.IsValid())
	{
		drawcall->Texture = geometry->texture;
		drawcall->bTextureChanged = true;
		drawcall->bNeedToUpdateVertex = true;
	}
	MarkVerticesDirty(false, true, true, false);
	MarkCanvasUpdate(true, true, false);
}

void UUIProceduralRect::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	static FLGUISpriteInfo SimpleRectSpriteData;
	UIGeometry::UpdateUIProceduralRectSimpleVertex(&InGeo,
		this->BlockData.bOuterShadow, this->BlockData.OuterShadowOffset, this->BlockData.OuterShadowSize, this->BlockData.OuterShadowBlur,
		this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), SimpleRectSpriteData, RenderCanvas.Get(), this, GetFinalColor(),
		InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
	);

	if (InTriangleChanged || InVertexPositionChanged || InVertexUVChanged || InVertexColorChanged)
	{
		auto& vertices = InGeo.vertices;
		if (this->BlockData.bOuterShadow)
		{
			for (int i = 0; i < 8; i++)
			{
				vertices[i].TextureCoordinate[1] = FVector2f(DataStartPosition.X, DataStartPosition.Y);
			}
			for (int i = 0; i < 4; i++)
			{
				vertices[i].TextureCoordinate[2] = FVector2f(1, 0);
			}
		}
		else
		{
			for (int i = 0; i < 4; i++)
			{
				vertices[i].TextureCoordinate[1] = FVector2f(DataStartPosition.X, DataStartPosition.Y);
				vertices[i].TextureCoordinate[2] = FVector2f(0, 0);
			}
		}


		BlockData.QuadSize.X = this->GetWidth() * 0.5f;
		BlockData.QuadSize.Y = this->GetHeight() * 0.5f;

		auto BlockSize = ProceduralRectData->GetBlockSizeInByte();
		uint8* BlockBuffer = new uint8[BlockSize];
		FMemory::Memzero(BlockBuffer, BlockSize);
		BlockData.FillData(BlockBuffer);
		ProceduralRectData->UpdateBlock(DataStartPosition, BlockBuffer);
	}
}

void UUIProceduralRect::OnAnchorChange(bool InPivotChange, bool InWidthChange, bool InHeightChange, bool InDiscardCache)
{
	Super::OnAnchorChange(InPivotChange, InWidthChange, InHeightChange, InDiscardCache);
}

#undef LOCTEXT_NAMESPACE

#if LGUI_CAN_DISABLE_OPTIMIZATION
PRAGMA_ENABLE_OPTIMIZATION
#endif
