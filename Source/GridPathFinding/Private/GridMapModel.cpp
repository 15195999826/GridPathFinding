// Fill out your copyright notice in the Description page of Project Settings.


#include "GridMapModel.h"

#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "HGTypes.h"
#include "WaitGroupManager.h"

UGridMapModel::UGridMapModel()
{
}

void UGridMapModel::BeginDestroy()
{
	if (BuildTilesDataTask.IsValid())
	{
		BuildTilesDataTaskCancelled = true;
		if (!BuildTilesDataTask->IsDone())
		{
			BuildTilesDataTask->GetTask().OnComplete = nullptr;
			if (!BuildTilesDataTask->Cancel())
			{
				UE_LOG(LogGridPathFinding, Warning, TEXT("BuildTilesDataTask cancel failed"));
				BuildTilesDataTask->EnsureCompletion(false);
			}
		}
		BuildTilesDataTask.Reset();
	}
	UObject::BeginDestroy();
}

void UGridMapModel::BuildTilesData(const FGridMapConfig& InMapConfig,
                                   const TMap<FHCubeCoord, FSerializableTile>& InTilesData)
{
	IsBuilding = true;
	MapConfig = InMapConfig;
	SixDirections.DirVectors.Empty();
	if (MapConfig.MapType == EGridMapType::RECTANGLE_SIX_DIRECTION || MapConfig.MapType == EGridMapType::HEX_STANDARD)
	{
		auto SimulateCoord = FHCubeCoord(0, 0, 0);
		auto SimulateCoordPosition = StableCoordToWorld(SimulateCoord);
		for (int32 i = 0; i < 6; ++i)
		{
			const auto& Direction = SixDirections.Directions[i];
			auto DirectionPosition = StableCoordToWorld(SimulateCoord + Direction);
			SixDirections.DirVectors.Add(DirectionPosition - SimulateCoordPosition);
		}
	}

	// 构造寻路缓存数据
	BuildPathFindingCache();

	// 终止上一个可能正在运行的异步任务
	if (BuildTilesDataTask.IsValid())
	{
		// 设置取消标志
		BuildTilesDataTaskCancelled = true;

		OnTilesDataBuildCancel.Broadcast();

		// 确保任务被取消
		if (!BuildTilesDataTask->IsDone())
		{
			// 异步等待任务完成
			BuildTilesDataTask->EnsureCompletion();
		}
	}

	// 重置取消标志
	BuildTilesDataTaskCancelled = false;

	// 清理现有数据
	Tiles.Empty();

	// 创建临时数组，用于异步填充
	auto TempTilesPtr = MakeShared<TArray<FTileInfo>>();
	auto TempEnvDataPtr = MakeShared<TMap<FHCubeCoord, FTileEnvData>>();
	TempTilesPtr->Reserve(GetMaxValidIndex());
	TempEnvDataPtr->Reserve(GetMaxValidIndex());

	// TempEnvDataPtr填充格子大小的Coord数量
	StableForEachMapGrid([this, TempEnvDataPtr, TempTilesPtr](const FHCubeCoord& Coord, int32 Row, int32 Column)
	{
		TempTilesPtr->Add(FTileInfo(Coord));
		TempEnvDataPtr->Add(Coord, FTileEnvData());
	});
	
	// 创建异步任务
	BuildTilesDataTask = MakeShared<FAsyncTask<FBuildTilesDataTask>>(this, InTilesData, TempTilesPtr, TempEnvDataPtr);

	// 设置任务完成的回调，将结果复制到 Tiles 数组
	BuildTilesDataTask->GetTask().OnComplete = [this, TempTilesPtr, TempEnvDataPtr]
	{
		if (!BuildTilesDataTaskCancelled)
		{
			// 将临时数组中的数据复制到 Tiles 数组
			FScopeLock Lock(&TilesLock);
			Tiles = MoveTemp(*TempTilesPtr);
			TileEnvDataMap = MoveTemp(*TempEnvDataPtr);
			UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData completed successfully with %d tiles, EnvData Num: %d"),
			       Tiles.Num(), TileEnvDataMap.Num());
		}
		else
		{
			UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task was cancelled"));
		}

		// 触发地图数据更新完成事件
		IsBuilding = false;
		OnTilesDataBuildComplete.Broadcast();
	};

	// TFuture<void> Task = Async(EAsyncExecution::ThreadPool, [this]() {
	// 	// 这里可以执行一些初始化操作
	// 	// 例如，设置任务的初始状态等
	// });
	// 启动异步任务
	BuildTilesDataTask->StartBackgroundTask();
}

void UGridMapModel::UpdateTileEnv(const FSerializableTile& InTileData, bool bNotify)
{
	FTileEnvData OldTileEnv = TileEnvDataMap[InTileData.Coord];
	TileEnvDataMap[InTileData.Coord] = InTileData.TileEnvData;
	if (bNotify)
	{
		OnTileEnvModify.Broadcast(InTileData.Coord, OldTileEnv, TileEnvDataMap[InTileData.Coord]);
	}
}

void UGridMapModel::ModifyTileTokens(ETileTokenModifyType ModifyType, const FHCubeCoord& InCoord,
	const int32 TokenIndex, const FSerializableTokenData& InTokenData, bool bNotify)
{
}

ATokenActor* UGridMapModel::GetTokenByIndex(const FHCubeCoord& InCoord, int32 InTokenIndex, bool bErrorIfNotExist)
{
	if (Coord2TokensMap.Contains(InCoord))
	{
		const auto& Tokens = Coord2TokensMap[InCoord];
		if (InTokenIndex >= 0 && InTokenIndex < Tokens.Num())
		{
			return Tokens[InTokenIndex];
		}

		UE_LOG(LogGridPathFinding, Error, TEXT("[GetTokenByIndex]: Invalid token index %d for coord %s"),
		       InTokenIndex, *InCoord.ToString());
		return nullptr;

	}

	if (bErrorIfNotExist)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[GetTokenByIndex]: No tokens found for coord %s"), *InCoord.ToString());
	}
	
	return nullptr;
}

