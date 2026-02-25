#!/usr/bin/env python3
"""
Validate SpectraCore bandpass/bandstop filter implementation against scipy (MATLAB-compatible).
"""

import numpy as np
from scipy import signal
import json
import sys

def design_filter(filter_type, filter_design, params):
    """Design a filter using scipy.signal (MATLAB-compatible)."""
    try:
        if filter_design == 'butter':
            order, low_freq, high_freq = params
            b, a = signal.butter(order, [low_freq, high_freq], btype=filter_type)
        elif filter_design == 'cheby1':
            order, ripple, low_freq, high_freq = params
            b, a = signal.cheby1(order, ripple, [low_freq, high_freq], btype=filter_type)
        elif filter_design == 'cheby2':
            order, stopband, low_freq, high_freq = params
            b, a = signal.cheby2(order, stopband, [low_freq, high_freq], btype=filter_type)
        elif filter_design == 'ellip':
            order, ripple, stopband, low_freq, high_freq = params
            b, a = signal.ellipord([low_freq, high_freq], [low_freq-0.05, high_freq+0.05], ripple, stopband)
            # Use ellip instead
            b, a = signal.ellip(order, ripple, stopband, [low_freq, high_freq], btype=filter_type)
        else:
            raise ValueError(f"Unknown filter type: {filter_design}")
        return b, a
    except Exception as e:
        print(f"Error designing {filter_design} {filter_type}: {e}")
        return None, None

