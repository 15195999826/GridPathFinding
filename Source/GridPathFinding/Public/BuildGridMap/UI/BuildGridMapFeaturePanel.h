// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridMapPropertyArrayPanel.h"
#include "Blueprint/UserWidget.h"
#include "Types/SerializableTokenData.h"
#include "BuildGridMapFeaturePanel.generated.h"

class UVerticalBox;
class UBuildGridMapPropertyLine;

DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFeaturePropertyChangedDelegate, int32 FeatureIndex, const FName& PropertyName, const FString& NewValue);
DECLARE_MULTICAST_DELEGATE_TwoParams(FOnAddTokenPropertyArrayDelegate,const int32 FeatureIndex,const FName& PropertyArrayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnDeletePropertyArraySignature,const int32 ,FeatureIndex,const FName&,PropertyArrayName,const int32,ArrayIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FOnPropertyArrayValueChangedSignature,const int32 ,FeatureIndex,const FName&,PropertyArrayName,const int32,ArrayIndex,
	const FName&,PropertyName,const FString&,NewValue);

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

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<UBuildGridMapPropertyArrayPanel> PropertyArrayPanelClass;
	
	void BindFeatureData(int32 InFeatureIndex, const FSerializableTokenFeature& InFeatureData);
	void Clean();

	FOnFeaturePropertyChangedDelegate OnFeaturePropertyChanged;

	FOnAddTokenPropertyArrayDelegate OnAddTokenPropertyArrayDelegate;

	FOnDeletePropertyArraySignature OnDeletePropertyArraySignature;

	FOnPropertyArrayValueChangedSignature OnPropertyArrayValueChangedSignature;

	void UpdateTokenPropertyArray(const FName& InArrayName,const FSerializableTokenPropertyArray& InPropertyArrayData);

	void UpdateTokenProperty(const FSerializableTokenProperty& InPropertyData);

	void UpdateTokenPropertyInArray(const FName& InArrayName,const int32 InArrayIndex, const FSerializableTokenProperty& InPropertyData);
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SetFeatureName(const FString& InFeatureName);

	UFUNCTION()
	void OnPropertyLineChanged(const FName& PropertyName, const FString& NewValue);
	
	UFUNCTION()
	void AddTokenPropertyAraay(const FName& PropertyArrayName);

	UFUNCTION()
	void OnDeleteTokenPropertyArray(const FName& InPropertyArrayName,const int32 InArrayIndex);

	UFUNCTION()
	void PropertyArrayValueChanged(const FName& ArrayName,const int32 InArrayIndex,const FName& PropertyName,const FString& NewValue);
	
private:
	int32 FeatureIndex = -1;
};
