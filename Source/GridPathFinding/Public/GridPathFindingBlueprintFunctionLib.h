// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HGTypes.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/GridMapSave.h"
#include "GridPathFindingBlueprintFunctionLib.generated.h"

class UGridMapModel;
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UGridPathFindingBlueprintFunctionLib : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintPure)
	static TArray<FName> GetAllMapSaveNames();

	UFUNCTION(BlueprintCallable)
	static FGridMapSave LoadGridMapSave(FName InMapName);
	static FString GGB_SerializeSaveData(FGGB_SaveData Data);
	static FGGB_SaveData GGB_DeSerializeSaveData(FString Base64Str);

	// 将FGridMapTilesSave序列化为Base64字符串
	UFUNCTION(BlueprintCallable, Category = "GridPathFinding|Serialization")
	static FString SerializeGridMapTiles(FGridMapTilesSave TilesSave);

	// 从Base64字符串反序列化为FGridMapTilesSave
	UFUNCTION(BlueprintCallable, Category = "GridPathFinding|Serialization")
	static FGridMapTilesSave DeserializeGridMapTiles(const FString& Base64Str);

	// 将FGridMapTilesSave保存到文件
	// UFUNCTION(BlueprintCallable, Category = "GridPathFinding|Serialization")
	static bool SaveGridMapTilesToFile(const FGridMapTilesSave& TilesSave, const FString& FilePath);

	// 从文件加载FGridMapTilesSave， 异步加载逻辑自行在实际游戏逻辑中实现
	// UFUNCTION(BlueprintCallable, Category = "GridPathFinding|Serialization")
	static bool LoadGridMapTilesFromFile(const FString& FilePath, FGridMapTilesSave& OutTilesSave);

	/**
	 * Coord是否在地图范围内
	 * @param InMapConfig 
	 * @param InCoord 
	 * @return
	 */
	static bool IsCoordInMapArea(const FGridMapConfig& InMapConfig, const FHCubeCoord& InCoord);

	/**
	 * 行列转换
	 * @param InMapConfig 
	 * @param InCoord 
	 * @return 
	 */
	static FIntVector2 StableCoordToRowColumn(const FGridMapConfig& InMapConfig, const FHCubeCoord& InCoord);

	/**
	 * 遍历地图格子
	 * @param InMapConfig
	 * @param Func 
	 */
	static void StableForEachMapGrid(const FGridMapConfig& InMapConfig, TFunction<void(const FHCubeCoord& Coord, int32 Row, int32 Column)> Func);

	/**
	 * Cube坐标转世界坐标
	 * @param InMapConfig 
	 * @param InTileOrientation 
	 * @param InCoord 
	 * @return 
	 */
	static FVector StableCoordToWorld(const FGridMapConfig& InMapConfig, const FHTileOrientation& InTileOrientation, const FHCubeCoord& InCoord);
};
