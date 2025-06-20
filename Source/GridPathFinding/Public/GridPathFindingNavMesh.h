﻿// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "NavMesh/RecastNavMesh.h"
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
	FGridPathFilter(const AGridPathFindingNavMesh &InNavMeshRef) : NavMeshRef(InNavMeshRef) {}

	int StartIdx{};
	int EndIdx{};

	/**
	 * Used as GetHeuristicCost's multiplier
	 */
	FVector::FReal GetHeuristicScale() const;

	/**
	 * Estimate of cost from StartNodeRef to EndNodeRef
	 */
	FVector::FReal GetHeuristicCost(const int32 StartNodeRef, const int32 EndNodeRef) const;

	/**
	 * Real cost of traveling from StartNodeRef directly to EndNodeRef
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
