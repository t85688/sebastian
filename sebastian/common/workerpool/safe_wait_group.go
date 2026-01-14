package workerpool

import "sync"

type safeWaitGroup struct {
	wg    sync.WaitGroup
	mutex sync.Mutex
}

func newSafeWaitGroup() *safeWaitGroup {
	instance := &safeWaitGroup{}
	return instance
}

func (swg *safeWaitGroup) Add(n int) {
	if n <= 0 {
		panic("[AddN] n should be greater than 0")
	}

	swg.mutex.Lock()
	defer swg.mutex.Unlock()
	swg.wg.Add(n)
}

func (swg *safeWaitGroup) Wait() {
	swg.mutex.Lock()
	defer swg.mutex.Unlock()
	swg.wg.Wait()
}

func (swg *safeWaitGroup) Done() {
	swg.wg.Done()
}
