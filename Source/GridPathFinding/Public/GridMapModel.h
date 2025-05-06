// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HGTypes.h"
#include "Types/GridMapSave.h"
#include "Types/TileInfo.h"
#include "UObject/Object.h"
#include "GridMapModel.generated.h"

UENUM()
enum class EGridMapModelTileModifyType
{
	Add,
	Remove,
	Update,
};

DECLARE_MULTICAST_DELEGATE_FourParams(FGridMapMoldelTileModifyDelegate, EGridMapModelTileModifyType, const FHCubeCoord&, const FTileInfo& OldTileInfo, const FTileInfo& NewTileInfo);

/**
 * 地图的逻辑层数据
 * 插件内格子本身不承载自定义玩法数据， 只处理寻路的功能
 * 可能的处理方式：
 * 1. EnvironmentType的Decoration类型设置为Actor， 然后自定义数据放在该Actor上
 * 2. 继承UGridMapModel， 重写BuildTilesData， 新建一个数组orMap来创建自定义数据
 *
 * 就目前规划而言， 自定义数据会使用Json字符串的形式， 无论任何数据结构都可编辑， 那些数据我将通过String的形式保存FSerializableGridTile中
 * 在项目中使用方式2应该更好
 *
 * 坐标方向：
 * 采用默认UE摄像机默认坐标轴， X轴向后， Y轴向右， Z轴向上
 */
UCLASS()
class GRIDPATHFINDING_API UGridMapModel : public UObject
{
	GENERATED_BODY()

public:
	UGridMapModel();

protected:
	virtual void BeginDestroy() override;

protected:
	UPROPERTY()
	TArray<FTileInfo> Tiles;

	// 记录格子上的Actor, 寻路系统会使用该数据来判断格子是否被占用
	UPROPERTY()
	TMap<FHCubeCoord, TWeakObjectPtr<AActor>> StandingActors;

	// 用于寻路时快速查询Tile数据
	UPROPERTY()
	TMap<FHCubeCoord, int32> TileCoordToIndexMap;

	// 环境数据，目前只存放了运行时的渲染数据
	UPROPERTY()
	TMap<FHCubeCoord, FTileEnvData> TileEnvDataMap;

	UPROPERTY()
	FGridMapConfig MapConfig;

	UPROPERTY()
	bool IsBuilding{false};

public:
	FSimpleMulticastDelegate OnTilesDataBuildCancel;
	/** 地图数据构建完成事件 */
	FSimpleMulticastDelegate OnTilesDataBuildComplete;

	// Build时不会抛出事件， 仅Build完成后再修改才会Broadcast
	FGridMapMoldelTileModifyDelegate OnTileModify;

	/** 
	 * 构建地图数据
	 * 考虑到 InTilesData 的数据量可能会比较大，采用异步任务填充 Tiles 数组
	 * 允许在再次调用此函数时，终止上次的异步任务，清理数据后启动新的异步任务
	 */
	virtual void BuildTilesData(const FGridMapConfig& InMapConfig, const TMap<FHCubeCoord, FSerializableTile>& InTilesData);

	void ModifyTilesData(EGridMapModelTileModifyType ModifyType, const FSerializableTile& InTileData, bool bNotify = true);

	bool IsBuildingTilesData() const
	{
		return IsBuilding;
	}

	const FGridMapConfig* GetMapConfigPtr() const
	{
		return &MapConfig;
	}

	const TArray<FTileInfo>* GetTilesArrayPtr() const
	{
		return &Tiles;
	}

	const TMap<FHCubeCoord, FTileEnvData>* GetTileEnvDataMapPtr() const
	{
		return &TileEnvDataMap;
	}

	const FTileInfo* GetTilePtr(const FHCubeCoord& InCoord) const
	{
		if (TileCoordToIndexMap.Contains(InCoord))
		{
			return &Tiles[TileCoordToIndexMap[InCoord]];
		}

		return nullptr;
	}


	void UpdateStandingActor(const FHCubeCoord& OldCoord, const FHCubeCoord& NewCoord, AActor* InActor);

	bool TryGetStandingActor(const FHCubeCoord& Coord, AActor*& OutActor) const
	{
		if (StandingActors.Contains(Coord))
		{
			OutActor = StandingActors[Coord].Get();
			return OutActor != nullptr;
		}

		OutActor = nullptr;
		return false;
	}

protected:
	/** 异步任务类，用于填充 Tiles 数组 */
	class FBuildTilesDataTask : public FNonAbandonableTask
	{
	public:
		FBuildTilesDataTask(UGridMapModel* InOwner, const TMap<FHCubeCoord, FSerializableTile>& InTilesData,
		                    TSharedPtr<TArray<FTileInfo>> OutTiles, TSharedPtr<TMap<FHCubeCoord, int32>> OutIndexMap,
		                    TSharedPtr<TMap<FHCubeCoord, FTileEnvData>> OutEnvData);

