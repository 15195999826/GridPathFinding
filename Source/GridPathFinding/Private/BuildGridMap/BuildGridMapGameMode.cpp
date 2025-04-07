// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/BuildGridMapGameMode.h"

#include "EngineUtils.h"
#include "GridMapModel.h"
#include "GridMapRenderer.h"
#include "GridPathFinding.h"
#include "GridPathFindingBlueprintFunctionLib.h"
#include "GridPathFindingSettings.h"
#include "JsonObjectConverter.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/Command/BuildGridMapChangeMapNameCommand.h"
#include "BuildGridMap/Command/BuildGridMapChangeMapTypeCommand.h"
#include "BuildGridMap/Command/BuildGridMapChangeTileEnvCommand.h"
#include "BuildGridMap/Command/BuildGridMapChangeTileEnvTextureCommand.h"
#include "BuildGridMap/Command/BuildGridMapChangeTileSizeCommand.h"
#include "BuildGridMap/Command/BuildGridMapCommandManager.h"
#include "BuildGridMap/UI/BuildGridMapMapConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "Components/EditableTextBox.h"

void ABuildGridMapGameMode::BeginPlay()
{
	DataRecover();
	
	// 遍历场景Actor查询GridMapRenderer
	for (TActorIterator<ABuildGridMapRenderer> It(GetWorld()); It; ++It)
	{
		BuildGridMapRenderer = *It;
	}

	// check(BackgroundModel);
	check(BuildGridMapRenderer);
	
	check(GridMapModel == nullptr);
	GridMapModel = NewObject<UGridMapModel>(this, UGridMapModel::StaticClass());
	BuildGridMapRenderer->SetModel(GridMapModel);

	CommandManager = NewObject<UBuildGridMapCommandManager>(this);
	EmptyMapSave.MapName = TEXT("SP_Empty");
	EmptyMapSave.MapConfig.MapSize = FIntPoint(1, 1);
	EditingMapSave = EmptyMapSave;
	HasValidMapSave = false;
	Super::BeginPlay();


	// 监听窗口事件， 然后通过CommandManager来执行命令
	// 地图配置
	BuildGridMapWindow->MapConfigWidget->MapNameTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnMapNameTextCommitted);
	BuildGridMapWindow->MapConfigWidget->OnMapTypeChanged.AddUObject(this, &ABuildGridMapGameMode::OnMapTypeSelectionChanged);
	BuildGridMapWindow->MapConfigWidget->OnMapDrawModeChanged.AddUObject(this, &ABuildGridMapGameMode::OnMapDrawModeSelectionChanged);
	BuildGridMapWindow->MapConfigWidget->OnTileOrientationChanged.AddUObject(this, &ABuildGridMapGameMode::OnHexTileOrientationSelectionChanged);
	BuildGridMapWindow->MapConfigWidget->MapRowTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnMapRowTextCommitted);
	BuildGridMapWindow->MapConfigWidget->MapColumnTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnMapColumnTextCommitted);
	BuildGridMapWindow->MapConfigWidget->MapRadiusTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnMapRadiusTextCommitted);
	BuildGridMapWindow->MapConfigWidget->GridSizeXTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnGridSizeXTextCommitted);
	BuildGridMapWindow->MapConfigWidget->GridSizeYTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnGridSizeYTextCommitted);
	BuildGridMapWindow->MapConfigWidget->GridRadiusTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnGridRadiusTextCommitted);

	// 格子配置
	BuildGridMapWindow->TileConfigWidget->OnTileEnvChanged.AddUObject(this, &ABuildGridMapGameMode::OnTileEnvSelectionChanged);
	BuildGridMapWindow->TileConfigWidget->EnvTextureIndexTextBox->OnTextCommitted.AddDynamic(this, &ABuildGridMapGameMode::OnEnvTextureIndexTextCommitted);

	OnSaveStart.AddLambda([this](EBuildGridMapSaveMode InSaveMode) {
        bIsSaving = true;
    });

	OnSaveOver.AddLambda([this]() {
        bIsSaving = false;
    });
	
	FGGB_SaveData SaveData;
	SaveData.Data = "Test";
	SaveData.Transform = FTransform(FVector(1, 2, 3));
	SaveData.HISM = {
		FTransform(FVector(1, 2, 3)),
		FTransform(FVector(4, 5, 6)),
		FTransform(FVector(7, 8, 9))
	};

	// 使用Json序列化
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(SaveData, JsonString);
	
	// 测试二进制序列化
	auto s = UGridPathFindingBlueprintFunctionLib::GGB_SerializeSaveData(SaveData);

	// 对比二级制字符产和Json字符串， 哪个数据更短
	
	int32 JsonSize = JsonString.Len();
	int32 BinarySize = s.Len();

	UE_LOG(LogTemp, Warning, TEXT("序列化大小比较:"));
	UE_LOG(LogTemp, Warning, TEXT("JSON数据大小: %d 字节"), JsonSize);
	UE_LOG(LogTemp, Warning, TEXT("二进制数据大小: %d 字节"), BinarySize);
	UE_LOG(LogTemp, Warning, TEXT("二进制/JSON比例: %.2f%%"), (float)BinarySize / JsonSize * 100.0f);

	auto deco = UGridPathFindingBlueprintFunctionLib::GGB_DeSerializeSaveData(s);
	UE_LOG(LogTemp, Warning, TEXT("Data: %s"), *deco.Data);
	UE_LOG(LogTemp, Warning, TEXT("Transform: %s"), *deco.Transform.ToString());
	for (const auto& item : deco.HISM)
	{
		UE_LOG(LogTemp, Warning, TEXT("HISM: %s"), *item.ToString());
	}
}

