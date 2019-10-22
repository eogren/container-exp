#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/stat.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <fcntl.h>

int should_wait = 0;

void write_pid_to_buf(const char* buf);

void dump_cgroup(const char *prefix) {
    FILE *fp = fopen("/proc/self/cgroup", "r");
    if (fp == NULL) {
      perror("open cgroup");
      return;
    }

    char *line = NULL;
    size_t len = 0;
    ssize_t read;

    while ((read = getline(&line, &len, fp)) != -1) {
      fprintf(stderr, "%s: %s", prefix, line);
    }
}

void switch_groups(const char* cgroupname) {
    if (geteuid() != 0) {
      fprintf(stderr, "not root, can't switch cgroups\n");
      return;
    }

    char buf[1024];

    const char * const namespaces[] = {
      "blkio",
      "cpu,cpuacct",
      "cpuset",
      "devices",
      "freezer",
      "hugetlb",
      "memory",
      "net_cls,net_prio",
      "perf_event",
      "pids",
      "systemd"
    };

    int i;
    for (i = 0; i < sizeof(namespaces)/sizeof(namespaces[0]); i++) {
	strcpy(buf, "/sys/fs/cgroup/");
        strcat(buf, namespaces[i]);
        strcat(buf, "/");
        strcat(buf, cgroupname);
  
	fprintf(stderr, "Creating %s\n", buf); 
        int err = mkdir(buf, 0700);
        if (err == -1 && errno != EEXIST) {
          perror("mkdir");
          continue;
        }

       write_pid_to_buf(buf);
    } 
    fprintf(stderr, "Switched cgroup to %s\n", cgroupname);
}

void write_pid_to_buf(const char* buf) {
    char new_buf[1024];
    sprintf(new_buf, "%s/cgroup.procs", buf);
    fprintf(stderr, "Opening %s\n", new_buf);
    int fd = open(new_buf, O_WRONLY|O_CREAT|O_TRUNC);
    if (fd == -1) {
      perror("open");
      return;
    }

    const char *zero = "0"; 
    ssize_t bytes_written = write(fd, zero, 1);
    if (bytes_written <= 0) {
      perror("write to cgroup");
    }
    close(fd);
}    
    
int main(int argc, char **argv) {
    fprintf(stderr, "About to fork...\n");

    pid_t child_pid = fork();
    if (child_pid != 0) {
        // I am the parent
	fprintf(stderr, "I am the parent... dumping cgroup\n");
	dump_cgroup("parent");
        if (should_wait) {
        int status;
        pid_t ret = waitpid(child_pid, &status, 0);
        if (ret != child_pid) {
            perror("waiting for child");
        } else {
            fprintf(stderr, "Child process completed\n");
        }
        } else {
           fprintf(stderr, "Not waiting - exiting immediately. Let's see if child survives...\n");
        }
        return 0;
    }

    // I am the child
    pid_t pid = setsid();
    if (pid == -1) {
        perror("setsid");
        return 1;
    }

    switch_groups("myawesome");
    fprintf(stderr, "I am the child...dumping cgroup\n");
    dump_cgroup("child");
    fprintf(stderr, "I am the child...sleeping for 60s\n");
    sleep(60);

    fprintf(stderr, "Awake, trying to read from file\n");
    FILE* fp = fopen("/README.md", "r");
    if (fp == NULL) {
        perror("opening /README.md");
        return 1;
    }

    char buf[1024];
    size_t bytes_read;
    while ((bytes_read = fread(buf, 1, 1024 - 1, fp)) > 0) {
        buf[bytes_read] = 0;
        fprintf(stderr, "%s", buf);
    }

    fprintf(stderr, "Done.\n");
    return 0;
}
