// Fill out your copyright notice in the Description page of Project Settings.


#pragma once

#include "CoreMinimal.h"
#include "TokenFeatureInterface.h"
#include "BuildGridMap/BuildTokenFeatureInterface.h"
#include "Components/ActorComponent.h"
#include "SimpleObstacleFeature.generated.h"


UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class GRIDPATHFINDING_API USimpleObstacleFeature : public UActorComponent, public ITokenFeatureInterface, public IBuildTokenFeatureInterface

{
	GENERATED_BODY()

	virtual FSerializableTokenFeature SerializeFeatureProperties() const override;

	virtual void DeserializeFeatureProperties(const FSerializableTokenFeature& TokenFeature) override;

	virtual void UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty) override;

	virtual void InitGameplayFeature(UGridMapModel* MapModel) override;

	virtual void InitBuildGridMapFeature() override;

	virtual TArray<FSerializableTokenProperty> CreatePropertyArray(const FName& PropertyArrayName) override;

	virtual void UpdateFeaturePropertyArray(const TArray<FSerializableTokenProperty>& InNewPropertyArray,const FName& PropertyArrayName,const int32 UpdateIndex) override;
	
public:
	// Sets default values for this component's properties
	USimpleObstacleFeature();

protected:
	// Called when the game starts
	virtual void BeginPlay() override;
};

