package netdl

type LinkType int

const (
	LinkTypeNone LinkType = iota
	LinkTypeTrunk
	LinkTypeWireless
	LinkTypeSFP
	LinkTypeRstpRedundancy
	LinkTypeMgmtEndpoint
	LinkTypePRPLanA
	LinkTypePRPLanB
	LinkTypeHSR
)

const (
	LinkTypeSFPString            = "sfp"
	LinkTypeRstpRedundancyString = "rstpRedundancy"
	LInkTypeWirelessString       = "wireless"
	LinkTypeMgmtEndpointString   = "mgmtEndpoint"
	LinkTypePRPLanAString        = "prpLanA"
	LinkTypePRPLanBString        = "prpLanB"
	LinkTypeHSRString            = "hsr"
)

var (
	linkTypeMap = map[LinkType]string{
		LinkTypeNone:           "none",
		LinkTypeSFP:            LinkTypeSFPString,
		LinkTypeRstpRedundancy: LinkTypeRstpRedundancyString,
		LinkTypeMgmtEndpoint:   LinkTypeMgmtEndpointString,
		LinkTypePRPLanA:        LinkTypePRPLanAString,
		LinkTypePRPLanB:        LinkTypePRPLanBString,
		LinkTypeHSR:            LinkTypeHSRString,
	}
)

func (linkType LinkType) String() string {
	if linkTypeStr, ok := linkTypeMap[linkType]; ok {
		return linkTypeStr
	}

	return "none"
}

func NewLinkType(linkTypeStr string) LinkType {
	for k, v := range linkTypeMap {
		if v == linkTypeStr {
			return k
		}
	}

	return LinkTypeNone
}
