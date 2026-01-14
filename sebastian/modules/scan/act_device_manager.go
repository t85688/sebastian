package scan

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/topomanager"
)

func getActDeviceManager() (topomanager.IActDeviceManager, error) {
	return dipool.GetInstance[topomanager.IActDeviceManager]()
}
