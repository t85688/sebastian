package importconfig

import (
	"fmt"
	"path/filepath"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/dipool"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommand"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func (importconfig *ImportConfig) executeDeviceImportConfig(device domain.Device, mafDeviceId string, project domain.Project) error {
	logger.Info("Start executeDevicesImportConfig")
	// deviceIdMap(ProjectDeviceId(int64), MafDeviceId(string))
	dmManager, err := dipool.GetDMManager()
	if err != nil {
		return err
	}

	progress := int64(80)

	// Get file path
	tmpFolder, err := internal.GetCogsworthTmpDeviceConfigDirFromEnviron()
	if err != nil {
		return err
	}
	file_name := fmt.Sprintf("%s_%s.ini", project.ProjectSetting.ProjectName, device.Ipv4.IpAddress)
	filePath := filepath.Join(tmpFolder, file_name)

	cfg := schema.ImportConfigInput{
		FilePath:  filePath,
		IsStartUp: false,
		// Timeout:   importconfig.mafImportConfigTimeout,
	}
	_, code, err := dmManager.ImportConfig(mafDeviceId, cfg)
	if code >= common.MafGenericErrorBase || err != nil {
		importconfig.onDeviceResultProgressResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.OperationsDeviceResult{
			Id:           device.Id,
			Status:       "Failed",
			Progress:     progress,
			ErrorMessage: "Trigger ImportConfig failed",
		})
		errMsg := ""
		if err != nil {
			errMsg = err.Error()
		}
		logger.Info(fmt.Sprintf("Trigger %v ImportConfig failed(code:%v, err:%v)\n", device.Id, code, errMsg))

	} else {
		importconfig.onDeviceResultProgressResponse(importconfig.wsConnId, importconfig.wsOpCode, wscommand.OperationsDeviceResult{
			Id:           device.Id,
			Status:       "Success",
			Progress:     progress,
			ErrorMessage: "",
		})
		logger.Info(fmt.Sprintf("Trigger %v ImportConfig success\n", device.Id))
	}

	return err
}
