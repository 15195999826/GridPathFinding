// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "Engine/DeveloperSettings.h"
#include "Types/MapConfig.h"
#include "GridPathFindingSettings.generated.h"

class ATokenActor;
class AGridMapRenderer;
class UGridMapBuilder;
class IGridMapLoaderInterface;
class UGridEnvironmentType;

UENUM(BlueprintType)
enum class ETokenActorType : uint8
{
	None UMETA(DisplayName = "None"),
	SpawnPoint UMETA(DisplayName = "出生点"),
	Wall UMETA(DisplayName = "傅西墙"),
	Door UMETA(DisplayName = "门"),
	
	CharacterNPC UMETA(DisplayName = "角色-生物"),
	CharacterPlayer UMETA(DisplayName = "角色-玩家"),
	
	DecorationCommon UMETA(DisplayName = "物件-通用"),
	DecorationTreasure UMETA(DisplayName = "物件-宝箱"),
	DecorationTrap UMETA(DisplayName = "物件-陷阱"),
	DecorationLight UMETA(DisplayName = "物件-光源"),

	Environment UMETA(DisplayName = "环境"),
};

USTRUCT(BlueprintType)
struct FTokenActorClassArray
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "TokenActor")
	TArray<TSubclassOf<ATokenActor>> TokenActorClasses;

	FTokenActorClassArray() = default;

	FTokenActorClassArray(const TArray<TSubclassOf<ATokenActor>>& InTokenActorClasses)
		: TokenActorClasses(InTokenActorClasses)
	{
	}
};

/**
 * 
 */
UCLASS(config=Game, DefaultConfig)
class GRIDPATHFINDING_API UGridPathFindingSettings : public UDeveloperSettings
{
	GENERATED_BODY()

public:
	virtual FName GetCategoryName() const override;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "环境类型"))
	TArray<TSoftObjectPtr<UGridEnvironmentType>> EnvironmentTypes;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "基础六边形边框尺寸"))
	float BaseHexFrameRadius = 100.f;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "基础六边形格子尺寸"))
	float BaseHexGridRadius = 100.f;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "地图存档文件夹名"))
	FString MapSaveFolder;

	// Todo: 读取Json文件，创建属性编辑窗口
	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "自定义格子数据Json文件"))
	FString CustomGridDataJsonFile;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "地图Chunk尺寸"))
	FIntPoint MapChunkSize = FIntPoint(25, 25);

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "六边形网格朝向"))
	ETileOrientationFlag HexTileOrientation = ETileOrientationFlag::FLAT;
	
	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "TokenActor根目录"))
	FString TokenActorRootDir;

	UPROPERTY(config, EditAnywhere, meta = (DisplayName = "可用Mesh根目录数组"))
	TArray<FString> UsingMeshRootPaths;

	UPROPERTY(Config, EditAnywhere, meta = (DisplayName = "TokenActor承载物"))
	TMap<ETokenActorType, FTokenActorClassArray> TokenActorClassMap;
};
