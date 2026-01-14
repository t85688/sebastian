package firmwareupgrade

type IFirmwareUpgrade interface {
	StartFirmwareUpgrade(connId string, projectId int64, deviceIds []int64, firmwareFileName string)
	Stop()
}
