#!/bin/bash

HTML_FILE="v1-ui-test.html"
VALIDATION_PASSED=true
ISSUES=()

echo "Validating WebView constraints for ${HTML_FILE}..."
echo ""

# Check 1: No viewport units
if grep -qE "100vh|100vw|100dvh|100svh|100lvh|100dvw" "$HTML_FILE"; then
    echo "❌ FAIL: Forbidden viewport units found"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: viewport units (100vh/100vw) detected")
else
    echo "✅ PASS: No viewport units found"
fi

# Check 2: Required html height (using awk to handle multiline)
if awk '/html\s*\{/,/\}/' "$HTML_FILE" | grep -q "height: 100%"; then
    echo "✅ PASS: Required html height: 100% present"
else
    echo "❌ FAIL: Missing required html height: 100%"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: missing html height: 100%")
fi

# Check 2b: Required body height
if awk '/body\s*\{/,/\}/' "$HTML_FILE" | grep -q "height: 100%"; then
    echo "✅ PASS: Required body height: 100% present"
else
    echo "❌ FAIL: Missing required body height: 100%"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: missing body height: 100%")
fi

# Check 3: Native feel CSS
if grep -q "user-select: none" "$HTML_FILE"; then
    echo "✅ PASS: user-select: none present"
else
    echo "❌ FAIL: Missing user-select: none"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: missing user-select: none")
fi

# Check 4: Context menu disabled
if grep -qE 'addEventListener.*contextmenu' "$HTML_FILE"; then
    echo "✅ PASS: Context menu disabled"
else
    echo "❌ FAIL: Context menu not disabled"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: context menu not disabled")
fi

echo ""
echo "---"
echo ""

if [ "$VALIDATION_PASSED" = false ]; then
    echo "❌ WebView validation FAILED"
    echo ""
    echo "Issues found:"
    for issue in "${ISSUES[@]}"; do
        echo "  - $issue"
    done
    exit 1
else
    echo "✅ All WebView constraints validated successfully"
    exit 0
fi
