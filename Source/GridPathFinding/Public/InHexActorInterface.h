// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HGTypes.h"
#include "UObject/Interface.h"
#include "InHexActorInterface.generated.h"

// This class does not need to be modified.
UINTERFACE(MinimalAPI)
class UInHexActorInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GRIDPATHFINDING_API IInHexActorInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual const FHCubeCoord& GetCurrentHexCoord() const = 0;
	virtual const FHCubeCoord& GetBookedHexCoord() const = 0;

	virtual void SetCurrentHexCoord(const FHCubeCoord& InCoord) = 0;
	virtual void SetBookedHexCoord(const FHCubeCoord& InCoord) = 0;

	virtual uint32 GetFlag() const = 0;
};