/**
 * 在游戏线程上执行并等待结果
 * 1. 当bWait == true
 * ```
 * {
 *	// 调用线程会在此处等待，直到游戏线程执行完Function
 *	RunOnGameThread([...]() {
 		// DoSomething
 *	}, true);
 * 
 *  这里的代码只有在Function执行完成后才会运行
 *  可以安全地使用Function中设置的数据
 *  }
 *
 *  2. 当bWait == false
 *  {
 * 	  //提交任务后立即返回
 *	  RunOnGameThread([...]() {
 *	 	// 在游戏线程上执行，但调用线程不会等待
 *	  }, false);
 *	
 *	 这段代码会立即执行，不会等待Function完成
 *	 不能依赖Function中可能修改的数据
 *	}
 */
void RunOnGameThread(TFunction<void()> Function, bool bWait = false)
{
	if (IsInGameThread())
	{
		// 已经在游戏线程上，直接执行
		Function();
		return;
	}

	if (bWait)
	{
		// 使用新的API创建事件
		FEvent* DoneEvent = FPlatformProcess::GetSynchEventFromPool();
        
		// 在游戏线程上执行
		AsyncTask(ENamedThreads::GameThread, [Function, DoneEvent]() {
			Function();
			DoneEvent->Trigger(); // 完成后通知等待线程
		});

		// 等待完成
		DoneEvent->Wait();
		// 使用新的API归还事件
		FPlatformProcess::ReturnSynchEventToPool(DoneEvent);
	}
	else
	{
		// 异步执行，不等待
		AsyncTask(ENamedThreads::GameThread, Function);
	}
}

void ABuildGridMapGameMode::IntervalMapSaveToFile(const FGridMapSave& InMapSave)
{
	FString JsonString;
	FJsonObjectConverter::UStructToJsonObjectString(InMapSave, JsonString);

	auto Settings = GetDefault<UGridPathFindingSettings>();
	FString SavePath = FPaths::ProjectContentDir() / Settings->MapSaveFolder / (InMapSave.MapName.ToString() + TEXT("_Map.txt"));
	FFileHelper::SaveStringToFile(JsonString, *SavePath);
}

