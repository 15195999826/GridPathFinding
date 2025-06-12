// Fill out your copyright notice in the Description page of Project Settings.


#include "BuildGridMap/Command/BuildGridMapChangeTileEnvCommand.h"

#include "GridMapModel.h"
#include "GridPathFinding.h"
#include "BuildGridMap/BuildGridMapGameMode.h"
#include "BuildGridMap/BuildGridMapRenderer.h"
#include "BuildGridMap/UI/BuildGridMapTileConfigWidget.h"
#include "BuildGridMap/UI/BuildGridMapWindow.h"
#include "Types/SerializableTile.h"

void UBuildGridMapChangeTileEnvCommand::Initialize(const FHCubeCoord& InCoord, bool InNeedNewTile,
                                                   const FName& InOldTileEnv, const FName& InNewTileEnv)
{
	Coord = InCoord;
	NewTileEnv = InNewTileEnv;
	OldTileEnv = InOldTileEnv;
	NeedNewTile = InNeedNewTile;
}

bool UBuildGridMapChangeTileEnvCommand::Execute()
{
	auto GM = GetWorld()->GetAuthGameMode<ABuildGridMapGameMode>();
	
	// 数据层更新
	if (NeedNewTile)
	{
		// 创建新的需要记录的地块
		auto NewTile = FSerializableTile();
		NewTile.Coord = Coord;
		NewTile.TileEnvData.EnvironmentType = NewTileEnv;
		GM->GetMutEditingTiles().Add(Coord, NewTile);
		// 通知GridModel增加了新的Tile， 由GridModel触发渲染层的更新
		GM->GridMapModel->UpdateTileEnv(NewTile);
	}
	else
	{
		// 如果设置为空地块，那么移除该格子
		if (NewTileEnv == UGridEnvironmentType::EmptyEnvTypeID)
		{
			const auto SerializableTile = GM->GetEditingTiles()[Coord];
			GM->GetMutEditingTiles().Remove(Coord);
			// 同时重置UI上输入框的数据
			// 重置输入框内容， 当环境设置为空时， 数据上删除了该格子的数据
			GM->GetBuildGridMapWindow()->TileConfigWidget->BindSingleTileCoord(Coord);
			// 通知GridModel移除了格子
			GM->GridMapModel->UpdateTileEnv(SerializableTile);
		}
		else
		{
			GM->GetMutEditingTiles()[Coord].TileEnvData.EnvironmentType = NewTileEnv;
			// 通知GirdModel更新了格子
			GM->GridMapModel->UpdateTileEnv(GM->GetEditingTiles()[Coord]);
		}
	}
	
	// 更新Renderer的渲染， Todo: 看起来编辑器中总是不需要处理GridModel的， 因为编辑器中不需要考虑寻路
	GM->MarkEditingTilesDirty(Coord);
	return true;
}

bool UBuildGridMapChangeTileEnvCommand::Undo()
{
	return true;
}

FString UBuildGridMapChangeTileEnvCommand::GetDescription() const
{
	return FString::Printf(TEXT("修改地块 %s（从 %s 到 %s）"),
		*Coord.ToString(),
		*OldTileEnv.ToString(), 
		*NewTileEnv.ToString());
}
