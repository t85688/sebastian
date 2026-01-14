package httputils

import (
	"encoding/json"
	"fmt"
	"io"
	"math"
	"net/url"
	"strconv"
	"strings"

	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/api"
)

func Bind[T any](c *api.Context, data *T) (res statuscode.Response) {
	if err := c.Bind(data); err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	return statuscode.StatusOK(nil)
}

func GetQuery[T any](c *api.Context, key string, data *T, mandatory bool) statuscode.Response {
	if data == nil {
		return statuscode.StatusBadRequest("nil destination slice", 0)
	}

	s, ok := c.GetQuery(key)
	if mandatory && !ok {
		return statuscode.StatusBadRequest("missing query array: "+key, 0)
	}

	var t T
	switch p := any(&t).(type) {
	case *string:
		*p = s

	case *int64:
		i, err := strconv.ParseInt(s, 10, 64)
		if err != nil {
			return statuscode.StatusBadRequest("invalid int64: "+s, 0)
		}
		*p = i

	case *int:
		i, err := strconv.ParseInt(s, 10, 64)
		if err != nil {
			return statuscode.StatusBadRequest("invalid int: "+s, 0)
		}
		if strconv.IntSize == 32 && (i < math.MinInt32 || i > math.MaxInt32) {
			return statuscode.StatusBadRequest("value overflows int: "+s, 0)
		}
		*p = int(i)

	case *uint64:
		u, err := strconv.ParseUint(s, 10, 64)
		if err != nil {
			return statuscode.StatusBadRequest("invalid uint64: "+s, 0)
		}
		*p = u

	case *uint:
		u, err := strconv.ParseUint(s, 10, 64)
		if err != nil {
			return statuscode.StatusBadRequest("invalid uint: "+s, 0)
		}
		if strconv.IntSize == 32 && u > math.MaxUint32 {
			return statuscode.StatusBadRequest("value overflows uint: "+s, 0)
		}
		*p = uint(u)

	default:
		return statuscode.StatusBadRequest("unsupported element type", 0)
	}

	*data = t

	return statuscode.StatusOK(nil)
}

func GetQueryArray[T any](c *api.Context, key string, data *[]T, mandatory bool) statuscode.Response {
	if data == nil {
		return statuscode.StatusBadRequest("nil destination slice", 0)
	}

	values, ok := c.GetQueryArray(key)
	if mandatory && (!ok || len(values) == 0) {
		return statuscode.StatusBadRequest("missing query array: "+key, 0)
	}

	*data = (*data)[:0]

	for _, s := range values {
		var t T
		switch p := any(&t).(type) {
		case *string:
			*p = s

		case *int64:
			i, err := strconv.ParseInt(s, 10, 64)
			if err != nil {
				return statuscode.StatusBadRequest("invalid int64: "+s, 0)
			}
			*p = i

		case *int:
			i, err := strconv.ParseInt(s, 10, 64)
			if err != nil {
				return statuscode.StatusBadRequest("invalid int: "+s, 0)
			}
			if strconv.IntSize == 32 && (i < math.MinInt32 || i > math.MaxInt32) {
				return statuscode.StatusBadRequest("value overflows int: "+s, 0)
			}
			*p = int(i)

		case *uint64:
			u, err := strconv.ParseUint(s, 10, 64)
			if err != nil {
				return statuscode.StatusBadRequest("invalid uint64: "+s, 0)
			}
			*p = u

		case *uint:
			u, err := strconv.ParseUint(s, 10, 64)
			if err != nil {
				return statuscode.StatusBadRequest("invalid uint: "+s, 0)
			}
			if strconv.IntSize == 32 && u > math.MaxUint32 {
				return statuscode.StatusBadRequest("value overflows uint: "+s, 0)
			}
			*p = uint(u)

		default:
			return statuscode.StatusBadRequest("unsupported element type", 0)
		}

		*data = append(*data, t)
	}

	return statuscode.StatusOK(nil)
}

