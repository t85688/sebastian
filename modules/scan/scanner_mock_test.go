package scan

import "testing"

const (
	mockScanTopologyProjectId   int64 = 12345
	mockScanTopologyNewTopology       = true
)

func TestMockScanTopology(t *testing.T) {
	mockScanner := NewMockScanner(getMockOnScanTopologyProgress(t), getMockOnScanTopologyCompleted(t))

	mockScanner.ScanTopology(mockScanTopologyProjectId, mockScanTopologyNewTopology)
}

func getMockOnScanTopologyProgress(t *testing.T) func(projectId int64, progress int) {
	var lastProgress int = -1

	return func(projectId int64, progress int) {
		if progress < lastProgress {
			t.Fatalf("Progress should not decrease: last %d, current %d", lastProgress, progress)
		}

		lastProgress = progress
		t.Logf("Mock ScanTopology Progress: projectId=%d, progress=%d", projectId, progress)
	}
}

func getMockOnScanTopologyCompleted(t *testing.T) func(int64, []*ScanDeviceItem, error) {
	return func(projectId int64, deviceItems []*ScanDeviceItem, err error) {
		if len(deviceItems) == 0 {
			t.Fatal("Device items should not be empty")
		}

		if len(deviceItems) != len(mockScanItems) {
			t.Fatalf("Device items length mismatch: expected %d, got %d", len(mockScanItems), len(deviceItems))
		}

		for i, item := range deviceItems {
			if item.IP != mockScanItems[i].IP ||
				item.FirmwareVersion != mockScanItems[i].FirmwareVersion ||
				item.MacAddress != mockScanItems[i].MacAddress ||
				item.ModelName != mockScanItems[i].ModelName {
				t.Fatalf("Device item mismatch at index %d: expected %+v, got %+v", i, mockScanItems[i], item)
			}
		}
	}
}
