FROM nginx:stable-alpine

RUN apk update && \
    apk add \
        expat-dev \
        g++ \
        make \
        supervisor \
        wget \
        zlib-dev

COPY . /app/
COPY etc/supervisord.conf /etc/supervisor/conf.d/supervisord.conf

RUN cd /app && \
    ./configure --prefix=/app &&
    make install clean

EXPOSE 80
CMD ["/usr/bin/supervisord"]

