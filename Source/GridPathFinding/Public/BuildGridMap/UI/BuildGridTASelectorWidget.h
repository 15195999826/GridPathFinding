// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "TokenActor.h"
#include "Blueprint/UserWidget.h"
#include "BuildGridTASelectorWidget.generated.h"

class UComboBoxString;

/**
 * TokenActor 选择器小部件
 */
UCLASS()
class GRIDPATHFINDING_API UBuildGridTASelectorWidget : public UUserWidget
{
	GENERATED_BODY()

public:
	virtual void NativeConstruct() override;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> TypeComboBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UComboBoxString> ActorComboBox;

	UPROPERTY(BlueprintReadOnly, meta=(BindWidget))
	TObjectPtr<UWidget> ActorSelectorGo;

	UFUNCTION(BlueprintCallable)
	void OnCreateClick();

	UFUNCTION(BlueprintCallable)
	void OnDeleteClick();

	UFUNCTION()
	void OnTypeNameSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

	UFUNCTION()
	void OnActorClassSelectionChanged(FString SelectedItem, ESelectInfo::Type SelectionType);

private:
	FString SelectedClassPath;

	void ShowInfo();
	void UpdateTypeComboBox();
	void UpdateActorComboBox();
	void SpawnActorByPath(const FString& ClassPath);

	// 获取指定类型的 TokenActor 类
	static TArray<TSubclassOf<ATokenActor>> GetTokenActors(ETokenActorType Type)
	{
		const UGridPathFindingSettings* Settings = GetDefault<UGridPathFindingSettings>();
		if (Settings && Settings->TokenActorClassMap.Contains(Type))
		{
			return Settings->TokenActorClassMap[Type].TokenActorClasses;
		}

		UE_LOG(LogGridPathFinding, Error, TEXT("[UBuildGridTASelectorWidget.GetTokenActors] No token actors found for type: %s"), *UEnum::GetValueAsString(Type));
		return TArray<TSubclassOf<ATokenActor>>();
	}
};