		void DoWork();

		/** 设置任务的友元类，以便在任务完成时通知 */
		FORCEINLINE TStatId GetStatId() const
		{
			RETURN_QUICK_DECLARE_CYCLE_STAT(FBuildTilesDataTask, STATGROUP_ThreadPoolAsyncTasks);
		}

		/** 任务完成回调 */
		TFunction<void()> OnComplete;

	private:
		/** 持有对 UGridMapModel 的引用，以便在任务中访问 */
		UGridMapModel* Owner;

		/** 需要处理的原始数据 */
		const TMap<FHCubeCoord, FSerializableTile>& TilesData;

		/** 输出的目标数组 */
		TSharedPtr<TArray<FTileInfo>> TargetTilesPtr;

		/** 输出的索引数组 */
		TSharedPtr<TMap<FHCubeCoord, int32>> TargetTileCoordToIndexMapPtr;;

		/** 输出的环境数据 */
		TSharedPtr<TMap<FHCubeCoord, FTileEnvData>> TargetTileEnvDataMapPtr;
	};

	/** 当前正在运行的异步任务 */
	TSharedPtr<FAsyncTask<FBuildTilesDataTask>> BuildTilesDataTask;

	/** 标记异步任务是否被取消 */
	bool BuildTilesDataTaskCancelled = false;

	/** 用于保护 Tiles 数组的线程锁 */
	FCriticalSection TilesLock;

public:
	// 辅助函数, Stable 前缀的函数 表示不依赖该格子是否存在Tile
	// ---------- 坐标转换 Start -----------------
	/**
 	* Convert coordinates from Cube space to World space.
 	* 转换Cube坐标到世界坐标, 无视是否存在格子
 	* 六边形 : @see https://www.redblobgames.com/grids/hexagons/#hex-to-pixel
 	*/
	UFUNCTION(BlueprintCallable, Category = "GridPathFinding")
	FVector StableCoordToWorld(const FHCubeCoord& InCoord);
	FHCubeCoord StableWorldToCoord(const FVector& InWorldLocation);
	FVector StableSnapToGridLocation(const FVector& InWorldLocation);

	FIntPoint StableCoordToRowColumn(const FHCubeCoord& InCoord) const;
	FHCubeCoord StableRowColumnToCoord(const FIntPoint& InRowColumn) const;

	// ---------- 坐标转换 End -----------------

	// ---------- 方向、邻居 Start -----------------

	/**
	 * 0 Top	
	 * 1 Top Right
	 * 2 Bottom Right
	 * 3 Bottom
	 * 4 Bottom Left
	 * 5 Top Left
	 * @param InCoord 
	 * @param InDirection  
	 * @return 
	 */
	const FHCubeCoord GetNeighborCoord(const FHCubeCoord& InCoord, int32 InDirection) const;

	// ---------- 方向、邻居 End -----------------

	/**
	 * Coord是否在地图范围内
	 */
	bool IsCoordInMapArea(const FHCubeCoord& InCoord) const;

	int GetTileIndex(const FHCubeCoord& InCoord) const;

	/**
	 * 不考虑该格子是否存在Tile, 保证相同配置下的遍历顺序相同
	 * @param TileFunction 
	 */
	void StableForEachMapGrid(TFunction<void(const FHCubeCoord& Coord, int32 Row, int32 Column)> TileFunction);

	/**
	 * 获取这个格子在当前地图下遍历时的Index, 与Tiles数组无关
	 * @param InCoord
	 * @return
	 */
	int32 StableGetFullMapGridIterIndex(const FHCubeCoord& InCoord);

	// ---------- Chunk 分区功能 Start------------------
	int32 StableGetChunkCount() const;

	int32 StableGetCoordChunkIndex(const FHCubeCoord& InCoord) const;

	int32 GetDistance(const FHCubeCoord& A, const FHCubeCoord& B) const;

	void StableGetChunkCoords(const int32 InChunkIndex, TArray<FHCubeCoord>& OutCoords) const;
	// ---------- Chunk 分区功能 End------------------

	static FTileInfo IntervalCreateTileInfo(const FSerializableTile& InTileData);

	// 提取获取布局的辅助函数
	static FHTileOrientation GetTileOrientation(EGridMapType InMapType, ETileOrientationFlag InOrientation);

	TArray<FHCubeCoord> GetRangeCoords(const FHCubeCoord& Center, int32 Radius) const;
private:
	FSixDirections SixDirections{};

	FHCubeCoord HexCoordRound(const FHFractional& F);
};
