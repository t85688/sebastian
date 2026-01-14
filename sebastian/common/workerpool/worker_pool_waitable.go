package workerpool

import (
	"context"
	"sync"
)

type WorkerPoolWaitable struct {
	ctx         context.Context
	ctxCancelFn func()
	jobs        chan func()
	wg          *safeWaitGroup
	mutex       sync.Mutex
	isClosed    bool
}

func NewWorkerPoolWaitable(workerCount int, jobQueueSize int) IWorkerPoolWaitable {
	if workerCount <= 0 {
		panic("workerCount should be greater than 0")
	}

	if jobQueueSize <= 0 {
		panic("jobQueueSize should be greater than 0")
	}

	ctx, cancelFn := context.WithCancel(context.Background())
	pool := &WorkerPoolWaitable{
		ctx:         ctx,
		ctxCancelFn: cancelFn,
		jobs:        make(chan func(), jobQueueSize),
		wg:          newSafeWaitGroup(),
	}

	for i := 1; i <= workerCount; i++ {
		worker := &Worker{
			WorkerId: i,
			jobChan:  pool.jobs,
			wg:       pool.wg,
		}

		go worker.Start(ctx)
	}

	return pool
}

func (pool *WorkerPoolWaitable) AddJob(job func()) error {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	if pool.isClosed {
		return ErrAlreadyClosed
	}

	pool.wg.Add(1)
	pool.jobs <- job
	return nil
}

func (pool *WorkerPoolWaitable) CancelAndClose() error {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	pool.ctxCancelFn()

	if pool.isClosed {
		return ErrAlreadyClosed
	}

	pool.isClosed = true
	close(pool.jobs)
	return nil
}

func (pool *WorkerPoolWaitable) Close() error {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	if pool.isClosed {
		return ErrAlreadyClosed
	}

	pool.isClosed = true
	close(pool.jobs)
	return nil
}

func (pool *WorkerPoolWaitable) Wait() {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	pool.wg.Wait()
}

func (pool *WorkerPoolWaitable) WaitAndClose() error {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	if pool.isClosed {
		return ErrAlreadyClosed
	}

	pool.wg.Wait()

	pool.ctxCancelFn()
	pool.isClosed = true
	close(pool.jobs)

	return nil
}
