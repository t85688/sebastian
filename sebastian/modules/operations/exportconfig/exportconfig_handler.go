package exportconfig

import (
	"fmt"
	"os"
	"path/filepath"
	"time"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/modules/operations"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func (exportconfig *ExportConfig) executeDevicesExportConfig(orderedDeviceIdList []int64, deviceIdMap map[int64]string, project domain.Project) error {
	logger.Info("Start executeDevicesExportConfig")
	// deviceIdMap(ProjectDeviceId(int64), MafDeviceId(string))

	// Check divce list
	if len(orderedDeviceIdList) == 0 {
		return fmt.Errorf("Device list is empty")
	}

	// Get Folder
	tmpFolder, err := internal.GetCogsworthTmpDeviceConfigDirFromEnviron()
	if err != nil {
		return err
	}

	// Clear Folder's data
	entries, _ := os.ReadDir(tmpFolder)
	for _, e := range entries {
		err = os.RemoveAll(filepath.Join(tmpFolder, e.Name()))
		if err != nil {
			return fmt.Errorf("Failed to remove data: %w", err)
		}
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

			// Get Device IP to crate the export file name
			device, err := operations.GetProjectDeviceById(project, id)
			if err != nil {
				logger.Errorf("Device not found at Project. err: %v", err)
				return err
			}
			now := time.Now()
			timeStr := now.Format("200601021504") // yyyyMMddhhmm
			file_name := fmt.Sprintf("%s_%s_%s.ini", device.Ipv4.IpAddress, device.DeviceProperty.ModelName, timeStr)
			filePath := filepath.Join(tmpFolder, file_name)

			cfg := schema.ExportConfigInput{
				FilePath:  filePath,
				IsStartUp: false,
				Timeout:   exportconfig.mafExportConfigTimeout,
			}
			_, code, err := dmManager.ExportConfig(mafDeviceId, cfg)
			if code >= common.MafGenericErrorBase || err != nil {
				exportconfig.onDeviceResultProgressResponse(exportconfig.wsConnId, exportconfig.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Failed",
					Progress:     progress,
					ErrorMessage: "Trigger ExportConfig failed",
				})
				errMsg := ""
				if err != nil {
					errMsg = err.Error()
				}
				logger.Info(fmt.Sprintf("Trigger %v ExportConfig failed(code:%v, err:%v)\n", id, code, errMsg))

			} else {
				exportconfig.onDeviceResultProgressResponse(exportconfig.wsConnId, exportconfig.wsOpCode, wscommand.OperationsDeviceResult{
					Id:           id,
					Status:       "Success",
					Progress:     progress,
					ErrorMessage: "",
				})
				logger.Info(fmt.Sprintf("Trigger %v ExportConfig success\n", id))
			}
		} else {
			return fmt.Errorf("The Device(%v) not exists at the DeviceMappingMap", id)
		}

		if exportconfig.stopFlag {
			return nil
		}
	}

	return err
}
