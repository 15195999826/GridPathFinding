// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridMapPropertyLineArray.h"
#include "Blueprint/UserWidget.h"
#include "Components/Button.h"
#include "Types/SerializableTokenData.h"

#include "BuildGridMapPropertyArrayPanel.generated.h"


class UVerticalBox;
DECLARE_MULTICAST_DELEGATE_ThreeParams(FOnFeaturePropertyChangedDelegate, int32 FeatureIndex, const FName& PropertyName, const FString& NewValue);
DECLARE_MULTICAST_DELEGATE_OneParam(FOnTokenAddPropertyArrayDelegate,const FName& PropertyArrayName);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnDeletePropertyArrayDelegate, const FName&, ArrayName, int32, ArrayIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FourParams(FOnChangePropertyArrayValueSignature, const FName&, ArrayName, int32, ArrayIndex,
	const FName&, PropertyName, const FString&, NewValue);

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapPropertyArrayPanel : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<UBuildGridMapPropertyLineArray> PropertyLineArrayClass;

	// 增加数组元素按钮
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UButton> AddPropertiesButton;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UVerticalBox> PropertiesArrayRoot;

	void BindPropertiesArrayData(int32 InPropertiesArrayIndex, const FSerializableTokenPropertyArray& InPropertiesArrayData);

	FOnTokenAddPropertyArrayDelegate OnAddPropertyArrayDelegate;
	
	FOnDeletePropertyArrayDelegate OnDeletePropertyArrayDelegate;

	FOnChangePropertyArrayValueSignature OnChangePropertyArrayValueSignature;
	
	void Clean();

	const FName& GetArrayName() const
	{
		return ArrayName;
	}

	void UpdatePropertyArray(const FSerializableTokenPropertyArray& InPropertyArrayData);

	void UpdatePropertyInArray(const int32 InArrayIndex,const FSerializableTokenProperty& InPropertyData);
	
protected:
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetArrayName(const FString& InArrayName);
	
private:

	UPROPERTY()
	int ArrayIndex;
	
	UPROPERTY()
	FName ArrayName;

	UFUNCTION()
	void AddPropertyArrayButtonClick();

	UFUNCTION()
	void DeletePropertyArrayButtonClick(const int32 InArrayIndex);

	UFUNCTION()
	void PropertyArrayValueChanged(const int32 InArrayIndex,const FName& PropertyName,const FString& NewValue);
};
