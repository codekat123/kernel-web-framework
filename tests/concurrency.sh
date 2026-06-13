#!/bin/sh
# concurrency.sh - Concurrency tests for kernel-web-framework

BASE_URL="http://localhost:8080"

echo "Concurrency tests..."

echo "100 concurrent requests..."
for i in $(seq 1 100); do
    curl -s -o /dev/null "$BASE_URL/" &
done
wait

echo "Server still responsive:"
curl -s -o /dev/null -w "%{http_code}\n" "$BASE_URL/"
