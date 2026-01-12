package locator

type ILocator interface {
	StartLocator(connId string, projectId int64, deviceIds []int64, duration uint32)
	Stop()
}
