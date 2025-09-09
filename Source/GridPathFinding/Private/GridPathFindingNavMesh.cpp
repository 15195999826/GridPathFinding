// Fill out your copyright notice in the Description page of Project Settings.


#include "GridPathFindingNavMesh.h"

#include <chrono>

#include "GraphAStar.h"
#include "GridMapModel.h"
#include "GridPathFindingIdentifier.h"
#include "Types/HCubeCoord.h"


DEFINE_LOG_CATEGORY(LogGridPathFinding_NavMesh)

FPathFindingResult AGridPathFindingNavMesh::FindPath(const FNavAgentProperties& AgentProperties,
                                                     const FPathFindingQuery& Query)
{
	// =================================================================================================
	// The first part is the same of RecastNavMesh::FindPath implementation, the only difference is
	// the cast of ANavigationData to our class.
	SCOPE_CYCLE_COUNTER(STAT_Navigation_GASPathfinding);
	CSV_SCOPED_TIMING_STAT_EXCLUSIVE(Pathfinding);
	// 获取开始时间
	auto start = std::chrono::high_resolution_clock::now();
	// Because we are in a static function we don't have a "this" pointer so we can't access to class member variables like HexGrid
	// but luckily the FPathFindingQuery contain a pointer to the ANavigationData object.
	const ANavigationData *Self = Query.NavData.Get();
	check(Cast<const AGridPathFindingNavMesh>(Self));

	// Now we can cast to our class, this will allow us to access the member variables (and functions).
	// NOTE: remember, our AGraphAStarNavMesh inherit from ARecastNavMesh that inherit from ANavigationData so we can do the cast.
	const AGridPathFindingNavMesh *GraphAStarNavMesh{ Cast<const AGridPathFindingNavMesh>(Self) };
	
	if (Self == NULL)
	{
		return ENavigationQueryResult::Error;
	}


	if (!GraphAStarNavMesh->WeakMapModel.IsValid())
	{
		UE_LOG(LogGridPathFinding_NavMesh, Error, TEXT("FindPath: WeakMapModel is not valid!"));
		return ENavigationQueryResult::Error;
	}
	
	// This struct contains the result of our search and the Path that the AI will follow
	FPathFindingResult Result(ENavigationQueryResult::Error);

	FNavigationPath *NavPath = Query.PathInstanceToFill.Get();
	FGridNavMeshPath *NavMeshPath = NavPath ? NavPath->CastPath<FGridNavMeshPath>() : nullptr;

	if (NavMeshPath)
	{
		Result.Path = Query.PathInstanceToFill;
		NavMeshPath->ResetForRepath();
	}
	else
	{
		Result.Path = Self->CreatePathInstance<FGridNavMeshPath>(Query);
		NavPath = Result.Path.Get();
		NavMeshPath = NavPath ? NavPath->CastPath<FGridNavMeshPath>() : nullptr;
	}

	const FNavigationQueryFilter *NavFilter = Query.QueryFilter.Get();
	if (NavMeshPath && NavFilter)
	{
		NavMeshPath->ApplyFlags(Query.NavDataFlags);

		const FVector AdjustedEndLocation = NavFilter->GetAdjustedEndLocation(Query.EndLocation);
		if ((Query.StartLocation - AdjustedEndLocation).IsNearlyZero() == true)
		{
			Result.Path->GetPathPoints().Reset();
			Result.Path->GetPathPoints().Add(FNavPathPoint(AdjustedEndLocation));
			Result.Result = ENavigationQueryResult::Success;
		}
		// ============ END OF SAME CODE AS RECASTNAVMESH ========================================================

		else
		{
			// ====================== BEGIN OF OUR CODE ===========================================================

			// Reset the PathPoints array
			Result.Path->GetPathPoints().Reset();

			// The pathfinder need a starting and ending point, so we create two temporary
			// cube coordinates from the Query start and ending location
			// 中文：路径搜索需要一个起点和一个终点，所以我们从查询的起点和终点位置创建两个临时的立方体坐标
			const FHCubeCoord& StartCCoord{ GraphAStarNavMesh->WeakMapModel->StableWorldToCoord(Query.StartLocation) };
			const FHCubeCoord& EndCCoord{ GraphAStarNavMesh->WeakMapModel->StableWorldToCoord(Query.EndLocation) };
			
			// and then we search in the HexGrid CubeCoordinates array for the index of items 
			// equals to our temp coordinates.
			// 中文：然后我们在HexGrid CubeCoordinates数组中搜索与我们的临时坐标相等的项目的索引。
			const int32 StartIdx{ GraphAStarNavMesh->WeakMapModel->StableGetFullMapGridIterIndex(StartCCoord) };
			const int32 EndIdx{ GraphAStarNavMesh->WeakMapModel->StableGetFullMapGridIterIndex(EndCCoord)};
			// UE_LOG(LogTemp, Warning, TEXT("StartCCoord: %s, StartIdx: %d, Query.StartLocation: %s, EndCCoord: %s, EndIdx: %d, Query.EndLocation: %s"), 
			// 	*StartCCoord.ToString(), StartIdx, *Query.StartLocation.ToString(), 
			// 	*EndCCoord.ToString(), EndIdx, *Query.EndLocation.ToString());
			// We need the index because the FGraphAStar work with indexes!
			// 中文：我们需要索引，因为FGraphAStar使用索引！

			// Here we will store the path generated from the pathfinder
			// 中文：这里我们将存储从路径搜索器生成的路径
			TArray<int32> PathIndices;
			
			// Initialization of the pathfinder, as you can see we pass our GraphAStarNavMesh as parameter,
			// so internally it can use the functions we implemented.
			// 中文：路径搜索器的初始化，正如你所看到的，我们将GraphAStarNavMesh作为参数传递，
			FGraphAStar Pathfinder(*GraphAStarNavMesh);

			// and run the A* algorithm, the FGraphAStar::FindPath function want a starting index, an ending index,
			// the FGridPathFilter which want our GraphAStarNavMesh as parameter and a reference to the array where
			// all the indices of our path will be stored
			// 中文：并运行A*算法，FGraphAStar::FindPath函数需要一个起始索引，一个结束索引，
			// FGridPathFilter需要我们的GraphAStarNavMesh作为参数，并引用存储我们路径所有索引的数组
			// Todo: 寻路的时候，
			auto Filter = FGridPathFilter(*GraphAStarNavMesh);

			// Todo: 增加一个接口， 然后实现身份标识的功能
			if (Query.Owner.IsValid())
			{
				if (auto IdentifierInterface = Cast<IGridPathFindingIdentifier>(Query.Owner.Get()))
				{
					// 如果有身份标识接口，设置过滤器的身份标识
					Filter.Identifier = IdentifierInterface->GetGridPathFindingIdentifier();
				}
			}
			
			Filter.StartIdx = StartIdx;
			Filter.EndIdx = EndIdx;
			// UE_LOG(LogHexAStar_NavMesh, Warning, TEXT("EndCCoord: %s, EndIdx: %d, Query.EndLocation: %s"), *EndCCoord.ToString(), EndIdx, *Query.EndLocation.ToString());
			EGraphAStarResult AStarResult{ Pathfinder.FindPath(StartIdx, EndIdx, Filter, PathIndices) };

			// The FGraphAStar::FindPath return a EGraphAStarResult enum, we need to assign the right
			// value to the FPathFindingResult (that is returned by AGraphAStarNavMesh::FindPath) based on this.
			// In the first three cases are simple and process the three "bad" results
			// 中文：FGraphAStar::FindPath返回一个EGraphAStarResult枚举，我们需要根据这个枚举为FPathFindingResult（由AGraphAStarNavMesh::FindPath返回）分配正确的值。
			// 在前三种情况下，根据这三种“坏”结果简单处理
			switch (AStarResult)
			{
				case GoalUnreachable:
					
					Result.Result = ENavigationQueryResult::Invalid;
					break;

				case InfiniteLoop:
					Result.Result = ENavigationQueryResult::Error;
					break;

				case SearchFail:
					Result.Result = ENavigationQueryResult::Fail;
					break;

				// The search was successful so let's process the computed path, remember, right now
				// we only have an array of indexes so we have to compute the path locations
				// from these indexes and pass them to the PathPoints array of the Path
				// that the AI will follow.
				// 中文：搜索成功，所以让我们处理计算出的路径，记住，现在我们只有一个索引数组，所以我们必须从这些索引计算路径位置，并将它们传递给AI将要遵循的路径的PathPoints数组。
				case SearchSuccess:

					// Search succeeded
					Result.Result = ENavigationQueryResult::Success;
					
					// PathIndices array computed by FGraphAStar will not contain the starting point, so
					// we need to add it manually to the Path::PathPoints array
					// 中文：FGraphAStar计算的PathIndices数组不包含起点，所以我们需要手动将其添加到Path::PathPoints数组中
					Result.Path->GetPathPoints().Add(FNavPathPoint(Query.StartLocation));

					// Let's traverse the PathIndices array and build the FNavPathPoints we 
					// need to add to the Path.
					// 中文：让我们遍历PathIndices数组并构建我们需要添加到路径的FNavPathPoints。
					for (const int32 &PathIndex : PathIndices)
					{
						// Get a temporary Cube Coordinate from our HexGrid
						// 中文：从我们的HexGrid获取一个临时立方体坐标
						const FHCubeCoord GridCoord{ GraphAStarNavMesh->WeakMapModel->StableGetCoordByIndex(PathIndex) };

						// Create a temporary FNavPathPoint
						FNavPathPoint PathPoint{};

						// Because we can create HexGrid with only Cube Coordinates and no tiles
						// we look if the current index we are using is a valid index for the GridTiles array
						// 中文：因为我们可以创建只有立方体坐标而没有瓦片的HexGrid
						// 我们查看我们正在使用的当前索引是否是GridTiles数组的有效索引

						// Todo: 这里可以用来做Z轴偏移
						// if (GraphAStarNavMesh->WeakMapModel->GridTiles.IsValidIndex(PathIndex))
						// {
						// 	// If the index is valid (so we have a HexGrid with tiles) we compute the Location
						// 	// of the PathPoint, we use the World Space coordinates of the current Cube Coordinate
						// 	// as a base location then we add an offset to the Z.
						// 	// How to compute the Z axis of the path is up to you, this is only an example!
						// 	// 中文：如果索引有效（因此我们有一个带有瓦片的HexGrid）我们计算PathPoint的位置，
						// 	// 我们使用当前立方体坐标的世界空间坐标作为基本位置，并在Z轴上添加一个偏移量。
						// 	// 如何计算路径的Z轴取决于您，这只是一个示例！
						// 	PathPoint.Location = GraphAStarNavMesh->WeakMapModel->StableCoordToWorld(GridCoord) +
						// 		FVector(0.f, 0.f, GraphAStarNavMesh->WeakMapModel->GridTiles[PathIndex].Cost +
						// 		GraphAStarNavMesh->PathPointZOffset);
						// }
						// else
						// {
						// 	// If the current PathIndex isn't a valid index for the GridTiles array
						// 	// (so we assume our HexGrid is only a "logical" grid with only cube coordinates and no tiles)
						// 	// we simply transform the coordinates from cube space to world space and pass it to the PathPoint
						// 	// 中文：如果当前PathIndex不是GridTiles数组的有效索引（因此我们假设我们的HexGrid只是一个带有立方体坐标而没有瓦片的“逻辑”网格）
						// 	// 我们只需将坐标从立方体空间转换为世界空间并将其传递给PathPoint
						// 	
						// }

						PathPoint.Location = GraphAStarNavMesh->WeakMapModel->StableCoordToWorld(GridCoord);

						// We finally add the computed PathPoint to the Path::PathPoints array
						// 中文：我们最终将计算出的PathPoint添加到Path::PathPoints数组
						Result.Path->GetPathPoints().Add(FNavPathPoint(PathPoint));
					}

					// We finished to create the Path so mark it as Ready.
					// 中文：我们完成了创建路径，因此将其标记为Ready。
					Result.Path->MarkReady();
					break;
			}
			// =========================== END OF OUR CODE ============================================================
		}
	}
	
	// // 获取结束时间
	auto end = std::chrono::high_resolution_clock::now();
	//
	// // 计算并打印执行时间, 0.001毫秒级别寻路，稍长一点也只有0.05毫秒
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	UE_LOG(LogTemp, Warning, TEXT("FindPath execution time: %lld microseconds"), duration.count());


	return Result;
}

