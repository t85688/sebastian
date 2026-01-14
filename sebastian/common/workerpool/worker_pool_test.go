package workerpool_test

import (
	"sync"
	"testing"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/workerpool"
)

func TestWorkerPool_FireAndForget(t *testing.T) {
	pool := workerpool.NewWorkerPool(5, 16)

	syncMap := sync.Map{}

	for i := 1; i <= 32; i++ {
		pool.AddJob(newMockJob(i, func() {
			syncMap.Store(i, true)
		}))
	}

	time.Sleep(time.Second * 4)

	for i := 1; i <= 32; i++ {
		if _, ok := syncMap.Load(i); !ok {
			t.Fatalf("cannot find %v in syncMap", i)
		}
	}
}
