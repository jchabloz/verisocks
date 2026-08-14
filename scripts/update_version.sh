#!/usr/bin/env bash
set -euo pipefail

usage() {
    echo "Usage: $0 <version>" >&2
    echo "Example: $0 1.7.0-dev" >&2
    exit 1
}

if [[ $# -ne 1 ]]; then
    usage
fi

version="$1"

if [[ ! "$version" =~ ^[0-9]+\.[0-9]+(\.[0-9]+)?(-[A-Za-z0-9.-]+)?$ ]]; then
    echo "Invalid version: $version" >&2
    echo "Expected format like 1.7, 1.7.0, or 1.7.0-dev" >&2
    exit 1
fi

short_version="$version"
major=0
minor=0
patch=0
patch_value=0
suffix=""

if [[ "$version" =~ ^([0-9]+)\.([0-9]+)\.([0-9]+)(-([A-Za-z0-9.-]+))?$ ]]; then
    major="${BASH_REMATCH[1]}"
    minor="${BASH_REMATCH[2]}"
    patch="${BASH_REMATCH[3]}"
    suffix="${BASH_REMATCH[5]:-}"
    patch_value="${patch}${suffix}"
    short_version="${major}.${minor}"
elif [[ "$version" =~ ^([0-9]+)\.([0-9]+)(-([A-Za-z0-9.-]+))?$ ]]; then
    major="${BASH_REMATCH[1]}"
    minor="${BASH_REMATCH[2]}"
    patch=0
    suffix="${BASH_REMATCH[4]:-}"
    short_version="${major}.${minor}"
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
docs_conf="${repo_root}/docs/conf.py"
py_init="${repo_root}/python/verisocks/__init__.py"
version_h="${repo_root}/include/version.h"

if [[ ! -f "$docs_conf" ]]; then
    echo "Missing documentation config: $docs_conf" >&2
    exit 1
fi

if [[ ! -f "$py_init" ]]; then
    echo "Missing Python package init: $py_init" >&2
    exit 1
fi

if [[ ! -f "$version_h" ]]; then
    echo "Missing version header: $version_h" >&2
    exit 1
fi

sed -i -E "s#^(version = ).*#\1'${short_version}'#" "$docs_conf"
sed -i -E "s#^(release = ).*#\1'${version}'#" "$docs_conf"
sed -i -E "s#^(__version__ = \".*\")#__version__ = \"${version}\"#" "$py_init"
sed -i -E "s|^(#define VERISOCKS_VERSION_MAJOR ).*|\1${major}|" "$version_h"
sed -i -E "s|^(#define VERISOCKS_VERSION_MINOR ).*|\1${minor}|" "$version_h"
sed -i -E "s|^(#define VERISOCKS_VERSION_PATCH ).*|\1${patch}|" "$version_h"
# sed -i -E "s|^(#define VERISOCKS_VERSION_PATCH ).*|\1${patch}-${suffix}|" "$version_h"

echo "Updated Verisocks version to ${version}"
echo "  docs/conf.py: version=${short_version}, release=${version}"
echo "  python/verisocks/__init__.py: __version__=${version}"
# echo "  include/version.h: VERISOCKS_VERSION=${major}.${minor}.${patch}-${suffix}"
echo "  include/version.h: VERISOCKS_VERSION=${major}.${minor}.${patch}"

