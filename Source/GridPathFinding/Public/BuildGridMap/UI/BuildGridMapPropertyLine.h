// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Types/SerializableTokenData.h"
#include "BuildGridMapPropertyLine.generated.h"

class UCheckBox;
class UComboBoxString;
class UEditableTextBox;
class UTextBlock;

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnPropertyValueChangedSignature, const FName&, PropertyName, const FString&, NewValue);

/**
 * Todo: 可以放到通用的Lomolib中， 作为常用的Key-Value对的显示组件
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridMapPropertyLine : public UUserWidget
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UTextBlock> LabelText;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> ComboBoxString;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UCheckBox> CheckBox;
	
	void InitPropertyLine(const FSerializableTokenProperty& InPropertyLine);
	void Clean();

	UPROPERTY(BlueprintCallable)
	FOnPropertyValueChangedSignature OnPropertyValueChanged;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FString GetValueString();

	const FName& GetPropertyName() const
	{
		return PropertyName;
	}

	void UpdateProperty(const FSerializableTokenProperty& InPropertyData);
	
protected:
	UFUNCTION(BlueprintImplementableEvent)
	void SetVectorValue(const FVector& Value);

	UFUNCTION(BlueprintImplementableEvent)
	FVector GetVectorValue();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetGeneralValue(const FString& Value);

	UFUNCTION(BlueprintImplementableEvent)
	FString GetGeneralValue();

	UFUNCTION(BlueprintImplementableEvent)
	void SetComboBoxValue(const FString& Value);

	UFUNCTION(BlueprintImplementableEvent)
	FString GetComboBoxValue();
	
	UFUNCTION(BlueprintImplementableEvent)
	void SetCheckBoxValue(const FString& Value);

	UFUNCTION(BlueprintImplementableEvent)
	FString GetCheckBoxValue();
	
	
	UPROPERTY(BlueprintReadOnly)
	FName PropertyName;
	
private:
	ETokenPropertyType PropertyType{ETokenPropertyType::None};
};
