FROM ubuntu:18.04

RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y \
    software-properties-common \
    sudo \
    gnome-session \
    gnome-session-flashback \
    light-themes \
    nemo \
    lxterminal \
    gnome-terminal \
    xrdp \
    openssh-server \
    wget \
    curl \
    nkf \
    gftp \
    locales \
    tzdata \
    psmisc \
    netcat-openbsd \
    telnet \
    iputils-ping \
    net-tools \
    iproute2 \
    tcpdump \
    g++ \
    make \
    python \
    python3 \
    tmux \
    supervisor

RUN DEBIAN_FRONTEND=noninteractive add-apt-repository ppa:jonathonf/vim
RUN apt-get update && DEBIAN_FRONTEND=noninteractive apt-get install -y vim

RUN apt-get clean && rm -rf /var/cache/apt/archives/* && rm -rf /var/lib/apt/lists/*

# for debug
RUN echo root:root | chpasswd

# set locale
RUN locale-gen en_US.UTF-8
ENV LANG en_US.UTF-8
ENV LANGUAGE en_US:en
ENV LC_ALL en_US.UTF-8

RUN sed -i -e 's/FuseMountName=thinclient_drives/FuseMountName=\.thinclient_drives/g' /etc/xrdp/sesman.ini

RUN install -o root -g xrdp -m 2775 -d /var/run/xrdp
RUN install -o root -g xrdp -m 3777 -d /var/run/xrdp/sockdir
RUN install -o root -g root -m 0755 -d /var/run/dbus
RUN install -o root -g root -m 0755 -d /var/run/sshd

# setup supervisord config
RUN { \
      echo "[supervisord]"; \
      echo "user=root"; \
      echo "nodaemon=true"; \
      echo "logfile=/var/log/supervisor/supervisord.log"; \
      echo "childlogdir=/var/log/supervisor"; \
      echo ""; \
      echo "[program:sshd]"; \
      echo "command=/usr/sbin/sshd -D"; \
      echo ""; \
      echo "[program:dbus]"; \
      echo "command=/usr/bin/dbus-daemon --system --nofork --nopidfile"; \
      echo ""; \
      echo "[program:xrdp-sesman]"; \
      echo "command=/usr/sbin/xrdp-sesman --nodaemon"; \
      echo ""; \
      echo "[program:xrdp]"; \
      echo "command=/usr/sbin/xrdp --nodaemon"; \
      echo "user=xrdp"; \
    } > /root/services.conf


# add user
ARG UID=1000
ARG GID=1000
ARG USERNAME=develop
ARG GROUPNAME=develop
ARG PASSWORD=develop
RUN groupadd -g ${GID} ${GROUPNAME} && \
  useradd -m -s /bin/bash -u ${UID} -g ${GID} ${USERNAME} && \
  echo ${USERNAME}:${PASSWORD} | chpasswd

USER develop

# setup desktop
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.nautilus.preferences executable-text-activation ask"
RUN /bin/bash -lc "xdg-mime default nemo.desktop inode/directory application/x-gnome-saved-search"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.desktop.background show-desktop-icons true"
RUN /bin/bash -lc "dbus-launch gsettings set org.nemo.desktop show-desktop-icons true"
#RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.shell.extensions.dash-to-dock dash-max-icon-size 30"
#RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.shell.extensions.dash-to-dock dock-position 'BOTTOM'"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.shell favorite-apps []"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.nautilus.desktop trash-icon-visible false"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.nautilus.desktop home-icon-visible false"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.desktop.screensaver lock-enabled false"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.desktop.screensaver idle-activation-enabled false"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.desktop.interface enable-animations false"
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.desktop.input-sources sources \"[('xkb', 'us')]\""
RUN /bin/bash -lc "dbus-launch gsettings set org.gnome.desktop.background picture-uri 'file:///usr/share/backgrounds/warty-final-ubuntu.png'"

# setup xsession
RUN echo "gnome-session --session=gnome-flashback-metacity" > ~/.xsession
RUN { \
      echo "export DESKTOP_SESSION=gnome-flashback-metacity"; \
      echo 'export GDMSESSION=${DESKTOP_SESSION}'; \
      echo "export GNOME_SHELL_SESSION_MODE=ubuntu"; \
      echo 'export XDG_SESSION_DESKTOP=${DESKTOP_SESSION}'; \
      echo "export XDG_CURRENT_DESKTOP=GNOME-Flashback:GNOME:"; \
      echo ""; \
      echo "# set locale"; \
      echo "export LANG=en_US.UTF-8"; \
      echo "export LANGUAGE=en_US:en"; \
      echo "export LC_ALL=en_US.UTF-8"; \
    } > ~/.xsessionrc

# setup logout button
RUN mkdir -p ~/.local/share/applications
RUN { \
      echo "[Desktop Entry]"; \
      echo "Name=Log Out..."; \
      echo "Exec=gnome-session-quit --logout"; \
      echo "Terminal=false"; \
      echo "Type=Application"; \
      echo "Icon=system-logout"; \
    } > ~/.local/share/applications/logout-for-dock.desktop


USER root

# copy entrypoint script
#COPY entrypoint.sh /root/
#RUN chmod +x /root/entrypoint.sh

EXPOSE 22 3389

#CMD /bin/bash
#ENTRYPOINT ["/root/entrypoint.sh"]
ENTRYPOINT ["/usr/bin/supervisord", "-c", "/root/services.conf"]


# -----------------------------------------------------------------------------
# --- docker build ---
# docker build -t xrdp_test .

# --- run daemon mode ---
# docker run -it -d -p 33891:3389 -p 22001:22 --name "xrdp_test_con" xrdp_test

# --- bash console ---
# docker exec -it -w /root xrdp_test_con /bin/bash -il
# docker exec -it -u develop -w /home/test xrdp_test_con /bin/bash -il