void UGridMapModel::AppendToken(const FHCubeCoord& InCoord, ATokenActor* InTokenActor)
{
	if (InCoord == FHCubeCoord::Invalid || InTokenActor == nullptr)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[AppendToken] Invalid coord or token actor"));
		return;
	}

	if (!Coord2TokensMap.Contains(InCoord))
	{
		Coord2TokensMap.Add(InCoord, TArray<TObjectPtr<ATokenActor>>());
	}

	Coord2TokensMap[InCoord].Add(InTokenActor);
}

void UGridMapModel::RemoveToken(const FHCubeCoord& InCoord, ATokenActor* InTokenActor)
{
	if (InCoord == FHCubeCoord::Invalid || InTokenActor == nullptr)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[RemoveToken] Invalid coord or token actor"));
		return;
	}

	if (Coord2TokensMap.Contains(InCoord))
	{
		int32 RemovedNum = Coord2TokensMap[InCoord].Remove(InTokenActor);

		if (RemovedNum > 0)
		{
			// 如果移除后该坐标下没有Token了，则清理该坐标的记录
			if (Coord2TokensMap[InCoord].Num() == 0)
			{
				Coord2TokensMap.Remove(InCoord);
			}
			return;
		}
		
		UE_LOG(LogGridPathFinding, Error, TEXT("[RemoveToken] Token actor not found for coord %s"), *InCoord.ToString());
		return;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[RemoveToken] Coord %s 不存在任何Token"), *InCoord.ToString());
}

void UGridMapModel::UpdateStandingActor(const FHCubeCoord& OldCoord, const FHCubeCoord& NewCoord, AActor* InActor)
{
	if (OldCoord != FHCubeCoord::Invalid)
	{
		check(StandingActors.Contains(OldCoord));
		check(StandingActors[OldCoord] == InActor);
		StandingActors.Remove(OldCoord);
	}

	if (NewCoord == FHCubeCoord::Invalid)
	{
		return;
	}

	StandingActors.Add(NewCoord, InActor);
}

void UGridMapModel::IntervalDeserializeTokens(const FHCubeCoord& InCoord,
	const TArray<FSerializableTokenData>& InTokensData, bool Clear)
{
	// 清空当前的
	if (Clear)
	{
		if (Coord2TokensMap.Contains(InCoord))
		{
			auto& ExistingTokens = Coord2TokensMap[InCoord];
			for (auto& TokenActor : ExistingTokens)
			{
				if (TokenActor)
				{
					TokenActor->Destroy();
				}
			}
			Coord2TokensMap[InCoord].Empty();
		}
	}

	// 创建新的并保存到Map
	TArray<TObjectPtr<ATokenActor>> TokenActors;
	static FActorSpawnParameters SpawnParams;
	SpawnParams.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AlwaysSpawn;
	SpawnParams.ObjectFlags |= RF_Transient;
	for (const auto& TokenData : InTokensData)
	{
		if (TokenData.TokenClass == nullptr)
		{
			UE_LOG(LogGridPathFinding, Error,
				   TEXT("[FBuildTilesDataTask::DoWork] TokenData.TokenClass is null at %s"),
				   *InCoord.ToString());
			continue;
		}

		UE_LOG(LogGridPathFinding, Log, TEXT("[FBuildTilesDataTask::DoWork] 创建TokenActor: %s at %s"),
			   *TokenData.TokenClass->GetName(), *InCoord.ToString());

		// 创建TokenActor实例
		auto Location = StableCoordToWorld(InCoord);
		auto TokenActor = GetWorld()->SpawnActor<ATokenActor>(
			TokenData.TokenClass, Location, FRotator::ZeroRotator, SpawnParams);

		TokenActor->SetActorEnableCollision(EnableTokenCollision);
					
		if (TokenActor)
		{
			TokenActors.Add(TokenActor);
			TokenActor->DeserializeTokenData(TokenData);
		}
		else
		{
			UE_LOG(LogGridPathFinding, Error,
				   TEXT("[FBuildTilesDataTask::DoWork]在 %s 位置创建TokenActor: %s 失败"),
				   *InCoord.ToString(), *TokenData.TokenClass->GetName());
		}
	}

	Coord2TokensMap.Add(InCoord, TokenActors);
}

// 异步任务类的实现
UGridMapModel::FBuildTilesDataTask::FBuildTilesDataTask(UGridMapModel* InOwner,
                                                        const TMap<FHCubeCoord, FSerializableTile>& InTilesData,
                                                        TSharedPtr<TArray<FTileInfo>> OutTiles,
                                                        TSharedPtr<TMap<FHCubeCoord, FTileEnvData>> OutEnvData):
	Owner(InOwner)
	, TilesData(InTilesData)
	, TargetTilesPtr(OutTiles)
	, TargetTileEnvDataMapPtr(OutEnvData)
{
}

