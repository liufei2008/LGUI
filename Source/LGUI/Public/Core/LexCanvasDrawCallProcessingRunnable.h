// Copyright 2019-Present LexLiu. All Rights Reserved.

#pragma once
#include "LexCanvasProcessingDrawCallData.h"

class FLexCanvasDrawCallProcessingRunnable : public FRunnable
{
public:
	FLexCanvasDrawCallProcessingRunnable(const FString& InDebugName);
	void Start();

	//~ Begin FRunnable Interface
	virtual uint32 Run() override;
	virtual void Stop() override;
	virtual void Exit() override;
	//~ End FRunnable Interface
	bool IsBatching()const { return bIsBatching; }

	void PushPreparedDrawCallData(FLexCanvasPreparedDrawCallData InData);
	bool TryGetDrawCallData(FLexCanvasPendingDrawCallData& OutData);

private:
	TSharedPtr<TQueue<FLexCanvasPreparedDrawCallData>> PreparedDrawCallDataQueue;
	TSharedPtr<TQueue<FLexCanvasPendingDrawCallData>> PendingRebuildDrawCallQueue;
	FEvent* PreparedDrawCallDataQueueEvent = nullptr;
	
	std::atomic<bool> bIsRunning = false;
	std::atomic<bool> bIsBatching = false;
	TUniquePtr<FRunnableThread> Thread;
	FString DebugName;
};