void ABuildGridMapGameMode::SaveEditingMapSave(EBuildGridMapSaveMode InSaveMode)
{
	if (!HasValidMapSave)
	{
		return;
	}

	if (bIsSaving)
	{
		// Todo: 碰到了一次 保存完毕后，SaveState仍为Saving的情况， 暂时不知道怎么复现
		GEngine->AddOnScreenDebugMessage(-1, 5.f, FColor::Red,
			TEXT("正在保存地图数据，请稍后再试"));
		return;
	}
	
	// FGridMapSave数据较为简单， 反复保存问题不大
	// 序列为Json
	IntervalMapSaveToFile(EditingMapSave);
	OnSaveStart.Broadcast(InSaveMode);

	if (GridMapModel->GetMapConfig()->DrawMode != EGridMapDrawMode::BaseOnRowColumn)
	{
		// Todo: 暂不支持其它格式的地图保存， 其它模式的分区功能要额外实现
		OnSaveOver.Broadcast();
		return;
	}
	
	FString ChunkDir = GetChunksRootDir();
				
	// Todo: 当切换地图、更改地图的行列参数时， 地图的Chunk会发生改变， 此时需要进行一次FullSave
	switch (InSaveMode) {
		case EBuildGridMapSaveMode::FullSave:
			{
				// 改进的安全保存流程
				// 1. 创建Temp文件夹，生成新的数据
				// 2. 创建备份：将当前Chunks文件夹重命名为Backup
				// 3. 将Temp文件夹重命名为正式的Chunks文件夹
				// 4. 删除备份文件夹
				// 5. 实现崩溃恢复：启动时检查是否存在Temp或Backup文件夹，如有则进行恢复
				if (ProgressUpdateTimerHandle.IsValid())
				{
					ProgressUpdateTimerHandle.Invalidate();
				}
	
				GetWorldTimerManager().SetTimer(ProgressUpdateTimerHandle, this, 
					&ABuildGridMapGameMode::UpdateSaveProgressUI, 0.1f, true);

				check(FPaths::DirectoryExists(ChunkDir));
				const FString TempDir = ChunkDir + TEXT("_Temp");
				const FString BackupDir = ChunkDir + TEXT("_Backup");
				// UI出现Mask(窗口监听事件)， 并禁止其它所有键盘操作(PC监听事件)， 更新全部格子数据, 通过专门的按钮触发, 无论如何都会保存
				IFileManager::Get().MakeDirectory(*TempDir, true);
				// MakeChunkData
				// 创建共享状态对象
				struct FSaveState
				{
					// 保存每个区块的数据
					TMap<int32, FGridMapTilesSave> ChunkTilesMap;
    
					// 当前正在处理的元素索引
					int32 ProcessedCount = 0;
					// 使用指针而不是迭代器
					TMap<FHCubeCoord, FSerializableTile>* EditingTilesPtr = nullptr;

					// 与主线程共享的进度对象
					TSharedPtr<FSaveProgress> Progress;
				};

				CurrentSaveProgress = MakeShared<FSaveProgress>();
				TSharedPtr<FSaveState> SaveStatePtr = MakeShared<FSaveState>();
				SaveStatePtr->EditingTilesPtr = &EditingTiles; // 设置指针指向编辑中的格子
				SaveStatePtr->Progress = CurrentSaveProgress;

				Async(EAsyncExecution::TaskGraph, [this, SaveStatePtr, TempDir, BackupDir, ChunkDir]
				{
					const int32 BatchSize = 1000; // 每批处理量
					const auto& EditingTiles = *(SaveStatePtr->EditingTilesPtr);

					TArray<FHCubeCoord> Keys;
					EditingTiles.GetKeys(Keys);

					while (SaveStatePtr->ProcessedCount < Keys.Num())
					{
						float ProgressPercent = (float)SaveStatePtr->ProcessedCount / Keys.Num();
						SaveStatePtr->Progress->Progress = ProgressPercent /0.2f; // 0-20%范围
						SaveStatePtr->Progress->StatusMessage = FString::Printf(TEXT("格子数据分区: %d/%d"),
							SaveStatePtr->ProcessedCount, Keys.Num());
						SaveStatePtr->Progress->bIsDirty = true;
						
						// 处理一批数据
						int32 EndIdx = FMath::Min(SaveStatePtr->ProcessedCount + BatchSize, Keys.Num());

						for (int32 i = SaveStatePtr->ProcessedCount; i < EndIdx; ++i)
						{
							const FHCubeCoord& Coord = Keys[i];
							const FSerializableTile& Tile = EditingTiles[Coord];

							// 获取区块索引
							int32 ChunkIndex = GridMapModel->StableGetCoordChunkIndex(Coord);

							// 添加到对应区块
							if (!SaveStatePtr->ChunkTilesMap.Contains(ChunkIndex))
							{
								FGridMapTilesSave NewChunkSave;
								// Todo: 暂时未使用数据版本号。
								NewChunkSave.Version = 1;
								SaveStatePtr->ChunkTilesMap.Add(ChunkIndex, NewChunkSave);
							}
							SaveStatePtr->ChunkTilesMap[ChunkIndex].GridTiles.Add(Tile);
						}

						SaveStatePtr->ProcessedCount = EndIdx;
						// 暂停片刻，让出CPU时间
						FPlatformProcess::Sleep(0.001f);
					}

					if (DebugSave)
					{
						// 增加Sleep , 测试功能
						FPlatformProcess::Sleep(1.f);
					}
					
					// 数据分区已经完成， 序列化保存
					SaveStatePtr->Progress->TotalTaskCount = SaveStatePtr->ChunkTilesMap.Num();
					TArray<TFuture<void>> SaveTasks;
					for (const auto& ChunkPair : SaveStatePtr->ChunkTilesMap)
					{
						const int32 ChunkIndex = ChunkPair.Key;
						const FGridMapTilesSave& ChunkSave = ChunkPair.Value;

						// 序列化数据
						FString ChunkFilePath = TempDir / FString::Printf(TEXT("Chunk_%d.bin"), ChunkIndex);
						
						// 保存到Temp目录
						SaveTasks.Add(Async(EAsyncExecution::ThreadPool, [SaveStatePtr, ChunkSave, ChunkFilePath]()
						{
							bool SaveSuccess = UGridPathFindingBlueprintFunctionLib::SaveGridMapTilesToFile(ChunkSave, ChunkFilePath);
							if (!SaveSuccess)
							{
								UE_LOG(LogTemp, Error, TEXT("[FullSave] Failed to save tiles to file,ChunkIndex: %s"), *ChunkFilePath);
							}
							// 线程安全地更新进度
							int32 Completed = SaveStatePtr->Progress->CompletedTasks.Increment();
							int32 Total = SaveStatePtr->Progress->TotalTaskCount;

							// 使用互斥锁保护对非原子类型的修改
							FScopeLock Lock(&SaveStatePtr->Progress->ProgressLock);
							SaveStatePtr->Progress->Progress = 0.2f + (0.7f * Completed / Total); // 20%-90%范围
							SaveStatePtr->Progress->StatusMessage = FString::Printf(TEXT("保存临时文件: %d/%d"),
								Completed, Total);
							SaveStatePtr->Progress->bIsDirty = true;
						}));
					}

					
					if (DebugSave)
					{
						// 增加Sleep , 测试功能
						FPlatformProcess::Sleep(1.f);
					}

					for (auto& Task : SaveTasks)
					{
						Task.Wait();
					}

					
					if (DebugSave)
					{
						// 增加Sleep , 测试功能
						FPlatformProcess::Sleep(1.f);
					}
					
					// 将ChunkDir改名为BackupDir
					IFileManager::Get().Move(*BackupDir, *ChunkDir);
					SaveStatePtr->Progress->Progress = 0.92f;
					SaveStatePtr->Progress->StatusMessage = TEXT("保存备份");
					SaveStatePtr->Progress->bIsDirty = true;
					
					if (DebugSave)
					{
						// 增加Sleep , 测试功能
						FPlatformProcess::Sleep(1.f);
					}
					
					// 将TempDir改名为ChunkDir
					IFileManager::Get().Move(*ChunkDir, *TempDir);
					SaveStatePtr->Progress->Progress = 0.95f;
					SaveStatePtr->Progress->StatusMessage = TEXT("保存完成");
					SaveStatePtr->Progress->bIsDirty = true;
					
					if (DebugSave)
					{
						// 增加Sleep , 测试功能
						FPlatformProcess::Sleep(1.f);
					}

					// 删除BackupDir
					IFileManager::Get().DeleteDirectory(*BackupDir, false, true);
					SaveStatePtr->Progress->Progress = 1.0f;
					SaveStatePtr->Progress->StatusMessage = TEXT("删除备份");
					SaveStatePtr->Progress->bIsDirty = true;
					
					if (DebugSave)
					{
						// 增加Sleep , 测试功能
						FPlatformProcess::Sleep(1.f);
					}
					// 返回游戏线程完成保存
					AsyncTask(ENamedThreads::GameThread, [this]()
					{
						OnSaveOver.Broadcast();
					});
				});
			}
			break;
		case EBuildGridMapSaveMode::IncrementalSave:
			{
				// Ctrl + S 保存 和 经过一定间隔后进行自动保存， 使用增量保存， 只更新那些发生了数据变化的格子
				if (DirtyChunks.Num() == 0)
				{
					OnSaveOver.Broadcast();
					return;
				}

				// 只重新DirtyCoords中的格子
				// 流程:
				// 1. 创建Temp文件夹，和 Backup文件夹
				// 2. 将需要更新的Chunk数据保存到Temp文件夹中 
				// 3. 将需要替换的Chunk数据移动到Backup文件夹中
				// 4. 将Temp文件夹中的数据移动到Chunk文件夹中
				// 5. 删除Temp文件夹和Backup文件夹

				auto DirtyChunksCopy = DirtyChunks;
				DirtyChunks.Empty();
				Async(EAsyncExecution::TaskGraph, [this, DirtyChunksCopy, ChunkDir]
				{
					FString TempDir = ChunkDir + TEXT("/Temp");
					FString BackupDir = ChunkDir + TEXT("/Backup");

					// 创建临时文件夹
					IFileManager::Get().MakeDirectory(*TempDir);
					IFileManager::Get().MakeDirectory(*BackupDir);

					TArray<TFuture<void>> SaveTasks;
					for (int32 ChunkIndex : DirtyChunksCopy)
					{
						FString ChunkFileName = FString::Printf(TEXT("Chunk_%d.bin"), ChunkIndex);
						FString TempFile = TempDir / ChunkFileName;
						// 创建区块数据
						FGridMapTilesSave ChunkSave;
						// 不去保护Save过程中又更新了格子数据的情况， 直接去拿当前的Tile数据
						// 线程安全地获取需要保存的数据
						RunOnGameThread([this, ChunkIndex, &ChunkSave]()
						{
							// 在游戏线程中收集属于该区块的所有格子数据
							TArray<FHCubeCoord> InChunkCoords;
							GridMapModel->StableGetChunkCoords(ChunkIndex, InChunkCoords);
							for (const auto& Coord : InChunkCoords)
							{
								auto Tile = EditingTiles.Find(Coord);
								if (Tile)
								{
									ChunkSave.GridTiles.Add(*Tile);
								}
							}
						}, true); // 同步等待

						// 保存到Temp目录
						SaveTasks.Add(Async(EAsyncExecution::ThreadPool, [ChunkSave, TempFile]()
						{
							bool SaveSuccess = UGridPathFindingBlueprintFunctionLib::SaveGridMapTilesToFile(ChunkSave, TempFile);
							if (!SaveSuccess)
							{
								UE_LOG(LogGridPathFinding, Error, TEXT("[FullSave] Failed to save tiles to file,ChunkIndex: %s"), *TempFile);
							}
						}));
					}

					for (auto& Task : SaveTasks)
					{
						Task.Wait();
					}

					// 将需要替换的Chunk数据移动到Backup文件夹中
					for (int32 ChunkIndex : DirtyChunksCopy)
					{
						FString ChunkFileName = FString::Printf(TEXT("Chunk_%d.bin"), ChunkIndex);
						FString ChunkFilePath = ChunkDir / ChunkFileName;
						// 检查是否存在Chunk文件
						if (!FPaths::FileExists(ChunkFilePath))
						{
							UE_LOG(LogGridPathFinding, Warning, TEXT("Chunk文件不存在: %s"), *ChunkFilePath);
							continue;
						}
						FString BackupFile = BackupDir / ChunkFileName;
						IFileManager::Get().Move(*BackupFile, *ChunkFilePath);
					}

					// 将Temp文件夹中的数据移动到Chunk文件夹中
					TArray<FString> TempFiles;
					IFileManager::Get().FindFiles(TempFiles, *TempDir, TEXT("*.bin"));
					for (const auto& TempFile : TempFiles)
					{
						FString TempFilePath = TempDir / TempFile;
						FString ChunkFilePath = ChunkDir / TempFile;
						IFileManager::Get().Move(*ChunkFilePath, *TempFilePath);
					}

					// 删除Temp文件夹和Backup文件夹
					IFileManager::Get().DeleteDirectory(*TempDir, false, true);
					IFileManager::Get().DeleteDirectory(*BackupDir, false, true);

					// 返回游戏线程完成保存
					AsyncTask(ENamedThreads::GameThread, [this]()
					{
						OnSaveOver.Broadcast();
					});
				});
			}
			break;
		default:
			UE_LOG(LogTemp, Error, TEXT("未处理的保存模式: %s"), *UEnum::GetValueAsString(InSaveMode));
			break;
	}
}

