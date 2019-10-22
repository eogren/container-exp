Based on hadoop/hadoop-yarn-project/hadoop-yarn/hadoop-yarn-server/hadoop-yarn-server-nodemanager/src/main/native/container-executor/impl

Executable will consist of:

1. A parent process that forks() a child and optionally waits for it to complete. (Right now this is just hardcoded at the top of the .c file)

1. A child process that will try to move itself into a new cgroup, then sleep for 60 seconds, then try to cat /README.md (this is to test if the root filesystem is still around somewhere). If the container is not running in privileged mode or /sys/fs/cgroups isn't mounted, the rebind will fail.

A few experiments:

1. If we run this experiment in a Centos VM, the child process will outlive the parent as we would expect.

1. If PID namespaces are on, or if the child process cannot cgroup itself, when the parent exits, the child is automatically killed as we guessed.

1. If `run_privileged.sh` is used to run in privileged mode, it seems like the child can outlive the parent and still access info in the overlayfs. Docker reports the container as running but it does shut down when the container exits. Seems promising?

1. Seems like the `cpuset` cgroup isn't getting moved for some reason ... I don't know if this is keeping the container alive or is a bug.