void UGridMapModel::FBuildTilesDataTask::DoWork()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task started"));

	// 首先将TilesData的所有键存储到KeyArray中，供后续批量创建Token使用
	KeyArray = MakeShared<TArray<FHCubeCoord>>();
	KeyArray->Reserve(TilesData.Num());
	for (const auto& TileDataPair : TilesData)
	{
		KeyArray->Add(TileDataPair.Key);
	}
	
	// Todo: 出现了一次加载时崩溃的问题， 无法复现
	// 遍历输入的 TilesData 并转换为 FTileInfo
	for (const auto& TileDataPair : TilesData)
	{
		// 检查是否已取消任务
		if (Owner->BuildTilesDataTaskCancelled)
		{
			UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task cancelled during processing"));
			return;
		}

		const FHCubeCoord& Coord = TileDataPair.Key;
		const FSerializableTile& TileData = TileDataPair.Value;

		// 添加到临时数组
		TargetTileEnvDataMapPtr->Add(Coord, TileData.TileEnvData);

		// 如果处理了一批数据，可以在此处添加短暂的休眠，避免阻塞主线程太长时间
		if (TargetTileEnvDataMapPtr->Num() % 1000 == 0)
		{
			FPlatformProcess::Sleep(0.001f); // 每处理1000个格子休眠一小段时间

			// 再次检查是否取消任务
			if (Owner->BuildTilesDataTaskCancelled)
			{
				UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task cancelled during sleep"));
				return;
			}
		}
	}

	auto WG = Owner->GetWorld()->GetSubsystem<UWaitGroupManager>()->CreateWaitGroup(TEXT("Token创建"));
	WG.Value->Add(1);
	CreateTokenWaitGroupID = WG.Key;
	WG.Value->Next([this]()
	{
		bTokensSpawned = true;
	});

	// Todo: 如果任务取消， 需要清空当前的KeyArray，避免后续创建Token时出错
	StartBatchTokenCreation(100, 0);

	while (!bTokensSpawned)
	{
		FPlatformProcess::Sleep(0.001f); // 每处理1000个格子休眠一小段时间
	}

	// 最后检查一次是否任务被取消
	if (Owner->BuildTilesDataTaskCancelled)
	{
		UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task cancelled after processing"));
		return;
	}

	// 处理完成后调用回调
	if (OnComplete)
	{
		AsyncTask(ENamedThreads::GameThread, [this]()
		{
			OnComplete();
		});
	}
}

void UGridMapModel::FBuildTilesDataTask::StartBatchTokenCreation(int32 InBatchSize, int32 CurrentCursor)
{
	AsyncTask(ENamedThreads::GameThread, [this, InBatchSize, CurrentCursor]()
	{
		if (CurrentCursor >= KeyArray->Num())
		{
			UE_LOG(LogGridPathFinding, Log, TEXT("[StartBatchTokenCreation] Token创建完毕"));
			auto WG = Owner->GetWorld()->GetSubsystem<UWaitGroupManager>()->FindWaitGroup(CreateTokenWaitGroupID);
			WG->Done();
			return;
		}

		int32 DesiredEnd = FMath::Min(CurrentCursor + InBatchSize, KeyArray->Num());
		for (int32 i = CurrentCursor; i < DesiredEnd; ++i)
		{
			const FHCubeCoord& Coord =  (*KeyArray)[i];
			const FSerializableTile& TileData = TilesData[Coord];
			Owner->IntervalDeserializeTokens(Coord, TileData.SerializableTokens, false);
		}

	
		// 下一帧继续处理
		Owner->GetWorld()->GetTimerManager().SetTimerForNextTick([this, InBatchSize, CurrentCursor]()
		{
			StartBatchTokenCreation(InBatchSize, CurrentCursor + InBatchSize);
		});
	});
}

FVector UGridMapModel::StableCoordToWorld(const FHCubeCoord& InCoord)
{
	// Set the layout orientation
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			FHTileOrientation TileOrientation = GetTileOrientation(MapConfig.MapType, MapConfig.TileOrientation);
			FVector2D GridSize = MapConfig.GetGridSize();

			float x = ((TileOrientation.f0 * InCoord.QRS.X) + (TileOrientation.f1 * InCoord.QRS.Y)) * GridSize.X;
			float y = ((TileOrientation.f2 * InCoord.QRS.X) + (TileOrientation.f3 * InCoord.QRS.Y)) * GridSize.Y;

			return FVector(x + MapConfig.MapCenter.X, y + MapConfig.MapCenter.Y, MapConfig.MapCenter.Z);
		}
	default:
		break;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[StableCoordToWorld]尚未实现的坐标转换: %d"), (int)MapConfig.MapType);
	return FVector::ZeroVector;
}

FHCubeCoord UGridMapModel::StableWorldToCoord(const FVector& InWorldLocation)
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			FHTileOrientation TileOrientation = GetTileOrientation(MapConfig.MapType, MapConfig.TileOrientation);
			FVector2D GridSize = MapConfig.GetGridSize();

			FVector InternalLocation{
				(InWorldLocation.X - MapConfig.MapCenter.X) / GridSize.X,
				(InWorldLocation.Y - MapConfig.MapCenter.Y) / GridSize.Y,
				(InWorldLocation.Z - MapConfig.MapCenter.Z) // Z is useless here.
			};

			float q = ((TileOrientation.b0 * InternalLocation.X) + (TileOrientation.b1 * InternalLocation.Y));
			float r = ((TileOrientation.b2 * InternalLocation.X) + (TileOrientation.b3 * InternalLocation.Y));

			FVector v{(MapConfig.TileOrientation == ETileOrientationFlag::FLAT) ? FVector(q, -q - r, r) : FVector(q, r, -q - r)};


			FHCubeCoord DesiredCoord = HexCoordRound(FHFractional(v));

			if (MapConfig.MapType == EGridMapType::HEX_STANDARD)
			{
				return DesiredCoord;
			}

			// Todo: 目前只考虑了Flat的情况，Pointy的再说
			// 处理矩形地图的坐标
			// 检查是否在坐标是否在DesiredCoord范围内
			auto DesiredCoordCenter = StableCoordToWorld(DesiredCoord);
			// DrawDebugSphere(GetWorld(), DesiredCoordCenter, 10.f, 12, FColor::Green, false, 0.1f);
			if (FMath::Abs(InWorldLocation.X - DesiredCoordCenter.X) <= GridSize.X * 0.5f &&
				FMath::Abs(InWorldLocation.Y - DesiredCoordCenter.Y) <= GridSize.Y * 0.5f)
			{
				return DesiredCoord;
			}

			// 检查六方向相邻坐标
			for (int32 i = 0; i < 6; ++i)
			{
				auto NeighborCoordPosition = DesiredCoordCenter + SixDirections.DirVectors[i];
				if (FMath::Abs(InWorldLocation.X - NeighborCoordPosition.X) <= GridSize.X * 0.5f &&
					FMath::Abs(InWorldLocation.Y - NeighborCoordPosition.Y) <= GridSize.Y * 0.5f)
				{
					return DesiredCoord + SixDirections.Directions[i];
				}
			}

			// 反解坐标后，不应该出现过大的偏移，如果出现，则是逆矩阵的值算错了。不应该找不到返回值
			check(false);
		}
	default:
		break;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[StableWorldToHex]尚未实现的坐标转换: %d"), (int)MapConfig.MapType);
	return FHCubeCoord::Invalid;
}

