package semaphore

import "fmt"

var (
	ErrAcquireTimeout = fmt.Errorf("acquire semaphore timeout")
)

type Semaphore struct {
	capacity int
	ch       chan struct{}
}

func NewSemaphore(n int) *Semaphore {
	if n <= 0 {
		panic(fmt.Sprintf("NewSemaphore input should be not less then 1, invalid input: %v", n))
	}

	instance := &Semaphore{
		ch:       make(chan struct{}, n),
		capacity: n,
	}

	return instance
}

func (semaphore *Semaphore) Acquire() {
	semaphore.ch <- struct{}{}
}

func (semaphore *Semaphore) AcquireWithTimeout(deadline <-chan struct{}) error {
	select {
	case semaphore.ch <- struct{}{}:
		return nil
	case <-deadline:
		return ErrAcquireTimeout
	}
}

func (semaphore *Semaphore) Capacity() int {
	return semaphore.capacity
}

func (semaphore *Semaphore) TryAcquire() bool {
	select {
	case semaphore.ch <- struct{}{}:
		return true
	default:
		return false
	}

}

func (semaphore *Semaphore) Release() {
	<-semaphore.ch
}
