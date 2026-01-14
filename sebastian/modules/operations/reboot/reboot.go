package reboot

type IReboot interface {
	StartReboot(connId string, projectId int64, deviceIds []int64)
	Stop()
}
