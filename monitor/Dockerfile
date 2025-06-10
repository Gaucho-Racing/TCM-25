FROM --platform=$BUILDPLATFORM golang:1.23-alpine AS builder

RUN apk --no-cache add ca-certificates
RUN apk add --no-cache tzdata

WORKDIR /app

COPY go.mod ./
COPY go.sum ./
RUN go mod download

COPY . ./
ARG TARGETOS
ARG TARGETARCH
RUN GOOS=$TARGETOS GOARCH=$TARGETARCH go build -o /tcm_monitor

##
## Deploy
##
FROM python:3.12-slim-bullseye

# Install jetson-stats
RUN python3 -m pip install --upgrade pip
RUN python3 -m pip install -U jetson-stats

WORKDIR /

COPY --from=builder /tcm_monitor /tcm_monitor
COPY /scripts/jetson-stats.py /jetson-stats.py

COPY --from=builder /usr/share/zoneinfo /usr/share/zoneinfo
ENV TZ=America/Los_Angeles

ENTRYPOINT ["/tcm_monitor"]