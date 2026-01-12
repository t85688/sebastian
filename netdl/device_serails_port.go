package netdl

type SerialPortEntry struct {
	Name      string `json:"name"`
	Device    string `json:"device"`
	Interface string `json:"interface" validate:"oneof=rs-232 rs-485-2w,rs-422,rs-485-4w"`
}
