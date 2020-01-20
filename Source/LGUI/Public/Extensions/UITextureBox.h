// Copyright 2019-2020 LexLiu. All Rights Reserved.

#pragma once

#include "Core/ActorComponent/UITextureBase.h"
#include "Core/Actor/UIBaseActor.h"
#include "UITextureBox.generated.h"


UCLASS(ClassGroup = (LGUI), Blueprintable, meta = (BlueprintSpawnableComponent))
class LGUI_API UUITextureBox : public UUITextureBase
{
	GENERATED_BODY()

public:	
	UUITextureBox(const FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	virtual void EditorForceUpdateImmediately() override;
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif
protected:
	virtual void BeginPlay()override;

	virtual void OnCreateGeometry()override;
	virtual void OnUpdateGeometry(bool InVertexPositionChanged, bool InVertexUVChanged, bool InVertexColorChanged)override;

	UPROPERTY(EditAnywhere, Category = "LGUI")
		float thickness = 1.0f;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		bool seperateFrontColor = false;
	UPROPERTY(EditAnywhere, Category = "LGUI")
		FColor frontFaceColor = FColor::White;
};


UCLASS()
class LGUI_API AUITextureBoxActor : public AUIBaseActor
{
	GENERATED_BODY()

public:
	AUITextureBoxActor();

	FORCEINLINE virtual UUIItem* GetUIItem()const override { return UITextureBox; }
	FORCEINLINE UUITextureBox* GetUITextureBox()const { return UITextureBox; }
private:
	UPROPERTY(Category = "LGUI", VisibleAnywhere, BlueprintReadOnly, Transient, meta = (AllowPrivateAccess = "true"))
		class UUITextureBox* UITextureBox;

};
