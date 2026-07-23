#!/usr/bin/env bash
set -euo pipefail

repo_dir="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"
device="${1:-0}"

if [[ $# -gt 0 ]]; then
  shift
fi

exec "${repo_dir}/src/redux" -uti260b -d "${device}" "$@"
