package workerpool

import (
	"context"
	"runtime/debug"
)

type Worker struct {
	WorkerId int
	wg       *safeWaitGroup
	jobChan  <-chan func()
}

func (worker *Worker) Start(ctx context.Context) {
	for {
		select {
		case job, ok := <-worker.jobChan:
			if !ok {
				return
			}

			func() {
				defer func() {
					if r := recover(); r != nil {
						logger.Errorf("worker %d panic: %v\n%s", worker.WorkerId, r, debug.Stack())
					}
				}()

				defer func() {
					if worker.wg != nil {
						worker.wg.Done()
					}
				}()

				job()
			}()
		case <-ctx.Done():
			return
		}
	}
}
