// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/EditableText.h"
#include "Components/EditableTextBox.h"
#include "Components/TextBlock.h"
#include "Types/TokenActorData.h"
#include "BuildGridTileActorConfigWidget.generated.h"

struct FSerializableTile;
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridTileActorConfigWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableText> InputPathTxt;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> NameTxt;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> ShowBtnTxt;

	UFUNCTION(BlueprintCallable)
	void OnSpawnActorClick();

	UFUNCTION(BlueprintCallable)
	void OnShowActorClick();

	void ShowInfo(const FSerializableTile& InTileData);
private:
	void SpawnActorByPath(const FString& ClassPath);

	// utils
	bool JsonToStructUtil(const FString& JsonString, FTokenActorStruct& OutActorStruct);
};
