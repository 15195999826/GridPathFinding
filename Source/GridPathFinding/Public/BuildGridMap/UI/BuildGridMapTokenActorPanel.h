// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SerializableTokenData.h"
#include "BuildGridMapTokenActorPanel.generated.h"

class UBuildGridMapFeaturePanel;
class UVerticalBox;
class UComboBoxString;


DECLARE_MULTICAST_DELEGATE_TwoParams(FOnTokenActorTypeChangedDelegate, int32 SerializedTokenIndex, const FString& SelectedTypeString);
DECLARE_MULTICAST_DELEGATE_FourParams(FOnTokenFeaturePropertyChangedDelegate, int32 SerializedTokenIndex, int32 FeatureIndex,const FName PropertyName, const FString NewValue);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenDeleteSigniture, int32, SerializedTokenIndex);
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnTokenAddPropertyArraySigniture, const int32 SerializedTokenIndex, const int32 FeatureIndex,const FName& InPropertyArrayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnTokenDeletePropertyArraySignature, const int32, SerializedTokenIndex, const int32, FeatureIndex, const FName& ,PropertyArrayName, const int32, ArrayIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_SixParams(FOnPropertyArrayValueChangedDelegate, const int32, SerializedTokenIndex, const int32, FeatureIndex, const FName& ,PropertyArrayName, const int32, ArrayIndex,
	const FName&, PropertyName, const FString&, NewValue);


/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapTokenActorPanel : public UUserWidget
{
	GENERATED_BODY()

	bool IsInitialized{false};
	void InitTokenActorPanel();
public:
	
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> TokenActorTypeComboBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UVerticalBox> FeatureRoot;

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<UBuildGridMapFeaturePanel> FeaturePanelClass;

	
	void BindTokenActor(int32 InTokeActorIndex, const FSerializableTokenData& TokenData);

	void UpdateTokenPropertyArray(const int32 InFeatureIndex,const FName& InArrayName,
		const FSerializableTokenPropertyArray& InPropertyArrayData);

	void UpdateTokenProperty(const int32 InFeatureIndex, const FSerializableTokenProperty& InPropertyData);

	void UpdateTokenPropertyInArray(const int32 InFeatureIndex,const FName& InArrayName,const int32 InArrayIndex,
		const FSerializableTokenProperty& InPropertyData);
	
	void Clean();

	FOnTokenActorTypeChangedDelegate OnTokenActorTypeChanged;

	FOnTokenFeaturePropertyChangedDelegate OnTokenFeaturePropertyChanged;
	
	FOnTokenAddPropertyArraySigniture OnAddTokenPropertyArraySigniture;
	
	UPROPERTY(BlueprintCallable)
	FOnTokenDeleteSigniture OnTokenDeleteClicked;

	FOnTokenDeletePropertyArraySignature OnTokenDeletePropertyArray;

	FOnPropertyArrayValueChangedDelegate OnPropertyArrayValueChanged;

protected:
	UPROPERTY(BlueprintReadOnly)
	int32 TokenActorIndex{0};

private:
	UFUNCTION()
	void OnTokenActorTypeSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);
	void OnFeaturePropertyChange(int InFeatureIndex,const FName& PropertyName, const FString& PropertyValue);

	UFUNCTION()
	void OnAddTokenPropertyArray(const int32 FeatureIndex,const FName& InPropertyArrayName);
	
	UFUNCTION()
	void OnDeleteTokenPropertyArray(const int32 InFeatureIndex, const FName& InPropertyArrayName, const int32 InArrayIndex);

	UFUNCTION()
	void OnTokenPropertyArrayValueChanged(const int32 InFeatureIndex, const FName& InPropertyArrayName, const int32 InArrayIndex,
		const FName& PropertyName,const FString& NewValue);
};
