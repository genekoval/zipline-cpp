FROM git.aurora.aur/aurora/cpp

RUN --mount=type=bind,target=. \
    cmake --workflow --preset=docker
