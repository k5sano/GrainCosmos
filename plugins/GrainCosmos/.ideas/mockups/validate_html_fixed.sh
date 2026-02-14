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

# Check 2: Required html/body height (separate rules allowed)
if grep -qE "html\s*\{[^}]*height:\s*100%" "$HTML_FILE" && grep -qE "body\s*\{[^}]*height:\s*100%" "$HTML_FILE"; then
    echo "✅ PASS: Required html/body height: 100% present"
else
    echo "❌ FAIL: Missing required html/body height: 100%"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: missing html/body height: 100%")
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
if grep -qE 'addEventListener\("contextmenu"' "$HTML_FILE" || grep -qE 'addEventListener.*contextmenu' "$HTML_FILE"; then
    echo "✅ PASS: Context menu disabled"
else
    echo "❌ FAIL: Context menu not disabled"
    VALIDATION_PASSED=false
    ISSUES+=("WebView constraint violation: context menu not disabled")
fi

# Check 5: Box model
if grep -q "box-sizing: border-box" "$HTML_FILE"; then
    echo "✅ PASS: box-sizing: border-box present"
else
    echo "⚠️  WARNING: Missing box-sizing: border-box (recommended)"
fi

# Check 6: Overflow hidden
if grep -q "overflow: hidden" "$HTML_FILE"; then
    echo "✅ PASS: overflow: hidden present"
else
    echo "⚠️  WARNING: Missing overflow: hidden (recommended for fixed-size plugins)"
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
