// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/BuildGridMapRenderer.h"

#include "GridEnvironmentType.h"
#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "SpringArmCameraActor.h"
#include "BuildGridMap/BuildGridMapBlueprintFunctionLib.h"
#include "BuildGridMap/BuildGridMapPlayerController.h"
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

void ABuildGridMapRenderer::RenderTiles()
{
	// 更新光标尺寸
	auto SpringArmCamera = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController())->GetSpringArmCamera();
	SpringArmCamera->UpdateCursorSize(UBuildGridMapBlueprintFunctionLib:: GetCursorSize(*GridModel->GetMapConfigPtr()));
	Super::RenderTiles();
}

void ABuildGridMapRenderer::OnTileEnvUpdate(const FHCubeCoord& InCoord, const FTileEnvData& OldTileEnv,
                                            const FTileEnvData& NewTileEnv)
{
	
	UpdateTileEnvRenderer(InCoord,OldTileEnv, NewTileEnv);
}
