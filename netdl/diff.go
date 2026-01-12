package netdl

import "fmt"

type DiffType int

const (
	DiffTypeDeviceAdded DiffType = iota + 1
	DiffTypeDeviceChanged
	DiffTypeDeviceRemoved
	DiffTypeLinkAdded
	DiffTypeLinkChanged
	DiffTypeLinkRemoved
)

var diffTypeMap = map[DiffType]string{
	DiffTypeDeviceAdded:   "deviceAdded",
	DiffTypeDeviceChanged: "deviceUpdated",
	DiffTypeDeviceRemoved: "deviceDeleted",
	DiffTypeLinkAdded:     "linkAdded",
	DiffTypeLinkChanged:   "linkUpdated",
	DiffTypeLinkRemoved:   "linkDeleted",
}

func (dt DiffType) String() string {
	if str, ok := diffTypeMap[dt]; ok {
		return str
	}

	return ""
}

func NewDiffUnit(diffType DiffType, data any) (*DiffUnit, error) {
	if diffTypeStr := diffType.String(); diffTypeStr == "" {
		return nil, fmt.Errorf("not supported DiffUnit")
	}

	switch data := data.(type) {
	case *Device:
		unit := &DiffUnit{
			DiffType: diffType,
			Device:   data,
		}
		return unit, nil
	case *Link:
		unit := &DiffUnit{
			DiffType: diffType,
			Link:     data,
		}
		return unit, nil
	default:

		return nil, fmt.Errorf("not supported data type")
	}
}

type DiffUnit struct {
	DiffType DiffType `json:"diffType"`
	Device   *Device  `json:"device"`
	Link     *Link    `json:"link"`
}

type DiffSetDevice struct {
	Added   []*Device `json:"added,omitempty"`
	Updated []*Device `json:"updated,omitempty"`
	Deleted []*Device `json:"deleted,omitempty"`
}

type DiffSetLink struct {
	Added   []*Link `json:"added,omitempty"`
	Updated []*Link `json:"updated,omitempty"`
	Deleted []*Link `json:"deleted,omitempty"`
}

type DiffOutput struct {
	Devices *DiffSetDevice `json:"devices,omitempty"`
	Links   *DiffSetLink   `json:"links,omitempty"`
}

func ToAddedDevice(data any) map[string]map[string][]any {
	return map[string]map[string][]any{
		"added": {
			"devices": []any{data},
		},
	}
}

func ToChangedDevice(data any) map[string]map[string][]any {
	return map[string]map[string][]any{
		"changed": {
			"devices": []any{data},
		},
	}
}

func ToRmDevice(data any) map[string]map[string][]any {
	return map[string]map[string][]any{
		"removed": {
			"devices": []any{data},
		},
	}
}

func ToAddedLink(data any) map[string]map[string][]any {
	return map[string]map[string][]any{
		"added": {
			"links": []any{data},
		},
	}
}

func ToChangedLink(data any) map[string]map[string][]any {
	return map[string]map[string][]any{
		"changed": {
			"links": []any{data},
		},
	}
}

func ToRmLink(data any) map[string]map[string][]any {
	return map[string]map[string][]any{
		"removed": {
			"links": []any{data},
		},
	}
}
