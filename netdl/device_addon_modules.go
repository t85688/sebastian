package netdl

type Modules struct {
	Ethernet []*EthernetModule `json:"ethernet"`
	Power    []*PowerModule    `json:"power"`
}

type EthernetModule struct {
	SlotID int `json:"slotID"`
	// Exist           bool   `json:"exist"`
	ModuleName      string `json:"moduleName,omitempty"`
	SerialNumber    string `json:"serialNumber,omitempty"`
	ProductRevision string `json:"productRevision,omitempty"`
	Status          string `json:"status,omitempty"`
	ModuleID        int    `json:"moduleId"`
}

type PowerModule struct {
	SlotID int `json:"slotID"`
	// Exist           bool   `json:"exist"`
	ModuleName      string `json:"moduleName,omitempty"`
	SerialNumber    string `json:"serialNumber,omitempty"`
	ProductRevision string `json:"productRevision,omitempty"`
	Status          string `json:"status,omitempty"`
	// ModuleID        int    `json:"moduleId"`
	// ModuleID ??
}