FVector UGridMapModel::StableSnapToGridLocation(const FVector& InWorldLocation)
{
	const double TempZ{InWorldLocation.Z};
	FHCubeCoord hexCoord = StableWorldToCoord(InWorldLocation);
	FVector snappedLocation = StableCoordToWorld(hexCoord);
	snappedLocation.Z = TempZ; // 保持原始Z坐标
	return snappedLocation;
}

FIntPoint UGridMapModel::StableCoordToRowColumn(const FHCubeCoord& InCoord) const
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			if (MapConfig.DrawMode == EGridMapDrawMode::BaseOnRadius)
			{
				UE_LOG(LogGridPathFinding, Error, TEXT("[StableCoordToXY]暂不支持的坐标转换: %s"), *UEnum::GetValueAsString(MapConfig.DrawMode));
			}

			if (MapConfig.TileOrientation == ETileOrientationFlag::FLAT)
			{
				return FIntPoint{InCoord.QRS.Y + (InCoord.QRS.X - (InCoord.QRS.X & 1)) / 2, InCoord.QRS.X};
			}

			if (MapConfig.TileOrientation == ETileOrientationFlag::POINTY)
			{
				return FIntPoint{InCoord.QRS.Y, InCoord.QRS.X + (InCoord.QRS.Y - (InCoord.QRS.Y & 1)) / 2};
			}
		}
		break;
	default:
		UE_LOG(LogGridPathFinding, Error, TEXT("[StableCoordToXY]尚未实现的坐标转换: %s"), *UEnum::GetValueAsString(MapConfig.MapType));
		break;
	}

	return FIntPoint::ZeroValue;;
}

FHCubeCoord UGridMapModel::StableRowColumnToCoord(const FIntPoint& InRowColumn) const
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			if (MapConfig.DrawMode == EGridMapDrawMode::BaseOnRadius)
			{
				UE_LOG(LogGridPathFinding, Error, TEXT("[StableRowColumnToCoord]暂不支持的坐标转换: %s"),
				       *UEnum::GetValueAsString(MapConfig.DrawMode));
				return FHCubeCoord::Invalid;
			}

			if (MapConfig.TileOrientation == ETileOrientationFlag::FLAT)
			{
				// 对于Flat方向，从StableCoordToRowColumn的代码反推
				// Row = R + (Q - (Q & 1)) / 2 => R = Row - (Q - (Q & 1)) / 2
				auto Row = InRowColumn.X;
				auto Col = InRowColumn.Y;

				// Q = Col
				int32 Q = Col;
				// 求解R: R = Row - (Q - (Q & 1)) / 2
				int32 R = Row - (Q - (Q & 1)) / 2;
				// 计算S: S = -Q - R
				int32 S = -Q - R;

				return FHCubeCoord(FIntVector(Q, R, S));
			}

			if (MapConfig.TileOrientation == ETileOrientationFlag::POINTY)
			{
				// 对于Pointy方向，从StableCoordToRowColumn的代码反推
				// Row = R, Col = Q + (R - (R & 1)) / 2 => Q = Col - (R - (R & 1)) / 2
				auto Row = InRowColumn.X;
				auto Col = InRowColumn.Y;

				// R = Row
				int32 R = Row;
				// 求解Q: Q = Col - (R - (R & 1)) / 2
				int32 Q = Col - (R - (R & 1)) / 2;
				// 计算S: S = -Q - R
				int32 S = -Q - R;

				return FHCubeCoord(FIntVector(Q, R, S));
			}
		}
		break;
	default:
		UE_LOG(LogGridPathFinding, Error, TEXT("[StableRowColumnToCoord]尚未实现的坐标转换: %s"),
		       *UEnum::GetValueAsString(MapConfig.MapType));
		break;
	}

	return FHCubeCoord::Invalid;
}

FVector UGridMapModel::StableRowColumnToWorld(const FIntPoint& InRowColumn)
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			auto Coord = StableRowColumnToCoord(InRowColumn);
			return StableCoordToWorld(Coord);
		}
	default:
		break;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("[StableRowColumnToWorld]尚未实现的坐标转换: %s"), *UEnum::GetValueAsString(MapConfig.MapType));
	return FVector::ZeroVector;
}

const FHCubeCoord UGridMapModel::GetNeighborCoord(const FHCubeCoord& InCoord, int32 InDirection) const
{
	return InCoord + SixDirections.Directions[InDirection];
}

const FHCubeCoord UGridMapModel::GetBackwardCoord(const FHCubeCoord& InLocalCoord, const FHCubeCoord& InNextCoord) const
{
	// 计算从 InLocalCoord 到 InNextCoord 的方向向量
	FHCubeCoord Direction = FHCubeCoord(
		InNextCoord.QRS.X - InLocalCoord.QRS.X,
		InNextCoord.QRS.Y - InLocalCoord.QRS.Y,
		InNextCoord.QRS.Z - InLocalCoord.QRS.Z
	);

	// 将这个方向应用到 InNextCoord，得到背后一格的位置
	return FHCubeCoord(
		InNextCoord.QRS.X + Direction.QRS.X,
		InNextCoord.QRS.Y + Direction.QRS.Y,
		InNextCoord.QRS.Z + Direction.QRS.Z
	);
}

