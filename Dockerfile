FROM nginx:stable-alpine

RUN addgroup -S overpass && adduser -D -S -h /db -s /sbin/nologin -G overpass overpass

RUN apk add --no-cache --virtual .build-deps \
        autoconf \
        automake \
        expat-dev \
        g++ \
        libtool \
        m4 \
        make \
        zlib-dev

COPY . /app/

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

RUN apk add --no-cache --virtual .run-deps \
        supervisor \
        bash \
        lftp \
        fcgiwrap \
        bzip2 \
        wget

RUN mkdir /nginx && chown nginx:nginx /nginx && mkdir -p /db/db /db/diffs && chown -R overpass:overpass /db
COPY etc/supervisord.conf /etc/supervisor/conf.d/supervisord.conf
COPY etc/nginx-overpass.conf /etc/nginx/nginx.conf
VOLUME /db

EXPOSE 80
CMD ["/app/docker-entrypoint.sh"]
