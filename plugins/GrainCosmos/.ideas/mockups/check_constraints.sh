#!/bin/bash

echo "WebView Constraint Validation"
echo "============================"
echo ""

PASS=0
FAIL=0

# Check 1: No viewport units
echo -n "Checking for forbidden viewport units... "
if grep -qE "100vh|100vw|100dvh|100svh|100lvh|100dvw" v1-ui-test.html; then
    echo "❌ FAIL"
    FAIL=$((FAIL+1))
else
    echo "✅ PASS"
    PASS=$((PASS+1))
fi

# Check 2: html height 100%
echo -n "Checking html height: 100%... "
if grep -q "html {" v1-ui-test.html && grep -q "height: 100%" v1-ui-test.html; then
    echo "✅ PASS"
    PASS=$((PASS+1))
else
    echo "❌ FAIL"
    FAIL=$((FAIL+1))
fi

# Check 3: body height 100%
echo -n "Checking body height: 100%... "
if grep -q "body {" v1-ui-test.html && grep -q "height: 100%" v1-ui-test.html; then
    echo "✅ PASS"
    PASS=$((PASS+1))
else
    echo "❌ FAIL"
    FAIL=$((FAIL+1))
fi

# Check 4: user-select: none
echo -n "Checking user-select: none... "
if grep -q "user-select: none" v1-ui-test.html; then
    echo "✅ PASS"
    PASS=$((PASS+1))
else
    echo "❌ FAIL"
    FAIL=$((FAIL+1))
fi

# Check 5: context menu disabled
echo -n "Checking context menu disabled... "
if grep -q "contextmenu" v1-ui-test.html && grep -q "preventDefault" v1-ui-test.html; then
    echo "✅ PASS"
    PASS=$((PASS+1))
else
    echo "❌ FAIL"
    FAIL=$((FAIL+1))
fi

# Check 6: box-sizing: border-box
echo -n "Checking box-sizing: border-box... "
if grep -q "box-sizing: border-box" v1-ui-test.html; then
    echo "✅ PASS"
    PASS=$((PASS+1))
else
    echo "⚠️  WARNING"
fi

echo ""
echo "Results: $PASS passed, $FAIL failed"
echo ""

if [ $FAIL -eq 0 ]; then
    echo "✅ All WebView constraints validated successfully"
    exit 0
else
    echo "❌ WebView validation FAILED"
    exit 1
fi
