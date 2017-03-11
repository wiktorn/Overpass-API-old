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
        wget -O /db/planet "$OVERPASS_PLANET_URL"
        /app/bin/init_osm3s.sh /db/planet /db/db /app/bin "--meta=$OVERPASS_META"
        rm /db/planet
    fi
fi

/usr/bin/supervisord -c /etc/supervisor/conf.d/supervisord.conf
