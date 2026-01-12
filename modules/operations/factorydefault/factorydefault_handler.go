package factorydefault

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func (factoryDefault *FactoryDefault) executeDevicesFactoryDefault(orderedDeviceIdList []int64, deviceIdMap map[int64]string) error {
	logger.Info("Start executeDevicesFactoryDefault")
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

			cfg := schema.FactoryDefaultInput{
				Timeout:            factoryDefault.mafFactoryDefaultTimeout,
				KeepNetworkSetting: false,
			}
			_, code, err := dmManager.FactoryDefault(mafDeviceId, cfg)
			if code >= common.MafGenericErrorBase || err != nil {
				factoryDefault.onDeviceResultProgressResponse(factoryDefault.wsConnId, factoryDefault.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Failed",
					Progress:     progress,
					ErrorMessage: "Trigger FactoryDefault failed",
				})
				errMsg := ""
				if err != nil {
					errMsg = err.Error()
				}
				logger.Info(fmt.Sprintf("Trigger %v FactoryDefault failed(code:%v, err:%v)\n", id, code, errMsg))

			} else {
				factoryDefault.onDeviceResultProgressResponse(factoryDefault.wsConnId, factoryDefault.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Success",
					Progress:     progress,
					ErrorMessage: "",
				})
				logger.Info(fmt.Sprintf("Trigger %v FactoryDefault success\n", id))
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}

		if factoryDefault.stopFlag {
			return nil
		}
	}

	return err
}
