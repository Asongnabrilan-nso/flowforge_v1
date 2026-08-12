#pragma once

// Marlin-style PID: functional-range bang-bang bootstrap, accumulator-
// clamped anti-windup, EMA-smoothed derivative-on-measurement. Still
// time-independent of the caller's loop rate: pass the actual elapsed
// seconds each call (Heater drives this at a fixed PID_SAMPLE_MS).
class PID {
public:
    PID(float kp, float ki, float kd, float outMin, float outMax,
        float functionalRangeC, float k1)
        : _kp(kp), _ki(ki), _kd(kd), _outMin(outMin), _outMax(outMax),
          _functionalRange(functionalRangeC), _k1(k1) {}

    void reset() {
        _iState = 0.0f;
        _dTerm = 0.0f;
        _firstSample = true;
    }

    float compute(float setpoint, float measured, float dtSeconds) {
        float error = setpoint - measured;

        // Bang-bang bootstrap: far outside the functional range, skip PID
        // math entirely. Avoids integral windup while ramping from cold and
        // wasted math while grossly overshot. Mirrors Marlin's
        // PID_FUNCTIONAL_RANGE behavior.
        if (error > _functionalRange) {
            reset();
            _prevMeasured = measured; // seed so hand-off to PID mode has no kick
            return _outMax;
        }
        if (error < -_functionalRange) {
            reset();
            _prevMeasured = measured;
            return _outMin;
        }

        float workP = _kp * error;

        // Integral: clamp the raw (pre-Ki) accumulator, not the output
        // term, so the anti-windup ceiling stays correct regardless of Ki.
        float minIState = (_ki > 0.0f) ? (_outMin / _ki) : 0.0f;
        float maxIState = (_ki > 0.0f) ? (_outMax / _ki) : 0.0f;
        _iState += error * dtSeconds;
        if (_iState > maxIState) _iState = maxIState;
        if (_iState < minIState) _iState = minIState;
        float workI = _ki * _iState;

        // Derivative-on-measurement (no kick from setpoint steps) with EMA
        // smoothing (Marlin PID_K1) so a single noisy ADC sample doesn't
        // spike the output.
        if (_firstSample) {
            _prevMeasured = measured;
            _firstSample = false;
        }
        float rawD = (_kd / dtSeconds) * (_prevMeasured - measured);
        float k2 = 1.0f - _k1;
        _dTerm = _dTerm + k2 * (rawD - _dTerm);
        _prevMeasured = measured;

        float output = workP + workI + _dTerm;
        if (output > _outMax) output = _outMax;
        if (output < _outMin) output = _outMin;
        return output;
    }

private:
    float _kp, _ki, _kd;
    float _outMin, _outMax;
    float _functionalRange;
    float _k1;

    float _iState = 0.0f;
    float _dTerm = 0.0f;
    float _prevMeasured = 0.0f;
    bool _firstSample = true;
};
