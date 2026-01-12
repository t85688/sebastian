package netdl

/*
func (device *Device) DeepCopy(mutex *sync.RWMutex, keepRaw bool) *Device {
	if device == nil {
		return nil
	}
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	newDevice := &Device{
		// ID:            device.ID,
		Communication: device.Communication,
		DeviceBasic:   *device.DeviceBasic.DeepCopy(mutex),
	}

	newDevice.Redundancy = xcopy.DeepCopy(device.Redundancy).(*RedundancyStatus)
	newDevice.SystemStatus = xcopy.DeepCopy(device.SystemStatus).(*SystemStatus)
	newDevice.Interfaces = xcopy.DeepCopy(device.Interfaces).([]*Interface)
	newDevice.Ports = xcopy.DeepCopy(device.Ports).([]*Port)
	newDevice.Modules = xcopy.DeepCopy(device.Modules).(*Modules)
	newDevice.Configuration = xcopy.DeepCopy(device.Configuration).(*Configuration)

	if keepRaw {
		newDevice.Raw = device.Raw.Clone()
	} else {
		newDevice.Raw = nil
	}

	return newDevice
}

func (device *Device) SetRaw(mutex *sync.RWMutex, data map[string]*protocoldata.Value) {
	if mutex != nil {
		mutex.Lock()
		defer mutex.Unlock()
	}

	device.Raw = NewDeviceRawWithData(data)
}

func (device *Device) AppendRaw(mutex *sync.RWMutex, key string, val *protocoldata.Value) {
	if mutex != nil {
		mutex.Lock()
		defer mutex.Unlock()
	}

	if device.Raw == nil {
		device.Raw = NewDeviceRaw()
	}

	device.Raw.Set(key, val)
}

func (device *Device) AppendRaws(mutex *sync.RWMutex, raws map[string]*protocoldata.Value) {
	if mutex != nil {
		mutex.Lock()
		defer mutex.Unlock()
	}

	if device.Raw == nil {
		device.Raw = NewDeviceRaw()
	}

	for key, val := range raws {
		device.Raw.Set(key, val)
	}
}

func (device *Device) GetRawByKey(mutex *sync.RWMutex, key string) (*protocoldata.Value, bool) {
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	if device.Raw == nil {
		return nil, false
	}
	val, exists := device.Raw.GetOk(key)
	return val, exists
}

func (device *Device) GetRawString(mutex *sync.RWMutex, key string, protocol string) (string, bool) {
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	if device.Raw == nil {
		return "", false
	}

	if ival := device.Raw.GetFirst(key, protocol); ival != nil {
		switch val := ival.(type) {
		case []byte:
			return string(val), true
		case string:
			return val, true
		}
	}

	return "", false
}

func (device *Device) ClearRaw(mutex *sync.RWMutex) {
	if mutex != nil {
		mutex.Lock()
		defer mutex.Unlock()
	}

	device.Raw = NewDeviceRaw()
}
*/
