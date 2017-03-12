#!/bin/bash

set -eo pipefail
shopt -s nullglob
OVERPASS_META=${OVERPASS_META:-no}
OVERPASS_MODE=${OVERPASS_MODE:-clone}

if [ ! -d /db/db ] ; then
    if [ "$OVERPASS_MODE" = "clone" ]; then
        shift
        /app/bin/download_clone.sh --db-dir=/db/db --source=http://dev.overpass-api.de/api_drolbr/ "--meta=$OVERPASS_META"
    fi

    if [ "$OVERPASS_MODE" = "init" ]; then
        lftp -c "get -c $OVERPASS_PLANET_URL -o /db/planet; exit" \
        && /app/bin/init_osm3s.sh /db/planet /db/db /app "--meta=$OVERPASS_META" \
        && echo $OVERPASS_MINUTE_ID > /db/db/replicate_id \
        && rm /db/planet \
        && chown -R overpass:overpass /db \
        && echo "Overpass ready, you can start you container with docker start"
        exit
    fi
fi

/usr/bin/supervisord -c /etc/supervisor/conf.d/supervisord.conf
