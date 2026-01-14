package workerpool

import (
	"context"
	"sync"
)

type WorkerPool struct {
	ctx         context.Context
	ctxCancelFn func()
	jobs        chan func()
	mutex       sync.Mutex
	isClosed    bool
}

func NewWorkerPool(workerCount int, jobQueueSize int) IWorkerPool {
	if workerCount <= 0 {
		panic("workerCount should be greater than 0")
	}

	if jobQueueSize <= 0 {
		panic("jobQueueSize should be greater than 0")
	}

	ctx, cancelFn := context.WithCancel(context.Background())
	pool := &WorkerPool{
		ctx:         ctx,
		ctxCancelFn: cancelFn,
		jobs:        make(chan func(), jobQueueSize),
	}

	for i := 1; i <= workerCount; i++ {
		worker := &Worker{
			WorkerId: i,
			jobChan:  pool.jobs,
			wg:       nil,
		}

		go worker.Start(ctx)
	}

	return pool
}

func (pool *WorkerPool) AddJob(job func()) error {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	if pool.isClosed {
		return ErrAlreadyClosed
	}

	pool.jobs <- job
	return nil
}

func (pool *WorkerPool) CancelAndClose() error {
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

func (pool *WorkerPool) Close() error {
	pool.mutex.Lock()
	defer pool.mutex.Unlock()

	if pool.isClosed {
		return ErrAlreadyClosed
	}

	pool.isClosed = true
	close(pool.jobs)
	return nil
}
