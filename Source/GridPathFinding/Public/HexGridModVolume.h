// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Volume.h"
#include "HexGridModVolume.generated.h"

UENUM(BlueprintType)
enum class EHexGridModType : uint8
{
	Subtract UMETA(DisplayName = "剔除"),
};

/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API AHexGridModVolume : public AVolume
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	EHexGridModType ModType;
};
