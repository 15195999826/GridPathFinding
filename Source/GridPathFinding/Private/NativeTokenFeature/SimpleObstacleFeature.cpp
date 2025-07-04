﻿// Fill out your copyright notice in the Description page of Project Settings.


#include "NativeTokenFeature/SimpleObstacleFeature.h"

#include "GridMapModel.h"
#include "Service/GridPathFindingService.h"


TArray<FSerializableTokenProperty> USimpleObstacleFeature::SerializeFeatureProperties() const
{
	return TArray<FSerializableTokenProperty>();
}

void USimpleObstacleFeature::DeserializeFeatureProperties(const TArray<FSerializableTokenProperty>& Properties)
{
}

void USimpleObstacleFeature::UpdateFeatureProperty(const FSerializableTokenProperty& InNewProperty)
{
}

void USimpleObstacleFeature::InitGameplayFeature()
{
	auto MapModel = UGridPathFindingService::Get()->GetGridMapModel();
	
	if (!MapModel)
	{
		return;
	}

	MapModel->BlockTileOnce(GetOwner()->GetActorLocation());
}

void USimpleObstacleFeature::InitBuildGridMapFeature()
{
	// Todo: 在编辑器中， 标记该格子不可通行
}

// Sets default values for this component's properties
USimpleObstacleFeature::USimpleObstacleFeature()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	PrimaryComponentTick.bCanEverTick = false;

	// ...
}


// Called when the game starts
void USimpleObstacleFeature::BeginPlay()
{
	Super::BeginPlay();

	// ...
	
}