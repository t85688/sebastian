package configmapper

import (
	"fmt"
	"strings"
	"sync"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/core"
)

var (
	mutex                   sync.Mutex
	powerModuleNameAliasMap map[string]string
	etherModuleNameAliasMap map[string]string
	powerModuleMap          map[string]int64
	etherModuleMap          map[string]int64
)

func initPowerModuleList() error {
	mutex.Lock()
	defer mutex.Unlock()

	if powerModuleMap != nil {
		return nil
	}

	idMap, status := core.GetPowerModuleNameIdMap()
	if !status.IsSuccess() {
		return fmt.Errorf("%s", status.ErrorMessage)
	}

	logger.Infof("get powerModuleMap successfully: %+v", idMap)

	aliasMap := make(map[string]string)
	result := make(map[string]int64, len(idMap))
	for name, id := range idMap {
		aliasMap[strings.ToUpper(name)] = name
		result[name] = id
	}

	powerModuleNameAliasMap = aliasMap
	powerModuleMap = result

	logger.Infof("powerModule alias map: %+v", powerModuleMap)

	return nil
}

func initEtherModuleList() error {
	mutex.Lock()
	defer mutex.Unlock()

	if etherModuleMap != nil {
		return nil
	}

	ethernetModuleMap, status := core.GetEthernetModuleNameIdMap()
	if !status.IsSuccess() {
		return fmt.Errorf("%s", status.ErrorMessage)
	}

	logger.Infof("get ethernetModuleMap successfully: %+v", ethernetModuleMap)

	aliasMap := make(map[string]string)
	result := make(map[string]int64, len(ethernetModuleMap))
	for name, id := range ethernetModuleMap {
		aliasMap[strings.ToUpper(name)] = name
		result[name] = id
	}

	etherModuleNameAliasMap = aliasMap
	etherModuleMap = result

	logger.Infof("ethernetModule alias map: %+v", aliasMap)

	return nil
}