void ABuildGridMapGameMode::UpdateSaveProgressUI()
{
	if (!CurrentSaveProgress || !CurrentSaveProgress->bIsDirty)
		return;
	
	OnSaveProgressUpdated.Broadcast(CurrentSaveProgress->Progress, CurrentSaveProgress->StatusMessage);
	CurrentSaveProgress->bIsDirty = false;
    
	// 当保存完成时停止定时器
	if (!bIsSaving)
	{
		// 清除CurrentSaveProgress
		CurrentSaveProgress.Reset();
		GetWorldTimerManager().ClearTimer(ProgressUpdateTimerHandle);
	}
}

void ABuildGridMapGameMode::DataRecover()
{
	// 经测试， 我在SaveEditingMapSave进行全量保存时，
	// 当我执行完 将ChunkDir改名为BackupDir这一步骤时， 终止程序， UE会自动将文件夹数据还原， 这是什么原理？
	// 但是目前看来在子线程上进行的磁盘读写操作，在发生错误后， UE会自动恢复

	// 如果理解正确的话， 文件夹中不可能出现Temp和Backup文件夹
	// Todo: 增加一个check
}

void ABuildGridMapGameMode::MarkEditingTilesDirty(const FHCubeCoord& InDirtyCoord)
{
	// 只记录需要更新的Chunk
	auto ChunkIndex = GridMapModel->StableGetCoordChunkIndex(InDirtyCoord);
	if (ChunkIndex == INDEX_NONE)
	{
		return;
	}
	DirtyChunks.Add(ChunkIndex);
}

