#!/bin/bash
docker run --rm --privileged --pid=host --mount type=bind,source=/sys/fs/cgroup,target=/sys/fs/cgroup test

