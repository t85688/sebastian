package domain

type ManufactureResult struct {
	ManufactureReport []interface{}          `json:"ManufactureReport"` // 空陣列，沒給元素細節就用 interface{}
	Order             string                 `json:"Order"`
	Ready             ManufactureBatchStatus `json:"Ready"`
	Remain            ManufactureBatchStatus `json:"Remain"`
}

type ManufactureBatchStatus struct {
	Batch     int           `json:"Batch"`
	Devices   []interface{} `json:"Devices"` // 空陣列，沒給元素細節用 interface{}
	Status    string        `json:"Status"`
	TimeStamp int64         `json:"TimeStamp"`
}
