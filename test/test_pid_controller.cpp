// test/test_pid_controller.cpp
#include "pid_controller.hpp"
#include <unity.h>

using namespace ctrl;

void setUp(void) {
    // 每个测试前执行
}

void tearDown(void) {
    // 每个测试后执行
}

// 测试构造函数
void test_default_constructor(void) {
    pid_controller pid;

    TEST_ASSERT_EQUAL_DOUBLE(0.0, pid.get_target());
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pid.get_kp());
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pid.get_ki());
    TEST_ASSERT_EQUAL_DOUBLE(0.0, pid.get_kd());
}

void test_parameterized_constructor(void) {
    pid_controller pid(1.0, 0.5, 0.2);

    TEST_ASSERT_EQUAL_DOUBLE(1.0, pid.get_kp());
    TEST_ASSERT_EQUAL_DOUBLE(0.5, pid.get_ki());
    TEST_ASSERT_EQUAL_DOUBLE(0.2, pid.get_kd());
}

// 测试 setter/getter
void test_set_get_target(void) {
    pid_controller pid;
    pid.set_target(100.0);

    TEST_ASSERT_EQUAL_DOUBLE(100.0, pid.get_target());
}

void test_set_get_gains(void) {
    pid_controller pid;

    pid.set_kp(2.0);
    pid.set_ki(1.5);
    pid.set_kd(0.8);

    TEST_ASSERT_EQUAL_DOUBLE(2.0, pid.get_kp());
    TEST_ASSERT_EQUAL_DOUBLE(1.5, pid.get_ki());
    TEST_ASSERT_EQUAL_DOUBLE(0.8, pid.get_kd());
}

// 测试纯 P 控制器
void test_p_controller_only(void) {
    pid_controller pid(1.0, 0.0, 0.0);
    pid.set_target(100.0);

    // 第一次更新
    double output = pid.update(50.0, dura_t{ 1000 }); // error = 50
    TEST_ASSERT_EQUAL_DOUBLE(50.0, output);           // P = 50 * 1.0 = 50

    // 第二次更新
    output = pid.update(75.0, dura_t{ 2000 }); // error = 25
    TEST_ASSERT_EQUAL_DOUBLE(25.0, output);    // P = 25 * 1.0 = 25
}

// 测试 PI 控制器
void test_pi_controller(void) {
    pid_controller pid(1.0, 0.1, 0.0);
    pid.set_target(100.0);

    // 第一次更新 (dt = 0 因为是第一次采样)
    double output = pid.update(50.0, dura_t{ 0 });
    TEST_ASSERT_EQUAL_DOUBLE(50.0, output); // P = 50, I = 0

    // 第二次更新 (dt = 1000)
    output = pid.update(60.0, dura_t{ 1000 });
    // error = 40, P = 40
    // integral = 50*0 + 40*1000 = 40000
    // I = 40000 * 0.1 = 4000
    // output = 40 + 4000 = 4040
    TEST_ASSERT_EQUAL_DOUBLE(4040.0, output);
}

// 测试 PD 控制器
void test_pd_controller(void) {
    pid_controller pid(1.0, 0.0, 0.5);
    pid.set_target(100.0);

    // 第一次更新
    double output = pid.update(50.0, dura_t{ 0 });
    TEST_ASSERT_EQUAL_DOUBLE(50.0, output); // P = 50, D = 0 (第一次采样)

    // 第二次更新
    output = pid.update(70.0, dura_t{ 1000 });
    // error = 30, P = 30
    // derivative = -(30 - 50) / 1000 = 20 / 1000 = 0.02
    // D = 0.02 * 0.5 = 0.01
    // output = 30 + 0.01 = 30.01
    TEST_ASSERT_EQUAL_DOUBLE(30.01, output);
}

// 测试完整的 PID 控制器
void test_full_pid_controller(void) {
    pid_controller pid(1.0, 0.01, 0.5);
    pid.set_target(100.0);

    // 第一次更新
    double output1 = pid.update(50.0, dura_t{ 0 });
    // error = 50, P = 50, I = 0, D = 0
    TEST_ASSERT_EQUAL_DOUBLE(50.0, output1);

    // 第二次更新
    double output2 = pid.update(60.0, dura_t{ 1000 });
    // error = 40
    // P = 40
    // integral = 0 + 40*1000 = 40000, I = 40000 * 0.01 = 400
    // derivative = -(40-50)/1000 = 0.01, D = 0.01 * 0.5 = 0.005
    // output = 40 + 400 + 0.005 = 440.005
    TEST_ASSERT_EQUAL_DOUBLE(440.005, output2);
}

// 测试 reset 功能
void test_reset(void) {
    pid_controller pid(1.0, 0.1, 0.5);
    pid.set_target(100.0);

    // 运行几次更新
    pid.update(50.0, dura_t{ 0 });
    pid.update(60.0, dura_t{ 1000 });
    pid.update(70.0, dura_t{ 2000 });

    // 重置
    pid.reset();

    // 重置后的第一次更新应该像初始状态
    double output = pid.update(50.0, dura_t{ 3000 });
    TEST_ASSERT_EQUAL_DOUBLE(50.0, output); // 应该只有 P 分量
}

// 测试零误差情况
void test_zero_error(void) {
    pid_controller pid(1.0, 0.1, 0.5);
    pid.set_target(100.0);

    pid.update(100.0, dura_t{ 0 });
    double output = pid.update(100.0, dura_t{ 1000 });

    TEST_ASSERT_EQUAL_DOUBLE(0.0, output);
}

// 测试负误差
void test_negative_error(void) {
    pid_controller pid(1.0, 0.0, 0.0);
    pid.set_target(50.0);

    double output = pid.update(75.0, dura_t{ 0 });
    TEST_ASSERT_EQUAL_DOUBLE(-25.0, output); // error = 50 - 75 = -25
}

// 测试积分累积
void test_integral_accumulation(void) {
    pid_controller pid(0.0, 1.0, 0.0);
    pid.set_target(100.0);

    pid.update(50.0, dura_t{ 0 });

    // 持续的误差应该累积
    double output1 = pid.update(50.0, dura_t{ 100 });
    double output2 = pid.update(50.0, dura_t{ 200 });

    TEST_ASSERT_GREATER_THAN(output1, output2); // 积分应该增加
}

int main() {
    UNITY_BEGIN();

    RUN_TEST(test_default_constructor);
    RUN_TEST(test_parameterized_constructor);
    RUN_TEST(test_set_get_target);
    RUN_TEST(test_set_get_gains);
    RUN_TEST(test_p_controller_only);
    RUN_TEST(test_pi_controller);
    RUN_TEST(test_pd_controller);
    RUN_TEST(test_full_pid_controller);
    RUN_TEST(test_reset);
    RUN_TEST(test_zero_error);
    RUN_TEST(test_negative_error);
    RUN_TEST(test_integral_accumulation);

    UNITY_END();
}
