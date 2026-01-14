package configmapper

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
)

func MapMafSpanningTreeCompatibilityToActSpanningTreeVersion(input string) (string, bool) {
	switch input {
	case "stp":
		return domain.SpanningTreeVersionSTP.String(), true
	case "rstp":
		return domain.SpanningTreeVersionRSTP.String(), true
	default:
		return "", false
	}

}
