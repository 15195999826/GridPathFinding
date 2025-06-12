// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SerializableTokenData.h"
#include "BuildGridMapFeaturePanel.generated.h"

class UVerticalBox;
class UBuildGridMapPropertyLine;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFeaturePropertyChangedDelegate, int32 FeatureIndex, const FName& PropertyName, const FString& NewValue);

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapFeaturePanel : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UVerticalBox> PropertiesRoot;
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<UBuildGridMapPropertyLine> PropertyLineClass;

	void BindFeatureData(int32 InFeatureIndex, const FSerializableTokenFeature& InFeatureData);
	void Clean();

	FOnFeaturePropertyChangedDelegate OnFeaturePropertyChanged;
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SetFeatureName(const FString& InFeatureName);

	UFUNCTION()
	void OnPropertyLineChanged(const FName& PropertyName, const FString& NewValue);
	
private:
	int32 FeatureIndex = -1;
};
