package workerpool

import "errors"

var (
	ErrAlreadyClosed = errors.New("worker pool is already closed")
	ErrWaitTimeout   = errors.New("wait timed out")
)
