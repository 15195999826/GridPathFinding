// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TokenFeatureInterface.h"
#include "Components/StaticMeshComponent.h"
#include "TokenMeshFeatureComponent.generated.h"

/**
 * 
 */
UCLASS(Blueprintable, ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDPATHFINDING_API UTokenMeshFeatureComponent : public UStaticMeshComponent, public ITokenFeatureInterface
{
	GENERATED_BODY()
	
	static const FName RelativePositionPropertyName;
	static const FName RelativeRotationPropertyName;
	static const FName RelativeScalePropertyName;
	static const FName SoftMeshPathPropertyName;
	// static const FName HiddenInGamePropertyName;

private:
	// UPROPERTY()
	// bool HiddenInProjectGame = false;
	
public:
	UTokenMeshFeatureComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual TArray<FSerializableTokenProperty> SerializeFeatureProperties() const override;

	virtual void DeserializeFeatureProperties(const TArray<FSerializableTokenProperty>& Properties) override;

	virtual void UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty) override;

	virtual void InitGameplayFeature() override{};
};