TArray<FHCubeCoord> UGridMapModel::GetCoordsBetween(const FHCubeCoord& StartCoord, const FHCubeCoord& EndCoord)
{
	// Todo: AI写的，尚未验证
	TArray<FHCubeCoord> Results;

	// 计算两点之间的距离（六边形距离）
	const int32 N = GetDistance(StartCoord, EndCoord);

	// 如果起点和终点相同，直接返回起点
	if (N == 0)
	{
		Results.Add(StartCoord);
		return Results;
	}

	// 添加一个微小的偏移量，以避免落在六边形边界上
	const FHFractional Epsilon(1e-6, 2e-6, -3e-6);
	FHFractional Start(static_cast<double>(StartCoord.QRS.X),
	                   static_cast<double>(StartCoord.QRS.Y),
	                   static_cast<double>(StartCoord.QRS.Z));
	FHFractional End(static_cast<double>(EndCoord.QRS.X),
	                 static_cast<double>(EndCoord.QRS.Y),
	                 static_cast<double>(EndCoord.QRS.Z));

	// 应用微小偏移量
	Start.QRS.X += Epsilon.QRS.X;
	Start.QRS.Y += Epsilon.QRS.Y;
	Start.QRS.Z += Epsilon.QRS.Z;

	// 计算线段上的所有点
	for (int32 i = 0; i <= N; ++i)
	{
		// 计算插值系数
		const double t = (N == 0) ? 0.0 : static_cast<double>(i) / static_cast<double>(N);

		// 在立方体坐标空间中进行线性插值
		FHFractional LerpResult(
			Start.QRS.X + (End.QRS.X - Start.QRS.X) * t,
			Start.QRS.Y + (End.QRS.Y - Start.QRS.Y) * t,
			Start.QRS.Z + (End.QRS.Z - Start.QRS.Z) * t
		);

		// 将插值结果四舍五入到最近的六边形坐标
		FHCubeCoord RoundedCoord = HexCoordRound(LerpResult);

		// 添加到结果数组
		Results.Add(RoundedCoord);
	}

	// 剔除StartCoord
	if (Results.Num() > 0 && Results[0] == StartCoord)
	{
		Results.RemoveAt(0);
	}

	return Results;
}

bool UGridMapModel::IsCoordInMapArea(const FHCubeCoord& InCoord) const
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			switch (MapConfig.DrawMode)
			{
			case EGridMapDrawMode::BaseOnRadius:
			case EGridMapDrawMode::BaseOnVolume:
				UE_LOG(LogGridPathFinding, Error, TEXT("[IsCoordInMapArea]暂不支持的坐标转换: %s"), *UEnum::GetValueAsString(MapConfig.DrawMode));
				break;
			case EGridMapDrawMode::BaseOnRowColumn:
				{
					auto RowColumn = StableCoordToRowColumn(InCoord);
					return RowColumn.X >= -MapConfig.MapSize.X / 2.f &&
						RowColumn.X <= MapConfig.MapSize.X / 2.f &&
						RowColumn.Y >= -MapConfig.MapSize.Y / 2.f &&
						RowColumn.Y <= MapConfig.MapSize.Y / 2.f;
				}
			}
		}
	default:
		UE_LOG(LogGridPathFinding, Error, TEXT("[IsCoordInMapArea]尚未实现的坐标转换: %s"), *UEnum::GetValueAsString(MapConfig.MapType));
		break;
	}

	return false;
}

void UGridMapModel::StableForEachMapGrid(
	TFunction<void(const FHCubeCoord& Coord, int32 Row, int32 Column)> TileFunction)
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		{
			switch (MapConfig.DrawMode)
			{
			case EGridMapDrawMode::BaseOnRadius:
				{
				}
				break;
			case EGridMapDrawMode::BaseOnRowColumn:
				{
					const auto RowStart = -FMath::FloorToInt(MapConfig.MapSize.X / 2.f);
					const auto RowEnd = FMath::CeilToInt(MapConfig.MapSize.X / 2.f);
					const auto ColumnStart = -FMath::FloorToInt(MapConfig.MapSize.Y / 2.f);
					const auto ColumnEnd = FMath::CeilToInt(MapConfig.MapSize.Y / 2.f);

					if (MapConfig.TileOrientation == ETileOrientationFlag::FLAT)
					{
						for (int32 Column{ColumnStart}; Column < ColumnEnd; ++Column)
						{
							for (int32 Row{RowStart}; Row < RowEnd; ++Row)
							{
								auto Q = Column;
								auto R = Row - (Column - (Column & 1)) / 2;
								FHCubeCoord CCoord{FIntVector(Q, R, -Q - R)};

								// 调用传入的函数
								TileFunction(CCoord, Row, Column);
							}
						}
					}
					else if (MapConfig.TileOrientation == ETileOrientationFlag::POINTY)
					{
						for (int32 Row{RowStart}; Row < RowEnd; ++Row)
						{
							for (int32 Column{ColumnStart}; Column < ColumnEnd; ++Column)
							{
								auto Q = Column - (Row - (Row & 1)) / 2;
								auto R = Row;
								FHCubeCoord CCoord{FIntVector(Q, R, -Q - R)};
								// 调用传入的函数
								TileFunction(CCoord, Row, Column);
							}
						}
					}
				}
				break;
			case EGridMapDrawMode::BaseOnVolume:
				{
					// Todo
				}
				break;
			}
		}
		break;
	case EGridMapType::SQUARE_STANDARD:
		break;
	case EGridMapType::RECTANGLE_STANDARD:
		break;
	}
}

