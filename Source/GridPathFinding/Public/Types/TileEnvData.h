#pragma once
#include "CoreMinimal.h"
#include "TileEnvData.generated.h"

/**
 * GridEnvironmentType 是静态数据， FTileEnvData是动态数据， 各个格子在相同Env下可以有不同的参数
 * Todo: 不同项目中， TileEnvData的内容可能不同， 如何寻找一种通用的， 不依赖项目的方案
 */
USTRUCT(BlueprintType)
struct FTileEnvData
{
	GENERATED_BODY()

	FTileEnvData(): TextureIndex(0)
	{
	}

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int32 TextureIndex;
};
