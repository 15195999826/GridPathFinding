// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "Types/GridMapSave.h"
#include "GridPathFindingBlueprintFunctionLib.generated.h"

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
};
