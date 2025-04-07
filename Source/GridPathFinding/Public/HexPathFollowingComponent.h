// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "CoreMinimal.h"
#include "HexAStarNavMesh.h"
#include "HGTypes.h"
#include "InHexActorInterface.h"
#include "Navigation/PathFollowingComponent.h"
#include "HexPathFollowingComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnActorBumpDelegate, const FVector&, BumpLocation);
GRIDPATHFINDING_API DECLARE_LOG_CATEGORY_EXTERN(LogHexPathFollow, Log, All);
/**
 * 
 */
UCLASS()
class GRIDPATHFINDING_API UHexPathFollowingComponent : public UPathFollowingComponent
{
	GENERATED_BODY()
	/** called when moving agent collides with another actor */
	virtual void OnActorBump(AActor *SelfActor, AActor *OtherActor, FVector NormalImpulse, const FHitResult &Hit) override;

public:
	virtual void BeginPlay() override;

	virtual void SetNavMovementInterface(INavMovementInterface* NavMoveInterface) override;
	virtual FAIRequestID RequestMove(const FAIMoveRequest& RequestData, FNavPathSharedPtr InPath) override;
	
protected:
	virtual void OnSegmentFinished() override;
	/** follow current path segment */
	virtual void FollowPathSegment(float DeltaTime) override;

	virtual void OnPathFinished(const FPathFollowingResult& Result) override;

public:
	FSimpleMulticastDelegate OnSegmentFinishedDelegate;
	/**
	 * Executed if a "Bump" happen, we bind this delegate on the activation of our Behavior Tree Service BTS_BindBump
	 */
	UPROPERTY(BlueprintAssignable, Category = "HexAStar|PathFollowingComponent")
	FOnActorBumpDelegate OnActorBumped;
	UPROPERTY(BlueprintReadOnly, Category = "HexAStar|PathFollowingComponent")
	TWeakObjectPtr<AHexAStarNavMesh> HexMesh;
	/**
	 * Toggle our debug drawing.
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "HexAStar|PathFollowingComponent")
	bool DrawDebug{};

protected:
	bool IsGetNewMovRequest = false;
	bool IsWaitingForEnterBookedCoord = false;
	FHCubeCoord CurrentCoord; // 当前所在的格子
	FHCubeCoord BookedCoord; // 将要前往的下一个格子

	TScriptInterface<IInHexActorInterface> InHexActorInterface;
	UPROPERTY()
	TWeakObjectPtr<AActor> WeakMoveComOwner;
public:
	void OnAIDead();

protected:
	void EnterHexCoord(const FHCubeCoord& Coord);
	void BookHexCoord(const FHCubeCoord& Coord);
	
	void LeaveHexCoord(const FHCubeCoord& Coord);
	void ReleaseBookedCoord();

private:
	void UpdateCurrentCoord(const FHCubeCoord& InCoord);
	void UpdateBookedCoord(const FHCubeCoord& InCoord);
};
