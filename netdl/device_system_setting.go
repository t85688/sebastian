package netdl

type SyslogSetting struct {
	LoggingEnable bool           `json:"loggingEnable"`
	ServerTable   []SyslogServer `json:"serverTable"`
}

type SyslogServer struct {
	Enable         bool   `json:"enable"`
	Address        string `json:"address,omitempty"`
	UdpPort        int    `json:"udpPort"`
	Authentication string `json:"authentication"`
}

type SystemTimeConfig struct {
	ClockSource      string `json:"clockSource" validate:"omitempty,oneof=local sntp ntp ptp"`
	FirstTimeServer  string `json:"firstTimeServer"`
	SecondTimeServer string `json:"secondTimeServer"`
}

type SetSystemDaylightSavingInput struct {
	TimeZone string      `json:"timeZone"`
	Enable   bool        `json:"enable"`
	Offset   string      `json:"offset"`
	RTime    *RegionTime `json:"regionTime,omitempty"`
	YTime    *YearTime   `json:"yearTime,omitempty"`
}

type RegionTime struct {
	Start DayLightTimeRegionMode `json:"start"`
	End   DayLightTimeRegionMode `json:"end"`
}

type YearTime struct {
	Start DayLightTimeYearMode `json:"start"`
	End   DayLightTimeYearMode `json:"end"`
}

type DayLightTimeRegionMode struct {
	Day    int `json:"day"`
	Hour   int `json:"hour"`
	Minute int `json:"minute"`
	Month  int `json:"month"`
	Week   int `json:"week"`
}

type DayLightTimeYearMode struct {
	Date   int `json:"date"`
	Hour   int `json:"hour"`
	Minute int `json:"minute"`
	Month  int `json:"month"`
	Year   int `json:"year"`
}
