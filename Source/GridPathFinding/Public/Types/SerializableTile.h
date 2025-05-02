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

	friend bool operator==(const FSerializableTile& lhs, const FSerializableTile& rhs)
	{
		return /*lhs.Coord == rhs.Coord &&*/
			FMath::IsNearlyEqual(lhs.Height, rhs.Height) &&
			lhs.EnvironmentType == rhs.EnvironmentType &&
			lhs.TileEnvData == rhs.TileEnvData &&
			lhs.CustomGameplayData == rhs.CustomGameplayData;
	}

	friend bool operator!=(const FSerializableTile& lhs, const FSerializableTile& rhs)
	{
		return !(lhs == rhs);
	}
};
