#!/usr/bin/env bash
set -euo pipefail

: "${GITHUB_OWNER:?Set GITHUB_OWNER}"
: "${GITHUB_REPO:?Set GITHUB_REPO}"
: "${RUNNER_TOKEN:?Set RUNNER_TOKEN}"
: "${RUNNER_NAME:=g3-runner-01}"
: "${RUNNER_LABELS:=g3,linux,x86_64,usb-flash}"
: "${RUNNER_WORKDIR:=_work}"

cd /home/runner/actions-runner

if [ ! -f .runner ]; then
  ./config.sh --unattended \
    --url "https://github.com/${GITHUB_OWNER}/${GITHUB_REPO}" \
    --token "${RUNNER_TOKEN}" \
    --name "${RUNNER_NAME}" \
    --labels "${RUNNER_LABELS}" \
    --work "${RUNNER_WORKDIR}"
fi

cleanup() {
  echo "Removing runner..."
  ./config.sh remove --unattended --token "${RUNNER_TOKEN}" || true
}
trap cleanup EXIT

exec ./run.sh
