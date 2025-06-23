// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Types/SerializableTokenData.h"
#include "UObject/Interface.h"
#include "TokenFeatureInterface.generated.h"

// This class does not need to be modified.
UINTERFACE()
class UTokenFeatureInterface : public UInterface
{
	GENERATED_BODY()
};

/**
 * 
 */
class GRIDPATHFINDING_API ITokenFeatureInterface
{
	GENERATED_BODY()

	// Add interface functions to this class. This is the class that will be inherited to implement this interface.
public:
	virtual TArray<FSerializableTokenProperty> SerializeFeatureProperties() const = 0;

	virtual void DeserializeFeatureProperties(const TArray<FSerializableTokenProperty>& Properties) = 0;

	virtual void UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty) = 0;

	virtual void InitGameplayFeature() = 0;
};
