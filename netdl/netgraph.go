package netdl

type Node struct {
	Device
}

type Display struct {
	X          int    `json:"x"`
	Y          int    `json:"y"`
	CustomIcon string `json:"customIcon"`
}
