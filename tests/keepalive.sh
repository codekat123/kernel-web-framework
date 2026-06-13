#!/bin/sh
# keepalive.sh - HTTP Keep-Alive tests for kernel-web-framework

BASE_URL="http://localhost:8080"

echo "Keep-alive tests..."

if command -v nc >/dev/null 2>&1; then
    echo "Multiple requests on same connection..."
    response=$(printf 'GET / HTTP/1.1\r\nHost: localhost\r\n\r\nGET /hello HTTP/1.1\r\nHost: localhost\r\n\r\n' | nc -w 5 localhost 8080 2>/dev/null)
    echo "$response" | grep -c "HTTP/1.1" | xargs echo "Responses received:"
fi

curl -s -I "$BASE_URL/" | grep -i "Connection"
