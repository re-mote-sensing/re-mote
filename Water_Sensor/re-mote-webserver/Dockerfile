# syntax=docker/dockerfile:1

# Build docker image
FROM golang:1.16-buster AS build

WORKDIR /app

# Update and install npm
RUN apt-get update && apt-get upgrade -y && \
    apt-get install -y nodejs \
    npm 

# Install yarn
RUN npm install -g yarn

# Install influxdb
RUN apt-get install influxdb -y

# Build Go Backend Server
ENV GOBIN /go/bin
COPY backend/go.mod ./backend/
COPY backend/go.sum ./backend/
RUN cd backend && go mod download && cd ..
COPY . .
RUN cd backend && go build main.go && cd ..

# Build React Frontend
RUN cd frontend && yarn install && yarn build && cd ..

# Port 8080: Webserver
# Port 8086: influxdb
EXPOSE 8080 8086

# influxDB Database files
# https://docs.influxdata.com/influxdb/v1.8/concepts/file-system-layout/
VOLUME ["/var/lib/influxdb"]

# Start server and database
ENTRYPOINT ./run.sh