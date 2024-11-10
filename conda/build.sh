#!/usr/bin/env bash

set -e

echo -e "\033[1;34m--- PREPARE ---\033[0m"

case "$ARCH" in
  64)
    CFLAGS=$(echo "$CFLAGS" | sed -e 's/-march=\[a-zA-Z0-9]*/-march=skylake/g' -e 's/-mtune=[a-zA-Z0-9]*/-mtune=skylake/g')
    CXXFLAGS=$(echo "$CXXFLAGS" | sed -e 's/-march=[a-zA-Z0-9]*/-march=skylake/g' -e 's/-mtune=[a-zA-Z0-9]*/-mtune=skylake/g')
    ;;
  aarch64|arm64)
    ;;
  *)
    (>&2 echo -e "\033[1;31mERROR: Unsupported ARCH=$ARCH.\033[0m") && exit 1
    ;;
esac

CFLAGS=$(echo "$CFLAGS" | sed -e 's/-O[0-9]/-O3/g')
CPPFLAGS=$(echo "$CPPFLAGS" | sed -e 's/-O[0-9]/-O3/g')
CXXFLAGS=$(echo "$CXXFLAGS" | sed -e 's/-O[0-9]/-O3/g')
LDFLAGS=$(echo "$LDFLAGS" | sed -e 's/-O[0-9]/-O3/g')

if [[ $build_platform == *"linux"* ]]; then
  CPPFLAGS="$CPPFLAGS -Wno-psabi -Wno-stringop-overflow"
fi

if [[ $build_platform == *"osx"* ]]; then
  CPPFLAGS="$CPPFLAGS -D_LIBCPP_DISABLE_AVAILABILITY"
fi

CXXFLAGS+=" $CPPFLAGS"  # CMake doesn't use CPPFLAGS

echo -e "\033[1;34m--- ENVIRONMENT ---\033[0m"

env | sort

echo -e "\033[1;34m--- CONFIGURE ---\033[0m"

cmake ${CMAKE_ARGS} \
  -DCMAKE_INSTALL_PREFIX=$PREFIX \
  -DBUILD_TESTING=ON \
  -DBUILD_DOCS=OFF \
  .

echo -e "\033[1;34m--- BUILD ---\033[0m"

cmake --build . \
  --clean-first \
  --parallel=${CPU_COUNT:-2} \
  -- VERBOSE=1

echo -e "\033[1;34m--- INSTALL ---\033[0m"

cmake --install .

echo -e "\033[1;34m--- DONE ---\033[0m"
