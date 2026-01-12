package monitor

import (
	"context"
	"testing"
)

func Test_getSessionId(t *testing.T) {
	ctx := context.WithValue(context.Background(), ContextKeySessionId, int64(12345))
	sessionId, ok := getSessionId(ctx)
	if !ok {
		t.Errorf("Expected to find session ID in context")
	}

	if sessionId != 12345 {
		t.Errorf("Expected session ID to be 12345, got %d", sessionId)
	}

	ctx = context.WithValue(context.Background(), ContextKeySessionId, "not-an-int64")
	_, ok = getSessionId(ctx)
	if ok {
		t.Errorf("Expected not to find valid session ID in context")
	}

	ctx = context.Background()
	_, ok = getSessionId(ctx)
	if ok {
		t.Errorf("Expected not to find session ID in empty context")
	}
}
