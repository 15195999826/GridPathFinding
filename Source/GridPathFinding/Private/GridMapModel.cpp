// Fill out your copyright notice in the Description page of Project Settings.


#include "GridMapModel.h"

#include "GridPathFinding.h"
#include "GridPathFindingSettings.h"
#include "HGTypes.h"

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
	if (MapConfig.MapType == EGridMapType::RECTANGLE_SIX_DIRECTION)
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
	auto TempMapPtr = MakeShared<TMap<FHCubeCoord, int32>>();
	auto TempEnvDataPtr = MakeShared<TMap<FHCubeCoord, FTileEnvData>>();
	TempTilesPtr->Reserve(InTilesData.Num());
	TempMapPtr->Reserve(InTilesData.Num());
	TempEnvDataPtr->Reserve(InTilesData.Num());

	// 创建异步任务
	BuildTilesDataTask = MakeShared<FAsyncTask<FBuildTilesDataTask>>(this, InTilesData, TempTilesPtr, TempMapPtr, TempEnvDataPtr);

	// 设置任务完成的回调，将结果复制到 Tiles 数组
	BuildTilesDataTask->GetTask().OnComplete = [this, TempTilesPtr, TempMapPtr, TempEnvDataPtr]
	{
		if (!BuildTilesDataTaskCancelled)
		{
			// 将临时数组中的数据复制到 Tiles 数组
			FScopeLock Lock(&TilesLock);
			Tiles = MoveTemp(*TempTilesPtr);
			TileCoordToIndexMap = MoveTemp(*TempMapPtr);
			TileEnvDataMap = MoveTemp(*TempEnvDataPtr);
			UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData completed successfully with %d tiles, IndexMap Num: %d, EnvData Num: %d"),
			       Tiles.Num(), TileCoordToIndexMap.Num(), TileEnvDataMap.Num());

			// 触发地图数据更新完成事件
			IsBuilding = false;
			OnTilesDataBuildComplete.Broadcast();
		}
		else
		{
			UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task was cancelled"));
		}
	};

	// TFuture<void> Task = Async(EAsyncExecution::ThreadPool, [this]() {
	// 	// 这里可以执行一些初始化操作
	// 	// 例如，设置任务的初始状态等
	// });
	// 启动异步任务
	BuildTilesDataTask->StartBackgroundTask();
}

