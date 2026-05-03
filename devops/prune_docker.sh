#!/bin/bash

echo "========================================"
echo "      DOCKER CLEANUP SCRIPT             "
echo "========================================"

# Stop all running containers
echo "=> Stopping all Docker containers..."
CONTAINERS=$(docker ps -aq)
if [ -n "$CONTAINERS" ]; then
    docker stop $CONTAINERS
    echo "All containers stopped."
else
    echo "No containers found to stop."
fi

# Run docker system prune
echo "=> Executing docker system prune..."
docker system prune -a -f --volumes

echo "========================================"
echo "      CLEANUP COMPLETED                 "
echo "========================================"
