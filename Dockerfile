FROM ros:humble

RUN apt-get update && apt-get upgrade -y
RUN apt-get install -y g++ make
RUN apt-get install -y ros-$ROS_DISTRO-rmw-cyclonedds-cpp

RUN rm -rf /var/lib/apt/lists/*

ARG arenasdk_root_on_host=./resources/ArenaSDK/linux_arm64/ArenaSDK_Linux_ARM64
ARG arenasdk_root=/ArenaSDK_Linux_ARM64

# Copy entire SDK directory into container
COPY ${arenasdk_root_on_host} ${arenasdk_root}

# Manually create Arena_SDK.conf with the correct absolute paths
# (mimics what Arena_SDK_ARM64.conf script does, but without $CURRENTDIR ambiguity)
RUN echo "${arenasdk_root}/lib" > /etc/ld.so.conf.d/Arena_SDK.conf \
    && echo "${arenasdk_root}/GenICam/library/lib/Linux64_ARM" >> /etc/ld.so.conf.d/Arena_SDK.conf \
    && echo "${arenasdk_root}/ffmpeg" >> /etc/ld.so.conf.d/Arena_SDK.conf \
    && ldconfig

# Setup entrypoint
ADD ./arena_camera_ros_entrypoint.sh /
ENTRYPOINT [ "/arena_camera_ros_entrypoint.sh" ]
COPY . /arena_camera_ros2
WORKDIR /arena_camera_ros2/ros2_ws