package factorydefault

type IFactoryDefault interface {
	StartFactoryDefault(connId string, projectId int64, deviceIds []int64)
	Stop()
}
