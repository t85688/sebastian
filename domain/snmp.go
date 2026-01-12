package domain

type SNMPVersion int

const (
	SNMPVersionUnknown SNMPVersion = 0
	SNMPVersionV1      SNMPVersion = 1
	SNMPVersionV2c     SNMPVersion = 2
	SNMPVersionV3      SNMPVersion = 3
)

var SNMPVersionStrMap = map[SNMPVersion]string{
	SNMPVersionUnknown: "unknown",
	SNMPVersionV1:      "v1",
	SNMPVersionV2c:     "v2c",
	SNMPVersionV3:      "v3",
}

var SNMPVersionStrParseMap = map[string]SNMPVersion{
	"unknown": SNMPVersionUnknown,
	"v1":      SNMPVersionV1,
	"v2c":     SNMPVersionV2c,
	"v3":      SNMPVersionV3,
}

func (v SNMPVersion) String() string {
	if str, ok := SNMPVersionStrMap[v]; ok {
		return str
	}

	return SNMPVersionStrMap[SNMPVersionUnknown]
}

func ParseSNMPVersion(str string) SNMPVersion {
	if v, ok := SNMPVersionStrParseMap[str]; ok {
		return v
	}

	return SNMPVersionUnknown
}

type SNMPAuthenticationType int

const (
	SNMPAuthenticationTypeUnknown SNMPAuthenticationType = 0
	SNMPAuthenticationTypeNone    SNMPAuthenticationType = 1
	SNMPAuthenticationTypeMD5     SNMPAuthenticationType = 2
	SNMPAuthenticationTypeSHA1    SNMPAuthenticationType = 3
	SNMPAuthenticationTypeSHA256  SNMPAuthenticationType = 4
	SNMPAuthenticationTypeSHA512  SNMPAuthenticationType = 5
)

var SNMPAuthenticationTypeStrMap = map[SNMPAuthenticationType]string{
	SNMPAuthenticationTypeUnknown: "Unknown",
	SNMPAuthenticationTypeNone:    "None",
	SNMPAuthenticationTypeMD5:     "MD5",
	SNMPAuthenticationTypeSHA1:    "SHA1",
	SNMPAuthenticationTypeSHA256:  "SHA256",
	SNMPAuthenticationTypeSHA512:  "SHA512",
}

var SNMPAuthenticationTypeStrParseMap = map[string]SNMPAuthenticationType{
	"Unknown": SNMPAuthenticationTypeUnknown,
	"None":    SNMPAuthenticationTypeNone,
	"MD5":     SNMPAuthenticationTypeMD5,
	"SHA1":    SNMPAuthenticationTypeSHA1,
	"SHA256":  SNMPAuthenticationTypeSHA256,
	"SHA512":  SNMPAuthenticationTypeSHA512,
}

func (t SNMPAuthenticationType) String() string {
	if str, ok := SNMPAuthenticationTypeStrMap[t]; ok {
		return str
	}

	return SNMPAuthenticationTypeStrMap[SNMPAuthenticationTypeUnknown]
}

func ParseSNMPAuthenticationType(str string) SNMPAuthenticationType {
	if v, ok := SNMPAuthenticationTypeStrParseMap[str]; ok {
		return v
	}
	return SNMPAuthenticationTypeUnknown
}

type SNMPEncryptionType int

const (
	SNMPEncryptionTypeUnknown SNMPEncryptionType = 0
	SNMPEncryptionTypeNone    SNMPEncryptionType = 1
	SNMPEncryptionTypeDES     SNMPEncryptionType = 2
	SNMPEncryptionTypeAES     SNMPEncryptionType = 3
)

var SNMPEncryptionTypeStrMap = map[SNMPEncryptionType]string{
	SNMPEncryptionTypeUnknown: "Unknown",
	SNMPEncryptionTypeNone:    "None",
	SNMPEncryptionTypeDES:     "DES",
	SNMPEncryptionTypeAES:     "AES",
}

var SNMPEncryptionTypeStrParseMap = map[string]SNMPEncryptionType{
	"Unknown": SNMPEncryptionTypeUnknown,
	"None":    SNMPEncryptionTypeNone,
	"DES":     SNMPEncryptionTypeDES,
	"AES":     SNMPEncryptionTypeAES,
}

func (t SNMPEncryptionType) String() string {
	if str, ok := SNMPEncryptionTypeStrMap[t]; ok {
		return str
	}

	return SNMPEncryptionTypeStrMap[SNMPEncryptionTypeUnknown]
}

func ParseSNMPEncryptionType(str string) SNMPEncryptionType {
	if v, ok := SNMPEncryptionTypeStrParseMap[str]; ok {
		return v
	}
	return SNMPEncryptionTypeUnknown
}
