// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Extensions/UIRenderTarget.h"
#include "Core/ActorComponent/LGUICanvas.h"
#include "LGUI.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Utils/LGUIUtils.h"
#include "PrefabSystem/LGUIPrefabManager.h"
#include "Core/UIGeometry.h"
#include "Core/LGUISpriteInfo.h"
#include "Core/LGUICustomMesh.h"

#define LOCTEXT_NAMESPACE "UIRenderTarget"

UUIRenderTarget::UUIRenderTarget(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer)
{
	PrimaryComponentTick.bCanEverTick = false;
	PrimaryComponentTick.bStartWithTickEnabled = false;

	TargetCanvas = FLGUIComponentReference(ULGUICanvas::StaticClass());
}

void UUIRenderTarget::OnBeforeCreateOrUpdateGeometry()
{

}
UTexture* UUIRenderTarget::GetTextureToCreateGeometry()
{
	UTexture* Result = nullptr;
	if (auto Canvas = GetCanvas())
	{
		Result = Canvas->GetRenderTarget();
	}
#if WITH_EDITOR
	if (!Result && !GetWorld()->IsGameWorld())//if not find valid texture (because canvas not create rendertarget yet, and edit mode not register the callback event), then get it next frame
	{
		ULGUIPrefabManagerObject::AddOneShotTickFunction([this]() {
			MarkTextureDirty();
			});
	}
#endif
	return Result;
}
void UUIRenderTarget::OnUpdateGeometry(UIGeometry& InGeo, bool InTriangleChanged, bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)
{
	if (IsValid(CustomMesh))
	{
		CustomMesh->UIGeo = &InGeo;
		CustomMesh->OnFillMesh(this, InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged);
	}
	else
	{
		static FLGUISpriteInfo SpriteInfo;
		UIGeometry::UpdateUIRectSimpleVertex(&InGeo,
			this->GetWidth(), this->GetHeight(), FVector2f(this->GetPivot()), SpriteInfo, RenderCanvas.Get(), this, GetFinalColor(),
			InTriangleChanged, InVertexPositionChanged, InVertexUVChanged, InVertexColorChanged
		);
	}
}

void UUIRenderTarget::BeginPlay()
{
	Super::BeginPlay();
	if (!ULGUIPrefabWorldSubsystem::IsLGUIPrefabSystemProcessingActor(this->GetOwner()))
	{
		Awake_Implementation();
	}
}
void UUIRenderTarget::EndPlay(EEndPlayReason::Type Reason)
{
	Super::EndPlay(Reason);
}

void UUIRenderTarget::Awake_Implementation()
{
	if (auto Canvas = GetCanvas())
	{
		Canvas->OnRenderTargetCreatedOrChanged.AddWeakLambda(this, [this](UTextureRenderTarget2D* RenderTarget, bool CreatedOrChanged) {
			SetIsUIActive(true);
			this->MarkTextureDirty();
			});
	}
}
ULGUICanvas* UUIRenderTarget::GetTargetCanvas_Implementation()const
{
	return GetCanvas();
}
bool UUIRenderTarget::PerformLineTrace_Implementation(const int32& InHitFaceIndex, const FVector& InHitPoint, const FVector& InLineStart, const FVector& InLineEnd, FVector2D& OutHitUV)
{
	if (IsValid(CustomMesh))
	{
		FVector2D HitUV;
		bool Result = CustomMesh->GetHitUV(this, InHitFaceIndex, InHitPoint, InLineStart, InLineEnd, OutHitUV);
		OutHitUV.Y = 1.0f - OutHitUV.Y;
		return Result;
	}
	else
	{
		// Find the hit location on the component
		FVector ComponentHitLocation = GetComponentTransform().InverseTransformPosition(InHitPoint);

		// Convert the 3D position of component space, into the 2D equivalent
		auto LocationRelativeToLeftBottom = FVector2D(ComponentHitLocation.Y, ComponentHitLocation.Z) - this->GetLocalSpaceLeftBottomPoint();
		auto Location01 = LocationRelativeToLeftBottom / FVector2D(this->GetWidth(), this->GetHeight());

		OutHitUV = Location01;
		return true;
	}
}

#if WITH_EDITOR
bool UUIRenderTarget::CanEditChange(const FProperty* InProperty) const
{
	if (InProperty)
	{
		FString PropertyName = InProperty->GetName();

	}

	return Super::CanEditChange(InProperty);
}
void UUIRenderTarget::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	if (auto Property = PropertyChangedEvent.MemberProperty)
	{
		auto PropertyName = Property->GetFName();
		if (PropertyName == GET_MEMBER_NAME_STRING_CHECKED(UUIRenderTarget, TargetCanvas))
		{
			if (!TargetCanvas.IsValidComponentReference())
			{
				TargetCanvasObject = nullptr;
			}
		}
		else if (PropertyName == GET_MEMBER_NAME_CHECKED(UUIRenderTarget, CustomMesh))
		{
			if (IsValid(CustomMesh))//custom mesh use geometry raycast to get precise uv
			{
				this->SetRaycastType(EUIRenderableRaycastType::Mesh);
			}
		}
	}
}
#endif

ULGUICanvas* UUIRenderTarget::GetCanvas()const
{
	if (TargetCanvasObject.IsValid())
	{
		return TargetCanvasObject.Get();
	}
	if (!TargetCanvas.IsValidComponentReference())
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIRenderTarget::GetCanvas]TargetCanvas not valid!"));
		return nullptr;
	}
	auto Canvas = TargetCanvas.GetComponent<ULGUICanvas>();
	if (Canvas == nullptr)
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIRenderTarget::GetCanvas]TargetCanvas not valid!"));
		return nullptr;
	}
	if (!Canvas->IsRootCanvas())
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIRenderTarget::GetCanvas]TargetCanvas must be a root canvas!"));
		return nullptr;
	}
	if (Canvas->GetRenderMode() != ELGUIRenderMode::RenderTarget)
	{
		UE_LOG(LGUI, Warning, TEXT("[UUIRenderTarget::GetCanvas]TargetCanvas's render mode must be RenderTarget!"));
		return nullptr;
	}
	TargetCanvasObject = Canvas;
	return Canvas;
}

void UUIRenderTarget::SetCanvas(ULGUICanvas* Value)
{
	if (TargetCanvasObject.Get() != Value)
	{
		TargetCanvasObject = Value;
	}
}

AUIRenderTargetActor::AUIRenderTargetActor()
{
	PrimaryActorTick.bCanEverTick = false;

	UIRenderTarget = CreateDefaultSubobject<UUIRenderTarget>(TEXT("UIRenderTarget"));
	RootComponent = UIRenderTarget;
}

#undef LOCTEXT_NAMESPACE
