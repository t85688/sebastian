package monitor

import (
	"context"
	"fmt"
	"sync"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/workerpool"
)

const (
	monitorScheduledTaskDispatchInterval = 3 * time.Second
)

type monitorTaskDispatcher struct {
	mutex               sync.Mutex
	ctx                 context.Context
	workerPool          workerpool.IWorkerPoolWaitable
	scheduledTasks      map[string]MonitorTask
	eventTriggeredTasks map[string]MonitorTask
	oneTimeTasks        chan MonitorTask
}

func NewMonitorTaskDispatcher(ctx context.Context, workerPool workerpool.IWorkerPoolWaitable) *monitorTaskDispatcher {
	return &monitorTaskDispatcher{
		ctx:                 ctx,
		workerPool:          workerPool,
		scheduledTasks:      make(map[string]MonitorTask),
		eventTriggeredTasks: make(map[string]MonitorTask),
		oneTimeTasks:        make(chan MonitorTask, 32),
	}
}

func (dispatcher *monitorTaskDispatcher) RegisterTask(taskName string, strategy DispatchStrategy, task MonitorTask) error {
	dispatcher.mutex.Lock()
	defer dispatcher.mutex.Unlock()

	switch strategy {
	case MonitorTaskDispatchStrategyOneTime:
		dispatcher.oneTimeTasks <- task
	case MonitorTaskDispatchStrategyScheduled:
		dispatcher.scheduledTasks[taskName] = task
	case MonitorTaskDispatchStrategyEventTriggered:
		dispatcher.eventTriggeredTasks[taskName] = task
	}
	return nil
}

func (dispatcher *monitorTaskDispatcher) Start() {
	go dispatcher.dispatchScheduledTasks()
	go dispatcher.dispatchOneTimeTasks()
}

func (dispatcher *monitorTaskDispatcher) dispatchScheduledTasks() {
	ticker := time.NewTicker(monitorScheduledTaskDispatchInterval)
	defer ticker.Stop()

	for {
		select {
		case <-ticker.C:
			dispatcher.mutex.Lock()
			for _, task := range dispatcher.scheduledTasks {
				dispatcher.workerPool.AddJob(func() {
					task(dispatcher.ctx)
				})
			}
			dispatcher.mutex.Unlock()
		case <-dispatcher.ctx.Done():
			return
		}
	}
}

func (dispatcher *monitorTaskDispatcher) dispatchOneTimeTasks() {
	for {
		select {
		case task := <-dispatcher.oneTimeTasks:
			dispatcher.workerPool.AddJob(func() {
				task(dispatcher.ctx)
			})
		case <-dispatcher.ctx.Done():
			return
		}
	}
}

func (dispatcher *monitorTaskDispatcher) TriggerTask(taskName string) error {
	dispatcher.mutex.Lock()
	defer dispatcher.mutex.Unlock()

	task, exists := dispatcher.eventTriggeredTasks[taskName]
	if !exists {
		return fmt.Errorf("task not found: %s", taskName)
	}

	dispatcher.workerPool.AddJob(func() {
		task(dispatcher.ctx)
	})
	return nil
}

func (dispatcher *monitorTaskDispatcher) WaitAndClose() {
	dispatcher.mutex.Lock()
	defer dispatcher.mutex.Unlock()

	err := dispatcher.workerPool.WaitAndClose()
	if err != nil {
		logger.Errorf("Failed to wait and close worker pool: %v", err)
	}
}
