package netdl

// Device System Utilization Status
type SystemStatus struct {
	CPULoading   *float64       `json:"cpuLoading,omitempty"`
	MemoryUsage  *float64       `json:"memoryUsage,omitempty"`
	Availability *string        `json:"availability,omitempty"` // Availability in 24 HR
	Uptime       *int64         `json:"uptime,omitempty"`
	PowerStatus  map[int]string `json:"powerStatus,omitempty"` // enums:"on,off"
}
