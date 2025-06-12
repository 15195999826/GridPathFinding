#pragma once
#include "IDetailCustomization.h"

class FTokenActorDetails : public IDetailCustomization
{
public:
	// 创建实例
	static TSharedRef<IDetailCustomization> MakeInstance();

	// 自定义详情面板
	virtual void CustomizeDetails(IDetailLayoutBuilder& DetailBuilder) override;

private:
	// 导出JSON按钮点击处理
	FReply OnExportJsonClicked();

	// 导入JSON按钮点击处理
	FReply OnImportJsonClicked();

	// 保存当前选中的TokenActor
	TWeakObjectPtr<class ATokenActor> SelectedTokenActor;

	bool IsTokenActorValid() const;
};
