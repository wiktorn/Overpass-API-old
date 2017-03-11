FROM nginx:stable

RUN addgroup overpass && adduser --home /db --disabled-password --ingroup overpass overpass

RUN apt-get update \
    && apt-get install --no-install-recommends --no-install-suggests -y \
        autoconf \
        automake \
        expat \
        expat-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib \
        zlib-dev

RUN apt-get install --no-install-recommends --no-install-suggests -y \
        supervisor \
        bash \
        lftp


COPY . /app/
COPY etc/supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN cd /app/src \
    && autoscan \
    && aclocal \
    && autoheader \
    && libtoolize \
    && automake --add-missing  \
    && autoconf \
    && ./configure --prefix=/app  \
    && make -j $(grep -c ^processor /proc/cpuinfo) install clean \
    && apt-get remove \
        autoconf \
        automake \
        expat-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib-dev \
    && apt-get autoremove

EXPOSE 80
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/supervisord.conf"]
ENTRYPOINT ["/app/docker-entrypoint.sh"]
