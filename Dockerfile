FROM nginx:stable-alpine

RUN apk add --no-cache --virtual .build-deps \
        autoconf \
        automake \
        expat-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib-dev

RUN apk add --no-cache --virtual .run-deps \
        supervisor \
        wget

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
    && runDeps="$( \
        scanelf --needed --nobanner /app/bin/* \
            | awk '{ gsub(/,/, "\nso:", $2); print "so:" $2 }' \
            | sort -u \
            | xargs -r apk info --installed \
            | sort -u \
    )" \
    && apk add --no-cache --virtual .overpass-rundeps $runDeps \
    && apk del .build-deps

EXPOSE 80
CMD ["/usr/bin/supervisord", "-c", "/etc/supervisor/conf.d/supervisord.conf"]

