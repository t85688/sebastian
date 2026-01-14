package topomanager

import "sync"

type IActDeviceManager interface {
	IDeviceIDMapper
}

type ActDeviceManager struct {
	mutexDeviceIDMapping     sync.RWMutex
	ActDeviceIDToMAFDeviceID map[int64]string
	MAFDeviceIDToActDeviceID map[string]int64
}

func NewActDeviceManager() IActDeviceManager {
	return &ActDeviceManager{
		ActDeviceIDToMAFDeviceID: make(map[int64]string),
		MAFDeviceIDToActDeviceID: make(map[string]int64),
	}
}
