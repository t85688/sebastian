package reboot

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func (reboot *Reboot) executeDevicesReboot(orderedDeviceIdList []int64, deviceIdMap map[int64]string) error {
	logger.Info("Start executeDevicesReboot")
	// deviceIdMap(ProjectDeviceId(int64), MafDeviceId(string))

	if len(orderedDeviceIdList) == 0 {
		return fmt.Errorf("Device list is empty")
	}

	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	minProgress := int64(60)
	maxProgress := int64(90)
	step := (maxProgress - minProgress) / int64(len(orderedDeviceIdList))

	logger.Info(fmt.Sprintf("Execution sequence: %v", orderedDeviceIdList))
	for index, id := range orderedDeviceIdList {
		if mafDeviceId, ok := deviceIdMap[id]; ok {

			progress := minProgress + step*int64(index+1)

			cfg := schema.RebootInput{
				Timeout: reboot.mafRebootTimeout,
			}
			_, code, err := dmManager.Reboot(mafDeviceId, cfg)
			if code >= common.MafGenericErrorBase || err != nil {
				reboot.onDeviceResultProgressResponse(reboot.wsConnId, reboot.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Failed",
					Progress:     progress,
					ErrorMessage: "Trigger Reboot failed",
				})
				errMsg := ""
				if err != nil {
					errMsg = err.Error()
				}
				logger.Info(fmt.Sprintf("Trigger %v Reboot failed(code:%v, err:%v)\n", id, code, errMsg))

			} else {
				reboot.onDeviceResultProgressResponse(reboot.wsConnId, reboot.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Success",
					Progress:     progress,
					ErrorMessage: "",
				})
				logger.Info(fmt.Sprintf("Trigger %v Reboot success\n", id))
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}

		if reboot.stopFlag {
			return nil
		}
	}

	return err
}
