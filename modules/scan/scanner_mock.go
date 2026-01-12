package scan

var (
	mockScanItems = []*ScanDeviceItem{
		{
			IP:              "192.168.1.1",
			FirmwareVersion: "1.0.0",
			MacAddress:      "00:11:22:33:44:55",
			ModelName:       "DeviceModelY",
		},
		{
			IP:              "192.168.1.2",
			FirmwareVersion: "21.0.0",
			MacAddress:      "00:11:22:33:44:56",
			ModelName:       "DeviceModelX",
		},
	}
)

type MockScanner struct {
	onProgress  func(projectId int64, progress int)
	onCompleted func(projectId int64, deviceItems []*ScanDeviceItem, err error)
}

func NewMockScanner(
	onProgress func(projectId int64, progress int),
	onCompleted func(projectId int64, deviceItems []*ScanDeviceItem, err error)) *MockScanner {
	return &MockScanner{
		onProgress:  onProgress,
		onCompleted: onCompleted,
	}
}

func (scanner *MockScanner) ScanTopology(projectId int64, newTopology bool) {
	scanner.onProgress(projectId, 0)
	scanner.onProgress(projectId, 10)
	scanner.onProgress(projectId, 30)
	scanner.onProgress(projectId, 60)
	scanner.onProgress(projectId, 90)
	scanner.onCompleted(projectId, mockScanItems, nil)
}
