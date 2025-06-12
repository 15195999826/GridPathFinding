// Fill out your copyright notice in the Description page of Project Settings.


#include "HexPathFollowingComponent.h"
#include "HexGrid.h"

DEFINE_LOG_CATEGORY(LogHexPathFollow);

void UHexPathFollowingComponent::BeginPlay()
{
	Super::BeginPlay();
}

void UHexPathFollowingComponent::SetNavMovementInterface(INavMovementInterface* NavMoveInterface)
{
	Super::SetNavMovementInterface(NavMoveInterface);
	if (MyNavData == nullptr || !NavMovementInterface.IsValid())
	{
		return;
	}

	auto MoveComOwner = NavMovementInterface->GetOwnerAsObject();
	if (MoveComOwner->Implements<UInHexActorInterface>())
	{
		InHexActorInterface = TScriptInterface<IInHexActorInterface>(MoveComOwner);
		WeakMoveComOwner = CastChecked<AActor>(MoveComOwner);
	}
	else
	{
		UE_LOG(LogHexPathFollow, Error, TEXT("[Actor %s] SetMovementComponent: Owner of MovementComponent should implement IInHexActorInterface"), *MoveComOwner->GetName());
	}

	HexMesh = CastChecked<AHexAStarNavMesh>(MyNavData);
	auto MoveComPos = NavMovementInterface->GetFeetLocation();
	BookHexCoord(HexMesh->HexGrid->WorldToHex(MoveComPos));
	EnterHexCoord(HexMesh->HexGrid->WorldToHex(MoveComPos));
}

FAIRequestID UHexPathFollowingComponent::RequestMove(const FAIMoveRequest& RequestData, FNavPathSharedPtr InPath)
{
	auto Ret = Super::RequestMove(RequestData, InPath);
	auto CurrentPathIndex = GetCurrentPathIndex();
	auto NextPathIndex = GetNextPathIndex();
	switch (Status)
	{
		case EPathFollowingStatus::Idle:
			break;
		case EPathFollowingStatus::Waiting:
			break;
		case EPathFollowingStatus::Paused:
			break;
		case EPathFollowingStatus::Moving:
			IsGetNewMovRequest = true;
			IsWaitingForEnterBookedCoord = true;
			EnterHexCoord(HexMesh->HexGrid->WorldToHex(Path->GetPathPointLocation(CurrentPathIndex).Position));
			BookHexCoord(HexMesh->HexGrid->WorldToHex(Path->GetPathPointLocation(NextPathIndex).Position));
			UE_LOG(LogHexPathFollow, Verbose, TEXT("[Actor %s] RequestMove: Moving CurrentCoord: %s, BookedCoord: %s"), *InHexActorInterface.GetObject()->GetName(), *CurrentCoord.ToString(), *BookedCoord.ToString());
			break;
	}

	return Ret;
}

void UHexPathFollowingComponent::OnSegmentFinished()
{
	
	int32 ToBookIndex = static_cast<int32>(GetNextPathIndex() + 1);
	int32 PathLength = Path->GetPathPoints().Num();
	
	if (ToBookIndex < PathLength)
	{
		BookHexCoord(HexMesh->HexGrid->WorldToHex(Path->GetPathPointLocation(ToBookIndex).Position));
		// UE_LOG(LogHexPathFollow, Warning, TEXT("[Actor %s]OnSegmentFinished: Change BookedCoord: %s"), *InHexActorInterface.GetObject()->GetName(), *BookedCoord.ToString());
		IsWaitingForEnterBookedCoord = true;
	}

	OnSegmentFinishedDelegate.Broadcast();
}

void UHexPathFollowingComponent::OnPathFinished(const FPathFollowingResult& Result)
{
	if (IsGetNewMovRequest)
	{
		IsGetNewMovRequest = false;
		if (CurrentCoord != BookedCoord)
		{
			ReleaseBookedCoord();
		}
		
		UE_LOG(LogHexPathFollow, Verbose, TEXT("[Actor %s] OnPathFinished: %s"), *InHexActorInterface.GetObject()->GetName(), *Result.ToString());
	}
	Super::OnPathFinished(Result);
}

void UHexPathFollowingComponent::OnAIDead()
{
	FHexTile& Tile = HexMesh->HexGrid->GetMutableHexTile(CurrentCoord);
	Tile.StandingActor = nullptr;
	check(!Tile.HasAIBooked);
	
	if (CurrentCoord != BookedCoord)
	{
		FHexTile& BookedTile = HexMesh->HexGrid->GetMutableHexTile(BookedCoord);
		BookedTile.HasAIBooked = false;
	}
}

void UHexPathFollowingComponent::EnterHexCoord(const FHCubeCoord& Coord)
{
	UpdateCurrentCoord(Coord);
	FHexTile& Tile = HexMesh->HexGrid->GetMutableHexTile(CurrentCoord);
	Tile.HasAIBooked = false;
	Tile.StandingActor = WeakMoveComOwner;
}

