// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "GridMapModel.h"
#include "NavMesh/RecastNavMesh.h"
#include "Types/MapConfig.h"
#include "GridPathFindingNavMesh.generated.h"

class UGridMapModel;
DECLARE_LOG_CATEGORY_EXTERN(LogGridPathFinding_NavMesh, Log, All);

DECLARE_CYCLE_STAT(TEXT("Grid A* Pathfinding"), STAT_Navigation_GASPathfinding, STATGROUP_Navigation);

UCLASS()
class GRIDPATHFINDING_API AGridPathFindingNavMesh : public ARecastNavMesh
{
	GENERATED_BODY()

public:
	/**
	 * the function is static for a reason, (wiki copy-paste->) 
	 * comments in the code explain it's for performance reasons: Epic are concerned 
	 * that if a lot of agents call the pathfinder in the same frame the virtual call overhead will accumulate and take too long, 
	 * so instead the function is declared static and stored in the FindPathImplementation function pointer.
	 * Which means you need to manually set the function pointer in your new navigation class constructor 
	 * or in some other function like we do here in SetHexGrid().
	 */
	static FPathFindingResult FindPath(const FNavAgentProperties &AgentProperties, const FPathFindingQuery &Query);
	
	/* Set a pointer to a grid map, it can be nullptr */
	void SetMapModel(UGridMapModel* InMapModel);

	//////////////////////////////////////////////////////////////////////////
	/**
	 * Generic graph A* implementation
	 * TGraph holds graph representation.Needs to implement functions :
	*/

	/* Type used as identification of nodes in the graph */
	typedef int32 FNodeRef;

	/* Returns number of neighbors that the graph node identified with NodeRef has */
	int32 GetNeighbourCount(FNodeRef NodeRef) const;

	/* Returns whether given node identification is correct */
	bool IsValidRef(FNodeRef NodeRef) const;

	/* Returns neighbor ref */
	FNodeRef GetNeighbour(const FNodeRef NodeRef, const int32 NeiIndex) const;
	//////////////////////////////////////////////////////////////////////////

	void SetToStatic();

	/* Just a pointer to a grid map model */
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "HexAStar|NavMesh")
	TWeakObjectPtr<UGridMapModel> WeakMapModel;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category = "HexAStar|NavMesh")
	float PathPointZOffset{0.f};
};


/**
 * TQueryFilter (FindPath's parameter) filter class is what decides which graph edges can be used and at what cost.
 */
struct FGridPathFilter
{
	FGridPathFilter(const AGridPathFindingNavMesh &InNavMeshRef) : NavMeshRef(InNavMeshRef)
	{
		// 构造时缓存地图参数
		InitializeDistanceCache();
	}

	int StartIdx{};
	int EndIdx{};
	int Identifier = INDEX_NONE;

	/**
	 * Used as GetHeuristicCost's multiplier
	 * 作用: 作为启发式代价的乘数，调节算法行为
     * 值为1: 标准A算法
     * 值>1: 更倾向于直奔目标（可能不是最优路径）
     * 值<1: 更倾向于探索（接近Dijkstra算法）
	 */
	FVector::FReal GetHeuristicScale() const;

	/**
	 * Estimate of cost from StartNodeRef to EndNodeRef
	 * GetHeuristicCost (启发式代价) - 作用: 估算从当前节点到目标节点的理论最小代价 - 特点: 这是A算法的核心，必须满足"可接受性"（不能高估实际代价）
	 * 建议: 应该基于六边形网格的曼哈顿距离计算
	 */
	FVector::FReal GetHeuristicCost(const int32 StartNodeRef, const int32 EndNodeRef) const;

	/**
	 * Real cost of traveling from StartNodeRef directly to EndNodeRef
	 * 作用: 计算从一个相邻节点移动到另一个节点的实际代价
     * 特点: 必须>0，否则算法会停止
     * 用途: 这是实现控制区机制的最佳位置！
	 */
	FVector::FReal GetTraversalCost(const int32 StartNodeRef, const int32 EndNodeRef) const;

	/**
	 * Whether traversing given edge is allowed
	 */
	bool IsTraversalAllowed(const int32 NodeA, const int32 NodeB) const;

	/**
	 * Whether to accept solutions that do not reach the goal
	 */
	bool WantsPartialSolution() const;

protected:

	/**
	 * A reference to our NavMesh
	 */
	const AGridPathFindingNavMesh &NavMeshRef;

private:
	// 寻路缓存参数 - 构造时初始化一次，整个寻路过程复用
	int32 CachedMapRows = 0;
	int32 CachedMapColumns = 0;
	int32 CachedMapRowsHalf = 0;
	int32 CachedMapColumnsHalf = 0;
	bool bCachedIsFlatOrientation = true;

	void InitializeDistanceCache()
	{
		const auto& MapConfig = NavMeshRef.WeakMapModel->GetMapConfig();
		CachedMapRows = MapConfig.MapSize.X;
		CachedMapColumns = MapConfig.MapSize.Y;
		CachedMapRowsHalf = CachedMapRows / 2;
		CachedMapColumnsHalf = CachedMapColumns / 2;
		bCachedIsFlatOrientation = (MapConfig.TileOrientation == ETileOrientationFlag::FLAT);
	}

	// 极速距离计算 - 零内存访问开销
	// 仅适用于六边形网格（FLAT或POINTY），且地图通过RowColumn方式遍历
	FORCEINLINE int32 GetDistanceByIndexUltraFast(const int32 A, const int32 B) const
	{
		if (A == B) return 0;

		int32 qA, rA, qB, rB;

		if (bCachedIsFlatOrientation)
		{
			// 使用缓存值，全部在栈上，CPU缓存友好
			const int32 columnA = A / CachedMapRows;
			const int32 rowA = A % CachedMapRows;
			qA = columnA - CachedMapColumnsHalf;
			rA = rowA - (columnA - (columnA & 1)) / 2 - CachedMapRowsHalf;

			const int32 columnB = B / CachedMapRows;
			const int32 rowB = B % CachedMapRows;
			qB = columnB - CachedMapColumnsHalf;
			rB = rowB - (columnB - (columnB & 1)) / 2 - CachedMapRowsHalf;
		}
		else // POINTY
		{
			const int32 rowA = A / CachedMapColumns;
			const int32 columnA = A % CachedMapColumns;
			qA = columnA - (rowA - (rowA & 1)) / 2 - CachedMapColumnsHalf;
			rA = rowA - CachedMapRowsHalf;

			const int32 rowB = B / CachedMapColumns;
			const int32 columnB = B % CachedMapColumns;
			qB = columnB - (rowB - (rowB & 1)) / 2 - CachedMapColumnsHalf;
			rB = rowB - CachedMapRowsHalf;
		}

		return (FMath::Abs(qA - qB) + FMath::Abs(qA + rA - qB - rB) + FMath::Abs(rA - rB)) / 2;
	}
};


// We inherit this struct because we need a custom GetCost/GetLength
struct FGridNavMeshPath : public FNavMeshPath
{
	FORCEINLINE
	virtual FVector::FReal GetCostFromIndex(int32 PathPointIndex) const override
	{
		return CurrentPathCost;
	}

	FORCEINLINE
	virtual FVector::FReal GetLengthFromPosition(FVector SegmentStart, uint32 NextPathPointIndex) const override
	{
		// We exclude the starting point so -1	
		return PathPoints.Num() - 1;
	}

	float CurrentPathCost{ 0 };
};
