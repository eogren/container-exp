#!/bin/bash

# Still using PID namespace, but no privileged mode; can't change cgroups
docker run --rm --pid=host test

