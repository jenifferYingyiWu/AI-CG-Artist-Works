#!/bin/sh

OCIO_VERSION="2.3.1"
OCIO_LIB_NAME="OpenColorIO-$OCIO_VERSION"

SCRIPT_DIR=`cd $(dirname "$BASH_SOURCE"); pwd`
UE_ENGINE_DIR=`cd $SCRIPT_DIR/../../..; pwd`
UE_THIRD_PARTY_DIR="$UE_ENGINE_DIR/Source/ThirdParty"

cd $SCRIPT_DIR

# Remove previously extracted build library folder
if [ -d "$OCIO_LIB_NAME" ]; then
   echo "Deleting previously extracted $OCIO_LIB_NAME folder"
   rm -rf "$OCIO_LIB_NAME"
fi

git clone --depth 1 --branch v$OCIO_VERSION https://github.com/AcademySoftwareFoundation/OpenColorIO.git $OCIO_LIB_NAME

pushd $OCIO_LIB_NAME

UE_C_FLAGS="-mmacosx-version-min=10.9 -arch x86_64 -arch arm64"
UE_CXX_FLAGS="-mmacosx-version-min=10.9 -arch x86_64 -arch arm64"

#IMATH_CMAKE_LOCATION="$UE_THIRD_PARTY_DIR/Imath/Deploy/Imath-3.1.3/Mac/lib/cmake/Imath"

# Configure OCIO cmake and launch a release build
echo "Configuring build..."
cmake -S . -B build \
    -DCMAKE_OSX_ARCHITECTURES="x86_64;arm64" \
    -DCMAKE_OSX_DEPLOYMENT_TARGET="11.0" \
    -DCMAKE_BUILD_TYPE="Release" \
    -DCMAKE_MACOSX_RPATH=TRUE \
    -DBUILD_SHARED_LIBS=ON \
    -DCMAKE_CXX_STANDARD=11 \
    -DCMAKE_C_FLAGS="${UE_C_FLAGS}" \
    -DCMAKE_CXX_FLAGS="${UE_CXX_FLAGS}" \
    -DOCIO_BUILD_APPS=OFF \
    -DOCIO_BUILD_GPU_TESTS=OFF \
    -DOCIO_BUILD_NUKE=OFF \
    -DOCIO_BUILD_DOCS=OFF \
    -DOCIO_BUILD_TESTS=OFF \
    -DOCIO_BUILD_PYTHON=OFF \
    -DOCIO_INSTALL_EXT_PACKAGES=ALL \
    -Dexpat_STATIC_LIBRARY=ON \
    -DEXPAT_C_FLAGS="${UE_C_FLAGS}" \
    -DEXPAT_CXX_FLAGS="${UE_CXX_FLAGS}" \
    -DImath_STATIC_LIBRARY=ON \
    -DImath_C_FLAGS="${UE_C_FLAGS}" \
    -DImath_CXX_FLAGS="${UE_CXX_FLAGS}" \
    -Dyaml-cpp_STATIC_LIBRARY=ON \
    -Dyaml-cpp_C_FLAGS="${UE_C_FLAGS}" \
    -Dyaml-cpp_CXX_FLAGS="${UE_CXX_FLAGS}" \
    -Dpystring_STATIC_LIBRARY=ON \
    -Dpystring_C_FLAGS="${UE_C_FLAGS}" \
    -Dpystring_CXX_FLAGS="${UE_CXX_FLAGS}" \
    -Dminizip-ng_STATIC_LIBRARY=ON

echo "Building Release build..."
cmake --build build --config Release

echo "Copying library build files..."
cp build/src/OpenColorIO/libOpenColorIO.2.3.1.dylib $UE_ENGINE_DIR/Binaries/ThirdParty/OpenColorIO/Mac/libOpenColorIO.2.3.dylib
# cp build/src/OpenColorIO/libOpenColorIO.2.3.1.dylib $UE_ENGINE_DIR/Binaries/ThirdParty/OpenColorIO/Mac/libOpenColorIO.dylib
# install_name_tool -id @rpath/libOpenColorIO.dylib $UE_ENGINE_DIR/Binaries/ThirdParty/OpenColorIO/Mac/libOpenColorIO.dylib

popd
