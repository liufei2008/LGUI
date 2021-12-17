// Copyright 2019-2021 LexLiu. All Rights Reserved.

#include "Core/ActorComponent/UIBaseRenderable.h"
#include "LGUI.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "Utils/LGUIUtils.h"
#include "GeometryModifier/UIGeometryModifierBase.h"
#include "Core/ActorComponent/UICanvasGroup.h"


UUIBaseRenderable::UUIBaseRenderable(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	uiRenderableType = EUIRenderableType::None;

	bColorChanged = true;
	bTransformChanged = true;
}

void UUIBaseRenderable::BeginPlay()
{
	Super::BeginPlay();
	bColorChanged = true;
	bTransformChanged = true;
}

void UUIBaseRenderable::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent( DeltaTime, TickType, ThisTickFunction );
}

#if WITH_EDITOR
void UUIBaseRenderable::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
}
#endif

void UUIBaseRenderable::OnRegister()
{
	Super::OnRegister();
}
void UUIBaseRenderable::OnUnregister()
{
	Super::OnUnregister();
}

void UUIBaseRenderable::ApplyUIActiveState()
{
	Super::ApplyUIActiveState();//this line must line before AddUIRenderable/RemoveUIRenderable, because UIActiveStateChangedDelegate need to call first. (UIActiveStateChangedDelegate lead to canvas: ParentCanvas->UIRenderableList.Add/Remove)

	if (GetIsUIActiveInHierarchy())
	{
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->AddUIRenderable(this);
		}
	}
	else
	{
		if (RenderCanvas.IsValid())
		{
			RenderCanvas->RemoveUIRenderable(this);
		}
	}
	bColorChanged = true;
}
void UUIBaseRenderable::OnRenderCanvasChanged(ULGUICanvas* OldCanvas, ULGUICanvas* NewCanvas)
{
	if (IsValid(OldCanvas))
	{
		OldCanvas->RemoveUIRenderable(this);
	}
	if (IsValid(NewCanvas))
	{
		NewCanvas->AddUIRenderable(this);
	}
}

void UUIBaseRenderable::OnCanvasGroupAlphaChange()
{
	MarkColorDirty();
}

void UUIBaseRenderable::MarkColorDirty()
{
	bColorChanged = true;
	MarkCanvasUpdate();
}

void UUIBaseRenderable::MarkAllDirtyRecursive()
{
	bColorChanged = true;
	Super::MarkAllDirtyRecursive();
}

void UUIBaseRenderable::MarkLayoutDirty(bool InTransformChange, bool InPivotChange, bool InSizeChange, bool DoPropergateLayoutChange)
{
	Super::MarkLayoutDirty(InTransformChange, InPivotChange, InSizeChange, DoPropergateLayoutChange);
	if (InTransformChange)bTransformChanged = true;
}

void UUIBaseRenderable::GetGeometryBoundsInLocalSpace(FVector2D& min, FVector2D& max)const//@todo: for UISprite, we should calculate with sprite's padding properties
{
	min = GetLocalSpaceLeftBottomPoint();
	max = GetLocalSpaceRightTopPoint();
}

bool UUIBaseRenderable::LineTraceUIGeometry(TSharedPtr<UIGeometry> InGeo, FHitResult& OutHit, const FVector& Start, const FVector& End)
{
	if (bRaycastComplex)
	{
		if (!bRaycastTarget)return false;
		if (!GetIsUIActiveInHierarchy())return false;
		if (!RenderCanvas.IsValid())return false;
		if (!GetOwner())return false;
		if (GetOwner()->GetRootComponent() != this)return false;//only root component can do line trace hit
		auto inverseTf = GetComponentTransform().Inverse();
		auto localSpaceRayOrigin = inverseTf.TransformPosition(Start);
		auto localSpaceRayEnd = inverseTf.TransformPosition(End);

		//DrawDebugLine(this->GetWorld(), Start, End, FColor::Red, false, 5.0f);//just for test
		//check Line-Plane intersection first, then check Line-Triangle
		//start and end point must be different side of X plane
		if (FMath::Sign(localSpaceRayOrigin.X) != FMath::Sign(localSpaceRayEnd.X))
		{
			auto result = FMath::LinePlaneIntersection(localSpaceRayOrigin, localSpaceRayEnd, FVector::ZeroVector, FVector(1, 0, 0));
			//hit point inside rect area
			if (result.Y > GetLocalSpaceLeft() && result.Y < GetLocalSpaceRight() && result.Z > GetLocalSpaceBottom() && result.Z < GetLocalSpaceTop())
			{
				//triangle hit test
				auto& vertices = InGeo->originPositions;
				auto& triangleIndices = InGeo->triangles;
				int triangleCount = triangleIndices.Num() / 3;
				int index = 0;
				for (int i = 0; i < triangleCount; i++)
				{
					auto point0 = (vertices[triangleIndices[index++]]);
					auto point1 = (vertices[triangleIndices[index++]]);
					auto point2 = (vertices[triangleIndices[index++]]);
					FVector hitPoint, hitNormal;
					if (FMath::SegmentTriangleIntersection(localSpaceRayOrigin, localSpaceRayEnd, point0, point1, point2, hitPoint, hitNormal))
					{
						OutHit.TraceStart = Start;
						OutHit.TraceEnd = End;
						OutHit.Actor = GetOwner();
						OutHit.Component = (UPrimitiveComponent*)this;//acturally this convert is incorrect, but I need this pointer
						OutHit.Location = GetComponentTransform().TransformPosition(hitPoint);
						OutHit.Normal = GetComponentTransform().TransformVector(hitNormal);
						OutHit.Normal.Normalize();
						OutHit.Distance = FVector::Distance(Start, OutHit.Location);
						OutHit.ImpactPoint = OutHit.Location;
						OutHit.ImpactNormal = OutHit.Normal;
						return true;
					}
				}
			}
		}
		return false;
	}
	else
	{
		return Super::LineTraceUI(OutHit, Start, End);
	}
}

