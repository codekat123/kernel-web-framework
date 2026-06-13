#!/bin/sh
# auth.sh - Authentication tests for kernel-web-framework

BASE_URL="http://localhost:8080"

echo "Auth tests..."

username="testuser_$(date +%s)"
password="testpassword123"

# Register
curl -s -o /dev/null -w "Register valid: %{http_code}\n" -X POST \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$username\",\"password\":\"$password\"}" \
    "$BASE_URL/register"

curl -s -o /dev/null -w "Register missing username: %{http_code}\n" -X POST \
    -H "Content-Type: application/json" \
    -d '{"password":"test"}' \
    "$BASE_URL/register"

curl -s -o /dev/null -w "Register missing password: %{http_code}\n" -X POST \
    -H "Content-Type: application/json" \
    -d '{"username":"test"}' \
    "$BASE_URL/register"

# Login
curl -s -o /dev/null -w "Login valid: %{http_code}\n" -X POST \
    -H "Content-Type: application/json" \
    -d "{\"username\":\"$username\",\"password\":\"$password\"}" \
    "$BASE_URL/login"

curl -s -o /dev/null -w "Login invalid username: %{http_code}\n" -X POST \
    -H "Content-Type: application/json" \
    -d '{"username":"wrong","password":"test"}' \
    "$BASE_URL/login"

curl -s -o /dev/null -w "Login missing credentials: %{http_code}\n" -X POST \
    -H "Content-Type: application/json" \
    -d '{}' \
    "$BASE_URL/login"
