package monitor

type DispatchStrategy int

const (
	MonitorTaskDispatchStrategyOneTime DispatchStrategy = iota
	MonitorTaskDispatchStrategyScheduled
	MonitorTaskDispatchStrategyEventTriggered
)
