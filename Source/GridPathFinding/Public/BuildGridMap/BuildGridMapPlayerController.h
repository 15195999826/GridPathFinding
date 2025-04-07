// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridMapGameMode.h"
#include "LomoGeneralPlayerController.h"
#include "Types/HCubeCoord.h"

#include "BuildGridMapPlayerController.generated.h"

UENUM()
enum class EMouseMode
{
	SingleSelect,
	MultiSelect,
	// 涂色
	Paint,
};

class ABuildGridMapGameMode;

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API ABuildGridMapPlayerController : public ALomoGeneralPlayerController
{
	GENERATED_BODY()

public:
	virtual void BeginPlay() override;

protected:
	UPROPERTY()
	TWeakObjectPtr<ABuildGridMapGameMode> WeakGM;
	virtual void RemapHitLocation(FVector& HitLocation) override;

	virtual void CustomTick(float DeltaSeconds, bool OverWidget, bool IsHitGround, const FVector& HitGroundLocation, AActor* InHitActor) override;
public:
	UFUNCTION(BlueprintCallable)
	void ChangeMouseMode(EMouseMode InMouseMode);
	
	const FHCubeCoord& GetFirstSelectedCoord() const
	{
		if (SelectedCoords.Num() > 0)
		{
			return SelectedCoords[0];
		}
		return FHCubeCoord::Invalid;
	}

	void SetCanInput(bool bCanInput)
	{
		CanNotInput = !bCanInput;
	}

protected:
	UPROPERTY(BlueprintReadOnly)
	bool CanNotInput{false};
	
private:
	EMouseMode MouseMode{EMouseMode::SingleSelect};

	// 单选模式
	TArray<FHCubeCoord> SelectedCoords;
	
	void OnSaveStart(EBuildGridMapSaveMode BuildGridMapSaveMode);
	void OnSaveOver();
};
