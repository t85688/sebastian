package scan

import (
	"context"
	"fmt"
)

var errScanCanceled = fmt.Errorf("ScanTopology task had been canceled")
var errScanBusy = fmt.Errorf("another ScanTopology task is in progress now")

type IScanner interface {
	StartScanTopology(ctx context.Context, projectId int64, newTopology bool) ([]*ScanDeviceItem, ScanResultStatus, error)
	StopScanTopology() StopScanResultStatus
}

type ScanDeviceItem struct {
	IP              string
	FirmwareVersion string
	MacAddress      string
	ModelName       string
}
