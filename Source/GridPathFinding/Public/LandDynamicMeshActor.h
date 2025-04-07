// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "GeometryScript/MeshBooleanFunctions.h"
#include "GeometryScript/MeshPrimitiveFunctions.h"
#include "LandDynamicMeshActor.generated.h"

struct FHexTile;
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API ALandDynamicMeshActor : public ADynamicMeshActor
{
	GENERATED_BODY()

protected:
	inline static FGeometryScriptPrimitiveOptions DefaultPrimitiveOptions = FGeometryScriptPrimitiveOptions();
	inline static FGeometryScriptMeshPlaneCutOptions DefaultPlaneCutOptions = FGeometryScriptMeshPlaneCutOptions();
	inline static FGeometryScriptMeshBooleanOptions DefaultBooleanOptions = FGeometryScriptMeshBooleanOptions();
	// 根号3
	inline static const float Sqrt3 = 1.7320508075688772935274463415059f;
	
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	int32 Seed;
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	int SolidifyResolution = 48;
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	float BelowGround = 400.f;
	// 在边缘生按Step间隔成对应数量的Cone， 在合并mesh时， 边缘的Cone数量越多， 边缘越平滑
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	int WidthEdgeSmoothStep;
	// 抖动的幅度
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	float WidthEdgeSmoothJitter;

	UPROPERTY(EditAnywhere, Category = "Land Generation")
	int HeightEdgeSmoothStep;

	UPROPERTY(EditAnywhere, Category = "Land Generation")
	float HeightEdgeSmoothJitter;

	// 在边缘生成的Cone的最小半径
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	int SmoothConeMinRadius;

	// 在边缘生成的Cone的最大半径
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	int SmoothConeMaxRadius;

	// 划定一个矩形区域， 保证Mesh生成在这个区域内
	UPROPERTY(EditAnywhere, Category = "Land Generation")
	FVector2f LandSize;
	
	UPROPERTY()
	TArray<FVector> ConeSpawnPoints;
	
	UPROPERTY(BlueprintReadOnly, Category="Land Generation")
	TObjectPtr<UDynamicMesh> DynMesh;

	UPROPERTY()
	UDynamicMesh* HexMeshTemplate;
	
public:
	UFUNCTION(BlueprintCallable, CallInEditor, Category="Land Generation")
	void TestGenerateLand();
	
	void GenerateLand(const TArray<FHexTile>& Array, const FRotator& HexRotator, float GridSize);

protected:
	UFUNCTION(BlueprintImplementableEvent, meta=(DisplayName="Add Hex"))
	void BP_AddHex(const FVector& Center, float Height, const FRotator& HexRotator, float GridSize);
	void CreateHexMeshTemplate(float GridSize);
	void AddHexTile(const FVector& Center, float Height, const FRotator& HexRotator, float GridSize);
	void AddHole(const FVector& Center, float Radius, float Height);
	void AddMountain(const FVector& Center, float Radius, float Height);
};
