#pragma once
#include "CoreMinimal.h"
#include "HCubeCoord.h"
#include "TileEnvData.h"

#include "TileInfo.generated.h"

// 运行时Tile信息， 用于寻路、位置关系查询等， 不包含特定的Gameplay逻辑
USTRUCT(BlueprintType)
struct FTileInfo
{
	GENERATED_BODY()

	FTileInfo(){}

	FTileInfo(FHCubeCoord InCoord): CubeCoord(InCoord)
	{
	}

	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	FHCubeCoord CubeCoord;
	
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Height = 0.f;

	// 从环境类型中读取的Cost， 方便快速寻路
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	float Cost = 1.f;

	// 从环境类型中读取的是否是障碍物， 方便快速寻路
	// Block计数， 只有当数值为0时, 不被Block
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	int32 BlockCount = 0;

	//  ---- 寻路格子占用数据 Start----
	UPROPERTY(VisibleAnywhere, BlueprintReadWrite)
	bool HasAIBooked{false};

	// ---- 项目自定义数据 ----
	UPROPERTY()
	TMap<FName, FString> CustomDataMap;
	
	//  ----- 寻路格子占用数据 End----

	friend bool operator==(const FTileInfo &A, const FTileInfo &B)
	{
		return (A.CubeCoord == B.CubeCoord) && (A.Height == B.Height);
	}

	friend bool operator!=(const FTileInfo &A, const FTileInfo &B)
	{
		return !(A == B);
	}

	bool IsBlocking() const
	{
		return BlockCount > 0;
	}
};
