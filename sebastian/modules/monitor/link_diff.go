package monitor

import (
	"fmt"
	"net"
	"strings"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/common/ipcompare"
)

type DiffLink struct {
	ip1   string
	port1 int
	ip2   string
	port2 int
}

type LinkInvalidReason int

const (
	LinkInvalidReasonNone LinkInvalidReason = iota
	LinkInvalidReasonSameIP
	LinkInvalidReasonInvalidIPFormat
	LinkInvalidReasonInvalidPort
	LinkInvalidReasonNotSupportedIPVersion
)

func NewDiffLink(ip1 string, port1 int, ip2 string, port2 int) *DiffLink {
	compareIPResult := ipcompare.CompareIPByString(ip1, ip2)

	if compareIPResult == ipcompare.CompareIPResultGreater {
		return &DiffLink{
			ip1:   ip2,
			port1: port2,
			ip2:   ip1,
			port2: port1,
		}
	}

	return &DiffLink{
		ip1:   ip1,
		port1: port1,
		ip2:   ip2,
		port2: port2,
	}
}

func (link *DiffLink) String() string {
	return fmt.Sprintf("%s:%d-%s:%d", link.ip1, link.port1, link.ip2, link.port2)
}

func (link *DiffLink) PeerOf(endpoint string) string {
	if endpoint == link.FromEndpoint() {
		return link.ToEndpoint()
	}

	if endpoint == link.ToEndpoint() {
		return link.FromEndpoint()
	}

	return ""
}

func (link *DiffLink) FromIP() string {
	return link.ip1
}

func (link *DiffLink) FromPort() int {
	return link.port1
}

func (link *DiffLink) ToIP() string {
	return link.ip2
}

func (link *DiffLink) ToPort() int {
	return link.port2
}

func (link *DiffLink) FromEndpoint() string {
	return fmt.Sprintf("%s:%d", link.ip1, link.port1)
}

func (link *DiffLink) ToEndpoint() string {
	return fmt.Sprintf("%s:%d", link.ip2, link.port2)
}

func (link *DiffLink) IsValid() (bool, LinkInvalidReason) {
	parsedIP1 := net.ParseIP(link.ip1)
	if parsedIP1 == nil {
		return false, LinkInvalidReasonInvalidIPFormat
	}

	if parsedIP1.To4() == nil && parsedIP1.To16() != nil {
		return false, LinkInvalidReasonNotSupportedIPVersion
	}

	parsedIP2 := net.ParseIP(link.ip2)
	if parsedIP2 == nil {
		return false, LinkInvalidReasonInvalidIPFormat
	}

	if parsedIP2.To4() == nil && parsedIP2.To16() != nil {
		return false, LinkInvalidReasonNotSupportedIPVersion
	}

	if link.port1 <= 0 {
		return false, LinkInvalidReasonInvalidPort
	}

	if link.port2 <= 0 {
		return false, LinkInvalidReasonInvalidPort
	}

	if link.ip1 == link.ip2 {
		return false, LinkInvalidReasonSameIP
	}

	return true, LinkInvalidReasonNone
}

type LinkDiffResult struct {
	InvalidActualLinks          map[string]*DiffLink
	InvalidBaselineLinks        map[string]*DiffLink
	UnchangedLinks              map[string]*DiffLink
	RemovedLinks                map[string]*DiffLink
	NewLinks                    map[string]*DiffLink
	RewiredLinks                map[string]*DiffLink
	BaselinePortConflictedLinks map[string][]*DiffLink
	ActualPortConflictedLinks   map[string][]*DiffLink
}

func (result *LinkDiffResult) String() string {
	sb := strings.Builder{}
	sb.WriteString("Unchanged Links:\n")
	for _, link := range result.UnchangedLinks {
		sb.WriteString(link.String() + "\n")
	}

	sb.WriteString("\n")
	sb.WriteString("Removed Links:\n")
	for _, link := range result.RemovedLinks {
		sb.WriteString(link.String() + "\n")
	}

	sb.WriteString("\n")
	sb.WriteString("New Links:\n")
	for _, link := range result.NewLinks {
		sb.WriteString(link.String() + "\n")
	}

	sb.WriteString("\n")
	sb.WriteString("Rewired Links:\n")
	for _, link := range result.RewiredLinks {
		sb.WriteString(link.String() + "\n")
	}

	sb.WriteString("\n")
	sb.WriteString("Invalid Baseline Links:\n")
	for _, link := range result.InvalidBaselineLinks {
		sb.WriteString(link.String() + "\n")
	}

	sb.WriteString("\n")
	sb.WriteString("Invalid Actual Links:\n")
	for _, link := range result.InvalidActualLinks {
		sb.WriteString(link.String() + "\n")
	}

	return sb.String()
}

type LinkDiffType int

const (
	Unknown LinkDiffType = iota
	Unchanged
	Removed
	New
	Rewired
	Invalid
	PortConflict
)

