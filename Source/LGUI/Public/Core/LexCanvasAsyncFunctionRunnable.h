#pragma once

class FLexCanvasAsyncFunctionRunnable : FRunnable
{
public:
	void Start()
	{
		bIsRunning = true;
		check(FunctionQueueEvent == nullptr);
		FunctionQueueEvent = FPlatformProcess::GetSynchEventFromPool();
		Thread.Reset(FRunnableThread::Create(this, TEXT("FLexCanvasAsyncFunctionRunnable"), 0, TPri_Normal));
	}
	void PushFunction(TFunction<void()> InFunction)
	{
		if (!bIsRunning || FunctionQueueEvent == nullptr)
		{
			return;
		}

		FunctionQueue.Enqueue(MoveTemp(InFunction));
		FunctionQueueEvent->Trigger();
		++ItemCount;
	}
	bool IsRunning()const
	{
		return bIsRunning;
	}

	int NumItems()const
	{
		return ItemCount;
	}
	bool IsEmpty()const
	{
		return ItemCount == 0;
	}

	//~ Begin FRunnable Interface
	virtual uint32 Run() override
	{
		while (bIsRunning)
		{
			auto QueueEvent = FunctionQueueEvent;
			if (QueueEvent == nullptr)
			{
				break;
			}

			if (QueueEvent->Wait(500))
			{
				while (FunctionQueue.Dequeue(TempFunction))
				{
					TempFunction();
					--ItemCount;
				}
			}
		}

		return 0;
	}
	virtual void Stop() override
	{
		if (!bIsRunning)
		{
			return;
		}

		bIsRunning = false;
		if (FunctionQueueEvent != nullptr)
		{
			FunctionQueueEvent->Trigger();
		}
		if (Thread.IsValid())
		{
			Thread->WaitForCompletion();
			Thread.Reset();
		}
		if (FunctionQueueEvent != nullptr)
		{
			FPlatformProcess::ReturnSynchEventToPool(FunctionQueueEvent);
			FunctionQueueEvent = nullptr;
		}
		ItemCount = 0;
	}
	virtual void Exit() override
	{
		Stop();
	}
	//~ End FRunnable Interface

private:
	TFunction<void()> TempFunction = nullptr;
	TQueue<TFunction<void()>, EQueueMode::Mpsc> FunctionQueue;
	std::atomic<int> ItemCount = 0;
	FEvent* FunctionQueueEvent = nullptr;
	
	std::atomic<bool> bIsRunning = false;
	TUniquePtr<FRunnableThread> Thread;
};