void UUIBaseRenderable::SetColor(FColor value)
{
	if (Color != value)
	{
		Color = value;
		MarkColorDirty();
	}
}
void UUIBaseRenderable::SetAlpha(float value)
{
	value = FMath::Clamp(value, 0.0f, 1.0f);
	auto uintAlpha = (uint8)(value * 255);
	if (Color.A != uintAlpha)
	{
		MarkColorDirty();
		Color.A = uintAlpha;
	}
}

FColor UUIBaseRenderable::GetFinalColor()const
{
	FColor ResultColor = Color;
	if (CanvasGroup.IsValid())
	{
		ResultColor.A = Color.A * CanvasGroup->GetFinalAlpha();
	}
	return ResultColor;
}

uint8 UUIBaseRenderable::GetFinalAlpha()const
{
	if (CanvasGroup.IsValid())
	{
		return (uint8)(Color.A * CanvasGroup->GetFinalAlpha());
	}
	return Color.A;
}

float UUIBaseRenderable::GetFinalAlpha01()const
{
	return Color255To1_Table[GetFinalAlpha()];
}

float UUIBaseRenderable::Color255To1_Table[256] =
{
	0,0.003921569,0.007843138,0.01176471,0.01568628,0.01960784,0.02352941,0.02745098,0.03137255,0.03529412,0.03921569,0.04313726,0.04705882,0.05098039
	,0.05490196,0.05882353,0.0627451,0.06666667,0.07058824,0.07450981,0.07843138,0.08235294,0.08627451,0.09019608,0.09411765,0.09803922,0.1019608,0.1058824
	,0.1098039,0.1137255,0.1176471,0.1215686,0.1254902,0.1294118,0.1333333,0.1372549,0.1411765,0.145098,0.1490196,0.1529412,0.1568628,0.1607843,0.1647059,0.1686275
	,0.172549,0.1764706,0.1803922,0.1843137,0.1882353,0.1921569,0.1960784,0.2,0.2039216,0.2078431,0.2117647,0.2156863,0.2196078,0.2235294,0.227451,0.2313726,0.2352941
	,0.2392157,0.2431373,0.2470588,0.2509804,0.254902,0.2588235,0.2627451,0.2666667,0.2705882,0.2745098,0.2784314,0.282353,0.2862745,0.2901961,0.2941177,0.2980392,0.3019608
	,0.3058824,0.3098039,0.3137255,0.3176471,0.3215686,0.3254902,0.3294118,0.3333333,0.3372549,0.3411765,0.345098,0.3490196,0.3529412,0.3568628,0.3607843,0.3647059,0.3686275
	,0.372549,0.3764706,0.3803922,0.3843137,0.3882353,0.3921569,0.3960784,0.4,0.4039216,0.4078431,0.4117647,0.4156863,0.4196078,0.4235294,0.427451,0.4313726,0.4352941,0.4392157
	,0.4431373,0.4470588,0.4509804,0.454902,0.4588235,0.4627451,0.4666667,0.4705882,0.4745098,0.4784314,0.4823529,0.4862745,0.4901961,0.4941176,0.4980392,0.5019608,0.5058824,0.509804
	,0.5137255,0.5176471,0.5215687,0.5254902,0.5294118,0.5333334,0.5372549,0.5411765,0.5450981,0.5490196,0.5529412,0.5568628,0.5607843,0.5647059,0.5686275,0.572549,0.5764706,0.5803922
	,0.5843138,0.5882353,0.5921569,0.5960785,0.6,0.6039216,0.6078432,0.6117647,0.6156863,0.6196079,0.6235294,0.627451,0.6313726,0.6352941,0.6392157,0.6431373,0.6470588,0.6509804,0.654902
	,0.6588235,0.6627451,0.6666667,0.6705883,0.6745098,0.6784314,0.682353,0.6862745,0.6901961,0.6941177,0.6980392,0.7019608,0.7058824,0.7098039,0.7137255,0.7176471,0.7215686,0.7254902,0.7294118
	,0.7333333,0.7372549,0.7411765,0.7450981,0.7490196,0.7529412,0.7568628,0.7607843,0.7647059,0.7686275,0.772549,0.7764706,0.7803922,0.7843137,0.7882353,0.7921569,0.7960784,0.8,0.8039216,0.8078431
	,0.8117647,0.8156863,0.8196079,0.8235294,0.827451,0.8313726,0.8352941,0.8392157,0.8431373,0.8470588,0.8509804,0.854902,0.8588235,0.8627451,0.8666667,0.8705882,0.8745098,0.8784314,0.8823529,0.8862745
	,0.8901961,0.8941177,0.8980392,0.9019608,0.9058824,0.9098039,0.9137255,0.9176471,0.9215686,0.9254902,0.9294118,0.9333333,0.9372549,0.9411765,0.945098,0.9490196,0.9529412,0.9568627,0.9607843,0.9647059
	,0.9686275,0.972549,0.9764706,0.9803922,0.9843137,0.9882353,0.9921569,0.9960784,1
};
