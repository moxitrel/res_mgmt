FROM  debian

RUN apt-get update
RUN apt-get -y install \
    cmake       \
    make        \
    g++         \
                \
    cppcheck    \
                \
    googletest  \
