package configmapper

import "gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"

func MapMafTransportProtocolToActTransportProtocol(mafProtocolStr string) (string, bool) {
	switch mafProtocolStr {
	case "udp":
		return domain.TransportLayerProtocolUDP.String(), true
	case "tcp":
		return domain.TransportLayerProtocolTCP.String(), true
	default:
		return "", false
	}
}