void AGridPathFindingNavMesh::SetMapModel(UGridMapModel* InMapModel)
{
	if (InMapModel)
	{
		// If the pointer is valid we will use our implementation of the FindPath function
		WeakMapModel = InMapModel;
		FindPathImplementation = FindPath;
	}
	else
	{
		// If the pointer is not valid we will fallback to the default RecastNavMesh implementation
		// of the FindPath function (the standard navigation behavior)
		// You can also use FindPathImplementation = ARecastNavMesh::FindPath;
		// but i start from the assumption that we are inheriting from ARecastNavMesh
		WeakMapModel = nullptr;
		FindPathImplementation = ARecastNavMesh::FindPath;
	}
}

//////////////////////////////////////////////////////////////////////////
// FGraphAStar: TGraph
// Functions implementation for our FGraphAStar struct
int32 AGridPathFindingNavMesh::GetNeighbourCount(FNodeRef NodeRef) const
{
	return 6;
}

bool AGridPathFindingNavMesh::IsValidRef(FNodeRef NodeRef) const
{
	// 检查Index是否在有效的地图范围内
	return NodeRef >= 0 && NodeRef <= WeakMapModel->GetMaxValidIndex(); 
}

AGridPathFindingNavMesh::FNodeRef AGridPathFindingNavMesh::GetNeighbour(const FNodeRef NodeRef,
	const int32 NeiIndex) const
{
	return WeakMapModel->GetNeighborIndex(NodeRef, NeiIndex);
}

