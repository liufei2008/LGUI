// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexUIClipData.h"

#include "Core/Components/LexWidget.h"
#include "Core/LexUIDataAsTexture.h"
#include "Core/Components/LexCanvas.h"



int FLexUIClipData::InheritClipDepth = 8;
int FLexUIClipData::SingleBlockSizeInBytes =
	sizeof(FMatrix44f)//canvas to clip object's space, last column of matrix: (half-width, half-height, isValid, softness)
	+ sizeof(FVector4f)//CornerRadius
	;
int FLexUIClipData::BlockSizeInBytes = SingleBlockSizeInBytes * FLexUIClipData::InheritClipDepth;

FLexUIClipData::FLexUIClipData(const TSharedPtr<FLexUIClipData>& InParent, ULexUIDataAsTexture* InDataTexture, ULexWidget* InWidget)
{
	this->Parent = InParent;
	this->DataTexture = InDataTexture;
	this->Widget = InWidget;
	this->BufferStartPos = this->DataTexture->RegisterBuffer();
}

FLexUIClipData::~FLexUIClipData()
{
	if (DataTexture.IsValid())
	{
		this->DataTexture->UnregisterBuffer(this->BufferStartPos);
	}
}

void FLexUIClipData::Add2DTranslationToMatrix(FMatrix44d& Matrix, const FVector2d& Translation)
{
	auto M = &Matrix.M[0][0];
	M[13] += Translation.X;
	M[14] += Translation.Y;
}
void FLexUIClipData::UpdateData()
{
	if (!bNeedUpdateData)return;
	bNeedUpdateData = false;
	
	TArray<uint8> BlockBuffer;
	BlockBuffer.SetNumUninitialized(BlockSizeInBytes);
	FMemory::Memzero(BlockBuffer.GetData(), BlockSizeInBytes);
	int BlockDataOffset = 0;
	auto RenderCanvasWidget = this->GetWidget()->GetRenderCanvas()->GetWidget();
	auto CanvasToWorldMatrix = RenderCanvasWidget->GetWorldTransform().ToMatrixWithScale();
	//auto CanvasLocalSpaceCenter = RenderCanvasWidget->GetLocalSpaceCenter();
	//Add2DTranslationToMatrix(CanvasToWorldMatrix, CanvasLocalSpaceCenter);
	FLexUIClipData* TargetClip = this;
	for (int i = 0; i < InheritClipDepth; i++)
	{
		auto WidgetToWorldMatrix = TargetClip->Widget->GetWorldTransform().ToMatrixWithScale();
		auto WidgetLocalSpaceCenter = TargetClip->Widget->GetLocalSpaceCenter();
		auto ClippingMargin = TargetClip->Widget->GetClippingMargin();
		auto CenterOffsetByMargin = FVector2D(ClippingMargin.Right - ClippingMargin.Left, ClippingMargin.Top - ClippingMargin.Bottom) * 0.5f;
		WidgetLocalSpaceCenter += CenterOffsetByMargin;
		Add2DTranslationToMatrix(WidgetToWorldMatrix, WidgetLocalSpaceCenter);
		auto WorldToWidgetMatrix = WidgetToWorldMatrix.Inverse();
		auto CanvasToWidgetMatrix = FMatrix44f(CanvasToWorldMatrix * WorldToWidgetMatrix);
		auto& M = CanvasToWidgetMatrix.M;
		auto RenderSize = FVector2f(TargetClip->Widget->GetWidth(), TargetClip->Widget->GetHeight()) + ClippingMargin.GetDesiredSize2f();
		M[0][3] = RenderSize.X * 0.5f;//half width
		M[1][3] = RenderSize.Y * 0.5f;//half height
		M[2][3] = 0;//@todo: softness
		M[3][3] = 1;//isValid
		CanvasToWidgetMatrix = CanvasToWidgetMatrix.GetTransposed();//matrix in memory is aligned as row-primary, so transpose it then in hlsl we can read as column-primary
		FMemory::Memcpy(BlockBuffer.GetData() + BlockDataOffset, &CanvasToWidgetMatrix, sizeof(FMatrix44f));
		BlockDataOffset += sizeof(FMatrix44f);
		auto CornerRadius = TargetClip->GetWidget()->GetClippingCornerRadius();
		CornerRadius = FVector4f(CornerRadius.Y, CornerRadius.X, CornerRadius.W, CornerRadius.Z);//flip vertical
		FMemory::Memcpy(BlockBuffer.GetData() + BlockDataOffset, &CornerRadius, sizeof(FVector4f));
		BlockDataOffset += sizeof(FVector4f);
		if (!TargetClip->Parent.IsValid())
		{
			break;
		}
		TargetClip = TargetClip->Parent.Pin().Get();
	}
	DataTexture->UpdateBlock(BufferStartPos, MoveTemp(BlockBuffer));
}

