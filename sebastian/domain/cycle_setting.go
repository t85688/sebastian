package domain

type CycleSetting struct {
	AdminBaseTime  AdminBaseTime  `json:"AdminBaseTime"`
	AdminCycleTime AdminCycleTime `json:"AdminCycleTime"`
	TimeSlots      []TimeSlot     `json:"TimeSlots"`
}

type AdminBaseTime struct {
	FractionalSeconds int `json:"FractionalSeconds"`
	Second            int `json:"Second"`
}

type AdminCycleTime struct {
	Denominator int `json:"Denominator"`
	Numerator   int `json:"Numerator"`
}

type TimeSlot struct {
	Active      bool   `json:"Active"`
	Index       int    `json:"Index"`
	Period      int    `json:"Period"`
	TrafficType string `json:"TrafficType"`
}