func ComputeLinkDiff(baselineLinkMap map[string]*DiffLink, actualLinkMap map[string]*DiffLink) (*LinkDiffResult, error) {
	if baselineLinkMap == nil {
		return nil, fmt.Errorf("baselineLinkMap is nil")
	}

	if actualLinkMap == nil {
		return nil, fmt.Errorf("actualLinkMap is nil")
	}

	result := &LinkDiffResult{}

	// check invalid link
	invalidBaselineLinks := make(map[string]*DiffLink, 0)
	for baselineLinkStr, baselineLink := range baselineLinkMap {
		isValidLink, _ := baselineLink.IsValid()
		if !isValidLink {
			invalidBaselineLinks[baselineLinkStr] = baselineLink
			delete(baselineLinkMap, baselineLinkStr)
		}
	}

	invalidActualLinks := make(map[string]*DiffLink, 0)
	for actualLinkStr, actualLink := range actualLinkMap {
		isValidLink, _ := actualLink.IsValid()
		if !isValidLink {
			invalidActualLinks[actualLinkStr] = actualLink
			delete(actualLinkMap, actualLinkStr)
		}
	}

	unchangedLinks := make(map[string]*DiffLink, 0)
	removedLinks := make(map[string]*DiffLink, 0)
	newLinks := make(map[string]*DiffLink, 0)
	rewiredLinks := make(map[string]*DiffLink, 0)

	// build baseline link port map
	baselineLinkEndpointMap := make(map[string]*DiffLink)
	baselinePortConflictedLinks := make(map[string][]*DiffLink, 0)
	baselineConflictEndpointSet := make(map[string]bool)
	for _, link := range baselineLinkMap {
		if existedLink, exists := baselineLinkEndpointMap[link.FromEndpoint()]; exists {
			baselineConflictEndpointSet[link.FromEndpoint()] = true

			if baselinePortConflictedLinks[link.FromEndpoint()] == nil {
				baselinePortConflictedLinks[link.FromEndpoint()] = []*DiffLink{
					existedLink,
					link,
				}
			} else {
				baselinePortConflictedLinks[link.FromEndpoint()] = append(baselinePortConflictedLinks[link.FromEndpoint()], link)
			}

		} else {
			baselineLinkEndpointMap[link.FromEndpoint()] = link
		}

		if existedLink, exists := baselineLinkEndpointMap[link.ToEndpoint()]; exists {
			baselineConflictEndpointSet[link.ToEndpoint()] = true
			if baselinePortConflictedLinks[link.ToEndpoint()] == nil {
				baselinePortConflictedLinks[link.ToEndpoint()] = []*DiffLink{
					existedLink,
					link,
				}
			} else {
				baselinePortConflictedLinks[link.ToEndpoint()] = append(baselinePortConflictedLinks[link.ToEndpoint()], link)
			}
		} else {
			baselineLinkEndpointMap[link.ToEndpoint()] = link
		}
	}

	// build actual link port map
	actualLinkEndpointMap := make(map[string]*DiffLink)
	acturalPortConflictedLinks := make(map[string][]*DiffLink, 0)
	actualConflictEndpointSet := make(map[string]bool)
	for _, link := range actualLinkMap {
		if existedLink, exists := actualLinkEndpointMap[link.FromEndpoint()]; exists {
			actualConflictEndpointSet[link.FromEndpoint()] = true
			if acturalPortConflictedLinks[link.FromEndpoint()] == nil {
				acturalPortConflictedLinks[link.FromEndpoint()] = []*DiffLink{
					existedLink,
					link,
				}
			} else {
				acturalPortConflictedLinks[link.FromEndpoint()] = append(acturalPortConflictedLinks[link.FromEndpoint()], link)
			}
		}

		if existedLink, exists := actualLinkEndpointMap[link.ToEndpoint()]; exists {
			actualConflictEndpointSet[link.ToEndpoint()] = true
			if acturalPortConflictedLinks[link.ToEndpoint()] == nil {
				acturalPortConflictedLinks[link.ToEndpoint()] = []*DiffLink{
					existedLink,
					link,
				}
			} else {
				acturalPortConflictedLinks[link.ToEndpoint()] = append(acturalPortConflictedLinks[link.ToEndpoint()], link)
			}
		}
	}

	for _, baselineLink := range baselineLinkMap {
		baselineLinkStr := baselineLink.String()
		if _, exists := actualLinkMap[baselineLinkStr]; exists {
			unchangedLinks[baselineLinkStr] = baselineLink
		} else {
			removedLinks[baselineLinkStr] = baselineLink
		}
	}

	for _, actualLink := range actualLinkMap {
		actualLinkStr := actualLink.String()

		if _, exists := baselineLinkMap[actualLinkStr]; !exists {
			newLinks[actualLinkStr] = actualLink
		}
	}

	for _, newLink := range newLinks {
		fromEndpoint := newLink.FromEndpoint()
		toEndpoint := newLink.ToEndpoint()

		_, fromExists := baselineLinkEndpointMap[fromEndpoint]
		if fromExists {
			rewiredLinks[newLink.String()] = newLink
		}

		_, toExists := baselineLinkEndpointMap[toEndpoint]
		if toExists {
			rewiredLinks[newLink.String()] = newLink
		}

	}

	result.InvalidActualLinks = invalidActualLinks
	result.InvalidBaselineLinks = invalidBaselineLinks
	result.UnchangedLinks = unchangedLinks
	result.RemovedLinks = removedLinks
	result.NewLinks = newLinks
	result.RewiredLinks = rewiredLinks
	result.BaselinePortConflictedLinks = baselinePortConflictedLinks
	result.ActualPortConflictedLinks = acturalPortConflictedLinks

	return result, nil
}