void AGridPathFindingNavMesh::SetToStatic()
{
	RuntimeGeneration = ERuntimeGenerationType::Static;
}

//////////////////////////////////////////////////////////////////////////

//==== FGridPathFilter functions implementation ===
// In these functions we do not check if the HexGrid is valid because it must be!
// Remember, if the HexGrid is a nullptr we will never use this code
// but we fallback to the RecastNavMesh implementation of it.

FVector::FReal FGridPathFilter::GetHeuristicScale() const
{
	// For the sake of simplicity we just return 1.f
	/**
	 * | 比例值   | 算法行为        | 搜索特点      | 性能  | 路径质量  |
	 * |---------|---------------|--------------|------|---------|
	 * | 0.0     | 变成Dijkstra   | 盲目全方向搜索 | 慢    | 绝对最优  |
	 * | 1.0     | 标准A*         | 平衡搜索      | 中等  | 最优     |
	 * | 1.5-3.0 | 启发式偏重      | 更倾向目标方向 | 快    | 通常良好  |
	 * | >5.0    | 贪婪搜索       | 几乎直奔目标   | 很快  | 可能不最优 |
	 */
	return 1.0f;
}

FVector::FReal FGridPathFilter::GetHeuristicCost(const int32 StartNodeRef, const int32 EndNodeRef) const
{
	/**
	 *	 * 起点S ────┐         ┌──── 终点G
	 *           │         │
	 *           └── 节点A ─┘
	 *           
	 * 没有启发式函数：随机搜索所有方向
	 * 有启发式函数：优先搜索朝向G的方向
	 */
	return GetDistanceByIndexUltraFast(StartNodeRef, EndNodeRef);
}

