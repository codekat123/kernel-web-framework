#!/bin/sh
# static.sh - Static file serving tests for kernel-web-framework

BASE_URL="http://localhost:8080"

echo "Static file tests..."

curl -s -o /dev/null -w "GET /index.html: %{http_code}\n" "$BASE_URL/index.html"
curl -s -o /dev/null -w "GET /nonexistent.html: %{http_code}\n" "$BASE_URL/nonexistent.html"
curl -s -o /dev/null -w "Path traversal ../etc/passwd: %{http_code}\n" "$BASE_URL/../etc/passwd"
curl -s -o /dev/null -w "Path traversal %2e%2e/etc/passwd: %{http_code}\n" "$BASE_URL/%2e%2e/etc/passwd"
curl -s -o /dev/null -w "Path traversal ../../../../etc/passwd: %{http_code}\n" "$BASE_URL/../../../../../../etc/passwd"