void ABuildGridMapGameMode::MarkEditingTilesDirty(const TArray<FHCubeCoord>& InDirtyCoords)
{
	for (const auto& DirtyCoord : InDirtyCoords)
	{
		MarkEditingTilesDirty(DirtyCoord);
	}
}

void ABuildGridMapGameMode::CreateGridMapSave(FName InMapName)
{
	auto NewMapSave = FGridMapSave{};
	NewMapSave.MapName = InMapName;
	NewMapSave.CreateTime = FDateTime::Now();
	// 在创建FGridMapSave时会创建一个唯一的文件夹与之对应， 并且不会被改变
	// 创建唯一文件夹名称
	NewMapSave.ChunksDir = FGuid::NewGuid().ToString();
	// 创建文件夹
	auto Settings = GetDefault<UGridPathFindingSettings>();
	FString ChunkDir = FPaths::ProjectContentDir() / Settings->MapSaveFolder / NewMapSave.ChunksDir;

	check(!FPaths::DirectoryExists(ChunkDir));
	IFileManager::Get().MakeDirectory(*ChunkDir, true);
	
	IntervalMapSaveToFile(NewMapSave);
	
	OnCreateGridMapSave.Broadcast();
}

void ABuildGridMapGameMode::SwitchEditingMapSave(const FGridMapSave& InMapSave)
{
	// 切换地图后，清除命令历史
	CommandManager->ClearCommandHistory();
	
	if (HasValidMapSave)
	{
		// 移除当前地图内容
		EditingTiles.Empty();
		DirtyChunks.Empty();
	}
	
	EditingMapSave = InMapSave;
	HasValidMapSave = EditingMapSave.MapName != TEXT("SP_Empty");

	if (!HasValidMapSave)
	{
		GridMapModel->BuildTilesData(EditingMapSave.MapConfig, EditingTiles);
		BuildGridMapRenderer->ClearGridMap();
		
		OnSwitchEditingMapSave.Broadcast();
		return;
	}

	BuildGridMapWindow->SetCanInput(false);
	auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
	PC->SetCanInput(false);

	BuildGridMapRenderer->OnRenderOver.AddLambda([this]()
	{
		BuildGridMapWindow->SetCanInput(true);
		auto PC = Cast<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController());
		PC->SetCanInput(true);
		BuildGridMapRenderer->OnRenderOver.RemoveAll(this);
	});

	// Todo: 卡顿的话这里也需要修改成异步的
	// 加载Chunk中的格子数据
	auto ChunkDir = GetChunksRootDir();
	check(FPaths::DirectoryExists(ChunkDir));
	TArray<FString> ChunkFiles;
	IFileManager::Get().FindFiles(ChunkFiles, *ChunkDir, TEXT("*.bin"));
	for (const auto& ChunkFile : ChunkFiles)
	{
		FString ChunkFilePath = ChunkDir / ChunkFile;
		FGridMapTilesSave ChunkSave;
		if (UGridPathFindingBlueprintFunctionLib::LoadGridMapTilesFromFile(ChunkFilePath, ChunkSave))
		{
			for (const auto& Tile : ChunkSave.GridTiles)
			{
				EditingTiles.Add(Tile.Coord, Tile);
			}
		}
	}
	
	// 加载正在编辑的地图
	GridMapModel->BuildTilesData(EditingMapSave.MapConfig, EditingTiles);
	BuildGridMapRenderer->RenderGridMap();
	
	OnSwitchEditingMapSave.Broadcast();
}

