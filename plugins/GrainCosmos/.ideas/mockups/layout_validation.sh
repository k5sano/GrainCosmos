#!/bin/bash

VERSION="v2"
WINDOW_WIDTH=800
WINDOW_HEIGHT=500
MIN_EDGE_PADDING=15
MIN_CONTROL_SPACING=10

echo "=========================================="
echo "LAYOUT VALIDATION: GrainCosmos v2"
echo "=========================================="
echo "Window: ${WINDOW_WIDTH}x${WINDOW_HEIGHT}"
echo ""

# Parse controls from YAML
declare -A CTRL_X CTRL_Y CTRL_W CTRL_H CTRL_TYPE
declare -a CTRL_IDS

# Manual control positions from YAML (same as v1)
CTRL_IDS=(delay grain envelope distortion feedback chaos character mix freeze sync)
CTRL_X=(60 220 380 540 60 220 380 540 640 60)
CTRL_Y=(80 80 80 80 240 240 240 240 275 425)
CTRL_W=(80 80 80 80 80 80 80 80 60 60)
CTRL_H=(100 100 100 100 100 100 100 100 30 30)
CTRL_TYPES=(knob knob knob knob knob knob knob knob toggle toggle)

ERRORS=()
WARNINGS=()
VALIDATION_PASSED=true

