#!/bin/sh
# benchmark.sh - Performance benchmark for kernel-web-framework

if command -v wrk >/dev/null 2>&1; then
    echo "Benchmarking with wrk..."
    wrk -t4 -c50 -d10s http://localhost:8080/
    wrk -t4 -c50 -d10s -s - <<'EOF'
request = function()
    body = '{"username":"test","password":"test"}'
    return wrk.format("POST", "/login", nil, body)
end
EOF
elif command -v ab >/dev/null 2>&1; then
    echo "Benchmarking with ApacheBench..."
    ab -n 10000 -c 100 http://localhost:8080/
    echo '{"username":"test","password":"test"}' | \
        ab -n 1000 -c 50 -p /dev/stdin -T application/json http://localhost:8080/login
else
    echo "Error: wrk or ApacheBench (ab) required"
    exit 1
fi
