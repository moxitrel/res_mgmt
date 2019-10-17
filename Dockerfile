# * build
# docker image build -t registry.gitlab.com/colorate/res_mgmt .
#
FROM  debian

RUN apt-get update
RUN apt-get -y install \
    cmake       \
    make        \
    gcc         \
    g++         \
                \
    cppcheck    \
                \
    googletest  \
