package api

import (
	"mqtt/config"
	"net/http"

	"github.com/gin-gonic/gin"
)

func Ping(c *gin.Context) {
	c.JSON(http.StatusOK, gin.H{"message": "GR25 TCM MQTT v" + config.Version + " is online!"})
}