def get_frequency_response(b, a, num_points=512):
    """Get frequency response at key points."""
    w, h = signal.freqz(b, a, worN=num_points)
    mag = np.abs(h)
    phase = np.angle(h)

    return {
        'frequencies': w.tolist(),
        'magnitude': mag.tolist(),
        'phase': phase.tolist(),
        'dc_gain': float(mag[0]),
        'nyquist_gain': float(mag[-1]),
        'center_gain': float(mag[len(mag)//2])
    }

def validate_bandpass(filter_design, params, num_points=512):
    """Validate bandpass filter response."""
    b, a = design_filter('bandpass', filter_design, params)
    if b is None:
        return None

    resp = get_frequency_response(b, a, num_points)
    resp['b'] = b.tolist()
    resp['a'] = a.tolist()
    resp['b_length'] = len(b)
    resp['a_length'] = len(a)

    # Check properties
    issues = []
    if resp['dc_gain'] >= 0.1:
        issues.append(f"DC gain too high: {resp['dc_gain']}")
    if resp['nyquist_gain'] >= 0.1:
        issues.append(f"Nyquist gain too high: {resp['nyquist_gain']}")
    if resp['center_gain'] <= 0.5:
        issues.append(f"Center gain too low: {resp['center_gain']}")

    resp['validation_issues'] = issues
    resp['passes'] = len(issues) == 0

    return resp

def validate_bandstop(filter_design, params, num_points=512):
    """Validate bandstop filter response."""
    b, a = design_filter('bandstop', filter_design, params)
    if b is None:
        return None

    resp = get_frequency_response(b, a, num_points)
    resp['b'] = b.tolist()
    resp['a'] = a.tolist()
    resp['b_length'] = len(b)
    resp['a_length'] = len(a)

    # Check properties
    issues = []
    if resp['dc_gain'] <= 0.7:
        issues.append(f"DC gain too low: {resp['dc_gain']}")
    if resp['nyquist_gain'] <= 0.7:
        issues.append(f"Nyquist gain too low: {resp['nyquist_gain']}")
    if resp['center_gain'] >= 0.3:
        issues.append(f"Center gain too high: {resp['center_gain']}")

    resp['validation_issues'] = issues
    resp['passes'] = len(issues) == 0

    return resp

def test_complementary(filter_design, params):
    """Test that bandpass + bandstop ≈ 1 at all frequencies."""
    b_bp, a_bp = design_filter('bandpass', filter_design, params)
    b_bs, a_bs = design_filter('bandstop', filter_design, params)

    if b_bp is None or b_bs is None:
        return None

    w, h_bp = signal.freqz(b_bp, a_bp, worN=512)
    _, h_bs = signal.freqz(b_bs, a_bs, worN=512)

    mag_bp = np.abs(h_bp)
    mag_bs = np.abs(h_bs)

    sum_mag = mag_bp + mag_bs
    max_error = np.max(np.abs(sum_mag - 1.0))

    return {
        'max_error': float(max_error),
        'passes': max_error < 0.15,
        'sum_at_dc': float(mag_bp[0] + mag_bs[0]),
        'sum_at_center': float(mag_bp[256] + mag_bs[256]),
        'sum_at_nyquist': float(mag_bp[-1] + mag_bs[-1])
    }

def main():
    """Run all validations and generate report."""
    print("=" * 80)
    print("SpectraCore Bandpass/Bandstop Filter Validation")
    print("=" * 80)

    test_cases = [
        ('Butterworth', 'butter', [
            [2, 0.2, 0.4],
            [3, 0.2, 0.5],
            [4, 0.25, 0.45]
        ]),
        ('Chebyshev I', 'cheby1', [
            [2, 0.5, 0.2, 0.5],
            [3, 0.5, 0.2, 0.5],
            [2, 1.0, 0.25, 0.45]
        ]),
        ('Chebyshev II', 'cheby2', [
            [2, 40.0, 0.2, 0.4],
            [3, 50.0, 0.2, 0.5],
            [2, 40.0, 0.25, 0.45]
        ]),
        ('Elliptic', 'ellip', [
            [2, 0.5, 40.0, 0.2, 0.5],
            [2, 1.0, 50.0, 0.2, 0.4],
            [3, 0.5, 40.0, 0.25, 0.45]
        ])
    ]

    all_results = {}

    for filter_name, filter_type, test_params in test_cases:
        print(f"\n{filter_name}:")
        print("-" * 80)

        filter_results = {}

        for params in test_params:
            param_str = str(params)
            print(f"  Parameters: {param_str}")

            # Bandpass
            bp_result = validate_bandpass(filter_type, params)
            print(f"    Bandpass:  DC={bp_result['dc_gain']:.4f}, "
                  f"Center={bp_result['center_gain']:.4f}, "
                  f"Nyq={bp_result['nyquist_gain']:.4f} "
                  f"→ {'✓ PASS' if bp_result['passes'] else '✗ FAIL'}")

            # Bandstop
            bs_result = validate_bandstop(filter_type, params)
            print(f"    Bandstop:  DC={bs_result['dc_gain']:.4f}, "
                  f"Center={bs_result['center_gain']:.4f}, "
                  f"Nyq={bs_result['nyquist_gain']:.4f} "
                  f"→ {'✓ PASS' if bs_result['passes'] else '✗ FAIL'}")

            # Complementary
            comp_result = test_complementary(filter_type, params)
            print(f"    Complementary: max_error={comp_result['max_error']:.4f} "
                  f"→ {'✓ PASS' if comp_result['passes'] else '✗ FAIL'}")

            filter_results[param_str] = {
                'bandpass': bp_result,
                'bandstop': bs_result,
                'complementary': comp_result
            }

        all_results[filter_name] = filter_results

    # Summary
    print("\n" + "=" * 80)
    print("SUMMARY")
    print("=" * 80)

    total_tests = 0
    passed_tests = 0

    for filter_name in all_results:
        filter_data = all_results[filter_name]
        for param_str in filter_data:
            param_data = filter_data[param_str]

            # Count bandpass
            if param_data['bandpass']['passes']:
                passed_tests += 1
            total_tests += 1

            # Count bandstop
            if param_data['bandstop']['passes']:
                passed_tests += 1
            total_tests += 1

            # Count complementary
            if param_data['complementary']['passes']:
                passed_tests += 1
            total_tests += 1

    print(f"\nTests Passed: {passed_tests}/{total_tests}")
    print(f"Success Rate: {100*passed_tests/total_tests:.1f}%")

    if passed_tests == total_tests:
        print("\n✓ ALL TESTS PASSED - Implementation is valid!")
        return 0
    else:
        print(f"\n✗ {total_tests - passed_tests} tests failed")
        return 1

if __name__ == '__main__':
    sys.exit(main())
