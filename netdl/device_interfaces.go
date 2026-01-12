package netdl

type InterfaceStatus string

const (
	InterfaceStatusUp   InterfaceStatus = "up"
	InterfaceStatusDown InterfaceStatus = "down"
)

type Interface struct {
	ID          int             `json:"id,omitempty"`
	Type        string          `json:"type,omitempty"`
	Description string          `json:"description,omitempty"`
	Speed       string          `json:"speed,omitempty"`
	Status      InterfaceStatus `json:"status,omitempty"`
	Ports       []int           `json:"ports,omitempty"`
}