void UGridMapModel::ModifyTilesData(EGridMapModelTileModifyType ModifyType, const FSerializableTile& InTileData,
                                    bool bNotify)
{
	FTileInfo OldTileInfo;
	FTileInfo NewTileInfo;
	switch (ModifyType)
	{
	case EGridMapModelTileModifyType::Add:
		{
			OldTileInfo.EnvironmentType = UGridEnvironmentType::EmptyEnvTypeID;
			NewTileInfo = IntervalCreateTileInfo(InTileData);
			Tiles.Add(NewTileInfo);
			TileCoordToIndexMap.Add(InTileData.Coord, Tiles.Num() - 1);
			TileEnvDataMap.Add(InTileData.Coord, InTileData.TileEnvData);
		}
		break;
	case EGridMapModelTileModifyType::Remove:
		{
			auto Index = TileCoordToIndexMap.Find(InTileData.Coord);
			check(Index);
			OldTileInfo = Tiles[*Index];
			NewTileInfo = OldTileInfo;

			int32 RemoveIdx = *Index;
			Tiles.RemoveAt(RemoveIdx);
			TileCoordToIndexMap.Remove(InTileData.Coord);
			TileEnvDataMap.Remove(InTileData.Coord);

			for (auto& Pair : TileCoordToIndexMap)
			{
				if (Pair.Value > RemoveIdx)
				{
					Pair.Value--;
				}
			}
		}
		break;
	case EGridMapModelTileModifyType::Update:
		{
			auto Index = TileCoordToIndexMap.Find(InTileData.Coord);
			check(Index);
			OldTileInfo = Tiles[*Index];
			NewTileInfo = IntervalCreateTileInfo(InTileData);
			Tiles[*Index] = NewTileInfo;
			TileEnvDataMap[InTileData.Coord] = InTileData.TileEnvData;
		}
		break;
	}

	if (bNotify)
	{
		OnTileModify.Broadcast(ModifyType, InTileData.Coord, OldTileInfo, NewTileInfo);
	}
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

// 异步任务类的实现
UGridMapModel::FBuildTilesDataTask::FBuildTilesDataTask(UGridMapModel* InOwner,
                                                        const TMap<FHCubeCoord, FSerializableTile>& InTilesData,
                                                        TSharedPtr<TArray<FTileInfo>> OutTiles,
                                                        TSharedPtr<TMap<FHCubeCoord, int32>> OutIndexMap,
                                                        TSharedPtr<TMap<FHCubeCoord, FTileEnvData>> OutEnvData):
	Owner(InOwner)
	, TilesData(InTilesData)
	, TargetTilesPtr(OutTiles)
	, TargetTileCoordToIndexMapPtr(OutIndexMap)
	, TargetTileEnvDataMapPtr(OutEnvData)
{
}

void UGridMapModel::FBuildTilesDataTask::DoWork()
{
	UE_LOG(LogGridPathFinding, Log, TEXT("BuildTilesData task started"));
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

		// 创建并填充 FTileInfo
		FTileInfo TileInfo = IntervalCreateTileInfo(TileData);

		// 添加到临时数组
		TargetTileCoordToIndexMapPtr->Add(Coord, TargetTilesPtr->Num());
		TargetTileEnvDataMapPtr->Add(Coord, TileData.TileEnvData);
		TargetTilesPtr->Add(TileInfo);

		// 如果处理了一批数据，可以在此处添加短暂的休眠，避免阻塞主线程太长时间
		if (TargetTilesPtr->Num() % 1000 == 0)
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

const FHCubeCoord UGridMapModel::GetNeighborCoord(const FHCubeCoord& InCoord, int32 InDirection) const
{
	return InCoord + SixDirections.Directions[InDirection];
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

int UGridMapModel::GetTileIndex(const FHCubeCoord& InCoord) const
{
	// Todo:
	return -1;
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

int32 UGridMapModel::StableGetFullMapGridIterIndex(const FHCubeCoord& InCoord)
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

int32 UGridMapModel::GetDistance(const FHCubeCoord& A, const FHCubeCoord& B) const
{
	switch (MapConfig.MapType) {
		case EGridMapType::HEX_STANDARD:
			case EGridMapType::RECTANGLE_SIX_DIRECTION:
		return (FMath::Abs(A.QRS.X - B.QRS.X) + FMath::Abs(A.QRS.Y - B.QRS.Y) + FMath::Abs(A.QRS.Z - B.QRS.Z)) / 2;
		case EGridMapType::SQUARE_STANDARD:
			UE_LOG(LogGridPathFinding, Error, TEXT("GetDistance: 暂不支持的地图类型 %s"), *UEnum::GetValueAsString(MapConfig.MapType));
			break;
		case EGridMapType::RECTANGLE_STANDARD:
			UE_LOG(LogGridPathFinding, Error, TEXT("GetDistance: 暂不支持的地图类型 %s"), *UEnum::GetValueAsString(MapConfig.MapType));
			break;
	}

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
	TileInfo.EnvironmentType = InTileData.EnvironmentType;
	TileInfo.Height = InTileData.Height;

	// 暂时用默认值填充 Cost 和 bIsBlocking，实际应用中应从 EnvironmentType 中读取
	// 在实际项目中，这部分可能需要根据环境类型查找相应的数据
	// TODO: 从 EnvironmentType 中获取 Cost 和 bIsBlocking
	TileInfo.Cost = 1.0f;
	TileInfo.bIsBlocking = false;

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
