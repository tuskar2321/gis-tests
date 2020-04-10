FROM centos:7.2.1511

# Install python-reqs
RUN yum install -y \
    python3 \
    python3-devel \
    python3-pip \
    gcc \
    iproute \
    iputils \
    && yum update \
    || true

ADD requirements.txt /
RUN pip3 install -r requirements.txt

# Build pymapapi
ADD ./gisserver.noarch.rpm /
RUN yum localinstall -y gisserver.noarch.rpm || true

ADD pymapapi /
RUN python3 setup_pymapapi.py install

# Run tests
ADD . /
CMD LD_LIBRARY_PATH=/usr/gisserver/ pytest test_integration.py
