#!/bin/bash
set -e
source /opt/ros/humble/setup.bash
cd /arena_camera_ros2/ros2_ws 
rosdep install --from-paths src --ignore-src -r -y
rm -rf build install log
colcon build --symlink-install
source install/local_setup.bash
exec "$@"