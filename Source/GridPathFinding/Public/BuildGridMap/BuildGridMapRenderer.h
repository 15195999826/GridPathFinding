// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridEnvironmentType.h"
#include "GridMapRenderer.h"
#include "Types/TileEnvData.h"
#include "BuildGridMapRenderer.generated.h"

UCLASS()
class GRIDPATHFINDING_API ABuildGridMapRenderer : public AGridMapRenderer
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABuildGridMapRenderer();

protected:
	// Called when the game starts or when spawned
	virtual void BeginPlay() override;

public:
	// Called every frame
	virtual void Tick(float DeltaTime) override;
	
protected:
	virtual void RenderTiles() override;

	virtual void OnTileEnvUpdate(const FHCubeCoord& InCoord, const FTileEnvData& OldTileEnv, const FTileEnvData& NewTileEnv) override;
};
