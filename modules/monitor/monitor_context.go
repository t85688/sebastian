package monitor

import "context"

type MonitorContextKey string

const (
	ContextKeySessionId MonitorContextKey = "SesssionId"
	ContextKeyProjectId MonitorContextKey = "ProjectId"
)

func getContextValue[T any](ctx context.Context, key MonitorContextKey) (T, bool) {
	value, ok := ctx.Value(key).(T)
	return value, ok
}

func getSessionId(ctx context.Context) (int64, bool) {
	sessionID, ok := ctx.Value(ContextKeySessionId).(int64)
	return sessionID, ok
}

func getProjectId(ctx context.Context) (int64, bool) {
	projectID, ok := ctx.Value(ContextKeyProjectId).(int64)
	return projectID, ok
}