// 不需要缓存Coord到Index的映射，寻路中只在初始化时调用了两次
int32 UGridMapModel::StableGetFullMapGridIterIndex(const FHCubeCoord& InCoord) const
{
	// 计算背景中Coord所在Instance的Index
	auto Q = InCoord.QRS.X;
	auto R = InCoord.QRS.Y;

	switch (MapConfig.DrawMode)
	{
	case EGridMapDrawMode::BaseOnRadius:
		UE_LOG(LogGridPathFinding, Error, TEXT("[HighLightBackground] 暂不支持遍历: %s"), *UEnum::GetValueAsString(MapConfig.DrawMode));
		break;
	case EGridMapDrawMode::BaseOnRowColumn:
		{
			const auto RowStart = -FMath::FloorToInt(MapConfig.MapSize.X / 2.f);
			const auto RowEnd = FMath::CeilToInt(MapConfig.MapSize.X / 2.f);
			const auto ColumnStart = -FMath::FloorToInt(MapConfig.MapSize.Y / 2.f);
			const auto ColumnEnd = FMath::CeilToInt(MapConfig.MapSize.Y / 2.f);

			if (MapConfig.TileOrientation == ETileOrientationFlag::FLAT)
			{
				auto Row = R + (Q - (Q & 1)) / 2;
				if (Q < ColumnStart || Q >= ColumnEnd || Row < RowStart || Row >= RowEnd)
				{
					// 不在地图中
					break;
				}

				auto DeltaCol = Q - ColumnStart;
				int32 PassedCount = DeltaCol * MapConfig.MapSize.X;

				return PassedCount + Row - RowStart;
			}

			if (MapConfig.TileOrientation == ETileOrientationFlag::POINTY)
			{
				auto Col = Q + (R - (R & 1)) / 2;
				if (Col < ColumnStart || Col >= ColumnEnd || R < RowStart || R >= RowEnd)
				{
					break;
				}

				auto DeltaRow = R - RowStart;
				int32 PassedCount = DeltaRow * MapConfig.MapSize.Y;

				return PassedCount + Col - ColumnStart;
			}
		}
		break;
	case EGridMapDrawMode::BaseOnVolume:
		UE_LOG(LogGridPathFinding, Error, TEXT("[HighLightBackground] 暂不支持高亮: %s"), *UEnum::GetValueAsString(MapConfig.DrawMode));
		break;
	}

	return INDEX_NONE;
}

int32 UGridMapModel::StableGetChunkCount() const
{
	auto GSettings = GetDefault<UGridPathFindingSettings>();
	switch (MapConfig.DrawMode)
	{
	case EGridMapDrawMode::BaseOnRadius:
		{
		}
		break;
	case EGridMapDrawMode::BaseOnRowColumn:
		{
			// 从左上开始， 每ChunkRowSize*ChunkColumnSize的方形区域构成一个Chunk, 根据MapSize计算总共需要划分几个区块
			const auto RowStart = -FMath::FloorToInt(MapConfig.MapSize.X / 2.f);
			const auto RowEnd = FMath::CeilToInt(MapConfig.MapSize.X / 2.f);
			const auto ColumnStart = -FMath::FloorToInt(MapConfig.MapSize.Y / 2.f);
			const auto ColumnEnd = FMath::CeilToInt(MapConfig.MapSize.Y / 2.f);

			const int32 MapRows = RowEnd - RowStart;
			const int32 MapColumns = ColumnEnd - ColumnStart;

			const int32 NumChunksX = FMath::CeilToInt((float)MapRows / GSettings->MapChunkSize.X);
			const int32 NumChunksY = FMath::CeilToInt((float)MapColumns / GSettings->MapChunkSize.Y);

			return NumChunksX * NumChunksY;
		}
	case EGridMapDrawMode::BaseOnVolume:
		{
			// Todo
		}
		break;
	}

	return 0;
}

int32 UGridMapModel::StableGetCoordChunkIndex(const FHCubeCoord& InCoord) const
{
	auto GSettings = GetDefault<UGridPathFindingSettings>();
	const int32 ChunkSize = GSettings->MapChunkSize.X; // 使用正确的Chunk大小而不是Chunk数量
	switch (MapConfig.DrawMode)
	{
	case EGridMapDrawMode::BaseOnRadius:
		{
		}
		break;
	case EGridMapDrawMode::BaseOnRowColumn:
		{
			const auto RowStart = -FMath::FloorToInt(MapConfig.MapSize.X / 2.f);
			const auto ColumnStart = -FMath::FloorToInt(MapConfig.MapSize.Y / 2.f);
			const auto ColumnEnd = FMath::CeilToInt(MapConfig.MapSize.Y / 2.f);

			auto CoordRowCol = StableCoordToRowColumn(InCoord);

			// 计算相对于地图左上角的偏移
			int32 RelRow = CoordRowCol.X - RowStart;
			int32 RelCol = CoordRowCol.Y - ColumnStart;

			// 计算该坐标所在的区块行列
			int32 ChunkRow = RelRow / ChunkSize;
			int32 ChunkCol = RelCol / ChunkSize;

			// 计算地图的总区块列数
			const int32 MapColumns = ColumnEnd - ColumnStart;
			const int32 NumChunksY = FMath::CeilToInt(static_cast<double>(MapColumns) / ChunkSize);

			// 计算区块索引：区块行 * 每行区块数 + 区块列
			return ChunkRow * NumChunksY + ChunkCol;
		}
	case EGridMapDrawMode::BaseOnVolume:
		{
			// Todo
		}
		break;
	}

	return INDEX_NONE;
}