# Validation 1: Bounds containment
echo "1. BOUNDS CONTAINMENT"
for i in "${!CTRL_IDS[@]}"; do
    id="${CTRL_IDS[$i]}"
    x="${CTRL_X[$i]}"
    y="${CTRL_Y[$i]}"
    w="${CTRL_W[$i]}"
    h="${CTRL_H[$i]}"
    
    right_edge=$((x + w))
    bottom_edge=$((y + h))
    
    # Check bounds
    if [ $x -lt 0 ]; then
        ERRORS+=("Bounds violation: $id extends past left edge (x=$x)")
        VALIDATION_PASSED=false
    fi
    
    if [ $y -lt 0 ]; then
        ERRORS+=("Bounds violation: $id extends past top edge (y=$y)")
        VALIDATION_PASSED=false
    fi
    
    if [ $right_edge -gt $WINDOW_WIDTH ]; then
        overflow=$((right_edge - WINDOW_WIDTH))
        ERRORS+=("Bounds violation: $id overflows right edge by ${overflow}px")
        VALIDATION_PASSED=false
    fi
    
    if [ $bottom_edge -gt $WINDOW_HEIGHT ]; then
        overflow=$((bottom_edge - WINDOW_HEIGHT))
        ERRORS+=("Bounds violation: $id overflows bottom edge by ${overflow}px")
        VALIDATION_PASSED=false
    fi
    
    # Check edge padding
    if [ $x -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Edge padding violation: $id only ${x}px from left edge (need ${MIN_EDGE_PADDING}px)")
        VALIDATION_PASSED=false
    fi
    
    if [ $y -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Edge padding violation: $id only ${y}px from top edge (need ${MIN_EDGE_PADDING}px)")
        VALIDATION_PASSED=false
    fi
    
    right_padding=$((WINDOW_WIDTH - right_edge))
    if [ $right_padding -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Edge padding violation: $id only ${right_padding}px from right edge (need ${MIN_EDGE_PADDING}px)")
        VALIDATION_PASSED=false
    fi
    
    bottom_padding=$((WINDOW_HEIGHT - bottom_edge))
    if [ $bottom_padding -lt $MIN_EDGE_PADDING ]; then
        ERRORS+=("Edge padding violation: $id only ${bottom_padding}px from bottom edge (need ${MIN_EDGE_PADDING}px)")
        VALIDATION_PASSED=false
    fi
    
    echo "  ✓ $id: ($x, $y, ${w}x${h})"
done

# Validation 2: Overlap detection
echo ""
echo "2. OVERLAP DETECTION"
for i in "${!CTRL_IDS[@]}"; do
    id1="${CTRL_IDS[$i]}"
    x1="${CTRL_X[$i]}"
    y1="${CTRL_Y[$i]}"
    w1="${CTRL_W[$i]}"
    h1="${CTRL_H[$i]}"
    
    for j in $(seq $((i + 1)) "${#CTRL_IDS[@]}"); do
        if [ $j -ge "${#CTRL_IDS[@]}" ]; then continue; fi
        
        id2="${CTRL_IDS[$j]}"
        x2="${CTRL_X[$j]}"
        y2="${CTRL_Y[$j]}"
        w2="${CTRL_W[$j]}"
        h2="${CTRL_H[$j]}"
        
        # Bounding box collision
        if [ $x1 -lt $((x2 + w2)) ] && \
           [ $((x1 + w1)) -gt $x2 ] && \
           [ $y1 -lt $((y2 + h2)) ] && \
           [ $((y1 + h1)) -gt $y2 ]; then
            ERRORS+=("Overlap detected: $id1 and $id2 occupy same space")
            VALIDATION_PASSED=false
            echo "  ✗ $id1 overlaps $id2"
        fi
    done
done
echo "  ✓ No overlaps found"

# Validation 3: Minimum usable sizes
echo ""
echo "3. MINIMUM USABLE SIZES"
for i in "${!CTRL_IDS[@]}"; do
    id="${CTRL_IDS[$i]}"
    type="${CTRL_TYPES[$i]}"
    w="${CTRL_W[$i]}"
    h="${CTRL_H[$i]}"
    
    case $type in
        knob)
            if [ $w -lt 40 ] || [ $h -lt 40 ]; then
                ERRORS+=("Size violation: $id too small (${w}x${h}, need min 40px)")
                VALIDATION_PASSED=false
            fi
            ;;
        toggle)
            if [ $w -lt 60 ] || [ $h -lt 24 ]; then
                ERRORS+=("Size violation: $id too small (${w}x${h}, need min 60x24px)")
                VALIDATION_PASSED=false
            fi
            ;;
    esac
    echo "  ✓ $id: ${w}x${h}px (${type})"
done

# Validation 4: Minimum spacing (warnings only)
echo ""
echo "4. MINIMUM SPACING"
SPACING_WARNINGS=0
for i in "${!CTRL_IDS[@]}"; do
    id1="${CTRL_IDS[$i]}"
    x1="${CTRL_X[$i]}"
    w1="${CTRL_W[$i]}"
    
    for j in $(seq $((i + 1)) "${#CTRL_IDS[@]}"); do
        if [ $j -ge "${#CTRL_IDS[@]}" ]; then continue; fi
        
        id2="${CTRL_IDS[$j]}"
        x2="${CTRL_X[$j]}"
        
        if [ $x1 -lt $x2 ]; then
            gap=$((x2 - x1 - w1))
        else
            w2="${CTRL_W[$j]}"
            gap=$((x1 - x2 - w2))
        fi
        
        if [ $gap -lt $MIN_CONTROL_SPACING ] && [ $gap -gt 0 ]; then
            WARNINGS+=("Spacing: $id1 and $id2 only have ${gap}px gap (recommend ${MIN_CONTROL_SPACING}px)")
            SPACING_WARNINGS=$((SPACING_WARNINGS + 1))
        fi
    done
done
echo "  ✓ All controls have proper spacing"

echo ""
echo "=========================================="
if [ "$VALIDATION_PASSED" = true ]; then
    echo "✅ VALIDATION PASSED"
    if [ ${#WARNINGS[@]} -gt 0 ]; then
        echo ""
        echo "⚠️  WARNINGS (${#WARNINGS[@]}):"
        for warning in "${WARNINGS[@]}"; do
            echo "  - $warning"
        done
    fi
    exit 0
else
    echo "❌ VALIDATION FAILED"
    echo ""
    echo "ERRORS (${#ERRORS[@]}):"
    for error in "${ERRORS[@]}"; do
        echo "  - $error"
    done
    exit 1
fi
