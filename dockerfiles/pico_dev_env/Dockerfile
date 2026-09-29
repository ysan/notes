FROM debian:bookworm-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  cmake \
  git \
  python3 \
  gcc-arm-none-eabi \
  libnewlib-arm-none-eabi \
  libstdc++-arm-none-eabi-newlib \
  pkg-config \
  libusb-1.0-0-dev \
  ca-certificates \
  sudo \
  vim \
  && rm -rf /var/lib/apt/lists/*

# ユーザーを追加します。ホスト側のUID/GIDに合わせることで、bind mount先の
# ファイルがrootで汚染されないようにします。
ARG UID=1000
ARG GID=1000
ARG USER=builder
ARG GROUP=builder
RUN groupadd -g ${GID} ${GROUP} && \
  useradd -m -s /bin/bash -u ${UID} -g ${GID} -G sudo ${USER} && \
  echo "${USER} ALL=NOPASSWD: ALL" > /etc/sudoers.d/container_users

# ユーザーを切り替えます
USER ${USER}
WORKDIR /home/${USER}

CMD ["/bin/bash"]


# =============================================================================
# Raspberry Pi Pico (RP2040) / Pico 2 (RP2350) ビルド環境
# SDK等のソースはコンテナに焼き込まず、host側でcloneしてbind mountします。
# =============================================================================


# -----------------------------------------------------------------------------
# --- host側でこのDockerfileと同じ場所にリポジトリをcloneします ---
# git clone -b 2.3.1 --depth=1 --recurse-submodules --shallow-submodules \
#   https://github.com/raspberrypi/pico-sdk.git pico-sdk-2.3.1

# git clone -b 2.3.1 --depth=1 https://github.com/raspberrypi/picotool.git picotool-2.3.1

# (RTOSを使うなら) FreeRTOS-Kernelも同じ場所にcloneします。詳細は末尾の
# 「FreeRTOSを組み込む場合」を参照してください。
# git clone -b V11.2.0 --depth=1 \
#   https://github.com/FreeRTOS/FreeRTOS-Kernel.git freertos-kernel-V11.2.0

# 自分のアプリのソースは別ディレクトリに用意します (例: ./skeleton)


# --- docker build ---
# docker build -t pico-builder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .


# --- daemonモードで起動 ---
# --device=/dev/bus/usb はpicotoolでのUSB書き込み用。書き込みをしないなら外してよい。
# docker run -it -d --name pico-builder-dev \
#   --mount type=bind,src=$(pwd)/pico-sdk-2.3.1,dst=/home/builder/pico-sdk \
#   --mount type=bind,src=$(pwd)/picotool-2.3.1,dst=/home/builder/picotool \
#   --mount type=bind,src=$(pwd)/skeleton,dst=/home/builder/skeleton \
#   --device=/dev/bus/usb \
#   pico-builder
#
# (RTOSを使うなら) 上記に加えて以下もmountします (skeleton-freertosの
# 動作サンプルを使う場合の例。自分のプロジェクトディレクトリなら適宜読み替え)。
#   --mount type=bind,src=$(pwd)/freertos-kernel-V11.2.0,dst=/home/builder/freertos-kernel \
#   --mount type=bind,src=$(pwd)/skeleton-freertos,dst=/home/builder/skeleton-freertos \

# docker exec -it pico-builder-dev /bin/bash


# --- picotoolのビルド (初回のみ、コンテナ内で) ---
# cmake ..が生成したMakefileをmakeが読んでビルドします。
# cd picotool
# mkdir -p build && cd build
# cmake ..
# make -j$(nproc)
# sudo make install
# cd ../..


# --- 自分のプロジェクトをビルド ---
# cd skeleton
# cp /home/builder/pico-sdk/external/pico_sdk_import.cmake .
# mkdir -p build && cd build
# cmake .. -DPICO_BOARD=pico -DPICO_SDK_PATH=/home/builder/pico-sdk   # Pico W: pico_w / Pico 2: pico2
# (compile_commands.jsonも作る場合は -DCMAKE_EXPORT_COMPILE_COMMANDS=ON を追加)
# make -j$(nproc)
# cd ..
# 成果物: build/blink.uf2 (ファイル名はディレクトリ名ではなくCMakeLists.txtの
# project()/add_executable()で付けた名前になる)


# --- picotoolで書き込み ---
# (BOOTSELモードでUSB接続したPicoに対して書き込みます。コンテナは --device=/dev/bus/usb で起動しておく必要があります)
# picotool load -x build/blink.uf2
# picotool reboot


# --- host側でcompile_commands.jsonのパスをホスト用に置換 ---
# sed -i \
#   -e "s|/home/builder/pico-sdk|$(pwd)/pico-sdk-2.3.1|g" \
#   -e "s|/home/builder/skeleton|$(pwd)/skeleton|g" \
#   skeleton/build/compile_commands.json
# （置換後compile_commands.jsonをskeleton, pico-sdkと同階層に配置）


# --- SDKのバージョンを上げる場合 ---
# 別ディレクトリに再clone (例: pico-sdk-2.4.0) して、docker run の --mount の
# src と、cmakeの -DPICO_SDK_PATH をそこに差し替えるだけでよいです。イメージの
# 再ビルドは不要です。


# =============================================================================
# (オプション) FreeRTOSを組み込む場合
# clone/mountはそれぞれ上のセクションに追記済みです。ここでは自分のプロジェクト
# への追加手順のみ記載します。
# =============================================================================

# --- 自分のプロジェクトへの追加手順 ---
# 動作するサンプルは ./skeleton-freertos を参照してください
# (CMakeLists.txt, FreeRTOSConfig.h, FreeRTOS_Kernel_import.cmakeを含む)。
# RP2040用ポートは portable/ThirdParty/GCC/RP2040 にあります。
# cd skeleton-freertos
# cp /home/builder/pico-sdk/external/pico_sdk_import.cmake .
# cp /home/builder/freertos-kernel/portable/ThirdParty/GCC/RP2040/FreeRTOS_Kernel_import.cmake .
# mkdir -p build && cd build
# cmake .. -DPICO_BOARD=pico -DPICO_SDK_PATH=/home/builder/pico-sdk -DFREERTOS_KERNEL_PATH=/home/builder/freertos-kernel
# (compile_commands.jsonも作る場合は -DCMAKE_EXPORT_COMPILE_COMMANDS=ON を追加)
# make -j$(nproc)
# 成果物: build/<add_executableで付けた名前>.uf2


# --- host側でcompile_commands.jsonのパスをホスト用に置換 (skeleton-freertos) ---
# sed -i \
#   -e "s|/home/builder/pico-sdk|$(pwd)/pico-sdk-2.3.1|g" \
#   -e "s|/home/builder/freertos-kernel|$(pwd)/freertos-kernel-V11.2.0|g" \
#   -e "s|/home/builder/skeleton-freertos|$(pwd)/skeleton-freertos|g" \
#   skeleton-freertos/build/compile_commands.json
