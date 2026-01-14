package deploy

type IDeployer interface {
	Deploy(connId string, projectId int64, baselineId int64, deviceIds []int64, skipMappingDevice bool)
	Stop()
}

type DeployDeviceResult struct {
	DeviceId     int64
	Status       string
	ErrorMessage string
}
