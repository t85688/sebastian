package monitor

import "context"

type MonitorTask func(ctx context.Context) error
