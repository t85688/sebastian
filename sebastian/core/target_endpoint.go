package core

import (
	"fmt"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
)

func GetCogsworthHttpEndpoint() string {
	httpPort, err := internal.GetCogsworthHttpPortFromEnviron()
	if err != nil {
		panic("failed to get cogsworth http port from environment: " + err.Error())
	}

	endpoint := fmt.Sprintf("http://localhost:%d", httpPort)

	return endpoint
}

func GetCogsworthAPIPathPrefix() string {
	return "api/v2/web/cogsworth"
}
