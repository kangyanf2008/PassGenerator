package utils

import "time"

//格式化时间
func FormatTime(t time.Time) string {
	f := "2006-01-02 15-04-05"
	return t.Format(f)
}
