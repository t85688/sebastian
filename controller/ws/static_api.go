package ws

import "gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wsconn"

func Broadcast(msg []byte) {
	wsconn.GetDefaultHubInstance().Broadcast(msg)
}

func Multicast(msg []byte, connIds ...string) {
	wsconn.GetDefaultHubInstance().Multicast(msg, connIds...)
}

func Unicast(msg []byte, connId string) {
	wsconn.GetDefaultHubInstance().Unicast(msg, connId)
}

func MulticastToSpecifiedProjectId(msg []byte, projectId int64) {
	wsconn.GetDefaultHubInstance().MulticastToSpecifiedProjectId(msg, projectId)
}

func MulticastToSystem(msg []byte) {
	wsconn.GetDefaultHubInstance().MulticastToSystem(msg)
}
