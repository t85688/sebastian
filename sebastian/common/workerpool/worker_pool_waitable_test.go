package workerpool_test

import (
	"fmt"
	"sync"
	"testing"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/workerpool"
)

func Test_WorkerPoolWaitable_Wait(t *testing.T) {
	pool := workerpool.NewWorkerPoolWaitable(5, 32)
	syncMap := sync.Map{}

	for i := 1; i <= 64; i++ {
		pool.AddJob(newMockJob(i, func() {
			syncMap.Store(i, true)
		}))
	}

	fmt.Println("[1st] pool.WaitAndClose() ...")
	pool.Wait()
	fmt.Println("[1st] pool.WaitAndClose() ... done")

	for i := 1; i <= 64; i++ {
		if _, ok := syncMap.Load(i); !ok {
			t.Fatalf("cannot find %v in syncMap", i)
		}
	}

	for i := 65; i <= 128; i++ {
		pool.AddJob(newMockJob(i, func() {
			syncMap.Store(i, true)
		}))
	}

	fmt.Println("[2nd] pool.WaitAndClose() ...")
	pool.Wait()
	fmt.Println("[2nd] pool.WaitAndClose() ... done")

	for i := 1; i <= 128; i++ {
		if _, ok := syncMap.Load(i); !ok {
			t.Fatalf("cannot find %v in syncMap", i)
		}
	}
}

func Test_WorkerPoolWaitable_WaitAndClose(t *testing.T) {
	pool := workerpool.NewWorkerPoolWaitable(5, 32)
	syncMap := sync.Map{}

	for i := 1; i <= 64; i++ {
		pool.AddJob(newMockJob(i, func() {
			syncMap.Store(i, true)
		}))
	}

	fmt.Println("pool.WaitAndClose() ...")
	pool.WaitAndClose()
	fmt.Println("pool.WaitAndClose() ... done")

	for i := 1; i <= 64; i++ {
		if _, ok := syncMap.Load(i); !ok {
			t.Fatalf("cannot find %v in syncMap", i)
		}
	}
}

func Test_WorkerPoolWaitable_Wait_JobPanic(t *testing.T) {
	pool := workerpool.NewWorkerPoolWaitable(2, 4)

	for i := 1; i <= 4; i++ {
		pool.AddJob(newMockJobWithPanic(i))
	}

	fmt.Println("pool.WaitAndClose() ...")
	pool.Wait()
	fmt.Println("pool.WaitAndClose() ... done")

	for i := 5; i <= 8; i++ {
		pool.AddJob(newMockJobWithPanic(i))
	}

	fmt.Println("pool.WaitAndClose() ...")
	pool.Wait()
	fmt.Println("pool.WaitAndClose() ... done")
}

func Test_WorkerPoolWaitable_WaitAndClose_JobPanic(t *testing.T) {
	pool := workerpool.NewWorkerPoolWaitable(2, 4)

	for i := 1; i <= 4; i++ {
		pool.AddJob(newMockJobWithPanic(i))
	}

	fmt.Println("pool.WaitAndClose() ...")
	pool.WaitAndClose()
	fmt.Println("pool.WaitAndClose() ... done")
}

func newMockJob(n int, jobFunc func()) func() {
	return func() {
		time.Sleep(time.Millisecond * 50)
		jobFunc()
		fmt.Printf("Job %d completed\n", n)
	}
}

func newMockJobWithPanic(n int) func() {
	return func() {
		time.Sleep(time.Millisecond * 30)
		panic(fmt.Sprintf("Job %d panic", n))
	}
}
