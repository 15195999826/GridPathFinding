// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "TokenFeatureInterface.h"
#include "Components/StaticMeshComponent.h"

#include "TokenMeshFeatureComponent.generated.h"

USTRUCT()
struct FMyStruct
{
	GENERATED_BODY()

	FName Name;
	int32 Int;
};

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

	FMyStruct TestStruct;
	
public:
	UTokenMeshFeatureComponent(const FObjectInitializer& ObjectInitializer);
	
	virtual FSerializableTokenFeature SerializeFeatureProperties() const override;

	virtual void DeserializeFeatureProperties(const FSerializableTokenFeature& TokenFeature) override;

	virtual void UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty) override;

	virtual void InitGameplayFeature() override{};

	virtual TArray<FSerializableTokenProperty> CreatePropertyArray(const FName& PropertyArrayName) override;
	
	virtual void UpdateFeaturePropertyArray(const TArray<FSerializableTokenProperty>& InNewPropertyArray,const FName& PropertyArrayName,const int32 UpdateIndex) override;
};
