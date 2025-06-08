#!/bin/bash

# Get the version from config.go
VERSION=$(grep 'var Version =' config/config.go | cut -d '"' -f 2)

# Check if version was successfully extracted
if [ -z "$VERSION" ]
  then
    echo "Error: Could not extract version from config/config.go"
    exit 1
fi

# Check if docker is installed
if ! [ -x "$(command -v docker)" ]; then
  echo 'Error: docker is not installed.' >&2
  exit 1
fi

echo "Building container for GR25 TCM MQTT v$VERSION"
# Build the docker container
docker build -t gauchoracing/gr25_tcm_mqtt:"$VERSION" -t gauchoracing/gr25_tcm_mqtt:latest --platform linux/amd64,linux/arm64 --push --progress=plain .

echo "Container deployed successfully"