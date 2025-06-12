#pragma once

#include "CoreMinimal.h"
#include "GridEnvironmentType.h"
#include "HCubeCoord.h"
#include "SerializableTokenData.h"
#include "TileEnvData.h"
#include "SerializableTile.generated.h"


USTRUCT(BlueprintType)
struct FSerializableTile
{
	GENERATED_BODY()

	FSerializableTile()
	{
	}

	static const FSerializableTile Invalid;

	// 坐标， 行列表示, 六边形地图中， 读取后转化为HCubeCoord
	UPROPERTY()
	FHCubeCoord Coord;

	UPROPERTY()
	float Height = 1;
	
	UPROPERTY()
	FTileEnvData TileEnvData;

	UPROPERTY()
	TArray<FSerializableTokenData> SerializableTokens; 

	UPROPERTY()
	FString CustomGameplayData;

	friend bool operator==(const FSerializableTile& lhs, const FSerializableTile& rhs)
	{
		return /*lhs.Coord == rhs.Coord &&*/
			FMath::IsNearlyEqual(lhs.Height, rhs.Height) &&
			lhs.TileEnvData == rhs.TileEnvData &&
			lhs.CustomGameplayData == rhs.CustomGameplayData;

		//Todo: 检查SerializableTokens是否相同
	}

	friend bool operator!=(const FSerializableTile& lhs, const FSerializableTile& rhs)
	{
		return !(lhs == rhs);
	}
};
