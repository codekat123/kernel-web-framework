#!/bin/sh
# security.sh - Security tests for kernel-web-framework

BASE_URL="http://localhost:8080"

echo "Security tests..."

curl -s -o /dev/null -w "Path traversal ../../../etc/passwd: %{http_code}\n" "$BASE_URL/../../../etc/passwd"
curl -s -o /dev/null -w "Path traversal %2e%2e%2fetc%2fpasswd: %{http_code}\n" "$BASE_URL/%2e%2e%2fetc%2fpasswd"
curl -s -o /dev/null -w "Null byte injection: %{http_code}\n" "$BASE_URL/index.html%00.txt"
curl -s -o /dev/null -w "SQL injection: %{http_code}\n" -X POST -H "Content-Type: application/json" -d '{"username":"admin'\'' OR '\''1'\''='\''1","password":"test"}' "$BASE_URL/login"
curl -s -o /dev/null -w "XSS attempt: %{http_code}\n" -X POST -H "Content-Type: application/json" -d '{"username":"<script>alert(1)</script>","password":"test"}' "$BASE_URL/register"
curl -s -o /dev/null -w "Invalid JSON: %{http_code}\n" -X POST -H "Content-Type: application/json" -d '{"invalid": json}' "$BASE_URL/login"

if command -v nc >/dev/null 2>&1; then
    echo "Malformed request line test..."
    echo "INVALID REQUEST LINE" | nc -w 2 localhost 8080 2>/dev/null && echo "Server handled malformed request"
fi