void ABuildGridMapGameMode::OnMapNameTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	if (!HasValidMapSave)
	{
		return;
	}
	
	switch (CommitMethod) {
		case ETextCommit::OnEnter:
		case ETextCommit::OnUserMovedFocus:
			{
				auto NewName = FName(*Text.ToString());
				auto ChangeMapNameCommand = NewObject<UBuildGridMapChangeMapNameCommand>(CommandManager);
				ChangeMapNameCommand->Initialize(EditingMapSave.MapName, NewName);
				CommandManager->ExecuteCommand(ChangeMapNameCommand);
			}
		break;
		default:
			break;
	}
}

void ABuildGridMapGameMode::OnMapTypeSelectionChanged(EGridMapType InMapType)
{
	if (!HasValidMapSave)
	{
		return;
	}
	
	auto ChangeMapTypeCommand = NewObject<UBuildGridMapChangeMapTypeCommand>(CommandManager);
	ChangeMapTypeCommand->Initialize(EditingMapSave.MapConfig.MapType, InMapType);
	CommandManager->ExecuteCommand(ChangeMapTypeCommand);
}

void ABuildGridMapGameMode::OnMapDrawModeSelectionChanged(EGridMapDrawMode InDrawMode)
{
}

void ABuildGridMapGameMode::OnHexTileOrientationSelectionChanged(ETileOrientationFlag InTileOrientation)
{
}

