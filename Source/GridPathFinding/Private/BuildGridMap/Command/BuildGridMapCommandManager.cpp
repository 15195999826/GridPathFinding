// Fill out your copyright notice in the Description page of Project Settings.

#include "BuildGridMap/Command/BuildGridMapCommandManager.h"

#include "GridPathFinding.h"

UBuildGridMapCommandManager::UBuildGridMapCommandManager()
{
	CurrentCommandIndex = -1;
	MaxHistoryCount = 20; // 默认最大历史记录数量为20
}

bool UBuildGridMapCommandManager::ExecuteCommand(TScriptInterface<IGridMapCommand> Command)
{
	UE_LOG(LogGridPathFinding, Log, TEXT("[BuildGridMapCommandManager.ExecuteCommand] %s"), *Command.GetObject()->GetName());
	if (!Command)
	{
		return false;
	}

	// 执行命令
	const bool bExecuteSuccess = Command->Execute();
	if (!bExecuteSuccess)
	{
		return false;
	}

	// 如果当前有已撤销的命令，需要清除掉
	if (CurrentCommandIndex < CommandHistory.Num() - 1)
	{
		CommandHistory.RemoveAt(CurrentCommandIndex + 1, CommandHistory.Num() - CurrentCommandIndex - 1);
	}

	// 添加到历史记录
	CommandHistory.Add(Command);
	CurrentCommandIndex = CommandHistory.Num() - 1;

	// 检查并确保历史记录数量不超过最大限制
	EnforceHistoryLimit();

	return true;
}

bool UBuildGridMapCommandManager::UndoCommand()
{
	if (!CanUndo())
	{
		return false;
	}

	// 执行撤销
	const bool bUndoSuccess = CommandHistory[CurrentCommandIndex]->Undo();
	if (bUndoSuccess)
	{
		CurrentCommandIndex--;
	}

	UE_LOG(LogGridPathFinding, Log, TEXT("[BuildGridMapCommandManager.UndoCommand] succeed: %d"), bUndoSuccess);
	return bUndoSuccess;
}

bool UBuildGridMapCommandManager::RedoCommand()
{
	if (!CanRedo())
	{
		return false;
	}

	// 执行重做（重新执行命令）
	const bool bRedoSuccess = CommandHistory[CurrentCommandIndex + 1]->Execute();
	if (bRedoSuccess)
	{
		CurrentCommandIndex++;
	}

	UE_LOG(LogGridPathFinding, Log, TEXT("[BuildGridMapCommandManager.RedoCommand] succeed: %d"), bRedoSuccess);
	return bRedoSuccess;
}

bool UBuildGridMapCommandManager::CanUndo() const
{
	return CurrentCommandIndex >= 0;
}

bool UBuildGridMapCommandManager::CanRedo() const
{
	return CurrentCommandIndex < CommandHistory.Num() - 1;
}

void UBuildGridMapCommandManager::ClearCommandHistory()
{
	CommandHistory.Empty();
	CurrentCommandIndex = -1;
}

int32 UBuildGridMapCommandManager::GetUndoCommandCount() const
{
	return CurrentCommandIndex + 1;
}

int32 UBuildGridMapCommandManager::GetRedoCommandCount() const
{
	return CommandHistory.Num() - CurrentCommandIndex - 1;
}

void UBuildGridMapCommandManager::SetMaxHistoryCount(int32 InMaxHistoryCount)
{
	MaxHistoryCount = FMath::Max(0, InMaxHistoryCount); // 确保最大历史记录数量不为负数
	EnforceHistoryLimit(); // 设置新限制后，立即检查当前历史记录
}

int32 UBuildGridMapCommandManager::GetMaxHistoryCount() const
{
	return MaxHistoryCount;
}

FString UBuildGridMapCommandManager::GetHistoryCommandInfo() const
{
	FString Ret = TEXT("Command History:\n");

	for (int32 i = 0; i < CommandHistory.Num(); ++i)
	{
		FString CommandName = TEXT("Unknown Command");
		if (CommandHistory[i])
		{
			CommandName = CommandHistory[i]->GetDescription();
		}

		if (i == CurrentCommandIndex)
		{
			Ret += FString::Printf(TEXT("[%d] -> %s(Current)\n"), i, *CommandName);
		}
		else
		{
			Ret += FString::Printf(TEXT("[%d] %s()\n"), i, *CommandName);
		}
	}

	if (CommandHistory.Num() == 0)
	{
		Ret += TEXT("No commands executed yet.");
	}

	return Ret;
}

void UBuildGridMapCommandManager::EnforceHistoryLimit()
{
	// 如果MaxHistoryCount为0，表示不限制历史记录数量
	if (MaxHistoryCount <= 0)
	{
		return;
	}

	// 如果历史记录数量超过最大限制
	const int32 ExcessCount = CommandHistory.Num() - MaxHistoryCount;
	if (ExcessCount > 0)
	{
		// 移除最早的历史记录
		CommandHistory.RemoveAt(0, ExcessCount);
		// 更新当前命令索引
		CurrentCommandIndex = FMath::Max(0, CurrentCommandIndex - ExcessCount);
	}
}
