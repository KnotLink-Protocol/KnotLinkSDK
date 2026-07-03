#!/usr/bin/env bash
set -euo pipefail

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(cd "$SCRIPT_DIR/../.." && pwd)"
BUILD_DIR="${TMPDIR:-/tmp}/knotlink-java-sdk-check"

rm -rf "$BUILD_DIR"
mkdir -p "$BUILD_DIR"

javac -encoding UTF-8 -d "$BUILD_DIR" \
  "$REPO_ROOT"/knotlink-sdk-java/knotlink/*.java \
  "$SCRIPT_DIR/JavaSdkRegressionTest.java"

java -cp "$BUILD_DIR" -Dknotlink.repoRoot="$REPO_ROOT" JavaSdkRegressionTest
