// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/SerializableTokenData.h"
#include "Blueprint/UserWidget.h"
#include "BuildGridMap/UI/BuildGridMapPropertyLine.h"
#include "BuildGridMapPropertyLineArray.generated.h"

/**
 * 
 */

class UVerticalBox;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnTokenDeletePropertyArrayDelegate,const int32, PropertyArrayIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FOnChangePropertyArrayValueDelegate,const int32, ArrayIndex,
	const FName& ,PropertyName, const FString&, NewValue);

UCLASS()
class GRIDPATHFINDING_API UBuildGridMapPropertyLineArray : public UUserWidget
{
	GENERATED_BODY()

public:
	
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UVerticalBox> PropertiesRoot;
	
	UPROPERTY(BlueprintCallable,BlueprintAssignable)
	FOnTokenDeletePropertyArrayDelegate OnTokenDeletePropertyArrayDelegate;
	
	void BindPropertiesArrayData(int32 InPropertyArrayIndex, const TArray<FSerializableTokenProperty>& InPropertyArrayData);

	UPROPERTY(BlueprintReadOnly, EditDefaultsOnly)
	TSubclassOf<UBuildGridMapPropertyLine> PropertyLineClass;

	void Clean();

	FOnChangePropertyArrayValueDelegate OnChangePropertyArrayValueDelegate;

	void UpdatePropertyInArray(const FSerializableTokenProperty& InPropertyData);
	
protected:
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetPropertyArrayName();

	UPROPERTY(BlueprintReadOnly)
	int32 ArrayIndex = -1;

private:
	UFUNCTION(BlueprintCallable)
	void DeletePropertyArrayButtonClick();

	UFUNCTION()
	void PropertyValueChanged(const FName& PropertyName, const FString& NewValue);
};
