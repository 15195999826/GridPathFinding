// Fill out your copyright notice in the Description page of Project Settings.


#include "LandDynamicMeshActor.h"

#include "GeometryScript/MeshUVFunctions.h"
#include "GeometryScript/MeshVoxelFunctions.h"
#include "HexGrid.h"

void ALandDynamicMeshActor::TestGenerateLand()
{
	CreateHexMeshTemplate(100);
	
	DynMesh = GetDynamicMeshComponent()->GetDynamicMesh();
	DynMesh->Reset();
	
	UE_LOG(LogTemp, Error, TEXT("No HexGrid found in the scene"));
	for (int i = 0; i < 5; i++)
	{
		for (int j = 0; j < 5; j++)
		{
			AddHexTile(FVector(i * 150, j * 150, 0), 0, FRotator::ZeroRotator, 100);
		}
	}
	
	ReleaseComputeMesh(HexMeshTemplate);
	HexMeshTemplate = nullptr;
	ReleaseAllComputeMeshes();
}

void ALandDynamicMeshActor::GenerateLand(const TArray<FHexTile>& Array, const FRotator& HexRotator, float GridSize)
{
	CreateHexMeshTemplate(GridSize);
	DynMesh = GetDynamicMeshComponent()->GetDynamicMesh();
	DynMesh->Reset();
	
	for (const auto& Tile : Array)
	{
		AddHexTile(Tile.WorldPosition, Tile.Height, HexRotator, GridSize);
	}

	FGeometryScriptSolidifyOptions SolidifyOptions;
	SolidifyOptions.GridParameters.GridResolution = SolidifyResolution;

	// 体素化
	UGeometryScriptLibrary_MeshVoxelFunctions::ApplyMeshSolidify(DynMesh, SolidifyOptions);

	// UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
	// 	DynMesh,
	// 	FTransform::Identity,
	// 	DefaultPlaneCutOptions
	// 	);

	FTransform DstTransform = FTransform(FRotator::ZeroRotator, FVector(0, 0, 0), FVector(100.f, 100.f, 100.f));
	UGeometryScriptLibrary_MeshUVFunctions::SetMeshUVsFromPlanarProjection(DynMesh,0,DstTransform, FGeometryScriptMeshSelection());
	ReleaseComputeMesh(HexMeshTemplate);
	HexMeshTemplate = nullptr;
	ReleaseAllComputeMeshes();
}

void ALandDynamicMeshActor::CreateHexMeshTemplate(float GridSize)
{
	HexMeshTemplate = AllocateComputeMesh();
	UGeometryScriptLibrary_MeshPrimitiveFunctions::AppendBox(
		HexMeshTemplate,
		DefaultPrimitiveOptions,
		FTransform::Identity,
		Sqrt3 * GridSize,
		GridSize * 2,
		BelowGround
		);

	auto DeltaX = Sqrt3 * GridSize * 0.25f;
	auto DeltaY = GridSize * 0.75f;
	// 蓝图中的Rotator填写顺序是InRoll, InPitch, InYaw
	auto CutFrame = FTransform(
		FRotator(90.f, 60.f, 0.f),
		FVector(-DeltaX, -DeltaY, 0.f),
		FVector(1.f, 1.f, 1)
	);
	
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
	HexMeshTemplate,
	CutFrame,
	DefaultPlaneCutOptions
	);
	//
	CutFrame = FTransform(
		FRotator(-90.f, -60.f, 0.f),
		FVector(DeltaX, -DeltaY, 0.f),
		FVector(1.f, 1.f, 1));
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
	HexMeshTemplate,
	CutFrame,
	DefaultPlaneCutOptions
	);
	
	CutFrame = FTransform(
		FRotator(-90.f, 60.f, 0.f),
		FVector(DeltaX, DeltaY, 0.f),
		FVector(1.f, 1.f, 1));
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
		HexMeshTemplate,
		CutFrame,
		DefaultPlaneCutOptions
	);
	
	CutFrame = FTransform(
		FRotator(90.f, -60.f, 0.f),
		FVector(-DeltaX, DeltaY, 0.f),
		FVector(1.f, 1.f, 1));
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshPlaneCut(
		HexMeshTemplate,
		CutFrame,
		DefaultPlaneCutOptions
	);
}

void ALandDynamicMeshActor::AddHexTile(const FVector& Center, float Height, const FRotator& HexRotator, float GridSize)
{
	auto DstCenter = FVector(Center.X, Center.Y, Height - BelowGround);
	auto DstTransform = FTransform(HexRotator, DstCenter, FVector(1.f, 1.f, 1));
	// Todo: 改成预先生成好大地面，然后Apply 减法
	UGeometryScriptLibrary_MeshBooleanFunctions::ApplyMeshBoolean(
		DynMesh,
		FTransform::Identity,
		HexMeshTemplate,
		DstTransform,
		EGeometryScriptBooleanOperation::Union,
		DefaultBooleanOptions)
	;
}

void ALandDynamicMeshActor::AddHole(const FVector& Center, float Radius, float Height)
{
}

void ALandDynamicMeshActor::AddMountain(const FVector& Center, float Radius, float Height)
{
}
