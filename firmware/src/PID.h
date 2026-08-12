#pragma once

// Textbook PID with clamped output and clamped-integral anti-windup.
// Time-independent of the caller's loop rate: pass the actual elapsed
// seconds each call (Heater drives this at a fixed PID_SAMPLE_MS).
class PID {
public:
    PID(float kp, float ki, float kd, float outMin, float outMax)
        : _kp(kp), _ki(ki), _kd(kd), _outMin(outMin), _outMax(outMax) {}

    void reset() {
        _integral = 0.0f;
        _prevError = 0.0f;
        _firstSample = true;
    }

    float compute(float setpoint, float measured, float dtSeconds) {
        float error = setpoint - measured;

        _integral += _ki * error * dtSeconds;
        if (_integral > _outMax) _integral = _outMax;
        if (_integral < _outMin) _integral = _outMin;

        float derivative = _firstSample ? 0.0f : (error - _prevError) / dtSeconds;
        _firstSample = false;
        _prevError = error;

        float output = _kp * error + _integral + _kd * derivative;
        if (output > _outMax) output = _outMax;
        if (output < _outMin) output = _outMin;
        return output;
    }

private:
    float _kp, _ki, _kd;
    float _outMin, _outMax;
    float _integral = 0.0f;
    float _prevError = 0.0f;
    bool _firstSample = true;
};
