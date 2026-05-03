#!/bin/bash

# Exit immediately if any command exits with a non-zero status
set -e

echo "==================================================="
echo "       STARTING SYSTEM DEPLOYMENT PROCESS          "
echo "==================================================="

# 1. Install Nginx (For Ubuntu/Debian)
echo "=> [1/5] Checking and installing Nginx..."
if ! command -v nginx &> /dev/null; then
    echo "Nginx is not installed. Installing now..."
    sudo apt-get update
    sudo apt-get install -y nginx
else
    echo "Nginx is already installed, skipping..."
fi

# 2. Copy Nginx Configuration
echo "=> [2/5] Applying Nginx configuration (nginx.conf)..."
# Backup existing config and copy the new one
sudo cp /etc/nginx/nginx.conf /etc/nginx/nginx.conf.bak || true
sudo cp ./nginx.conf /etc/nginx/nginx.conf

# Verify Nginx configuration syntax
echo "Testing Nginx configuration syntax..."
if ! sudo nginx -t; then
    echo "ERROR: Nginx configuration test failed. Stopping deployment to prevent service disruption."
    exit 1
fi

# 3. Start/Restart Nginx
echo "=> [3/5] Restarting Nginx service..."
sudo systemctl restart nginx
sudo systemctl enable nginx

# 4. Run Docker Compose
echo "=> [4/5] Starting Docker Compose..."
if command -v docker-compose &> /dev/null; then
    DOCKER_CMD="docker-compose"
else
    DOCKER_CMD="docker compose"
fi

# Build and run containers in detached mode
$DOCKER_CMD up -d --build

# 5. Check logs and results
echo "=> [5/5] Checking operation status..."
echo "Waiting 5 seconds for services to initialize..."
sleep 5

# Identify containers that are not in 'running' or 'Up' status
ERRORS=$($DOCKER_CMD ps -a | grep -v 'Up' | grep -v -E 'CONTAINER ID|^NAME' || true)

if [ -n "$ERRORS" ]; then
    echo "==================================================="
    echo "WARNING: SERVICE STARTUP ERROR DETECTED"
    echo "==================================================="
    echo "Failed container list:"
    echo "$ERRORS"
    echo "---------------------------------------------------"
    echo "Extracting logs from containers..."
    $DOCKER_CMD logs --tail=30
    echo "==================================================="
    echo "SETUP FAILED - Please check your configuration"
    exit 1
else
    echo "==================================================="
    echo "SETUP SUCCESSFUL! ALL SERVICES ARE RUNNING"
    echo "==================================================="
    $DOCKER_CMD ps
fi
