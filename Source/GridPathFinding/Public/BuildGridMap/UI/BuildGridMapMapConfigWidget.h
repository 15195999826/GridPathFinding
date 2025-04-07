// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridMapWindow.h"
#include "Blueprint/UserWidget.h"
#include "BuildGridMapMapConfigWidget.generated.h"

class UComboBoxString;
class UEditableTextBox;

DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapTypeChangedDelegate, EGridMapType);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnMapDrawModeChangedDelegate, EGridMapDrawMode);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTileOrientationChangedDelegate, ETileOrientationFlag);
/**
 * 配置地图属性， 地图名称、不同类型地图的网格尺寸、不同类型地图的地图尺寸
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapMapConfigWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;
	
	// 地图名称
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> MapNameTextBox;

	// 地图类型， ComboBoxString
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> MapTypeComboBox;

	// 地图绘制类型
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> MapDrawModeComboBox;

	// 六向地图格子方向
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> TileOrientationComboBox;

	// 地图行数
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> MapRowTextBox;

	// 地图列数
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> MapColumnTextBox;

	// 地图半径
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> MapRadiusTextBox;

	// 格子尺寸X
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> GridSizeXTextBox;

	// 格子尺寸Y
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> GridSizeYTextBox;

	// 格子半径
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> GridRadiusTextBox;

	FOnMapTypeChangedDelegate OnMapTypeChanged;
	FOnMapDrawModeChangedDelegate OnMapDrawModeChanged;
	FOnTileOrientationChangedDelegate OnTileOrientationChanged;

	// Todo: 引入高度概念
	void SetMapConfig(const FName& InMapName, const FGridMapConfig& MapConfig);

private:
	UFUNCTION()
	void OnMapTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnMapDrawModeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	UFUNCTION()
	void OnHexTileOrientationSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	

	EGridMapType GetMapTypeFromString(const FString& InString) const;
	EGridMapDrawMode GetMapDrawModeFromString(const FString& InString) const;
	ETileOrientationFlag GetTileOrientationFromString(const FString& InString) const;

	TMap<EGridMapType, TMap<EGridMapDrawMode, TArray<UWidget*>>> VisibilityRules;
	void UpdateShowLine(EGridMapType InMapType, EGridMapDrawMode InDrawMode);
};