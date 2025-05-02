// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Components/Border.h"
#include "Components/CanvasPanel.h"
#include "SelectionBoxWidget.generated.h"

/**
 * 选择框
 */
UCLASS()
class GRIDPATHFINDING_API USelectionBoxWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	USelectionBoxWidget(const FObjectInitializer& ObjectInitializer);

	UFUNCTION(BlueprintNativeEvent, Category="Selection")
	void UpdateSelectionBox(const FVector2D& StartPosition, const FVector2D& CurrentPosition);

	UFUNCTION(BlueprintCallable, Category="Selection")
	void SetSelectionBoxVisible(bool bVisible);

protected:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadWrite, meta = (BindWidget))
	TObjectPtr<UBorder> SelectionBorder;
};