void ABuildGridMapGameMode::OnMapRowTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
}

void ABuildGridMapGameMode::OnMapColumnTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Todo: 修改地图尺寸时， 需要裁剪EditingTiles中的数据, 剔除那些超出范围的格子
}

void ABuildGridMapGameMode::OnMapRadiusTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	// Todo: 修改地图尺寸时， 需要裁剪EditingTiles中的数据, 剔除那些超出范围的格子
}

void ABuildGridMapGameMode::OnGridSizeXTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	switch (CommitMethod)
	{
		case ETextCommit::OnEnter:
		case ETextCommit::OnUserMovedFocus:
			{
				auto NewSizeX = FCString::Atod(*Text.ToString());
				auto SizeY = EditingMapSave.MapConfig.GetGridSize().Y;
				IntervalChangeTileSize(NewSizeX, SizeY);
			}
		break;
		default:
			break;
	}
}

void ABuildGridMapGameMode::OnGridSizeYTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	switch (CommitMethod)
	{
		case ETextCommit::OnEnter:
		case ETextCommit::OnUserMovedFocus:
			{
				auto NewSizeY = FCString::Atod(*Text.ToString());
				auto SizeX = EditingMapSave.MapConfig.GetGridSize().X;
				IntervalChangeTileSize(SizeX, NewSizeY);
			}
		break;
		default:
			break;
	}
}

