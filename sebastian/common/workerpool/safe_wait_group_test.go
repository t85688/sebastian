package workerpool

import (
	"fmt"
	"testing"
	"time"
)

func Test_SafeWaitGroup(t *testing.T) {
	wg := newSafeWaitGroup()

	wg.Add(1)
	wg.Add(2)

	worker := func(id int) {
		defer wg.Done()

		time.Sleep(time.Second * 1)
		fmt.Printf("Worker: %d done\n", id)
	}

	go worker(1)
	go worker(2)
	go worker(3)

	wg.Wait()
	fmt.Println("all done")
}

func Test_AddZero(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Fatalf("Expected panic, but did not occur")
		}
	}()

	wg := newSafeWaitGroup()

	wg.Add(0)
}

func Test_AddNegative(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Fatalf("Expected panic, but did not occur")
		}
	}()

	wg := newSafeWaitGroup()

	wg.Add(-1)
}
