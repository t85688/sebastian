package statuscode

import (
	"encoding/json"
	"fmt"
	"net/http"
)

type MessageCode int

const (
	MessageCode_HttpRequestSuccess             MessageCode = 0x01
	MessageCode_HttpRequestCreated             MessageCode = 0x02
	MessageCode_HttpRequestNoContent           MessageCode = 0x03
	MessageCode_HttpRequestUnprocessableEntity MessageCode = 0x04
	MessageCode_HttpRequestFailed              MessageCode = 0x05
	MessageCode_UnmarshalDataFailed            MessageCode = 0x06
)

var MessageCodeToString = map[MessageCode]string{
	MessageCode_HttpRequestSuccess:             "HttpRequestSuccess",
	MessageCode_HttpRequestCreated:             "HttpRequestCreated",
	MessageCode_HttpRequestNoContent:           "HttpRequestNoContent",
	MessageCode_HttpRequestUnprocessableEntity: "HttpRequestUnprocessableEntity",
	MessageCode_HttpRequestFailed:              "HttpRequestFailed",
	MessageCode_UnmarshalDataFailed:            "UnmarshalDataFailed",
}

var StringToMessageCode = map[string]MessageCode{
	"HttpRequestSuccess":             MessageCode_HttpRequestSuccess,
	"HttpRequestCreated":             MessageCode_HttpRequestCreated,
	"HttpRequestNoContent":           MessageCode_HttpRequestNoContent,
	"HttpRequestUnprocessableEntity": MessageCode_HttpRequestUnprocessableEntity,
	"HttpRequestFailed":              MessageCode_HttpRequestFailed,
	"UnmarshalDataFailed":            MessageCode_UnmarshalDataFailed,
}

type Response struct {
	StatusCode   int         `json:"StatusCode"`
	Data         any         `json:"Data"`
	ErrorMessage string      `json:"ErrorMessage"`
	MessageCode  MessageCode `json:"MessageCode"`
}

func (res Response) String() string {
	jsonBytes, _ := json.MarshalIndent(res, "", "  ")
	return string(jsonBytes)
}

func (res *Response) UnmarshalJSONData(data []byte) error {
	return json.Unmarshal(data, &res)
}

type ErrorParamsData struct {
	ErrorParams map[string]any `json:"ErrorParams"`
}

func (res Response) IsSuccess() bool {
	return res.StatusCode >= http.StatusOK && res.StatusCode < http.StatusMultipleChoices
}

func ParseResponse(status_code int, data []byte) (value []byte, res Response) {
	if status_code < http.StatusOK || status_code >= http.StatusMultipleChoices {
		err := json.Unmarshal(data, &res)
		if err != nil {
			return data, StatusBadRequest(err.Error(), 0)
		}
		return data, res
	}
	return data, StatusResponse(status_code, data, "Http request success", MessageCode_HttpRequestSuccess)
}

func StatusResponse(status_code int, data any, message string, message_code MessageCode) Response {
	return Response{
		StatusCode:   status_code,
		Data:         data,
		ErrorMessage: message,
		MessageCode:  message_code,
	}
}

func StatusOK(data any) Response {
	return Response{StatusCode: http.StatusOK,
		Data:         data,
		ErrorMessage: "200 OK",
		MessageCode:  MessageCode_HttpRequestSuccess,
	}
}

func StatusCreated(data any) Response {
	return Response{StatusCode: http.StatusCreated,
		Data:         data,
		ErrorMessage: "201 Created",
		MessageCode:  MessageCode_HttpRequestCreated,
	}
}

func StatusNoContent() Response {
	return Response{
		StatusCode:   http.StatusNoContent,
		ErrorMessage: "204 No Content",
		MessageCode:  MessageCode_HttpRequestNoContent,
	}
}

func StatusBadRequest(message string, message_code MessageCode) Response {
	return Response{
		StatusCode:   http.StatusBadRequest,
		ErrorMessage: message,
		MessageCode:  message_code,
	}
}

func StatusUnprocessableEntity(error_params_data ErrorParamsData, message string) Response {
	return Response{
		StatusCode:   http.StatusBadRequest,
		Data:         error_params_data,
		ErrorMessage: message,
		MessageCode:  MessageCode_HttpRequestUnprocessableEntity,
	}
}

func StatusRequestFailed(message string) Response {
	return Response{
		StatusCode:   http.StatusBadRequest,
		ErrorMessage: message,
		MessageCode:  MessageCode_HttpRequestFailed,
	}
}

func StatusInternalError(message string) Response {
	return Response{
		StatusCode:   http.StatusInternalServerError,
		ErrorMessage: message,
		MessageCode:  MessageCode_HttpRequestFailed,
	}
}

func StatusUnmarshalDataFailed(message string) Response {
	return Response{
		StatusCode:   http.StatusBadRequest,
		ErrorMessage: fmt.Sprintf("Unmarshal JSON data failed: %s", message),
		MessageCode:  MessageCode_UnmarshalDataFailed,
	}
}
