package domain

type SkuQuantity struct {
	Quantity        int64  `json:"Quantity"`
	InTopologyCount int64  `json:"InTopologyCount"`
	Price           string `json:"Price"`
	Currency        string `json:"Currency"`
	TotalPrice      string `json:"TotalPrice"`
}

type CurrencyEnum int

const (
	CurrencyEnumTWD CurrencyEnum = iota
	CurrencyEnumUSD
	CurrencyEnumEUR
)

var CurrencyEnumToString = map[CurrencyEnum]string{
	CurrencyEnumTWD: "TWD",
	CurrencyEnumUSD: "USD",
	CurrencyEnumEUR: "EUR",
}

func (currency CurrencyEnum) String() string {
	if str, ok := CurrencyEnumToString[currency]; ok {
		return str
	}

	return "Unknown"
}
