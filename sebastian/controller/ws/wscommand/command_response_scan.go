package wscommand

type ScanTopologyResult struct {
	Progress   int                        `json:"Progress"`
	ScanResult *[]*ScanTopologyResultItem `json:"ScanResult,omitempty"`
}

type ScanTopologyResultItem struct {
	FirmwareVersion string `json:"FirmwareVersion"`
	IP              string `json:"Ip"`
	MacAddress      string `json:"MacAddress"`
	ModelName       string `json:"ModelName"`
}
