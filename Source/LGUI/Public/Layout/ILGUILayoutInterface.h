// Copyright 2019-2021 LexLiu. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "ILGUILayoutInterface.generated.h"


/**
 * Interface for handling LGUI's layout update.
 * Need to register UObject with RegisterLGUILayout, check UIBaseLayout for reference
 */
UINTERFACE(Blueprintable, MinimalAPI)
class ULGUILayoutInterface : public UInterface
{
	GENERATED_BODY()
};
/**
 * Interface for handling LGUI's culture changed.
 * Need to register UObject with RegisterLGUILayout, check UIBaseLayout for reference
 */ 
class LGUI_API ILGUILayoutInterface
{
	GENERATED_BODY()
public:
	/**
	 * Called when need to update layout.
	 */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		void OnUpdateLayout();

	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildAnchor()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildHorizontalAnchoredPosition()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildVerticalAnchoredPosition()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildWidth()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildHeight()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildAnchorLeft()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildAnchorRight()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildAnchorBottom()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlChildAnchorTop()const;

	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfAnchor()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfHorizontalAnchoredPosition()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfVerticalAnchoredPosition()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfWidth()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfHeight()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfAnchorLeft()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfAnchorRight()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfAnchorBottom()const;
	/** Editor helper function, should be "EditorOnly", but there is no "EditorOnly" interface for blueprint */
	UFUNCTION(BlueprintNativeEvent, BlueprintCallable, Category = "LGUI Layout")
		bool CanControlSelfAnchorTop()const;
};