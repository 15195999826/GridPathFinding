// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "BuildGridMapGameMode.h"
#include "LomoGeneralPlayerController.h"
#include "Components/SelectionComponent.h"
#include "Types/HCubeCoord.h"
#include "InputCoreTypes.h"

#include "BuildGridMapPlayerController.generated.h"

UENUM(BlueprintType)
enum class EMouseMode : uint8
{
	Invalid UMETA(DisplayName = "无效"),
	Select UMETA(DisplayName = "选择"),
	Paint UMETA(DisplayName = "画笔")
};

UENUM(BlueprintType)
enum class ESelectFilterType : uint8
{
	None UMETA(DisplayName = "无"),
	Tile UMETA(DisplayName = "格子"),
	Volume UMETA(DisplayName = "体积"),
};

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

	UPROPERTY(BlueprintReadOnly)
	EMouseMode MouseMode{EMouseMode::Select};

	UPROPERTY(BlueprintReadOnly)
	ESelectFilterType SelectFilterType{ESelectFilterType::None};

	// 选择组件
	UPROPERTY(VisibleAnywhere, Category="Selection")
	TObjectPtr<USelectionComponent> SelectionComponent;

protected:
	UPROPERTY()
	TWeakObjectPtr<ABuildGridMapGameMode> WeakGM;

	virtual void RemapHitLocation(FVector& HitLocation, bool IsHitGround, AActor* InHitActor) override;

	virtual void CustomTick(float DeltaSeconds, bool OverWidget, bool IsHitGround, const FVector& HitGroundLocation, AActor* InHitActor, UPrimitiveComponent* InHitComponent, const
	                        FVector& InHitResultLocation) override;

public:
	UFUNCTION(BlueprintCallable)
	void ChangeMouseModeEnum(EMouseMode InMouseMode);
	UFUNCTION(BlueprintCallable)
	void ChangeMouseModeInt(int32 InMouseMode);

	UFUNCTION(BlueprintCallable)
	void ChangeSelectFilterType(ESelectFilterType InSelectFilterType);

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
	void OnSaveStart(EBuildGridMapSaveMode BuildGridMapSaveMode);
	void OnSaveOver();

	// ------ 复制粘贴 Start-------
	FSerializableTile CopiedTileData;
	FHCubeCoord CopiedSourceCoord;
	void CopySelectedTile();
	void PasteToSelectedTiles();
	// ------ 复制粘贴 End -------
	
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
	
	void OnKeyBoardCPressedHandler();
	void OnKeyBoardVPressedHandler();
	// 鼠标/键盘绑定 end
};
