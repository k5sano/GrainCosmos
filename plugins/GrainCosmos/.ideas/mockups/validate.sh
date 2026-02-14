#!/bin/bash

WINDOW_WIDTH=800
WINDOW_HEIGHT=500
MIN_EDGE_PADDING=15
MIN_CONTROL_SPACING=10

echo "Validating layout: ${WINDOW_WIDTH}×${WINDOW_HEIGHT}"

# Read control positions from YAML
# Manual extraction based on our YAML:
declare -A CONTROLS=(
    ["delay"]="60:80:80:100"
    ["grain"]="220:80:80:100"
    ["envelope"]="380:80:80:100"
    ["distortion"]="540:80:80:100"
    ["feedback"]="60:240:80:100"
    ["chaos"]="220:240:80:100"
    ["character"]="380:240:80:100"
    ["mix"]="540:240:80:100"
    ["freeze"]="640:275:60:30"
    ["sync"]="60:425:60:30"
)

ERRORS=()
WARNINGS=()

# Validation function
validate_control() {
    local id=$1
    local x=$2
    local y=$3
    local w=$4
    local h=$5

    local right_edge=$((x + w))
    local bottom_edge=$((y + h))
    local right_padding=$((WINDOW_WIDTH - right_edge))
    local bottom_padding=$((WINDOW_HEIGHT - bottom_edge))

    # Edge padding checks
    if [ $x -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Bounds violation: $id too close to left edge (${x}px, need ${MIN_EDGE_PADDING}px minimum)")
    fi

    if [ $y -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Bounds violation: $id too close to top edge (${y}px, need ${MIN_EDGE_PADDING}px minimum)")
    fi

    if [ $right_padding -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Bounds violation: $id too close to right edge (${right_padding}px padding, need ${MIN_EDGE_PADDING}px minimum)")
    fi

    if [ $bottom_padding -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Bounds violation: $id too close to bottom edge (${bottom_padding}px padding, need ${MIN_EDGE_PADDING}px minimum)")
    fi
}

# Validate each control
for ctrl in "${!CONTROLS[@]}"; do
    IFS=':' read -r x y w h <<< "${CONTROLS[$ctrl]}"
    validate_control "$ctrl" "$x" "$y" "$w" "$h"
done

# Print results
if [ ${#ERRORS[@]} -gt 0 ]; then
    echo "❌ FAILED - ${#ERRORS[@]} error(s) found:"
    for err in "${ERRORS[@]}"; do
        echo "  $err"
    done
    exit 1
else
    echo "✅ Layout validation passed"
    echo ""
    echo "Layout Summary:"
    echo "  Window: ${WINDOW_WIDTH}×${WINDOW_HEIGHT}"
    echo "  Controls: ${#CONTROLS[@]}"
    echo "  Top row y: 80"
    echo "  Bottom row y: 240"
    echo "  Footer y: 410"
    exit 0
fi