bool FLexUIClipData::IsPointVisible(const FVector& WorldPoint) const
{
	auto TargetClip = this;
	for (int i = 0; i < InheritClipDepth; i++)
	{
		auto TargetWidget = TargetClip->Widget;
		auto LocalPoint = TargetWidget->GetWorldTransform().InverseTransformPosition(WorldPoint);
		auto ClippingMargin = TargetWidget->GetClippingMargin();
		if (LocalPoint.Y < TargetWidget->GetLocalSpaceLeft() - ClippingMargin.Left)return false;
		if (LocalPoint.Y > TargetWidget->GetLocalSpaceRight() + ClippingMargin.Right)return false;
		if (LocalPoint.Z < TargetWidget->GetLocalSpaceBottom() - ClippingMargin.Bottom)return false;
		if (LocalPoint.Z > TargetWidget->GetLocalSpaceTop() + ClippingMargin.Top)return false;
		if (!IsPointVisible_CheckCornerRadius(FVector2D(LocalPoint.Y, LocalPoint.Z), TargetWidget.Get()))
			return false;
		if (!TargetClip->Parent.IsValid())
		{
			break;
		}
		
		TargetClip = TargetClip->Parent.Pin().Get();
	}
	return true;
}

bool FLexUIClipData::IsPointVisible_CheckCornerRadius(const FVector2D& InLocalHitPoint, ULexWidget* InWidget) const
{
	auto CornerRadius = InWidget->GetClippingCornerRadius();
	if (CornerRadius.X <= 0 && CornerRadius.Y <= 0 && CornerRadius.Z <= 0 && CornerRadius.W <= 0)
		return true;
	auto ClippingMargin = InWidget->GetClippingMargin();
	auto HalfWidth = (InWidget->GetWidth() + ClippingMargin.Left + ClippingMargin.Right) * 0.5f;
	auto HalfHeight = (InWidget->GetHeight() + ClippingMargin.Bottom + ClippingMargin.Top) * 0.5f;
	auto MinSize = FMath::Min(HalfWidth, HalfHeight);
	CornerRadius.X = FMath::Min(CornerRadius.X, MinSize);
	CornerRadius.Y = FMath::Min(CornerRadius.Y, MinSize);
	CornerRadius.Z = FMath::Min(CornerRadius.Z, MinSize);
	CornerRadius.W = FMath::Min(CornerRadius.W, MinSize);
	//right bottom area of rect
	{
		auto Radius = CornerRadius.X;
		auto RoundCornerCenterPos = FVector2D(InWidget->GetLocalSpaceRight() + ClippingMargin.Right - Radius, InWidget->GetLocalSpaceBottom() - ClippingMargin.Bottom + Radius);
		if (InLocalHitPoint.X > RoundCornerCenterPos.X && InLocalHitPoint.Y < RoundCornerCenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, RoundCornerCenterPos) > Radius * Radius)
			{
				return false;
			}
			return true;
		}
	}
	//right top area of rect
	{
		auto Radius = CornerRadius.Y;
		auto RoundCornerCenterPos = FVector2D(InWidget->GetLocalSpaceRight() + ClippingMargin.Right - Radius, InWidget->GetLocalSpaceTop() + ClippingMargin.Top - Radius);
		if (InLocalHitPoint.X > RoundCornerCenterPos.X && InLocalHitPoint.Y > RoundCornerCenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, RoundCornerCenterPos) > Radius * Radius)
			{
				return false;
			}
			return true;
		}
	}
	//left top area of rect
	{
		auto Radius = CornerRadius.Z;
		auto RoundCornerCenterPos = FVector2D(InWidget->GetLocalSpaceLeft() - ClippingMargin.Left + Radius, InWidget->GetLocalSpaceTop() + ClippingMargin.Top - Radius);
		if (InLocalHitPoint.X < RoundCornerCenterPos.X && InLocalHitPoint.Y > RoundCornerCenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, RoundCornerCenterPos) > Radius * Radius)
			{
				return false;
			}
			return true;
		}
	}
	//left bottom area of rect
	{
		auto Radius = CornerRadius.W;
		auto RoundCornerCenterPos = FVector2D(InWidget->GetLocalSpaceLeft() - ClippingMargin.Left + Radius, InWidget->GetLocalSpaceBottom() - ClippingMargin.Bottom + Radius);
		if (InLocalHitPoint.X < RoundCornerCenterPos.X && InLocalHitPoint.Y < RoundCornerCenterPos.Y)
		{
			if (FVector2D::DistSquared(InLocalHitPoint, RoundCornerCenterPos) > Radius * Radius)
			{
				return false;
			}
			return true;
		}
	}
	return true;
}


