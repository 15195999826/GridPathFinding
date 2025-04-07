#pragma once
#include "CoreMinimal.h"

#include "MapRandomConfig.generated.h"


UENUM(BlueprintType)
enum class ETileHeightRandomType : uint8
{
	NONE UMETA(DisplayName = "无高度差"),
	// NOISE UMETA(DisplayName = "噪声"), TODO: 噪声高度差，暂时没必要
	RDHeightArea UMETA(DisplayName = "随机高度区块")
};

// 随机Cost: 读取环境配置中的全部地块的Cost
UENUM(BlueprintType)
enum class ETileCostRandomType : uint8
{
	// 总是使用最小Cost的地块
	NONE UMETA(DisplayName = "总是最小Cost"),
	SIMPLE UMETA(DisplayName = "简单随机"),
	// 噪声值Map到 Cost区间
	NOISE UMETA(DisplayName = "噪声"),
};

USTRUCT(BlueprintType)
struct FMapRandomConfig
{
	GENERATED_BODY()

	FMapRandomConfig(){}

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(DisplayName="随机种子"))
	int32 Seed = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="地形高度随机算法"))
	ETileHeightRandomType HeightRandomType {ETileHeightRandomType::NONE};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Cost随机算法"))
	ETileCostRandomType CostRandomType {ETileCostRandomType::NONE};
};
