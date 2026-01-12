package exportconfig

type IExportConfig interface {
	StartExportConfig(connId string, projectId int64, deviceIds []int64)
	Stop()
}
