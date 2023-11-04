basedir=$PWD
runner_os=${RUNNER_OS:-`uname -s`}
case $runner_os in
  macOS|Darwin) os=mac ;;
  Linux) os=linux ;;
  Windows|MINGW*) os=windows ; basedir=$(pwd -W) ;;
  *) echo "unknown runner_os: $runner_os" ; exit 7 ; ;;
esac
version=${VULKAN_SDK_VERSION:-${{ inputs.version }}}
sdk_dir=${VULKAN_SDK:-$basedir/VULKAN_SDK}
test -d $sdk_dir || mkdir -pv $sdk_dir
if [[ $version == 'latest' ]] ; then
  url=https://vulkan.lunarg.com/sdk/latest/$os.txt
  echo "note: resolving '$version' for '$os' via webservices lookup: $url" >&2
  version=$(curl -sL $url)
  test -n "$version" || { echo "could not resolve latest version" ; exit 9 ; }
  [[ "${{ inputs.quiet }}" == "true" ]] || echo "::notice title=Using Vulkan SDK $version::resolved via '$url'"
fi

export VULKAN_SDK=$sdk_dir
export VULKAN_SDK_VERSION=$version
export VULKAN_SDK_PLATFORM=$os

./vulkan-sdk-install/vulkan_prebuilt_helpers.sh
download_vulkan_installer ${VULKAN_SDK_PLATFORM}
unpack_vulkan_installer ${VULKAN_SDK_PLATFORM}
$VULKAN_SDK/bin/glslangValidator --version