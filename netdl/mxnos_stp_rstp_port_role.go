package netdl

type RstpPortRole int

const (
	RstpPortRoleUnknown RstpPortRole = iota - 1
	RstpPortRoleDisabled
	RstpPortRoleAlternate
	RstpPortRoleBackup
	RstpPortRoleRoot
	RstpPortRoleDesignated
	RstpPortRoleMaster
)

var RstpPortRoleMap = map[RstpPortRole]string{
	RstpPortRoleUnknown:    "unknown",
	RstpPortRoleDisabled:   "disabled",
	RstpPortRoleAlternate:  "alternate",
	RstpPortRoleBackup:     "backup",
	RstpPortRoleRoot:       "root",
	RstpPortRoleDesignated: "designated",
	RstpPortRoleMaster:     "master",
}

func (role RstpPortRole) String() string {
	if str, ok := RstpPortRoleMap[role]; ok {
		return str
	}
	return "unknown"
}
