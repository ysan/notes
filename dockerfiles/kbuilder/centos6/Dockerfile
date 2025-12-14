FROM centos:6

RUN sed -i 's/^mirrorlist/#mirrorlist/g' /etc/yum.repos.d/CentOS-Base.repo && \
  sed -i 's/^#baseurl=http:\/\/mirror.centos.org\/centos\/$releasever\//baseurl=http:\/\/vault.centos.org\/6.10\//g' /etc/yum.repos.d/CentOS-Base.repo && \
  yum clean all

RUN yum -y update && \
  yum -y groupinstall "Development Tools" && \
  yum -y install \
  wget \
  curl \
  tar \
  rpm-build \
  redhat-rpm-config \
  ncurses-devel \
  hmaccalc \
  zlib-devel \
  binutils-devel \
  elfutils-libelf-devel \
  bc \
  sudo \
  yum-utils

RUN rm -f /var/lib/rpm/__db*
RUN rpm --rebuilddb
RUN yum clean all
RUN yum update -y coreutils

RUN echo root:root | chpasswd

# add user
ARG UID=1000
ARG GID=1000
ARG USER=builder
ARG GROUP=builder
ARG PASS=builder
RUN groupadd -g $GID ${GROUP} && \
  useradd -m -s /bin/bash -u ${UID} -g ${GID} -G wheel ${USER} && \
  echo ${USER}:${PASS} | chpasswd
RUN echo "%wheel ALL=(ALL) NOPASSWD: ALL" >> /etc/sudoers

# switch user
USER ${USER}
WORKDIR /home/${USER}

ENV GIT_SSL_NO_VERIFY 1

CMD /bin/bash


# -----------------------------------------------------------------------------
# KNAME=kernel-2.6.32-754.35.1.el6
# curl -LOk http://vault.centos.org/6.10/updates/Source/SPackages/${KNAME}.src.rpm
# mkdir ${KNAME} && cd ${KNAME}
# rpm2cpio ../${KNAME}.src.rpm | cpio -idmv
# mkdir -p ./rpmbuild/{BUILD,RPMS,SOURCES,SPECS,SRPMS}
# mv *.spec ./rpmbuild/SPECS/
# mv * ./rpmbuild/SOURCES/

# --- docker build ---
# docker build -t kbuilder-centos6 --build-arg UID=$(id -u) --build-arg GID=$(id -g) .

# --- run daemon mode ---
# docker run -it -d --name kbuilder-centos6_con \
#   --mount type=bind,src=/path/to/rpmbuild,dst=/home/builder/rpmbuild \
#   kbuilder-centos6

# docker exec -it kbuilder-centos6_con /bin/bash
# docker exec -it -u builder -w /home/builder kbuilder-centos6_con /bin/bash

# --- build kernel ---
# cd rpmbuild/SPEC
# - It stops midway through, but is it installed?
# - Stop it with ctrl+\ and run it again until it finishes.
# sudo yum-builddep kernel.spec
# rpmbuild -bp kernel.spec
# rpmbuild -bc kernel.spec

# --- create build log ---
# - modify kernel.spec
# 1085c1085
# <     make -s %{?cross_opts} ARCH=$Arch V=1 %{?_smp_mflags} $MakeTarget %{?sparse_mflags}
# ---
# >     make %{?cross_opts} ARCH=$Arch V=1 %{?_smp_mflags} $MakeTarget %{?sparse_mflags} > build_log.txt
# rpmbuild --clean
# rpmbuild -bp kernel.spec
# rpmbuild -bc --without debug --without debuginfo kernel.spec
#
# --- create compile_commands.json at host ---
# pip3 install compiledb
# compiledb -p build_log.txt
# sed -i -e 's/\/home\/builder\/rpmbuild/\/home\/user\/path\/to\/rpmbuild/g' compile_commands.json

