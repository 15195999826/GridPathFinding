#pragma once
#include "CoreMinimal.h"

#include "MapConfig.generated.h"

// 地图类型: 标准六边形, 标准正方形, 标准矩形, 六方向矩形
UENUM(BlueprintType)
enum class EGridMapType : uint8
{
	HEX_STANDARD UMETA(DisplayName = "标准六边形"),
	SQUARE_STANDARD UMETA(DisplayName = "标准正方形"),
	RECTANGLE_STANDARD UMETA(DisplayName = "标准矩形"),
	RECTANGLE_SIX_DIRECTION UMETA(DisplayName = "六方向矩形")
};

UENUM(BlueprintType)
enum class ETileOrientationFlag : uint8
{
	FLAT UMETA(DisplayName = "平顶"),
	POINTY UMETA(DisplayName = "尖顶"),
};

UENUM(BlueprintType)
enum class EGridMapDrawMode : uint8
{
	BaseOnRadius UMETA(DisplayName = "基于半径"),
	BaseOnRowColumn UMETA(DisplayName = "基于行列"),
	BaseOnVolume UMETA(DisplayName = "基于框选")
};

UENUM(BlueprintType)
enum class EGridMapVolumeShapeType : uint8
{
	BOX UMETA(DisplayName = "立方体"),
	SPHERE UMETA(DisplayName = "球体"),
	// Todo: 其它类型
};

USTRUCT(BlueprintType)
struct FGripMapVolume
{
	GENERATED_BODY()

	FGripMapVolume(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Vol形状"))
	EGridMapVolumeShapeType ShapeType{EGridMapVolumeShapeType::BOX};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="中心位置"))
	FVector Center{FVector::ZeroVector};

	// Box类型Extent
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Box类型长宽高", EditCondition="ShapeType==EGridMapVolumeShapeType::BOX", EditConditionHides))
	FVector BoxExtent{FVector::ZeroVector};

	// Sphere类型Radius
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="Sphere类型半径", EditCondition="ShapeType==EGridMapVolumeShapeType::SPHERE", EditConditionHides))
	float SphereRadius{100.f};
};

// 进决定了地图如何绘制, 不包含寻路的配置
USTRUCT(BlueprintType)
struct FGridMapConfig
{
	GENERATED_BODY()

	FGridMapConfig(){}

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="地图类型"))
	EGridMapType MapType{EGridMapType::HEX_STANDARD};

	//六边形网格的半径
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="六边形网格半径", EditCondition="MapType==EGridMapType::HEX_STANDARD", EditConditionHides))
	float HexGridRadius{100.f};

	// 正方形网格的边长
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="正方形网格边长", EditCondition="MapType==EGridMapType::SQUARE_STANDARD", EditConditionHides))
	float SquareSize{100.f};
	
	// 矩形网格的宽， 长; X为前后方向的距离， Y为左右方向的距离
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="矩形网格长宽", EditCondition="MapType==EGridMapType::RECTANGLE_STANDARD || MapType==EGridMapType::RECTANGLE_SIX_DIRECTION", EditConditionHides))
	FVector2D RectSize{FVector2D(80.f, 100.f)};

	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="六边形网格朝向", EditCondition="MapType==EGridMapType::HEX_STANDARD || MapType==EGridMapType::RECTANGLE_SIX_DIRECTION", EditConditionHides))
	ETileOrientationFlag TileOrientation{ETileOrientationFlag::FLAT};
	
	// 决定了Tile的位置
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="地图绘制模式"))
	EGridMapDrawMode DrawMode{EGridMapDrawMode::BaseOnRowColumn};

	// 半径地图尺寸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="半径地图尺寸", EditCondition="DrawMode==EGridMapDrawMode::BaseOnRadius", EditConditionHides))
	int32 MapRadius{5};
	
	// 行列地图尺寸
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="行列地图尺寸", EditCondition="DrawMode==EGridMapDrawMode::BaseOnRowColumn", EditConditionHides))
	FIntPoint MapSize{FIntPoint(10, 10)};

	// 框选地图区域
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="框选地图区域", EditCondition="DrawMode==EGridMapDrawMode::BaseOnVolume", EditConditionHides))
	TArray<FGripMapVolume> PlaceVolumes;

	// 对任意类型地图都生效的剔除区域
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="剔除区域"))
	TArray<FGripMapVolume> SubtractVolumes;
	
	// 地图中心位置, 场景世界坐标
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta=(DisplayName="地图中心位置"))
	FVector MapCenter{FVector::ZeroVector};

	// 获取网格尺寸的辅助函数
	FVector2D GetGridSize() const;
};