// 内置函数寻路中调用GetTraversalCost时，两个格子总是邻居
// GetHeuristicCost函数，两个格子不一定是邻居
FVector::FReal FGridPathFilter::GetTraversalCost(const int32 StartNodeRef, const int32 EndNodeRef) const
{
	// If EndNodeRef is a valid index of the GridTiles array we return the tile cost, 
	// if not we return 1 because the traversal cost need to be > 0 or the FGraphAStar will stop the execution
	// look at GraphAStar.h line 244: ensure(NewTraversalCost > 0);
	// 中文：如果EndNodeRef是GridTiles数组的有效索引，我们返回瓦片成本，否则我们返回1，因为遍历成本需要> 0，否则FGraphAStar将停止执行
	// 请查看GraphAStar.h第244行：ensure(NewTraversalCost > 0);

	// Todo: 暂时未考虑格子的Cost， 全部格子Cost相同
	// auto StartTileHeight = NavMeshRef.WeakMapModel->GetTileHeight(StartNodeRef);
	// auto EndTileHeight = NavMeshRef.WeakMapModel->GetTileHeight(EndNodeRef);
	// auto HeightPenalty = NavMeshRef.WeakMapModel->GetHeightCost(EndTileHeight, StartTileHeight);
	// // 额外惩罚， 由项目自行定义
	// auto ExtraPenalty = NavMeshRef.WeakMapModel->GetExtraPenalty(Identifier, StartNodeRef, EndNodeRef);
	
	// 1.f 为基础消耗， 防止0的出现
	// return 1.f + HeightPenalty + ExtraPenalty;
	return 1.f + NavMeshRef.WeakMapModel->GetTraversalCost(Identifier, StartNodeRef, EndNodeRef);
}

bool FGridPathFilter::IsTraversalAllowed(const int32 NodeA, const int32 NodeB) const
{
	// If NodeB is a valid index of the GridTiles array we return bIsBlocking, 
	// if not we assume we can traverse so we return true.
	// Here you can make a more complex operation like use a line trace to see
	// there is some obstacles (like an enemy), in our example we just use a simple implementation
	// 中文：如果NodeB是GridTiles数组的有效索引，我们返回bIsBlocking，否则我们假设我们可以遍历，所以我们返回true。
	// 在这里，您可以执行更复杂的操作，例如使用线跟踪来查看是否有障碍物（例如敌人），在我们的示例中，我们只是使用简单的实现
	// Todo: 检查该格子是否存在Tile信息， 判定是否可以通行
	return NavMeshRef.WeakMapModel->CanTravelTo(NodeA, NodeB);
}

bool FGridPathFilter::WantsPartialSolution() const
{
	// Just return true
	// Todo: 暂时不知道这个函数有啥用
	return true;
}
//==== END OF FGridPathFilter functions implementation ====