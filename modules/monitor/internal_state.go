package monitor

type InternalState int

const (
	InternalStateStopped InternalState = iota
	InternalStateStopping
	InternalStateStarting
	InternalStateRunning
	InternalStateReloading
	InternalStatePaused
)

var internalStateStrMap = map[InternalState]string{
	InternalStateStopped:   "Stopped",
	InternalStateStopping:  "Stopping",
	InternalStateStarting:  "Starting",
	InternalStateRunning:   "Running",
	InternalStateReloading: "Reloading",
	InternalStatePaused:    "Paused",
}

func (state InternalState) String() string {
	if str, ok := internalStateStrMap[state]; ok {
		return str
	}
	return "Unknown"
}

func (state InternalState) isRunning() bool {
	return state == InternalStateRunning
}

func (state InternalState) isReloading() bool {
	return state == InternalStateReloading
}
