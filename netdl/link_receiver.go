package netdl

import (
	"sync"

	"gitlab.com/moxa/sw/maf/moxa-app-framework/network/share/util/xcopy"

	"github.com/google/go-cmp/cmp"
)

func (link *Link) Compare(mutex *sync.RWMutex, anotherLink *Link) bool {
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	if anotherLink == nil {
		return false
	}

	return cmp.Equal(&(link.LinkInner), &(anotherLink.LinkInner))
}

func (link *Link) DeepCopy(mutex *sync.RWMutex) *Link {
	if mutex != nil {
		mutex.RLock()
		defer mutex.RUnlock()
	}

	newLink := &Link{
		LinkInner: LinkInner{
			ID:           link.ID,
			LinkIdentity: link.LinkIdentity,
			Status:       link.Status,
			Speed:        link.Speed,
		},
	}

	newLink.LinkType = xcopy.DeepCopy(link.LinkType).(map[string]bool)
	newLink.VPN = xcopy.DeepCopy(link.VPN).(*LinkVPN)
	newLink.SFP = xcopy.DeepCopy(link.SFP).(*LinkSFP)
	newLink.Traffic = xcopy.DeepCopy(link.Traffic).(*LinkTraffic)
	newLink.Redundancy = xcopy.DeepCopy(link.Redundancy).(*LinkRedundancy)

	return newLink
}
