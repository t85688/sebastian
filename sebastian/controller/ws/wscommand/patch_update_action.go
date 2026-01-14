package wscommand

type PatchUpdateAction int

const (
	PatchUpdateActionCreate PatchUpdateAction = iota + 1
	PatchUpdateActionDelete
	PatchUpdateActionUpdate
)

var patchUpdateActionStringMap = map[PatchUpdateAction]string{
	PatchUpdateActionCreate: "Create",
	PatchUpdateActionDelete: "Delete",
	PatchUpdateActionUpdate: "Update",
}

func (action PatchUpdateAction) String() string {
	if str, ok := patchUpdateActionStringMap[action]; ok {
		return str
	}
	return "Unknown"
}
