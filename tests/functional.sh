#!/bin/sh
# functional.sh - Functional tests for kernel-web-framework

BASE_URL="http://localhost:8080"

echo "Functional tests..."
curl -s -o /dev/null -w "GET /: %{http_code}\n" "$BASE_URL/"
curl -s -o /dev/null -w "GET /hello: %{http_code}\n" "$BASE_URL/hello"
curl -s -o /dev/null -w "GET /nonexistent: %{http_code}\n" "$BASE_URL/nonexistent"
curl -s -o /dev/null -w "POST / (method not checked): %{http_code}\n" -X POST "$BASE_URL/"
curl -s -o /dev/null -w "GET //hello: %{http_code}\n" "$BASE_URL//hello"

if command -v nc >/dev/null 2>&1; then
    echo "Malformed request test..."
    echo "INVALID HTTP REQUEST" | nc -w 2 localhost 8080 2>/dev/null && echo "Server handled malformed request"
fi
