package firmwareupgrade

import (
	"fmt"
	"path/filepath"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func (firmwareupgrade *FirmwareUpgrade) executeDevicesFirmwareUpgrade(orderedDeviceIdList []int64, deviceIdMap map[int64]string, firmwareFileName string) error {
	logger.Info("Start executeDevicesFirmwareUpgrade")
	// deviceIdMap(ProjectDeviceId(int64), MafDeviceId(string))

	// Check divce list
	if len(orderedDeviceIdList) == 0 {
		return fmt.Errorf("Device list is empty")
	}

	// Get Folder
	tmpFolder, err := internal.GetCogsworthTmpFirmwareDirFromEnviron()
	if err != nil {
		return err
	}

	filePath := filepath.Join(tmpFolder, firmwareFileName)

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

			cfg := schema.UpgradeFirmwareInput{
				FilePath: filePath,
				Timeout:  firmwareupgrade.mafFirmwareUpgradeTimeout,
			}
			_, code, err := dmManager.FirmwareUpgrade(mafDeviceId, cfg)
			if code >= common.MafGenericErrorBase || err != nil {
				firmwareupgrade.onDeviceResultProgressResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Failed",
					Progress:     progress,
					ErrorMessage: "Trigger FirmwareUpgrade failed",
				})
				errMsg := ""
				if err != nil {
					errMsg = err.Error()
				}
				logger.Info(fmt.Sprintf("Trigger %v FirmwareUpgrade failed(code:%v, err:%v)\n", id, code, errMsg))

			} else {
				firmwareupgrade.onDeviceResultProgressResponse(firmwareupgrade.wsConnId, firmwareupgrade.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Success",
					Progress:     progress,
					ErrorMessage: "",
				})
				logger.Info(fmt.Sprintf("Trigger %v FirmwareUpgrade success\n", id))
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}

		if firmwareupgrade.stopFlag {
			return nil
		}
	}

	return err
}
