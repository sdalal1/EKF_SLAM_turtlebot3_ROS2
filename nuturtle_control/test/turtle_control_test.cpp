#include <chrono>

#include "catch_ros2/catch_ros2.hpp"
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "nuturtlebot_msgs/msg/wheel_commands.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "nuturtlebot_msgs/msg/sensor_data.hpp"


using namespace std::chrono_literals;

geometry_msgs::msg::Twist twist_test = geometry_msgs::msg::Twist();

double left_wheel_test=100.0;
double right_wheel_test;

void velo_callback(const nuturtlebot_msgs::msg::WheelCommands::SharedPtr msg){
    left_wheel_test = msg->left_velocity;
    right_wheel_test = msg->right_velocity;
}

TEST_CASE("Linear speed test", "[linear_speed]"){

auto node = rclcpp::Node::make_shared("turtle_control");

auto publisher_cmd = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

twist_test.linear.x = 0.1;
twist_test.angular.z = 0.0;


auto sub_wheel_cmd = node->create_subscription<nuturtlebot_msgs::msg::WheelCommands>(
"wheel_cmd", 
10, &velo_callback);

rclcpp::Time start_time = rclcpp::Clock().now();

while (
    rclcpp::ok() &&
    ((rclcpp::Clock().now() - start_time) < 1s)
  )
  {
    // Repeatedly check for the dummy service until its found
    rclcpp::spin_some(node);
    publisher_cmd->publish(twist_test);
  }

  // Test assertions - check that the dummy node was found
  CHECK_THAT(left_wheel_test, Catch::Matchers::WithinAbs(126.0, 1e-5));
  CHECK_THAT(right_wheel_test, Catch::Matchers::WithinAbs(126.0, 1e-5));
};

TEST_CASE("Rotation speed test", "[rotation_speed]"){

auto node = rclcpp::Node::make_shared("turtle_control");

auto publisher_cmd = node->create_publisher<geometry_msgs::msg::Twist>("cmd_vel", 10);

twist_test.linear.x = 0.0;
twist_test.angular.z = 0.1;


auto sub_wheel_cmd = node->create_subscription<nuturtlebot_msgs::msg::WheelCommands>(
"wheel_cmd", 
10, &velo_callback);

rclcpp::Time start_time = rclcpp::Clock().now();

while (
    rclcpp::ok() &&
    ((rclcpp::Clock().now() - start_time) < 1s)
  )
  {
    // Repeatedly check for the dummy service until its found
    rclcpp::spin_some(node);
    publisher_cmd->publish(twist_test);
  }

  // Test assertions - check that the dummy node was found
  CHECK_THAT(left_wheel_test, Catch::Matchers::WithinAbs(-10.0, 1e-5));
  CHECK_THAT(right_wheel_test, Catch::Matchers::WithinAbs(10.0, 1e-5));
};

double left_angle, left_velo, right_angle, right_velo;
void joint_callback(sensor_msgs::msg::JointState::SharedPtr msg){
    auto timestamp = msg->header.stamp.nanosec;
    // std::cout<<"time"<<timestamp;
    // std::cout<<"position_end"<<*(msg->position.end());
    left_angle = msg->position.at(0);
    right_angle = msg->position.at(1);
    left_velo = msg->velocity.at(0);
    right_velo = msg->velocity.at(1);
};

nuturtlebot_msgs::msg::SensorData sensor_test = nuturtlebot_msgs::msg::SensorData();
nuturtlebot_msgs::msg::SensorData sensor_test2 = nuturtlebot_msgs::msg::SensorData();

TEST_CASE("Joint state calculation", "[joint_state]"){

auto node = rclcpp::Node::make_shared("turtle_control");

auto publisher_sensor = node->create_publisher<nuturtlebot_msgs::msg::SensorData>("sensor_data", 10);
sensor_test.stamp.nanosec = rclcpp::Clock().now().nanoseconds();
sensor_test.left_encoder = 126.0;
sensor_test.right_encoder = 126.0;

sensor_test2.stamp.nanosec = rclcpp::Clock().now().nanoseconds() + (100*1e9);
sensor_test2.left_encoder = 156.0;
sensor_test2.right_encoder = 156.0;

auto sub_joint_state = node->create_subscription<sensor_msgs::msg::JointState>(
"joint_states", 
10, &joint_callback);

rclcpp::Time start_time = rclcpp::Clock().now();

while (
    rclcpp::ok() &&
    ((rclcpp::Clock().now() - start_time) < 3s)
  )
  {
    // Repeatedly check for the dummy service until its found
    
    publisher_sensor->publish(sensor_test);
    publisher_sensor->publish(sensor_test2);
    rclcpp::spin_some(node);
  }
  rclcpp::spin_some(node);
  // Test assertions - check that the dummy node was found
  CHECK_THAT(left_angle, Catch::Matchers::WithinAbs(82139.2299322029, 1e-5));
  CHECK_THAT(right_angle, Catch::Matchers::WithinAbs(82139.2299322029, 1e-5));
//   CHECK_THAT(left_velo, Catch::Matchers::WithinAbs(-6351.2815257095, 1e-5));
//   CHECK_THAT(right_velo, Catch::Matchers::WithinAbs(-6351.2815257095, 1e-5));

};