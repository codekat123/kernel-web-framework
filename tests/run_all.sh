#!/bin/sh
# run_all.sh - Run all test scripts

TEST_DIR="$(dirname "$0")"

for script in functional.sh auth.sh static.sh keepalive.sh concurrency.sh security.sh benchmark.sh; do
    echo "=== Running $script ==="
    sh "$TEST_DIR/$script"
    echo ""
done
