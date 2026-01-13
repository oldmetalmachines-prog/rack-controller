#!/bin/bash
#
# lint.sh - Run linters on codebase
#
# Usage: ./lint.sh [--fix]
#

set -e

SCRIPT_DIR="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
REPO_ROOT="$(dirname "$SCRIPT_DIR")"

FIX_MODE=false

while [[ $# -gt 0 ]]; do
    case $1 in
        --fix)
            FIX_MODE=true
            shift
            ;;
        -h|--help)
            echo "Usage: $0 [--fix]"
            echo ""
            echo "Options:"
            echo "  --fix   Automatically fix issues where possible"
            exit 0
            ;;
        *)
            echo "Unknown option: $1"
            exit 1
            ;;
    esac
done

echo "============================================"
echo "Running Linters"
echo "============================================"
echo ""

ERRORS=0

# Python linting
echo "[1/3] Checking Python files..."
if command -v ruff &> /dev/null; then
    if [[ "$FIX_MODE" == "true" ]]; then
        ruff check "$REPO_ROOT/tools" --fix || ERRORS=$((ERRORS + 1))
        ruff format "$REPO_ROOT/tools" || true
    else
        ruff check "$REPO_ROOT/tools" || ERRORS=$((ERRORS + 1))
    fi
elif command -v flake8 &> /dev/null; then
    flake8 "$REPO_ROOT/tools" --max-line-length=100 || ERRORS=$((ERRORS + 1))
else
    echo "  ⊘ No Python linter found (install ruff or flake8)"
fi
echo ""

# Shell script linting
echo "[2/3] Checking shell scripts..."
if command -v shellcheck &> /dev/null; then
    shellcheck "$REPO_ROOT/tools"/*.sh || ERRORS=$((ERRORS + 1))
else
    echo "  ⊘ shellcheck not found (install with: apt install shellcheck)"
fi
echo ""

# C/C++ linting (basic check)
echo "[3/3] Checking C/C++ files..."
if command -v cppcheck &> /dev/null; then
    cppcheck --enable=warning,style \
             --suppress=missingIncludeSystem \
             "$REPO_ROOT/firmware/esp32_s3_pio/src" \
             "$REPO_ROOT/firmware/cyd_pio/src" \
             "$REPO_ROOT/firmware/esp32_p4_idf/main" \
             2>&1 || ERRORS=$((ERRORS + 1))
else
    echo "  ⊘ cppcheck not found (install with: apt install cppcheck)"
fi
echo ""

# Summary
echo "============================================"
if [[ $ERRORS -eq 0 ]]; then
    echo "✓ All linter checks passed"
else
    echo "✗ Linter found $ERRORS issue(s)"
fi
echo "============================================"

exit $ERRORS
