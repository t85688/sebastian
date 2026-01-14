package ws

import (
	"net/http"
	"strconv"

	"github.com/gin-gonic/gin"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wscommon"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/controller/ws/wsconn"
	afhttp "gitlab.com/moxa/sw/maf/moxa-app-framework/pkg/protocol/http"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func WebsocketRouteV2(c *api.Context) {
	logger.Info("Received WebSocket connection request")
	var conn, err = afhttp.NewWebSocketConnection(c.Writer, c.Request, nil)
	if err != nil {
		c.JSON(http.StatusBadGateway, gin.H{
			"msg":   "failed to upgrade websocket",
			"error": err.Error(),
		})
		return
	}
	logger.Info("WebSocket connection Upgrade successful")

	var projectId int64 = wscommon.ProjectIdNone
	projectIdStr := c.Param("projectId")
	if projectIdStr != "" {
		projectId, err = strconv.ParseInt(projectIdStr, 10, 64)
		if err != nil {
			c.JSON(http.StatusBadRequest, gin.H{
				"msg":   "invalid project ID",
				"error": err.Error(),
			})
			return
		}

		if projectId <= 0 {
			c.JSON(http.StatusBadRequest, gin.H{
				"msg":   "project ID must be a positive integer",
				"error": "invalid project ID",
			})
			return
		}
	}

	wsConn := wsconn.NewWsConn(conn, projectId)
	defer wsConn.Close()

	wsconn.GetDefaultHubInstance().AddConnection(wsConn)

	logger.Infof("WebSocket connection established, ID: %s", wsConn.ID)

	wsConn.Wg.Add(1)
	go wsConn.ReadWorker()
	wsConn.Wg.Add(1)
	go wsConn.WriteWorker()

	wsConn.Wg.Wait()

	logger.Infof("WebSocket connection closed, ID: %s", wsConn.ID)
}
