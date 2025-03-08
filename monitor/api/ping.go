package api

import (
	"monitor/config"
	"net/http"
	"strconv"
	"time"

	"github.com/gin-gonic/gin"
)

func Ping(c *gin.Context) {
	if millis := c.Query("millis"); millis != "" {
		millisInt, err := strconv.Atoi(millis)
		if err != nil {
			c.JSON(http.StatusBadRequest, gin.H{"error": "Invalid millis"})
			return
		}
		c.JSON(http.StatusOK, gin.H{
			"message": "GR25 TCM Monitor v" + config.Version + " is online!",
			"ping":    millisInt,
			"pong":    time.Now().UnixMilli(),
			"latency": time.Now().UnixMilli() - int64(millisInt),
		})
		return
	}
	c.JSON(http.StatusOK, gin.H{"message": "GR25 TCM Monitor v" + config.Version + " is online!"})
}