void ABuildGridMapGameMode::OnGridRadiusTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	switch (CommitMethod)
	{
		case ETextCommit::OnEnter:
		case ETextCommit::OnUserMovedFocus:
			{
				auto NewRadius = FCString::Atod(*Text.ToString());
				IntervalChangeTileSize(NewRadius, NewRadius);
			}
		break;
		default:
			break;
	}
}

void ABuildGridMapGameMode::OnTileEnvSelectionChanged(TObjectPtr<UGridEnvironmentType> NewGridEnvironment)
{
	auto SelectedCoord = GetSelectedCoord();
	auto ChangeEnvCommand = NewObject<UBuildGridMapChangeTileEnvCommand>(CommandManager);
	bool bHasSelected = EditingTiles.Contains(SelectedCoord);
	FName OldEnvTypeID = bHasSelected ? EditingTiles[SelectedCoord].EnvironmentType : UGridEnvironmentType::EmptyEnvTypeID;
	FName NewEnvTypeID = NewGridEnvironment == nullptr? UGridEnvironmentType::EmptyEnvTypeID : NewGridEnvironment->TypeID;
	ChangeEnvCommand->Initialize(SelectedCoord, !bHasSelected, OldEnvTypeID, NewEnvTypeID);
	CommandManager->ExecuteCommand(ChangeEnvCommand);
}

void ABuildGridMapGameMode::OnEnvTextureIndexTextCommitted(const FText& Text, ETextCommit::Type CommitMethod)
{
	switch (CommitMethod) {
		case ETextCommit::OnEnter:
		case ETextCommit::OnUserMovedFocus:
			{
				// Todo: 目前只考虑了选中一个格子的情况， 增加多选情况需要调整代码
				auto SelectedCoord = GetSelectedCoord();
				
				if (!EditingTiles.Contains(SelectedCoord))
				{
					UE_LOG(LogGridPathFinding, Error, TEXT("[%s]空地块不支持修改纹理索引"), *SelectedCoord.ToString());
					return;
				}
				
				auto ChangeTextureIndexCommand = NewObject<UBuildGridMapChangeTileEnvTextureCommand>(CommandManager);
				ChangeTextureIndexCommand->Initialize(SelectedCoord,
				                                      EditingTiles[SelectedCoord].TileEnvData.TextureIndex,
				                                      FCString::Atoi(*Text.ToString()));
				CommandManager->ExecuteCommand(ChangeTextureIndexCommand);
			}
			break;
		default:
			break;
	}
}

void ABuildGridMapGameMode::IntervalChangeTileSize(double InSizeX, double InSizeY)
{
	auto Command = NewObject<UBuildGridMapChangeTileSizeCommand>(CommandManager);
	auto OldSize = EditingMapSave.MapConfig.GetGridSize();
	Command->Initialize(OldSize, FVector2D(InSizeX, InSizeY));
	CommandManager->ExecuteCommand(Command);
}

FHCubeCoord ABuildGridMapGameMode::GetSelectedCoord()
{
	return CastChecked<ABuildGridMapPlayerController>(GetWorld()->GetFirstPlayerController())->
		GetFirstSelectedCoord();
}

FString ABuildGridMapGameMode::GetChunksRootDir()
{
	return FPaths::ProjectContentDir() / GetDefault<UGridPathFindingSettings>()->MapSaveFolder / EditingMapSave.
		ChunksDir;
}
