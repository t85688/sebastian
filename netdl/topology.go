package netdl

import (
	"sort"
	"sync"
)

type Topology struct {
	Devices map[string]*Device `json:"devices"`
	Links   map[string]*Link   `json:"links"`
}

func (t *Topology) GetDevices(mutex *sync.RWMutex) []*Device {
	devices := make([]*Device, 0, len(t.Devices))
	for _, device := range t.Devices {
		devices = append(devices, device)
	}
	sort.SliceStable(devices, func(i, j int) bool {
		return devices[i].DeviceId < devices[j].DeviceId //TODO(Kevin)
	})
	return devices
}

func (t *Topology) GetLinks() []*Link {
	links := make([]*Link, 0, len(t.Links))
	for _, link := range t.Links {
		links = append(links, link)
	}
	sort.SliceStable(links, func(i, j int) bool {
		return links[i].ID < links[j].ID
	})
	return links
}
