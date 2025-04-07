// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/BuildGridMapRenderer.h"

#include "GridEnvironmentType.h"
#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "Components/InstancedStaticMeshComponent.h"


// Sets default values
ABuildGridMapRenderer::ABuildGridMapRenderer()
{
	// Set this actor to call Tick() every frame.  You can turn this off to improve performance if you don't need it.
	PrimaryActorTick.bCanEverTick = true;
}

// Called when the game starts or when spawned
void ABuildGridMapRenderer::BeginPlay()
{
	Super::BeginPlay();
}

// Called every frame
void ABuildGridMapRenderer::Tick(float DeltaTime)
{
	Super::Tick(DeltaTime);
}

void ABuildGridMapRenderer::OnTileModify(EGridMapModelTileModifyType GridMapModelTileModify, const FHCubeCoord& InCoord,
                                         const FTileInfo& OldTileInfo, const FTileInfo& NewTileInfo)
{
	switch (GridMapModelTileModify)
	{
		case EGridMapModelTileModifyType::Add:
			{
				const auto TileEnvData = GridModel->GetTileEnvDataMapPtr()->Find(InCoord);
				UpdateTile(InCoord, UGridEnvironmentType::EmptyEnvTypeID,
				           NewTileInfo.EnvironmentType, *TileEnvData);
			}
			break;
		case EGridMapModelTileModifyType::Remove:
			UpdateTile(InCoord, OldTileInfo.EnvironmentType,
			           UGridEnvironmentType::EmptyEnvTypeID, FTileEnvData());
			break;
		case EGridMapModelTileModifyType::Update:
			{
				const auto TileEnvData = GridModel->GetTileEnvDataMapPtr()->Find(InCoord);
				UpdateTile(InCoord, OldTileInfo.EnvironmentType, NewTileInfo.EnvironmentType,
				           *TileEnvData);
			}

			break;
	}
}