#pragma once

#include "CoreMinimal.h"
#include "GridEnvironmentType.h"
#include "HCubeCoord.h"
#include "TileEnvData.h"
#include "SerializableTile.generated.h"


USTRUCT(BlueprintType)
struct FSerializableTile
{
	GENERATED_BODY()

	FSerializableTile()
	{
		EnvironmentType = UGridEnvironmentType::EmptyEnvTypeID;
	}

	static const FSerializableTile Invalid;
	
	// 坐标， 行列表示, 六边形地图中， 读取后转化为HCubeCoord
	UPROPERTY()
	FHCubeCoord Coord;

	UPROPERTY()
	float Height = 1;

	UPROPERTY()
	FName EnvironmentType;

	UPROPERTY()
	FTileEnvData TileEnvData;

	UPROPERTY()
	FString CustomGameplayData;
};