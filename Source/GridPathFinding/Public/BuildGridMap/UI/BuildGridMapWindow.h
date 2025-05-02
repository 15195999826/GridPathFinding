// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "BuildGridMap/BuildGridMapPlayerController.h"
#include "Types/GridMapSave.h"
#include "BuildGridMapWindow.generated.h"

enum class EBuildGridMapSaveMode : uint8;
class UBuildGridMapTileConfigWidget;
class UBuildGridMapMapConfigWidget;
class UTextBlock;
class AGridMapRenderer;
class UGridMapModel;
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapWindow : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

public:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UBuildGridMapMapConfigWidget> MapConfigWidget;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UBuildGridMapTileConfigWidget> TileConfigWidget;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> EditingMapNameText;

public:
	UFUNCTION(BlueprintNativeEvent)
	void SingleSelectTileCoord(const FHCubeCoord& SelectedCoord);

	UFUNCTION(BlueprintNativeEvent)
	void SingleSelectTileData(const FSerializableTile& InTileData);

	UFUNCTION(BlueprintImplementableEvent, BlueprintCallable)
	void UpdateMapGroup(const FString& SelectedMapName, bool bNotifySelected);

	UFUNCTION(BlueprintImplementableEvent)
	void SetCanInput(bool bCond);

protected:
	UFUNCTION(BlueprintImplementableEvent)
	void ChangeMapGroupButtonText(const FName& InOldMapName, const FName& InNewMapName);

	UFUNCTION(BlueprintImplementableEvent)
	void OnSaveStart(EBuildGridMapSaveMode InSaveMode);
	UFUNCTION(BlueprintImplementableEvent)
	void OnSaveOver();
	UFUNCTION(BlueprintImplementableEvent)
	void OnCreateGridMapSave();

private:
	void OnEmptyEditingMapSave();
	void OnSwitchEditingMapSave();
	void OnChangeMapName(const FName& InOldName, const FName& InNewName);
	void OnChangeMapSizeX(int32 InOldSizeX, int32 InNewSizeX);
	void OnChangeMapSizeY(int32 InOldSizeY, int32 InNewSizeY);
};
