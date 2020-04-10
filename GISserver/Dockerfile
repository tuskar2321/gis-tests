FROM centos:7.2.1511

RUN yum install -y epel-release && \
    yum updateinfo && \
    yum install -y \
    yum-plugin-ovl \
    bind-libs \
    httpd \
    php \
    libcurl \
    php-gd \
    php-mbstring \
    php-pgsql \
    libicu \
    php-mysql \
    php-xml \
    php-xmlrpc \
    php-zip \ 
    postgis \
    qt-x11 

COPY gisserver.noarch.rpm /home/root/

RUN rpm -i /home/root/gisserver.noarch.rpm 

EXPOSE 2047

CMD LD_LIBRARY_PATH=/usr/gisserver /usr/gisserver/gsservice.exe && /bin/bash 
   