func ParseInt[T any](c *api.Context, param string, data *T) statuscode.Response {
	if data == nil {
		return statuscode.StatusBadRequest("nil destination", 0)
	}

	raw := strings.TrimSpace(c.Param(param))
	if raw == "" {
		return statuscode.StatusBadRequest("missing parameter: "+param, 0)
	}

	if i64, err := strconv.ParseInt(raw, 10, 64); err == nil {
		switch p := any(data).(type) {
		case *int:
			if strconv.IntSize == 32 && (i64 < math.MinInt32 || i64 > math.MaxInt32) {
				return statuscode.StatusBadRequest("value overflows int", 0)
			}
			*p = int(i64)
		case *int64:
			*p = i64
		case *int32:
			if i64 < math.MinInt32 || i64 > math.MaxInt32 {
				return statuscode.StatusBadRequest("value overflows int32", 0)
			}
			*p = int32(i64)
		case *int16:
			if i64 < math.MinInt16 || i64 > math.MaxInt16 {
				return statuscode.StatusBadRequest("value overflows int16", 0)
			}
			*p = int16(i64)
		case *int8:
			if i64 < math.MinInt8 || i64 > math.MaxInt8 {
				return statuscode.StatusBadRequest("value overflows int8", 0)
			}
			*p = int8(i64)
		case *uint, *uint64, *uint32, *uint16, *uint8:
			if i64 < 0 {
				return statuscode.StatusBadRequest("negative value not allowed for unsigned type", 0)
			}
			u64 := uint64(i64)
			switch q := any(data).(type) {
			case *uint:
				if strconv.IntSize == 32 && u64 > math.MaxUint32 {
					return statuscode.StatusBadRequest("value overflows uint", 0)
				}
				*q = uint(u64)
			case *uint64:
				*q = u64
			case *uint32:
				if u64 > math.MaxUint32 {
					return statuscode.StatusBadRequest("value overflows uint32", 0)
				}
				*q = uint32(u64)
			case *uint16:
				if u64 > math.MaxUint16 {
					return statuscode.StatusBadRequest("value overflows uint16", 0)
				}
				*q = uint16(u64)
			case *uint8:
				if u64 > math.MaxUint8 {
					return statuscode.StatusBadRequest("value overflows uint8", 0)
				}
				*q = uint8(u64)
			}
		default:
			return statuscode.StatusBadRequest("unsupported type", 0)
		}
		return statuscode.StatusOK(nil)
	}

	u64, err := strconv.ParseUint(raw, 10, 64)
	if err != nil {
		return statuscode.StatusBadRequest("invalid integer: "+raw, 0)
	}
	switch p := any(data).(type) {
	case *uint:
		if strconv.IntSize == 32 && u64 > math.MaxUint32 {
			return statuscode.StatusBadRequest("value overflows uint", 0)
		}
		*p = uint(u64)
	case *uint64:
		*p = u64
	case *uint32:
		if u64 > math.MaxUint32 {
			return statuscode.StatusBadRequest("value overflows uint32", 0)
		}
		*p = uint32(u64)
	case *uint16:
		if u64 > math.MaxUint16 {
			return statuscode.StatusBadRequest("value overflows uint16", 0)
		}
		*p = uint16(u64)
	case *uint8:
		if u64 > math.MaxUint8 {
			return statuscode.StatusBadRequest("value overflows uint8", 0)
		}
		*p = uint8(u64)
	default:
		return statuscode.StatusBadRequest("unsupported type for unsigned value", 0)
	}
	return statuscode.StatusOK(nil)
}

func ReadAll[T any](c *api.Context, data *T) statuscode.Response {
	body, err := io.ReadAll(c.Request.Body)
	if err != nil {
		return statuscode.StatusBadRequest(err.Error(), 0)
	}
	defer c.Request.Body.Close()

	switch p := any(data).(type) {
	case *[]byte:
		*p = body
	case *string:
		*p = string(body)
	default:
		if err := json.Unmarshal(body, data); err != nil {
			return statuscode.StatusBadRequest("invalid JSON body", 0)
		}
	}
	return statuscode.StatusOK(nil)
}

func Respond(c *api.Context, res statuscode.Response) {
	c.JSON(res.StatusCode, api.H{"data": res.Data, "message": res.ErrorMessage, "messageCode": res.MessageCode})
}

func QueryParams(params map[string]any) string {
	values := url.Values{}
	for key, value := range params {
		values.Add(key, fmt.Sprintf("%v", value))
	}
	return values.Encode()
}