// 不需要缓存， 只在计算寻路结果时的一个遍历中调用
FHCubeCoord UGridMapModel::StableGetCoordByIndex(const int32 InIndex) const
{
	switch (MapConfig.DrawMode)
    {
    case EGridMapDrawMode::BaseOnRowColumn:
        {
            const auto RowStart = -FMath::FloorToInt(MapConfig.MapSize.X / 2.f);
            const auto RowEnd = FMath::CeilToInt(MapConfig.MapSize.X / 2.f);
            const auto ColumnStart = -FMath::FloorToInt(MapConfig.MapSize.Y / 2.f);
            const auto ColumnEnd = FMath::CeilToInt(MapConfig.MapSize.Y / 2.f);
            
            const int32 MapRows = RowEnd - RowStart;
            const int32 MapColumns = ColumnEnd - ColumnStart;
            
            if (InIndex < 0 || InIndex >= MapRows * MapColumns)
            {
                UE_LOG(LogGridPathFinding, Warning, TEXT("StableGetCoordByIndex: Index %d out of range"), InIndex);
                return FHCubeCoord::Invalid;
            }

            if (MapConfig.TileOrientation == ETileOrientationFlag::FLAT)
            {
                // 基于遍历顺序：Column优先，然后Row
                int32 DeltaCol = InIndex / MapRows;
                int32 DeltaRow = InIndex % MapRows;
                
                int32 Column = ColumnStart + DeltaCol;
                int32 Row = RowStart + DeltaRow;
                
                auto Q = Column;
                auto R = Row - (Column - (Column & 1)) / 2;
                return FHCubeCoord{FIntVector(Q, R, -Q - R)};
            }

    		if (MapConfig.TileOrientation == ETileOrientationFlag::POINTY)
            {
	            // 基于遍历顺序：Row优先，然后Column
	            int32 DeltaRow = InIndex / MapColumns;
	            int32 DeltaCol = InIndex % MapColumns;
                
	            int32 Row = RowStart + DeltaRow;
	            int32 Column = ColumnStart + DeltaCol;
                
	            auto Q = Column - (Row - (Row & 1)) / 2;
	            auto R = Row;
	            return FHCubeCoord{FIntVector(Q, R, -Q - R)};
            }
        }
        break;
    case EGridMapDrawMode::BaseOnRadius:
        // TODO: 实现基于半径的索引转换
        UE_LOG(LogGridPathFinding, Warning, TEXT("StableGetCoordByIndex: BaseOnRadius not implemented yet"));
        break;
    case EGridMapDrawMode::BaseOnVolume:
        // TODO: 实现基于体积的索引转换
        UE_LOG(LogGridPathFinding, Warning, TEXT("StableGetCoordByIndex: BaseOnVolume not implemented yet"));
        break;
    }
    
    return FHCubeCoord::Invalid;
}

int32 UGridMapModel::GetDistance(const FHCubeCoord& A, const FHCubeCoord& B) const
{
	switch (MapConfig.MapType)
	{
	case EGridMapType::HEX_STANDARD:
	case EGridMapType::RECTANGLE_SIX_DIRECTION:
		return (FMath::Abs(A.QRS.X - B.QRS.X) + FMath::Abs(A.QRS.Y - B.QRS.Y) + FMath::Abs(A.QRS.Z - B.QRS.Z)) / 2;
	case EGridMapType::SQUARE_STANDARD:
		UE_LOG(LogGridPathFinding, Error, TEXT("GetDistance: 暂不支持的地图类型 %s"),
		       *UEnum::GetValueAsString(MapConfig.MapType));
		break;
	case EGridMapType::RECTANGLE_STANDARD:
		UE_LOG(LogGridPathFinding, Error, TEXT("GetDistance: 暂不支持的地图类型 %s"),
		       *UEnum::GetValueAsString(MapConfig.MapType));
		break;
	}

	return 0;
}

int32 UGridMapModel::GetNeighborDirection(const FHCubeCoord& From, const FHCubeCoord& To) const
{
	if (GetDistance(From, To) != 1)
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("GetNeighborDirection: From and To are not neighbors"));
		return 0; // 如果不是相邻格子，返回0
	}

	// 根据SixDirections返回方向
	for (int32 i = 0; i < SixDirections.Directions.Num(); ++i)
	{
		if (From + SixDirections.Directions[i] == To)
		{
			return i; // 返回方向索引
		}
	}
	UE_LOG(LogGridPathFinding, Error, TEXT("GetNeighborDirection: No valid direction found from %s to %s"),
	       *From.ToString(), *To.ToString());
	return 0;
}


void UGridMapModel::StableGetChunkCoords(const int32 InChunkIndex, TArray<FHCubeCoord>& OutCoords) const
{
	auto GSettings = GetDefault<UGridPathFindingSettings>();
	const int32 ChunkSize = GSettings->MapChunkSize.X; // 假设X和Y相同

	switch (MapConfig.DrawMode)
	{
	case EGridMapDrawMode::BaseOnRowColumn:
		{
			// 计算地图边界
			const auto RowStart = -FMath::FloorToInt(MapConfig.MapSize.X / 2.f);
			const auto ColumnStart = -FMath::FloorToInt(MapConfig.MapSize.Y / 2.f);
			const auto ColumnEnd = FMath::CeilToInt(MapConfig.MapSize.Y / 2.f);
			// 计算区块数量
			const int32 MapColumns = ColumnEnd - ColumnStart;
			const int32 NumChunksY = FMath::CeilToInt(static_cast<float>(MapColumns) / ChunkSize);

			// 计算区块的行列
			const int32 ChunkRow = InChunkIndex / NumChunksY;
			const int32 ChunkCol = InChunkIndex % NumChunksY;

			// 计算区块的起始行列
			const int32 StartRow = RowStart + ChunkRow * ChunkSize;
			const int32 StartCol = ColumnStart + ChunkCol * ChunkSize;
			const int32 EndRow = FMath::Min(StartRow + ChunkSize, RowStart + MapConfig.MapSize.X);
			const int32 EndCol = FMath::Min(StartCol + ChunkSize, ColumnStart + MapConfig.MapSize.Y);

			if (MapConfig.TileOrientation == ETileOrientationFlag::FLAT)
			{
				for (int32 Column = StartCol; Column < EndCol; ++Column)
				{
					for (int32 Row = StartRow; Row < EndRow; ++Row)
					{
						auto Q = Column;
						auto R = Row - (Column - (Column & 1)) / 2;
						FHCubeCoord Coord(FIntVector(Q, R, -Q - R));
						OutCoords.Add(Coord);
					}
				}
			}
			else // POINTY
			{
				for (int32 Row = StartRow; Row < EndRow; ++Row)
				{
					for (int32 Column = StartCol; Column < EndCol; ++Column)
					{
						auto Q = Column - (Row - (Row & 1)) / 2;
						auto R = Row;
						FHCubeCoord Coord(FIntVector(Q, R, -Q - R));
						OutCoords.Add(Coord);
					}
				}
			}
			break;
		}

	case EGridMapDrawMode::BaseOnRadius:
	case EGridMapDrawMode::BaseOnVolume:
		UE_LOG(LogGridPathFinding, Warning, TEXT("StableGetChunkCoords: 暂不支持在 %s 模式下获取区块坐标"),
		       *UEnum::GetValueAsString(MapConfig.DrawMode));
		break;
	}
}

