# avr-libc 2.1以降でないとtinyAVR 0/1/2シリーズに対応しないため、
# bookwormではなくtrixieを使います。
FROM debian:trixie-slim

ENV DEBIAN_FRONTEND=noninteractive

RUN apt-get update && apt-get install -y --no-install-recommends \
  build-essential \
  gcc-avr \
  binutils-avr \
  avr-libc \
  avrdude \
  make \
  git \
  ca-certificates \
  sudo \
  vim \
  && rm -rf /var/lib/apt/lists/*

# ユーザーを追加します。ホスト側のUID/GIDに合わせることで、bind mount先の
# ファイルがrootで汚染されないようにします。
# dialoutグループはシリアルポート (/dev/ttyUSB*) へのアクセス用です。
ARG UID=1000
ARG GID=1000
ARG USER=builder
ARG GROUP=builder
RUN groupadd -g ${GID} ${GROUP} && \
  useradd -m -s /bin/bash -u ${UID} -g ${GID} -G sudo,dialout ${USER} && \
  echo "${USER} ALL=NOPASSWD: ALL" > /etc/sudoers.d/container_users

# ユーザーを切り替えます
USER ${USER}
WORKDIR /home/${USER}

CMD ["/bin/bash"]


# =============================================================================
# AVR (ATtiny404等のtinyAVR) ビルド・書き込み環境
# 自分のアプリのソースは別ディレクトリに用意し、bind mountします (例: ./skeleton)
# =============================================================================

# --- docker build ---
# docker build -t avr-builder --build-arg UID=$(id -u) --build-arg GID=$(id -g) .


# --- daemonモードで起動 ---
# 書き込みにはUPDI対応のUSBシリアルアダプタ (USB-シリアル変換 + 4.7k程度の抵抗を
# TXD-RXD間に入れた「SerialUPDI」構成など) を使い、ホストのデバイスを渡します。
# 書き込みをしないなら --device は外してよい。
# docker run -it -d --name avr-builder-dev \
#   --mount type=bind,src=$(pwd)/skeleton,dst=/home/builder/skeleton \
#   --device=/dev/ttyUSB0 \
#   avr-builder
#
# ホスト側で /dev/ttyUSB0 がdialout等のグループ所有で、コンテナ内ユーザーが
# 読み書きできない場合は --group-add $(stat -c %g /dev/ttyUSB0) を追加します。
# (SELinux環境でbind mountが拒否される場合は --mount ではなく
#  -v $(pwd)/skeleton:/home/builder/skeleton:Z を使う)

# docker exec -it avr-builder-dev /bin/bash


# --- ビルド ---
# cd skeleton
# make MCU=attiny1604 PART=t1604    # blink.hex を生成 (MCU, PARTは必須)
# (F_CPUはデフォルト3.3MHz = 20MHz内蔵OSC/6分周。変更時は make F_CPU=20000000UL)
#
# 型番の対応 (チップを変えるときは両方変える。詳細はskeleton/Makefileの冒頭):
#   MCU  = avr-gcc用。"attiny"+型番 (attiny404, attiny1604)。
#          gccのdevice-specsが選ばれ、avr/io.h経由でiotn<型番>.h (レジスタ定義) が読まれる。
#          一覧: avr-size --mlist-devices
#   PART = avrdude用。"t"+型番 (t404, t1604)。/etc/avrdude.conf のpart ID。
#          一覧: avrdude -p ?


# --- 書き込み (UPDI) ---
# make flash MCU=attiny1604 PART=t1604 PORT=/dev/ttyUSB0
# 直接avrdudeを呼ぶ場合:
# avrdude -p t1604 -c serialupdi -P /dev/ttyUSB0 -U flash:w:blink.hex:i
# 接続確認だけ: avrdude -p t1604 -c serialupdi -P /dev/ttyUSB0 -v
# (うまくいかない場合は -b 57600 を付けてボーレートを下げる)
# (Atmel-ICE / Curiosity Nano等を使う場合は -c atmelice_updi / pkobn_updi に変更)

