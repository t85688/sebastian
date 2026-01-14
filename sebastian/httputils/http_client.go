package httputils

import (
	"bytes"
	"context"
	"fmt"
	"io"
	"net/http"
	"time"

	"github.com/go-resty/resty/v2"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal"
	"gitlab.com/moxa/sw/act/chamberlain-sebastian-golang/internal/sebastian/statuscode"
	"gitlab.com/moxa/sw/maf/moxa-app-framework/runtime/logging"
)

const (
	chamberlainToken                   = "MoxaTech89191230"
	shouldDisableHttpProxyForLocalhost = false
)

var (
	logger     = logging.NewWithField("origin", "httputils")
	httpClient *resty.Client
)

func InitClient() {
	httpClient = resty.New()

	// 1. 設定全域超時 (非常重要！)
	// 避免請求因為網路問題無限期卡住
	httpClient.SetTimeout(10 * time.Second)

	// 2. 設定重試機制 (可選，但在微服務中很有用)
	httpClient.SetRetryCount(3).
		SetRetryWaitTime(500 * time.Millisecond).
		SetRetryMaxWaitTime(2 * time.Second)

	// 3. 配置底層的 Connection Pool (Transport)
	// 這是控制並發連線數的關鍵
	transport := &http.Transport{
		MaxIdleConns:        100,              // 總共保留的最大閒置連線數
		MaxIdleConnsPerHost: 20,               // 每個 Host 保留的最大閒置連線數
		IdleConnTimeout:     90 * time.Second, // 閒置連線存活時間
	}
	httpClient.SetTransport(transport)
}

func GetClient() *resty.Client {
	return httpClient
}

func HttpClient(method, url string, request []byte) (data []byte, res statuscode.Response) {
	proxy := internal.ProxyFromEnvironment
	if shouldDisableHttpProxyForLocalhost {
		proxy = http.ProxyFromEnvironment
	}
	client := &http.Client{
		Transport: &http.Transport{
			DisableKeepAlives: true,
			Proxy:             proxy,
		},
	}

	var body io.Reader
	if request != nil {
		body = bytes.NewBuffer(request)
	} else {
		body = nil
	}

	// 建立 request
	req, err := http.NewRequest(method, url, body)
	if err != nil {
		return data, statuscode.StatusRequestFailed(fmt.Sprintf("Request error: %v", err))
	}
	req.Header.Add("Content-Type", "application/json")
	req.Header.Set("Authorization", fmt.Sprintf("Bearer %s", chamberlainToken))

	// 加上 context timeout
	ctx, cancel := context.WithTimeout(context.Background(), 5*time.Second)
	defer cancel()

	// 發送 request
	resp, err := client.Do(req.WithContext(ctx))
	if err != nil {
		return data, statuscode.StatusRequestFailed(fmt.Sprintf("HTTP request failed: %v", err))
	}
	defer resp.Body.Close()

	// 讀 response body
	data, err = io.ReadAll(resp.Body)
	if err != nil {
		logger.Debug("[HTTP Client] Read error: %v", err)
		return data, statuscode.StatusRequestFailed(fmt.Sprintf("Read body failed: %v", err))
	}

	// 成功回傳
	return statuscode.ParseResponse(resp.StatusCode, data)
}
