#include <Arduino.h>
#include <Modulino.h>

#include "WiFi.h"

#include "Arduino_LED_Matrix.h"

#include "led_matrix.hpp"
#include "literals.hpp"
#include "pid_controller.hpp"

#define ENABLE_LOGGING
#include "logger.hpp"

using namespace ::literals;

// hardware devices
LED_Matrix led_matrix;
ModulinoKnob knob;
ModulinoButtons button;
ModulinoMovement imu;
ModulinoPixels pixels;


auto setup() -> void {


    LOG_BEGIN(115200);
    LOG_SETSHOWLEVEL(true);
    LOG_SETSHOWLOCATION(true);

    LOG_INFO("Modulino begin");
    Modulino.begin();

    LOG_INFO("Knob begin");
    knob.begin();
    knob.set(0);

    LOG_INFO("Pixels begin");
    pixels.begin();
    pixels.clear();
    pixels.show();

    LOG_INFO("Button begin");
    button.begin();

    LOG_INFO("IMU begin");
    imu.begin();
    imu.update();

    LOG_INFO("LED Matrix begin");
    led_matrix.begin();
    led_matrix.flash();
}

auto loop() -> void {

    enum class WorkState {
        IDLE,
        PIXEL_TEST,
        SHOW_IMU,
        SHOW_KNOB,
    } state;
    state = WorkState::IDLE;

    // IMU相关变量
    float vel_x = 0.0f, vel_y = 0.0f;
    float pos_x = 0.0f, pos_y = 0.0f;

    // 使用高通滤波去除DC偏移，而不是简单的offset
    float acc_x_lpf = 0.0f, acc_y_lpf = 0.0f; // 低频成分（偏移）
    float acc_x_hpf = 0.0f, acc_y_hpf = 0.0f; // 高频成分（实际运动）

    uint32_t last_time_stamp = micros();
    uint32_t dtus            = 0;

    // 调整后的参数
    const float lpf_alpha          = 0.98f;  // 低通滤波提取缓慢变化的偏移
    const float smoothing_alpha    = 0.3f;   // 平滑滤波（更快响应）
    const float deadzone           = 0.01f;  // 减小死区
    const float velocity_threshold = 0.001f; // 速度死区

    // 缩放因子（根据LED矩阵大小调整）
    const float position_scale = 5.0f; // 增加灵敏度

    float acc_x_smooth = 0.0f, acc_y_smooth = 0.0f;


    while (1) {
        // main loop

        uint32_t current_time = micros();
        dtus                  = current_time - last_time_stamp;
        last_time_stamp       = current_time;

        float dt = dtus / 1000000.0f;
        button.update();

        if (button.isPressed('A') && button.isPressed('B') && button.isPressed('C')) {
            state = WorkState::IDLE;
        }

        if (button.isPressed('A') && state != WorkState::SHOW_KNOB) {
            LOG_INFO("Press A");
            led_matrix.clear();
            state = WorkState::SHOW_KNOB;
        } else if (button.isPressed('B') && state != WorkState::SHOW_IMU) {
            LOG_INFO("Press B");
            led_matrix.clear();

            vel_x        = 0.0f;
            vel_y        = 0.0f;
            pos_x        = 0.0f;
            pos_y        = 0.0f;
            acc_x_lpf    = 0.0f;
            acc_y_lpf    = 0.0f;
            acc_x_smooth = 0.0f;
            acc_y_smooth = 0.0f;

            imu.update();
            acc_x_lpf = imu.getX();
            acc_y_lpf = imu.getY();

            state = WorkState::SHOW_IMU;
        } else if (button.isPressed('C') && state != WorkState::PIXEL_TEST) {
            LOG_INFO("Press C");
            led_matrix.clear();
            state = WorkState::PIXEL_TEST;
        }

        switch (state) {
        case WorkState::IDLE: {
            break;
        }
        /// ===================== PIXEL_TEST ====================
        case WorkState::PIXEL_TEST: {
            static enum : uint8_t {
                S1,
                S2,
                S3,
                S4,
                S_CLEAN,
            } ws;

            static unsigned long last_update_time = millis();

            int brightness = constrain(knob.get(), 0, 100);
            knob.set(brightness);

            LOG_TRACE("waiting time {} ms", millis() - last_update_time);

            auto wave = [](unsigned long time) -> ModulinoColor {
                return ModulinoColor{
                    ((sin(time + 5) + 1.) / 2) * 255,
                    ((sin(time - 3) + 1.) / 2) * 255,
                    ((sin(time + 2) + 1.) / 2) * 255
                };
            };

            if (millis() - last_update_time >= 200) {
                last_update_time = millis();
                auto t           = millis();
                switch (ws) {
                case S1:
                    pixels.set(0, wave(t), brightness);
                    pixels.set(1, wave(t), brightness);
                    LOG_DEBUG("S1");
                    ws = S2;
                    break;
                case S2:
                    pixels.set(2, wave(t), brightness);
                    pixels.set(3, wave(t), brightness);
                    LOG_DEBUG("S2");
                    ws = S3;
                    break;
                case S3:
                    pixels.set(4, wave(t), brightness);
                    pixels.set(5, wave(t), brightness);
                    LOG_DEBUG("S3");
                    ws = S4;
                    break;
                case S4:
                    pixels.set(6, wave(t), brightness);
                    pixels.set(7, wave(t), brightness);
                    LOG_DEBUG("S4");
                    ws = S_CLEAN;
                    break;
                case S_CLEAN:
                    pixels.clear();
                    LOG_DEBUG("S_CLEAN");
                    ws = S1;
                    break;
                }
                pixels.show();
            }


            break;
        }
        /// ===================== SHOW_IMU ====================
        case WorkState::SHOW_IMU: {
            imu.update();

            if (dt > 0.1f || dt < 0.001f) {
                LOG_ERROR("dt out of range, dt = {}, use default 0.02.", dt);
                dt = 0.02f;
            }

            // 获取原始加速度
            float acc_x_raw = imu.getX();
            float acc_y_raw = imu.getY();

            // 高通滤波：提取动态加速度
            // 更新低频成分（缓慢变化的偏移）
            acc_x_lpf = lpf_alpha * acc_x_lpf + (1 - lpf_alpha) * acc_x_raw;
            acc_y_lpf = lpf_alpha * acc_y_lpf + (1 - lpf_alpha) * acc_y_raw;

            // 高通滤波结果 = 原始信号 - 低频成分
            acc_x_hpf = acc_x_raw - acc_x_lpf;
            acc_y_hpf = acc_y_raw - acc_y_lpf;

            // 平滑处理（轻度滤波以减少噪声）
            acc_x_smooth = smoothing_alpha * acc_x_smooth + (1 - smoothing_alpha) * acc_x_hpf;
            acc_y_smooth = smoothing_alpha * acc_y_smooth + (1 - smoothing_alpha) * acc_y_hpf;

            // 应用死区
            float acc_x = (abs(acc_x_smooth) > deadzone) ? acc_x_smooth : 0.0f;
            float acc_y = (abs(acc_y_smooth) > deadzone) ? acc_y_smooth : 0.0f;

            // 转换为 m/s²
            float acc_x_ms2 = acc_x * 9.81f;
            float acc_y_ms2 = acc_y * 9.81f;

            // 积分得到速度
            vel_x += acc_x_ms2 * dt;
            vel_y += acc_y_ms2 * dt;

            // 速度死区和轻度衰减（仅在小速度时）
            if (abs(vel_x) < velocity_threshold) {
                vel_x *= 0.9f; // 快速衰减接近零的速度
            } else {
                vel_x *= 0.99f; // 运动中的速度轻度衰减
            }

            if (abs(vel_y) < velocity_threshold) {
                vel_y *= 0.9f;
            } else {
                vel_y *= 0.99f;
            }

            // 积分得到位置
            pos_x += vel_x * dt;
            pos_y += vel_y * dt;

            // 缩放到LED矩阵坐标
            float display_x = pos_x * position_scale;
            float display_y = pos_y * position_scale;

            // 限制显示范围
            display_x = constrain(display_x, -50.0f, 50.0f);
            display_y = constrain(display_y, -50.0f, 50.0f);

            led_matrix.clean();
            led_matrix.draw_point(0, display_y);
            led_matrix.draw_point(1, display_y);
            led_matrix.draw_point(2, display_y);
            led_matrix.draw_point(3, display_y);
            led_matrix.draw_point(4, display_y);
            led_matrix.draw_point(5, display_y);
            led_matrix.draw_point(6, display_y);
            led_matrix.draw_point(7, display_y);
            led_matrix.draw_point(8, display_y);
            led_matrix.draw_point(9, display_y);
            led_matrix.draw_point(10, display_y);
            led_matrix.draw_point(11, display_y);
            led_matrix.show();

            // 调试输出 - 每隔一段时间输出一次
            static uint32_t last_log_time = 0;
            if (millis() - last_log_time > 200) {
                LOG_INFO("Pos: {}, {}; Vel: {}, {}, Acc: {}, {}", pos_x, pos_y, vel_x, vel_y, acc_x, acc_y);
                last_log_time = millis();
            }
            break;
        }
        /// ===================== SHOW_KNOB ====================
        case WorkState::SHOW_KNOB: {
            if (knob.isPressed()) {
                if (knob.get() != 0) {
                    LOG_INFO("Knob at pos:{} fine", knob.get());
                }
                knob.set(0);
            } else {
                auto val = constrain(knob.get(), -999, 9999);
                knob.set(val);

                // pixels.set(1, ModulinoColor{ 100, 100, 100 }, map(val, -999, 9999, 0, 100));
                // pixels.show();

                led_matrix.clear();
                led_matrix.print(val);
            }
            break;
        }
        }
    }
}
