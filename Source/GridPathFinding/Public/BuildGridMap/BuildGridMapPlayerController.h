// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridMapGameMode.h"
#include "LomoGeneralPlayerController.h"
#include "Components/SelectionComponent.h"
#include "Types/HCubeCoord.h"
#include "InputCoreTypes.h"

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
	ABuildGridMapPlayerController();
	virtual void BeginPlay() override;
	virtual void SetupInputComponent() override;

	// 选择组件
	UPROPERTY(VisibleAnywhere, Category="Selection")
	TObjectPtr<USelectionComponent> SelectionComponent;

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
		if (!SelectionComponent)
		{
			UE_LOG(LogTemp, Error, TEXT("[ABuildGridMapPlayerController.GetFirstSelectedCoord] SelectionComponent Invalid!"));
			return FHCubeCoord::Invalid;
		}

		if (SelectionComponent->GetSelectedTiles().Num() > 0)
		{
			return SelectionComponent->GetSelectedTiles()[0];
		}
		return FHCubeCoord::Invalid;
	}

	const TArray<FHCubeCoord>& GetSelectedCoords() const
	{
		return SelectionComponent->GetSelectedTiles();
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

	void OnSaveStart(EBuildGridMapSaveMode BuildGridMapSaveMode);
	void OnSaveOver();

	// 鼠标/键盘绑定 start
	void OnLeftMousePressHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
	                             const FVector& HitGroundLocation, AActor* InHitActor);
	void OnLeftMousePressingHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
	                                const FVector& HitGroundLocation, AActor* InHitActor);
	void OnLeftMouseReleaseHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
	                               const FVector& HitGroundLocation, AActor* InHitActor);
	void OnRightMouseReleaseHandler(float DeltaSeconds, bool OverWidget, bool IsHitGround,
	                                const FVector& HitGroundLocation, AActor* InHitActor);

	void OnKeyBoardZPressedHandler();
	void OnKeyBoardLeftShiftPressHandler();
	void OnKeyBoardLeftShiftReleaseHandler();
	// 鼠标/键盘绑定 end
};
