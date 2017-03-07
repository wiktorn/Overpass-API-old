FROM nginx:stable-alpine

RUN apk update && \
    apk add \
        autoconf \
        automake \
        expat \
        expat-dev \
        g++ \
        libtool \
        m4 \
        make \
        supervisor \
        wget \
        zlib \
        zlib-dev

COPY . /app/
COPY etc/supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN cd /app/src && \
    autoscan && \
    aclocal && \
    autoheader && \
    libtoolize && \
    automake --add-missing && \
    autoconf && \
    ./configure --prefix=/app && \
    make install clean

RUN apk del \
        autoconf \
        automake \
        expat-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib-dev

EXPOSE 80
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/supervisord.conf"]

