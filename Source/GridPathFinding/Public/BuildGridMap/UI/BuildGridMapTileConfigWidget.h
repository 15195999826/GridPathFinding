// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/HCubeCoord.h"
#include "Types/SerializableTile.h"
#include "BuildGridMapTileConfigWidget.generated.h"

class UEditableTextBox;
class UGridEnvironmentType;
class UComboBoxString;
class UTextBlock;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnTileEnvChangedDelegate, const TObjectPtr<UGridEnvironmentType> InEnvType);
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapTileConfigWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> EnvComboBox;
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> EnvTextureIndexTextBox;

	FOnTileEnvChangedDelegate OnTileEnvChanged;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> TileRowText;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> TileColumnText;

public:
	void BindSingleTileCoord(FHCubeCoord SelectedCoord);
	void BindSingleTileData(const FSerializableTile& InTileData);

private:
	UPROPERTY()
	TMap<FString, TObjectPtr<UGridEnvironmentType>> EnvTypeMap;
	UPROPERTY()
	TMap<FName, FString> EnvType2DisplayNameMap;

	UFUNCTION()
	void OnEnvTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
