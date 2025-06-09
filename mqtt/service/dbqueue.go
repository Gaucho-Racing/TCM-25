package service

import (
	"mqtt/database"
	"mqtt/model"
	"mqtt/utils"
	"sync"
	"time"
)

type DBQueue struct {
	messages  chan model.Gr25Message
	batchSize int
	flushTime time.Duration
	mu        sync.RWMutex
	stopped   bool
	wg        sync.WaitGroup
}

var dbQueue *DBQueue

func InitDBQueue() {
	dbQueue = &DBQueue{
		messages:  make(chan model.Gr25Message, 10000), // Buffer for 10k messages
		batchSize: 100,                                 // Batch size of 100 messages
		flushTime: 250 * time.Millisecond,              // Flush every 250ms
	}

	dbQueue.wg.Add(1)
	go dbQueue.worker()

	utils.SugarLogger.Infof("[DB Queue] Initialized with batch size %d", dbQueue.batchSize)
}

func QueueDBWrite(timestamp int, vehicleID, topic string, data []byte, sourceNode string, targetNode string) {
	dbQueue.mu.RLock()
	stopped := dbQueue.stopped
	dbQueue.mu.RUnlock()

	if stopped {
		return
	}

	msg := model.Gr25Message{
		Timestamp:  timestamp,
		VehicleID:  vehicleID,
		Topic:      topic,
		Data:       data,
		Synced:     0,
		SourceNode: sourceNode,
		TargetNode: targetNode,
	}

	select {
	case dbQueue.messages <- msg:
		// Successfully queued
	default:
		// Queue is full, drop message
		utils.SugarLogger.Warnf("[DB] Queue full, dropping message")
	}
}

func (q *DBQueue) worker() {
	defer q.wg.Done()

	batch := make([]model.Gr25Message, 0, q.batchSize)
	ticker := time.NewTicker(q.flushTime)
	defer ticker.Stop()

	for {
		select {
		case msg, ok := <-q.messages:
			if !ok {
				// Channel closed, flush remaining and exit
				if len(batch) > 0 {
					q.writeBatch(batch)
				}
				return
			}

			batch = append(batch, msg)

			// Write batch when full
			if len(batch) >= q.batchSize {
				q.writeBatch(batch)
				batch = batch[:0] // Reset slice
			}

		case <-ticker.C:
			// Periodic flush
			if len(batch) > 0 {
				q.writeBatch(batch)
				batch = batch[:0] // Reset slice
			}
		}
	}
}

func (q *DBQueue) writeBatch(batch []model.Gr25Message) {
	if len(batch) == 0 {
		return
	}

	start := time.Now()
	result := database.DB.CreateInBatches(&batch, len(batch))
	duration := time.Since(start)

	if result.Error != nil {
		utils.SugarLogger.Errorf("[DB] Failed to batch insert %d messages: %v", len(batch), result.Error)
	} else {
		utils.SugarLogger.Debugf("[DB] Inserted %d messages in %v", len(batch), duration)
	}
}

func StopDBQueue() {
	if dbQueue == nil {
		return
	}

	dbQueue.mu.Lock()
	dbQueue.stopped = true
	dbQueue.mu.Unlock()

	close(dbQueue.messages)
	dbQueue.wg.Wait()

	utils.SugarLogger.Infof("[DB] Stopped")
}
