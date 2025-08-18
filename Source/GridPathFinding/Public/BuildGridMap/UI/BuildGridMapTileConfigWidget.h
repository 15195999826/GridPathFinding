// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridTileActorConfigWidget.h"
#include "Blueprint/UserWidget.h"
#include "Types/HCubeCoord.h"
#include "Types/SerializableTile.h"
#include "BuildGridMapTileConfigWidget.generated.h"

class USpinBox;
class UVerticalBox;
class UBuildGridMapTokenActorPanel;
class UButton;
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
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> EnvComboBox;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<USpinBox> HeightSpinBox;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UEditableTextBox> EnvTextureIndexTextBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UVerticalBox> TokenActorPanelRoot;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UButton> AddTokenButton;
	
	FOnTileEnvChangedDelegate OnTileEnvChanged;

protected:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> TileRowText;
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> TileColumnText;

	UPROPERTY(EditDefaultsOnly, BlueprintReadOnly)
	TSubclassOf<UBuildGridMapTokenActorPanel> TokenActorPanelClass;

public:
	void BindSingleTileCoord(FHCubeCoord SelectedCoord);
	void BindSingleTileData(const FSerializableTile& InTileData);
	
	void IntervalCreateTokenActorPanel(int32 InIndex, const FSerializableTokenData& InTokenData);
	void IntervalUpdateTokenActorPanel(int InIndex, const FSerializableTokenData& InTokenData);
	void IntervalDeleteTokenActorPanel(int32 InIndex);

	// UI上刷新PropertyArrayPenel
	void IntervalUpdateTokenPropertyArray(const int32 InTokenIndex,const int32 InFeatureIndex,const FName& InArrayName,
		const FSerializableTokenPropertyArray& InPropertyArrayData );

	// UI上刷新Property(单属性)
	void IntervalUpdateTokenProperty(const int32 InTokenIndex,const int32 InFeatureIndex,const FSerializableTokenProperty& InPropertyData);

	// UI上刷新Property(数组)
	void IntervalUpdateTokenPropertyInArray(const int32 InTokenIndex,const int32 InFeatureIndex,const FName& InArrayName,
		const int32 InArrayIndex,const FSerializableTokenProperty& InPropertyData);
	
private:
	UPROPERTY()
	TMap<FString, TObjectPtr<UGridEnvironmentType>> EnvTypeMap;
	UPROPERTY()
	TMap<FName, FString> EnvType2DisplayNameMap;
	
	UFUNCTION()
	void OnEnvTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
};
