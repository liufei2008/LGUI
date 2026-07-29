// Copyright 2019-Present LexLiu. All Rights Reserved.

#include "Core/LexCanvasDrawCallProcessingRunnable.h"

#include "Core/Components/LexCanvas.h"

FLexCanvasDrawCallProcessingRunnable::FLexCanvasDrawCallProcessingRunnable(const FString& InDebugName)
{
	DebugName = InDebugName;
}

void FLexCanvasDrawCallProcessingRunnable::Start()
{
	check (!bIsRunning);
	check (PreparedDrawCallDataQueue == nullptr);
	check (PreparedDrawCallDataQueueEvent == nullptr);
	PreparedDrawCallDataQueue = MakeShared<TQueue<FLexCanvasPreparedDrawCallData>>();
	PreparedDrawCallDataQueueEvent = FPlatformProcess::GetSynchEventFromPool();
	PendingRebuildDrawCallQueue = MakeShared<TQueue<FLexCanvasPendingDrawCallData>>();
	bIsRunning = true;
	bIsBatching = false;
	Thread.Reset(FRunnableThread::Create(this, TEXT("FLexCanvasDrawCallProcessingRunnable"), 0, TPri_Normal));
}

uint32 FLexCanvasDrawCallProcessingRunnable::Run()
{
	while (bIsRunning)
	{
		auto QueueEvent = PreparedDrawCallDataQueueEvent;
		if (QueueEvent == nullptr || !PreparedDrawCallDataQueue.IsValid() || !PendingRebuildDrawCallQueue.IsValid())
		{
			break;
		}

		if (QueueEvent->Wait(500))
		{
			if (!bIsRunning)break;
			bIsBatching = true;
			FLexCanvasPreparedDrawCallData PreparedDrawCallData;
			while (PreparedDrawCallDataQueue->Dequeue(PreparedDrawCallData)){}//discard old data and get the newest one
			FLexCanvasPendingDrawCallData PendingDrawCallData;
			PendingDrawCallData.FrameNumber = PreparedDrawCallData.FrameNumber;
			ULexCanvas::BatchDrawCallAsync(PreparedDrawCallData.LeftBottomPoint, PreparedDrawCallData.RightTopPoint, PreparedDrawCallData.DataArray, PendingDrawCallData.DrawCallArray);
			//push to main thread queue
			PendingRebuildDrawCallQueue->Enqueue(MoveTemp(PendingDrawCallData));
			bIsBatching = false;
		}
	}

	return 0;
}
void FLexCanvasDrawCallProcessingRunnable::Stop()
{
	if (!bIsRunning)
	{
		return;
	}

	bIsRunning = false;
	if (PreparedDrawCallDataQueueEvent != nullptr)
	{
		PreparedDrawCallDataQueueEvent->Trigger();
	}
	if (Thread.IsValid())
	{
		Thread->WaitForCompletion();
		Thread.Reset();
	}
	if (PreparedDrawCallDataQueueEvent != nullptr)
	{
		FPlatformProcess::ReturnSynchEventToPool(PreparedDrawCallDataQueueEvent);
		PreparedDrawCallDataQueueEvent = nullptr;
	}
	PreparedDrawCallDataQueue.Reset();
	PendingRebuildDrawCallQueue.Reset();
	bIsBatching = false;
}
void FLexCanvasDrawCallProcessingRunnable::Exit()
{
	Stop();
}

void FLexCanvasDrawCallProcessingRunnable::PushPreparedDrawCallData(FLexCanvasPreparedDrawCallData InData)
{
	if (!bIsRunning || !PreparedDrawCallDataQueue.IsValid() || PreparedDrawCallDataQueueEvent == nullptr)
	{
		return;
	}

	PreparedDrawCallDataQueue->Enqueue(MoveTemp(InData));
	PreparedDrawCallDataQueueEvent->Trigger();
}

bool FLexCanvasDrawCallProcessingRunnable::TryGetDrawCallData(FLexCanvasPendingDrawCallData& OutData)
{
	if (!PendingRebuildDrawCallQueue.IsValid())
	{
		return false;
	}

	if (!PendingRebuildDrawCallQueue->IsEmpty())
	{
		while (PendingRebuildDrawCallQueue->Dequeue(OutData)){}//only use the newest one
		return true;
	}
	return false;
}