void UHexPathFollowingComponent::BookHexCoord(const FHCubeCoord& Coord)
{
	UpdateBookedCoord(Coord);
	FHexTile& Tile = HexMesh->HexGrid->GetMutableHexTile(BookedCoord);
	Tile.HasAIBooked = true;
}

void UHexPathFollowingComponent::ReleaseBookedCoord()
{
	FHexTile& BookedTile = HexMesh->HexGrid->GetMutableHexTile(BookedCoord);
	BookedTile.HasAIBooked = false;
	UpdateBookedCoord(CurrentCoord);
}

void UHexPathFollowingComponent::UpdateCurrentCoord(const FHCubeCoord& InCoord)
{
	CurrentCoord = InCoord;
	
	if (InHexActorInterface.GetObject())
	{
		InHexActorInterface->SetCurrentHexCoord(CurrentCoord);
	}
}

void UHexPathFollowingComponent::UpdateBookedCoord(const FHCubeCoord& InCoord)
{
	BookedCoord = InCoord;
	if (InHexActorInterface.GetObject())
	{
		InHexActorInterface->SetBookedHexCoord(BookedCoord);
	}
}

void UHexPathFollowingComponent::LeaveHexCoord(const FHCubeCoord& Coord)
{
	FHexTile& Tile = HexMesh->HexGrid->GetMutableHexTile(Coord);
	Tile.StandingActor = nullptr;
}


void UHexPathFollowingComponent::OnActorBump(AActor *SelfActor, AActor *OtherActor, FVector NormalImpulse, const FHitResult &Hit)
{
	Super::OnActorBump(SelfActor, OtherActor, NormalImpulse, Hit);

	// Let's see if we are moving or waiting.
	if ((SelfActor->GetClass() == OtherActor->GetClass()) && (GetStatus() != EPathFollowingStatus::Idle))
	{
		// Just broadcast the event.
		OnActorBumped.Broadcast(OtherActor->GetActorLocation());
	}
}

void UHexPathFollowingComponent::FollowPathSegment(float DeltaTime)
{
	Super::FollowPathSegment(DeltaTime);

	/**
	 * FollowPathSegment is the main UE4 Path Follow tick function, and so when you want to add completely 
	 * custom coding you can use this function as your starting point to adjust normal UE4 path behavior!
	 *
	 * Let me show you a simple example with some debug drawings.
	 */

	// Todo: 记录当前线段进度，50%是，认为已经占据了下一个棋格
	if (Path && Status == EPathFollowingStatus::Moving && IsWaitingForEnterBookedCoord)
	{
		auto StartPos = Path->GetPathPointLocation(GetCurrentPathIndex()).Position;
		auto EndPos = Path->GetPathPointLocation(GetNextPathIndex()).Position;
		auto CurrentPos = NavMovementInterface->GetFeetLocation();
		auto Progress = FVector::Distance(CurrentPos, StartPos) / FVector::Distance(StartPos, EndPos);
		if (Progress >= 0.5f)
		{
			IsWaitingForEnterBookedCoord = false;
			LeaveHexCoord(CurrentCoord);
			EnterHexCoord(BookedCoord);
			// UE_LOG(LogHexPathFollow, Warning, TEXT("[Actor %s] FollowPathSegment: Change CurrentCoord: %s"), *InHexActorInterface.GetObject()->GetName(), *CurrentCoord.ToString());
		}
	}
	
	if (Path && DrawDebug)
	{
		// Just draw the current path
		Path->DebugDraw(MyNavData, FColor::White, nullptr, false, -1.f);
		
		// Draw the start point of the current path segment we are traveling.
		FNavPathPoint CurrentPathPoint{};
		FNavigationPath::GetPathPoint(&Path->AsShared().Get(), GetCurrentPathIndex(), CurrentPathPoint);
		DrawDebugLine(GetWorld(), CurrentPathPoint.Location, CurrentPathPoint.Location + FVector(0.f, 0.f, 200.f), FColor::Blue);
		DrawDebugSphere(GetWorld(), CurrentPathPoint.Location + FVector(0.f, 0.f, 200.f), 25.f, 16, FColor::Blue);

		// Draw the end point of the current path segment we are traveling.
		FNavPathPoint NextPathPoint{};
		FNavigationPath::GetPathPoint(&Path->AsShared().Get(), GetNextPathIndex(), NextPathPoint);
		DrawDebugLine(GetWorld(), NextPathPoint.Location, NextPathPoint.Location + FVector(0.f, 0.f, 200.f), FColor::Green);
		DrawDebugSphere(GetWorld(), NextPathPoint.Location + FVector(0.f, 0.f, 200.f), 25.f, 16, FColor::Green);
	}
}
