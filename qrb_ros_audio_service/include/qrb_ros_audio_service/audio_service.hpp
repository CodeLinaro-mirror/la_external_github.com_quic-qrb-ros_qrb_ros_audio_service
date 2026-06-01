// Copyright (c) Qualcomm Technologies, Inc. and/or its subsidiaries.
// SPDX-License-Identifier: BSD-3-Clause-Clear

#ifndef QRB_ROS_AUDIO_SERVICE__AUDIO_SERVICE_HPP_
#define QRB_ROS_AUDIO_SERVICE__AUDIO_SERVICE_HPP_

// clang-format off
#include <rclcpp/rclcpp.hpp>
#include <rmw/types.h>
#include <unordered_map>

#include "qrb_ros_audio_service_msgs/msg/audio_data.hpp"
#include "qrb_ros_audio_service_msgs/srv/audio_request.hpp"
// clang-format on

using AudioService = qrb_ros_audio_service_msgs::srv::AudioRequest;
using AudioData = qrb_ros_audio_service_msgs::msg::AudioData;

namespace qrb_ros
{
namespace audio_service
{

class AudioServer : public rclcpp::Node
{
public:
  explicit AudioServer(const rclcpp::NodeOptions & options);

private:
  rclcpp::CallbackGroup::SharedPtr callback_group_{ nullptr };
  rclcpp::Service<AudioService>::SharedPtr server_{ nullptr };

  std::unordered_map<uint32_t, rclcpp::Publisher<AudioData>::SharedPtr> capture_pubs_;
  std::unordered_map<uint32_t, rclcpp::Subscription<AudioData>::SharedPtr> playback_subs_;

  void service_callback(const std::shared_ptr<rmw_request_id_t> request_header,
      const std::shared_ptr<AudioService::Request> request,
      std::shared_ptr<AudioService::Response> response);
  void shutdown_callback();
  void on_stream_data(uint32_t am_handle, const void * data, size_t size);
  void on_audio_data(uint32_t am_handle,
      const qrb_ros_audio_service_msgs::msg::AudioData::SharedPtr msg);
  void create_pcm_topic(uint32_t stream_handle,
      const std::string & topic_name,
      bool is_capture);
  void delete_pcm_topic(uint32_t stream_handle);
};

}  // namespace audio_service
}  // namespace qrb_ros

#endif  // QRB_ROS_AUDIO_SERVICE__AUDIO_SERVICE_HPP_
