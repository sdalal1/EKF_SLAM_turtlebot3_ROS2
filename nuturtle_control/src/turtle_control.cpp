#include <chrono>
#include <functional>
#include <memory>
#include <string>
#include <vector>

#include "std_msgs/msg/u_int64.hpp"
#include "rclcpp/rclcpp.hpp"
#include "geometry_msgs/msg/twist.hpp"
#include "sensor_msgs/msg/joint_state.hpp"
#include "turtlelib/diff_drive.hpp"
#include "turtlelib/se2d.hpp"
#include "turtlelib/geometry2d.hpp"
#include "nuturtlebot_msgs/msg/sensor_data.hpp"
#include "nuturtlebot_msgs/msg/wheel_commands.hpp"

using namespace std::chrono_literals;
using turtlelib::almost_equal;


class TurtleControl : public rclcpp::Node
{
public:
  TurtleControl()
  : Node("turtle_control")
  {
    declare_parameter("wheel_radius", wheel_radius_);
    declare_parameter("track_width", track_width_);
    declare_parameter("motor_cmd_max", motor_cmd_max_);
    declare_parameter("encoder_ticks_per_rad", encoder_ticks_per_rad_);
    declare_parameter("collision_radius", collision_radius_);

    wheel_radius_ = get_parameter("wheel_radius").as_double();
    track_width_ = get_parameter("track_width").as_double();
    motor_cmd_max_ = get_parameter("motor_cmd_max").as_double();
    encoder_ticks_per_rad_ = get_parameter("encoder_ticks_per_rad").as_double();
    collision_radius_ = get_parameter("collision_radius").as_double();
    timer_ = create_wall_timer(
      1000ms, std::bind(&TurtleControl::timer_callback, this));
    velo_subscription = create_subscription<geometry_msgs::msg::Twist>(
      "cmd_vel", 10, std::bind(&TurtleControl::velo_callback, this, std::placeholders::_1));
    velo_publish = create_publisher<nuturtlebot_msgs::msg::WheelCommands>(
      "wheel_cmd", 10);
    sensor_subscription = create_subscription<nuturtlebot_msgs::msg::SensorData>(
      "sensor_data", 10, std::bind(&TurtleControl::sensor_callback, this, std::placeholders::_1));
    joint_publish = create_publisher<sensor_msgs::msg::JointState>(
      "joint_states", 10);

    if(wheel_radius_ == 0.0 || track_width_ ==0.0 || motor_cmd_max_ ==0.0|| encoder_ticks_per_rad_ == 0.0|| collision_radius_ ==0.0){
      RCLCPP_ERROR_STREAM(this->get_logger(), "a paramter not defined not defined");
      RCLCPP_ERROR_STREAM(this->get_logger(), "wheel_radius"<<wheel_radius_);
      RCLCPP_ERROR_STREAM(this->get_logger(), "track_width"<<track_width_);
      RCLCPP_ERROR_STREAM(this->get_logger(), "motor_cmd_max"<<motor_cmd_max_);
      RCLCPP_ERROR_STREAM(this->get_logger(), "encoder_ticks_per_rad"<<encoder_ticks_per_rad_);
      RCLCPP_ERROR_STREAM(this->get_logger(), "collision_radius"<<collision_radius_);
      throw std::runtime_error("runtime_error_exception");
    }
  }

private:
  rclcpp::TimerBase::SharedPtr timer_;
  double wheel_radius_ = 0.0;
  double track_width_ = 0.0;
  double motor_cmd_max_ = 0.0;
  double encoder_ticks_per_rad_ = 0.0;
  double collision_radius_ = 0.0;

  rclcpp::Subscription<geometry_msgs::msg::Twist>::SharedPtr velo_subscription;
  rclcpp::Publisher<nuturtlebot_msgs::msg::WheelCommands>::SharedPtr velo_publish;
  rclcpp::Subscription<nuturtlebot_msgs::msg::SensorData>::SharedPtr sensor_subscription;
  rclcpp::Publisher<sensor_msgs::msg::JointState>::SharedPtr joint_publish;

  void timer_callback()
  {
    RCLCPP_INFO(this->get_logger(), "Publishing:");
  }

  void velo_callback(const geometry_msgs::msg::Twist::SharedPtr msg){
    auto a = msg->angular.z;
    auto lx = msg->linear.x;
    auto ly = msg->linear.y;
    turtlelib::DiffDrive diff(track_width_, wheel_radius_);
    turtlelib::Twist2D twist{a, lx, ly};
    auto wheel_angle =  diff.compute_ik(twist);
    auto pub_wheel = nuturtlebot_msgs::msg::WheelCommands();
    pub_wheel.left_velocity = wheel_angle.left / 0.024;
    pub_wheel.right_velocity = wheel_angle.right / 0.024;

    if (pub_wheel.left_velocity > 264 || pub_wheel.right_velocity > 264){
      pub_wheel.left_velocity = 264;
      pub_wheel.right_velocity = 264;
    }
    else if(pub_wheel.left_velocity < -264 || pub_wheel.right_velocity < -264){
      pub_wheel.left_velocity = -264;
      pub_wheel.right_velocity = -264;
    }

    velo_publish -> publish(pub_wheel);
  };

  void sensor_callback(const nuturtlebot_msgs::msg::SensorData::SharedPtr msg){
    auto left_en = msg->left_encoder;
    auto right_en = msg->right_encoder;

    

  };
};

/// \brief The main function to spin the node
int main(int argc, char * argv[])
{
  rclcpp::init(argc, argv);
  rclcpp::spin(std::make_shared<TurtleControl>());
  rclcpp::shutdown();
  return 0;

}