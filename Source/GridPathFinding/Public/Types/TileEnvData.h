#pragma once
#include "CoreMinimal.h"
#include "GridEnvironmentType.h"
#include "TileEnvData.generated.h"

/**
 * GridEnvironmentType 是静态数据， FTileEnvData是动态数据， 各个格子在相同Env下可以有不同的参数
 * Todo: 不同项目中， TileEnvData的内容可能不同， 如何寻找一种通用的， 不依赖项目的方案
 */
USTRUCT(BlueprintType)
struct FTileEnvData
{
	GENERATED_BODY()

	static const FTileEnvData Invalid;
	
	FTileEnvData(): TextureIndex(0)
	{
		EnvironmentType = UGridEnvironmentType::EmptyEnvTypeID;
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FName EnvironmentType;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TextureIndex;
	

	FString ToString() const
	{
		FString Ret = FString::Printf(TEXT("TextureIndex: %d, EnvironmentType: %s"), TextureIndex, *EnvironmentType.ToString());
		return Ret;
	}

	friend bool operator==(const FTileEnvData& lhs, const FTileEnvData& rhs)
	{
		return lhs.TextureIndex == rhs.TextureIndex &&
			lhs.EnvironmentType == rhs.EnvironmentType;
	}

	friend bool operator!=(const FTileEnvData& lhs, const FTileEnvData& rhs)
	{
		return !(lhs == rhs);
	}
};
