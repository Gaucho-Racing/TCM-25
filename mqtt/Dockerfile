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
RUN GOOS=$TARGETOS GOARCH=$TARGETARCH go build -o /tcm_mqtt

##
## Deploy
##
FROM alpine:3.21

WORKDIR /

COPY --from=builder /tcm_mqtt /tcm_mqtt

COPY --from=builder /usr/share/zoneinfo /usr/share/zoneinfo
ENV TZ=America/Los_Angeles

ENTRYPOINT ["/tcm_mqtt"]