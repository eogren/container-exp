FROM ubuntu:18.04 as build

RUN apt -y update && apt -y install build-essential

COPY ./src /src
WORKDIR /src
RUN mkdir out && gcc -Wall main.c -o out/main

FROM ubuntu:18.04 as runtime
RUN groupadd -g 1000 vagrant && \
    useradd -u 1000 -g 1000 vagrant
COPY ./README.md /
COPY --from=build /src/out/main /main_in_docker
#USER vagrant
WORKDIR /
CMD /main_in_docker

 
