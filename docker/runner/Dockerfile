FROM ubuntu:22.04

ARG DEBIAN_FRONTEND=noninteractive
ARG RUNNER_VERSION=2.319.1

RUN apt-get update && apt-get install -y \
  ca-certificates curl git jq sudo unzip tar \
  python3 python3-pip \
  build-essential \
  libicu70 \
  && rm -rf /var/lib/apt/lists/*

# Create runner user
RUN useradd -m -s /bin/bash runner && \
    usermod -aG sudo runner && \
    echo "runner ALL=(ALL) NOPASSWD:ALL" > /etc/sudoers.d/runner

USER runner
WORKDIR /home/runner

# Install GitHub Actions Runner
RUN mkdir -p actions-runner && cd actions-runner && \
    curl -L -o actions-runner.tar.gz \
      https://github.com/actions/runner/releases/download/v${RUNNER_VERSION}/actions-runner-linux-x64-${RUNNER_VERSION}.tar.gz && \
    tar xzf actions-runner.tar.gz && rm actions-runner.tar.gz

WORKDIR /home/runner/actions-runner
COPY docker/runner/entrypoint.sh /home/runner/actions-runner/entrypoint.sh
RUN chmod +x /home/runner/actions-runner/entrypoint.sh

ENTRYPOINT ["/home/runner/actions-runner/entrypoint.sh"]
