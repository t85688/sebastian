package wsconn

import (
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommon"
)

var defaultConnHub = newWsConnectionHub()

func GetDefaultHubInstance() *wsConnectionHub {
	return defaultConnHub
}

type wsConnectionHub struct {
	connections  map[string]*WsConn
	systemMap    map[string]*WsConn
	projectIdMap map[int64][]*WsConn // Map to hold connections by project ID
	mutex        sync.Mutex
}

func newWsConnectionHub() *wsConnectionHub {
	return &wsConnectionHub{
		connections:  make(map[string]*WsConn),
		systemMap:    make(map[string]*WsConn),
		projectIdMap: make(map[int64][]*WsConn),
	}
}

func (hub *wsConnectionHub) Broadcast(msg []byte) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	for _, conn := range hub.connections {
		if err := conn.Write(msg); err != nil {
			logger.Infof("Failed to write message to connection %s: %v\n", conn.ID, err)
		}
	}
}

func (hub *wsConnectionHub) Multicast(msg []byte, connIds ...string) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	for _, id := range connIds {
		if conn, exists := hub.connections[id]; exists {
			if err := conn.Write(msg); err != nil {
				logger.Infof("Failed to write message to connection %s: %v\n", id, err)
			}
		} else {
			logger.Infof("Connection with ID %s does not exist\n", id)
		}
	}
}

func (hub *wsConnectionHub) Unicast(msg []byte, connId string) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	if conn, exists := hub.connections[connId]; exists {
		if err := conn.Write(msg); err != nil {
			logger.Infof("Failed to write message to connection %s: %v\n", connId, err)
		}
	}
}

func (hub *wsConnectionHub) MulticastToSpecifiedProjectId(msg []byte, projectId int64) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	if connections, exists := hub.projectIdMap[projectId]; exists {
		for _, conn := range connections {
			if err := conn.Write(msg); err != nil {
				logger.Infof("Failed to write message to connection %s: %v\n", conn.ID, err)
			}
		}
	} else {
		logger.Debugf("No connections found for project ID %d\n", projectId)
	}
}

func (hub *wsConnectionHub) MulticastToSystem(msg []byte) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	for _, conn := range hub.systemMap {
		if err := conn.Write(msg); err != nil {
			logger.Infof("Failed to write message to system connection %s: %v\n", conn.ID, err)
		}
	}
}

func (hub *wsConnectionHub) Count() int {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()
	return len(hub.connections)
}

func (hub *wsConnectionHub) AddConnection(conn *WsConn) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	hub.connections[conn.ID] = conn
	if conn.ProjectId == wscommon.ProjectIdNone {
		hub.systemMap[conn.ID] = conn
	} else {
		hub.projectIdMap[conn.ProjectId] = append(hub.projectIdMap[conn.ProjectId], conn)
	}
}

func (hub *wsConnectionHub) removeConnection(id string) {
	hub.mutex.Lock()
	defer hub.mutex.Unlock()

	if conn, exists := hub.connections[id]; exists {
		delete(hub.connections, id)

		if conn.ProjectId == wscommon.ProjectIdNone {
			delete(hub.systemMap, conn.ID)
		}

		if conn.ProjectId != wscommon.ProjectIdNone {
			connections := hub.projectIdMap[conn.ProjectId]
			for i, c := range connections {
				if c.ID == id {
					hub.projectIdMap[conn.ProjectId] = append(connections[:i], connections[i+1:]...)
					break
				}
			}

			if len(hub.projectIdMap[conn.ProjectId]) == 0 {
				delete(hub.projectIdMap, conn.ProjectId)
			}
		}
	}
}
