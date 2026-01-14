package semaphore

import (
	"context"
	"testing"
	"time"
)

func Test_Semaphore(t *testing.T) {
	semaphore := NewSemaphore(3)

	semaphore.Acquire()
	semaphore.Acquire()
	semaphore.Acquire()
	tryResult := semaphore.TryAcquire()
	if tryResult {
		t.Fatalf("tryResult should be false")
	}

	semaphore.Release()
	semaphore.Release()
	semaphore.Release()
}

func Test_Semaphore_AcquireWithTimeout(t *testing.T) {
	semaphore := NewSemaphore(3)

	semaphore.Acquire()
	semaphore.Acquire()
	semaphore.Acquire()

	ctx, _ := context.WithTimeout(context.Background(), time.Second*1)

	err := semaphore.AcquireWithTimeout(ctx.Done())
	if err == nil {
		t.Fatalf("err should not be nil")
	}
}

func Test_NewSemaphore_InvalidInput_Zero(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Fatalf("NewSemaphore should be panicked, but not")
		}
	}()

	NewSemaphore(0)
}

func Test_NewSemaphore_InvalidInput_Negative(t *testing.T) {
	defer func() {
		if r := recover(); r == nil {
			t.Fatalf("NewSemaphore should be panicked, but not")
		}
	}()

	NewSemaphore(-1)
}
