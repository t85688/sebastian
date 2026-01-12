package netdl

type LoopProtectionSetting struct {
	Enable         bool  `json:"enable"`
	DetectInterval int32 `json:"detectInterval"`
}