FTileInfo UGridMapModel::IntervalCreateTileInfo(const FSerializableTile& InTileData)
{
	FTileInfo TileInfo;
	TileInfo.CubeCoord = InTileData.Coord;
	TileInfo.Height = InTileData.Height;

	// 暂时用默认值填充 Cost 和 bIsBlocking，实际应用中应从 EnvironmentType 中读取
	// 在实际项目中，这部分可能需要根据环境类型查找相应的数据
	// TODO: 从 EnvironmentType 中获取 Cost 和 bIsBlocking
	TileInfo.Cost = 1.0f;
	TileInfo.bIsBlocking = false;

	// Todo: 地图创建完成后，更新Cost和bIsBlocking数据
	
	return TileInfo;
}

FHTileOrientation UGridMapModel::GetTileOrientation(EGridMapType InMapType, ETileOrientationFlag InOrientation)
{
	if (InMapType == EGridMapType::HEX_STANDARD)
	{
		if (InOrientation == ETileOrientationFlag::FLAT)
			return HFlatTopLayout;

		return HPointyLayout;
	}

	if (InMapType == EGridMapType::RECTANGLE_SIX_DIRECTION)
	{
		if (InOrientation == ETileOrientationFlag::FLAT)
			return RectFlatTopLayout;

		return RectPointyTopLayout;
	}

	UE_LOG(LogGridPathFinding, Error, TEXT("未知的地图类型: %d"), (int)InMapType);
	return FHTileOrientation();
}

TArray<FHCubeCoord> UGridMapModel::GetRangeCoords(const FHCubeCoord& Center, int32 Radius) const
{
	TArray<FHCubeCoord> Result;

	for (int32 dq = -Radius; dq <= Radius; ++dq)
	{
		for (int32 dr = FMath::Max(-Radius, -dq - Radius); dr <= FMath::Min(Radius, -dq + Radius); ++dr)
		{
			int32 ds = -dq - dr;
			Result.Add(FHCubeCoord{FIntVector(Center.QRS.X + dq, Center.QRS.Y + dr, Center.QRS.Z + ds)});
		}
	}
	return Result;
}

int32 UGridMapModel::GetNeighborIndex(int32 NodeIndex, int32 Direction) const
{
	return NeighborIndicesCache[NodeIndex][Direction];
}

FHCubeCoord UGridMapModel::HexCoordRound(const FHFractional& F)
{
	int32 q{int32(FMath::RoundToDouble(F.QRS.X))};
	int32 r{int32(FMath::RoundToDouble(F.QRS.Y))};
	int32 s{int32(FMath::RoundToDouble(F.QRS.Z))};

	double q_diff{FMath::Abs(q - F.QRS.X)};
	const double r_diff{FMath::Abs(r - F.QRS.Y)};
	const double s_diff{FMath::Abs(s - F.QRS.Z)};

	if ((q_diff > r_diff) && (q_diff > s_diff))
	{
		q = -r - s;
	}
	else if (r_diff > s_diff)
	{
		r = -q - s;
	}
	else
	{
		s = -q - r;
	}

	return FHCubeCoord{FIntVector(q, r, s)};
}

void UGridMapModel::BuildPathFindingCache()
{
	if (MapConfig.DrawMode == EGridMapDrawMode::BaseOnRowColumn)
	{
		MaxValidIndex = MapConfig.MapSize.X * MapConfig.MapSize.Y - 1;
	}
	else
	{
		UE_LOG(LogGridPathFinding, Error, TEXT("[Error] 未实现寻路缓存, 会导致寻路错误, 绘制模式 %s"), *UEnum::GetValueAsString(MapConfig.DrawMode));
	}
	
	NeighborIndicesCache.Empty();

	// 计算总的网格数量
	int32 TotalGridCount = MapConfig.MapSize.X * MapConfig.MapSize.Y;
	NeighborIndicesCache.SetNum(TotalGridCount);
    
	// 为每个Index计算其6个邻居
	for (int32 i = 0; i < TotalGridCount; ++i)
	{
		NeighborIndicesCache[i].SetNum(6);
		FHCubeCoord CurrentCoord = StableGetCoordByIndex(i);
        
		for (int32 dir = 0; dir < 6; ++dir)
		{
			FHCubeCoord NeighborCoord = GetNeighborCoord(CurrentCoord, dir);
			NeighborIndicesCache[i][dir] = StableGetFullMapGridIterIndex(NeighborCoord);
		}
	}
}
