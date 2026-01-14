package configmapper

import (
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/domain"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/moxa/device/sdk/schema"
)

func MapActSNMPVersionToMafSNMPVersion(actSNMPVersionStr string) (string, bool) {
	actSNMPVersion := domain.ParseSNMPVersion(actSNMPVersionStr)

	if actSNMPVersion == domain.SNMPVersionUnknown {
		return "", false
	}

	switch actSNMPVersion {

	case domain.SNMPVersionV1:
		return schema.SNMPVersion1, true
	case domain.SNMPVersionV2c:
		return schema.SNMPVersion2c, true
	case domain.SNMPVersionV3:
		return schema.SNMPVersion3, true
	default:
		return "", false
	}
}

func MapActSNMPAuthenticationTypeToMafSNMPAuthenticationType(actAuthTypeStr string) (string, bool) {
	actAuthType := domain.ParseSNMPAuthenticationType(actAuthTypeStr)
	if actAuthType == domain.SNMPAuthenticationTypeUnknown {
		return "", false
	}

	switch actAuthType {
	case domain.SNMPAuthenticationTypeNone:
		return schema.SNMPAuthNone, true
	case domain.SNMPAuthenticationTypeMD5:
		return schema.SNMPAuthMD5, true
	case domain.SNMPAuthenticationTypeSHA1:
		return schema.SNMPAuthSHA, true
	case domain.SNMPAuthenticationTypeSHA256:
		return schema.SNMPAuthSHA256, true
	case domain.SNMPAuthenticationTypeSHA512:
		return schema.SNMPAuthSHA512, true
	default:
		return "", false
	}
}

func MapActSNMPEncryptionTypeToMafSNMPEncryptionType(actEncTypeStr string) (string, bool) {
	actEncType := domain.ParseSNMPEncryptionType(actEncTypeStr)
	if actEncType == domain.SNMPEncryptionTypeUnknown {
		return "", false
	}

	switch actEncType {
	case domain.SNMPEncryptionTypeNone:
		return schema.SNMPPrivNone, true
	case domain.SNMPEncryptionTypeDES:
		return schema.SNMPPrivDES, true
	case domain.SNMPEncryptionTypeAES:
		return schema.SNMPPrivAES, true
	default:
		return "", false
	}
}
