package topomanager

type IDeviceIDMapper interface {
	AddDeviceMapping(actDeviceID int64, mafDeviceID string)
	GetMAFDeviceID(actDeviceID int64) (string, bool)
	GetActDeviceID(mafDeviceID string) (int64, bool)
	DeleteDeviceMappingByActDeviceID(actDeviceID int64)
	DeleteDeviceMappingByMAFDeviceID(mafDeviceID string)
	ClearDeviceMappings()
}

func (actDeviceMgr *ActDeviceManager) AddDeviceMapping(actDeviceID int64, mafDeviceID string) {
	actDeviceMgr.mutexDeviceIDMapping.Lock()
	defer actDeviceMgr.mutexDeviceIDMapping.Unlock()

	actDeviceMgr.ActDeviceIDToMAFDeviceID[actDeviceID] = mafDeviceID
	actDeviceMgr.MAFDeviceIDToActDeviceID[mafDeviceID] = actDeviceID
}

func (actDeviceMgr *ActDeviceManager) GetMAFDeviceID(actDeviceID int64) (string, bool) {
	actDeviceMgr.mutexDeviceIDMapping.RLock()
	defer actDeviceMgr.mutexDeviceIDMapping.RUnlock()

	mafDeviceID, exists := actDeviceMgr.ActDeviceIDToMAFDeviceID[actDeviceID]
	return mafDeviceID, exists
}

func (actDeviceMgr *ActDeviceManager) GetActDeviceID(mafDeviceID string) (int64, bool) {
	actDeviceMgr.mutexDeviceIDMapping.RLock()
	defer actDeviceMgr.mutexDeviceIDMapping.RUnlock()

	actDeviceID, exists := actDeviceMgr.MAFDeviceIDToActDeviceID[mafDeviceID]
	return actDeviceID, exists
}

func (actDeviceMgr *ActDeviceManager) DeleteDeviceMappingByActDeviceID(actDeviceID int64) {
	actDeviceMgr.mutexDeviceIDMapping.Lock()
	defer actDeviceMgr.mutexDeviceIDMapping.Unlock()

	if mafDeviceID, exists := actDeviceMgr.ActDeviceIDToMAFDeviceID[actDeviceID]; exists {
		delete(actDeviceMgr.MAFDeviceIDToActDeviceID, mafDeviceID)
	}

	delete(actDeviceMgr.ActDeviceIDToMAFDeviceID, actDeviceID)
}

func (actDeviceMgr *ActDeviceManager) DeleteDeviceMappingByMAFDeviceID(mafDeviceID string) {
	actDeviceMgr.mutexDeviceIDMapping.Lock()
	defer actDeviceMgr.mutexDeviceIDMapping.Unlock()

	if actDeviceID, exists := actDeviceMgr.MAFDeviceIDToActDeviceID[mafDeviceID]; exists {
		delete(actDeviceMgr.ActDeviceIDToMAFDeviceID, actDeviceID)
	}

	delete(actDeviceMgr.MAFDeviceIDToActDeviceID, mafDeviceID)
}

func (actDeviceMgr *ActDeviceManager) ClearDeviceMappings() {
	actDeviceMgr.mutexDeviceIDMapping.Lock()
	defer actDeviceMgr.mutexDeviceIDMapping.Unlock()

	actDeviceMgr.ActDeviceIDToMAFDeviceID = make(map[int64]string)
	actDeviceMgr.MAFDeviceIDToActDeviceID = make(map[string]int64)
}
