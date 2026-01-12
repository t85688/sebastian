package importconfig

type IImportConfig interface {
	StartImportConfig(connId string, projectId int64, deviceId []int64)
	Stop()
}
