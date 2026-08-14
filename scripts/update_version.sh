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

if [[ "$version" =~ ^([0-9]+)\.([0-9]+)(\.([0-9]+))?(-[A-Za-z0-9.-]+)?$ ]]; then
    major="${BASH_REMATCH[1]}"
    minor="${BASH_REMATCH[2]}"
    patch="${BASH_REMATCH[4]:-0}"
    short_version="${major}.${minor}"
fi

repo_root="$(cd "$(dirname "${BASH_SOURCE[0]}")/.." && pwd)"
docs_conf="${repo_root}/docs/conf.py"
py_init="${repo_root}/python/verisocks/__init__.py"
verisocks_h="${repo_root}/include/verisocks.h"
vsl_h="${repo_root}/include/vsl.h"

if [[ ! -f "$docs_conf" ]]; then
    echo "Missing documentation config: $docs_conf" >&2
    exit 1
fi

if [[ ! -f "$py_init" ]]; then
    echo "Missing Python package init: $py_init" >&2
    exit 1
fi

if [[ ! -f "$verisocks_h" ]]; then
    echo "Missing Verisocks header: $verisocks_h" >&2
    exit 1
fi

if [[ ! -f "$vsl_h" ]]; then
    echo "Missing VSL header: $vsl_h" >&2
    exit 1
fi

sed -i -E "s#^(version = ).*#\1'${short_version}'#" "$docs_conf"
sed -i -E "s#^(release = ).*#\1'${version}'#" "$docs_conf"
sed -i -E "s#^(__version__ = \".*\")#__version__ = \"${version}\"#" "$py_init"
sed -i -E "s|^(#define VS_VERSION_MAJOR ).*|\1${major}|" "$verisocks_h"
sed -i -E "s|^(#define VS_VERSION_MINOR ).*|\1${minor}|" "$verisocks_h"
sed -i -E "s|^(#define VS_VERSION_PATCH ).*|\1${patch}|" "$verisocks_h"
sed -i -E "s|^(#define VSL_VERSION_MAJOR ).*|\1${major}|" "$vsl_h"
sed -i -E "s|^(#define VSL_VERSION_MINOR ).*|\1${minor}|" "$vsl_h"
sed -i -E "s|^(#define VSL_VERSION_PATCH ).*|\1${patch}|" "$vsl_h"

echo "Updated Verisocks version to ${version}"
echo "  docs/conf.py: version=${short_version}, release=${version}"
echo "  python/verisocks/__init__.py: __version__=${version}"
echo "  include/verisocks.h: VS_VERSION=${major}.${minor}.${patch}"
echo "  include/vsl.h: VSL_VERSION=${major}.${minor}.${patch}"

