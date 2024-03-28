docker run --rm -itd \
    --privileged \
    --volume /sys:/sys \
    --volume /proc:/proc \
    --volume /lib/modules:/lib/modules \
    --volume /dev:/dev \
    tiered_shm:latest