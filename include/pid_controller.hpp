#pragma once

#include <stdint.h>

#include "literals.hpp"

namespace ctrl {

using namespace ::literals;

class pid_controller {
    private:
    double m_target;

    double m_kp;
    double m_ki;
    double m_kd;

    double m_int;
    double m_prev_err;

    dura_t m_prev_time;

    bool m_first_sample;

    public:
    pid_controller() noexcept
    : m_target(.0),
      m_kp(.0), m_ki(.0), m_kd(.0),
      m_int(.0),
      m_prev_err(.0),
      m_prev_time(0s),
      m_first_sample(true) {
    }

    pid_controller(double kp, double ki, double kd) noexcept
    : pid_controller() {
        m_kp = kp;
        m_ki = ki;
        m_kd = kd;
    }

    ~pid_controller() noexcept = default;

    auto update(double val, dura_t now_time) -> double {
        double dt = now_time.v - m_prev_time.v;
        if (m_first_sample) dt = 0;

        double err = m_target - val;

        double p = err * m_kp;

        m_int += err * dt;
        double i = m_int * m_ki;

        double d = 0;
        if (!m_first_sample) {
            double der = -(err - m_prev_err) / dt;

            d = der * m_kd;
        }

        m_prev_err     = err;
        m_prev_time    = now_time;
        m_first_sample = false;

        double output = p + i + d;
        return output;
    }

    auto reset() -> void {
        m_int          = 0;
        m_prev_err     = 0;
        m_first_sample = true;
    }

    auto get_target() noexcept -> const decltype(m_target) { return m_target; }
    auto set_target(double target) -> void { m_target = target; }

    auto get_kp() noexcept -> const decltype(m_kp) { return m_kp; }
    auto set_kp(double kp) -> void { m_kp = kp; }

    auto get_ki() noexcept -> const decltype(m_ki) { return m_ki; }
    auto set_ki(double ki) -> void { m_ki = ki; }

    auto get_kd() noexcept -> const decltype(m_kd) { return m_kd; }
    auto set_kd(double kd) -> void { m_kd = kd; }
};

} // namespace ctrl
