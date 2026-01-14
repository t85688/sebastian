package workerpool

type IWorkerPool interface {
	AddJob(job func()) error
	CancelAndClose() error
	Close() error
}

type IWorkerPoolWaitable interface {
	IWorkerPool
	IWaitable
}

type IWaitable interface {
	Wait()
	WaitAndClose() error
}
