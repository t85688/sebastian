package netdl

type LoginPolicySetting struct {
	LoginMessage          string `json:"loginMessage"`
	LoginFailureMessage   string `json:"loginFailureMessage"`
	AccountLockout        bool   `json:"enableFailureLockout"`
	RetryFailureThreshold int    `json:"retryFailureThreshold"`
	FailureLockoutTime    int    `json:"failureLockoutTime"`
	AutoLogout            int    `json:"autoLogout"`
}
