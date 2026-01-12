package netdl

import (
	"encoding/json"
	"fmt"
	"sync"

	"gitlab.com/moxa/sw/maf/moxa-app-framework/network/netcore/kernel/protocoldata"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/network/share/protocoltag"
)

type DeviceRaw struct {
	lock *sync.RWMutex
	data map[string]*protocoldata.Value
}

func NewDeviceRaw() *DeviceRaw {
	return &DeviceRaw{
		lock: &sync.RWMutex{},
		data: map[string]*protocoldata.Value{},
	}
}

func NewDeviceRawWithData(in map[string]*protocoldata.Value) *DeviceRaw {
	return &DeviceRaw{
		lock: &sync.RWMutex{},
		data: in,
	}
}

func (r *DeviceRaw) Clone() *DeviceRaw {
	if r == nil {
		return nil
	}

	r.lock.RLock()
	defer r.lock.RUnlock()

	clonedData := map[string]*protocoldata.Value{}
	for k, v := range r.data {
		clonedData[k] = v
	}
	return &DeviceRaw{
		lock: &sync.RWMutex{},
		data: clonedData,
	}
}

func (r *DeviceRaw) Export() map[string]*protocoldata.Value {
	if r == nil {
		return nil
	}

	r.lock.RLock()
	defer r.lock.RUnlock()

	clonedData := map[string]*protocoldata.Value{}
	for k, v := range r.data {
		clonedData[k] = v
	}
	return clonedData
}

func (r *DeviceRaw) Size() int {
	if r == nil {
		return 0
	}

	r.lock.RLock()
	defer r.lock.RUnlock()
	return len(r.data)
}

func (r *DeviceRaw) HasKey(key string) bool {
	if r == nil {
		return false
	}

	r.lock.RLock()
	defer r.lock.RUnlock()
	_, ok := r.data[key]
	return ok
}

func (r *DeviceRaw) Get(key string) *protocoldata.Value {
	if r == nil {
		return nil
	}

	r.lock.RLock()
	defer r.lock.RUnlock()
	if v, ok := r.data[key]; ok {
		return v
	} else {
		return nil
	}
}

func (r *DeviceRaw) GetOk(key string) (*protocoldata.Value, bool) {
	if r == nil {
		return nil, false
	}

	r.lock.RLock()
	defer r.lock.RUnlock()
	if v, ok := r.data[key]; ok {
		return v, true
	} else {
		return nil, false
	}
}

func (r *DeviceRaw) GetFirst(key, protocol string) any {
	if r == nil {
		return nil
	}

	r.lock.RLock()
	defer r.lock.RUnlock()

	pval, ok := r.data[key]
	if !ok {
		return nil
	}

	if protocol == protocoltag.SNMP {
		if len(pval.SNMP) < 1 {
			return nil
		}
		return pval.SNMP[0].GetVal()
	} else if protocol == protocoltag.MMS {
		if len(pval.MMS) < 1 {
			return nil
		}
		return pval.MMS[0].GetVal()
	}
	return nil
}

func (r *DeviceRaw) Set(key string, pval2 *protocoldata.Value) {
	if r == nil {
		return
	}

	r.lock.Lock()
	defer r.lock.Unlock()

	r.data[key] = pval2
}

func (r *DeviceRaw) Remove(key string) {
	if r == nil {
		return
	}

	r.lock.Lock()
	defer r.lock.Unlock()

	delete(r.data, key)
}

func (r *DeviceRaw) Range(callback func(string, *protocoldata.Value) bool) {
	clone := r.Clone()

	if clone == nil {
		return
	}

	for k, v := range clone.data {
		if !callback(k, v) {
			break
		}
	}
}

func (r *DeviceRaw) MarshalJSON() ([]byte, error) {
	if r == nil {
		return []byte("null"), nil
	}

	r.lock.RLock()
	defer r.lock.RUnlock()
	return json.Marshal(r.data)
}

func (r *DeviceRaw) UnmarshalJSON(data []byte) error {
	if r == nil {
		return fmt.Errorf("cannot unmarshal into nil DeviceRaw")
	}

	r.lock.Lock()
	defer r.lock.Unlock()
	return json.Unmarshal(data, &r.data)
}
