package netdl

type LinkRedundancyType int

const (
	LinkRedundancyTypeNone         LinkRedundancyType = 0
	LinkRedundancyTypeSpanningTree LinkRedundancyType = 1
	LinkRedundancyTypeTubroRing    LinkRedundancyType = 2
	LinkRedundancyTypeTubroRingV2  LinkRedundancyType = 3
	LinkRedundancyTypeTubroChain   LinkRedundancyType = 4
	LinkRedundancyTypeHSR          LinkRedundancyType = 7
	LinkRedundancyTypePRPLanA      LinkRedundancyType = 10
	LinkRedundancyTypePRPLanB      LinkRedundancyType = 11
)

var linkRedundancyTypeMap = map[LinkRedundancyType]string{
	LinkRedundancyTypeNone:         "none",
	LinkRedundancyTypeSpanningTree: "spnningTree",
	LinkRedundancyTypeTubroRing:    "turboRing",
	LinkRedundancyTypeTubroRingV2:  "turboRingV2",
	LinkRedundancyTypeTubroChain:   "turboChain",
	LinkRedundancyTypeHSR:          "hsr",
	LinkRedundancyTypePRPLanA:      "prpLanA",
	LinkRedundancyTypePRPLanB:      "prpLanB",
}

func (rt LinkRedundancyType) String() string {
	if linkRedundancyTypeStr, exists := linkRedundancyTypeMap[rt]; exists {
		return linkRedundancyTypeStr
	}
	return "none"
}
